#include <ranges>
#include <tracy/Tracy.hpp>
#include <Core/ECS/Components/CModelRenderer.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/Global/ServiceLocator.h>
#include <Core/Rendering/FramebufferUtil.h>
#include <Core/Rendering/EngineBufferRenderFeature.h>
#include <Core/Rendering/EngineDrawableDescriptor.h>
#include <Core/Rendering/PostProcessRenderPass.h>
#include <Core/Rendering/ReflectionRenderFeature.h>
#include <Core/Rendering/ReflectionRenderPass.h>
#include <Core/Rendering/SkyBoxRenderPass .h>
#include <Core/Rendering/SceneRenderer.h>
#include <Core/Rendering/ShadowRenderFeature.h>
#include <Core/Rendering/ShadowRenderPass.h>
#include <Core/Rendering/GbufferPass.h>
#include <Core/ResourceManagement/ShaderManager.h>
#include <Rendering/Data/Frustum.h>
#include <Rendering/Features/LightingRenderFeature.h>
#include "Core/Rendering/SsaoRenderFeature.h"
#include <Rendering/HAL/Profiling.h>
#include <Rendering/Resources/Loaders/ShaderLoader.h>
#include <Rendering/HAL/Texture.h>

namespace
{
	using namespace Core::Rendering;

	class SceneRenderPass : public Rendering::Core::ARenderPass
	{
	public:
		SceneRenderPass(Rendering::Core::CompositeRenderer& p_renderer, bool stencilWrite = false) :
			Rendering::Core::ARenderPass(p_renderer),
			m_stencilWrite(stencilWrite)
		{
		}

	protected:
		void PrepareStencilBuffer(Rendering::Data::PipelineState& p_pso)
		{
			p_pso.stencilTest = true;
			p_pso.stencilWriteMask = 0xFF;
			p_pso.stencilFuncRef = 1;
			p_pso.stencilFuncMask = 0xFF;
			p_pso.stencilOpFail = Rendering::Settings::EOperation::REPLACE;
			p_pso.depthOpFail = Rendering::Settings::EOperation::REPLACE;
			p_pso.bothOpFail = Rendering::Settings::EOperation::REPLACE;
			p_pso.colorWriting.mask = 0x00;
		}

	private:
		bool m_stencilWrite;
	};

	class OpaqueRenderPass : public SceneRenderPass
	{
	public:
		OpaqueRenderPass(Rendering::Core::CompositeRenderer& p_renderer, bool p_stencilWrite = false) :
			SceneRenderPass(p_renderer, p_stencilWrite)
		{
		}

	protected:
		virtual void Draw(Rendering::Data::PipelineState p_pso) override
		{
			ZoneScoped;
			TracyGpuZone("OpaqueRenderPass");

			PrepareStencilBuffer(p_pso);

			const auto& drawables = m_renderer.GetDescriptor<SceneRenderer::SceneFilteredDrawablesDescriptor>();

			for (const auto& drawable : drawables.opaques | std::views::values)
			{
				m_renderer.DrawEntity(p_pso, drawable);
			}
		}
	};
	class LineRenderPass : public SceneRenderPass
	{
	public:
		LineRenderPass(Rendering::Core::CompositeRenderer& p_renderer, bool p_stencilWrite = false) :
			SceneRenderPass(p_renderer, p_stencilWrite)
		{
		}

	protected:
		virtual void Draw(Rendering::Data::PipelineState p_pso) override
		{
			ZoneScoped;
			TracyGpuZone("LineRenderPass");

			PrepareStencilBuffer(p_pso);

			// Mesh edge lines are coplanar with the faces they outline. With the
			// default strict LESS depth test they fail at equal depth, so the faces
			// hide the lines. The line pass runs after the opaque pass, so use
			// LEQUAL and skip depth writes: coplanar lines win, and lines never
			// occlude anything behind them.
			p_pso.depthFunc = ::Rendering::Settings::EComparaisonAlgorithm::GREATER_EQUAL;

			const auto& drawables = m_renderer.GetDescriptor<SceneRenderer::SceneFilteredDrawablesDescriptor>();

			for (auto drawable : drawables.lines | std::views::values)
			{
				drawable.stateMask.depthWriting = false;
				m_renderer.DrawEntity(p_pso, drawable);
			}
		}
	};
	class SectionCapRenderPass : public SceneRenderPass
	{
	public:
		SectionCapRenderPass(Rendering::Core::CompositeRenderer& p_renderer) :
			SceneRenderPass(p_renderer)
		{
			m_capMaterial.SetShader(GetShaderService[":Shaders\\SectionCap.ovfx"]);
			m_capMaterial.SetBackfaceCulling(false);
			m_capMaterial.SetFrontfaceCulling(false);
			m_capMaterial.SetDepthWriting(true);
			m_capMaterial.SetDepthTest(true);


			m_parityMaterial.SetShader(GetShaderService[":Shaders\\SectionParity.ovfx"]);
			m_parityMaterial.SetBackfaceCulling(false);
			m_parityMaterial.SetFrontfaceCulling(false);
			m_parityMaterial.SetDepthWriting(false);
			m_parityMaterial.SetDepthTest(false);
			m_parityMaterial.SetColorWriting(false);
		}

