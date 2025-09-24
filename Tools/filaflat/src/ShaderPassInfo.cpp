
#include "filaflat/ShaderPassInfo.h"
namespace filament
{
ShaderPassInfo::ShaderPassInfo()
{
}
void ShaderPassInfo::addPass(const std::string& passName, const std::unordered_map<std::string, std::vector<std::string>>& features)
{
    int passId = mPass.size();
    mPass.push_back(passName);
    mPassToIndex[passName] = passId;
    for( const auto& it1 : features )
    {
        mFeatureToIndex[passId];
        for( int i = 0; i < it1.second.size();i++ )
        {
            mFeatureToIndex[passId][it1.first][it1.second[i]] = i;
        }
    }
    mFeatures.push_back(features);
}
std::vector<std::string>& ShaderPassInfo::getPasses()
{
    return mPass;
}
int ShaderPassInfo::getPassCount()
{
    return mPass.size();
}
int ShaderPassInfo::getPassId(const std::string& passName)
{    auto it = mPassToIndex.end();    if (it == mPassToIndex.end()) {        return -1;    }
    return mPassToIndex[passName];
}
int ShaderPassInfo::getFeatureId(const std::string& passName,const std::string& stage  , const std::string& featureName)
{
    return mFeatureToIndex[mPassToIndex[passName]][stage][featureName];
   
}
int ShaderPassInfo::getVariant(const std::string& passName, const std::string& stage, const std::vector<std::string>& features)
{

    int passId = mPassToIndex[passName];
    if( mFeatureToIndex[passId].find(stage) == mFeatureToIndex[passId].end() )
    {
        return 0;
    }
    int ans = 0;
    for( int i = 0; i < features.size(); i++ )
    {
        if( mFeatureToIndex[passId][stage].find(features[i]) != mFeatureToIndex[passId][stage].end() )
        {
            ans|=(1<<mFeatureToIndex[passId][stage][features[i]]);        
        }
    }
    return ans;
}
int ShaderPassInfo::getFeaturesCount(int passId, const std::string& stage)
{
    return mFeatures[passId][stage].size();
}
std::vector<std::unordered_map<std::string, std::vector<std::string>>>& ShaderPassInfo::getFeatures()
{
    return mFeatures;
}
void ShaderPassInfo::clear()
{
    mPass.clear();
    mFeatures.clear();
}
} // namespace filament
