

#include "FCBRepAlgoAPI_Common.h"
#include <TopoDS_Shape.hxx>

FCBRepAlgoAPI_Common::FCBRepAlgoAPI_Common()
{
  myOperation=BOPAlgo_COMMON;
}

FCBRepAlgoAPI_Common::FCBRepAlgoAPI_Common(const TopoDS_Shape& S1, 
                                       const TopoDS_Shape& S2)
: FCBRepAlgoAPI_BooleanOperation(S1, S2, BOPAlgo_COMMON)
{
  Build();
}