	protected:
		virtual void Draw(Rendering::Data::PipelineState p_pso) override
		{
			auto& feature = m_renderer.GetFeature<::Core::Rendering::EngineBufferRenderFeature>();
			if (feature.IsEnableClip()) {
				ZoneScoped;
				TracyGpuZone("SectionCapRenderPass");

				const auto& frameDescriptor = m_renderer.GetFrameDescriptor();
				if (!frameDescriptor.outputMsaaBuffer) return;

				// Stencil parity fill (same idea as OCCT capping): clear the stencil,
				// then INVERT it for every model surface closer to the eye than the
				// clip plane. Odd parity means the plane cuts the model at this pixel.
				m_renderer.Clear(false, false, true, Maths::FVector4(0.0f, 0.0f, 0.0f, 0.0f));

				p_pso.stencilTest = true;
				p_pso.stencilWriteMask = 0xFF;
				p_pso.stencilFuncOp = ::Rendering::Settings::EComparaisonAlgorithm::ALWAYS;
				p_pso.stencilFuncRef = 0;
				p_pso.stencilFuncMask = 0xFF;
				p_pso.stencilOpFail = ::Rendering::Settings::EOperation::KEEP;
				p_pso.depthOpFail = ::Rendering::Settings::EOperation::KEEP;
				p_pso.bothOpFail = ::Rendering::Settings::EOperation::INVERT;
				p_pso.colorWriting.mask = 0x00;
				p_pso.depthWriting = false;
				p_pso.depthTest = false;

				const auto& drawables = m_renderer.GetDescriptor<SceneRenderer::SceneFilteredDrawablesDescriptor>();
				for (auto drawable : drawables.opaques | std::views::values)
				{
					if (drawable.primitiveMode != ::Rendering::Settings::EPrimitiveMode::TRIANGLES) continue;
					const auto& desc = drawable.GetDescriptor<SceneRenderer::SceneDrawableDescriptor>();
					if (desc.actor.GetTag() == "SkyBox") continue;

					drawable.pass = "";
					drawable.material = m_parityMaterial;
					drawable.stateMask = m_parityMaterial.GenerateStateMask();
					drawable.stateMask.depthTest = false;
					drawable.stateMask.depthWriting = false;
					drawable.stateMask.colorWriting = false;
					drawable.stateMask.frontfaceCulling = false;
					drawable.stateMask.backfaceCulling = false;
					m_renderer.DrawEntity(p_pso, drawable);
				}

				// Draw the cap where the parity is odd, occluded by kept surfaces in
				// front of the clip plane (depth test against the scene depth). The
				// IBL textures are resolved here because the skybox pass allocates
				// them on its first draw, after this pass is constructed.
				m_capMaterial.SetProperty("_IrradianceCube", m_renderer.GetIrradianceCube());
				m_capMaterial.SetProperty("_PrefilterCube", m_renderer.GetPrefilterCube());
				m_capMaterial.SetProperty("_BRDFLut", m_renderer.GetBrdfTexture());
				p_pso.stencilFuncOp = ::Rendering::Settings::EComparaisonAlgorithm::NOTEQUAL;
				p_pso.stencilFuncRef = 0;
				p_pso.stencilFuncMask = 0xFF;
				p_pso.stencilOpFail = ::Rendering::Settings::EOperation::KEEP;
				p_pso.depthOpFail = ::Rendering::Settings::EOperation::KEEP;
				p_pso.bothOpFail = ::Rendering::Settings::EOperation::KEEP;
				p_pso.stencilWriteMask = 0x00;
				p_pso.colorWriting.mask = 0xFF;
				p_pso.depthWriting = false;
				p_pso.depthTest = true;
				p_pso.depthFunc = ::Rendering::Settings::EComparaisonAlgorithm::GREATER_EQUAL;

				Rendering::Entities::Drawable drawable;
				drawable.mesh = m_renderer.m_unitQuad;
				drawable.material = m_capMaterial;
				drawable.stateMask = m_capMaterial.GenerateStateMask();
				drawable.stateMask.depthWriting = true;
				drawable.stateMask.frontfaceCulling = false;
				drawable.stateMask.backfaceCulling = false;
				m_renderer.DrawEntity(p_pso, drawable);

			}
		}

