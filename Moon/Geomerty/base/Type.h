#pragma once
// Std. configurations

#include <string>
#include <map>
#include <set>
#include <vector>


namespace Base
{

struct TypeData;


/** Type system class
  Many of the classes in the FreeCAD must have their type
  information registered before any instances are created (including,
  but not limited to: App::Feature, App::Property, Gui::ViewProvider
  ). The use of Type to store this information provides
  lots of various functionality for working with class hierarchies,
  comparing class types, instantiating objects from classnames, etc
  etc.

  It is for instance possible to do things like this:

  \code
  void getRightFeature(Base::Base * anode)
  {
    assert(anode->isDerivedFrom<App::Feature>());

    if (anode->is<Mesh::MeshFeature>()) {
      // do something..
    }
    else if (anode->is<Part::PartFeature>()) {
      // do something..
    }
    else {
      Base::Console().Warning("getRightFeature", "Unknown feature type %s!\n",
                                anode->getTypeId().getName());
    }
  }
  \endcode

  A notable feature of the Type class is that it is only 16 bits
  long and therefore should be passed around by value for efficiency
  reasons.

  One important note about the use of Type to register class
  information: super classes must be registered before any of their
  derived classes are.
*/
class  Type
{
public:
    /// Construction
    Type(const Type& type) = default;
    Type(Type&& type) = default;
    Type() = default;
    /// Destruction
    ~Type() = default;

    /// creates a instance of this type
    void* createInstance();
    /// Checks whether this type can instantiate
    bool canInstantiate() const;
    /// creates a instance of the named type
    static void* createInstanceByName(const char* TypeName, bool bLoadModule = false);
    static void importModule(const char* TypeName);

    using instantiationMethod = void* (*)();

    static Type fromName(const char* name);
    static Type fromKey(unsigned int key);
    const char* getName() const;
    Type getParent() const;
    bool isDerivedFrom(const Type& type) const;

    static int getAllDerivedFrom(const Type& type, std::vector<Type>& List);
    /// Returns the given named type if is derived from parent type, otherwise return bad type
    static Type
    getTypeIfDerivedFrom(const char* name, const Type& parent, bool bLoadModule = false);

    static int getNumTypes();

    static Type
    createType(const Type& parent, const char* name, instantiationMethod method = nullptr);

    unsigned int getKey() const;
    bool isBad() const;

    Type& operator=(const Type& type) = default;
    Type& operator=(Type&& type) = default;
    bool operator==(const Type& type) const;
    bool operator!=(const Type& type) const;

    bool operator<(const Type& type) const;
    bool operator<=(const Type& type) const;
    bool operator>=(const Type& type) const;
    bool operator>(const Type& type) const;

    static Type badType();
    static void init();
    static void destruct();

    static std::string getModuleName(const char* ClassName);


private:
    unsigned int index {0};

    static std::map<std::string, unsigned int> typemap;
    static std::vector<TypeData*> typedata;
    static std::set<std::string> loadModuleSet;
};


inline unsigned int Type::getKey() const
{
    return this->index;
}

inline bool Type::operator!=(const Type& type) const
{
    return (this->getKey() != type.getKey());
}

inline bool Type::operator==(const Type& type) const
{
    return (this->getKey() == type.getKey());
}

inline bool Type::operator<(const Type& type) const
{
    return (this->getKey() < type.getKey());
}

inline bool Type::operator<=(const Type& type) const
{
    return (this->getKey() <= type.getKey());
}

inline bool Type::operator>=(const Type& type) const
{
    return (this->getKey() >= type.getKey());
}

inline bool Type::operator>(const Type& type) const
{
    return (this->getKey() > type.getKey());
}

inline bool Type::isBad() const
{
    return (this->index == 0);
}

}  // namespace Base


