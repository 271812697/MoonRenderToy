

#include "CodeGenerator.h"

#include "MaterialInfo.h"
//#include "../PushConstantDefinitions.h"

#include "test/shaders/shaders.h"

#include <test/DriverEnums.h>

#include <test/utils/sstream.h>

#include <cctype>
#include <iomanip>

#include <assert.h>

namespace TEST {

	// From driverEnum namespace

	using namespace utils;

	sstream& CodeGenerator::generateSeparator(sstream& out) {
		out << '\n';
		return out;
	}

	sstream& CodeGenerator::generateProlog(sstream& out, ShaderStage stage,
		MaterialInfo const& material, Variant v) const {
		out << "#version 450 core\n\n";
		// This allows our includer system to use the #line directive to denote the source file for
		// #included code. This way, glslang reports errors more accurately.
		out << "#extension GL_GOOGLE_cpp_style_line_directive : enable\n\n";
		if (v.hasStereo() && stage == ShaderStage::VERTEX) {

		}
		if (stage == ShaderStage::COMPUTE) {
			out << "layout(local_size_x = " << material.groupSize.x
				<< ", local_size_y = " << material.groupSize.y
				<< ", local_size_z = " << material.groupSize.z
				<< ") in;\n\n";
		}
		out << "#define TARGET_GL_ENVIRONMENT\n";
		out << "#define FILAMENT_OPENGL_SEMANTICS\n";
		out << "#define FILAMENT_HAS_FEATURE_TEXTURE_GATHER\n";
		out << "#define FILAMENT_HAS_FEATURE_INSTANCING\n";



		// During compilation and optimization, __VERSION__ reflects the shader language version of the
		// intermediate code, not the version of the final code. spirv-cross automatically adapts
		// certain language features (e.g. fragment output) but leaves others untouched (e.g. sampler
		// functions, bit shift operations). Client code may have to make decisions based on this
		// information, so define a FILAMENT_EFFECTIVE_VERSION constant.
		const char* effective_version = "__VERSION__";;

		generateDefine(out, "FILAMENT_EFFECTIVE_VERSION", effective_version);



		if (stage == ShaderStage::VERTEX) {
			CodeGenerator::generateDefine(out, "FLIP_UV_ATTRIBUTE", material.flipUV);
			CodeGenerator::generateDefine(out, "LEGACY_MORPHING", material.useLegacyMorphing);
		}
		if (stage == ShaderStage::FRAGMENT) {
			CodeGenerator::generateDefine(out, "MATERIAL_HAS_CUSTOM_DEPTH",
				material.userMaterialHasCustomDepth);
		}
		generateDefine(out, "VARYING", "varying");
		generateDefine(out, "ATTRIBUTE", "attribute");
		auto getShadingDefine = [](Shading shading) -> const char* {
			switch (shading) {
			case Shading::LIT:                 return "SHADING_MODEL_LIT";
			case Shading::UNLIT:               return "SHADING_MODEL_UNLIT";
			case Shading::SUBSURFACE:          return "SHADING_MODEL_SUBSURFACE";
			case Shading::CLOTH:               return "SHADING_MODEL_CLOTH";
			case Shading::SPECULAR_GLOSSINESS: return "SHADING_MODEL_SPECULAR_GLOSSINESS";
			}
			};

		CodeGenerator::generateDefine(out, getShadingDefine(material.shading), true);

		generateQualityDefine(out, material.quality);

		// precision qualifiers
		out << '\n';
		Precision const defaultPrecision = getDefaultPrecision(stage);
		const char* precision = getPrecisionQualifier(defaultPrecision);
		out << "precision " << precision << " float;\n";
		out << "precision " << precision << " int;\n";

		// Filament-reserved specification constants (limited by CONFIG_MAX_RESERVED_SPEC_CONSTANTS)
		out << '\n';
		generateSpecializationConstant(out, "BACKEND_FEATURE_LEVEL",
			+ReservedSpecializationConstants::BACKEND_FEATURE_LEVEL, 1);
		generateSpecializationConstant(out, "CONFIG_MAX_INSTANCES",
			+ReservedSpecializationConstants::CONFIG_MAX_INSTANCES, (int)CONFIG_MAX_INSTANCES);

		// the default of 1024 (16KiB) is needed for 32% of Android devices
		generateSpecializationConstant(out, "CONFIG_FROXEL_BUFFER_HEIGHT",
			+ReservedSpecializationConstants::CONFIG_FROXEL_BUFFER_HEIGHT, 1024);

		// directional shadowmap visualization
		generateSpecializationConstant(out, "CONFIG_DEBUG_DIRECTIONAL_SHADOWMAP",
			+ReservedSpecializationConstants::CONFIG_DEBUG_DIRECTIONAL_SHADOWMAP, false);

		// froxel visualization
		generateSpecializationConstant(out, "CONFIG_DEBUG_FROXEL_VISUALIZATION",
			+ReservedSpecializationConstants::CONFIG_DEBUG_FROXEL_VISUALIZATION, false);

		// Workaround a Metal pipeline compilation error with the message:
		// "Could not statically determine the target of a texture". See light_indirect.fs
		generateSpecializationConstant(out, "CONFIG_STATIC_TEXTURE_TARGET_WORKAROUND",
			+ReservedSpecializationConstants::CONFIG_STATIC_TEXTURE_TARGET_WORKAROUND, false);

		generateSpecializationConstant(out, "CONFIG_POWER_VR_SHADER_WORKAROUNDS",
			+ReservedSpecializationConstants::CONFIG_POWER_VR_SHADER_WORKAROUNDS, false);

		generateSpecializationConstant(out, "CONFIG_STEREO_EYE_COUNT",
			+ReservedSpecializationConstants::CONFIG_STEREO_EYE_COUNT, material.stereoscopicEyeCount);

		generateSpecializationConstant(out, "CONFIG_SH_BANDS_COUNT",
			+ReservedSpecializationConstants::CONFIG_SH_BANDS_COUNT, 3);

		generateSpecializationConstant(out, "CONFIG_SHADOW_SAMPLING_METHOD",
			+ReservedSpecializationConstants::CONFIG_SHADOW_SAMPLING_METHOD, 1);

		// CONFIG_MAX_STEREOSCOPIC_EYES is used to size arrays and on Adreno GPUs + vulkan, this has to
		// be explicitly, statically defined (as in #define). Otherwise (using const int for
		// example), we'd run into a GPU crash.
		out << "#define CONFIG_MAX_STEREOSCOPIC_EYES " << (int)CONFIG_MAX_STEREOSCOPIC_EYES << "\n";


		out << '\n';
		std::string a = "aa";
		out << SHADERS_COMMON_DEFINES_GLSL_DATA;
		out << "\n";
		return out;
	}

