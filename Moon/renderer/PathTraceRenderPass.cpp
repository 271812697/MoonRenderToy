#include <tracy/Tracy.hpp>
#include <Core/ECS/Components/CPostProcessStack.h>
#include "renderer/PathTraceRenderPass.h"
#include <Core/Global/ServiceLocator.h>
#include <Core/Rendering/FramebufferUtil.h>
#include <Core/Rendering/SceneRenderer.h>
#include <Core/ResourceManagement/ShaderManager.h>
#include <Core/SceneSystem/BvhService.h>
#include "core/log.h"
#include "Rendering/Resources/Texture.h"
#include "renderer/SceneView.h"
#include <Rendering/Core/CompositeRenderer.h>
#include <Rendering/HAL/Profiling.h>
#include <Rendering/Settings/EPixelDataFormat.h>
#include <Rendering/Settings/ETextureType.h>
#include "Settings/DebugSetting.h"
#include <stb_Image/stb_image.h>
#include <fstream>
namespace Editor::Rendering {
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
	static float Luminance(float r, float g, float b)
	{
		return 0.212671f * r + 0.715160f * g + 0.072169f * b;
	}
	class EnvironmentMap
	{
	public:
		EnvironmentMap() : width(0), height(0), img(nullptr), cdf(nullptr) , totalSum(0.0f){
		
		}
		~EnvironmentMap() {
			stbi_image_free(img);
			delete[] cdf;
		}

		bool LoadMap(const std::string& filename) {
			stbi_set_flip_vertically_on_load(false);
			img = stbi_loadf(filename.c_str(), &width, &height, NULL, 3);
			if (img == nullptr)
				return false;
			BuildCDF();
			return true;
		}
		void BuildCDF() {

			// Gather weights for CDF
			float* weights = new float[width * height];
			for (int v = 0; v < height; v++)
			{
				for (int u = 0; u < width; u++)
				{
					int imgIdx = v * width * 3 + u * 3;
					weights[u + v * width] = Luminance(img[imgIdx + 0], img[imgIdx + 1], img[imgIdx + 2]);
				}
			}
			// Build CDF
			cdf = new float[width * height];
			cdf[0] = weights[0];
			for (int i = 1; i < width * height; i++)
				cdf[i] = cdf[i - 1] + weights[i];

			totalSum = cdf[width * height - 1];
			delete[] weights;
		}

		Maths::FVector3 Sample(float u, float v) {
			float w;
			float alpha;
			alpha = modf(u * (width - 1), &w);
			float h;
			float betha;
			betha = modf(v * (height - 1), &h);
			return (1 - betha) * ((1 - alpha) * this->Color(h, w) + alpha * this->Color(h, w + 1)) + betha * ((1 - alpha) * this->Color(h + 1, w) + alpha * this->Color(h + 1, w + 1));
		}
		Maths::FVector3 Color(int x, int y) {
			int imgIdx = x * width * 3 + y * 3;
			return Maths::FVector3(img[imgIdx + 0], img[imgIdx + 1], img[imgIdx + 2]);
		}
		//importance sample
		Maths::FVector3 Sample(float u, Maths::FVector2* out) {
			float value = totalSum * u;
			int left = 0;
			int right = width * height - 1;
			int mid = 0;
			while (left < right) {
				mid = (left + right) / 2;
				if (cdf[mid] < value) {
					left = mid + 1;
				}
				else {
					right = mid;
				}
			}
			mid = (left + right) / 2;
			int row = mid / width;
			int col = mid % width;
			*out = { 1.0f * row / height ,1.0f * col / width };
			//clamp?
			return Color(row, col);
		}
		int width;
		int height;
		//环境的总亮度
		float totalSum;
		//纹理数据(一个像素3字节)
		float* img;
		//纹理亮度累加和序列
		float* cdf;
	};
	PathTraceRenderPass::PathTraceRenderPass(::Rendering::Core::CompositeRenderer& p_renderer): 
		::Rendering::Core::ARenderPass(p_renderer),
		pathTracefbo("pathTrace"),
		pathTraceFBOLowRes("pathTraceLowRes"),
		accumFBO("accumFBO"),
		outputFBO{
		::Rendering::HAL::Framebuffer{"BloomSamplingBuffer0"},
		::Rendering::HAL::Framebuffer{"BloomSamplingBuffer1"} }
	{
		envMap = new EnvironmentMap();
		envMap->LoadMap(PATH_TRACE_HDR_PATH"/outdoor.hdr");
		pixelRatio = 0.25f;

		auto fBColorDesc = ::Rendering::Settings::TextureDesc{
			.width = 1, // Unknown size at this point
			.height = 1,
			.minFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR,
			.magFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR,
			.horizontalWrap = ::Rendering::Settings::ETextureWrapMode::CLAMP_TO_EDGE,
			.verticalWrap = ::Rendering::Settings::ETextureWrapMode::CLAMP_TO_EDGE,
			.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA32F,
			.useMipMaps = false,
			.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
				.format = ::Rendering::Settings::EFormat::RGBA,
				.type = ::Rendering::Settings::EPixelDataType::FLOAT
			}
		};

