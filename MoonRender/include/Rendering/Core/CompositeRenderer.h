#pragma once
#include <typeindex>
#include <memory>
#include <Rendering/Core/ABaseRenderer.h>
#include <Rendering/Core/ARenderPass.h>
#include <Rendering/Data/Describable.h>
#include <Rendering/Features/ARenderFeature.h>
#include <Rendering/Types/RenderFeatureType.h>
#include <Rendering/Types/RenderPassType.h>

namespace Rendering::Core
{
	class CompositeRenderer : public ABaseRenderer, public Data::Describable
	{
	public:
		Tools::Eventing::Event<Rendering::Data::PipelineState&, const Entities::Drawable&> preDrawEntityEvent;
		Tools::Eventing::Event<const Entities::Drawable&> postDrawEntityEvent;

		CompositeRenderer(Context::Driver& p_driver);

		virtual void BeginFrame(const Data::FrameDescriptor& p_frameDescriptor);

		virtual void DrawFrame() final;

		virtual void EndFrame() override;


		virtual void DrawEntity(
			Rendering::Data::PipelineState p_pso,
			const Entities::Drawable& p_drawable
		) override;


		template<Types::RenderFeatureType T, Features::EFeatureExecutionPolicy Policy, typename ... Args>
		T& AddFeature(Args&&... p_args);

		template<Types::RenderFeatureType T>
		bool RemoveFeature();

		template<Types::RenderFeatureType T>
		T& GetFeature() const;

		template<Types::RenderFeatureType T>
		bool HasFeature() const;

		template<Types::RenderPassType T, typename ... Args>
		T& AddPass(const std::string& p_name, uint32_t p_order, Args&&... p_args);

		template<Types::RenderPassType T>
		T& GetPass(const std::string& p_name) const;
		std::multimap<uint32_t, std::pair<std::string, std::unique_ptr<Core::ARenderPass>>>& GetPasses() {
			return m_passes;
		}
		::Rendering::HAL::Texture* GetSkyBoxCube();
		::Rendering::HAL::Texture* GetIrradianceCube();
		::Rendering::HAL::Texture* GetPrefilterCube();
		::Rendering::HAL::Texture* GetBrdfTexture();
	protected:
		std::unordered_map<std::type_index, std::unique_ptr<Features::ARenderFeature>> m_features;
		std::multimap<uint32_t, std::pair<std::string, std::unique_ptr<Core::ARenderPass>>> m_passes;

	private:
		Tools::Utils::OptRef<Core::ARenderPass> m_currentPass;
	};
}

#include "Rendering/Core/CompositeRenderer.inl"
