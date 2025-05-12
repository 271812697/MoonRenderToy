#include "Renderer.h"
#include "Camera.h"
#include "Texture.h"
#include "Quad.h"
#include "opengl/asset/fbo.h"
#include "opengl/asset/shader.h"
#include "opengl/asset/buffer.h"
#include "Scene.h"
#include "stb_image/stb_image.h"
#include "stb_image/stb_image_write.h"
#include "core/log.h"


#include "oidn/include/OpenImageDenoise/oidn.hpp"
#include <fstream>
namespace PathTrace {

	extern bool space_down;
	extern float screenX[2];
	extern float screenY[2];

}
inline bool raster = false;
namespace PathTrace
{
	// 加载shader的源码
	std::string loadShaderSource(std::string path)
	{
		static bool isRecursiveCall = false;
		std::string fullSourceCode = "";
		std::ifstream file(path);
		if (!file.is_open())
		{
			CORE_ERROR("ERROR: could not open the shader at: {0}\n", path);

			return "";
		}
		std::string lineBuffer;
		while (std::getline(file, lineBuffer))
		{
			fullSourceCode += lineBuffer + '\n';
		}
		fullSourceCode += '\0';
		file.close();
		return fullSourceCode;
	}


	Renderer::Renderer(Scene* scene, const std::string& shadersDirectory)
		: scene(scene)
		, BVHBuffer(0)
		, BVHTex(0)
		, vertexIndicesBuffer(0)
		, vertexIndicesTex(0)
		, verticesBuffer(0)
		, verticesTex(0)
		, normalsBuffer(0)
		, normalsTex(0)
		, materialsTex(0)
		, transformsTex(0)
		, lightsTex(0)
		, textureMapsArrayTex(0)
		, envMapTex(0)
		, envMapCDFTex(0)
		, pathTraceTextureLowRes(0)
		, pathTraceTexture(0)
		, accumTexture(0)
		, tileOutputTexture()
		, denoisedTexture(0)
		, shadersDirectory(shadersDirectory)
		, pathTraceShaderLowRes(nullptr)
		, outputShader(nullptr)
		, tonemapShader(nullptr)
	{
		if (scene == nullptr)
		{

			printf("No Scene Found\n");
			return;
		}

		if (!scene->initialized)
			scene->ProcessScene();

		InitGPUDataBuffers();
		quad = new Quad();
		pixelRatio = 0.25f;

		InitFBOs();
		InitShaders();
		PreRaster();
	}

	Renderer::~Renderer()
	{
		delete quad;

		glDeleteTextures(1, &BVHTex);
		glDeleteTextures(1, &vertexIndicesTex);
		glDeleteTextures(1, &verticesTex);
		glDeleteTextures(1, &normalsTex);
		glDeleteTextures(1, &materialsTex);
		glDeleteTextures(1, &transformsTex);
		glDeleteTextures(1, &lightsTex);
		glDeleteTextures(1, &textureMapsArrayTex);
		glDeleteTextures(1, &envMapTex);
		glDeleteTextures(1, &envMapCDFTex);
		glDeleteTextures(1, &pathTraceTexture);
		glDeleteTextures(1, &pathTraceTextureLowRes);
		glDeleteTextures(1, &accumTexture);
		glDeleteTextures(1, &tileOutputTexture[0]);
		glDeleteTextures(1, &tileOutputTexture[1]);
		glDeleteTextures(1, &denoisedTexture);

		glDeleteBuffers(1, &BVHBuffer);
		glDeleteBuffers(1, &vertexIndicesBuffer);
		glDeleteBuffers(1, &verticesBuffer);
		glDeleteBuffers(1, &normalsBuffer);

		delete[] denoiserInputFramePtr;
		delete[] frameOutputPtr;

	}