		::Core::Rendering::FramebufferUtil::SetupFramebuffer(pathTracefbo, fBColorDesc, false, false,false);
		fBColorDesc.minFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST;
		fBColorDesc.magFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST;
		::Core::Rendering::FramebufferUtil::SetupFramebuffer(pathTraceFBOLowRes, fBColorDesc, false, false, false);
		fBColorDesc.minFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR;
		fBColorDesc.magFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR;
		::Core::Rendering::FramebufferUtil::SetupFramebuffer(accumFBO, fBColorDesc, false, false, false);
		::Core::Rendering::FramebufferUtil::SetupFramebuffer(outputFBO[0], fBColorDesc, false, false, false);
		::Core::Rendering::FramebufferUtil::SetupFramebuffer(outputFBO[1], fBColorDesc, false, false, false);
	     

		MOON::DebugSettings::instance().addCallBack("PathTrace", "Default", [this](MOON::NodeBase* self) {
			bool value = self->getData<bool>();
			this->SetEnabled(value);
			});
		MOON::DebugSettings::instance().addCallBack("reBuildBvh", "Default", [this](MOON::NodeBase* self) {
			bool value = self->getData<bool>();	
			if (value) {
				::Core::SceneSystem::Scene* scene = GetService(Editor::Core::Context).sceneManager.GetCurrentScene();
				scene->BuildSceneBvh();
			}
			});
		//we need to init shaders
		UpdateShaders();
	}
	PathTraceRenderPass::~PathTraceRenderPass()
	{
		DestoryResource();
	}
	void PathTraceRenderPass::DestoryResource()
	{
		delete envMap;
		if(BVHTex)
		delete BVHTex;
		if (vertexIndicesTex)
		delete vertexIndicesTex;
		if (verticesTex)
		delete verticesTex;
		if (normalsTex)
		delete normalsTex;
		if (materialsTex)
		delete materialsTex;
		if (transformsTex)
		delete transformsTex;
		if (lightsTex)
		delete lightsTex;
		if (textureMapsArrayTex)
			delete textureMapsArrayTex;
		if (envMapTex) {
			delete envMapTex;
		}
		if (envMapCDFTex) {
			delete envMapCDFTex;
		}
		//??
		delete[] denoiserInputFramePtr;
		delete[] frameOutputPtr;
	}
	void PathTraceRenderPass::UpdateGPUDataBuffers() {
		auto& view = GetService(Editor::Panels::SceneView);;
		auto bvhService = view.GetScene()->GetBvhService();
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		// BVH Texture
		::Rendering::Settings::TextureDesc desc;
		desc.isTextureBuffer = true;
		desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGB32F;
		desc.buffetLen = bvhService->nodes.size() * sizeof(::Core::SceneSystem::BvhService::Node);
		desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
			.data = bvhService->nodes.data()
		};

		if (BVHTex == nullptr) {
			BVHTex = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_BUFFER);
		}
		BVHTex->Allocate(desc);
		

		// Vertex Indices Texture
		desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGB32I;
		desc.buffetLen = bvhService->vertIndices.size() * sizeof(::Core::SceneSystem::Indices);
		desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
			.data = bvhService->vertIndices.data()
		};
		if (vertexIndicesTex == nullptr) {
			vertexIndicesTex = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_BUFFER);
		}
		vertexIndicesTex->Allocate(desc);

		// Vertices Texture
		desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA32F;
		desc.buffetLen = bvhService->verticesUVX.size() * sizeof(Maths::FVector4);
		desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
			.data = bvhService->verticesUVX.data()
		};
		if (verticesTex== nullptr) {
			verticesTex= new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_BUFFER);
		}
		verticesTex->Allocate(desc);

		// Normals Texture
		desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA32F;
		desc.buffetLen = bvhService->normalsUVY.size() * sizeof(Maths::FVector4);
		desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
			.data = bvhService->normalsUVY.data()
		};
		if (normalsTex== nullptr) {
			normalsTex = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_BUFFER);
		}
		normalsTex->Allocate(desc);

		//Material Texture
		desc.isTextureBuffer = false;
		desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA32F;
		desc.width = bvhService->materials.size() * (sizeof(::Core::SceneSystem::Material) / sizeof(Maths::FVector4));
		desc.height = 1;
		desc.minFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST;
		desc.magFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST;
		desc.buffetLen = bvhService->materials.size() * sizeof(::Core::SceneSystem::Material);
		desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
			.format = ::Rendering::Settings::EFormat::RGBA,
			.type = ::Rendering::Settings::EPixelDataType::FLOAT,
			.data = bvhService->materials.data()
		};
		if (materialsTex== nullptr) {
			materialsTex = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_2D);
		}
		materialsTex->Allocate(desc);

		// Transform Texture
		desc.isTextureBuffer = false;
		desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA32F;
		desc.width = bvhService->transforms.size() * (sizeof(Maths::FMatrix4) / sizeof(Maths::FVector4));
		desc.height = 1;
		desc.minFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST;
		desc.magFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST;
		desc.buffetLen = bvhService->transforms.size() * sizeof(Maths::FMatrix4);
		desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
			.format = ::Rendering::Settings::EFormat::RGBA,
			.type = ::Rendering::Settings::EPixelDataType::FLOAT,
			.data = bvhService->transforms.data()

		};
		if (transformsTex== nullptr) {
			transformsTex = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_2D);
		}
		transformsTex->Allocate(desc);

		// Lights Texture
		if (!bvhService->lights.empty()) {
			desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGB32F;
			desc.width = bvhService->lights.size() * (sizeof(::Core::SceneSystem::Light) / sizeof(Maths::FVector3));
			desc.height = 1;
			desc.minFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST;
			desc.magFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST;
			desc.buffetLen = bvhService->lights.size() * sizeof(::Core::SceneSystem::Light);
			desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
				.format = ::Rendering::Settings::EFormat::RGB,
				.type = ::Rendering::Settings::EPixelDataType::FLOAT,
				.data = bvhService->lights.data()

			};
			if (lightsTex== nullptr) {
				lightsTex = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_2D);
			}
			lightsTex->Allocate(desc);
		}

		// Enviroment Texture
		desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGB32F;
		desc.width = envMap->width;
		desc.height = envMap->height;
		desc.minFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR;
		desc.magFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR;
		desc.buffetLen = envMap->width * envMap->height * sizeof(Maths::FVector3);
		desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
			.format = ::Rendering::Settings::EFormat::RGB,
			.type = ::Rendering::Settings::EPixelDataType::FLOAT,
			.data = envMap->img
		};
		if (envMapTex== nullptr) {
			envMapTex= new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_2D);
		}
		envMapTex->Allocate(desc);

		desc.internalFormat = ::Rendering::Settings::EInternalFormat::R32F;
		desc.minFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST;
		desc.magFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST;
		desc.buffetLen = envMap->width * envMap->height * sizeof(float);
		desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
			.format = ::Rendering::Settings::EFormat::RED,
			.type = ::Rendering::Settings::EPixelDataType::FLOAT,
			.data = envMap->cdf
		};
		if (envMapCDFTex== nullptr) {
			envMapCDFTex = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_2D);
		}
		envMapCDFTex->Allocate(desc);

		// Scene Textures
		if (!bvhService->textures.empty()) {
			desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA8;
			desc.minFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR;
			desc.magFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR;
			desc.buffetLen = bvhService->textureMapsArray.size();
			desc.width = bvhService->renderOptions.texArrayWidth;
			desc.height = bvhService->renderOptions.texArrayHeight;
			desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
				.format = ::Rendering::Settings::EFormat::BGRA,
				.type = ::Rendering::Settings::EPixelDataType::UNSIGNED_BYTE,
				.data = bvhService->textureMapsArray.data(),
				.arrayLayers = static_cast<int>(bvhService->textures.size())
			};
			if (textureMapsArrayTex== nullptr) {
				textureMapsArrayTex = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_2DARRAY);
			}
			textureMapsArrayTex->Allocate(desc);
		}
	}
	void PathTraceRenderPass::UpdateShaders() {

		static std::string pathTraceShaderSrcObj = loadShaderSource(PROJECT_ENGINE_PATH"/Shaders/PathTrace/PathTrace.ovfx");
		static std::string pathTraceShaderLowResSrcObj = loadShaderSource(PROJECT_ENGINE_PATH"/Shaders/PathTrace/PathTraceLowRes.ovfx");
		static std::string outputShaderSrcObj = loadShaderSource(PROJECT_ENGINE_PATH"/Shaders/PathTrace/Output.ovfx");
		static std::string tonemapShaderSrcObj = loadShaderSource(PROJECT_ENGINE_PATH"/Shaders/PathTrace/ToneMap.ovfx");

		static std::unordered_map<std::string, std::unique_ptr<::Rendering::Resources::Shader>>pathTraceShaderMap;
		static std::unordered_map<std::string, std::unique_ptr<::Rendering::Resources::Shader>>pathTraceShaderLowResMap;
		static std::unordered_map<std::string, std::unique_ptr<::Rendering::Resources::Shader>>outputShaderMap;
		static std::unordered_map<std::string, std::unique_ptr<::Rendering::Resources::Shader>>tonemapShaderMap;

		std::string tempPathTraceShaderSrcObj = pathTraceShaderSrcObj;
		std::string tempPathTraceShaderLowResSrcObj = pathTraceShaderLowResSrcObj;
		std::string tempOutputShaderSrcObj = outputShaderSrcObj;
		std::string tempTonemapShaderSrcObj = tonemapShaderSrcObj;


		auto& view = GetService(Editor::Panels::SceneView);;
		auto bvhService = view.GetScene()->GetBvhService();
		//分析renderOptions添加向源码中预定义宏
		std::string pathtraceDefines = "";
		std::string tonemapDefines = "";
		std::string outputDefines = "";

		if (bvhService->renderOptions.enableEnvMap && envMap != nullptr)
			pathtraceDefines += "#define OPT_ENVMAP\n";

		if (!bvhService->lights.empty()) {
			pathtraceDefines += "#define OPT_LIGHTS\n";
			bvhService->renderOptions.optLight = true;
		}


		if (bvhService->renderOptions.enableRR)
		{
			pathtraceDefines += "#define OPT_RR\n";
			pathtraceDefines += "#define OPT_RR_DEPTH " + std::to_string(bvhService->renderOptions.RRDepth) + "\n";
		}

		if (bvhService->renderOptions.enableUniformLight)
			pathtraceDefines += "#define OPT_UNIFORM_LIGHT\n";

		if (bvhService->renderOptions.openglNormalMap)
			pathtraceDefines += "#define OPT_OPENGL_NORMALMAP\n";

		if (bvhService->renderOptions.hideEmitters)
			pathtraceDefines += "#define OPT_HIDE_EMITTERS\n";

		if (bvhService->renderOptions.enableBackground)
		{
			pathtraceDefines += "#define OPT_BACKGROUND\n";
			tonemapDefines += "#define OPT_BACKGROUND\n";
		}

		if (bvhService->renderOptions.transparentBackground)
		{
			pathtraceDefines += "#define OPT_TRANSPARENT_BACKGROUND\n";
			tonemapDefines += "#define OPT_TRANSPARENT_BACKGROUND\n";
		}

		for (int i = 0; i < bvhService->materials.size(); i++)
		{
			if ((int)bvhService->materials[i].alphaMode != ::Core::SceneSystem::AlphaMode::Opaque)
			{
				pathtraceDefines += "#define OPT_ALPHA_TEST\n";
				bvhService->renderOptions.optAlphaTest = true;
				break;
			}
		}

		if (bvhService->renderOptions.enableRoughnessMollification)
			pathtraceDefines += "#define OPT_ROUGHNESS_MOLLIFICATION\n";

		for (int i = 0; i < bvhService->materials.size(); i++)
		{
			if ((int)bvhService->materials[i].mediumType != ::Core::SceneSystem::MediumType::None)
			{
				pathtraceDefines += "#define OPT_MEDIUM\n";
				bvhService->renderOptions.optMedium = true;
				break;
			}
		}

		if (bvhService->renderOptions.enableVolumeMIS)
			pathtraceDefines += "#define OPT_VOL_MIS\n";


		if (pathtraceDefines.size() > 0)
		{
			size_t idx = tempPathTraceShaderSrcObj.find("#shader fragment");
			idx = tempPathTraceShaderSrcObj.find("#version", idx);
			if (idx != -1)
				idx = tempPathTraceShaderSrcObj.find("\n", idx);
			else
				idx = 0;
			tempPathTraceShaderSrcObj.insert(idx + 1, pathtraceDefines);

			idx = tempPathTraceShaderLowResSrcObj.find("#shader fragment");
			idx = tempPathTraceShaderLowResSrcObj.find("#version", idx);
			if (idx != -1)
				idx = tempPathTraceShaderLowResSrcObj.find("\n", idx);
			else
				idx = 0;
			tempPathTraceShaderLowResSrcObj.insert(idx + 1, pathtraceDefines);
		}

		if (tonemapDefines.size() > 0)
		{
			size_t idx = tempTonemapShaderSrcObj.find("#shader fragment");
			idx = tempTonemapShaderSrcObj.find("#version", idx);
			if (idx != -1)
				idx = tempTonemapShaderSrcObj.find("\n", idx);
			else
				idx = 0;
			tempTonemapShaderSrcObj.insert(idx + 1, tonemapDefines);
		}

		if (pathTraceShaderMap.find(pathtraceDefines) != pathTraceShaderMap.end()) {
			pathTraceShader.SetShader(pathTraceShaderMap[pathtraceDefines].get());
		}
		else {
			auto shader = ::Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager.CreateFromSource(tempPathTraceShaderSrcObj);
			pathTraceShaderMap[pathtraceDefines] = std::unique_ptr<::Rendering::Resources::Shader>(shader);
			pathTraceShader.SetShader(pathTraceShaderMap[pathtraceDefines].get());
		}

		if (pathTraceShaderLowResMap.find(pathtraceDefines) != pathTraceShaderLowResMap.end()) {
			pathTraceShaderLowRes.SetShader(pathTraceShaderLowResMap[pathtraceDefines].get());
		}
		else {
			auto shader = ::Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager.CreateFromSource(tempPathTraceShaderLowResSrcObj);
			pathTraceShaderLowResMap[pathtraceDefines] = std::unique_ptr<::Rendering::Resources::Shader>(shader);
			pathTraceShaderLowRes.SetShader(pathTraceShaderLowResMap[pathtraceDefines].get());
		}

		if (tonemapShaderMap.find(tonemapDefines) != tonemapShaderMap.end()) {
			tonemapShader.SetShader(tonemapShaderMap[tonemapDefines].get());
		}
		else {
			auto shader = ::Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager.CreateFromSource(tempTonemapShaderSrcObj);
			tonemapShaderMap[tonemapDefines] = std::unique_ptr<::Rendering::Resources::Shader>(shader);
			tonemapShader.SetShader(tonemapShaderMap[tonemapDefines].get());
		}

		if (outputShaderMap.find(outputDefines) != outputShaderMap.end()) {
			outputShader.SetShader(outputShaderMap[outputDefines].get());
		}
		else {
			auto shader = ::Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager.CreateFromSource(tempOutputShaderSrcObj);
			outputShaderMap[outputDefines] = std::unique_ptr<::Rendering::Resources::Shader>(shader);
			outputShader.SetShader(outputShaderMap[outputDefines].get());
		}
	}
	void PathTraceRenderPass::ResizeRenderer(int width, int height)
	{
		refreshFlag = true;
		UpdateFBOs();
	}
	void PathTraceRenderPass::UpdateFBOs() {
		//resume fbo
		sampleCounter = 1;
		currentBuffer = 0;
		frameCounter = 1;
		auto& view = GetService(Editor::Panels::SceneView);;
		auto bvhService = view.GetScene()->GetBvhService();
		auto [w, h] = view.GetSafeSize();
		renderSize = { static_cast<float>(w) ,static_cast<float>(h) };
		windowSize = renderSize;

		tileWidth = bvhService->renderOptions.tileWidth;
		tileHeight = bvhService->renderOptions.tileHeight;
		//单块瓦片所占据的uv区间长度
		invNumTiles.x = (float)tileWidth / renderSize.x;
		invNumTiles.y = (float)tileHeight / renderSize.y;

		numTiles.x = ceil((float)renderSize.x / tileWidth);
		numTiles.y = ceil((float)renderSize.y / tileHeight);

		tile.x = -1;
		tile.y = numTiles.y - 1;

		pathTracefbo.Resize(tileWidth, tileHeight);
		pathTraceFBOLowRes.Resize(static_cast<int>(windowSize.x * pixelRatio), static_cast<int>(windowSize.y * pixelRatio));
		accumFBO.Resize(static_cast<int>(renderSize.x), static_cast<int>(renderSize.y));
		outputFBO[0].Resize(static_cast<int>(renderSize.x), static_cast<int>(renderSize.y));
		outputFBO[1].Resize(static_cast<int>(renderSize.x), static_cast<int>(renderSize.y));
	
	}
	void PathTraceRenderPass::Draw(::Rendering::Data::PipelineState p_pso)
	{
		auto bvhService= GetService(Editor::Panels::SceneView).GetScene()->GetBvhService();
		if (bvhService->isDirty) {
			bvhService->isDirty = false;
			UpdateGPUDataBuffers();
		}
		Update();
		Render();
		Present();
	}
	void PathTraceRenderPass::Update() {
		auto& view = GetService(Editor::Panels::SceneView);;
		auto bvhService = view.GetScene()->GetBvhService();
		if (!refreshFlag && bvhService->renderOptions.maxSpp != -1 && sampleCounter >= bvhService->renderOptions.maxSpp)
			return;
		auto camera=view.GetCamera();
		//更新场景
		//if (scene->instancesModified)
		//{
		//	// Transform
		//	glBindTexture(GL_TEXTURE_2D, transformsTex);
		//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (sizeof(Mat4) / sizeof(Vec4)) * scene->transforms.size(), 1, 0, GL_RGBA, GL_FLOAT, &scene->transforms[0]);

		//	// Material
		//	glBindTexture(GL_TEXTURE_2D, materialsTex);
		//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (sizeof(Material) / sizeof(Vec4)) * scene->materials.size(), 1, 0, GL_RGBA, GL_FLOAT, &scene->materials[0]);

		//	// BVH
		//	int index = scene->bvhTranslator.topLevelIndex;
		//	int offset = sizeof(RadeonRays::BvhTranslator::Node) * index;
		//	int size = sizeof(RadeonRays::BvhTranslator::Node) * (scene->bvhTranslator.nodes.size() - index);
		//	glBindBuffer(GL_TEXTURE_BUFFER, BVHBuffer);
		//	glBufferSubData(GL_TEXTURE_BUFFER, offset, size, &scene->bvhTranslator.nodes[index]);
		//}

		// 更新环境贴图
		//if (scene->envMapModified)
		//{

		//	if (scene->envMap != nullptr)
		//	{
		//		glBindTexture(GL_TEXTURE_2D, envMapTex);
		//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, scene->envMap->width, scene->envMap->height, 0, GL_RGB, GL_FLOAT, scene->envMap->img);

		//		glBindTexture(GL_TEXTURE_2D, envMapCDFTex);
		//		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, scene->envMap->width, scene->envMap->height, 0, GL_RED, GL_FLOAT, scene->envMap->cdf);

		//		GLuint shaderObject;
		//		pathTraceShader->Bind();
		//		shaderObject = pathTraceShader->ID();
		//		glUniform2f(glGetUniformLocation(shaderObject, "envMapRes"), (float)scene->envMap->width, (float)scene->envMap->height);
		//		glUniform1f(glGetUniformLocation(shaderObject, "envMapTotalSum"), scene->envMap->totalSum);
		//		pathTraceShader->Unbind();

		//		pathTraceShaderLowRes->Bind();
		//		shaderObject = pathTraceShaderLowRes->ID();
		//		glUniform2f(glGetUniformLocation(shaderObject, "envMapRes"), (float)scene->envMap->width, (float)scene->envMap->height);
		//		glUniform1f(glGetUniformLocation(shaderObject, "envMapTotalSum"), scene->envMap->totalSum);
		//		pathTraceShaderLowRes->Unbind();
		//	}
		//}

		// 降噪处理
		//if (bvhService->renderOptions.enableDenoiser && sampleCounter > 1)
		//{
		//	//每间隔降噪
		//	if (!denoised || (frameCounter % (bvhService->renderOptions.denoiserFrameCnt * (numTiles.x * numTiles.y)) == 0))
		//	{
		//		glBindTexture(GL_TEXTURE_2D, tileOutputTexture[1 - currentBuffer]);
		//		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, denoiserInputFramePtr);
		//		memcpy(frameOutputPtr, denoiserInputFramePtr, renderSize.x * renderSize.y * 16);
		//		oidn::DeviceRef device = oidn::newDevice();
		//		device.commit();


		//		oidn::FilterRef filter = device.newFilter("RT"); // generic ray tracing filter
		//		filter.setImage("color", denoiserInputFramePtr, oidn::Format::Float3, renderSize.x, renderSize.y, 0, 16, 0);
		//		filter.setImage("output", frameOutputPtr, oidn::Format::Float3, renderSize.x, renderSize.y, 0, 16, 0);
		//		filter.set("hdr", false);
		//		filter.commit();
		//		filter.execute();
		//		const char* errorMessage;
		//		if (device.getError(errorMessage) != oidn::Error::None)
		//			std::cout << "Error: " << errorMessage << std::endl;
		//		glBindTexture(GL_TEXTURE_2D, denoisedTexture);
		//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, renderSize.x, renderSize.y, 0, GL_RGBA, GL_FLOAT, frameOutputPtr);

		//		denoised = true;
		//	}
		//}
		//else
		//	denoised = false;

		// 重新开始计算，重置参数
		if (refreshFlag)
		{
			tile.x = -1;
			tile.y = numTiles.y - 1;
			sampleCounter = 1;
			denoised = false;
			frameCounter = 1;
			accumFBO.Clear(::Rendering::Settings::EFramebufferAttachment::COLOR,0);
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
		auto cpos=camera->GetPosition();
		auto cwr=-camera->GetTransform().GetWorldRight();
		auto cwu=camera->GetTransform().GetWorldUp();
		auto cwf=camera->GetTransform().GetWorldForward();
		float fov=camera->GetFov()/180.0f*3.15157f;
		float focalDist=camera->GetNear();
		float aperture = 0.0f;

		
		//set shader unifoms
		pathTraceShader.SetProperty("resolution", Maths::FVector2(renderSize.x, renderSize.y));
		pathTraceShader.SetProperty("invNumTiles", Maths::FVector2(invNumTiles.x, invNumTiles.y));
		pathTraceShader.SetProperty("camera.position",cpos);
		pathTraceShader.SetProperty("camera.right", cwr);
		pathTraceShader.SetProperty("camera.up",cwu );
		pathTraceShader.SetProperty("camera.forward", cwf);
		pathTraceShader.SetProperty("camera.fov", fov);
		pathTraceShader.SetProperty("camera.focalDist", focalDist);
		pathTraceShader.SetProperty("camera.aperture", aperture);
		pathTraceShader.SetProperty("enableEnvMap",envMap == nullptr ? false : bvhService->renderOptions.enableEnvMap );
		pathTraceShader.SetProperty("envMapIntensity", bvhService->renderOptions.envMapIntensity);
		pathTraceShader.SetProperty("envMapRot", bvhService->renderOptions.envMapRot / 360.0f);
		pathTraceShader.SetProperty("maxDepth", bvhService->renderOptions.maxDepth);
		pathTraceShader.SetProperty("tileOffset", Maths::FVector2((float)tile.x * invNumTiles.x, (float)tile.y * invNumTiles.y));
		pathTraceShader.SetProperty("uniformLightCol", bvhService->renderOptions.uniformLightCol);
		pathTraceShader.SetProperty("roughnessMollificationAmt", bvhService->renderOptions.roughnessMollificationAmt);
		pathTraceShader.SetProperty("frameNum", frameCounter);

		pathTraceShaderLowRes.SetProperty("resolution", Maths::FVector2(renderSize.x, renderSize.y));
		pathTraceShaderLowRes.SetProperty("camera.position", cpos);
		pathTraceShaderLowRes.SetProperty("camera.right", cwr);
		pathTraceShaderLowRes.SetProperty("camera.up", cwu);
		pathTraceShaderLowRes.SetProperty("camera.forward", cwf);
		pathTraceShaderLowRes.SetProperty("camera.fov", fov);
		pathTraceShaderLowRes.SetProperty("camera.focalDist", focalDist);
		pathTraceShaderLowRes.SetProperty("camera.aperture", aperture);
		pathTraceShaderLowRes.SetProperty("enableEnvMap", envMap == nullptr ? false : bvhService->renderOptions.enableEnvMap);
		pathTraceShaderLowRes.SetProperty("envMapIntensity", bvhService->renderOptions.envMapIntensity);
		pathTraceShaderLowRes.SetProperty("envMapRot", bvhService->renderOptions.envMapRot / 360.0f);
		pathTraceShaderLowRes.SetProperty("maxDepth", bvhService->renderOptions.maxDepth);
		pathTraceShaderLowRes.SetProperty("uniformLightCol", bvhService->renderOptions.uniformLightCol);
		pathTraceShaderLowRes.SetProperty("roughnessMollificationAmt", bvhService->renderOptions.roughnessMollificationAmt);

		tonemapShader.SetProperty("invSampleCounter",1.0f / (sampleCounter));
		tonemapShader.SetProperty("enableTonemap",bvhService->renderOptions.enableTonemap);
		tonemapShader.SetProperty("enableAces",bvhService->renderOptions.enableAces);
		tonemapShader.SetProperty("simpleAcesFit", bvhService->renderOptions.simpleAcesFit);
		tonemapShader.SetProperty("backgroundCol", bvhService->renderOptions.backgroundCol);
		tonemapShader.SetProperty("pathTraceTexture", &accumFBO.GetAttachment<::Rendering::HAL::GLTexture>(::Rendering::Settings::EFramebufferAttachment::COLOR, 0).value());

		// set BVH Resources uniforms
		if (envMap) {
			pathTraceShader.SetProperty("envMapRes", Maths::FVector2(envMap->width, envMap->height));
			pathTraceShader.SetProperty("envMapTotalSum", envMap->totalSum);
		}
		pathTraceShader.SetProperty("topBVHIndex", bvhService->topLevelIndex);
		pathTraceShader.SetProperty("numOfLights", (int)bvhService->lights.size());
		pathTraceShader.SetProperty("accumTexture", &accumFBO.GetAttachment<::Rendering::HAL::GLTexture>(::Rendering::Settings::EFramebufferAttachment::COLOR, 0).value());
		pathTraceShader.SetProperty("BVH", BVHTex);
		pathTraceShader.SetProperty("vertexIndicesTex", vertexIndicesTex);
		pathTraceShader.SetProperty("verticesTex", verticesTex);
		pathTraceShader.SetProperty("normalsTex", normalsTex);
		pathTraceShader.SetProperty("materialsTex", materialsTex);
		pathTraceShader.SetProperty("transformsTex", transformsTex);
		pathTraceShader.SetProperty("lightsTex", lightsTex);
		pathTraceShader.SetProperty("textureMapsArrayTex", textureMapsArrayTex);
		pathTraceShader.TrySetProperty("envMapTex", envMapTex);
		pathTraceShader.TrySetProperty("envMapCDFTex", envMapCDFTex);

		if (envMap) {
			pathTraceShaderLowRes.SetProperty("envMapRes", Maths::FVector2(envMap->width, envMap->height));
			pathTraceShaderLowRes.SetProperty("envMapTotalSum", envMap->totalSum);
		}
		pathTraceShaderLowRes.SetProperty("topBVHIndex", bvhService->topLevelIndex);
		pathTraceShaderLowRes.SetProperty("numOfLights", (int)bvhService->lights.size());
		pathTraceShaderLowRes.SetProperty("accumTexture", &accumFBO.GetAttachment<::Rendering::HAL::GLTexture>(::Rendering::Settings::EFramebufferAttachment::COLOR, 0).value());
		pathTraceShaderLowRes.SetProperty("BVH", BVHTex);
		pathTraceShaderLowRes.SetProperty("vertexIndicesTex", vertexIndicesTex);
		pathTraceShaderLowRes.SetProperty("verticesTex", verticesTex);
		pathTraceShaderLowRes.SetProperty("normalsTex", normalsTex);
		pathTraceShaderLowRes.SetProperty("materialsTex", materialsTex);
		pathTraceShaderLowRes.SetProperty("transformsTex", transformsTex);
		pathTraceShaderLowRes.SetProperty("lightsTex", lightsTex);
		pathTraceShaderLowRes.SetProperty("textureMapsArrayTex", textureMapsArrayTex);
		pathTraceShaderLowRes.TrySetProperty("envMapTex", envMapTex);
		pathTraceShaderLowRes.TrySetProperty("envMapCDFTex", envMapCDFTex);
    }
	void PathTraceRenderPass::Render() {
		auto& view = GetService(Editor::Panels::SceneView);;
		auto bvhService = view.GetScene()->GetBvhService();
		if (!refreshFlag && bvhService->renderOptions.maxSpp != -1 && sampleCounter >= bvhService->renderOptions.maxSpp)
			return;
		if (refreshFlag)
		{
			::Rendering::Entities::Drawable blit;
			blit.mesh = m_renderer.m_unitQuad;
			blit.material = pathTraceShaderLowRes;
			blit.stateMask.depthWriting = false;
			blit.stateMask.colorWriting = true;
			blit.stateMask.blendable = false;
			blit.stateMask.frontfaceCulling = false;
			blit.stateMask.backfaceCulling = false;
			blit.stateMask.depthTest = false;
			
			auto pso = m_renderer.CreatePipelineState();
			pso.depthFunc = ::Rendering::Settings::EComparaisonAlgorithm::ALWAYS;
			pathTraceFBOLowRes.Bind();
			glViewport(0, 0, windowSize.x * pixelRatio, windowSize.y * pixelRatio);
			m_renderer.DrawEntity(pso, blit);
		
			pathTraceFBOLowRes.Unbind();
			//scene->instancesModified = false;
			refreshFlag = false;
			//scene->envMapModified = false;
		}
		else
		{
			::Rendering::Entities::Drawable blit;
			blit.mesh = m_renderer.m_unitQuad;
			blit.material = pathTraceShader;
			blit.stateMask.depthWriting = false;
			blit.stateMask.colorWriting = true;
			blit.stateMask.blendable = false;
			blit.stateMask.frontfaceCulling = false;
			blit.stateMask.backfaceCulling = false;
			blit.stateMask.depthTest = false;
			auto pso = m_renderer.CreatePipelineState();
			pathTracefbo.Bind();
			glViewport(0, 0, tileWidth, tileHeight);
			m_renderer.DrawEntity(pso, blit);
			pathTracefbo.Unbind();

			glNamedFramebufferReadBuffer(pathTracefbo.GetID(), GL_COLOR_ATTACHMENT0);
			glNamedFramebufferDrawBuffer(accumFBO.GetID(), GL_COLOR_ATTACHMENT0);
			glBlitNamedFramebuffer(pathTracefbo.GetID(), accumFBO.GetID(), 0, 0, tileWidth, tileHeight, tileWidth * tile.x, tileHeight * tile.y,
				tileWidth * tile.x + tileWidth, tileHeight * tile.y + tileHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);


			outputFBO[currentBuffer].Bind();
			blit.material = tonemapShader;
			glViewport(0, 0, renderSize.x, renderSize.y);
			m_renderer.DrawEntity(pso, blit);
			outputFBO[currentBuffer].Unbind();
		}
	}
	void PathTraceRenderPass::Present() {
		glViewport(0, 0, windowSize.x , windowSize.y);
		auto& view = GetService(Editor::Panels::SceneView);;
		auto bvhService = view.GetScene()->GetBvhService();
		//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		//glClearDepth(1.0);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glViewport(0, 0, renderSize.x, renderSize.y);
		//glActiveTexture(GL_TEXTURE0);
		auto& mssaaframebuffer = m_renderer.GetFrameDescriptor().outputMsaaBuffer.value();
		mssaaframebuffer.Bind();
		if (refreshFlag || sampleCounter == 1)
		{
			m_renderer.Present(pathTraceFBOLowRes);
			//glBindTexture(GL_TEXTURE_2D,);
			//quad->Draw(tonemapShader.get());
		}
		else
		{
			//if (scene->renderOptions.enableDenoiser && denoised)
			//	glBindTexture(GL_TEXTURE_2D, denoisedTexture);
			//else
				//glBindTexture(GL_TEXTURE_2D, tileOutputTexture[1 - currentBuffer]);
			m_renderer.Present(outputFBO[1-currentBuffer]);
			//quad->Draw(outputShader.get());
		}

	}
}

