#pragma once
#include <filamat/IncludeCallback.h>
#include "Path.h"
#include <assert.h>


namespace matc {

// Functor callback handler used to resolve includes relative to a root include directory.
class DirIncluder {
public:
    void setIncludeDirectory(Path dir) noexcept {
        assert(dir.isDirectory());
        mIncludeDirectory = dir;
    }

    bool operator()(filamat::IncludeResult& result);

private:
    Path mIncludeDirectory;

};

} // namespace matc

