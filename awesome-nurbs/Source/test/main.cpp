#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <filament/Camera.h>
#include <filament/Engine.h>
#include "../src/details/Engine.h"
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Renderer.h>
#include <filament/Viewport.h>
#include <utils/EntityManager.h>
#include "generated/resources/resources.h"
#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

#include <utils/EntityManager.h>
#include "Device.h"
#include "WindowSettings.h"
#include "Window.h"
#include "InputManager.h"


using namespace filament;
using utils::Entity;
using utils::EntityManager;

struct App {
	Config config;
	VertexBuffer* vb;
	IndexBuffer* ib;
	Material* mat;
	Camera* cam;
	Entity camera;
	Skybox* skybox;
	Entity renderable;
};

struct Vertex {
	filament::math::float2 position;
	uint32_t color;
};

static const Vertex TRIANGLE_VERTICES[3] = {
	{{1, 0}, 0xffff0000u},
	{{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u},
	{{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu},
};

static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };
int main(int argc, char* argv[])
{
	App app{};
	app.config.title = "hellotriangle";
	app.config.featureLevel = backend::FeatureLevel::FEATURE_LEVEL_0;
	app.config.backend = filament::Engine::Backend::OPENGL;


	auto setup = [&app](Engine* engine, View* view, Scene* scene) {
		app.skybox = Skybox::Builder().color({ 0.1, 0.125, 0.25, 1.0 }).build(*engine);
		scene->setSkybox(app.skybox);
		view->setPostProcessingEnabled(false);
		static_assert(sizeof(Vertex) == 12, "Strange vertex size.");
		app.vb = VertexBuffer::Builder()
			.vertexCount(3)
			.bufferCount(1)
			.attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
			.attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
			.normalized(VertexAttribute::COLOR)
			.build(*engine);
		app.vb->setBufferAt(*engine, 0,
			VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr));
		app.ib = IndexBuffer::Builder()
			.indexCount(3)
			.bufferType(IndexBuffer::IndexType::USHORT)
			.build(*engine);
		app.ib->setBuffer(*engine,
			IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));
		app.mat = Material::Builder()
			.package(RESOURCES_BAKEDCOLOR_DATA, RESOURCES_BAKEDCOLOR_SIZE)
			.build(*engine);
		app.renderable = EntityManager::get().create();
		RenderableManager::Builder(1)
			.boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
			.material(0, app.mat->getDefaultInstance())
			.geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0, 3)
			.culling(false)
			.receiveShadows(false)
			.castShadows(false)
			.build(*engine, app.renderable);
		scene->addEntity(app.renderable);
		app.camera = utils::EntityManager::get().create();
		app.cam = engine->createCamera(app.camera);
		view->setCamera(app.cam);
		};


	auto cleanup = [&app](Engine* engine, View*, Scene*) {
		engine->destroy(app.skybox);
		engine->destroy(app.renderable);
		engine->destroy(app.mat);
		engine->destroy(app.vb);
		engine->destroy(app.ib);
		engine->destroyCameraComponent(app.camera);
		utils::EntityManager::get().destroy(app.camera);
		};
	FilamentApp::get().animate([&app](Engine* engine, View* view, double now) {
		constexpr float ZOOM = 1.5f;
		const uint32_t w = view->getViewport().width;
		const uint32_t h = view->getViewport().height;
		const float aspect = (float)w / h;
		app.cam->setProjection(Camera::Projection::ORTHO,
			-aspect * ZOOM, aspect * ZOOM,
			-ZOOM, ZOOM, 0, 1);
		auto& tcm = engine->getTransformManager();
		tcm.setTransform(tcm.getInstance(app.renderable),
			filament::math::mat4f::rotation(now, filament::math::float3{ 0, 0, 1 }));
		});

	FilamentApp::get().run(app.config, setup, cleanup);
	return 0;
}

