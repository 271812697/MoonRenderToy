#include <Rendering/Data/FeatureSet.h>

namespace Rendering::Data
{
	size_t FeatureSetHash::operator()(const FeatureSet& fs) const
	{
		size_t hash = 0;

		for (const auto& feature : fs)
		{
			hash ^= std::hash<std::string>{}(feature);
		}

		return hash;
	}

	bool FeatureSetEqual::operator()(const FeatureSet& lhs, const FeatureSet& rhs) const
	{
		return lhs == rhs;
	};
}

Rendering::Data::FeatureSet operator+(const Rendering::Data::FeatureSet& p_lhs, const std::string& p_feature)
{
	Rendering::Data::FeatureSet result = p_lhs;
	result.insert(p_feature);
	return result;
}

Rendering::Data::FeatureSet operator-(const Rendering::Data::FeatureSet& p_lhs, const std::string& p_feature)
{
	Rendering::Data::FeatureSet result = p_lhs;
	result.erase(p_feature);
	return result;
}