	private:
		Rendering::Data::Material m_capMaterial;
		Rendering::Data::Material m_parityMaterial;
	};
	class SectionContourRenderPass : public SceneRenderPass
	{
	public:
		SectionContourRenderPass(Rendering::Core::CompositeRenderer& p_renderer) :
			SceneRenderPass(p_renderer)
		{
			m_contourMaterial.SetShader(GetShaderService[":Shaders\\SectionContour.ovfx"]);
			m_contourMaterial.SetBackfaceCulling(false);
			m_contourMaterial.SetFrontfaceCulling(false);
			m_contourMaterial.SetDepthWriting(false);
			m_contourMaterial.SetDepthTest(true);
			m_contourMaterial.SetLineWidth(3.0);
			m_contourMaterial.SetProperty("color", Maths::FVector3(1.0f, 0.0f, 1.0f));
		}

	protected:
		virtual void Draw(Rendering::Data::PipelineState p_pso) override
		{
			auto& feature = m_renderer.GetFeature<::Core::Rendering::EngineBufferRenderFeature>();
			if (feature.IsEnableClip()) {
			
				ZoneScoped;
				TracyGpuZone("SectionContourRenderPass");

				PrepareStencilBuffer(p_pso);

				p_pso.depthFunc = ::Rendering::Settings::EComparaisonAlgorithm::GREATER_EQUAL;

				const auto& drawables = m_renderer.GetDescriptor<SceneRenderer::SceneFilteredDrawablesDescriptor>();
				for (auto drawable : drawables.opaques | std::views::values)
				{
					if (drawable.primitiveMode != ::Rendering::Settings::EPrimitiveMode::TRIANGLES) continue;
					const auto& desc = drawable.GetDescriptor<SceneRenderer::SceneDrawableDescriptor>();
					if (desc.actor.GetTag() == "SkyBox") continue;

					drawable.pass = "";
					drawable.material = m_contourMaterial;
					drawable.stateMask = m_contourMaterial.GenerateStateMask();
					m_renderer.DrawEntity(p_pso, drawable);
				}			
			}
		}
	private:
		Rendering::Data::Material m_contourMaterial;
	};
	class TransparentRenderPass : public SceneRenderPass
	{
	public:
		TransparentRenderPass(Rendering::Core::CompositeRenderer& p_renderer, bool p_stencilWrite = false) :
			SceneRenderPass(p_renderer, p_stencilWrite) {
			mLayerFbo.resize(8);
			::Rendering::Settings::TextureDesc desc;
			desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA32F;
			desc.width = 1;
			desc.height = 1;
			desc.minFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR;
			desc.magFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR;
		
			desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
				.format = ::Rendering::Settings::EFormat::RGBA,
				.type = ::Rendering::Settings::EPixelDataType::FLOAT,
				.arrayLayers= static_cast<int>(mLayerFbo.size())
			    
			};
			mLayerColor = std::make_shared<::Rendering::HAL::Texture>(
				::Rendering::Settings::ETextureType::TEXTURE_2DARRAY, "layercolor");
			mLayerColor->Allocate(desc);
			

			
			::Rendering::Settings::TextureDesc colorDesc{
				.width = 1,
				.height = 1,
				.minFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR,
				.magFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR,
				.horizontalWrap = ::Rendering::Settings::ETextureWrapMode::CLAMP_TO_BORDER,
				.verticalWrap = ::Rendering::Settings::ETextureWrapMode::CLAMP_TO_BORDER,
				.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA32F,
				.useMipMaps = false,
				.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
					.format = ::Rendering::Settings::EFormat::RGBA,
					.type = ::Rendering::Settings::EPixelDataType::FLOAT
				}
			};
			::Rendering::Settings::TextureDesc depthDesc{
				.width = 1,
				.height = 1,
				.minFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST,
				.magFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST,
				.horizontalWrap = ::Rendering::Settings::ETextureWrapMode::REPEAT,
				.verticalWrap = ::Rendering::Settings::ETextureWrapMode::REPEAT,
				.internalFormat = ::Rendering::Settings::EInternalFormat::DEPTH_COMPONENT32F,
				.useMipMaps = false,
				.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
					.format = ::Rendering::Settings::EFormat::DEPTH_COMPONENT,
					.type = ::Rendering::Settings::EPixelDataType::FLOAT
				}
			};
			for (int i = 0; i < mLayerFbo.size(); i++) {
				auto renderTexture = std::make_shared<::Rendering::HAL::Texture>(
					 ::Rendering::Settings::ETextureType::TEXTURE_2D,"peelcolor");
				renderTexture->Allocate(colorDesc);
				auto depthTexture = std::make_shared<::Rendering::HAL::Texture>(
					::Rendering::Settings::ETextureType::TEXTURE_2D,"peeldepth");
				depthTexture->Allocate(depthDesc);
				depthTexture->SetBorderColor(Maths::FVector4::Zero);
				mLayerFbo[i].Attach<::Rendering::HAL::Texture>(renderTexture, ::Rendering::Settings::EFramebufferAttachment::COLOR);
				mLayerFbo[i].Attach<::Rendering::HAL::Texture>(depthTexture, ::Rendering::Settings::EFramebufferAttachment::DEPTH);
				if (!mLayerFbo[i].Validate()) {
					std::cout << "invalidate buffer" << std::endl;
				}
			}

			auto renderTexture = std::make_shared<::Rendering::HAL::Texture>(
				::Rendering::Settings::ETextureType::TEXTURE_2D, "peelcolor");
			renderTexture->Allocate(colorDesc);
			auto depthTexture = std::make_shared<::Rendering::HAL::Texture>(
				::Rendering::Settings::ETextureType::TEXTURE_2D, "peeldepth");
			depthTexture->Allocate(depthDesc);
			depthTexture->SetBorderColor(Maths::FVector4::Zero);
			mBlendFbo.Attach<::Rendering::HAL::Texture>(renderTexture, ::Rendering::Settings::EFramebufferAttachment::COLOR);
			mBlendFbo.Attach<::Rendering::HAL::Texture>(depthTexture, ::Rendering::Settings::EFramebufferAttachment::DEPTH);
			std::string v = R"(
