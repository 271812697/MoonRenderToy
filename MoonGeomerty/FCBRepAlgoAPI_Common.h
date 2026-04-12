#pragma once
#include <BRepAlgoAPI_Common.hxx>
#include "FCBRepAlgoAPI_BooleanOperation.h"


class FCBRepAlgoAPI_Common : public FCBRepAlgoAPI_BooleanOperation
{
public:

    DEFINE_STANDARD_ALLOC
  
    //! Empty constructor
    Standard_EXPORT FCBRepAlgoAPI_Common();
  
    //! Constructor with two shapes
    //! <S1>  -argument
    //! <S2>  -tool
    //! <anOperation> - the type of the operation
    Standard_EXPORT FCBRepAlgoAPI_Common(const TopoDS_Shape& S1,
                                     const TopoDS_Shape& S2);

};