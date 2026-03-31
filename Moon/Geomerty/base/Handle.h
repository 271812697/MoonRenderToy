#pragma once

class QAtomicInt;

namespace Base
{

/** Reference class
 *  Implementation of the reference counting pattern.
 *  Only able to instantiate with a class inheriting
 *  Base::Handled.
 */
template<class T>
class Reference
{
public:
    //**************************************************************************
    // construction & destruction

    /** Pointer and default constructor */
    Reference()
        : _toHandle(nullptr)
    {}

    Reference(T* pointer)
        : _toHandle(pointer)
    {
        if (_toHandle) {
            _toHandle->ref();
        }
    }

    /** Copy constructor */
    Reference(const Reference<T>& ref)
        : _toHandle(ref._toHandle)
    {
        if (_toHandle) {
            _toHandle->ref();
        }
    }

    /** destructor
     *  Release the reference counter which causes,
     *  in case of the last one, the referenced object to
     *  be destructed!
     */
    ~Reference()
    {
        if (_toHandle) {
            _toHandle->unref();
        }
    }

    //**************************************************************************
    // operator implementation

    /** Assign operator from a pointer */
    Reference<T>& operator=(T* pointer)
    {
        // check if we want to reassign the same object
        if (_toHandle == pointer) {
            return *this;
        }
        if (_toHandle) {
            _toHandle->unref();
        }
        _toHandle = pointer;
        if (_toHandle) {
            _toHandle->ref();
        }
        return *this;
    }

    /** Assign operator from a handle */
    Reference<T>& operator=(const Reference<T>& ref)
    {
        // check if we want to reassign the same object
        if (_toHandle == ref._toHandle) {
            return *this;
        }
        if (_toHandle) {
            _toHandle->unref();
        }
        _toHandle = ref._toHandle;
        if (_toHandle) {
            _toHandle->ref();
        }
        return *this;
    }

    /** Dereference operator */
    T& operator*() const
    {
        return *_toHandle;
    }

    /** Dereference operator */
    T* operator->() const
    {
        return _toHandle;
    }

    operator T*() const
    {
        return _toHandle;
    }

    //**************************************************************************
    // checking on the state

    /// Test if it handles something
    bool isValid() const
    {
        return _toHandle != nullptr;
    }

    /// Test if it does not handle anything
    bool isNull() const
    {
        return _toHandle == nullptr;
    }

    /// Get number of references on the object, including this one
    int getRefCount() const
    {
        if (_toHandle) {
            return _toHandle->getRefCount();
        }
        return 0;
    }

private:
    T* _toHandle; /** the pointer to the handled object */
};

/** Handled class
 *  Implementation of the reference counting pattern.
 */
class  Handled
{
public:
    Handled();
    virtual ~Handled();

    void ref() const;
    void unref() const;
    int unrefNoDelete() const;

    int getRefCount() const;
    Handled& operator=(const Handled&);

    Handled(const Handled&) = delete;
    Handled(Handled&&) = delete;
    Handled& operator=(Handled&&) = delete;

private:
    QAtomicInt* _lRefCount;
};

}  