#version 450 core

layout(location = 0) in vec2 geo_Pos;
layout(location = 1) in vec2 geo_TexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = geo_TexCoords;
    gl_Position = vec4(geo_Pos, 0.0, 1.0);
}
)";
			std::string f = R"(
#version 450 core

in vec2 TexCoords;
out vec4 FRAGMENT_COLOR;

uniform sampler2D colorLayer;

void main()
{
	FRAGMENT_COLOR = texture(colorLayer, TexCoords);
}

)";
			mBlendColorShader = ::Rendering::Resources::Loaders::ShaderLoader::CreateFromSource(v, f);
			mBlendColorMat.SetShader(mBlendColorShader);
		}

	protected:
		virtual void ResizeRenderer(int width, int height) override {
			mLayerColor->Resize(width,height);
			mBlendFbo.Resize(width,height);
			for (int i = 0; i < mLayerFbo.size(); i++) {
				mLayerFbo[i].Resize(width,height);
			}
		}
		virtual void Draw(Rendering::Data::PipelineState p_pso) override
		{
			ZoneScoped;
			TracyGpuZone("TransparentRenderPass");
			const auto& drawables = m_renderer.GetDescriptor<SceneRenderer::SceneFilteredDrawablesDescriptor>();
			if (drawables.transparents.size()>0) {
				auto& msaaframebuffer = m_renderer.GetFrameDescriptor().outputMsaaBuffer.value();
				Core::Rendering::FramebufferUtil::CopyFramebufferColor(msaaframebuffer, 0, mBlendFbo, 0);
				Core::Rendering::FramebufferUtil::CopyFramebufferDepth(msaaframebuffer, mBlendFbo); 
			
				//peel 0 layers, use the depth buffer copied from opaque pass to peel the first layer of transparent
				//we can have better method to optimize the copy like that
				//we prepare a empty(value of depth is 0) depth texture,then we can unify the shader and (step1 and step2) for peeling all layers,
				//but for now we just want to get it working
				Core::Rendering::FramebufferUtil::CopyFramebufferDepth(mBlendFbo, mLayerFbo[0]);
				PrepareStencilBuffer(p_pso);
				/*
				1.the first step: peel the first layer of transparent and write the depth to layerfbo0, then we can use this depth to peel the second layer and so on
				*/
				mLayerFbo[0].Bind();
				m_renderer.Clear(true,false,false,Maths::FVector4(0,0,0,0));

				for ( auto drawable : drawables.transparents | std::views::values)
				{
					drawable.pass = "";				
					drawable.stateMask.depthWriting = true;
					drawable.stateMask.colorWriting = true;
					drawable.stateMask.blendable = false;
					drawable.stateMask.frontfaceCulling = false;
					drawable.stateMask.backfaceCulling = false;
					drawable.stateMask.depthTest = true;
					m_renderer.DrawEntity(p_pso, drawable);
				}
				mLayerFbo[0].Unbind();
				/*
				* 2.the second step: peel the second layer and the others, 
				we use the depth texture of previous layer to peel the next layer, 
				and we also need the depth texture of opaque layer to do culling,
				so we need to bind both of them to shader, 
				then we can peel all the layers in one pass,
				but for now we just do it one by one
				*/
				//peel the others layers
				for (int layer = 1; layer < mLayerFbo.size(); layer++) {
			
					int currFbo = layer ;
					int prevFbo = (layer - 1) ;

					// 绑定当前 FBO
					mLayerFbo[currFbo].Bind();
					m_renderer.Clear(true, true, false, Maths::FVector4(0, 0, 0, 0));
					const auto& depth=mLayerFbo[prevFbo].GetAttachment<::Rendering::HAL::GLTexture>(::Rendering::Settings::EFramebufferAttachment::DEPTH,0);
					const auto& opaquedepth = mBlendFbo.GetAttachment<::Rendering::HAL::GLTexture>(::Rendering::Settings::EFramebufferAttachment::DEPTH, 0);
					for ( auto drawable : drawables.transparents | std::views::values)
					{
						drawable.stateMask.depthWriting = true;
						drawable.stateMask.colorWriting = true;
						drawable.stateMask.blendable = false;
						drawable.stateMask.frontfaceCulling = false;
						drawable.stateMask.backfaceCulling = false;
						drawable.stateMask.depthTest = true;
						drawable.material->TrySetProperty("DepthPeelTex",&depth.value());
						drawable.material->TrySetProperty("OpaqueDepthTex", &opaquedepth.value());
						m_renderer.DrawEntity(p_pso, drawable);
					}
					mLayerFbo[currFbo].Unbind();
				}
				/*
				 * 3.the third step: blend the peeled layers back to front,
				 * we can also do it in one pass by binding all the peeled color textures to shader, but for now we just do it one by one
				 */
				mBlendFbo.Bind();
				for (int i = mLayerFbo.size() - 1; i >= 0; i--) {
					const auto& color = mLayerFbo[i].GetAttachment<::Rendering::HAL::GLTexture>(::Rendering::Settings::EFramebufferAttachment::COLOR, 0);
					mBlendColorMat.SetProperty("colorLayer",&color.value());
					Rendering::Entities::Drawable blit;
					blit.mesh = m_renderer.m_unitQuad;
					blit.material = mBlendColorMat;
					blit.stateMask.depthWriting = false;
					blit.stateMask.colorWriting = true;
					blit.stateMask.blendable = true;
					blit.stateMask.frontfaceCulling = false;
					blit.stateMask.backfaceCulling = false;
					blit.stateMask.depthTest = false;
					m_renderer.DrawEntity(p_pso,blit);
				}
				mBlendFbo.Unbind();
				
				msaaframebuffer.Bind();
				const auto& content = mBlendFbo.GetAttachment<::Rendering::HAL::GLTexture>(::Rendering::Settings::EFramebufferAttachment::COLOR, 0);
				m_renderer.Present(content.value());
			}
		}
	private:
		::Rendering::Resources::Shader* mBlendColorShader;
		::Rendering::Data::Material mBlendColorMat;
		std::vector<::Rendering::HAL::Framebuffer>mLayerFbo ;
		::Rendering::HAL::Framebuffer mBlendFbo;
		std::shared_ptr<::Rendering::HAL::Texture>mLayerColor;
	};

	class UIRenderPass : public SceneRenderPass
	{
	public:
		UIRenderPass(Rendering::Core::CompositeRenderer& p_renderer, bool p_stencilWrite = false) :
			SceneRenderPass(p_renderer, p_stencilWrite) {
		}

	protected:
		virtual void Draw(Rendering::Data::PipelineState p_pso) override
		{
			ZoneScoped;
			TracyGpuZone("UIRenderPass");

			PrepareStencilBuffer(p_pso);

			const auto& drawables = m_renderer.GetDescriptor<SceneRenderer::SceneFilteredDrawablesDescriptor>();

			for (const auto& drawable : drawables.ui | std::views::values)
			{
				m_renderer.DrawEntity(p_pso, drawable);
			}
		}
	};

	Rendering::Features::LightingRenderFeature::LightSet FindActiveLights(const Core::SceneSystem::Scene& p_scene)
	{
		Rendering::Features::LightingRenderFeature::LightSet lights;

		const auto& facs = p_scene.GetFastAccessComponents();

		for (auto light : facs.lights)
		{
			if (light->owner.IsActive())
			{
				lights.push_back(std::ref(light->GetData()));
			}
		}

		return lights;
	}

	std::vector<std::reference_wrapper<Core::ECS::Components::CReflectionProbe>> FindActiveReflectionProbes(const Core::SceneSystem::Scene& p_scene)
	{
		std::vector<std::reference_wrapper<Core::ECS::Components::CReflectionProbe>> probes;
		const auto& facs = p_scene.GetFastAccessComponents();
		for (auto probe : facs.reflectionProbes)
		{
			if (probe->owner.IsActive())
			{
				probes.push_back(*probe);
			}
		}
		return probes;
	}
}

