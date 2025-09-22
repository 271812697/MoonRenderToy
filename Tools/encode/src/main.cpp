#include "stb_image/stb_image_write.h"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <string.h>
#include "Path.h"
#include <filesystem>

using namespace std;

static std::string g_packageName = "resources";
static std::string g_deployDir = ".";
static bool g_keepExtension = true;
static bool g_appendNull = false;
static bool g_generateC = true;
static bool g_quietMode = false;
struct PathItem
{
    Path filePath;
    std::string reletiveDir;
    PathItem(const Path& p, const std::string& r)
        : filePath(p)
        , reletiveDir(r)
    {
    }
};
vector<PathItem> inputPaths;
void handleArg(int argc, char* argv[])
{
    for( int i = 1; i < argc;  )
    {
        if( strcmp(argv[i], "-p") == 0 )
        {
            g_packageName = std::string(argv[i + 1]);
            i += 2;
            continue;
        }
        if( strcmp(argv[i], "-x") == 0 )
        {
            g_deployDir = std::string(argv[i+1]);
            i += 2;
            continue;
        }
        
        Path file(argv[i]);
        if( file.isFile() )
        {
            inputPaths.push_back({file, ""});
        }
        else if(file.isDirectory())
        {
            std::string baseDir=file.getPath();
            auto contents=file.listContents();
            while(!contents.empty())
            {
                auto path = contents.back();
                contents.pop_back();
                if( path.isFile() )
                {
                    inputPaths.push_back(PathItem(path, path.getReleativeTo(baseDir)));
                }
                else if(path.isDirectory())
                {
                    auto subDir = path.listContents();
                    contents.insert(contents.end(),subDir.begin(),subDir.end());
                }
            }
        }
        i++;
    }  
}

