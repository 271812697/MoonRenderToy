#include "CodeGenerator.h"
#include "MaterialInfo.h"
#include "filaflat/backend/DriverEnums.h"
#include <sstream>
#include <cctype>
#include <iomanip>

#include <assert.h>

namespace filamat {

	// From driverEnum namespace
	using namespace filament;
	using namespace backend;


	std::ostringstream& CodeGenerator::generateSeparator(std::ostringstream& out) {
		out << '\n';
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

	std::ostringstream& CodeGenerator::generateCommonProlog(
        std::ostringstream& out, ShaderStage stage, const std::string& featureDefineStr) const
    {
        out << "#version 330 core\n\n";
        out << featureDefineStr;

        //// precision qualifiers
        //out << '\n';
        //Precision const defaultPrecision = getDefaultPrecision(stage);
        //const char* precision = getPrecisionQualifier(defaultPrecision);
        //out << "precision " << precision << " float;\n";
        //out << "precision " << precision << " int;\n";

        //out << "\n";
        return out;
    }

    std::ostringstream& CodeGenerator::generateCommonEpilog(std::ostringstream& out)
    {
		out << "\n"; // For line compression all shaders finish with a newline character.
		return out;
	}

	std::ostringstream& CodeGenerator::generateSurfaceTypes(std::ostringstream& out, ShaderStage stage) {

		return out;
	}

	std::ostringstream& CodeGenerator::generateSurfaceMain(std::ostringstream& out, ShaderStage stage) {
		return out;
	}

	std::ostringstream& CodeGenerator::generatePostProcessMain(std::ostringstream& out, ShaderStage stage) {

		return out;
	}



	std::ostringstream& CodeGenerator::generateSurfaceShaderInputs(std::ostringstream& out, ShaderStage stage) const {

		if (stage == ShaderStage::VERTEX) {
			out << "\n";

            out << "layout(location = 0) in vec3 VSInputPos;\n";
            out << "layout(location = 1) in vec3 VSInputNormal;\n";
            out << "layout(location = 2) in vec3 VSInputColor;\n" ;
            out << "layout(location = 3) in vec3 VSInputUV;\n";
            out << "layout(location = 4) in vec4 VSIn_InstancePose;\n";
            out << "layout(location = 5) in mat4 VSIn_RotationMatrix;\n" ; 
            out << "layout(location = 9) in vec4 VSIn_IsWorldSpace;\n" ;
			out << "\n";
			
		}

		out << "\n";

		return out;
	}




	std::ostringstream& CodeGenerator::generateSurfaceDepthMain(std::ostringstream& out, ShaderStage stage) {

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

	std::ostringstream& CodeGenerator::generateBuffers(std::ostringstream& out,
		MaterialInfo::BufferContainer const& buffers) const {

		for (auto const* buffer : buffers) {

			// FIXME: we need to get the bindings for the SSBOs and that will depend on the samplers
			backend::descriptor_binding_t binding = 0;

		    // For OpenGL, the set is not used bug the binding must be unique.
		    binding = getUniqueSsboBindingPoint();
			
			generateBufferInterfaceBlock(out, ShaderStage::COMPUTE, binding, *buffer);
		}
		return out;
	}

	std::ostringstream& CodeGenerator::generateUniforms(std::ostringstream& out, ShaderStage stage,
		const BufferInterfaceBlock& uib) const {


		return generateBufferInterfaceBlock(out, stage, getUniqueUboBindingPoint(), uib);
	}

	std::ostringstream& CodeGenerator::generateInterfaceFields(std::ostringstream& out,
		std::vector<BufferInterfaceBlock::FieldInfo> const& infos,
		Precision defaultPrecision) const {
        Precision const uniformPrecision = Precision::HIGH;

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

	std::ostringstream& CodeGenerator::generateUboAsPlainUniforms(std::ostringstream& out, ShaderStage stage,
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

	std::ostringstream& CodeGenerator::generateBufferInterfaceBlock(std::ostringstream& out, ShaderStage stage,
		filament::backend::descriptor_binding_t binding,
		const BufferInterfaceBlock& uib) const {


		auto const& infos = uib.getFieldInfoList();


		std::string blockName{ uib.getName() };
		std::string instanceName{ uib.getName() };
		blockName.front() = char(std::toupper((unsigned char)blockName.front()));
		instanceName.front() = char(std::tolower((unsigned char)instanceName.front()));

		out << "\nlayout(";

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
			//uint8_t qualifiers = uib.getQualifier();
			//while (qualifiers) {
			//	uint8_t const mask = 1u << utils::ctz(unsigned(qualifiers));
			//	switch (BufferInterfaceBlock::Qualifier(qualifiers & mask)) {
			//	case BufferInterfaceBlock::Qualifier::COHERENT:  out << "coherent "; break;
			//	case BufferInterfaceBlock::Qualifier::WRITEONLY: out << "writeonly "; break;
			//	case BufferInterfaceBlock::Qualifier::READONLY:  out << "readonly "; break;
			//	case BufferInterfaceBlock::Qualifier::VOLATILE:  out << "volatile "; break;
			//	case BufferInterfaceBlock::Qualifier::RESTRICT:  out << "restrict "; break;
			//	}
			//	qualifiers &= ~mask;
			//}
		}

		out << "{\n";

		generateInterfaceFields(out, infos, getDefaultPrecision(stage));

		out << "} " << ";\n";

		return out;
	}

	std::ostringstream& CodeGenerator::generateCommonSamplers(std::ostringstream& out,
        filament::SamplerInterfaceBlock::SamplerInfoList const& list, ShaderStage stage) const
    {
		if (list.empty()) {
			return out;
		}

		for (auto const& info : list) {
            uint8_t infoStageFlags = static_cast<uint8_t>(info.stageFlags);
            uint8_t shaderStage = static_cast<uint8_t>(stage);
            if(infoStageFlags & shaderStage )
            {
				auto type = info.type;
				char const* const typeName = getSamplerTypeName(type, info.format, info.multisample);
				char const* const precision = getPrecisionQualifier(info.precision);

				out << "uniform " << precision << " " << typeName << " " << info.uniformName.c_str();
				out << ";\n";
                //out << "uniform bool Has_" << info.uniformName.c_str();
                //out << ";\n";
				            
			}

		}
		out << "\n";

		return out;
	}



	std::ostringstream& CodeGenerator::generateDefine(std::ostringstream& out, const char* name, bool value) {
		if (value) {
			out << "#define " << name << "\n";
		}
		return out;
	}

	std::ostringstream& CodeGenerator::generateDefine(std::ostringstream& out, const char* name, uint32_t value) {
		out << "#define " << name << " " << value << "\n";
		return out;
	}

	std::ostringstream& CodeGenerator::generateDefine(std::ostringstream& out, const char* name, const char* string) {
		out << "#define " << name << " " << string << "\n";
		return out;
	}

	std::ostringstream& CodeGenerator::generateIndexedDefine(std::ostringstream& out, const char* name,
		uint32_t index, uint32_t value) {
		out << "#define " << name << index << " " << value << "\n";
		return out;
	}

	struct SpecializationConstantFormatter {
		std::string operator()(int value) noexcept { return std::to_string(value); }
		std::string operator()(float value) noexcept { return std::to_string(value); }
		std::string operator()(bool value) noexcept { return value ? "true" : "false"; }
	};



	std::ostringstream& CodeGenerator::generateMaterialProperty(std::ostringstream& out,
		MaterialBuilder::Property property, bool isSet) {
		if (isSet) {
			out << "#define " << "MATERIAL_HAS_" << getConstantName(property) << "\n";
		}
		return out;
	}

	std::ostringstream& CodeGenerator::generateQualityDefine(std::ostringstream& out, ShaderQuality quality) const {
	
		return out;
	}

	std::ostringstream& CodeGenerator::generateSurfaceCommon(std::ostringstream& out, ShaderStage stage) {
        static std::string common = R"(
mat4 getTranslateMatrix(vec3 translation) {
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(translation, 1.0)
    );
}

mat4 getScaleMatrix(float scale) {
    return mat4(
        vec4(scale, 0.0, 0.0, 0.0),
        vec4(0.0, scale, 0.0, 0.0),
        vec4(0.0, 0.0, scale, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
}

vec3 ComputeTriangleHeight(const vec2 p0, const vec2 p1, const vec2 p2) {
    // triangle sides
    float a = length(p1 - p2);
    float b = length(p2 - p0);
    float c = length(p1 - p0);

    // triangle angles
    float alpha = acos((b * b + c * c - a * a) / (2.0 * b * c));
    float beta  = acos((a * a + c * c - b * b) / (2.0 * a * c));

    // triangle height (altitude) on each side
    float ha = abs(c * sin(beta));
    float hb = abs(c * sin(alpha));
    float hc = abs(b * sin(alpha));

    return vec3(ha, hb, hc);
}

vec4 unpackIntColor(int p)
{
    float r = float((p >> 24) & 0xFF) / 255.0; 
    float g = (float((p >> 16) & 0xFF)) / 255.0;
    float b = (float((p >> 8) & 0xFF)) / 255.0;
    float a = (float(p & 0xFF)) / 255.0;
    vec4 res = vec4(r,g,b,a);
    return res;
}

vec2 viewportToNDC(vec2 widgetPos, int vpWidth, int vpHeight)
{
    float u = widgetPos.x / (vpWidth - 1);
    float v = widgetPos.y / (vpHeight - 1);

    vec2 ndc;
    ndc.x = 2.0f * u - 1.0f;
    ndc.y = 2.0f * v - 1.0f;

    return ndc;
}

vec2 ndcToScreenPos(vec2 ndc){
    ndc=(ndc+1.0)/2.0;
    return ndc*vec2(ViewportWidth,ViewportHeight);
}

vec2 worldToScreenPos(vec3 pos){
    vec4 clip= ViewProjection *vec4(pos, 1.0f);
    vec2 ndc=clip.xy/clip.w;
    return ndcToScreenPos(ndc);
}

vec3 getViewDir(vec3 pos){
    if(ViewType==0){
        return normalize(vec3(ViewMatrix[0][2],ViewMatrix[1][2],ViewMatrix[2][2]));
    }
    return normalize(EyePosition-pos);  
}

vec3 getOrthViewDir(){
    //Orth
    return vec3(ViewMatrix[0][2],ViewMatrix[1][2],ViewMatrix[2][2]);
}

)";
        out << common;
		return out;
	}

	std::ostringstream& CodeGenerator::generatePostProcessCommon(std::ostringstream& out, ShaderStage stage) {

		return out;
	}

	std::ostringstream& CodeGenerator::generateSurfaceFog(std::ostringstream& out, ShaderStage stage) {

		return out;
	}

	std::ostringstream& CodeGenerator::generateSurfaceMaterial(std::ostringstream& out, ShaderStage stage) {
        static std::string commonfs =R"(
vec3 calculateScreenSpaceNormal(vec3 p) {
    vec3 dx = dFdx(p);
    vec3 dy = -dFdy(p); // not sure if negation is needed for Vulkan
    return normalize(cross(dx, dy));
}

float blinnPhong(vec3 l,vec3 v,vec3 n){
     vec3 h=normalize(l+v);
    
     float diffuse=max(dot(n,l),0.0);
     float specular=pow(max(dot(n,h),0.0),100);
    
     return diffuse+specular*0.2;
}

vec3 mulBlinnPhong(mat4 LKitColor,mat4 LKitPosition,vec3 WorldPos,vec3 V,vec3 N)
{
    vec3 Lo = vec3(0.0);
    float ek=blinnPhong(normalize(LKitPosition[0].xyz-WorldPos),V,N);
    float ef=blinnPhong(normalize(LKitPosition[1].xyz-WorldPos),V,N);
    float eb1=blinnPhong(normalize(LKitPosition[2].xyz-WorldPos),V,N);
    float eb2=blinnPhong(normalize(LKitPosition[3].xyz-WorldPos),V,N);
    Lo+= LKitColor[0].w*ek* LKitColor[0].xyz;
    Lo+=  LKitColor[1].w*ef* LKitColor[1].xyz;
    Lo+=  LKitColor[2].w*eb1* LKitColor[2].xyz;
    Lo+=  LKitColor[3].w*eb2* LKitColor[3].xyz;
    return Lo;
}

bool doesNeedLightShading(int shadeModel)
{
    return (shadeModel == 1 || shadeModel == 2 || shadeModel == 4 );
}

struct LightShading_SurfaceParameter
{
    vec3 worldPosition;
    uint shadeModel;
    vec3 color;
    vec3 norm;
};

struct LightShading_Parameter
{
vec3 lightDir;
vec4 LightColor;
mat4 LightKitColor;
mat4 LightKitPosition;
};

vec3 lightColorShading(LightShading_SurfaceParameter surface, LightShading_Parameter lightParams, vec3 viewDir)
{
    vec3 outColor;
    if(surface.shadeModel==1u)
    {
        outColor=surface.color;
    }
    else
    {   
        outColor = 0.75 * blinnPhong(lightParams.lightDir, viewDir, surface.norm)* lightParams.LightColor.xyz;;
        outColor += mulBlinnPhong(lightParams.LightKitColor, lightParams.LightKitPosition, surface.worldPosition, viewDir, surface.norm);
        outColor = outColor * surface.color;   
    }
    
    return outColor;
}

vec3 shadeFaceAndWire(out float outEmix, vec3 faceColor, vec3 lineColor, float lineWidth, float opacityFactor, vec4 edgeEqn1,vec4 edgeEqn2, vec4 edgeEqn3)
{
    vec4 edgeEqn[3];
    edgeEqn[0] = edgeEqn1;
    edgeEqn[1] = edgeEqn2;
    edgeEqn[2] = edgeEqn3;

    float edist[3];
    edist[0] = dot(edgeEqn[0].xy, gl_FragCoord.xy) + edgeEqn[0].w;
    edist[1] = dot(edgeEqn[1].xy, gl_FragCoord.xy) + edgeEqn[1].w;
    edist[2] = dot(edgeEqn[2].xy, gl_FragCoord.xy) + edgeEqn[2].w;

    if (edist[0] < -0.5 && edgeEqn[0].z > 0.0) discard;
    if (edist[1] < -0.5 && edgeEqn[1].z > 0.0) discard;
    if (edist[2] < -0.5 && edgeEqn[2].z > 0.0) discard;
    edist[0] += edgeEqn[0].z;
    edist[1] += edgeEqn[1].z;
    edist[2] += edgeEqn[2].z;

    float emix0 = clamp(0.5 + 0.5 * lineWidth - edist[0], 0.0, 1.0);
    float emix1 = clamp(0.5 + 0.5 * lineWidth - edist[1], 0.0, 1.0);
    float emix2 = clamp(0.5 + 0.5 * lineWidth - edist[2], 0.0, 1.0);
    float emix = emix1;
    emix = mix(emix, max(emix, emix0), step(edgeEqn[0].z, 0.01));
    emix = mix(emix, max(emix, emix2), step(edgeEqn[2].z, 0.01));

    vec3 edgeColor = mix(faceColor, lineColor, opacityFactor);
    vec3 finalColor = mix(faceColor, edgeColor, emix * opacityFactor);

    outEmix = emix;

    return finalColor;  
}



)";
        out << commonfs;
		return out;
	}

	std::ostringstream& CodeGenerator::generatePostProcessInputs(std::ostringstream& out, ShaderStage stage) {

		return out;
	}

	std::ostringstream& CodeGenerator::generatePostProcessGetters(std::ostringstream& out, ShaderStage stage) {

		return out;
	}

	std::ostringstream& CodeGenerator::generateSurfaceGetters(std::ostringstream& out, ShaderStage stage) {

		return out;
	}

	std::ostringstream& CodeGenerator::generateSurfaceParameters(std::ostringstream& out, ShaderStage stage) {

		return out;
	}



	std::ostringstream& CodeGenerator::generateSurfaceReflections(std::ostringstream& out,
		ShaderStage stage) {

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
		case Property::SHADOW_STRENGTH:             return "SHADOW_STRENGTH";
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
        case SamplerType::SAMPLER_BUFFER:
            switch( format )
            {
            case SamplerFormat::INT:
                return  "isamplerBuffer" ;
            case SamplerFormat::UINT:
                return  "usamplerBuffer" ;
            case SamplerFormat::FLOAT:
                return  "samplerBuffer";
            }

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
			return (mTargetLanguage == TargetLanguage::SPIRV) ? "sampler2D" : "samplerExternalOES";
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