	Precision CodeGenerator::getDefaultPrecision(ShaderStage stage) const {
		switch (stage) {
		case ShaderStage::VERTEX:
			return Precision::HIGH;
		case ShaderStage::FRAGMENT:
			return Precision::HIGH;
		case ShaderStage::COMPUTE:
			return Precision::HIGH;
		}
	}

	Precision CodeGenerator::getDefaultUniformPrecision() const {
		return Precision::HIGH;

	}

	sstream& CodeGenerator::generateEpilog(sstream& out) {
		out << "\n"; // For line compression all shaders finish with a newline character.
		return out;
	}

	sstream& CodeGenerator::generateCommonTypes(sstream& out, ShaderStage stage) {
		out << '\n';
		switch (stage) {
		case ShaderStage::VERTEX:
			out << '\n';
			out << SHADERS_COMMON_TYPES_GLSL_DATA;
			break;
		case ShaderStage::FRAGMENT:
			out << '\n';
			out << SHADERS_COMMON_TYPES_GLSL_DATA;
			break;
		case ShaderStage::COMPUTE:
			break;
		}
		return out;
	}

	sstream& CodeGenerator::generateShaderMain(sstream& out, ShaderStage stage) {
		switch (stage) {
		case ShaderStage::VERTEX:
			out << SHADERS_MAIN_VS_DATA;
			break;
		case ShaderStage::FRAGMENT:
			out << SHADERS_MAIN_FS_DATA;
			break;
		case ShaderStage::COMPUTE:
			out << SHADERS_MAIN_CS_DATA;
			break;
		}
		return out;
	}

	sstream& CodeGenerator::generatePostProcessMain(sstream& out, ShaderStage type) {
		if (type == ShaderStage::VERTEX) {
			out << SHADERS_POST_PROCESS_VS_DATA;
		}
		else if (type == ShaderStage::FRAGMENT) {
			out << SHADERS_POST_PROCESS_FS_DATA;
		}
		return out;
	}

	sstream& CodeGenerator::generateVariable(sstream& out, ShaderStage stage,
		const MaterialBuilder::CustomVariable& variable, size_t index) {
		auto const& name = variable.name;
		const char* precisionString = getPrecisionQualifier(variable.precision);
		if (!name.empty()) {
			if (stage == ShaderStage::VERTEX) {
				out << "\n#define VARIABLE_CUSTOM" << index << " " << name.c_str() << "\n";
				out << "\n#define VARIABLE_CUSTOM_AT" << index << " variable_" << name.c_str() << "\n";
				out << "LAYOUT_LOCATION(" << index << ") VARYING " << precisionString << " vec4 variable_" << name.c_str() << ";\n";
			}
			else if (stage == ShaderStage::FRAGMENT) {
				if (!variable.hasPrecision && variable.precision == Precision::DEFAULT) {
					// for backward compatibility
					precisionString = "highp";
				}
				out << "\nLAYOUT_LOCATION(" << index << ") VARYING " << precisionString << " vec4 variable_" << name.c_str() << ";\n";
			}
		}
		return out;
	}

