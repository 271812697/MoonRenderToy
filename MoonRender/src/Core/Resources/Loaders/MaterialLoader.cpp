#include <tinyxml2.h>
#include <Core/Resources/Loaders/MaterialLoader.h>
#include <iostream>


Core::Resources::Material * Core::Resources::Loaders::MaterialLoader::Create(const std::string & p_path)
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile(p_path.c_str());
	if (!doc.Error())
	{
		tinyxml2::XMLNode* root = doc.FirstChild();

		Material* material = new Material();

		material->OnDeserialize(doc, root);

		return material;
	}
	else
	{
		return nullptr;
	}
}

void Core::Resources::Loaders::MaterialLoader::Reload(Material& p_material, const std::string& p_path)
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile(p_path.c_str());
	if (!doc.Error())
	{
		tinyxml2::XMLNode* root = doc.FirstChild();
		p_material.OnDeserialize(doc, root);
		std::cout << ("[MATERIAL] \"" + p_path + "\" Reloaded") << std::endl;
	}
}

void Core::Resources::Loaders::MaterialLoader::Save(Material& p_material, const std::string& p_path)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLNode* node = doc.NewElement("root");
	doc.InsertFirstChild(node);

	p_material.OnSerialize(doc, node);

	if (doc.SaveFile(p_path.c_str()) == tinyxml2::XML_SUCCESS)
		std::cout << ("[MATERIAL] \"" + p_path + "\": Saved") << std::endl;
	else
	{
		std::cout << ("[MATERIAL] \"" + p_path + "\": Failed to save") << std::endl;;
	}
		
}

bool Core::Resources::Loaders::MaterialLoader::Destroy(Material *& p_material)
{
	if (p_material)
	{
		delete p_material;
		p_material = nullptr;

		return true;
	}

	return false;
}