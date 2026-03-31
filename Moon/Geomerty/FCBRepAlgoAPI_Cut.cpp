

#include "FCBRepAlgoAPI_Cut.h"
#include <TopoDS_Shape.hxx>

FCBRepAlgoAPI_Cut::FCBRepAlgoAPI_Cut()
{
  myOperation=BOPAlgo_CUT;
}

FCBRepAlgoAPI_Cut::FCBRepAlgoAPI_Cut(const TopoDS_Shape& S1, 
                                       const TopoDS_Shape& S2)
: FCBRepAlgoAPI_BooleanOperation(S1, S2, BOPAlgo_CUT)
{
  Build();
}
