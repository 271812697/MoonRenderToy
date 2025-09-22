#pragma once
#include <string>
#include <unordered_map>
#include <vector>


namespace filament {
class ShaderPassInfo
{
public:
    ShaderPassInfo();
    void addPass(const std::string& passName, const std::unordered_map<std::string, std::vector<std::string>>& features);
    std::vector<std::string>& getPasses();
    std::vector<std::unordered_map<std::string, std::vector<std::string>>>& getFeatures();
    int getPassCount();
    int getPassId(const std::string& passName);
    int getFeatureId(const std::string& passName,const std::string&stage,const std::string& featureName);
    int getVariant(const std::string& passName, const std::string& stage, const std::vector<std::string>& features = {});
    int getFeaturesCount(int passId, const std::string& stage);
    void clear();
private:

    std::vector<std::string> mPass;
    std::unordered_map<std::string, int> mPassToIndex;

    std::vector<std::unordered_map<std::string,std::vector<std::string>>> mFeatures;
    std::unordered_map < int, std::unordered_map<std::string, std::unordered_map<std::string, int>>> mFeatureToIndex;
};

} // namespace filament