int main(int argc, char* argv[]) {

    handleArg(argc,argv);
    std::string packageFile = g_packageName;
    std::string packagePrefix = std::string(g_packageName) + "_";
    transform(packageFile.begin(), packageFile.end(), packageFile.begin(), ::tolower);
    transform(packagePrefix.begin(), packagePrefix.end(), packagePrefix.begin(), ::toupper);
    std::string package = packagePrefix + "PACKAGE";

    const Path deployDir(g_deployDir);
    if (!deployDir.exists()) {
        deployDir.mkdirRecursive();
    }
    
    const Path headerPath(deployDir + (packageFile + ".h"));
    const Path xxdPath(deployDir + (packageFile + ".cpp"));


    // Open the header file stream for writing.
    ostringstream headerStream;
    headerStream << "#ifndef " << packagePrefix << "H_" << endl
                 << "#define " << packagePrefix << "H_" << endl
                 << endl
                 << "#include <stdint.h>" << endl
                 << endl
                 << "#include <string>"<<endl
                 << "#include <unordered_map>" << endl
                 << endl
                 << " extern std::unordered_map<std::string, int> shaders_offset;" << endl
                 << endl
                 << "extern std::unordered_map<std::string, int> shaders_len;" << endl<<endl
           // << "" << endl << "extern \"C\" {" << endl
            << "   const uint8_t* ShaderPackage();" << endl;

    ostringstream headerMacros;
    ostringstream xxdDefinitions;

    ostringstream func;
    func << "static void initOffset(){\n\n";
    // Open the generated C file for writing.
    ostringstream xxdStream;
    if (g_generateC) {
       

        xxdStream << "#include <string>\n\n";
        xxdStream << "#include <unordered_map>\n\n"; 
        xxdStream << "#include <stdint.h>\n\n";
        xxdStream << "std::unordered_map<std::string, int> shaders_offset;\n\n"; 
        xxdStream << "std::unordered_map<std::string, int> shaders_len;\n\n";
            
        xxdStream << "  const uint8_t " << package << "[] = {\n";
    }

    // Consume each input file and write it back out into the various output streams.
    size_t offset = 0;
    for (const auto& inPath : inputPaths) {
        
        ifstream inStream(inPath.filePath.getPath(), ios::binary|ios::ate);
        if (!inStream) {
            cerr << "Unable to open " << inPath.filePath.getPath() << endl;
            exit(1);
        }
        std::streamsize size = inStream.tellg();
        inStream.seekg(0,std::ios::beg);
        vector<char> content(size);
        inStream.read(content.data(),size);
      
        if (g_appendNull) {
            content.push_back(0);
        }

        // Formulate the resource name and the prefixed resource name.
        std::string rname = inPath.filePath.getNameWithoutExtension()+inPath.filePath.getExtension();
        std::string filename = inPath.reletiveDir;
      
        replace(rname.begin(), rname.end(), '.', '_');
        transform(rname.begin(), rname.end(), rname.begin(), ::toupper);
        const std::string prname = packagePrefix + rname;

        // Write the offsets and sizes.
        headerMacros
                << "#define " << prname << "_DATA (" << package << " + " << prname << "_OFFSET)\n";

        headerStream
                << "    extern int " << prname << "_OFFSET;\n"
                << "    extern int " << prname << "_SIZE;\n";

        if (g_generateC) {
            xxdDefinitions
                    << "int " << prname << "_OFFSET = " << offset << ";\n"
                    << "int " << prname << "_SIZE = " << content.size() << ";\n";
            func << "\n shaders_offset[\"" << filename << "\"] = " << offset << ";\n";
            func << "\n shaders_len[\"" << filename << "\"] = " << content.size() << ";\n";

            xxdStream << "// " << rname << "\n";
            xxdStream << setfill('0') << hex;

            size_t i = 0;
            for (; i < content.size(); i++) {
                if (i > 0 && i % 20 == 0) {
                    xxdStream << "\n";
                }
                xxdStream << "0x" << setw(2) << (int) content[i] << ", ";
            }
            if (i % 20 != 0) xxdStream << "\n";
            xxdStream << "\n";
        }       
        offset += content.size();
    }
    headerStream << "" << headerMacros.str();
    headerStream << "\n#endif\n";

    func << "}\n";
    func << "const uint8_t* ShaderPackage()\n{\n"
         << "static bool flag=false;\n"
         << "if (!flag) {\n"
         << "flag = true;\n"
         << "initOffset();\n"
         << "}\n"
         << "return " << package << ";\n"
         << "}\n";
    // To optimize builds, avoid overwriting the header file if nothing has changed.
    bool headerIsDirty = true;
    ifstream headerInStream(headerPath.getPath(), std::ifstream::ate);
    string headerContents = headerStream.str();
    if (headerInStream) {
        long fileSize = static_cast<long>(headerInStream.tellg());
        if (fileSize == headerContents.size()) {
            vector<char> previous(fileSize);
            headerInStream.seekg(0);
            headerInStream.read(previous.data(), fileSize);
            headerIsDirty = 0 != memcmp(previous.data(), headerContents.c_str(), fileSize);
        }
    }
    if (headerIsDirty) {
        ofstream headerOutStream(headerPath.getPath());
        if (!headerOutStream) {
            cerr << "Unable to open " << headerPath << endl;
            exit(1);
        }
        headerOutStream << headerContents;
    }

    if (g_generateC) {
        xxdStream << "};\n\n" << xxdDefinitions.str();
        xxdStream << func.str();
        if (!g_quietMode) {
            cout << " " << xxdPath;
        }
    }
    xxdStream << headerContents;
    std::string buf = xxdStream.str();
    int size = buf.size() / 4;
    int width = sqrt(size);
    int left=width*(width+2)*4-buf.size();
    for (int i = 0; i < left; i++) {
        xxdStream << '\n';
    }
    buf = xxdStream.str();
    stbi_write_png("res.png", width, width + 2, 4, buf.data(), width * 4);
}