	sstream& CodeGenerator::generateShaderInputs(sstream& out, ShaderStage type,
		const AttributeBitset& attributes, Interpolation interpolation,
		MaterialBuilder::PushConstantList const& pushConstants) const {
		auto const& attributeDatabase = MaterialBuilder::getAttributeDatabase();

		const char* shading = getInterpolationQualifier(interpolation);
		out << "#define SHADING_INTERPOLATION " << shading << "\n";

		out << "\n";
		attributes.forEachSetBit([&out, &attributeDatabase](size_t i) {
			generateDefine(out, attributeDatabase[i].getDefineName().c_str(), true);
			});

		if (type == ShaderStage::VERTEX) {
			out << "\n";
			attributes.forEachSetBit([&out, &attributeDatabase, this](size_t i) {
				auto const& attribute = attributeDatabase[i];
				assert(i == attribute.location);
				out << "layout(location = " << size_t(attribute.location) << ") in ";
				out << getTypeName(attribute.type) << " " << attribute.getAttributeName() << ";\n";
				});

			out << "\n";
			generatePushConstants(out, pushConstants, attributes.size());
		}

		out << "\n";
		out << SHADERS_VARYINGS_GLSL_DATA;
		return out;
	}

	sstream& CodeGenerator::generateOutput(sstream& out, ShaderStage type,
		const CString& name, size_t index,
		MaterialBuilder::VariableQualifier qualifier,
		MaterialBuilder::Precision precision,
		MaterialBuilder::OutputType outputType) const {
		if (name.empty() || type == ShaderStage::VERTEX) {
			return out;
		}


		// TODO: add and support additional variable qualifiers
		(void)qualifier;
		assert(qualifier == MaterialBuilder::VariableQualifier::OUT);

		// The material output type is the type the shader writes to from the material.
		const MaterialBuilder::OutputType materialOutputType = outputType;

		const char* swizzleString = "";

		// Metal doesn't support some 3-component texture formats, so the backend uses 4-component
		// formats behind the scenes. It's an error to output fewer components than the attachment
		// needs, so we always output a float4 instead of a float3. It's never an error to output extra
		// components.
		//


		const char* precisionString = getPrecisionQualifier(precision);
		const char* materialTypeString = getOutputTypeName(materialOutputType);
		const char* typeString = getOutputTypeName(outputType);

		bool generate_essl3_code = true;

		out << "\n#define FRAG_OUTPUT" << index << " " << name.c_str();
		if (generate_essl3_code) {
			out << "\n#define FRAG_OUTPUT_AT" << index << " output_" << name.c_str();
		}
		else {
			out << "\n#define FRAG_OUTPUT_AT" << index << " gl_FragColor";
		}
		out << "\n#define FRAG_OUTPUT_MATERIAL_TYPE" << index << " " << materialTypeString;
		out << "\n#define FRAG_OUTPUT_PRECISION" << index << " " << precisionString;
		out << "\n#define FRAG_OUTPUT_TYPE" << index << " " << typeString;
		out << "\n#define FRAG_OUTPUT_SWIZZLE" << index << " " << swizzleString;
		out << "\n";

		if (generate_essl3_code) {
			out << "\nlayout(location=" << index << ") out " << precisionString << " "
				<< typeString << " output_" << name.c_str() << ";\n";
		}

		return out;
	}


	sstream& CodeGenerator::generateDepthShaderMain(sstream& out, ShaderStage type) {
		assert(type != ShaderStage::VERTEX);
		if (type == ShaderStage::FRAGMENT) {
			out << SHADERS_DEPTH_MAIN_FS_DATA;
		}
		return out;
	}

	const char* CodeGenerator::getUniformPrecisionQualifier(UniformType type, Precision precision,
		Precision uniformPrecision, Precision defaultPrecision) noexcept {
		if (!hasPrecision(type)) {
			// some types like bool can't have a precision qualifier
			return "";
		}
		if (precision == Precision::DEFAULT) {
			// if precision field is specified as default, turn it into the default precision for
			// uniforms (which might be different on desktop vs mobile)
			precision = uniformPrecision;
		}
		if (precision == defaultPrecision) {
			// finally if the precision match the default precision of this stage, don't omit
			// the precision qualifier -- which mean the effective precision might be different
			// in different stages.
			return "";
		}
		return getPrecisionQualifier(precision);
	}

	sstream& CodeGenerator::generateBuffers(sstream& out,
		MaterialInfo::BufferContainer const& buffers) const {

		for (auto const* buffer : buffers) {

			// FIXME: we need to get the bindings for the SSBOs and that will depend on the samplers
			descriptor_binding_t binding = getUniqueSsboBindingPoint();
			generateBufferInterfaceBlock(out, ShaderStage::COMPUTE,
				DescriptorSetBindingPoints::PER_MATERIAL, binding, *buffer);
		}
		return out;
	}

