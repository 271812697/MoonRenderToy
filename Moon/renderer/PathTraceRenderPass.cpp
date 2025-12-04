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
	     
		InitShaders();
	}
	PathTraceRenderPass::~PathTraceRenderPass()
	{
		delete envMap;
		DestoryResource();
	}
	void PathTraceRenderPass::DestoryResource()
	{
		
		delete BVHTex;
		delete vertexIndicesTex;
		delete verticesTex;
		delete normalsTex;
		delete materialsTex;
		delete transformsTex;
		delete lightsTex;
		delete textureMapsArrayTex;
		delete envMapTex;
		delete envMapCDFTex;



		delete[] denoiserInputFramePtr;
		delete[] frameOutputPtr;
	}
	void PathTraceRenderPass::BuildBvhResources()
	{
		DestoryResource();
		InitGPUDataBuffers();
	}
	void PathTraceRenderPass::Draw(::Rendering::Data::PipelineState p_pso)
	{
		auto bvhService= GetService(Editor::Panels::SceneView).GetScene()->GetBvhService();
		if (bvhService->isDirty) {
			bvhService->isDirty = false;
			BuildBvhResources();
		}
	}
	void PathTraceRenderPass::InitGPUDataBuffers() {
		auto& view = GetService(Editor::Panels::SceneView);;
		auto bvhService= view.GetScene()->GetBvhService();
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		// BVH Texture
		::Rendering::Settings::TextureDesc desc;
		desc.isTextureBuffer = true;
		desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGB32F;
		desc.buffetLen =  bvhService->nodes.size()* sizeof(::Core::SceneSystem::BvhService::Node);
		desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
			.data = bvhService->nodes.data()
		};
		BVHTex = new ::Rendering::Resources::Texture();;
		auto gltexture = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_BUFFER);
		gltexture->Allocate(desc);
		BVHTex->SetTexture(std::unique_ptr<::Rendering::HAL::Texture>(gltexture));

		// Vertex Indices Texture
		desc.internalFormat= ::Rendering::Settings::EInternalFormat::RGB32I;
		desc.buffetLen = bvhService->vertIndices.size() * sizeof(::Core::SceneSystem::Indices);
		desc.mutableDesc= ::Rendering::Settings::MutableTextureDesc{
			.data = bvhService->vertIndices.data()
		};
		vertexIndicesTex = new ::Rendering::Resources::Texture();
		gltexture = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_BUFFER);
		gltexture->Allocate(desc);
		vertexIndicesTex->SetTexture(std::unique_ptr<::Rendering::HAL::Texture>(gltexture));

		// Vertices Texture
		desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA32F;
		desc.buffetLen = bvhService->verticesUVX.size() * sizeof(Maths::FVector4);
		desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
			.data = bvhService->verticesUVX.data()
		};
		verticesTex = new ::Rendering::Resources::Texture();
		gltexture = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_BUFFER);
		gltexture->Allocate(desc);
		verticesTex->SetTexture(std::unique_ptr<::Rendering::HAL::Texture>(gltexture));

		// Normals Texture
		desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA32F;
		desc.buffetLen = bvhService->normalsUVY.size() * sizeof(Maths::FVector4);
		desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
			.data = bvhService->normalsUVY.data()
		};
		normalsTex = new ::Rendering::Resources::Texture();
		gltexture = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_BUFFER);
		gltexture->Allocate(desc);
		normalsTex->SetTexture(std::unique_ptr<::Rendering::HAL::Texture>(gltexture));

		//Material Texture
		desc.isTextureBuffer = false;
		desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA32F;
		desc.width = bvhService->materials.size()*(sizeof(::Core::SceneSystem::Material)/sizeof(Maths::FVector4));
		desc.height = 1;
		desc.minFilter= ::Rendering::Settings::ETextureFilteringMode::NEAREST;
		desc.magFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST;
		desc.buffetLen = bvhService->materials.size() * sizeof(::Core::SceneSystem::Material);
		desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
			.format = ::Rendering::Settings::EFormat::RGBA,
			.type = ::Rendering::Settings::EPixelDataType::FLOAT,
			.data = bvhService->materials.data()
		};
		materialsTex = new ::Rendering::Resources::Texture();
		gltexture = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_2D);
		gltexture->Allocate(desc);
		materialsTex->SetTexture(std::unique_ptr<::Rendering::HAL::Texture>(gltexture));
		
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
		transformsTex = new ::Rendering::Resources::Texture();
		gltexture = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_2D);
		gltexture->Allocate(desc);
		transformsTex->SetTexture(std::unique_ptr<::Rendering::HAL::Texture>(gltexture));

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
			lightsTex = new ::Rendering::Resources::Texture();
			gltexture = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_2D);
			gltexture->Allocate(desc);
			lightsTex->SetTexture(std::unique_ptr<::Rendering::HAL::Texture>(gltexture));
		
		}

		// Enviroment Texture
		desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGB32F;
		desc.width = envMap->width;
		desc.height = envMap->height;
		desc.minFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR;
		desc.magFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR;
		desc.buffetLen = envMap->width * envMap->height*sizeof(Maths::FVector3);
		desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
			.format = ::Rendering::Settings::EFormat::RGB,
			.type = ::Rendering::Settings::EPixelDataType::FLOAT,
			.data = envMap->img
		};
		envMapTex = new ::Rendering::Resources::Texture();
		gltexture = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_2D);
		gltexture->Allocate(desc);
		envMapTex->SetTexture(std::unique_ptr<::Rendering::HAL::Texture>(gltexture));

		desc.internalFormat = ::Rendering::Settings::EInternalFormat::R32F;
		desc.minFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST;
		desc.magFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST;
		desc.buffetLen = envMap->width * envMap->height * sizeof(float);
		desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
			.format = ::Rendering::Settings::EFormat::RED,
			.type = ::Rendering::Settings::EPixelDataType::FLOAT,
			.data = envMap->cdf
		};
		envMapCDFTex = new ::Rendering::Resources::Texture();
		gltexture = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_2D);
		gltexture->Allocate(desc);
		envMapCDFTex->SetTexture(std::unique_ptr<::Rendering::HAL::Texture>(gltexture));

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
			textureMapsArrayTex = new ::Rendering::Resources::Texture();
			gltexture = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_2DARRAY);
			gltexture->Allocate(desc);
			textureMapsArrayTex->SetTexture(std::unique_ptr<::Rendering::HAL::Texture>(gltexture));
		}
	}
	void PathTraceRenderPass::InitShaders() {
		
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
			if (idx != -1)
				idx = tempPathTraceShaderSrcObj.find("\n", idx);
			else
				idx = 0;
			tempPathTraceShaderSrcObj.insert(idx + 1, pathtraceDefines);

			idx = tempPathTraceShaderLowResSrcObj.find("#shader fragment");
			if (idx != -1)
				idx = tempPathTraceShaderLowResSrcObj.find("\n", idx);
			else
				idx = 0;
			tempPathTraceShaderLowResSrcObj.insert(idx + 1, pathtraceDefines);
		}

		if (tonemapDefines.size() > 0)
		{
			size_t idx = tempTonemapShaderSrcObj.find("#shader fragment");
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
			auto shader=::Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager.CreateFromSource(tempPathTraceShaderSrcObj);
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
		InitFBOs();
	}
	void PathTraceRenderPass::InitFBOs() {
		sampleCounter = 1;
		currentBuffer = 0;
		frameCounter = 1;
		auto& view = GetService(Editor::Panels::SceneView);;
		auto bvhService = view.GetScene()->GetBvhService();
		auto [w,h] = view.GetSafeSize();
		renderSize = { static_cast<float>(w) ,static_cast<float>(h)  };
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

		pathTracefbo.Resize(tileWidth,tileHeight);
		pathTraceFBOLowRes.Resize(static_cast<int>(windowSize.x * pixelRatio), static_cast<int>(windowSize.y * pixelRatio));
		accumFBO.Resize(static_cast<int>(renderSize.x), static_cast<int>(renderSize.y));
		outputFBO[0].Resize(static_cast<int>(renderSize.x), static_cast<int>(renderSize.y));
		outputFBO[1].Resize(static_cast<int>(renderSize.x), static_cast<int>(renderSize.y));
	}
}

