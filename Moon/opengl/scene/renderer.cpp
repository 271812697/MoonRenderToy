#include "../pch.h"


#include <glad/glad.h>



#include "core/log.h"

#include "../asset/buffer.h"
#include "../asset/fbo.h"
#include "../asset/shader.h"
#include "../component/all.h"
#include "../scene/entity.h"
#include "../scene/renderer.h"
#include "../scene/scene.h"
#include "../util/ext.h"
#include "../util/path.h"


using namespace asset;
using namespace component;

namespace scene {

	Scene* Renderer::last_scene = nullptr;
	Scene* Renderer::curr_scene = nullptr;
	std::queue<entt::entity> Renderer::render_queue{};

	static bool depth_prepass = false;
	static uint shadow_index = 0U;
	static asset_tmp<UBO> renderer_input = nullptr;

	///////////////////////////////////////////////////////////////////////////////////////////////
	void Renderer::SetScene(Scene* s) {

		curr_scene = s;
		// create the renderer input UBO on the first run (internal UBO)
		if (renderer_input == nullptr) {
			const std::vector<GLuint> offset{ 0U, 8U, 16U, 20U, 24U, 28U, 32U, 36U };
			const std::vector<GLuint> length{ 8U, 8U, 4U, 4U, 4U, 4U, 4U, 4U };
			const std::vector<GLuint> stride{ 8U, 8U, 4U, 4U, 4U, 4U, 4U, 4U };

			renderer_input = WrapAsset<UBO>(10, offset, length, stride);
		}

	}
	const Scene* Renderer::GetScene() {
		return curr_scene;
	}

	void Renderer::MSAA(bool enable) {
		// the built-in MSAA only works on the default framebuffer (without multi-pass)
		static GLint buffers = 0, samples = 0, max_samples = 0;
		if (samples == 0) {
			glGetIntegerv(GL_SAMPLE_BUFFERS, &buffers);
			glGetIntegerv(GL_SAMPLES, &samples);
			glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
			CORE_ASERT(buffers > 0, "MSAA buffers are not available! Check your window context...");
			CORE_ASERT(samples == 4, "Invalid MSAA buffer size! 4 samples per pixel is not available...");
		}

		static bool is_enabled = false;

		if (enable && !is_enabled) {
			glEnable(GL_MULTISAMPLE);
			is_enabled = true;
		}
		else if (!enable && is_enabled) {
			glDisable(GL_MULTISAMPLE);
			is_enabled = false;
		}
	}

	void Renderer::DepthPrepass(bool enable) {
		depth_prepass = enable;
	}

	void Renderer::DepthTest(bool enable) {
		static bool is_enabled = false;

		if (enable && !is_enabled) {
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
			glDepthFunc(GL_LEQUAL);
			glDepthRange(0.0f, 1.0f);
			is_enabled = true;
		}
		else if (!enable && is_enabled) {
			glDisable(GL_DEPTH_TEST);
			is_enabled = false;
		}
	}

	void Renderer::StencilTest(bool enable) {
		static bool is_enabled = false;

		if (enable && !is_enabled) {
			glEnable(GL_STENCIL_TEST);
			glStencilMask(0xFF);
			glStencilFunc(GL_EQUAL, 1, 0xFF);  // discard fragments whose stencil values != 1
			is_enabled = true;
		}
		else if (!enable && is_enabled) {
			glDisable(GL_STENCIL_TEST);
			is_enabled = false;
		}
	}