	sstream& CodeGenerator::generateUniforms(sstream& out, ShaderStage stage,
		DescriptorSetBindingPoints set,
		descriptor_binding_t binding,
		const BufferInterfaceBlock& uib) const {
		// For OpenGL, the set is not used bug the binding must be unique.
		binding = getUniqueUboBindingPoint();
		return generateBufferInterfaceBlock(out, stage, set, binding, uib);
	}

	sstream& CodeGenerator::generateInterfaceFields(sstream& out,
		FixedCapacityVector<BufferInterfaceBlock::FieldInfo> const& infos,
		Precision defaultPrecision) const {
		Precision const uniformPrecision = getDefaultUniformPrecision();

		for (auto const& info : infos) {

			char const* const type = getUniformTypeName(info);
			char const* const precision = getUniformPrecisionQualifier(info.type, info.precision,
				uniformPrecision, defaultPrecision);
			out << "    " << precision;
			if (precision[0] != '\0') out << " ";
			out << type << " " << info.name.c_str();
			if (info.isArray) {
				if (info.sizeName.empty()) {
					if (info.size) {
						out << "[" << info.size << "]";
					}
					else {
						out << "[]";
					}
				}
				else {
					out << "[" << info.sizeName.c_str() << "]";
				}
			}
			out << ";\n";
		}
		return out;
	}

	sstream& CodeGenerator::generateUboAsPlainUniforms(sstream& out, ShaderStage stage,
		const BufferInterfaceBlock& uib) const {

		auto const& infos = uib.getFieldInfoList();

		std::string blockName{ uib.getName() };
		std::string instanceName{ uib.getName() };
		blockName.front() = char(std::toupper((unsigned char)blockName.front()));
		instanceName.front() = char(std::tolower((unsigned char)instanceName.front()));

		out << "\nstruct " << blockName << " {\n";

		generateInterfaceFields(out, infos, Precision::DEFAULT);

		out << "};\n";
		out << "uniform " << blockName << " " << instanceName << ";\n";

		return out;
	}

	sstream& CodeGenerator::generateBufferInterfaceBlock(sstream& out, ShaderStage stage,
		DescriptorSetBindingPoints set,
		descriptor_binding_t binding,
		const BufferInterfaceBlock& uib) const {


		auto const& infos = uib.getFieldInfoList();


		std::string blockName{ uib.getName() };
		std::string instanceName{ uib.getName() };
		blockName.front() = char(std::toupper((unsigned char)blockName.front()));
		instanceName.front() = char(std::tolower((unsigned char)instanceName.front()));

		out << "\nlayout(";
		out << "binding = " << +binding << ", ";

		switch (uib.getAlignment()) {
		case BufferInterfaceBlock::Alignment::std140:
			out << "std140";
			break;
		case BufferInterfaceBlock::Alignment::std430:
			out << "std430";
			break;
		}
		out << ") ";

		switch (uib.getTarget()) {
		case BufferInterfaceBlock::Target::UNIFORM:
			out << "uniform ";
			break;
		case BufferInterfaceBlock::Target::SSBO:
			out << "buffer ";
			break;
		}

		out << blockName << " ";

		if (uib.getTarget() == BufferInterfaceBlock::Target::SSBO) {
			uint8_t qualifiers = uib.getQualifier();
			while (qualifiers) {
				uint8_t const mask = 1u << __builtin_ctz(unsigned(qualifiers));
				switch (BufferInterfaceBlock::Qualifier(qualifiers & mask)) {
				case BufferInterfaceBlock::Qualifier::COHERENT:  out << "coherent "; break;
				case BufferInterfaceBlock::Qualifier::WRITEONLY: out << "writeonly "; break;
				case BufferInterfaceBlock::Qualifier::READONLY:  out << "readonly "; break;
				case BufferInterfaceBlock::Qualifier::VOLATILE:  out << "volatile "; break;
				case BufferInterfaceBlock::Qualifier::RESTRICT:  out << "restrict "; break;
				}
				qualifiers &= ~mask;
			}
		}

		out << "{\n";

		generateInterfaceFields(out, infos, getDefaultPrecision(stage));

		out << "} " << instanceName << ";\n";

		return out;
	}

	sstream& CodeGenerator::generateSamplers(sstream& out,
		DescriptorSetBindingPoints set,
		SamplerInterfaceBlock::SamplerInfoList const& list) const {
		if (list.empty()) {
			return out;
		}

		for (auto const& info : list) {
			auto type = info.type;
			if (type == SamplerType::SAMPLER_EXTERNAL) {
				// we're generating the shader for the desktop, where we assume external textures
				// are not supported, in which case we revert to texture2d
				type = SamplerType::SAMPLER_2D;
			}
			char const* const typeName = getSamplerTypeName(type, info.format, info.multisample);
			char const* const precision = getPrecisionQualifier(info.precision);

			out << "uniform " << precision << " " << typeName << " " << info.uniformName.c_str();
			out << ";\n";
		}
		out << "\n";

		return out;
	}