Core::Rendering::SceneRenderer::SceneRenderer(::Rendering::Context::Driver& p_driver, bool p_stencilWrite)
	: ::Rendering::Core::CompositeRenderer(p_driver)
{
	using namespace ::Rendering::Features;
	using namespace ::Rendering::Settings;
	using enum ::Rendering::Features::EFeatureExecutionPolicy;

	AddFeature<EngineBufferRenderFeature, ALWAYS>();
	AddFeature<LightingRenderFeature, ALWAYS>();

	AddFeature<ReflectionRenderFeature, WHITELIST_ONLY>()
		.Include<OpaqueRenderPass>()
		.Include<TransparentRenderPass>();
	AddFeature<SsaoRenderFeature, WHITELIST_ONLY>()
		.Include<OpaqueRenderPass>();

	AddFeature<ShadowRenderFeature, WHITELIST_ONLY>()
		.Include<OpaqueRenderPass>()
		.Include<TransparentRenderPass>()
		.Include<UIRenderPass>();

	AddPass<ShadowRenderPass>("Shadows", ERenderPassOrder::Shadows);
	//AddPass<ReflectionRenderPass>("ReflectionRenderPass", ERenderPassOrder::Reflections);
	AddPass<SkyboxRenderPass>("SkyboxRenderPass",ERenderPassOrder::SkyBox);
	AddPass<GbufferPass>("Gbuffer", ERenderPassOrder::Opaque-1);
	AddPass<OpaqueRenderPass>("Opaques", ERenderPassOrder::Opaque, p_stencilWrite);

	AddPass<SectionCapRenderPass>("SectionCap", ERenderPassOrder::SectionCap);
	AddPass<SectionContourRenderPass>("SectionContour", ERenderPassOrder::SectionContour);


	AddPass<LineRenderPass>("Lines", ERenderPassOrder::LineAfterPathTrace, p_stencilWrite);
	AddPass<TransparentRenderPass>("Transparents", ERenderPassOrder::Transparent, p_stencilWrite);
	AddPass<PostProcessRenderPass>("Post-Process", ERenderPassOrder::PostProcessing);
	AddPass<UIRenderPass>("UI", ERenderPassOrder::UI);
}

