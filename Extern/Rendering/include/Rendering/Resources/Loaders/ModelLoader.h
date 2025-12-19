#pragma once
#include <string>
#include "Rendering/Resources/Model.h"
#include "Rendering/Resources/Parsers/AssimpParser.h"
#include "Maths/FVector2.h"

namespace Rendering::Resources::Loaders
{
	/**
	* Handle the Model creation and destruction
	*/
	class ModelLoader
	{
	public:
		/**
		* Disabled constructor
		*/
		ModelLoader() = delete;

		/**
		* Create a model
		* @param p_filepath
		* @param p_parserFlags
		*/
		static Model* Create(const std::string& p_filepath, Parsers::EModelParserFlags p_parserFlags = Parsers::EModelParserFlags::NONE);

		/**
		* Reload a model from file
		* @param p_model
		* @param p_filePath
		* @param p_parserFlags
		*/
		static void Reload(Model& p_model, const std::string& p_filePath, Parsers::EModelParserFlags p_parserFlags = Parsers::EModelParserFlags::NONE);

		/**
		* Disabled constructor
		* @param p_modelInstance
		*/
		static bool Destroy(Model*& p_modelInstance);
		static Model* LoadFromMemory(const std::vector<float>& v, const std::vector<unsigned int>& i);
		static Model* LoadFromMemory(const std::vector<Maths::FVector3>& vertex, const std::vector<unsigned int>& i);
		static Model* LoadFromMemory(const std::vector<Maths::FVector3>& vertex, const std::vector<Maths::FVector3>& normal, const std::vector<unsigned int>& i);
		static Model* LoadFromMemory(const std::vector<Maths::FVector3>& vertex, const std::vector<Maths::FVector3>& normal, const std::vector<Maths::FVector2>& uv, const std::vector<unsigned int>& i);
		static Model* LoadFromMemory(const std::vector<Maths::FVector3>& vertex, const std::vector<Maths::FVector2>& uv, const std::vector<unsigned int>& i);
	private:
		static Parsers::AssimpParser __ASSIMP;
	};
}