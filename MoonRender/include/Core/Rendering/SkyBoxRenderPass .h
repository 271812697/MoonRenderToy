#pragma once
#include <Rendering/Core/ARenderPass.h>
#include <Rendering/HAL/Framebuffer.h>
#include <Core/Rendering/PostProcess/AEffect.h>
#include <Core/Resources/Material.h>
namespace Core::Rendering
{
	enum class SkyMode
	{
		SkyBox=0,
		PureColor = 1,
		Gradient=2
	};
	struct SkyBoxSetting
	{
		SkyMode mode = SkyMode::SkyBox;
		Maths::FVector4 topColor{ 0.411f,0.411f,0.411f,1.0f };
		Maths::FVector4 bottomColor{ 1.0f,1.0f,1.0f,1.0f };
		Maths::FVector4 clearColor{1,1,1,1};
	};
	class SkyboxRenderPass : public ::Rendering::Core::ARenderPass
	{
	public:
		SkyboxRenderPass(::Rendering::Core::CompositeRenderer& p_renderer);
		::Rendering::HAL::Texture* GetSkyBoxCube();
		::Rendering::HAL::Texture* GetIrradianceCube();
		::Rendering::HAL::Texture* GetPrefilterCube();
		::Rendering::HAL::Texture* GetBrdfTexture();
		void updateSkyTexture(const std::string& path); 
		SkyBoxSetting& GetSetting() { return mSetting; }

	protected:
		virtual void Draw(::Rendering::Data::PipelineState p_pso) override;
	private:
		void computeSkyTexture();
		
	private:
		::Core::Resources::Material m_skyboxMaterial;
		::Core::Resources::Material m_skyboxIrrandianceMaterial;
		::Core::Resources::Material m_skyboxPrefilterMaterial;
		::Core::Resources::Material m_brdfMaterial;
		::Core::Resources::Material m_clearMaterial;
		::Rendering::HAL::Framebuffer skyBoxBuffer;
		::Rendering::HAL::Framebuffer irradianceBuffer;
		::Rendering::HAL::Framebuffer prefilterBuffer;
		::Rendering::HAL::Framebuffer brdfBuffer;
		::Rendering::Resources::Texture*skyTexture=nullptr;
		std::shared_ptr<::Rendering::HAL::Texture> skyBoxCube;
		std::shared_ptr<::Rendering::HAL::Texture> irradianceCube;
		std::shared_ptr<::Rendering::HAL::Texture> prefilterCube;
		SkyBoxSetting mSetting;
		std::string mPath = ":Textures/PureSky.hdr";
		bool needUpdateSkyTexture = true;
	};
}
