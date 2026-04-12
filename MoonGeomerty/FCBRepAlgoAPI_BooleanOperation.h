#pragma once
#include <BRepAlgoAPI_BooleanOperation.hxx>

class FCBRepAlgoAPIHelper
{
public:
    static void setAutoFuzzy(BRepAlgoAPI_BooleanOperation* op);
    static void setAutoFuzzy(BRepAlgoAPI_BuilderAlgo* op);
};

class FCBRepAlgoAPI_BooleanOperation : public BRepAlgoAPI_BooleanOperation
{
public:

    DEFINE_STANDARD_ALLOC

    //! Empty constructor
    Standard_EXPORT FCBRepAlgoAPI_BooleanOperation();

    // set fuzzyness based on size
    void setAutoFuzzy();

protected: //! @name Constructors

  //! Constructor to perform Boolean operation on only two arguments.
  //! Obsolete
  Standard_EXPORT FCBRepAlgoAPI_BooleanOperation(const TopoDS_Shape& theS1,
                                               const TopoDS_Shape& theS2,
                                               const BOPAlgo_Operation theOperation);

};