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
        static Feature* getSelectedFeature();
        static bool getSelectedBasedFeature(Feature*& f, std::vector<std::string>& subValues);
        static Core::ECS::Actor* createTopoActor(const Part::TopoShape& topo,const char* name=nullptr);
    };
}