void Core::Rendering::SceneRenderer::BeginFrame(const ::Rendering::Data::FrameDescriptor& p_frameDescriptor)
{
	ZoneScoped;

	assert(HasDescriptor<SceneDescriptor>()&& "Cannot find SceneDescriptor attached to this renderer");

	auto& sceneDescriptor = GetDescriptor<SceneDescriptor>();

	const bool frustumLightCulling = p_frameDescriptor.camera.value().HasFrustumLightCulling();

	AddDescriptor<::Rendering::Features::LightingRenderFeature::LightingDescriptor>({
		FindActiveLights(sceneDescriptor.scene),
		frustumLightCulling ? sceneDescriptor.frustumerride : std::nullopt
		});

	AddDescriptor<Core::Rendering::ReflectionRenderFeature::ReflectionDescriptor>({
		FindActiveReflectionProbes(sceneDescriptor.scene)
		});

	::Rendering::Core::CompositeRenderer::BeginFrame(p_frameDescriptor);

	AddDescriptor<SceneDrawablesDescriptor>({
		ParseScene(SceneParsingInput{
			.scene = sceneDescriptor.scene
		})
		});

	// Default filtered drawables descriptor using the main camera (used by most render passes).
	// Some other render passes can decide to filter the drawables themselves, using the 
	// SceneDrawablesDescriptor instead of the SceneFilteredDrawablesDescriptor one.
	AddDescriptor<SceneFilteredDrawablesDescriptor>({
		FilterDrawables(
			GetDescriptor<SceneDrawablesDescriptor>(),
			SceneDrawablesFilteringInput{
				.camera = p_frameDescriptor.camera.value(),
				.frustumerride = sceneDescriptor.frustumerride,
				.errideMaterial = sceneDescriptor.errideMaterial,
				.fallbackMaterial = sceneDescriptor.fallbackMaterial,
				.requiredVisibilityFlags = EVisibilityFlags::GEOMETRY
			}
		)
		});
}
void Core::Rendering::SceneRenderer::Resize(int width, int height)
{
	for (const auto& pass : m_passes | std::views::values)
	{
		//if (pass.second->IsEnabled())
		{
			pass.second->ResizeRenderer(width,height);;
		}
	}
}
void Core::Rendering::SceneRenderer::DrawModelWithSingleMaterial(::Rendering::Data::PipelineState p_pso, ::Rendering::Resources::Model& p_model, ::Rendering::Data::Material& p_material, const Maths::FMatrix4& p_modelMatrix)
{
	auto stateMask = p_material.GenerateStateMask();
	auto userMatrix = Maths::FMatrix4::Identity;

	auto engineDrawableDescriptor = EngineDrawableDescriptor{
		p_modelMatrix,
		userMatrix
	};

	for (auto mesh : p_model.GetMeshes())
	{
		::Rendering::Entities::Drawable element;
		element.mesh = *mesh;
		element.material = p_material;
		element.stateMask = stateMask;
		element.AddDescriptor(engineDrawableDescriptor);

		DrawEntity(p_pso, element);
	}
}

