/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Debug/Assertion.h>
#include <Rendering/Resources/Shader.h>

namespace
{
	void ValidateProgramRegistry(const Rendering::Resources::Shader::ProgramVariants& p_programs)
	{
		ASSERT(p_programs.size() > 0 && p_programs.contains({}),
			"Shader program registry must contain at least a default program"
		);
	}
}

size_t Rendering::Resources::Shader::FeatureSetHash::operator()(const FeatureSet& fs) const
{
	size_t hash = 0;

	for (const auto& feature : fs)
	{
		hash ^= std::hash<std::string>{}(feature)+0x9e3779b9 + (hash << 6) + (hash >> 2);
	}

	return hash;
}

bool Rendering::Resources::Shader::FeatureSetEqual::operator()(const FeatureSet& lhs, const FeatureSet& rhs) const
{
	return lhs == rhs;
};

Rendering::HAL::ShaderProgram& Rendering::Resources::Shader::GetProgram(const FeatureSet& p_featureSet)
{
	if (m_programs.contains(p_featureSet))
	{
		return *m_programs[p_featureSet];
	}
	else
	{
		ASSERT(m_programs.contains({}), "No default program found for this shader");
		return *m_programs[{}];
	}
}

const Rendering::Resources::Shader::FeatureSet& Rendering::Resources::Shader::GetFeatures() const
{
	return m_features;
}

Rendering::Resources::Shader::Shader(
	const std::string p_path,
	ProgramVariants&& p_program
) : path(p_path)
{
	SetPrograms(std::move(p_program));
}

void Rendering::Resources::Shader::SetPrograms(ProgramVariants&& p_programs)
{
	ValidateProgramRegistry(p_programs);
	m_programs = std::move(p_programs);

	m_features.clear();

	// Find all features based on the compiled programs
	for (const auto& [key, _] : m_programs)
	{
		m_features.insert(key.begin(), key.end());
	}
}
