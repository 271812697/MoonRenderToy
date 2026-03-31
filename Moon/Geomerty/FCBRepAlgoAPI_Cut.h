#pragma once
#include <BRepAlgoAPI_Cut.hxx>
#include "FCBRepAlgoAPI_BooleanOperation.h"


class FCBRepAlgoAPI_Cut : public FCBRepAlgoAPI_BooleanOperation
{
public:

    DEFINE_STANDARD_ALLOC
  
    //! Empty constructor
    Standard_EXPORT FCBRepAlgoAPI_Cut();
  
    //! Constructor with two shapes
    //! <S1>  -argument
    //! <S2>  -tool
    //! <anOperation> - the type of the operation
    Standard_EXPORT FCBRepAlgoAPI_Cut(const TopoDS_Shape& S1,
                                     const TopoDS_Shape& S2);

};