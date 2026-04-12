

#include "FCBRepAlgoAPI_Fuse.h"<>
#include <TopoDS_Shape.hxx>

FCBRepAlgoAPI_Fuse::FCBRepAlgoAPI_Fuse()
{
  myOperation=BOPAlgo_FUSE;
}

FCBRepAlgoAPI_Fuse::FCBRepAlgoAPI_Fuse(const TopoDS_Shape& S1, 
                                       const TopoDS_Shape& S2)
: FCBRepAlgoAPI_BooleanOperation(S1, S2, BOPAlgo_FUSE)
{
  Build();
}
