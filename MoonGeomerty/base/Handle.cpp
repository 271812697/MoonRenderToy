#pragma once

#include <cassert>
#include <iostream>

#include <QAtomicInt>

#include "Handle.h"


using namespace Base;

//**************************************************************************
// Construction/Destruction

Handled::Handled()
    : _lRefCount(new QAtomicInt(0))
{}

Handled::~Handled()
{
    if (static_cast<int>(*_lRefCount) != 0) {
        std::cerr << "Reference counter of deleted object is not zero!!!!!" << std::endl;
    }
    delete _lRefCount;
}

void Handled::ref() const
{
    _lRefCount->ref();
}

void Handled::unref() const
{
    assert(*_lRefCount > 0);
    if (!_lRefCount->deref()) {
        delete this;
    }
}

int Handled::unrefNoDelete() const
{
    int res = _lRefCount->deref();
    assert(res >= 0);
    return res;
}

int Handled::getRefCount() const
{
    return static_cast<int>(*_lRefCount);
}

Handled& Handled::operator=(const Handled& /*unused*/)
{
    // we must not assign _lRefCount
    return *this;
}