	void CodeGenerator::fixupExternalSamplers(
		std::string& shader, SamplerInterfaceBlock const& sib, FeatureLevel featureLevel) noexcept {
		auto const& infos = sib.getSamplerInfoList();
		if (infos.empty()) {
			return;
		}

		bool hasExternalSampler = false;

		// Replace sampler2D declarations by samplerExternal declarations as they may have
		// been swapped during a previous optimization step
		for (auto const& info : infos) {
			if (info.type == SamplerType::SAMPLER_EXTERNAL) {
				auto name = std::string("sampler2D ") + info.uniformName.c_str();
				size_t const index = shader.find(name);

				if (index != std::string::npos) {
					hasExternalSampler = true;
					auto newName =
						std::string("samplerExternalOES ") + info.uniformName.c_str();
					shader.replace(index, name.size(), newName);
				}
			}
		}

		// This method should only be called on shaders that have external samplers but since
		// they may have been removed by previous optimization steps, we check again here
		if (hasExternalSampler) {
			// Find the #version line, so we can insert the #extension directive
			size_t index = shader.find("#version");
			index += 8;

			// Find the end of the line and skip the line return
			while (shader[index] != '\n') index++;
			index++;

			const char* extensionLine = (featureLevel >= FeatureLevel::FEATURE_LEVEL_1)
				? "#extension GL_OES_EGL_image_external_essl3 : require\n\n"
				: "#extension GL_OES_EGL_image_external : require\n\n";
			shader.insert(index, extensionLine);
		}
	}


	sstream& CodeGenerator::generateDefine(sstream& out, const char* name, bool value) {
		if (value) {
			out << "#define " << name << "\n";
		}
		return out;
	}

	sstream& CodeGenerator::generateDefine(sstream& out, const char* name, uint32_t value) {
		out << "#define " << name << " " << value << "\n";
		return out;
	}

	sstream& CodeGenerator::generateDefine(sstream& out, const char* name, const char* string) {
		out << "#define " << name << " " << string << "\n";
		return out;
	}

	sstream& CodeGenerator::generateIndexedDefine(sstream& out, const char* name,
		uint32_t index, uint32_t value) {
		out << "#define " << name << index << " " << value << "\n";
		return out;
	}

	struct SpecializationConstantFormatter {
		std::string operator()(int value) noexcept { return std::to_string(value); }
		std::string operator()(float value) noexcept { return std::to_string(value); }
		std::string operator()(bool value) noexcept { return value ? "true" : "false"; }
	};

	sstream& CodeGenerator::generateSpecializationConstant(sstream& out,
		const char* name, uint32_t id, std::variant<int, float, bool> value) const {

		std::string const constantString = std::visit(SpecializationConstantFormatter(), value);

		static const char* types[] = { "int", "float", "bool" };

		out << "#ifndef SPIRV_CROSS_CONSTANT_ID_" << id << '\n'
			<< "#define SPIRV_CROSS_CONSTANT_ID_" << id << " " << constantString << '\n'
			<< "#endif" << '\n'
			<< "const " << types[value.index()] << " " << name << " = SPIRV_CROSS_CONSTANT_ID_" << id
			<< ";\n\n";

		return out;
	}

	sstream& CodeGenerator::generatePushConstants(sstream& out,
		MaterialBuilder::PushConstantList const& pushConstants, size_t const layoutLocation) const {
		static constexpr char const* STRUCT_NAME = "Constants";


		auto const getType = [](ConstantType const& type) {
			switch (type) {
			case ConstantType::BOOL:
				return "bool";
			case ConstantType::INT:
				return "int";
			case ConstantType::FLOAT:
				return "float";
			}
			};

		out << "struct " << STRUCT_NAME << " {\n";


		for (auto const& constant : pushConstants) {
			out << getType(constant.type) << " " << constant.name.c_str() << ";\n";
		}


		out << "};\n";
		out << "LAYOUT_LOCATION(" << static_cast<int>(layoutLocation) << ") uniform " << STRUCT_NAME
			<< " " << "pushConstants" << ";\n";

		return out;
	}

	sstream& CodeGenerator::generateMaterialProperty(sstream& out,
		MaterialBuilder::Property property, bool isSet) {
		if (isSet) {
			out << "#define " << "MATERIAL_HAS_" << getConstantName(property) << "\n";
		}
		return out;
	}

	sstream& CodeGenerator::generateQualityDefine(sstream& out, ShaderQuality quality) const {
		out << "#define FILAMENT_QUALITY_LOW    0\n";
		out << "#define FILAMENT_QUALITY_NORMAL 1\n";
		out << "#define FILAMENT_QUALITY_HIGH   2\n";


		out << "#define FILAMENT_QUALITY FILAMENT_QUALITY_HIGH\n";
		return out;
	}

