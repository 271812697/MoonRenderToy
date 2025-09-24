#pragma once
#include "CommandlineConfig.h"

#include <filamat/Package.h>

namespace matc {

class Compiler {
public:
    virtual ~Compiler() = default;

    bool compile(const Config& config) {
        if (!checkParameters(config)) {
            return false;
        }
        return run(config);
    }

protected:
    bool writePackage(const filamat::Package& package, const Config& config) {
        if (config.getOutputFormat() == CommandlineConfig::OutputFormat::BLOB) {
            return writeBlob(package, config);
        } else {
            return writeBlobAsHeader(package, config);
        }
    }
    virtual bool run(const Config& config) = 0;
    virtual bool checkParameters(const Config& config) = 0;

    // Write Package as binary to target filename
    bool writeBlob(const filamat::Package& pkg, const Config& config) const noexcept;

    // Write package as a C++ array content. Use this to include material
    // in your executable/library.
    bool writeBlobAsHeader(const filamat::Package& pkg, const Config& config) const noexcept;
};

} 