SceneRenderer::SceneDrawablesDescriptor Core::Rendering::SceneRenderer::ParseScene(const SceneParsingInput& p_input)
{
	ZoneScoped;

	using namespace Core::ECS::Components;

	// Containers for the parsed drawables.
	SceneRenderer::SceneDrawablesDescriptor result;

	const auto& scene = p_input.scene;

	for (const auto modelRenderer : scene.GetFastAccessComponents().modelRenderers)
	{
		auto& owner = modelRenderer->owner;
		if (!owner.IsActive()) continue;
		const auto model = modelRenderer->GetModel();
		if (!model) continue;
		const auto materialRenderer = modelRenderer->owner.GetComponent<CMaterialRenderer>();
		if (!materialRenderer) continue;

		const auto& transform = owner.transform.GetFTransform();
		const auto& materials = materialRenderer->GetMaterials();

		for (auto& mesh : model->GetMeshes())
		{
			std::vector<uint32_t> meshMatIndex = mesh->GetMaterialIndex();
			std::vector<uint32_t> meshRangeBufferIndex = mesh->GetSubRangeBufferIndex();
			for(int i=0;i<meshMatIndex.size();i++)
			{
				auto& materialIndex = meshMatIndex[i];
				int bufferIndex = meshRangeBufferIndex[i];
				if (mesh->GetIndexCount(bufferIndex) <= 0) {
					continue;
				}
				Tools::Utils::OptRef<::Rendering::Data::Material> material;

				if (materialIndex < kMaxMaterialCount)
				{
					material = materials.at(materialIndex);
				}
				auto primMode=mesh->GetPrimitiveMode();
				::Rendering::Entities::Drawable drawable{
					.mesh = *mesh,
					.material = material,
					.stateMask = material.has_value() ? material->GenerateStateMask() : ::Rendering::Data::StateMask{},
					.subIndexBufferIndex= bufferIndex,
					.primitiveMode=primMode,
				};

				auto bounds = [&]() -> std::optional<::Rendering::Geometry::BoundingSphere> {
					using enum CModelRenderer::EFrustumBehaviour;
					switch (modelRenderer->GetFrustumBehaviour())
					{
					case MESH_BOUNDS: return mesh->GetBoundingSphere();
					case DEPRECATED_MODEL_BOUNDS: return model->GetBoundingSphere();
					case CUSTOM_BOUNDS: return modelRenderer->GetCustomBoundingSphere();
					}
					return std::nullopt;
					}();
				drawable.AddDescriptor<SceneDrawableDescriptor>({
					.actor = modelRenderer->owner,
					.visibilityFlags = materialRenderer->GetVisibilityFlags(),
					.bounds = bounds,
					});
				drawable.AddDescriptor<EngineDrawableDescriptor>({
					transform.GetWorldMatrix(),
					materialRenderer->GetUserMatrix()
					});
				result.drawables.push_back(drawable);
			}
		}
	}
	return result;
}

