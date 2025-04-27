

#include <Core/Helpers/GUIDrawer.h>

#include <Rendering/Settings/ETextureFilteringMode.h>

#include <tools/PathParser.h>

#include "EditorResources.h"
//#include "Editor/Resources/RawTextures.h"
#include "RawShaders.h"

Editor::Core::EditorResources::EditorResources(const std::string& p_editorAssetsPath)
{
	using namespace Rendering::Resources::Loaders;

	std::string modelsFolder	= p_editorAssetsPath + "/Models/";


	Rendering::Resources::Parsers::EModelParserFlags modelParserFlags = Rendering::Resources::Parsers::EModelParserFlags::NONE;


	modelParserFlags |= Rendering::Resources::Parsers::EModelParserFlags::TRIANGULATE;
	modelParserFlags |= Rendering::Resources::Parsers::EModelParserFlags::GEN_SMOOTH_NORMALS;
	modelParserFlags |= Rendering::Resources::Parsers::EModelParserFlags::OPTIMIZE_MESHES;
	//modelParserFlags |= Rendering::Resources::Parsers::EModelParserFlags::OPTIMIZE_GRAPH;
	modelParserFlags |= Rendering::Resources::Parsers::EModelParserFlags::FIND_INSTANCES;
	modelParserFlags |= Rendering::Resources::Parsers::EModelParserFlags::CALC_TANGENT_SPACE;
	modelParserFlags |= Rendering::Resources::Parsers::EModelParserFlags::JOIN_IDENTICAL_VERTICES;
	modelParserFlags |= Rendering::Resources::Parsers::EModelParserFlags::DEBONE;
	modelParserFlags |= Rendering::Resources::Parsers::EModelParserFlags::FIND_INVALID_DATA;
	modelParserFlags |= Rendering::Resources::Parsers::EModelParserFlags::IMPROVE_CACHE_LOCALITY;
	modelParserFlags |= Rendering::Resources::Parsers::EModelParserFlags::GEN_UV_COORDS;
	modelParserFlags |= Rendering::Resources::Parsers::EModelParserFlags::PRE_TRANSFORM_VERTICES;
	modelParserFlags |= Rendering::Resources::Parsers::EModelParserFlags::GLOBAL_SCALE;

	Rendering::Settings::ETextureFilteringMode firstFilterEditor = Rendering::Settings::ETextureFilteringMode::LINEAR;
	Rendering::Settings::ETextureFilteringMode secondFilterEditor = Rendering::Settings::ETextureFilteringMode::LINEAR;

	Rendering::Settings::ETextureFilteringMode firstFilterBillboard = Rendering::Settings::ETextureFilteringMode::NEAREST;
	Rendering::Settings::ETextureFilteringMode secondFilterBillboard = Rendering::Settings::ETextureFilteringMode::NEAREST;

	

	/* Models */
	m_models["Cube"]			= ModelLoader::Create(modelsFolder + "Cube.fbx", modelParserFlags);
	m_models["Cylinder"]		= ModelLoader::Create(modelsFolder + "Cylinder.fbx", modelParserFlags);
	m_models["Plane"]			= ModelLoader::Create(modelsFolder + "Plane.fbx", modelParserFlags);
	m_models["Vertical_Plane"]	= ModelLoader::Create(modelsFolder + "Vertical_Plane.fbx", modelParserFlags);
	m_models["Roll"]			= ModelLoader::Create(modelsFolder + "Roll.fbx", modelParserFlags);
	m_models["Sphere"]			= ModelLoader::Create(modelsFolder + "Sphere.fbx", modelParserFlags);
	m_models["Arrow_Translate"]	= ModelLoader::Create(modelsFolder + "Arrow_Translate.fbx", modelParserFlags);
	m_models["Arrow_Rotate"]	= ModelLoader::Create(modelsFolder + "Arrow_Rotate.fbx", modelParserFlags);
	m_models["Arrow_Scale"]		= ModelLoader::Create(modelsFolder + "Arrow_Scale.fbx", modelParserFlags);
	m_models["Arrow_Picking"]	= ModelLoader::Create(modelsFolder + "Arrow_Picking.fbx", modelParserFlags);
	m_models["Camera"]			= ModelLoader::Create(modelsFolder + "Camera.fbx", modelParserFlags);

	/* Shaders */
	auto gridSource = Editor::Resources::RawShaders::GetGrid();
	auto gizmoSource = Editor::Resources::RawShaders::GetGizmo();
	auto billboardSource = Editor::Resources::RawShaders::GetBillboard();
	m_shaders["Grid"] = ShaderLoader::CreateFromSource(gridSource.first, gridSource.second);
	m_shaders["Gizmo"] = ShaderLoader::CreateFromSource(gizmoSource.first, gizmoSource.second);
	m_shaders["Billboard"] = ShaderLoader::CreateFromSource(billboardSource.first, billboardSource.second);



}

Editor::Core::EditorResources::~EditorResources()
{
	for (auto[id, texture] : m_textures)
		Rendering::Resources::Loaders::TextureLoader::Destroy(texture);

	for (auto [id, mesh] : m_models)
		Rendering::Resources::Loaders::ModelLoader::Destroy(mesh);

	for (auto [id, shader] : m_shaders)
		Rendering::Resources::Loaders::ShaderLoader::Destroy(shader);
}

Rendering::Resources::Texture* Editor::Core::EditorResources::GetFileIcon(const std::string& p_filename)
{
	using namespace Tools::Utils;
	return GetTexture("Icon_" + PathParser::FileTypeToString(PathParser::GetFileType(p_filename)));
}

Rendering::Resources::Texture* Editor::Core::EditorResources::GetTexture(const std::string& p_id)
{
	if (m_textures.find(p_id) != m_textures.end())
		return m_textures.at(p_id);

	return nullptr;
}

Rendering::Resources::Model* Editor::Core::EditorResources::GetModel(const std::string& p_id)
{
	if (m_models.find(p_id) != m_models.end())
		return m_models.at(p_id);

	return nullptr;
}

Rendering::Resources::Shader* Editor::Core::EditorResources::GetShader(const std::string& p_id)
{
	if (m_shaders.find(p_id) != m_shaders.end())
		return m_shaders.at(p_id);

	return nullptr;
}
