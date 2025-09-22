#include "matc/CommandlineConfig.h"
#include "matc/MaterialCompiler.h"

#include <iostream>

#include <stdlib.h>

using namespace matc;

int main(int argc, char** argv) {
    CommandlineConfig config(argc, argv);
    if (!config.isValid()) {
        std::cerr << "Invalid parameters." << std::endl;
        return EXIT_FAILURE;
    }

    MaterialCompiler compiler;
    if (!compiler.compile(config)) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