	sstream& CodeGenerator::generateCommon(sstream& out, ShaderStage stage) {

		out << SHADERS_COMMON_MATH_GLSL_DATA;

		switch (stage) {
		case ShaderStage::VERTEX:
			out << SHADERS_COMMON_INSTANCING_GLSL_DATA;
			out << SHADERS_COMMON_SHADOWING_GLSL_DATA;
			break;
		case ShaderStage::FRAGMENT:
			out << SHADERS_COMMON_INSTANCING_GLSL_DATA;
			out << SHADERS_COMMON_SHADOWING_GLSL_DATA;
			out << SHADERS_COMMON_SHADING_FS_DATA;
			out << SHADERS_COMMON_GRAPHICS_FS_DATA;
			out << SHADERS_COMMON_MATERIAL_FS_DATA;
			break;
		case ShaderStage::COMPUTE:
			out << '\n';
			// TODO: figure out if we need some common files here
			break;
		}
		return out;
	}

	sstream& CodeGenerator::generatePostProcessCommon(sstream& out, ShaderStage type) {
		out << SHADERS_COMMON_MATH_GLSL_DATA;
		if (type == ShaderStage::VERTEX) {
		}
		else if (type == ShaderStage::FRAGMENT) {
			out << SHADERS_COMMON_SHADING_FS_DATA;
			out << SHADERS_COMMON_GRAPHICS_FS_DATA;
		}
		return out;
	}

	sstream& CodeGenerator::generateFog(sstream& out, ShaderStage type) {
		if (type == ShaderStage::VERTEX) {
		}
		else if (type == ShaderStage::FRAGMENT) {
			out << SHADERS_FOG_FS_DATA;
		}
		return out;
	}

	sstream& CodeGenerator::generateCommonMaterial(sstream& out, ShaderStage type) {
		if (type == ShaderStage::VERTEX) {
			out << SHADERS_MATERIAL_INPUTS_VS_DATA;
		}
		else if (type == ShaderStage::FRAGMENT) {
			out << SHADERS_MATERIAL_INPUTS_FS_DATA;
		}
		return out;
	}

	sstream& CodeGenerator::generatePostProcessInputs(sstream& out, ShaderStage type) {
		if (type == ShaderStage::VERTEX) {
			out << SHADERS_POST_PROCESS_INPUTS_VS_DATA;
		}
		else if (type == ShaderStage::FRAGMENT) {
			out << SHADERS_POST_PROCESS_INPUTS_FS_DATA;
		}
		return out;
	}

	sstream& CodeGenerator::generatePostProcessGetters(sstream& out, ShaderStage type) {
		out << SHADERS_COMMON_GETTERS_GLSL_DATA;
		if (type == ShaderStage::VERTEX) {
			out << SHADERS_POST_PROCESS_GETTERS_VS_DATA;
		}
		else if (type == ShaderStage::FRAGMENT) {
		}
		return out;
	}

	sstream& CodeGenerator::generateGetters(sstream& out, ShaderStage stage) {
		out << SHADERS_COMMON_GETTERS_GLSL_DATA;
		switch (stage) {
		case ShaderStage::VERTEX:
			out << SHADERS_GETTERS_VS_DATA;
			break;
		case ShaderStage::FRAGMENT:
			out << SHADERS_GETTERS_FS_DATA;
			break;
		case ShaderStage::COMPUTE:
			out << SHADERS_GETTERS_CS_DATA;
			break;
		}
		return out;
	}

	sstream& CodeGenerator::generateParameters(sstream& out, ShaderStage type) {
		if (type == ShaderStage::VERTEX) {
		}
		else if (type == ShaderStage::FRAGMENT) {
			out << SHADERS_SHADING_PARAMETERS_FS_DATA;
		}
		return out;
	}

	sstream& CodeGenerator::generateShaderLit(sstream& out, ShaderStage type,
		Variant variant, Shading shading, bool customSurfaceShading) {
		if (type == ShaderStage::VERTEX) {
		}
		else if (type == ShaderStage::FRAGMENT) {
			out << SHADERS_COMMON_LIGHTING_FS_DATA;
			if (Variant::isShadowReceiverVariant(variant)) {
				out << SHADERS_SHADOWING_FS_DATA;
			}

			// the only reason we have this assert here is that we used to have a check,
			// which seemed unnecessary.
			assert(shading != Shading::UNLIT);

			out << SHADERS_BRDF_FS_DATA;
			switch (shading) {
			case Shading::UNLIT:
				// can't happen
				break;
			case Shading::SPECULAR_GLOSSINESS:
			case Shading::LIT:
				if (customSurfaceShading) {
					out << SHADERS_SHADING_LIT_CUSTOM_FS_DATA;
				}
				else {
					out << SHADERS_SHADING_MODEL_STANDARD_FS_DATA;
				}
				break;
			case Shading::SUBSURFACE:
				out << SHADERS_SHADING_MODEL_SUBSURFACE_FS_DATA;
				break;
			case Shading::CLOTH:
				out << SHADERS_SHADING_MODEL_CLOTH_FS_DATA;
				break;
			}

			out << SHADERS_AMBIENT_OCCLUSION_FS_DATA;
			out << SHADERS_LIGHT_INDIRECT_FS_DATA;

			if (variant.hasDirectionalLighting()) {
				out << SHADERS_LIGHT_DIRECTIONAL_FS_DATA;
			}
			if (variant.hasDynamicLighting()) {
				out << SHADERS_LIGHT_PUNCTUAL_FS_DATA;
			}

			out << SHADERS_SHADING_LIT_FS_DATA;
		}
		return out;
	}

