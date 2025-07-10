#ifdef SDLWindow
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
#else
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <filament/Camera.h>
#include <filament/Engine.h>
#include <../src/details/Engine.h>
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
#include <filameshio/MeshReader.h>
#include <generated/resources/resources.h>
#include "generated/resources/monkey.h"
#include <backend/platforms/PlatformGlfwGL.h>
#include <camutils/Manipulator.h>


struct App {

	utils::Entity light;
	filament::Material* material;
	filament::MaterialInstance* materialInstance;
	filamesh::MeshReader::Mesh mesh;
	filament::Camera* cam;
	filament::camutils::Manipulator<float>* mMainCameraMan;
	filament::math::mat4f transform;
};
int main()
{
	App app;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(1280, 800, "FilamentTest", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	filament::backend::PlatformGlfwGL* platform = new filament::backend::PlatformGlfwGL();
	auto createEngine = [&]() {
		auto backend = filament::Engine::Backend::OPENGL;
		filament::Engine::Config engineConfig = {};
		engineConfig.stereoscopicEyeCount = 2;
		engineConfig.stereoscopicType = filament::Engine::StereoscopicType::NONE;
		return filament::Engine::Builder()
			.backend(backend)
			.featureLevel(filament::backend::FeatureLevel::FEATURE_LEVEL_3)
			.config(&engineConfig).platform(platform)
			.build();
		};
	auto engine = createEngine();
	auto renderer = engine->createRenderer();
	auto scene = engine->createScene();
	auto view = engine->createView();
	auto swapchain = engine->createSwapChain(window, 0);
	view->setName("Main View");

	auto setup = [&]() {
		auto skybox = filament::Skybox::Builder().color({ 0.1, 0.125, 0.25, 1.0 }).build(*engine);
		scene->setSkybox(skybox);
		view->setPostProcessingEnabled(true);
		auto& tcm = engine->getTransformManager();
		auto& rcm = engine->getRenderableManager();
		auto& em = utils::EntityManager::get();
		app.mMainCameraMan = filament::camutils::Manipulator<float>::Builder()
			.targetPosition(0, 0, 0)
			.flightMoveDamping(15.0)
			.build(filament::camutils::Mode::ORBIT);
		app.material = filament::Material::Builder()
			.package(RESOURCES_AIDEFAULTMAT_DATA, RESOURCES_AIDEFAULTMAT_SIZE).build(*engine);
		auto mi = app.materialInstance = app.material->createInstance();
		mi->setParameter("baseColor", filament::RgbType::LINEAR, filament::math::float3{ 0.8 });
		mi->setParameter("metallic", 1.0f);
		mi->setParameter("roughness", 0.4f);
		mi->setParameter("reflectance", 0.5f);
		app.mesh = filamesh::MeshReader::loadMeshFromBuffer(engine, MONKEY_SUZANNE_DATA, nullptr, nullptr, mi);
		auto ti = tcm.getInstance(app.mesh.renderable);
		app.transform = filament::math::mat4f{ filament::math::mat3f(1), filament::math::float3(0, 0, -4) } *tcm.getWorldTransform(ti);
		rcm.setCastShadows(rcm.getInstance(app.mesh.renderable), false);
		scene->addEntity(app.mesh.renderable);

		// Add light sources into the scene.
		app.light = em.create();
		filament::LightManager::Builder(filament::LightManager::Type::SUN)
			.color(filament::Color::toLinear<filament::ACCURATE>(filament::sRGBColor(0.98f, 0.92f, 0.89f)))
			.intensity(110000)
			.direction({ 0.7, -1, -0.8 })
			.sunAngularRadius(1.9f)
			.castShadows(false)
			.build(*engine, app.light);
		scene->addEntity(app.light);
		//scene->addEntity(renderable);
		auto camera = utils::EntityManager::get().create();
		app.cam = engine->createCamera(camera);
		view->setCamera(app.cam);

		};
	setup();
	auto animate = [&app](filament::Engine* engine, filament::View* view, double now) {

		auto& tcm = engine->getTransformManager();
		auto ti = tcm.getInstance(app.mesh.renderable);
		tcm.setTransform(ti, app.transform * filament::math::mat4f::rotation(now, filament::math::float3{ 0, 1, 0 }));
		filament::math::float3 eye, center, up;
		app.mMainCameraMan->getLookAt(&eye, &center, &up);
		app.cam->lookAt(eye, center, up);
		constexpr float ZOOM = 1.5f;
		const uint32_t w = view->getViewport().width;
		const uint32_t h = view->getViewport().height;
		const float aspect = (float)w / h;
		app.cam->setProjection(filament::Camera::Projection::ORTHO,
			-aspect * ZOOM, aspect * ZOOM,
			-ZOOM, ZOOM, 0, 1000);
		};
	view->setScene(scene);
	view->setViewport({ 0,0,1280,800 });
	while (!glfwWindowShouldClose(window))
	{

		engine->execute();
		static double t = 0.0f;
		t += 0.0016;
		animate(engine, view, t);
		bool flag = renderer->beginFrame(swapchain);
		renderer->render(view);
		renderer->endFrame();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}
#endif // SDLWindow