	void Renderer::InitGPUDataBuffers()
	{

		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		// BVH
		//纹理缓冲对象
		glGenBuffers(1, &BVHBuffer);
		glBindBuffer(GL_TEXTURE_BUFFER, BVHBuffer);
		glBufferData(GL_TEXTURE_BUFFER, sizeof(RadeonRays::BvhTranslator::Node) * scene->bvhTranslator.nodes.size(), &scene->bvhTranslator.nodes[0], GL_STATIC_DRAW);
		glGenTextures(1, &BVHTex);
		glBindTexture(GL_TEXTURE_BUFFER, BVHTex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, BVHBuffer);

		//顶点索引
		glGenBuffers(1, &vertexIndicesBuffer);
		glBindBuffer(GL_TEXTURE_BUFFER, vertexIndicesBuffer);
		glBufferData(GL_TEXTURE_BUFFER, sizeof(Indices) * scene->vertIndices.size(), &scene->vertIndices[0], GL_STATIC_DRAW);
		glGenTextures(1, &vertexIndicesTex);
		glBindTexture(GL_TEXTURE_BUFFER, vertexIndicesTex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32I, vertexIndicesBuffer);

		// 顶点坐标 U
		glGenBuffers(1, &verticesBuffer);
		glBindBuffer(GL_TEXTURE_BUFFER, verticesBuffer);
		glBufferData(GL_TEXTURE_BUFFER, sizeof(Vec4) * scene->verticesUVX.size(), &scene->verticesUVX[0], GL_STATIC_DRAW);
		glGenTextures(1, &verticesTex);
		glBindTexture(GL_TEXTURE_BUFFER, verticesTex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, verticesBuffer);

		// 顶点法线 V
		glGenBuffers(1, &normalsBuffer);
		glBindBuffer(GL_TEXTURE_BUFFER, normalsBuffer);
		glBufferData(GL_TEXTURE_BUFFER, sizeof(Vec4) * scene->normalsUVY.size(), &scene->normalsUVY[0], GL_STATIC_DRAW);
		glGenTextures(1, &normalsTex);
		glBindTexture(GL_TEXTURE_BUFFER, normalsTex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, normalsBuffer);

		//材质
		glGenTextures(1, &materialsTex);
		glBindTexture(GL_TEXTURE_2D, materialsTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (sizeof(Material) / sizeof(Vec4)) * scene->materials.size(), 1, 0, GL_RGBA, GL_FLOAT, &scene->materials[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Transform
		glGenTextures(1, &transformsTex);
		glBindTexture(GL_TEXTURE_2D, transformsTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (sizeof(Mat4) / sizeof(Vec4)) * scene->transforms.size(), 1, 0, GL_RGBA, GL_FLOAT, &scene->transforms[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		// 光源信息
		if (!scene->lights.empty())
		{

			glGenTextures(1, &lightsTex);
			glBindTexture(GL_TEXTURE_2D, lightsTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, (sizeof(Light) / sizeof(Vec3)) * scene->lights.size(), 1, 0, GL_RGB, GL_FLOAT, &scene->lights[0]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// 场景的贴图数组
		if (!scene->textures.empty())
		{
			glGenTextures(1, &textureMapsArrayTex);
			glBindTexture(GL_TEXTURE_2D_ARRAY, textureMapsArrayTex);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, scene->renderOptions.texArrayWidth, scene->renderOptions.texArrayHeight, scene->textures.size(),
				0, GL_RGBA, GL_UNSIGNED_BYTE, &scene->textureMapsArray[0]);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		}

		//环境贴图，包括累加和
		if (scene->envMap != nullptr)
		{
			glGenTextures(1, &envMapTex);
			glBindTexture(GL_TEXTURE_2D, envMapTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, scene->envMap->width, scene->envMap->height, 0, GL_RGB, GL_FLOAT, scene->envMap->img);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);

			glGenTextures(1, &envMapCDFTex);
			glBindTexture(GL_TEXTURE_2D, envMapCDFTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, scene->envMap->width, scene->envMap->height, 0, GL_RED, GL_FLOAT, scene->envMap->cdf);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// 绑定到对应的槽位
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, BVHTex);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_BUFFER, vertexIndicesTex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_BUFFER, verticesTex);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_BUFFER, normalsTex);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, materialsTex);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, transformsTex);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, lightsTex);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D_ARRAY, textureMapsArrayTex);
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, envMapTex);
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, envMapCDFTex);
	}

	void Renderer::ResizeRenderer()
	{
		//重置贴图
		glDeleteTextures(1, &pathTraceTexture);
		glDeleteTextures(1, &pathTraceTextureLowRes);
		glDeleteTextures(1, &accumTexture);
		glDeleteTextures(1, &tileOutputTexture[0]);
		glDeleteTextures(1, &tileOutputTexture[1]);
		glDeleteTextures(1, &denoisedTexture);



		delete[] denoiserInputFramePtr;
		delete[] frameOutputPtr;



		InitFBOs();
		InitShaders();
	}

	void Renderer::InitFBOs()
	{
		sampleCounter = 1;
		currentBuffer = 0;
		frameCounter = 1;

		renderSize = scene->renderOptions.renderResolution;
		windowSize = scene->renderOptions.windowResolution;

		tileWidth = scene->renderOptions.tileWidth;
		tileHeight = scene->renderOptions.tileHeight;
		//单块瓦片所占据的uv区间长度
		invNumTiles.x = (float)tileWidth / renderSize.x;
		invNumTiles.y = (float)tileHeight / renderSize.y;

		numTiles.x = ceil((float)renderSize.x / tileWidth);
		numTiles.y = ceil((float)renderSize.y / tileHeight);

		tile.x = -1;
		tile.y = numTiles.y - 1;

		pathTracefbo.reset();
		pathTracefbo = std::make_shared<asset::FBO>(tileWidth, tileHeight);
		pathTraceFBOLowRes.reset();
		pathTraceFBOLowRes = std::make_shared<asset::FBO>(windowSize.x * pixelRatio, windowSize.y * pixelRatio);
		accumFBO.reset();
		accumFBO = std::make_shared<asset::FBO>(renderSize.x, renderSize.y);
		outputFBO.reset();
		outputFBO = std::make_shared<asset::FBO>(renderSize.x, renderSize.y);

		rasterFBO.reset();
		rasterFBO = std::make_shared<asset::FBO>(renderSize.x, renderSize.y);
		rasterFBO->AddColorTexture(1);
		rasterFBO->AddDepStTexture();
		rasterMsaaFBO.reset();
		rasterMsaaFBO = std::make_shared<asset::FBO>(renderSize.x, renderSize.y);
		rasterMsaaFBO->AddColorTexture(1, true);
		rasterMsaaFBO->AddDepStRenderBuffer(true);




		glGenTextures(1, &pathTraceTexture);
		glBindTexture(GL_TEXTURE_2D, pathTraceTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tileWidth, tileHeight, 0, GL_RGBA, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		pathTracefbo->SetColorTexture(0, pathTraceTexture);

		glGenTextures(1, &pathTraceTextureLowRes);
		glBindTexture(GL_TEXTURE_2D, pathTraceTextureLowRes);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowSize.x * pixelRatio, windowSize.y * pixelRatio, 0, GL_RGBA, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		pathTraceFBOLowRes->SetColorTexture(0, pathTraceTextureLowRes);


		glGenTextures(1, &accumTexture);
		glBindTexture(GL_TEXTURE_2D, accumTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, renderSize.x, renderSize.y, 0, GL_RGBA, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		accumFBO->SetColorTexture(0, accumTexture);

		glGenTextures(1, &tileOutputTexture[0]);
		glBindTexture(GL_TEXTURE_2D, tileOutputTexture[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, renderSize.x, renderSize.y, 0, GL_RGBA, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &tileOutputTexture[1]);
		glBindTexture(GL_TEXTURE_2D, tileOutputTexture[1]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, renderSize.x, renderSize.y, 0, GL_RGBA, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		outputFBO->SetColorTexture(0, tileOutputTexture[currentBuffer]);

		denoiserInputFramePtr = new float[renderSize.x * renderSize.y * 16];
		frameOutputPtr = new float[renderSize.x * renderSize.y * 16];

		glGenTextures(1, &denoisedTexture);
		glBindTexture(GL_TEXTURE_2D, denoisedTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, renderSize.x, renderSize.y, 0, GL_RGB, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

	}

	void Renderer::ReloadShaders()
	{
		InitShaders();
	}
	void Renderer::PreRaster() {

	}
	void Renderer::InitShaders()
	{
		//初始化shader
		std::string pathTraceShaderSrcObj = loadShaderSource(shadersDirectory + "/PathTrace.glsl");
		std::string pathTraceShaderLowResSrcObj = loadShaderSource(shadersDirectory + "/PathTraceLowRes.glsl");
		std::string outputShaderSrcObj = loadShaderSource(shadersDirectory + "/OutputShader.glsl");
		std::string tonemapShaderSrcObj = loadShaderSource(shadersDirectory + "/ToneMapShader.glsl");
		std::string pbrShaderSrcObj = loadShaderSource(shadersDirectory + "/pbr.glsl");
		std::string lineShaderSrcObj = loadShaderSource(shadersDirectory + "/line.glsl");
		//分析renderOptions添加向源码中预定义宏
		std::string pathtraceDefines = "";
		std::string tonemapDefines = "";

		if (scene->renderOptions.enableEnvMap && scene->envMap != nullptr)
			pathtraceDefines += "#define OPT_ENVMAP\n";

		if (!scene->lights.empty()) {
			pathtraceDefines += "#define OPT_LIGHTS\n";
			scene->renderOptions.optLight = true;
		}


		if (scene->renderOptions.enableRR)
		{
			pathtraceDefines += "#define OPT_RR\n";
			pathtraceDefines += "#define OPT_RR_DEPTH " + std::to_string(scene->renderOptions.RRDepth) + "\n";
		}

		if (scene->renderOptions.enableUniformLight)
			pathtraceDefines += "#define OPT_UNIFORM_LIGHT\n";

		if (scene->renderOptions.openglNormalMap)
			pathtraceDefines += "#define OPT_OPENGL_NORMALMAP\n";

		if (scene->renderOptions.hideEmitters)
			pathtraceDefines += "#define OPT_HIDE_EMITTERS\n";

		if (scene->renderOptions.enableBackground)
		{
			pathtraceDefines += "#define OPT_BACKGROUND\n";
			tonemapDefines += "#define OPT_BACKGROUND\n";
		}

		if (scene->renderOptions.transparentBackground)
		{
			pathtraceDefines += "#define OPT_TRANSPARENT_BACKGROUND\n";
			tonemapDefines += "#define OPT_TRANSPARENT_BACKGROUND\n";
		}

		for (int i = 0; i < scene->materials.size(); i++)
		{
			if ((int)scene->materials[i].alphaMode != AlphaMode::Opaque)
			{
				pathtraceDefines += "#define OPT_ALPHA_TEST\n";
				scene->renderOptions.optAlphaTest = true;
				break;
			}
		}

		if (scene->renderOptions.enableRoughnessMollification)
			pathtraceDefines += "#define OPT_ROUGHNESS_MOLLIFICATION\n";

		for (int i = 0; i < scene->materials.size(); i++)
		{
			if ((int)scene->materials[i].mediumType != MediumType::None)
			{
				pathtraceDefines += "#define OPT_MEDIUM\n";
				scene->renderOptions.optMedium = true;
				break;
			}
		}

		if (scene->renderOptions.enableVolumeMIS)
			pathtraceDefines += "#define OPT_VOL_MIS\n";

		if (pathtraceDefines.size() > 0)
		{
			size_t idx = pathTraceShaderSrcObj.find("#ifdef fragment_shader");
			if (idx != -1)
				idx = pathTraceShaderSrcObj.find("\n", idx);
			else
				idx = 0;
			pathTraceShaderSrcObj.insert(idx + 1, pathtraceDefines);

			idx = pathTraceShaderLowResSrcObj.find("#ifdef fragment_shader");
			if (idx != -1)
				idx = pathTraceShaderLowResSrcObj.find("\n", idx);
			else
				idx = 0;
			pathTraceShaderLowResSrcObj.insert(idx + 1, pathtraceDefines);
		}

		if (tonemapDefines.size() > 0)
		{
			size_t idx = tonemapShaderSrcObj.find("#ifdef fragment_shader");
			if (idx != -1)
				idx = tonemapShaderSrcObj.find("\n", idx);
			else
				idx = 0;
			tonemapShaderSrcObj.insert(idx + 1, tonemapDefines);
		}
		/*
		outputShader用于将结果绘制到默认帧缓冲
		pathTraceShader用于结果的一个瓦片
		tonemapShader用于对光线做平均，并作色调映射
		pathTraceShaderLowRes用来预览一个粗糙的结果
		*/
		pathTraceShader.reset();
		pathTraceShaderLowRes.reset();
		outputShader.reset();
		tonemapShader.reset();
		pbrShader.reset();
		lineShader.reset();
		pathTraceShader = std::make_shared<asset::Shader>();
		pathTraceShaderLowRes = std::make_shared<asset::Shader>();
		outputShader = std::make_shared<asset::Shader>();
		tonemapShader = std::make_shared<asset::Shader>();
		pbrShader = std::make_shared<asset::Shader>();
		lineShader = std::make_shared<asset::Shader>();

		pathTraceShader->LoadFromSource(pathTraceShaderSrcObj);
		pathTraceShaderLowRes->LoadFromSource(pathTraceShaderLowResSrcObj);
		outputShader->LoadFromSource(outputShaderSrcObj);
		tonemapShader->LoadFromSource(tonemapShaderSrcObj);
		pbrShader->LoadFromSource(pbrShaderSrcObj);
		lineShader->LoadFromSource(lineShaderSrcObj);

		GLuint shaderObject;
		pathTraceShader->Bind();
		shaderObject = pathTraceShader->ID();

		//初始化pathTraceShader中一些uniform的指标
		if (scene->envMap)
		{
			glUniform2f(glGetUniformLocation(shaderObject, "envMapRes"), (float)scene->envMap->width, (float)scene->envMap->height);
			glUniform1f(glGetUniformLocation(shaderObject, "envMapTotalSum"), scene->envMap->totalSum);
		}

		glUniform1i(glGetUniformLocation(shaderObject, "topBVHIndex"), scene->bvhTranslator.topLevelIndex);
		glUniform2f(glGetUniformLocation(shaderObject, "resolution"), float(renderSize.x), float(renderSize.y));
		glUniform2f(glGetUniformLocation(shaderObject, "invNumTiles"), invNumTiles.x, invNumTiles.y);
		glUniform1i(glGetUniformLocation(shaderObject, "numOfLights"), scene->lights.size());
		glUniform1i(glGetUniformLocation(shaderObject, "accumTexture"), 0);
		glUniform1i(glGetUniformLocation(shaderObject, "BVH"), 1);
		glUniform1i(glGetUniformLocation(shaderObject, "vertexIndicesTex"), 2);
		glUniform1i(glGetUniformLocation(shaderObject, "verticesTex"), 3);
		glUniform1i(glGetUniformLocation(shaderObject, "normalsTex"), 4);
		glUniform1i(glGetUniformLocation(shaderObject, "materialsTex"), 5);
		glUniform1i(glGetUniformLocation(shaderObject, "transformsTex"), 6);
		glUniform1i(glGetUniformLocation(shaderObject, "lightsTex"), 7);
		glUniform1i(glGetUniformLocation(shaderObject, "textureMapsArrayTex"), 8);
		glUniform1i(glGetUniformLocation(shaderObject, "envMapTex"), 9);
		glUniform1i(glGetUniformLocation(shaderObject, "envMapCDFTex"), 10);
		pathTraceShader->Unbind();

		//上传pathTraceShaderLowRes中的uniform
		pathTraceShaderLowRes->Bind();
		shaderObject = pathTraceShaderLowRes->ID();

		if (scene->envMap)
		{
			glUniform2f(glGetUniformLocation(shaderObject, "envMapRes"), (float)scene->envMap->width, (float)scene->envMap->height);
			glUniform1f(glGetUniformLocation(shaderObject, "envMapTotalSum"), scene->envMap->totalSum);
		}
		glUniform1i(glGetUniformLocation(shaderObject, "topBVHIndex"), scene->bvhTranslator.topLevelIndex);
		glUniform2f(glGetUniformLocation(shaderObject, "resolution"), float(renderSize.x), float(renderSize.y));
		glUniform1i(glGetUniformLocation(shaderObject, "numOfLights"), scene->lights.size());
		glUniform1i(glGetUniformLocation(shaderObject, "accumTexture"), 0);
		glUniform1i(glGetUniformLocation(shaderObject, "BVH"), 1);
		glUniform1i(glGetUniformLocation(shaderObject, "vertexIndicesTex"), 2);
		glUniform1i(glGetUniformLocation(shaderObject, "verticesTex"), 3);
		glUniform1i(glGetUniformLocation(shaderObject, "normalsTex"), 4);
		glUniform1i(glGetUniformLocation(shaderObject, "materialsTex"), 5);
		glUniform1i(glGetUniformLocation(shaderObject, "transformsTex"), 6);
		glUniform1i(glGetUniformLocation(shaderObject, "lightsTex"), 7);
		glUniform1i(glGetUniformLocation(shaderObject, "textureMapsArrayTex"), 8);
		glUniform1i(glGetUniformLocation(shaderObject, "envMapTex"), 9);
		glUniform1i(glGetUniformLocation(shaderObject, "envMapCDFTex"), 10);
		pathTraceShaderLowRes->Unbind();
		outputShader->Bind();
		glUniform1i(glGetUniformLocation(outputShader->ID(), "imgTex"), 0);
		outputShader->Unbind();
	}
	void Renderer::RenderPBR() {

		rasterMsaaFBO->Clear();
		rasterMsaaFBO->Bind();
		pbrShader->Bind();
		irradiance_map->Bind(17);
		prefiltered_map->Bind(18);
		BRDF_LUT->Bind(19);
		float viewMatrix[16];
		float projMatrix[16];
		scene->camera->ComputeViewProjectionMatrix(viewMatrix, projMatrix, 1.0f * renderSize.x / renderSize.y);
		glProgramUniformMatrix4fv(pbrShader->ID(), 1001, 1, GL_FALSE, viewMatrix);
		glProgramUniformMatrix4fv(pbrShader->ID(), 1002, 1, GL_FALSE, projMatrix);
		glProgramUniform3fv(pbrShader->ID(), 1003, 1, &scene->camera->position.x);

		//设置场景中光源的数量
		glProgramUniform1i(pbrShader->ID(), 932, scene->lights.size());

		unsigned int model[2] = { 1,0 };
		glProgramUniform2uiv(pbrShader->ID(), 999, 1, model);
		for (auto& meshinstace : scene->meshInstances) {
			Mesh* mesh = scene->meshes[meshinstace.meshID];
			auto& mat = scene->materials[meshinstace.materialID];
			float scale[2] = { 1.0,1.0 };
			float anisotropy[3] = { 0.0,0.0,0.0 };
			// shared properties
			glProgramUniform3fv(pbrShader->ID(), 912, 1, &mat.baseColor.x);
			glProgramUniform1f(pbrShader->ID(), 913, mat.roughness);
			glProgramUniform1f(pbrShader->ID(), 914, 1.0f);
			glProgramUniform3fv(pbrShader->ID(), 915, 1, &mat.emission.x);
			glProgramUniform2fv(pbrShader->ID(), 916, 1, scale);
			glProgramUniform1f(pbrShader->ID(), 928, mat.alphaCutoff);

			// standard model
			glProgramUniform1f(pbrShader->ID(), 917, mat.metallic);
			glProgramUniform1f(pbrShader->ID(), 918, mat.specTrans);
			glProgramUniform1f(pbrShader->ID(), 919, mat.anisotropic);
			glProgramUniform3fv(pbrShader->ID(), 920, 1, anisotropy);

			// refraction model
			glProgramUniform1f(pbrShader->ID(), 921, mat.specTrans);
			glProgramUniform1f(pbrShader->ID(), 922, mat.ior);
			glProgramUniform1f(pbrShader->ID(), 923, mat.mediumDensity);
			glProgramUniform3fv(pbrShader->ID(), 924, 1, &mat.mediumColor.x);

			// additive clear coat layer
			glProgramUniform1f(pbrShader->ID(), 929, mat.clearcoat);
			glProgramUniform1f(pbrShader->ID(), 930, mat.clearcoatGloss);

			if (mat.baseColorTexId != -1) {
				glBindTextureUnit(20, scene->textures[mat.baseColorTexId]->id);
				pbrShader->SetUniform(900U, true);
			}
			else {
				pbrShader->SetUniform(900U, false);
			}

			if (mat.normalmapTexID != -1) {
				glBindTextureUnit(21, scene->textures[mat.normalmapTexID]->id);
				pbrShader->SetUniform(901U, true);
			}
			else {
				pbrShader->SetUniform(901U, false);
			}

			if (mat.metallicRoughnessTexID != -1) {
				glBindTextureUnit(23, scene->textures[mat.metallicRoughnessTexID]->id);
				pbrShader->SetUniform(903U, true);
			}
			else {
				pbrShader->SetUniform(903U, false);
			}
			if (mat.emissionmapTexID != -1) {
				glBindTextureUnit(25, scene->textures[mat.emissionmapTexID]->id);
				pbrShader->SetUniform(905U, true);
			}
			else {
				pbrShader->SetUniform(905U, false);
			}
			glProgramUniformMatrix4fv(pbrShader->ID(), 1000, 1, GL_FALSE, &meshinstace.transform.data[0][0]);
			mesh->Draw();
		}
		pbrShader->Unbind();
		rasterMsaaFBO->Unbind();

		rasterFBO->Clear();
		asset::FBO::CopyColor(*rasterMsaaFBO, 0, *rasterFBO, 0);

	}

	void Renderer::Render()
	{
		if (raster) {
			RenderPBR();
			return;
		}

		if (!scene->dirty && scene->renderOptions.maxSpp != -1 && sampleCounter >= scene->renderOptions.maxSpp)
			return;

		glActiveTexture(GL_TEXTURE0);

		if (scene->dirty)
		{
			pathTraceFBOLowRes->Bind();
			glViewport(0, 0, windowSize.x * pixelRatio, windowSize.y * pixelRatio);
			quad->Draw(pathTraceShaderLowRes.get());
			pathTraceFBOLowRes->Unbind();

			scene->instancesModified = false;
			scene->dirty = false;
			scene->envMapModified = false;
		}
		else
		{


			pathTracefbo->Bind();
			glViewport(0, 0, tileWidth, tileHeight);
			glBindTexture(GL_TEXTURE_2D, accumTexture);
			quad->Draw(pathTraceShader.get());
			pathTracefbo->Unbind();

			glNamedFramebufferReadBuffer(pathTracefbo->ID(), GL_COLOR_ATTACHMENT0);
			glNamedFramebufferDrawBuffer(accumFBO->ID(), GL_COLOR_ATTACHMENT0);
			glBlitNamedFramebuffer(pathTracefbo->ID(), accumFBO->ID(), 0, 0, tileWidth, tileHeight, tileWidth * tile.x, tileHeight * tile.y,
				tileWidth * tile.x + tileWidth, tileHeight * tile.y + tileHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);


			outputFBO->Bind();
			outputFBO->SetColorTexture(0, tileOutputTexture[currentBuffer]);
			glViewport(0, 0, renderSize.x, renderSize.y);
			glBindTexture(GL_TEXTURE_2D, accumTexture);
			quad->Draw(tonemapShader.get());
			outputFBO->Unbind();
		}
	}
	void Renderer::PresentPBR() {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rasterFBO->GetColorTexture(0).ID());
		quad->Draw(outputShader.get());

	}

	void Renderer::SaveFrame()
	{

		unsigned char* data = nullptr;
		//getoup
		int w, h;
		GetOutputBuffer(&data, w, h);
		std::string filename = "./img_" + std::to_string(GetSampleCount()) + ".png";
		stbi_flip_vertically_on_write(true);
		stbi_write_png(filename.c_str(), w, h, 4, data, w * 4);
		printf("saved Frame: %s\n", filename.c_str());

		float x[2] = { std::min(screenX[0],screenX[1]),std::max(screenX[0],screenX[1]) };
		float y[2] = { std::min(screenY[0],screenY[1]),std::max(screenY[0],screenY[1]) };
		int sx = std::min(w * x[0], w * 1.0f);
		int sy = std::min(h * y[0], h * 1.0f);
		int ex = std::min(w * x[1], w * 1.0f);
		int ey = std::min(h * y[1], h * 1.0f);
		unsigned char* img = new unsigned char[(ex - sx + 1) * (ey - sy + 1) * 4];
		int j = 0;
		for (int i = sy; i <= ey; i++) {
			memcpy(img + (j) * (ex - sx + 1) * 4, data + (i)*w * 4 + sx * 4, (ex - sx + 1) * 4);
			j++;
		}
		filename = "./screen_" + std::to_string(GetSampleCount()) + ".png";
		stbi_write_png(filename.c_str(), (ex - sx + 1), (ey - sy + 1), 4, img, (ex - sx + 1) * 4);

	}

	void Renderer::SaveScene()
	{

	}

	void Renderer::Present()
	{
		if (raster) {
			PresentPBR();
			return;

		}
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glEnable(GL_BLEND);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, renderSize.x, renderSize.y);
		glActiveTexture(GL_TEXTURE0);

		if (scene->dirty || sampleCounter == 1)
		{
			glBindTexture(GL_TEXTURE_2D, pathTraceTextureLowRes);
			quad->Draw(tonemapShader.get());
		}
		else
		{
			if (scene->renderOptions.enableDenoiser && denoised)
				glBindTexture(GL_TEXTURE_2D, denoisedTexture);
			else
				glBindTexture(GL_TEXTURE_2D, tileOutputTexture[1 - currentBuffer]);

			quad->Draw(outputShader.get());
		}

		if (space_down) {
			float x[2] = { std::min(screenX[0],screenX[1]),std::max(screenX[0],screenX[1]) };
			float y[2] = { std::min(screenY[0],screenY[1]),std::max(screenY[0],screenY[1]) };
			lineShader->Bind();
			glProgramUniform2fv(lineShader->ID(), 0, 1, x);
			glProgramUniform2fv(lineShader->ID(), 1, 1, y);
			lineShader->Unbind();

			quad->Draw(lineShader.get());
		}

	}

	float Renderer::GetProgress()
	{
		int maxSpp = scene->renderOptions.maxSpp;
		return maxSpp <= 0 ? 0.0f : sampleCounter * 100.0f / maxSpp;
	}

	void Renderer::GetOutputBuffer(unsigned char** data, int& w, int& h)
	{
		w = renderSize.x;
		h = renderSize.y;

		*data = new unsigned char[w * h * 4];

		glActiveTexture(GL_TEXTURE0);

		if (scene->renderOptions.enableDenoiser && denoised)
			glBindTexture(GL_TEXTURE_2D, denoisedTexture);
		else
			glBindTexture(GL_TEXTURE_2D, tileOutputTexture[1 - currentBuffer]);

		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, *data);
	}

	int Renderer::GetSampleCount()
	{
		return sampleCounter;
	}

	void Renderer::Update(float secondsElapsed)
	{


		if (!scene->dirty && scene->renderOptions.maxSpp != -1 && sampleCounter >= scene->renderOptions.maxSpp)
			return;
		//更新场景
		if (scene->instancesModified)
		{
			// Transform
			glBindTexture(GL_TEXTURE_2D, transformsTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (sizeof(Mat4) / sizeof(Vec4)) * scene->transforms.size(), 1, 0, GL_RGBA, GL_FLOAT, &scene->transforms[0]);

			// Material
			glBindTexture(GL_TEXTURE_2D, materialsTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (sizeof(Material) / sizeof(Vec4)) * scene->materials.size(), 1, 0, GL_RGBA, GL_FLOAT, &scene->materials[0]);

			// BVH
			int index = scene->bvhTranslator.topLevelIndex;
			int offset = sizeof(RadeonRays::BvhTranslator::Node) * index;
			int size = sizeof(RadeonRays::BvhTranslator::Node) * (scene->bvhTranslator.nodes.size() - index);
			glBindBuffer(GL_TEXTURE_BUFFER, BVHBuffer);
			glBufferSubData(GL_TEXTURE_BUFFER, offset, size, &scene->bvhTranslator.nodes[index]);
		}

		// 更新环境贴图
		if (scene->envMapModified)
		{

			if (scene->envMap != nullptr)
			{
				glBindTexture(GL_TEXTURE_2D, envMapTex);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, scene->envMap->width, scene->envMap->height, 0, GL_RGB, GL_FLOAT, scene->envMap->img);

				glBindTexture(GL_TEXTURE_2D, envMapCDFTex);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, scene->envMap->width, scene->envMap->height, 0, GL_RED, GL_FLOAT, scene->envMap->cdf);

				GLuint shaderObject;
				pathTraceShader->Bind();
				shaderObject = pathTraceShader->ID();
				glUniform2f(glGetUniformLocation(shaderObject, "envMapRes"), (float)scene->envMap->width, (float)scene->envMap->height);
				glUniform1f(glGetUniformLocation(shaderObject, "envMapTotalSum"), scene->envMap->totalSum);
				pathTraceShader->Unbind();

				pathTraceShaderLowRes->Bind();
				shaderObject = pathTraceShaderLowRes->ID();
				glUniform2f(glGetUniformLocation(shaderObject, "envMapRes"), (float)scene->envMap->width, (float)scene->envMap->height);
				glUniform1f(glGetUniformLocation(shaderObject, "envMapTotalSum"), scene->envMap->totalSum);
				pathTraceShaderLowRes->Unbind();
			}
		}

		// 降噪处理
		if (scene->renderOptions.enableDenoiser && sampleCounter > 1)
		{
			//每间隔降噪
			if (!denoised || (frameCounter % (scene->renderOptions.denoiserFrameCnt * (numTiles.x * numTiles.y)) == 0))
			{

				glBindTexture(GL_TEXTURE_2D, tileOutputTexture[1 - currentBuffer]);
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, denoiserInputFramePtr);
				memcpy(frameOutputPtr, denoiserInputFramePtr, renderSize.x * renderSize.y * 16);
				oidn::DeviceRef device = oidn::newDevice();
				device.commit();


				oidn::FilterRef filter = device.newFilter("RT"); // generic ray tracing filter
				filter.setImage("color", denoiserInputFramePtr, oidn::Format::Float3, renderSize.x, renderSize.y, 0, 16, 0);
				filter.setImage("output", frameOutputPtr, oidn::Format::Float3, renderSize.x, renderSize.y, 0, 16, 0);
				filter.set("hdr", false);
				filter.commit();

				filter.execute();


				const char* errorMessage;
				if (device.getError(errorMessage) != oidn::Error::None)
					std::cout << "Error: " << errorMessage << std::endl;


				glBindTexture(GL_TEXTURE_2D, denoisedTexture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, renderSize.x, renderSize.y, 0, GL_RGBA, GL_FLOAT, frameOutputPtr);

				denoised = true;
			}
		}
		else
			denoised = false;

		// 重新开始计算，重置参数
		if (scene->dirty)
		{
			tile.x = -1;
			tile.y = numTiles.y - 1;
			sampleCounter = 1;
			denoised = false;
			frameCounter = 1;
			accumFBO->Clear(0);
		}
		else //走向下一个瓦片绘制
		{
			frameCounter++;
			tile.x++;
			if (tile.x >= numTiles.x)
			{
				tile.x = 0;
				tile.y--;
				if (tile.y < 0)
				{
					// 算完一帧，切换当前绘制缓冲，样本数加一
					tile.x = 0;
					tile.y = numTiles.y - 1;
					sampleCounter++;
					currentBuffer = 1 - currentBuffer;
				}
			}
		}

		// 更新参数

		GLuint shaderObject;
		pathTraceShader->Bind();
		shaderObject = pathTraceShader->ID();
		glUniform3f(glGetUniformLocation(shaderObject, "camera.position"), scene->camera->position.x, scene->camera->position.y, scene->camera->position.z);
		glUniform3f(glGetUniformLocation(shaderObject, "camera.right"), scene->camera->right.x, scene->camera->right.y, scene->camera->right.z);
		glUniform3f(glGetUniformLocation(shaderObject, "camera.up"), scene->camera->up.x, scene->camera->up.y, scene->camera->up.z);
		glUniform3f(glGetUniformLocation(shaderObject, "camera.forward"), scene->camera->forward.x, scene->camera->forward.y, scene->camera->forward.z);
		glUniform1f(glGetUniformLocation(shaderObject, "camera.fov"), scene->camera->fov);
		glUniform1f(glGetUniformLocation(shaderObject, "camera.focalDist"), scene->camera->focalDist);
		glUniform1f(glGetUniformLocation(shaderObject, "camera.aperture"), scene->camera->aperture);
		glUniform1i(glGetUniformLocation(shaderObject, "enableEnvMap"), scene->envMap == nullptr ? false : scene->renderOptions.enableEnvMap);
		glUniform1f(glGetUniformLocation(shaderObject, "envMapIntensity"), scene->renderOptions.envMapIntensity);
		glUniform1f(glGetUniformLocation(shaderObject, "envMapRot"), scene->renderOptions.envMapRot / 360.0f);
		glUniform1i(glGetUniformLocation(shaderObject, "maxDepth"), scene->renderOptions.maxDepth);
		glUniform2f(glGetUniformLocation(shaderObject, "tileOffset"), (float)tile.x * invNumTiles.x, (float)tile.y * invNumTiles.y);
		glUniform3f(glGetUniformLocation(shaderObject, "uniformLightCol"), scene->renderOptions.uniformLightCol.x, scene->renderOptions.uniformLightCol.y, scene->renderOptions.uniformLightCol.z);
		glUniform1f(glGetUniformLocation(shaderObject, "roughnessMollificationAmt"), scene->renderOptions.roughnessMollificationAmt);
		glUniform1i(glGetUniformLocation(shaderObject, "frameNum"), frameCounter);
		pathTraceShader->Unbind();

		pathTraceShaderLowRes->Bind();
		shaderObject = pathTraceShaderLowRes->ID();
		glUniform3f(glGetUniformLocation(shaderObject, "camera.position"), scene->camera->position.x, scene->camera->position.y, scene->camera->position.z);
		glUniform3f(glGetUniformLocation(shaderObject, "camera.right"), scene->camera->right.x, scene->camera->right.y, scene->camera->right.z);
		glUniform3f(glGetUniformLocation(shaderObject, "camera.up"), scene->camera->up.x, scene->camera->up.y, scene->camera->up.z);
		glUniform3f(glGetUniformLocation(shaderObject, "camera.forward"), scene->camera->forward.x, scene->camera->forward.y, scene->camera->forward.z);
		glUniform1f(glGetUniformLocation(shaderObject, "camera.fov"), scene->camera->fov);
		glUniform1f(glGetUniformLocation(shaderObject, "camera.focalDist"), scene->camera->focalDist);
		glUniform1f(glGetUniformLocation(shaderObject, "camera.aperture"), scene->camera->aperture);
		glUniform1i(glGetUniformLocation(shaderObject, "enableEnvMap"), scene->envMap == nullptr ? false : scene->renderOptions.enableEnvMap);
		glUniform1f(glGetUniformLocation(shaderObject, "envMapIntensity"), scene->renderOptions.envMapIntensity);
		glUniform1f(glGetUniformLocation(shaderObject, "envMapRot"), scene->renderOptions.envMapRot / 360.0f);
		glUniform1i(glGetUniformLocation(shaderObject, "maxDepth"), scene->dirty ? 2 : scene->renderOptions.maxDepth);
		glUniform3f(glGetUniformLocation(shaderObject, "camera.position"), scene->camera->position.x, scene->camera->position.y, scene->camera->position.z);
		glUniform3f(glGetUniformLocation(shaderObject, "uniformLightCol"), scene->renderOptions.uniformLightCol.x, scene->renderOptions.uniformLightCol.y, scene->renderOptions.uniformLightCol.z);
		glUniform1f(glGetUniformLocation(shaderObject, "roughnessMollificationAmt"), scene->renderOptions.roughnessMollificationAmt);
		pathTraceShaderLowRes->Unbind();

		tonemapShader->Bind();
		shaderObject = tonemapShader->ID();
		glUniform1f(glGetUniformLocation(shaderObject, "invSampleCounter"), 1.0f / (sampleCounter));
		glUniform1i(glGetUniformLocation(shaderObject, "enableTonemap"), scene->renderOptions.enableTonemap);
		glUniform1i(glGetUniformLocation(shaderObject, "enableAces"), scene->renderOptions.enableAces);
		glUniform1i(glGetUniformLocation(shaderObject, "simpleAcesFit"), scene->renderOptions.simpleAcesFit);
		glUniform3f(glGetUniformLocation(shaderObject, "backgroundCol"), scene->renderOptions.backgroundCol.x, scene->renderOptions.backgroundCol.y, scene->renderOptions.backgroundCol.z);
		tonemapShader->Unbind();
	}


}