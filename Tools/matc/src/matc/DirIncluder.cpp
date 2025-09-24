#include "DirIncluder.h"

#include <fstream>

namespace matc {

bool DirIncluder::operator()( filamat::IncludeResult& result) {
    auto getHeaderPath = [&result, this]() {

        if( result.includeName[0] != ':' )
        {
            return mIncludeDirectory.concat(result.includeName.c_str());
        }
        std::string p = PROJECT_MAT_PATH"/"+ result.includeName.substr(1);
      
        return Path(p); 
    };

    const Path& headerPath = getHeaderPath();

    if (!headerPath.isFile()) {
       
        return false;
    }

    std::ifstream stream(headerPath.getPath(), std::ios::binary);
    if (!stream) {
      
        return false;
    }

    std::string contents;

    stream.seekg(0, std::ios::end);
    contents.reserve(stream.tellg());
    stream.seekg(0, std::ios::beg);
    contents.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

    stream.close();

    result.text = std::string(contents.c_str());
    result.name = std::string(headerPath.c_str());

    return true;
}

} // namespace matc