	sstream& CodeGenerator::generateShaderUnlit(sstream& out, ShaderStage type,
		Variant variant, bool hasShadowMultiplier) {
		if (type == ShaderStage::VERTEX) {
		}
		else if (type == ShaderStage::FRAGMENT) {
			if (hasShadowMultiplier) {
				if (Variant::isShadowReceiverVariant(variant)) {
					out << SHADERS_SHADOWING_FS_DATA;
				}
			}
			out << SHADERS_SHADING_UNLIT_FS_DATA;
		}
		return out;
	}

	sstream& CodeGenerator::generateShaderReflections(sstream& out, ShaderStage type) {
		if (type == ShaderStage::VERTEX) {
		}
		else if (type == ShaderStage::FRAGMENT) {
			out << SHADERS_COMMON_LIGHTING_FS_DATA;
			out << SHADERS_LIGHT_REFLECTIONS_FS_DATA;
			out << SHADERS_SHADING_REFLECTIONS_FS_DATA;
		}
		return out;
	}

	/* static */
	char const* CodeGenerator::getConstantName(MaterialBuilder::Property property) noexcept {
		using Property = MaterialBuilder::Property;
		switch (property) {
		case Property::BASE_COLOR:                  return "BASE_COLOR";
		case Property::ROUGHNESS:                   return "ROUGHNESS";
		case Property::METALLIC:                    return "METALLIC";
		case Property::REFLECTANCE:                 return "REFLECTANCE";
		case Property::AMBIENT_OCCLUSION:           return "AMBIENT_OCCLUSION";
		case Property::CLEAR_COAT:                  return "CLEAR_COAT";
		case Property::CLEAR_COAT_ROUGHNESS:        return "CLEAR_COAT_ROUGHNESS";
		case Property::CLEAR_COAT_NORMAL:           return "CLEAR_COAT_NORMAL";
		case Property::ANISOTROPY:                  return "ANISOTROPY";
		case Property::ANISOTROPY_DIRECTION:        return "ANISOTROPY_DIRECTION";
		case Property::THICKNESS:                   return "THICKNESS";
		case Property::SUBSURFACE_POWER:            return "SUBSURFACE_POWER";
		case Property::SUBSURFACE_COLOR:            return "SUBSURFACE_COLOR";
		case Property::SHEEN_COLOR:                 return "SHEEN_COLOR";
		case Property::SHEEN_ROUGHNESS:             return "SHEEN_ROUGHNESS";
		case Property::GLOSSINESS:                  return "GLOSSINESS";
		case Property::SPECULAR_COLOR:              return "SPECULAR_COLOR";
		case Property::EMISSIVE:                    return "EMISSIVE";
		case Property::NORMAL:                      return "NORMAL";
		case Property::POST_LIGHTING_COLOR:         return "POST_LIGHTING_COLOR";
		case Property::POST_LIGHTING_MIX_FACTOR:    return "POST_LIGHTING_MIX_FACTOR";
		case Property::CLIP_SPACE_TRANSFORM:        return "CLIP_SPACE_TRANSFORM";
		case Property::ABSORPTION:                  return "ABSORPTION";
		case Property::TRANSMISSION:                return "TRANSMISSION";
		case Property::IOR:                         return "IOR";
		case Property::MICRO_THICKNESS:             return "MICRO_THICKNESS";
		case Property::BENT_NORMAL:                 return "BENT_NORMAL";
		case Property::SPECULAR_FACTOR:             return "SPECULAR_FACTOR";
		case Property::SPECULAR_COLOR_FACTOR:       return "SPECULAR_COLOR_FACTOR";
		}
	}