SceneRenderer::SceneFilteredDrawablesDescriptor Core::Rendering::SceneRenderer::FilterDrawables(
	const SceneDrawablesDescriptor& p_drawables,
	const SceneDrawablesFilteringInput& p_filteringInput
)
{
	ZoneScoped;

	using namespace Core::ECS::Components;

	SceneFilteredDrawablesDescriptor output;

	const auto& camera = p_filteringInput.camera;
	const auto& frustumerride = p_filteringInput.frustumerride;

	// Determine if we should use frustum culling
	Tools::Utils::OptRef<const ::Rendering::Data::Frustum> frustum;
	if (camera.HasFrustumGeometryCulling())
	{
		frustum = frustumerride ? frustumerride : camera.GetFrustum();
	}

	// Process each drawable
	for (const auto& drawable : p_drawables.drawables)
	{
		const auto& desc = drawable.GetDescriptor<SceneDrawableDescriptor>();

		// Skip drawables that do not satisfy the required visibility flags
		if (!SatisfiesVisibility(desc.visibilityFlags, p_filteringInput.requiredVisibilityFlags))
		{
			continue;
		}

		const auto targetMaterial =
			p_filteringInput.errideMaterial.has_value() ?
			p_filteringInput.errideMaterial.value() :
			(drawable.material.has_value() ? drawable.material.value() : p_filteringInput.fallbackMaterial);

		// Skip if material is invalid
		if (!targetMaterial || !targetMaterial->IsValid()) continue;

		// Filter drawables based on the type (UI, opaque, transparent)
		// Except for the fallback material, which is always included.
		if (!p_filteringInput.fallbackMaterial || &p_filteringInput.fallbackMaterial.value() != &targetMaterial.value())
		{
			const bool isUI = targetMaterial->IsUserInterface();
			if (isUI && !p_filteringInput.includeUI) continue;
			if (!isUI && !targetMaterial->IsBlendable() && !p_filteringInput.includeOpaque) continue;
			if (!isUI && targetMaterial->IsBlendable() && !p_filteringInput.includeTransparent) continue;
		}

		// Perform frustum culling if enabled
		if (frustum && desc.bounds.has_value())
		{
			ZoneScopedN("Frustum Culling");

			// Get the engine drawable descriptor to access transform information
			const auto& engineDesc = drawable.GetDescriptor<EngineDrawableDescriptor>();

			if (!frustum->BoundingSphereInFrustum(desc.bounds.value(), desc.actor.transform.GetFTransform()))
			{
				continue; // Skip this drawable as it's outside the frustum
			}
		}

		// Calculate distance to camera for sorting
		const float distanceToCamera = Maths::FVector3::Distance(
			desc.actor.transform.GetWorldPosition(),
			camera.GetPosition()
		);

		// At this point we want to copy the drawable to avoid modifying the original one.
		// The copy will use the updated material.
		// At this point, the filtered drawable should be guaranteed to have a valid material.
		auto drawableCopy = drawable;
		drawableCopy.material = targetMaterial;
		drawableCopy.stateMask = targetMaterial->GenerateStateMask();
		

		// Categorize drawable based on their type.
		// This is also where sorting happens, using
		// the multimap key.
		if (drawableCopy.material->IsUserInterface())
		{
			output.ui.emplace(decltype(decltype(output.ui)::value_type::first){
				.order = drawableCopy.material->GetDrawOrder(),
					.distance = distanceToCamera
			}, drawableCopy);
		}
		else if (drawableCopy.primitiveMode == ::Rendering::Settings::EPrimitiveMode::LINES) {
			output.lines.emplace(decltype(decltype(output.lines)::value_type::first){
				.order = drawableCopy.material->GetDrawOrder(),
					.distance = distanceToCamera
			}, drawableCopy);
		}
		else if (drawableCopy.material->IsTransparent())
		{
			drawableCopy.pass = "Transparents";
			output.transparents.emplace(decltype(decltype(output.transparents)::value_type::first){
				.order = drawableCopy.material->GetDrawOrder(),
					.distance = distanceToCamera
			}, drawableCopy);
		}
		else
		{
			output.opaques.emplace(decltype(decltype(output.opaques)::value_type::first){
				.order = drawableCopy.material->GetDrawOrder(),
					.distance = distanceToCamera
			}, drawableCopy);
		}
	}
	return output;
}
