#pragma once
#include <BRepAlgoAPI_Section.hxx>
#include <BRepBuilderAPI_Command.hxx>
#include "FCBRepAlgoAPI_BooleanOperation.h"


class FCBRepAlgoAPI_Section : public BRepAlgoAPI_Section
{
public:

    DEFINE_STANDARD_ALLOC

  
    //! Empty constructor
    Standard_EXPORT FCBRepAlgoAPI_Section();
  
    //! Constructor with two shapes
    //! <S1>  -argument
    //! <S2>  -tool
    //! <PerformNow> - the flag:
    //! if <PerformNow>=True - the algorithm is performed immediately
    //! Obsolete
    Standard_EXPORT FCBRepAlgoAPI_Section(const TopoDS_Shape& S1, const TopoDS_Shape& S2, const Standard_Boolean PerformNow = Standard_True);

    //! Constructor with two shapes
    //! <S1>  - argument
    //! <Pl>  - tool
    //! <PerformNow> - the flag:
    //! if <PerformNow>=True - the algorithm is performed immediately
    //! Obsolete
    Standard_EXPORT FCBRepAlgoAPI_Section(const TopoDS_Shape& S1, const gp_Pln& Pl, const Standard_Boolean PerformNow = Standard_True);

    // set fuzzyness based on size
    void setAutoFuzzy();
};