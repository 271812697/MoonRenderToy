#pragma once
#include <vector>
namespace Core::ECS
{
    class Actor;
}
namespace Part {
    class TopoShape;
}
namespace MOON
{
    class ViewTool {
    public:
        static Core::ECS::Actor* getLastestActorSelected();
        static bool getSelectedTopoShape(std::vector<Part::TopoShape>&topo);
        static bool createTopoActor(const Part::TopoShape& topo,const char* name=nullptr);
    };
}