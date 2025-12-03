#include <tracy/Tracy.hpp>
#include <Core/ECS/Components/CPostProcessStack.h>
#include "renderer/PathTraceRenderPass.h"
#include <Core/Global/ServiceLocator.h>
#include <Core/Rendering/FramebufferUtil.h>
#include <Core/Rendering/SceneRenderer.h>
#include <Core/ResourceManagement/ShaderManager.h>
#include <Core/SceneSystem/BvhService.h>
#include "Rendering/Resources/Texture.h"
#include "renderer/SceneView.h"
#include <Rendering/Core/CompositeRenderer.h>
#include <Rendering/HAL/Profiling.h>
#include <Rendering/Settings/EPixelDataFormat.h>
#include <Rendering/Settings/ETextureType.h>
#include <stb_Image/stb_image.h>
namespace Editor::Rendering {
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
	PathTraceRenderPass::PathTraceRenderPass(::Rendering::Core::CompositeRenderer& p_renderer): ::Rendering::Core::ARenderPass(p_renderer)
	{
	}
	void PathTraceRenderPass::Draw(::Rendering::Data::PipelineState p_pso)
	{
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



	}
	void PathTraceRenderPass::InitShaders() {

	}
	void PathTraceRenderPass::InitFBOs() {

	}
}