	char const* CodeGenerator::getTypeName(UniformType type) noexcept {
		switch (type) {
		case UniformType::BOOL:   return "bool";
		case UniformType::BOOL2:  return "bvec2";
		case UniformType::BOOL3:  return "bvec3";
		case UniformType::BOOL4:  return "bvec4";
		case UniformType::FLOAT:  return "float";
		case UniformType::FLOAT2: return "vec2";
		case UniformType::FLOAT3: return "vec3";
		case UniformType::FLOAT4: return "vec4";
		case UniformType::INT:    return "int";
		case UniformType::INT2:   return "ivec2";
		case UniformType::INT3:   return "ivec3";
		case UniformType::INT4:   return "ivec4";
		case UniformType::UINT:   return "uint";
		case UniformType::UINT2:  return "uvec2";
		case UniformType::UINT3:  return "uvec3";
		case UniformType::UINT4:  return "uvec4";
		case UniformType::MAT3:   return "mat3";
		case UniformType::MAT4:   return "mat4";
		case UniformType::STRUCT: return "";
		}
	}

	char const* CodeGenerator::getUniformTypeName(BufferInterfaceBlock::FieldInfo const& info) noexcept {
		using Type = BufferInterfaceBlock::Type;
		switch (info.type) {
		case Type::STRUCT: return info.structName.c_str();
		default:            return getTypeName(info.type);
		}
	}

	char const* CodeGenerator::getOutputTypeName(MaterialBuilder::OutputType type) noexcept {
		switch (type) {
		case MaterialBuilder::OutputType::FLOAT:  return "float";
		case MaterialBuilder::OutputType::FLOAT2: return "vec2";
		case MaterialBuilder::OutputType::FLOAT3: return "vec3";
		case MaterialBuilder::OutputType::FLOAT4: return "vec4";
		}
	}

	char const* CodeGenerator::getSamplerTypeName(SamplerType type, SamplerFormat format,
		bool multisample) const noexcept {
		switch (type) {
		case SamplerType::SAMPLER_2D:
			switch (format) {
			case SamplerFormat::INT:    return multisample ? "isampler2DMS" : "isampler2D";
			case SamplerFormat::UINT:   return multisample ? "usampler2DMS" : "usampler2D";
			case SamplerFormat::FLOAT:  return multisample ? "sampler2DMS" : "sampler2D";
			case SamplerFormat::SHADOW: return "sampler2DShadow";
			}
		case SamplerType::SAMPLER_3D:
			assert(format != SamplerFormat::SHADOW);
			switch (format) {
			case SamplerFormat::INT:    return "isampler3D";
			case SamplerFormat::UINT:   return "usampler3D";
			case SamplerFormat::FLOAT:  return "sampler3D";
			case SamplerFormat::SHADOW: return nullptr;
			}
		case SamplerType::SAMPLER_2D_ARRAY:
			switch (format) {
			case SamplerFormat::INT:    return multisample ? "isampler2DMSArray" : "isampler2DArray";
			case SamplerFormat::UINT:   return multisample ? "usampler2DMSArray" : "usampler2DArray";
			case SamplerFormat::FLOAT:  return multisample ? "sampler2DMSArray" : "sampler2DArray";
			case SamplerFormat::SHADOW: return "sampler2DArrayShadow";
			}
		case SamplerType::SAMPLER_CUBEMAP:
			switch (format) {
			case SamplerFormat::INT:    return "isamplerCube";
			case SamplerFormat::UINT:   return "usamplerCube";
			case SamplerFormat::FLOAT:  return "samplerCube";
			case SamplerFormat::SHADOW: return "samplerCubeShadow";
			}
		case SamplerType::SAMPLER_EXTERNAL:
			assert(format != SamplerFormat::SHADOW);
			// Vulkan doesn't have external textures in the sense as GL. Vulkan external textures
			// are created via VK_ANDROID_external_memory_android_hardware_buffer, but they are
			// backed by VkImage just like a normal texture, and sampled from normally.
			return  "samplerExternalOES";
		case SamplerType::SAMPLER_CUBEMAP_ARRAY:
			switch (format) {
			case SamplerFormat::INT:    return "isamplerCubeArray";
			case SamplerFormat::UINT:   return "usamplerCubeArray";
			case SamplerFormat::FLOAT:  return "samplerCubeArray";
			case SamplerFormat::SHADOW: return "samplerCubeArrayShadow";
			}
		}
	}

	char const* CodeGenerator::getInterpolationQualifier(Interpolation interpolation) noexcept {
		switch (interpolation) {
		case Interpolation::SMOOTH: return "";
		case Interpolation::FLAT:   return "flat ";
		}
	}

	/* static */
	char const* CodeGenerator::getPrecisionQualifier(Precision precision) noexcept {
		switch (precision) {
		case Precision::LOW:     return "lowp";
		case Precision::MEDIUM:  return "mediump";
		case Precision::HIGH:    return "highp";
		case Precision::DEFAULT: return "";
		}
	}

	/* static */
	bool CodeGenerator::hasPrecision(BufferInterfaceBlock::Type type) noexcept {
		switch (type) {
		case UniformType::BOOL:
		case UniformType::BOOL2:
		case UniformType::BOOL3:
		case UniformType::BOOL4:
		case UniformType::STRUCT:
			return false;
		default:
			return true;
		}
	}

} // namespace filamat
