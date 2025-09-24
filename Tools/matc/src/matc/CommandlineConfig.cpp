#include "CommandlineConfig.h"



#include "Path.h"

#include <istream>
#include <sstream>
#include <string>


namespace matc {

	static void usage(char* name) {
		std::string exec_name(Path(name).getName());
		std::string usage(
			"MATC is a command-line tool to compile material definition.\n"
			"\n"
			"Caution! MATC was designed to operate on trusted inputs. To minimize the risk of\n"
			"triggering memory corruption vulnerabilities, please make sure that the files passed\n"
			"to MATC come from a trusted source, or run MATC in a sandboxed environment.\n"
			"\n"
			"Usages:\n"
			"    MATC [options] <input-file>\n"
			"\n"
			"Supported input formats:\n"
			"    Filament material definition (.mat)\n"
			"\n"
			"Options:\n"
			"   --help, -h\n"
			"       Print this message\n\n"
			"   --license\n"
			"       Print copyright and license information\n\n"
			"   --output, -o\n"
			"       Specify path to output file\n\n"
			"   --platform, -p\n"
			"       Shader family to generate: desktop, mobile or all (default)\n\n"
			"   --optimize-size, -S\n"
			"       Optimize generated shader code for size instead of just performance\n\n"
			"   --api, -a\n"
			"       Specify the target API: opengl (default), vulkan, metal, or all\n"
			"       This flag can be repeated to individually select APIs for inclusion:\n"
			"           MATC --api opengl --api metal ...\n\n"
			"   --feature-level, -l\n"
			"       Specify the maximum feature level allowed (default is 3).\n\n"
			"   --no-essl1, -1\n"
			"       Don't generate ESSL 1.0 code even for Feature Level 0 mobile shaders.\n"
			"       Shaders are still validated against ESSL 1.0.\n\n"
			"   --define, -D\n"
			"       Add a preprocessor define macro via <macro>=<value>. <value> defaults to 1 if omitted.\n"
			"       Can be repeated to specify multiple definitions:\n"
			"           MATC -Dfoo=1 -Dbar -Dbuzz=100 ...\n\n"
			"   --template <macro>=<string>, -T<macro>=<string>\n"
			"       Replaces ${MACRO} with specified string before parsing\n"
			"       Unlike --define, this applies to the material specification, not GLSL.\n"
			"       Can be repeated to specify multiple macros:\n"
			"           MATC -TBLENDING=fade -TDOUBLESIDED=false ...\n\n"
			"   --material-parameter <key>=<value>, -P<key>=<value>\n"
			"       Set the material property pointed to by <key> to <value>\n"
			"       This overwrites the value configured in the material file.\n"
			"       Material property of array type is not supported.\n"
			"           MATC -PflipUV=false -PshadingModel=lit -Pname=myMat ...\n\n"
			"   --reflect, -r\n"
			"       Reflect the specified metadata as JSON: parameters\n\n"
			"   --variant-filter=<filter>, -V <filter>\n"
			"       Filter out specified comma-separated variants:\n"
			"           directionalLighting, dynamicLighting, shadowReceiver, skinning, vsm, fog,"
			"           ssr (screen-space reflections), stereo\n"
			"       This variant filter is merged with the filter from the material, if any\n\n"
			"   --version, -v\n"
			"       Print the material version number\n\n"
			"Internal use and debugging only:\n"
			"   --optimize-none, -g\n"
			"       Disable all shader optimizations, for debugging\n\n"
			"   --preprocessor-only, -E\n"
			"       Optimize shaders by running only the preprocessor\n\n"
			"   --raw, -w\n"
			"       Compile a raw GLSL shader into a SPIRV binary chunk\n\n"
			"   --output-format, -f\n"
			"       Specify output format: blob (default) or header\n\n"
			"   --debug, -d\n"
			"       Generate extra data for debugging\n\n"
			"   --no-sampler-validation, -F\n"
			"       Skip validation of number of sampler used\n\n"
			"   --print, -t\n"
			"       Print generated shaders for debugging\n\n"
			"   --save-raw-variants, -R\n"
			"       Write the raw generated GLSL for each variant to a text file in the current directory.\n\n"
		);
		const std::string from("MATC");
		for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
			usage.replace(pos, from.length(), exec_name);
		}
		printf("%s", usage.c_str());
	}

	static void license() {
		static const char* license[] = {

			nullptr
		};

		const char** p = &license[0];
		while (*p)
			std::cout << *p++ << std::endl;
	}



	CommandlineConfig::CommandlineConfig(int argc, char** argv) : Config(), mArgc(argc), mArgv(argv) {
		mIsValid = parse();
	}

	static void parseDefine(std::string defineString, Config::StringReplacementMap& defines) {
		const char* const defineArg = defineString.c_str();
		const size_t length = defineString.length();

		const char* p = defineArg;
		const char* end = p + length;

		while (p < end && *p != '=') {
			p++;
		}

		if (*p == '=') {
			if (p == defineArg || p + 1 >= end) {
				// Edge-cases, missing define name or value.
				return;
			}
			std::string def(defineArg, p - defineArg);
			defines.emplace(def, p + 1);
			return;
		}

		// No explicit assignment, use a default value of 1.
		std::string def(defineArg, p - defineArg);
		defines.emplace(def, "1");
	}

	bool CommandlineConfig::parse() {
        for( int i = 1; i < mArgc; )
        {
            if( strcmp(mArgv[i], "-o") == 0 )
            {
                mOutput = new FilesystemOutput(mArgv[i+1]);
                i += 2;
                continue;
            }
            mInput = new FilesystemInput(mArgv[i]);
            i++;
        }
		return true;
	}

} // namespace matc