	void Renderer::AlphaBlend(bool enable) {
		static bool is_enabled = false;

		if (enable && !is_enabled) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
			glBlendEquation(GL_FUNC_ADD);
			is_enabled = true;
		}
		else if (!enable && is_enabled) {
			glDisable(GL_BLEND);
			is_enabled = false;
		}
	}

	void Renderer::FaceCulling(bool enable) {
		static bool is_enabled = false;

		if (enable && !is_enabled) {
			glEnable(GL_CULL_FACE);
			glFrontFace(GL_CCW);
			glCullFace(GL_BACK);
			is_enabled = true;
		}
		else if (!enable && is_enabled) {
			glDisable(GL_CULL_FACE);
			is_enabled = false;
		}
	}

	void Renderer::SeamlessCubemap(bool enable) {
		static bool is_enabled = false;

		if (enable && !is_enabled) {
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
			is_enabled = true;
		}
		else if (!enable && is_enabled) {
			glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
			is_enabled = false;
		}
	}

	void Renderer::PrimitiveRestart(bool enable) {
		static bool is_enabled = false;

		if (enable && !is_enabled) {
			glEnable(GL_PRIMITIVE_RESTART);
			glPrimitiveRestartIndex(0xFFFFFF);
			is_enabled = true;
		}
		else if (!enable && is_enabled) {
			glDisable(GL_PRIMITIVE_RESTART);
			is_enabled = false;
		}
	}

	void Renderer::SetFrontFace(bool ccw) {
		glFrontFace(ccw ? GL_CCW : GL_CW);
	}

	void Renderer::SetViewport(GLuint width, GLuint height) {
		glViewport(0, 0, width, height);
	}

	void Renderer::SetShadowPass(unsigned int index) {
		// to cast shadows from multiple lights, we need multiple passes, once per light source
		shadow_index = index;  // use this to identify a specific shadow pass and light source
	}

	///////////////////////////////////////////////////////////////////////////////////////////////

	void Renderer::Attach(const std::string& title) {
	}

	void Renderer::Detach() {
		CORE_TRACE("Detaching scene \"{0}\" ......", curr_scene->title);

		last_scene = curr_scene;
		curr_scene = nullptr;

		delete last_scene;  // every object in the scene will be destructed
		last_scene = nullptr;


		Renderer::Reset();   // reset renderer to a clean default state
	}

	void Renderer::Reset() {
		// reset the rasterizer or raytracer to the default factory state
		MSAA(0);
		DepthPrepass(0);
		DepthTest(0);
		StencilTest(0);
		AlphaBlend(0);
		FaceCulling(0);
		SeamlessCubemap(0);
		PrimitiveRestart(0);
		SetFrontFace(1);

		SetShadowPass(0);
	}

	void Renderer::Clear() {


		glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
		glClearDepth(1.0f);
		glClearStencil(0);  // 8-bit integer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void Renderer::Flush() {



	}

	void Renderer::Render(const asset_ref<asset::Shader> custom_shader) {

		auto& reg = curr_scene->registry;
		auto mesh_group = reg.group<Mesh>(entt::get<Transform, Tag, Material>);
		auto model_group = reg.group<Model>(entt::get<Transform, Tag>);  // materials are managed by the model

		if (!render_queue.empty()) {
			constexpr float near_clip = 0.1f;
			constexpr float far_clip = 100.0f;

			// glm::ivec2 resolution = glm::ivec2(Window::width, Window::height);
			glm::ivec2 cursor_pos = { 0,0 };// ui::GetCursorPosition();

			//float total_time = Clock::time;
		   // float delta_time = Clock::delta_time;

			//renderer_input->SetUniform(0U, utils::val_ptr(resolution));
			renderer_input->SetUniform(1U, utils::val_ptr(cursor_pos));
			renderer_input->SetUniform(2U, utils::val_ptr(near_clip));
			renderer_input->SetUniform(3U, utils::val_ptr(far_clip));
			// renderer_input->SetUniform(4U, utils::val_ptr(total_time));
			 //renderer_input->SetUniform(5U, utils::val_ptr(delta_time));
			renderer_input->SetUniform(6U, utils::val_ptr(static_cast<int>(depth_prepass)));
			renderer_input->SetUniform(7U, utils::val_ptr(shadow_index));
		}

		while (!render_queue.empty()) {
			auto& e = render_queue.front();
			// skip null entities
			if (e == entt::null) {
				render_queue.pop();
				continue;
			}

			// entity is a native mesh
			if (mesh_group.contains(e)) {
				auto& transform = mesh_group.get<Transform>(e);
				auto& mesh = mesh_group.get<Mesh>(e);
				auto& material = mesh_group.get<Material>(e);
				auto& tag = mesh_group.get<Tag>(e);

				if (custom_shader) {
					custom_shader->SetUniform(1000U, transform.transform);
					custom_shader->SetUniform(1001U, 0U);
					custom_shader->Bind();
				}
				else {
					material.SetUniform(1000U, transform.transform);
					material.SetUniform(1001U, 0U);  // primitive mesh does not have a material id
					material.SetUniform(1002U, 0U);  // ext_1002
					material.SetUniform(1003U, 0U);  // ext_1003
					material.SetUniform(1004U, 0U);  // ext_1004
					material.SetUniform(1005U, 0U);  // ext_1005
					material.SetUniform(1006U, 0U);  // ext_1006
					material.SetUniform(1007U, 0U);  // ext_1007
					material.Bind();  // smart binding, no need to unbind
				}

				if (tag.Contains(ETag::Skybox)) {
					SetFrontFace(0);  // skybox has reversed winding order, we only draw the inner faces
					mesh.Draw();
					SetFrontFace(1);  // recover the global winding order
				}
				else {
					mesh.Draw();
				}
			}

			// entity is an imported model
			else if (model_group.contains(e)) {
				auto& transform = model_group.get<Transform>(e);
				auto& model = model_group.get<Model>(e);

				for (auto& mesh : model.meshes) {
					GLuint material_id = mesh.material_id;
					auto& material = model.materials.at(material_id);

					if (custom_shader) {
						custom_shader->SetUniform(1000U, transform.transform);
						custom_shader->SetUniform(1001U, material_id);
						custom_shader->Bind();
					}
					else {
						material.SetUniform(1000U, transform.transform);
						material.SetUniform(1001U, material_id);
						material.SetUniform(1002U, 0U);  // ext_1002
						material.SetUniform(1003U, 0U);  // ext_1003
						material.SetUniform(1004U, 0U);  // ext_1004
						material.SetUniform(1005U, 0U);  // ext_1005
						material.SetUniform(1006U, 0U);  // ext_1006
						material.SetUniform(1007U, 0U);  // ext_1007
						material.Bind();  // smart binding, no need to unbind
					}

					mesh.Draw();
				}
			}

			// a non-null entity must have either a mesh or a model component to be considered renderable
			else {
				CORE_ERROR("Entity {0} in the render list is non-renderable!", e);
				Clear();  // in this case just show a deep blue screen (UI stuff is separate)
			}

			render_queue.pop();
		}
	}

	void Renderer::DrawScene() {
		curr_scene->OnSceneRender();
	}

	void Renderer::DrawImGui() {
		curr_scene->OnImGuiRender();
	}

}
