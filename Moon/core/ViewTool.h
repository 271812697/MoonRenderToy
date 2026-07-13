#pragma once
#include <vector>
#include <string>
namespace Core::ECS
{
    class Actor;
}
namespace Part {
    class TopoShape;
}
namespace MOON
{
    class Feature;
    class ViewTool {
    public:
        static Core::ECS::Actor* getLastestActorSelected();
        static bool getSelectedTopoShape(std::vector<Part::TopoShape>&topo);
        static bool getSelectedFeature(Feature*& feature,std::vector<std::string>&subValue);
        static Core::ECS::Actor* createTopoActor(const Part::TopoShape& topo,const char* name=nullptr);
    };
}