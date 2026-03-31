#pragma once
#include "Type.h"


// NOLINTBEGIN(cppcoreguidelines-macro-usage)
/// define for subclassing Base::BaseClass
#define TYPESYSTEM_HEADER()                                                                        \
public:                                                                                            \
    static Base::Type getClassTypeId(void);                                                        \
    virtual Base::Type getTypeId(void) const;                                                      \
    static void init(void);                                                                        \
    static void* create(void);                                                                     \
                                                                                                   \
private:                                                                                           \
    static Base::Type classTypeId


/// Like TYPESYSTEM_HEADER, but declare getTypeId as 'override'
#define TYPESYSTEM_HEADER_WITH_OVERRIDE()                                                          \
public:                                                                                            \
    static Base::Type getClassTypeId(void);                                                        \
    Base::Type getTypeId(void) const override;                                                     \
    static void init(void);                                                                        \
    static void* create(void);                                                                     \
                                                                                                   \
private:                                                                                           \
    static Base::Type classTypeId


/// define to implement a  subclass of Base::BaseClass
#define TYPESYSTEM_SOURCE_P(_class_)                                                               \
    Base::Type _class_::getClassTypeId(void)                                                       \
    {                                                                                              \
        return _class_::classTypeId;                                                               \
    }                                                                                              \
    Base::Type _class_::getTypeId(void) const                                                      \
    {                                                                                              \
        return _class_::classTypeId;                                                               \
    }                                                                                              \
    Base::Type _class_::classTypeId = Base::Type::badType();                                       \
    void* _class_::create(void)                                                                    \
    {                                                                                              \
        return new _class_();                                                                      \
    }

/// define to implement a  subclass of Base::BaseClass
#define TYPESYSTEM_SOURCE_TEMPLATE_P(_class_)                                                      \
    template<>                                                                                     \
    Base::Type _class_::getClassTypeId(void)                                                       \
    {                                                                                              \
        return _class_::classTypeId;                                                               \
    }                                                                                              \
    template<>                                                                                     \
    Base::Type _class_::getTypeId(void) const                                                      \
    {                                                                                              \
        return _class_::classTypeId;                                                               \
    }                                                                                              \
    template<>                                                                                     \
    void* _class_::create(void)                                                                    \
    {                                                                                              \
        return new _class_();                                                                      \
    }

/// define to implement a  subclass of Base::BaseClass
#define TYPESYSTEM_SOURCE_ABSTRACT_P(_class_)                                                      \
    Base::Type _class_::getClassTypeId(void)                                                       \
    {                                                                                              \
        return _class_::classTypeId;                                                               \
    }                                                                                              \
    Base::Type _class_::getTypeId(void) const                                                      \
    {                                                                                              \
        return _class_::classTypeId;                                                               \
    }                                                                                              \
    Base::Type _class_::classTypeId = Base::Type::badType();                                       \
    void* _class_::create(void)                                                                    \
    {                                                                                              \
        return 0;                                                                                  \
    }


/// define to implement a subclass of Base::BaseClass
#define TYPESYSTEM_SOURCE(_class_, _parentclass_)                                                  \
    TYPESYSTEM_SOURCE_P(_class_)                                                                   \
    void _class_::init(void)                                                                       \
    {                                                                                              \
        initSubclass(_class_::classTypeId, #_class_, #_parentclass_, &(_class_::create));          \
    }

/// define to implement a subclass of Base::BaseClass
#define TYPESYSTEM_SOURCE_TEMPLATE_T(_class_, _parentclass_)                                       \
    TYPESYSTEM_SOURCE_TEMPLATE_P(_class_)                                                          \
    template<>                                                                                     \
    void _class_::init(void)                                                                       \
    {                                                                                              \
        initSubclass(_class_::classTypeId, #_class_, #_parentclass_, &(_class_::create));          \
    }

/// define to implement a subclass of Base::BaseClass
#define TYPESYSTEM_SOURCE_ABSTRACT(_class_, _parentclass_)                                         \
    TYPESYSTEM_SOURCE_ABSTRACT_P(_class_)                                                          \
    void _class_::init(void)                                                                       \
    {                                                                                              \
        initSubclass(_class_::classTypeId, #_class_, #_parentclass_, nullptr);                     \
    }
// NOLINTEND(cppcoreguidelines-macro-usage)

namespace Base
{
/// BaseClass class and root of the type system
class  BaseClass
{
public:
    static Type getClassTypeId();
    virtual Type getTypeId() const;
    bool isDerivedFrom(const Type type) const
    {
        return getTypeId().isDerivedFrom(type);
    }

    static void init();

    static void* create()
    {
        return nullptr;
    }

    template<typename T>
    bool is() const
    {
        return getTypeId() == T::getClassTypeId();
    }

    template<typename T>
    bool isDerivedFrom() const
    {
        return getTypeId().isDerivedFrom(T::getClassTypeId());
    }

private:
    static Type classTypeId;  // NOLINT

protected:
    static void initSubclass(Base::Type& toInit,
                             const char* ClassName,
                             const char* ParentName,
                             Type::instantiationMethod method = nullptr);

public:
    /// Construction
    BaseClass();
    BaseClass(const BaseClass&) = default;
    BaseClass& operator=(const BaseClass&) = default;
    BaseClass(BaseClass&&) = default;
    BaseClass& operator=(BaseClass&&) = default;
    /// Destruction
    virtual ~BaseClass();
};

/**
 * Template that works just like dynamic_cast, but expects the argument to
 * inherit from Base::BaseClass.
 *
 */
template<typename T>
T* freecad_dynamic_cast(Base::BaseClass* type)
{
    if (type && type->isDerivedFrom(T::getClassTypeId())) {
        return static_cast<T*>(type);
    }

    return nullptr;
}

/**
 * Template that works just like dynamic_cast, but expects the argument to
 * inherit from a const Base::BaseClass.
 *
 */
template<typename T>
const T* freecad_dynamic_cast(const Base::BaseClass* type)
{
    if (type && type->isDerivedFrom(T::getClassTypeId())) {
        return static_cast<const T*>(type);
    }

    return nullptr;
}


}  // namespace Base

