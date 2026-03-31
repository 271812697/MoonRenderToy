

#include "FCBRepAlgoAPI_Section.h"
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <TopoDS_Shape.hxx>
#include <Precision.hxx>
//#include <FuzzyHelper.h>

FCBRepAlgoAPI_Section::FCBRepAlgoAPI_Section()
{
    SetRunParallel(Standard_True);
    SetNonDestructive(Standard_True);
}

FCBRepAlgoAPI_Section::FCBRepAlgoAPI_Section(const TopoDS_Shape& S1, const TopoDS_Shape& S2, const Standard_Boolean PerformNow)
: BRepAlgoAPI_Section(S1,S2,false) 
{
    if (!BRepCheck_Analyzer(S1).IsValid()){
        Standard_ConstructionError::Raise("Base shape is not valid for boolean operation");
    }
    if (! BRepCheck_Analyzer(S2).IsValid()){
        Standard_ConstructionError::Raise("Tool shape is not valid for boolean operation");
    }
    setAutoFuzzy();
    SetRunParallel(Standard_True);
    SetNonDestructive(Standard_True);
    if (PerformNow) Build();
}

FCBRepAlgoAPI_Section::FCBRepAlgoAPI_Section
(const TopoDS_Shape&    Sh,
const gp_Pln&          Pl,
const Standard_Boolean PerformNow)
: 
BRepAlgoAPI_Section(Sh,Pl,false) 
{
    if (!BRepCheck_Analyzer(Sh).IsValid()){
        Standard_ConstructionError::Raise("Base shape is not valid for boolean operation");
    }
    setAutoFuzzy();
    SetRunParallel(Standard_True);
    if (PerformNow) Build();
}


void FCBRepAlgoAPI_Section::setAutoFuzzy()
{
    FCBRepAlgoAPIHelper::setAutoFuzzy(this);
}
