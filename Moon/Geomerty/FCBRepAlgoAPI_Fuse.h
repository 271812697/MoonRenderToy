#pragma once
#include <BRepAlgoAPI_Fuse.hxx>
#include "FCBRepAlgoAPI_BooleanOperation.h"


class FCBRepAlgoAPI_Fuse : public FCBRepAlgoAPI_BooleanOperation
{
public:

    DEFINE_STANDARD_ALLOC

  
    //! Empty constructor
    Standard_EXPORT FCBRepAlgoAPI_Fuse();
  
    //! Constructor with two shapes
    //! <S1>  -argument
    //! <S2>  -tool
    //! <anOperation> - the type of the operation
    Standard_EXPORT FCBRepAlgoAPI_Fuse(const TopoDS_Shape& S1,
                                     const TopoDS_Shape& S2);

};