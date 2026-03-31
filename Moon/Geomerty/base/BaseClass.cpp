#include <cassert>

#include "BaseClass.h"
using namespace Base;

Type BaseClass::classTypeId = Base::Type::badType();  // NOLINT


//**************************************************************************
// Construction/Destruction

/**
 * A constructor.
 * A more elaborate description of the constructor.
 */
BaseClass::BaseClass() = default;

/**
 * A destructor.
 * A more elaborate description of the destructor.
 */
BaseClass::~BaseClass() = default;


//**************************************************************************
// separator for other implementation aspects

void BaseClass::init()
{
    assert(BaseClass::classTypeId == Type::badType() && "don't init() twice!");
    /* Make sure superclass gets initialized before subclass. */
    /*assert(strcmp(#_parentclass_), "inherited"));*/
    /*Type parentType(Type::fromName(#_parentclass_));*/
    /*assert(parentType != Type::badType() && "you forgot init() on parentclass!");*/

    /* Set up entry in the type system. */
    BaseClass::classTypeId =
        Type::createType(Type::badType(), "Base::BaseClass", BaseClass::create);
}

Type BaseClass::getClassTypeId()
{
    return BaseClass::classTypeId;
}

Type BaseClass::getTypeId() const
{
    return BaseClass::classTypeId;
}


void BaseClass::initSubclass(Base::Type& toInit,
                             const char* ClassName,
                             const char* ParentName,
                             Type::instantiationMethod method)
{
    // don't init twice!
    assert(toInit == Base::Type::badType());
    // get the parent class
    Base::Type parentType(Base::Type::fromName(ParentName));
    // forgot init parent!
    assert(parentType != Base::Type::badType());

    // create the new type
    toInit = Base::Type::createType(parentType, ClassName, method);
}

