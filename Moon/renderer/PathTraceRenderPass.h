#pragma once
#include <Rendering/Core/ARenderPass.h>
#include <Rendering/HAL/Framebuffer.h>
#include <Rendering/Data/Material.h>
#include <Maths/FVector3.h>
namespace Rendering {
	namespace Resources {
		class Texture;
		class Mesh;
		class Model;
	}
}
namespace Editor::Rendering
{
	class EnvironmentMap;
	class PathTraceRenderPass : public ::Rendering::Core::ARenderPass
	{
	public:
		PathTraceRenderPass(::Rendering::Core::CompositeRenderer& p_renderer);
		~PathTraceRenderPass();
	    
		void DestoryResource();
		void BuildBvhResources();
		void InitGPUDataBuffers();
		void InitFBOs();
		void InitShaders();
		virtual void ResizeRenderer(int width, int height)override;	

	protected:
		virtual void Draw(::Rendering::Data::PipelineState p_pso) override;
		void Update();
		void Render();
		void Present();
	private:
		::Rendering::Data::Material pathTraceShader;
		::Rendering::Data::Material pathTraceShaderLowRes;
		::Rendering::Data::Material outputShader;
		::Rendering::Data::Material tonemapShader;

		::Rendering::Resources::Texture* BVHTex = nullptr;
		::Rendering::Resources::Texture* vertexIndicesTex = nullptr;
		::Rendering::Resources::Texture* verticesTex = nullptr;
		::Rendering::Resources::Texture* normalsTex = nullptr;

		::Rendering::Resources::Texture* materialsTex = nullptr;
		::Rendering::Resources::Texture* transformsTex = nullptr;
		::Rendering::Resources::Texture* lightsTex = nullptr;
		::Rendering::Resources::Texture* textureMapsArrayTex = nullptr;
		::Rendering::Resources::Texture* envMapTex = nullptr;
		::Rendering::Resources::Texture* envMapCDFTex = nullptr;
		EnvironmentMap* envMap;

		::Rendering::HAL::Framebuffer pathTracefbo;
		::Rendering::HAL::Framebuffer pathTraceFBOLowRes;
		::Rendering::HAL::Framebuffer accumFBO;
		::Rendering::HAL::Framebuffer outputFBO[2];


		Maths::FVector2 renderSize;
		Maths::FVector2 windowSize;
		Maths::FVector2 tile;
		Maths::FVector2 numTiles;
		Maths::FVector2 invNumTiles;
		int tileWidth;
		int tileHeight;
		int currentBuffer;
		int frameCounter;
		int sampleCounter;
		float pixelRatio;

		float* denoiserInputFramePtr=nullptr;
		float* frameOutputPtr=nullptr;
		bool denoised;
		bool initialized;
		bool refreshFlag = true;
	};
}
