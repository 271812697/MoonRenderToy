#include <cstring>
#include "core/log.h"
#include "MathUtil.h"
#include "LoadScene.h"
#include "Scene.h"
#include "Camera.h"
#include "Texture.h"
#include "Material.h"
#include "Renderer.h"
#include "renderer/Context.h"
#include "Core/Global/ServiceLocator.h"
#include "Core/ECS/Components/CMaterialRenderer.h"
#include <Core/ResourceManagement/MaterialManager.h>
#include <Core/ResourceManagement/ModelManager.h>
#include <Core/ResourceManagement/ShaderManager.h>

#include <Core/ResourceManagement/TextureManager.h>
#include <Core/SceneSystem/SceneManager.h>
#include <tinygltf/include/tiny_gltf.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
namespace PathTrace
{
	static const int kMaxLineLength = 2048;

	bool LoadSceneFromFile(const std::string& filename, Scene* scene, RenderOptions& renderOptions)
	{

		scene->setPath(filename);
		FILE* file;
		file = fopen(filename.c_str(), "r");

		if (!file)
		{
			CORE_ERROR("Couldn't open {0} for reading\n", filename.c_str());
			return false;
		}
		CORE_INFO("Loading Scene..\n");

		struct MaterialData
		{
			Material mat;
			int id;
		};
		std::map<std::string, MaterialData> materialMap;
		std::map<std::string, int>InstanceMap;
		std::vector<std::string> albedoTex;
		std::vector<std::string> metallicRoughnessTex;
		std::vector<std::string> normalTex;
		std::string path = filename.substr(0, filename.find_last_of("/\\")) + "/";
		int materialCount = 0;
		char line[kMaxLineLength];

		//Defaults
		Material defaultMat;
		scene->AddMaterial(defaultMat);

		while (fgets(line, kMaxLineLength, file))
		{
			// skip comments
			if (line[0] == '#')
				continue;

			// name used for materials and meshes
			char name[kMaxLineLength] = { 0 };

			//--------------------------------------------
			// Material

			if (sscanf(line, " material %s", name) == 1)
			{
				Material material;
				char albedoTexName[100] = "none";
				char metallicRoughnessTexName[100] = "none";
				char normalTexName[100] = "none";
				char emissionTexName[100] = "none";
				char alphaMode[20] = "none";
				char mediumType[20] = "none";

				while (fgets(line, kMaxLineLength, file))
				{
					// end group
					if (strchr(line, '}'))
						break;

					sscanf(line, " color %f %f %f", &material.baseColor.x, &material.baseColor.y, &material.baseColor.z);
					sscanf(line, " opacity %f", &material.opacity);
					sscanf(line, " alphamode %s", alphaMode);
					sscanf(line, " alphacutoff %f", &material.alphaCutoff);
					sscanf(line, " emission %f %f %f", &material.emission.x, &material.emission.y, &material.emission.z);
					sscanf(line, " metallic %f", &material.metallic);
					sscanf(line, " roughness %f", &material.roughness);
					sscanf(line, " subsurface %f", &material.subsurface);
					sscanf(line, " speculartint %f", &material.specularTint);
					sscanf(line, " anisotropic %f", &material.anisotropic);
					sscanf(line, " sheen %f", &material.sheen);
					sscanf(line, " sheentint %f", &material.sheenTint);
					sscanf(line, " clearcoat %f", &material.clearcoat);
					sscanf(line, " clearcoatgloss %f", &material.clearcoatGloss);
					sscanf(line, " spectrans %f", &material.specTrans);
					sscanf(line, " ior %f", &material.ior);
					sscanf(line, " albedotexture %s", albedoTexName);
					sscanf(line, " metallicroughnesstexture %s", metallicRoughnessTexName);
					sscanf(line, " normaltexture %s", normalTexName);
					sscanf(line, " emissiontexture %s", emissionTexName);
					sscanf(line, " mediumtype %s", mediumType);
					sscanf(line, " mediumdensity %f", &material.mediumDensity);
					sscanf(line, " mediumcolor %f %f %f", &material.mediumColor.x, &material.mediumColor.y, &material.mediumColor.z);
					sscanf(line, " mediumanisotropy %f", &material.mediumAnisotropy);
				}

				// Albedo Texture
				if (strcmp(albedoTexName, "none") != 0)
					material.baseColorTexId = scene->AddTexture(path + albedoTexName);

				// MetallicRoughness Texture
				if (strcmp(metallicRoughnessTexName, "none") != 0)
					material.metallicRoughnessTexID = scene->AddTexture(path + metallicRoughnessTexName);

				// Normal Map Texture
				if (strcmp(normalTexName, "none") != 0)
					material.normalmapTexID = scene->AddTexture(path + normalTexName);

				// Emission Map Texture
				if (strcmp(emissionTexName, "none") != 0)
					material.emissionmapTexID = scene->AddTexture(path + emissionTexName);

				// AlphaMode
				if (strcmp(alphaMode, "opaque") == 0)
					material.alphaMode = AlphaMode::Opaque;
				else if (strcmp(alphaMode, "blend") == 0)
					material.alphaMode = AlphaMode::Blend;
				else if (strcmp(alphaMode, "mask") == 0)
					material.alphaMode = AlphaMode::Mask;

				// MediumType
				if (strcmp(mediumType, "absorb") == 0)
					material.mediumType = MediumType::Absorb;
				else if (strcmp(mediumType, "scatter") == 0)
					material.mediumType = MediumType::Scatter;
				else if (strcmp(mediumType, "emissive") == 0)
					material.mediumType = MediumType::Emissive;

				// add material to map
				if (materialMap.find(name) == materialMap.end()) // New material
				{
					int id = scene->AddMaterial(material);
					materialMap[name] = MaterialData{ material, id };
				}
			}

			//--------------------------------------------
			// Light

			if (strstr(line, "light"))
			{
				Light light;
				Vec3 v1, v2;
				char lightType[20] = "none";

				while (fgets(line, kMaxLineLength, file))
				{
					// end group
					if (strchr(line, '}'))
						break;

					sscanf(line, " position %f %f %f", &light.position.x, &light.position.y, &light.position.z);
					sscanf(line, " emission %f %f %f", &light.emission.x, &light.emission.y, &light.emission.z);

					sscanf(line, " radius %f", &light.radius);
					sscanf(line, " v1 %f %f %f", &v1.x, &v1.y, &v1.z);
					sscanf(line, " v2 %f %f %f", &v2.x, &v2.y, &v2.z);
					sscanf(line, " type %s", lightType);
				}

				if (strcmp(lightType, "quad") == 0)
				{
					light.type = LightType::RectLight;
					light.u = v1 - light.position;
					light.v = v2 - light.position;
					light.area = Vec3::Length(Vec3::Cross(light.u, light.v));
				}
				else if (strcmp(lightType, "sphere") == 0)
				{
					light.type = LightType::SphereLight;
					light.area = 4.0f * PI * light.radius * light.radius;
				}
				else if (strcmp(lightType, "distant") == 0)
				{
					light.type = LightType::DistantLight;
					light.area = 0.0f;
				}

				scene->AddLight(light);
			}

			//--------------------------------------------
			// Camera

			if (strstr(line, "camera"))
			{
				Mat4 xform;
				Vec3 position;
				Vec3 lookAt;
				float fov;
				float aperture = 0, focalDist = 1;
				bool matrixPrided = false;

				while (fgets(line, kMaxLineLength, file))
				{
					// end group
					if (strchr(line, '}'))
						break;

					sscanf(line, " position %f %f %f", &position.x, &position.y, &position.z);
					sscanf(line, " lookat %f %f %f", &lookAt.x, &lookAt.y, &lookAt.z);
					sscanf(line, " aperture %f ", &aperture);
					sscanf(line, " focaldist %f", &focalDist);
					sscanf(line, " fov %f", &fov);

					if (sscanf(line, " matrix %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
						&xform[0][0], &xform[1][0], &xform[2][0], &xform[3][0],
						&xform[0][1], &xform[1][1], &xform[2][1], &xform[3][1],
						&xform[0][2], &xform[1][2], &xform[2][2], &xform[3][2],
						&xform[0][3], &xform[1][3], &xform[2][3], &xform[3][3]
					) != 0)
						matrixPrided = true;
				}

				delete scene->getCamera();

				if (matrixPrided)
				{
					Vec3 forward = Vec3(xform[2][0], xform[2][1], xform[2][2]);
					position = Vec3(xform[3][0], xform[3][1], xform[3][2]);
					lookAt = position + forward;
				}

				scene->AddCamera(position, lookAt, fov);
				scene->getCamera()->setAperture(aperture);
				scene->getCamera()->setFocalDist(focalDist);
			}

			//--------------------------------------------
			// Renderer

			if (strstr(line, "renderer"))
			{
				char envMap[200] = "none";
				char enableRR[10] = "none";
				char enableAces[10] = "none";
				char openglNormalMap[10] = "none";
				char hideEmitters[10] = "none";
				char transparentBackground[10] = "none";
				char enableBackground[10] = "none";
				char independentRenderSize[10] = "none";
				char enableTonemap[10] = "none";
				char enableRoughnessMollification[10] = "none";
				char enableVolumeMIS[10] = "none";
				char enableUniformLight[10] = "none";

				while (fgets(line, kMaxLineLength, file))
				{
					// end group
					if (strchr(line, '}'))
						break;

					sscanf(line, " envmapfile %s", envMap);
					sscanf(line, " resolution %d %d", &renderOptions.renderResolution.x, &renderOptions.renderResolution.y);
					sscanf(line, " windowresolution %d %d", &renderOptions.windowResolution.x, &renderOptions.windowResolution.y);
					sscanf(line, " envmapintensity %f", &renderOptions.envMapIntensity);
					sscanf(line, " maxdepth %i", &renderOptions.maxDepth);
					sscanf(line, " maxspp %i", &renderOptions.maxSpp);
					sscanf(line, " tilewidth %i", &renderOptions.tileWidth);
					sscanf(line, " tileheight %i", &renderOptions.tileHeight);
					sscanf(line, " enablerr %s", enableRR);
					sscanf(line, " rrdepth %i", &renderOptions.RRDepth);
					sscanf(line, " enabletonemap %s", enableTonemap);
					sscanf(line, " enableaces %s", enableAces);
					sscanf(line, " texarraywidth %i", &renderOptions.texArrayWidth);
					sscanf(line, " texarrayheight %i", &renderOptions.texArrayHeight);
					sscanf(line, " openglnormalmap %s", openglNormalMap);
					sscanf(line, " hideemitters %s", hideEmitters);
					sscanf(line, " enablebackground %s", enableBackground);
					sscanf(line, " transparentbackground %s", transparentBackground);
					sscanf(line, " backgroundcolor %f %f %f", &renderOptions.backgroundCol.x, &renderOptions.backgroundCol.y, &renderOptions.backgroundCol.z);
					sscanf(line, " independentrendersize %s", independentRenderSize);
					sscanf(line, " envmaprotation %f", &renderOptions.envMapRot);
					sscanf(line, " enableroughnessmollification %s", enableRoughnessMollification);
					sscanf(line, " roughnessmollificationamt %f", &renderOptions.roughnessMollificationAmt);
					sscanf(line, " enablevolumemis %s", enableVolumeMIS);
					sscanf(line, " enableuniformlight %s", enableUniformLight);
					sscanf(line, " uniformlightcolor %f %f %f", &renderOptions.uniformLightCol.x, &renderOptions.uniformLightCol.y, &renderOptions.uniformLightCol.z);
				}

				if (strcmp(envMap, "none") != 0)
				{
					scene->AddEnvMap(path + envMap);
					renderOptions.enableEnvMap = true;
				}
				else
					renderOptions.enableEnvMap = false;

				if (strcmp(enableAces, "false") == 0)
					renderOptions.enableAces = false;
				else if (strcmp(enableAces, "true") == 0)
					renderOptions.enableAces = true;

				if (strcmp(enableRR, "false") == 0)
					renderOptions.enableRR = false;
				else if (strcmp(enableRR, "true") == 0)
					renderOptions.enableRR = true;

				if (strcmp(openglNormalMap, "false") == 0)
					renderOptions.openglNormalMap = false;
				else if (strcmp(openglNormalMap, "true") == 0)
					renderOptions.openglNormalMap = true;

				if (strcmp(hideEmitters, "false") == 0)
					renderOptions.hideEmitters = false;
				else if (strcmp(hideEmitters, "true") == 0)
					renderOptions.hideEmitters = true;

				if (strcmp(enableBackground, "false") == 0)
					renderOptions.enableBackground = false;
				else if (strcmp(enableBackground, "true") == 0)
					renderOptions.enableBackground = true;

				if (strcmp(transparentBackground, "false") == 0)
					renderOptions.transparentBackground = false;
				else if (strcmp(transparentBackground, "true") == 0)
					renderOptions.transparentBackground = true;

				if (strcmp(independentRenderSize, "false") == 0)
					renderOptions.independentRenderSize = false;
				else if (strcmp(independentRenderSize, "true") == 0)
					renderOptions.independentRenderSize = true;

				if (strcmp(enableTonemap, "false") == 0)
					renderOptions.enableTonemap = false;
				else if (strcmp(enableTonemap, "true") == 0)
					renderOptions.enableTonemap = true;

				if (strcmp(enableRoughnessMollification, "false") == 0)
					renderOptions.enableRoughnessMollification = false;
				else if (strcmp(enableRoughnessMollification, "true") == 0)
					renderOptions.enableRoughnessMollification = true;

				if (strcmp(enableVolumeMIS, "false") == 0)
					renderOptions.enableVolumeMIS = false;
				else if (strcmp(enableVolumeMIS, "true") == 0)
					renderOptions.enableVolumeMIS = true;

				if (strcmp(enableUniformLight, "false") == 0)
					renderOptions.enableUniformLight = false;
				else if (strcmp(enableUniformLight, "true") == 0)
					renderOptions.enableUniformLight = true;

				if (!renderOptions.independentRenderSize)
					renderOptions.windowResolution = renderOptions.renderResolution;
			}


			//--------------------------------------------
			// Mesh

			if (strstr(line, "mesh"))
			{
				std::string filename;
				Vec4 rotQuat;
				Mat4 xform, translate, rot, scale;

				int material_id = 0; // Default Material ID
				int parentid = -1;
				char meshName[200] = "none";
				char parentName[200] = "none";
				bool matrixPrided = false;

				while (fgets(line, kMaxLineLength, file))
				{
					// end group
					if (strchr(line, '}'))
						break;

					char file[2048];
					char matName[100];

					sscanf(line, " name %[^\t\n]s", meshName);
					if (sscanf(line, " parent %s", parentName) == 1)
					{
						//in this case parent must prior to this meshinstance
						parentid = InstanceMap[parentName];

					}
					if (sscanf(line, " file %s", file) == 1)
						filename = path + file;

					if (sscanf(line, " material %s", matName) == 1)
					{
						// look up material in dictionary
						if (materialMap.find(matName) != materialMap.end())
							material_id = materialMap[matName].id;
						else
							printf("Could not find material %s\n", matName);
					}

					if (sscanf(line, " matrix %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
						&xform[0][0], &xform[1][0], &xform[2][0], &xform[3][0],
						&xform[0][1], &xform[1][1], &xform[2][1], &xform[3][1],
						&xform[0][2], &xform[1][2], &xform[2][2], &xform[3][2],
						&xform[0][3], &xform[1][3], &xform[2][3], &xform[3][3]
					) != 0)
						matrixPrided = true;

					sscanf(line, " position %f %f %f", &translate.data[3][0], &translate.data[3][1], &translate.data[3][2]);
					sscanf(line, " scale %f %f %f", &scale.data[0][0], &scale.data[1][1], &scale.data[2][2]);
					if (sscanf(line, " rotation %f %f %f %f", &rotQuat.x, &rotQuat.y, &rotQuat.z, &rotQuat.w) != 0)
						rot = Mat4::QuatToMatrix(rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w);
				}

				if (!filename.empty())
				{

					int mesh_id = scene->AddMesh(filename);
					if (mesh_id != -1)
					{
						std::string instanceName;

						if (strcmp(meshName, "none") != 0)
							instanceName = std::string(meshName);
						else
						{
							std::size_t pos = filename.find_last_of("/\\");
							instanceName = filename.substr(pos + 1);
						}

						Mat4 transformMat;

						if (matrixPrided)
							transformMat = xform;
						else
							transformMat = scale * rot * translate;

						MeshInstance instance(instanceName, mesh_id, transformMat, material_id);
						instance.localform = transformMat;
						instance.parentID = parentid;
						InstanceMap[instanceName] = scene->AddMeshInstance(instance);
					}
				}
			}

			//--------------------------------------------
			// GLTF

			if (strstr(line, "gltf"))
			{
				std::string filename;
				Vec4 rotQuat;
				Mat4 xform, translate, rot, scale;
				bool matrixPrided = false;

				while (fgets(line, kMaxLineLength, file))
				{
					// end group
					if (strchr(line, '}'))
						break;

					char file[2048];

					if (sscanf(line, " file %s", file) == 1)
						filename = path + file;

					if (sscanf(line, " matrix %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
						&xform[0][0], &xform[1][0], &xform[2][0], &xform[3][0],
						&xform[0][1], &xform[1][1], &xform[2][1], &xform[3][1],
						&xform[0][2], &xform[1][2], &xform[2][2], &xform[3][2],
						&xform[0][3], &xform[1][3], &xform[2][3], &xform[3][3]
					) != 0)
						matrixPrided = true;

					sscanf(line, " position %f %f %f", &translate.data[3][0], &translate.data[3][1], &translate.data[3][2]);
					sscanf(line, " scale %f %f %f", &scale.data[0][0], &scale.data[1][1], &scale.data[2][2]);
					if (sscanf(line, " rotation %f %f %f %f", &rotQuat.x, &rotQuat.y, &rotQuat.z, &rotQuat.w) != 0)
						rot = Mat4::QuatToMatrix(rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w);
				}

				if (!filename.empty())
				{
					std::string ext = filename.substr(filename.find_last_of(".") + 1);

					bool success = false;
					Mat4 transformMat;

					if (matrixPrided)
						transformMat = xform;
					else
						transformMat = scale * rot * translate;

					// TODO: Add support for instancing.
					// If the same gltf is loaded multiple times then mesh data gets duplicated
					if (ext == "gltf")
						success = LoadGLTF(filename, scene, renderOptions, transformMat, false);
					else if (ext == "glb")
						success = LoadGLTF(filename, scene, renderOptions, transformMat, true);

					if (!success)
					{
						printf("Unable to load gltf %s\n", filename.c_str());
						exit(0);
					}
				}
			}
		}

		fclose(file);

		return true;
	}

	bool LoadSingleModel(const std::string& filename, Scene* scene)
	{
		Assimp::Importer importer;
		const aiScene* ascene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_GenSmoothNormals);
		if (!ascene || ascene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !ascene->mRootNode) {
			CORE_ERROR("Assimp parse Error for {0}\n", importer.GetErrorString());
			return false;
		}
		//for (int i = 0;i < ascene->mNumMaterials;i++) {
		//	aiMaterial* material=ascene->mMaterials[i];
		//
		//}
		;
		Material mate;
		//mate.baseColor = { 1.0,1.0,0.0 };
		int mateId = scene->AddMaterial(mate);

		aiMatrix4x4 mat;
		std::vector<aiMatrix4x4>matStack = { mat };
		std::vector<aiNode*>nodeStack = { ascene->mRootNode };
		while (!nodeStack.empty()) {
			aiNode* node = nodeStack.back(); nodeStack.pop_back();
			aiMatrix4x4 parentMat = matStack.back(); matStack.pop_back();
			aiMatrix4x4 transMat = parentMat * node->mTransformation;

			for (int i = 0; i < node->mNumMeshes; i++) {
				aiMesh* mesh = ascene->mMeshes[node->mMeshes[i]];

				Mesh* pathTraceMesh = new Mesh();
				std::vector<Vec4> posArr;
				std::vector<Vec4>norArr;
				for (int k = 0; k < mesh->mNumVertices; k++) {
					const aiVector3D pos = mesh->mVertices[k];
					const aiVector3D nor = mesh->mNormals[k];
					posArr.push_back({ pos.x,pos.y,pos.z,1.0 });
					norArr.push_back({ nor.x,nor.y,nor.z,1.0 });
					//pathTraceMesh->verticesUVX.push_back({pos.x,pos.y,pos.z,1.0});
					//pathTraceMesh->normalsUVY.push_back({ nor.x,nor.y,nor.z,1.0 });
				}
				for (uint32_t faceID = 0; faceID < mesh->mNumFaces; ++faceID)
				{
					auto& face = mesh->mFaces[faceID];

					for (size_t indexID = 0; indexID < 3; ++indexID)
					{
						pathTraceMesh->getPosUvX().push_back(posArr[face.mIndices[indexID]]);
						pathTraceMesh->getNorUvY().push_back(norArr[face.mIndices[indexID]]);
					}

				}
				;
				int meshId = scene->getMeshes().size();
				scene->getMeshes().push_back(pathTraceMesh);
				Mat4  xform;
				memcpy(xform.data, &transMat.a1, 16 * sizeof(float));
				MeshInstance ins(std::string(mesh->mName.C_Str()), meshId, xform, mateId);
				scene->AddMeshInstance(ins);
			}
			for (int i = 0; i < node->mNumChildren; i++) {
				matStack.push_back(transMat);
				nodeStack.push_back(node->mChildren[i]);
			}
		}

		//const aiSce
		return true;
	}

	bool LoadSceneFromFile(const std::string& filename, Core::SceneSystem::Scene* scene)
	{
		FILE* file;
		file = fopen(filename.c_str(), "r");

		if (!file)
		{
			CORE_ERROR("Couldn't open {0} for reading\n", filename.c_str());
			return false;
		}
		CORE_INFO("Loading Scene..\n");
		struct MaterialData
		{
			Material mat;
			int id;
		};

		std::string path = filename.substr(0, filename.find_last_of("/\\")) + "/";
		int materialCount = 0;
		char line[kMaxLineLength];
		while (fgets(line, kMaxLineLength, file))
		{
			// skip comments
			if (line[0] == '#')
				continue;
			// name used for materials and meshes
			char name[kMaxLineLength] = { 0 };
			if (sscanf(line, " material %s", name) == 1)
			{
				struct aMaterial
				{
					aMaterial()
					{
						baseColorR = baseColorG = baseColorB = 1.0f;
						//各向异性
						anisotropic = 0.0f;

						emissionR = emissionG = emissionB = 0.0f;
						// padding1

						metallic = 0.0f;
						roughness = 0.5f;
						subsurface = 0.0f;
						specularTint = 0.0f;

						sheen = 0.0f;
						sheenTint = 0.0f;
						clearcoat = 0.0f;
						clearcoatGloss = 0.0f;

						specTrans = 0.0f;
						ior = 1.5f;
						//介质类型
						mediumType = 0.0f;
						mediumDensity = 0.0f;

						mediumColor = Vec3(1.0f, 1.0f, 1.0f);
						//各向同性
						mediumAnisotropy = 0.0f;

						baseColorTexId = -1.0f;
						metallicRoughnessTexID = -1.0f;
						normalmapTexID = -1.0f;
						emissionmapTexID = -1.0f;

						opacity = 1.0f;
						alphaMode = 0.0f;
						alphaCutoff = 0.0f;
						// padding2
					};
					float  baseColorR;
					float  baseColorG;
					float  baseColorB;
					float anisotropic;

					float emissionR;
					float emissionG;
					float emissionB;
					float ax;

					float metallic;
					float roughness;
					float subsurface;
					float specularTint;

					float sheen;
					float sheenTint;
					float clearcoat;
					float clearcoatGloss;

					float specTrans;
					float ior;
					float mediumType;
					float mediumDensity;

					Vec3 mediumColor;
					float mediumAnisotropy;

					float baseColorTexId;
					float metallicRoughnessTexID;
					float normalmapTexID;
					float emissionmapTexID;

					float opacity;
					float alphaMode;
					float alphaCutoff;
					float ay;
				};
				aMaterial material;
				char albedoTexName[100] = "none";
				char metallicRoughnessTexName[100] = "none";
				char normalTexName[100] = "none";
				char emissionTexName[100] = "none";
				char alphaMode[20] = "none";
				char mediumType[20] = "none";

				while (fgets(line, kMaxLineLength, file))
				{
					// end group
					if (strchr(line, '}'))
						break;

					sscanf(line, " color %f %f %f", &material.baseColorR, &material.baseColorG, &material.baseColorB);
					sscanf(line, " opacity %f", &material.opacity);
					sscanf(line, " alphamode %s", alphaMode);
					sscanf(line, " alphacutoff %f", &material.alphaCutoff);
					sscanf(line, " emission %f %f %f", &material.emissionR, &material.emissionG, &material.emissionB);
					sscanf(line, " metallic %f", &material.metallic);
					sscanf(line, " roughness %f", &material.roughness);
					sscanf(line, " subsurface %f", &material.subsurface);
					sscanf(line, " speculartint %f", &material.specularTint);
					sscanf(line, " anisotropic %f", &material.anisotropic);
					sscanf(line, " sheen %f", &material.sheen);
					sscanf(line, " sheentint %f", &material.sheenTint);
					sscanf(line, " clearcoat %f", &material.clearcoat);
					sscanf(line, " clearcoatgloss %f", &material.clearcoatGloss);
					sscanf(line, " spectrans %f", &material.specTrans);
					sscanf(line, " ior %f", &material.ior);
					sscanf(line, " albedotexture %s", albedoTexName);
					sscanf(line, " metallicroughnesstexture %s", metallicRoughnessTexName);
					sscanf(line, " normaltexture %s", normalTexName);
					sscanf(line, " emissiontexture %s", emissionTexName);
					sscanf(line, " mediumtype %s", mediumType);
					sscanf(line, " mediumdensity %f", &material.mediumDensity);
					sscanf(line, " mediumcolor %f %f %f", &material.mediumColor.x, &material.mediumColor.y, &material.mediumColor.z);
					sscanf(line, " mediumanisotropy %f", &material.mediumAnisotropy);
				}
				Core::Resources::Material* tempMat = new Core::Resources::Material();
				Core::Global::ServiceLocator::Get<Core::ResourceManagement::MaterialManager>().RegisterResource(name, tempMat);
				tempMat->SetShader(Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager[":Shaders\\StandardSurfaceWithEdge.ovfx"]);
				tempMat->SetProperty("u_Albedo", Maths::FVector4{ material.baseColorR, material.baseColorG, material.baseColorB, material.opacity });
				tempMat->SetProperty("u_EmissiveColor", Maths::FVector3{ material.emissionR, material.emissionG, material.emissionB });
				tempMat->SetProperty("u_Metallic", material.metallic);
				tempMat->SetProperty("u_Roughness", material.roughness);
				tempMat->SetProperty("u_RefractionIndex", material.ior);
				tempMat->SetProperty("u_EmissiveIntensity", 1.0f);
				//tempMat->SetProperty("u_Diffuse", Maths::FVector4{ 1.0,  1.0, 1.0, 1.0f });
				tempMat->SetBackfaceCulling(false);;
				tempMat->SetCastShadows(false);
				tempMat->SetReceiveShadows(false);
				// Albedo Texture
				auto albedo = Core::Global::ServiceLocator::Get<Core::ResourceManagement::TextureManager>().GetResource(path + albedoTexName, true);
				tempMat->SetProperty("u_AlbedoMap", albedo);


				// MetallicRoughness Texture
				auto roughness = Core::Global::ServiceLocator::Get<Core::ResourceManagement::TextureManager>().GetResource(path + metallicRoughnessTexName, true);
				tempMat->SetProperty("u_RoughnessMap", roughness);
				auto emission = Core::Global::ServiceLocator::Get<Core::ResourceManagement::TextureManager>().GetResource(path + emissionTexName, true);
				tempMat->SetProperty("u_EmissiveMap", roughness);
				auto metallicMap = Core::Global::ServiceLocator::Get<Core::ResourceManagement::TextureManager>().GetResource(path + metallicRoughnessTexName, true);
				tempMat->SetProperty("u_MetallicMap", metallicMap);
				// Normal Map Texture
				auto normalTex = Core::Global::ServiceLocator::Get<Core::ResourceManagement::TextureManager>().GetResource(path + normalTexName, true);
				tempMat->SetProperty("u_NormalMap", normalTex);
				if (normalTex)
					tempMat->AddFeature("NORMAL_MAPPING");


				// AlphaMode
				if (strcmp(alphaMode, "opaque") == 0)
					material.alphaMode = AlphaMode::Opaque;
				else if (strcmp(alphaMode, "blend") == 0)
					material.alphaMode = AlphaMode::Blend;
				else if (strcmp(alphaMode, "mask") == 0)
					material.alphaMode = AlphaMode::Mask;

				// MediumType
				if (strcmp(mediumType, "absorb") == 0)
					material.mediumType = MediumType::Absorb;
				else if (strcmp(mediumType, "scatter") == 0)
				{
					material.mediumType = MediumType::Scatter;
					tempMat->AddFeature("SPECULAR_WORKFLOW");
				}

				else if (strcmp(mediumType, "emissive") == 0)
					material.mediumType = MediumType::Emissive;

			}
			// Mesh
			if (strstr(line, "mesh"))
			{
				std::string filename;
				Vec4 rotQuat;
				Mat4 xform, translate, rot, scale;

				int material_id = 0; // Default Material ID
				int parentid = -1;
				char meshName[200] = "none";
				char parentName[200] = "none";
				bool matrixPrided = false;
				char matName[100];
				while (fgets(line, kMaxLineLength, file))
				{
					// end group
					if (strchr(line, '}'))
						break;

					char file[2048];


					sscanf(line, " name %[^\t\n]s", meshName);
					if (sscanf(line, " parent %s", parentName) == 1)
					{
						//in this case parent must prior to this meshinstance
						//parentid = InstanceMap[parentName];

					}
					if (sscanf(line, " file %s", file) == 1)
						filename = path + file;

					if (sscanf(line, " material %s", matName) == 1)
					{

					}

					if (sscanf(line, " matrix %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
						&xform[0][0], &xform[1][0], &xform[2][0], &xform[3][0],
						&xform[0][1], &xform[1][1], &xform[2][1], &xform[3][1],
						&xform[0][2], &xform[1][2], &xform[2][2], &xform[3][2],
						&xform[0][3], &xform[1][3], &xform[2][3], &xform[3][3]
					) != 0)
						matrixPrided = true;

					sscanf(line, " position %f %f %f", &translate.data[3][0], &translate.data[3][1], &translate.data[3][2]);
					sscanf(line, " scale %f %f %f", &scale.data[0][0], &scale.data[1][1], &scale.data[2][2]);
					if (sscanf(line, " rotation %f %f %f %f", &rotQuat.x, &rotQuat.y, &rotQuat.z, &rotQuat.w) != 0)
						rot = Mat4::QuatToMatrix(rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w);
				}

				if (!filename.empty())
				{
					auto& actor = scene->CreateActor();
					if (strcmp(meshName, "none") != 0)
					{
						actor.SetName(meshName);
					}
					auto mesh = Core::Global::ServiceLocator::Get<Core::ResourceManagement::ModelManager>().GetResource(filename);
					actor.AddComponent<Core::ECS::Components::CModelRenderer>().SetModel(mesh);
					Mat4 transformMat;

					if (matrixPrided)
						transformMat = xform;
					else
						transformMat = scale * rot * translate;
					actor.GetComponent<Core::ECS::Components::CTransform>()->SetMatrix(transformMat.data);
					auto& materilaRener = actor.AddComponent<Core::ECS::Components::CMaterialRenderer>();
					auto mat = Core::Global::ServiceLocator::Get<Core::ResourceManagement::MaterialManager>().GetResource(matName);
					materilaRener.SetMaterialAtIndex(0, *mat);
					materilaRener.UpdateMaterialList();

				}
			}

			if (strstr(line, "gltf"))
			{
				std::string filename;
				Vec4 rotQuat;
				Mat4 xform, translate, rot, scale;
				bool matrixPrided = false;

				while (fgets(line, kMaxLineLength, file))
				{
					// end group
					if (strchr(line, '}'))
						break;

					char file[2048];

					if (sscanf(line, " file %s", file) == 1)
						filename = path + file;

					if (sscanf(line, " matrix %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
						&xform[0][0], &xform[1][0], &xform[2][0], &xform[3][0],
						&xform[0][1], &xform[1][1], &xform[2][1], &xform[3][1],
						&xform[0][2], &xform[1][2], &xform[2][2], &xform[3][2],
						&xform[0][3], &xform[1][3], &xform[2][3], &xform[3][3]
					) != 0)
						matrixPrided = true;

					sscanf(line, " position %f %f %f", &translate.data[3][0], &translate.data[3][1], &translate.data[3][2]);
					sscanf(line, " scale %f %f %f", &scale.data[0][0], &scale.data[1][1], &scale.data[2][2]);
					if (sscanf(line, " rotation %f %f %f %f", &rotQuat.x, &rotQuat.y, &rotQuat.z, &rotQuat.w) != 0)
						rot = Mat4::QuatToMatrix(rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w);
				}

				if (!filename.empty())
				{
					std::string ext = filename.substr(filename.find_last_of(".") + 1);

					bool success = false;
					Mat4 transformMat;

					if (matrixPrided)
						transformMat = xform;
					else
						transformMat = scale * rot * translate;

					// TODO: Add support for instancing.
					// If the same gltf is loaded multiple times then mesh data gets duplicated
					if (ext == "gltf")
						success = LoadGLTF(filename, scene, transformMat, false);
					else if (ext == "glb")
						success = LoadGLTF(filename, scene, transformMat, true);

					if (!success)
					{
						printf("Unable to load gltf %s\n", filename.c_str());
						exit(0);
					}
				}
			}
		}
		fclose(file);
		return true;
	}



	struct Primitive
	{
		int primitiveId;
		int materialId;
	};

	// Note: A GLTF mesh can contain multiple primitives and each primitive can potentially have a different material applied.
	// The two level BVH in this repo holds material ids per mesh and not per primitive, so this function loads each primitive from the gltf mesh as a new mesh
	void LoadMeshes(Scene* scene, tinygltf::Model& gltfModel, std::map<int, std::vector<Primitive>>& meshPrimMap)
	{
		for (int gltfMeshIdx = 0; gltfMeshIdx < gltfModel.meshes.size(); gltfMeshIdx++)
		{
			tinygltf::Mesh gltfMesh = gltfModel.meshes[gltfMeshIdx];

			for (int gltfPrimIdx = 0; gltfPrimIdx < gltfMesh.primitives.size(); gltfPrimIdx++)
			{
				tinygltf::Primitive prim = gltfMesh.primitives[gltfPrimIdx];

				// Skip points and lines
				if (prim.mode != TINYGLTF_MODE_TRIANGLES)
					continue;

				int indicesIndex = prim.indices;
				int positionIndex = -1;
				int normalIndex = -1;
				int uv0Index = -1;

				if (prim.attributes.count("POSITION") > 0)
				{
					positionIndex = prim.attributes["POSITION"];
				}

				if (prim.attributes.count("NORMAL") > 0)
				{
					normalIndex = prim.attributes["NORMAL"];
				}

				if (prim.attributes.count("TEXCOORD_0") > 0)
				{
					uv0Index = prim.attributes["TEXCOORD_0"];
				}

				// Vertex positions
				tinygltf::Accessor positionAccessor = gltfModel.accessors[positionIndex];
				tinygltf::BufferView positionBufferView = gltfModel.bufferViews[positionAccessor.bufferView];
				const tinygltf::Buffer& positionBuffer = gltfModel.buffers[positionBufferView.buffer];
				const uint8_t* positionBufferAddress = positionBuffer.data.data();
				int positionStride = tinygltf::GetComponentSizeInBytes(positionAccessor.componentType) * tinygltf::GetNumComponentsInType(positionAccessor.type);
				// TODO: Recheck
				if (positionBufferView.byteStride > 0)
					positionStride = positionBufferView.byteStride;

				// FIXME: Some GLTF files like TriangleWithoutIndices.gltf have no indices
				// Vertex indices
				tinygltf::Accessor indexAccessor = gltfModel.accessors[indicesIndex];
				tinygltf::BufferView indexBufferView = gltfModel.bufferViews[indexAccessor.bufferView];
				const tinygltf::Buffer& indexBuffer = gltfModel.buffers[indexBufferView.buffer];
				const uint8_t* indexBufferAddress = indexBuffer.data.data();
				int indexStride = tinygltf::GetComponentSizeInBytes(indexAccessor.componentType) * tinygltf::GetNumComponentsInType(indexAccessor.type);

				// Normals
				tinygltf::Accessor normalAccessor;
				tinygltf::BufferView normalBufferView;
				const uint8_t* normalBufferAddress = nullptr;
				int normalStride = -1;
				if (normalIndex > -1)
				{
					normalAccessor = gltfModel.accessors[normalIndex];
					normalBufferView = gltfModel.bufferViews[normalAccessor.bufferView];
					const tinygltf::Buffer& normalBuffer = gltfModel.buffers[normalBufferView.buffer];
					normalBufferAddress = normalBuffer.data.data();
					normalStride = tinygltf::GetComponentSizeInBytes(normalAccessor.componentType) * tinygltf::GetNumComponentsInType(normalAccessor.type);
					if (normalBufferView.byteStride > 0)
						normalStride = normalBufferView.byteStride;
				}

				// Texture coordinates
				tinygltf::Accessor uv0Accessor;
				tinygltf::BufferView uv0BufferView;
				const uint8_t* uv0BufferAddress = nullptr;
				int uv0Stride = -1;
				if (uv0Index > -1)
				{
					uv0Accessor = gltfModel.accessors[uv0Index];
					uv0BufferView = gltfModel.bufferViews[uv0Accessor.bufferView];
					const tinygltf::Buffer& uv0Buffer = gltfModel.buffers[uv0BufferView.buffer];
					uv0BufferAddress = uv0Buffer.data.data();
					uv0Stride = tinygltf::GetComponentSizeInBytes(uv0Accessor.componentType) * tinygltf::GetNumComponentsInType(uv0Accessor.type);
					if (uv0BufferView.byteStride > 0)
						uv0Stride = uv0BufferView.byteStride;
				}

				std::vector<Vec3> vertices;
				std::vector<Vec3> normals;
				std::vector<Vec2> uvs;

				// Get vertex data
				for (size_t vertexIndex = 0; vertexIndex < positionAccessor.count; vertexIndex++)
				{
					Vec3 vertex, normal;
					Vec2 uv;

					{
						const uint8_t* address = positionBufferAddress + positionBufferView.byteOffset + positionAccessor.byteOffset + (vertexIndex * positionStride);
						memcpy(&vertex, address, 12);
					}

					if (normalIndex > -1)
					{
						const uint8_t* address = normalBufferAddress + normalBufferView.byteOffset + normalAccessor.byteOffset + (vertexIndex * normalStride);
						memcpy(&normal, address, 12);
					}

					if (uv0Index > -1)
					{
						const uint8_t* address = uv0BufferAddress + uv0BufferView.byteOffset + uv0Accessor.byteOffset + (vertexIndex * uv0Stride);
						memcpy(&uv, address, 8);
					}

					vertices.push_back(vertex);
					normals.push_back(normal);
					uvs.push_back({ uv.x, uv.y });
				}

				// Get index data
				if (indexAccessor.count == 0) {
					assert(false);
				}
				std::vector<int> indices(indexAccessor.count);
				const uint8_t* baseAddress = indexBufferAddress + indexBufferView.byteOffset + indexAccessor.byteOffset;
				if (indexStride == 1)
				{
					std::vector<uint8_t> quarter;
					quarter.resize(indexAccessor.count);

					memcpy(quarter.data(), baseAddress, (indexAccessor.count * indexStride));

					// Convert quarter precision indices to full precision
					for (size_t i = 0; i < indexAccessor.count; i++)
					{
						indices[i] = quarter[i];
					}
				}
				else if (indexStride == 2)
				{
					std::vector<uint16_t> half;
					half.resize(indexAccessor.count);

					memcpy(half.data(), baseAddress, (indexAccessor.count * indexStride));

					// Convert half precision indices to full precision
					for (size_t i = 0; i < indexAccessor.count; i++)
					{
						indices[i] = half[i];
					}
				}
				else
				{
					memcpy(indices.data(), baseAddress, (indexAccessor.count * indexStride));
				}

				Mesh* mesh = new Mesh();

				// Get triangles from vertex indices
				for (int v = 0; v < indices.size(); v++)
				{
					Vec3 pos = vertices[indices[v]];
					Vec3 nrm = normals[indices[v]];
					Vec2 uv = uvs[indices[v]];

					mesh->getPosUvX().push_back(Vec4(pos.x, pos.y, pos.z, uv.x));
					mesh->getNorUvY().push_back(Vec4(nrm.x, nrm.y, nrm.z, uv.y));
				}
				mesh->GenVAO();
				mesh->setName(gltfMesh.name);

				int sceneMeshId = scene->getMeshes().size();
				scene->getMeshes().push_back(mesh);
				// Store a mapping for a gltf mesh and the loaded primitive data
				// This is used for creating instances based on the primitive
				int sceneMatIdx = prim.material + scene->getMaterials().size();
				meshPrimMap[gltfMeshIdx].push_back(Primitive{ sceneMeshId, sceneMatIdx });
			}
		}
	}

	void LoadTextures(Scene* scene, tinygltf::Model& gltfModel)
	{
		for (size_t i = 0; i < gltfModel.textures.size(); ++i)
		{
			tinygltf::Texture& gltfTex = gltfModel.textures[i];
			tinygltf::Image& image = gltfModel.images[gltfTex.source];
			std::string texName = gltfTex.name;
			if (strcmp(gltfTex.name.c_str(), "") == 0)
				texName = image.uri;
			Texture* texture = new Texture(texName, image.image.data(), image.width, image.height, image.component);
			scene->getTextures().push_back(texture);
		}
	}

	void LoadMaterials(Scene* scene, tinygltf::Model& gltfModel)
	{
		int sceneTexIdx = scene->getTextures().size();
		// TODO: Support for KHR extensions
		for (size_t i = 0; i < gltfModel.materials.size(); i++)
		{
			const tinygltf::Material gltfMaterial = gltfModel.materials[i];
			const tinygltf::PbrMetallicRoughness pbr = gltfMaterial.pbrMetallicRoughness;

			// Convert glTF material
			Material material;

			// Albedo
			material.baseColor = Vec3((float)pbr.baseColorFactor[0], (float)pbr.baseColorFactor[1], (float)pbr.baseColorFactor[2]);
			if (pbr.baseColorTexture.index > -1)
				material.baseColorTexId = pbr.baseColorTexture.index + sceneTexIdx;

			// Opacity
			material.opacity = (float)pbr.baseColorFactor[3];

			// Alpha
			material.alphaCutoff = static_cast<float>(gltfMaterial.alphaCutoff);
			if (strcmp(gltfMaterial.alphaMode.c_str(), "OPAQUE") == 0) material.alphaMode = AlphaMode::Opaque;
			else if (strcmp(gltfMaterial.alphaMode.c_str(), "BLEND") == 0) material.alphaMode = AlphaMode::Blend;
			else if (strcmp(gltfMaterial.alphaMode.c_str(), "MASK") == 0) material.alphaMode = AlphaMode::Mask;

			// Roughness and Metallic
			material.roughness = sqrtf((float)pbr.roughnessFactor); // Repo's disney material doesn't use squared roughness
			material.metallic = (float)pbr.metallicFactor;
			if (pbr.metallicRoughnessTexture.index > -1)
				material.metallicRoughnessTexID = pbr.metallicRoughnessTexture.index + sceneTexIdx;

			// Normal Map
			material.normalmapTexID = gltfMaterial.normalTexture.index + sceneTexIdx;

			// Emission
			material.emission = Vec3((float)gltfMaterial.emissiveFactor[0], (float)gltfMaterial.emissiveFactor[1], (float)gltfMaterial.emissiveFactor[2]);
			if (gltfMaterial.emissiveTexture.index > -1)
				material.emissionmapTexID = gltfMaterial.emissiveTexture.index + sceneTexIdx;

			// KHR_materials_transmission
			if (gltfMaterial.extensions.find("KHR_materials_transmission") != gltfMaterial.extensions.end())
			{
				const auto& ext = gltfMaterial.extensions.find("KHR_materials_transmission")->second;
				if (ext.Has("transmissionFactor"))
					material.specTrans = (float)(ext.Get("transmissionFactor").Get<double>());
			}

			scene->AddMaterial(material);
		}

		// Default material
		if (scene->getMaterials().size() == 0)
		{
			Material defaultMat;
			scene->getMaterials().push_back(defaultMat);
		}
	}

	void TraverseNodes(Scene* scene, tinygltf::Model& gltfModel, int nodeIdx, Mat4& parentMat, std::map<int, std::vector<Primitive>>& meshPrimMap)
	{
		tinygltf::Node gltfNode = gltfModel.nodes[nodeIdx];

		Mat4 localMat;

		if (gltfNode.matrix.size() > 0)
		{
			localMat.data[0][0] = gltfNode.matrix[0];
			localMat.data[0][1] = gltfNode.matrix[1];
			localMat.data[0][2] = gltfNode.matrix[2];
			localMat.data[0][3] = gltfNode.matrix[3];

			localMat.data[1][0] = gltfNode.matrix[4];
			localMat.data[1][1] = gltfNode.matrix[5];
			localMat.data[1][2] = gltfNode.matrix[6];
			localMat.data[1][3] = gltfNode.matrix[7];

			localMat.data[2][0] = gltfNode.matrix[8];
			localMat.data[2][1] = gltfNode.matrix[9];
			localMat.data[2][2] = gltfNode.matrix[10];
			localMat.data[2][3] = gltfNode.matrix[11];

			localMat.data[3][0] = gltfNode.matrix[12];
			localMat.data[3][1] = gltfNode.matrix[13];
			localMat.data[3][2] = gltfNode.matrix[14];
			localMat.data[3][3] = gltfNode.matrix[15];
		}
		else
		{
			Mat4 translate, rot, scale;

			if (gltfNode.translation.size() > 0)
			{
				translate.data[3][0] = gltfNode.translation[0];
				translate.data[3][1] = gltfNode.translation[1];
				translate.data[3][2] = gltfNode.translation[2];
			}

			if (gltfNode.rotation.size() > 0)
			{
				rot = Mat4::QuatToMatrix(gltfNode.rotation[0], gltfNode.rotation[1], gltfNode.rotation[2], gltfNode.rotation[3]);
			}

			if (gltfNode.scale.size() > 0)
			{
				scale.data[0][0] = gltfNode.scale[0];
				scale.data[1][1] = gltfNode.scale[1];
				scale.data[2][2] = gltfNode.scale[2];
			}

			//localMat = scale * rot * translate;
			localMat = translate.Transpose() * rot.Transpose() * scale.Transpose();
		}

		Mat4 xform = parentMat * localMat.Transpose();
		//Mat4 xform = localMat * parentMat;
		//xform = xform.Transpose();
		// When at a leaf node, add an instance to the scene (if a mesh exists for it)
		if (gltfNode.children.size() == 0 && gltfNode.mesh != -1)
		{
			std::vector<Primitive> prims = meshPrimMap[gltfNode.mesh];

			// Write the instance data
			for (int i = 0; i < prims.size(); i++)
			{
				std::string name = gltfNode.name;
				// TODO: Better naming
				if (strcmp(name.c_str(), "") == 0)
					name = "Mesh " + std::to_string(gltfNode.mesh) + " Prim" + std::to_string(prims[i].primitiveId);

				MeshInstance instance(name, prims[i].primitiveId, xform, prims[i].materialId < 0 ? 0 : prims[i].materialId);
				scene->AddMeshInstance(instance);
			}
		}

		for (size_t i = 0; i < gltfNode.children.size(); i++)
		{
			TraverseNodes(scene, gltfModel, gltfNode.children[i], xform, meshPrimMap);
		}
	}

	void LoadInstances(Scene* scene, tinygltf::Model& gltfModel, Mat4 xform, std::map<int, std::vector<Primitive>>& meshPrimMap)
	{
		const tinygltf::Scene gltfScene = gltfModel.scenes[gltfModel.defaultScene];

		for (int rootIdx = 0; rootIdx < gltfScene.nodes.size(); rootIdx++)
		{

			std::vector<int>indexStack;
			std::vector<Mat4>transformStack;

			indexStack.push_back(gltfScene.nodes[rootIdx]);
			transformStack.push_back(xform);

			while (!indexStack.empty())
			{
				int nodeIdx = indexStack.back(); indexStack.pop_back();
				Mat4 parentMat = transformStack.back(); transformStack.pop_back();

				tinygltf::Node gltfNode = gltfModel.nodes[nodeIdx];
				Mat4 localMat;
				if (gltfNode.matrix.size() > 0)
				{
					localMat.data[0][0] = gltfNode.matrix[0];
					localMat.data[0][1] = gltfNode.matrix[1];
					localMat.data[0][2] = gltfNode.matrix[2];
					localMat.data[0][3] = gltfNode.matrix[3];

					localMat.data[1][0] = gltfNode.matrix[4];
					localMat.data[1][1] = gltfNode.matrix[5];
					localMat.data[1][2] = gltfNode.matrix[6];
					localMat.data[1][3] = gltfNode.matrix[7];

					localMat.data[2][0] = gltfNode.matrix[8];
					localMat.data[2][1] = gltfNode.matrix[9];
					localMat.data[2][2] = gltfNode.matrix[10];
					localMat.data[2][3] = gltfNode.matrix[11];

					localMat.data[3][0] = gltfNode.matrix[12];
					localMat.data[3][1] = gltfNode.matrix[13];
					localMat.data[3][2] = gltfNode.matrix[14];
					localMat.data[3][3] = gltfNode.matrix[15];
				}
				else
				{
					Mat4 translate, rot, scale;

					if (gltfNode.translation.size() > 0)
					{
						translate.data[3][0] = gltfNode.translation[0];
						translate.data[3][1] = gltfNode.translation[1];
						translate.data[3][2] = gltfNode.translation[2];
					}

					if (gltfNode.rotation.size() > 0)
					{
						rot = Mat4::QuatToMatrix(gltfNode.rotation[0], gltfNode.rotation[1], gltfNode.rotation[2], gltfNode.rotation[3]);
					}

					if (gltfNode.scale.size() > 0)
					{
						scale.data[0][0] = gltfNode.scale[0];
						scale.data[1][1] = gltfNode.scale[1];
						scale.data[2][2] = gltfNode.scale[2];
					}

					localMat = scale * rot * translate;
				}

				Mat4 xform = localMat * parentMat;
				// When at a leaf node, add an instance to the scene (if a mesh exists for it)
				if (gltfNode.children.size() == 0 && gltfNode.mesh != -1)
				{
					std::vector<Primitive> prims = meshPrimMap[gltfNode.mesh];

					// Write the instance data
					for (int i = 0; i < prims.size(); i++)
					{
						std::string name = gltfNode.name;
						// TODO: Better naming
						if (strcmp(name.c_str(), "") == 0)
							name = "Mesh " + std::to_string(gltfNode.mesh) + " Prim" + std::to_string(prims[i].primitiveId);

						MeshInstance instance(name, prims[i].primitiveId, xform, prims[i].materialId < 0 ? 0 : prims[i].materialId);
						scene->AddMeshInstance(instance);
					}
				}
				else {
					for (size_t i = 0; i < gltfNode.children.size(); i++)
					{
						indexStack.push_back(gltfNode.children[i]);
						transformStack.push_back(xform);
					}
				}
			}
		}
	}

	bool LoadGLTF(const std::string& filename, Scene* scene, RenderOptions& renderOptions, Mat4 xform, bool binary)
	{
		tinygltf::Model gltfModel;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		CORE_INFO("Loading GLTF{0}", filename.c_str());

		bool ret;

		if (binary)
			ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filename);
		else
			ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, filename);

		if (!ret)
		{
			CORE_ERROR("Unable to load file {0}. Error: {1}\n", filename.c_str(), err.c_str());
			return false;
		}

		std::map<int, std::vector<Primitive>> meshPrimMap;
		LoadMeshes(scene, gltfModel, meshPrimMap);
		LoadMaterials(scene, gltfModel);
		LoadTextures(scene, gltfModel);
		LoadInstances(scene, gltfModel, xform, meshPrimMap);
		return true;
	}

	bool LoadGLTF(const std::string& filename, Core::SceneSystem::Scene* scene, Mat4 formaa, bool binary)
	{
		tinygltf::Model gltfModel;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		CORE_INFO("Loading GLTF{0}", filename.c_str());

		bool ret;

		if (binary)
			ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filename);
		else
			ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, filename);

		if (!ret)
		{
			CORE_ERROR("Unable to load file {0}. Error: {1}\n", filename.c_str(), err.c_str());
			return false;
		}
		struct primIndex
		{
			std::string path;
			int materialIndex;
		};
		std::map<int, std::vector<primIndex>> meshPrimMap;
		std::unordered_map<int, std::string>textureMap;
		std::unordered_map<int, std::string>materilaMap;
		//Load Mesh
		{
			for (int gltfMeshIdx = 0; gltfMeshIdx < gltfModel.meshes.size(); gltfMeshIdx++)
			{
				tinygltf::Mesh gltfMesh = gltfModel.meshes[gltfMeshIdx];

				for (int gltfPrimIdx = 0; gltfPrimIdx < gltfMesh.primitives.size(); gltfPrimIdx++)
				{
					tinygltf::Primitive prim = gltfMesh.primitives[gltfPrimIdx];

					// Skip points and lines
					if (prim.mode != TINYGLTF_MODE_TRIANGLES)
						continue;

					int indicesIndex = prim.indices;
					int positionIndex = -1;
					int normalIndex = -1;
					int uv0Index = -1;

					if (prim.attributes.count("POSITION") > 0)
					{
						positionIndex = prim.attributes["POSITION"];
					}

					if (prim.attributes.count("NORMAL") > 0)
					{
						normalIndex = prim.attributes["NORMAL"];
					}

					if (prim.attributes.count("TEXCOORD_0") > 0)
					{
						uv0Index = prim.attributes["TEXCOORD_0"];
					}


					// Vertex positions
					tinygltf::Accessor positionAccessor = gltfModel.accessors[positionIndex];
					tinygltf::BufferView positionBufferView = gltfModel.bufferViews[positionAccessor.bufferView];
					const tinygltf::Buffer& positionBuffer = gltfModel.buffers[positionBufferView.buffer];
					const uint8_t* positionBufferAddress = positionBuffer.data.data();
					int positionStride = tinygltf::GetComponentSizeInBytes(positionAccessor.componentType) * tinygltf::GetNumComponentsInType(positionAccessor.type);
					// TODO: Recheck
					if (positionBufferView.byteStride > 0)
						positionStride = positionBufferView.byteStride;

					// FIXME: Some GLTF files like TriangleWithoutIndices.gltf have no indices
					// Vertex indices
					tinygltf::Accessor indexAccessor = gltfModel.accessors[indicesIndex];
					tinygltf::BufferView indexBufferView = gltfModel.bufferViews[indexAccessor.bufferView];
					const tinygltf::Buffer& indexBuffer = gltfModel.buffers[indexBufferView.buffer];
					const uint8_t* indexBufferAddress = indexBuffer.data.data();
					int indexStride = tinygltf::GetComponentSizeInBytes(indexAccessor.componentType) * tinygltf::GetNumComponentsInType(indexAccessor.type);

					// Normals
					tinygltf::Accessor normalAccessor;
					tinygltf::BufferView normalBufferView;
					const uint8_t* normalBufferAddress = nullptr;
					int normalStride = -1;
					if (normalIndex > -1)
					{
						normalAccessor = gltfModel.accessors[normalIndex];
						normalBufferView = gltfModel.bufferViews[normalAccessor.bufferView];
						const tinygltf::Buffer& normalBuffer = gltfModel.buffers[normalBufferView.buffer];
						normalBufferAddress = normalBuffer.data.data();
						normalStride = tinygltf::GetComponentSizeInBytes(normalAccessor.componentType) * tinygltf::GetNumComponentsInType(normalAccessor.type);
						if (normalBufferView.byteStride > 0)
							normalStride = normalBufferView.byteStride;
					}

					// Texture coordinates
					tinygltf::Accessor uv0Accessor;
					tinygltf::BufferView uv0BufferView;
					const uint8_t* uv0BufferAddress = nullptr;
					int uv0Stride = -1;
					if (uv0Index > -1)
					{
						uv0Accessor = gltfModel.accessors[uv0Index];
						uv0BufferView = gltfModel.bufferViews[uv0Accessor.bufferView];
						const tinygltf::Buffer& uv0Buffer = gltfModel.buffers[uv0BufferView.buffer];
						uv0BufferAddress = uv0Buffer.data.data();
						uv0Stride = tinygltf::GetComponentSizeInBytes(uv0Accessor.componentType) * tinygltf::GetNumComponentsInType(uv0Accessor.type);
						if (uv0BufferView.byteStride > 0)
							uv0Stride = uv0BufferView.byteStride;
					}

					std::vector<Maths::FVector3> vertices;
					std::vector<Maths::FVector3> normals;
					std::vector<Maths::FVector2> uvs;

					// Get vertex data
					for (size_t vertexIndex = 0; vertexIndex < positionAccessor.count; vertexIndex++)
					{
						Maths::FVector3 vertex, normal;
						Maths::FVector2 uv;

						{
							const uint8_t* address = positionBufferAddress + positionBufferView.byteOffset + positionAccessor.byteOffset + (vertexIndex * positionStride);
							memcpy(&vertex, address, 12);
						}

						if (normalIndex > -1)
						{
							const uint8_t* address = normalBufferAddress + normalBufferView.byteOffset + normalAccessor.byteOffset + (vertexIndex * normalStride);
							memcpy(&normal, address, 12);
						}

						if (uv0Index > -1)
						{
							const uint8_t* address = uv0BufferAddress + uv0BufferView.byteOffset + uv0Accessor.byteOffset + (vertexIndex * uv0Stride);
							memcpy(&uv, address, 8);
						}

						vertices.push_back(vertex);
						normals.push_back(normal);
						uvs.push_back({ uv.x,1 - uv.y });
					}

					// Get index data
					std::vector<unsigned int> indices(indexAccessor.count);
					const uint8_t* baseAddress = indexBufferAddress + indexBufferView.byteOffset + indexAccessor.byteOffset;
					if (indexStride == 1)
					{
						std::vector<uint8_t> quarter;
						quarter.resize(indexAccessor.count);

						memcpy(quarter.data(), baseAddress, (indexAccessor.count * indexStride));

						// Convert quarter precision indices to full precision
						for (size_t i = 0; i < indexAccessor.count; i++)
						{
							indices[i] = quarter[i];
						}
					}
					else if (indexStride == 2)
					{
						std::vector<uint16_t> half;
						half.resize(indexAccessor.count);

						memcpy(half.data(), baseAddress, (indexAccessor.count * indexStride));

						// Convert half precision indices to full precision
						for (size_t i = 0; i < indexAccessor.count; i++)
						{
							indices[i] = half[i];
						}
					}
					else
					{
						memcpy(indices.data(), baseAddress, (indexAccessor.count * indexStride));
					}
					std::string modelname = filename + std::to_string(gltfMeshIdx) + "#" + std::to_string(gltfPrimIdx);
					Core::Global::ServiceLocator::Get<Core::ResourceManagement::ModelManager>().LoadFromMemory(modelname, vertices, normals, uvs, indices);

					meshPrimMap[gltfMeshIdx].push_back(primIndex{ modelname, prim.material });
				}
			}
		}
		//Load Texture
		{
			for (size_t i = 0; i < gltfModel.textures.size(); ++i)
			{
				tinygltf::Texture& gltfTex = gltfModel.textures[i];
				tinygltf::Image& image = gltfModel.images[gltfTex.source];
				std::string texName = gltfTex.name;
				if (strcmp(gltfTex.name.c_str(), "") == 0)
					texName = image.uri;
				Core::Global::ServiceLocator::Get<Core::ResourceManagement::TextureManager>().CreateFromMemory(texName, image.image.data(), image.width, image.height);
				textureMap[i] = texName;
			}
		}
		//Load Material
		{

			// TODO: Support for KHR extensions
			for (size_t i = 0; i < gltfModel.materials.size(); i++)
			{
				const tinygltf::Material gltfMaterial = gltfModel.materials[i];
				const tinygltf::PbrMetallicRoughness pbr = gltfMaterial.pbrMetallicRoughness;

				// Convert glTF material

				materilaMap[i] = gltfMaterial.name;
				if (materilaMap[i] == "") {
					materilaMap[i] = filename + "::material " + std::to_string(i);
				}

				Core::Resources::Material* tempMat = new Core::Resources::Material();
				Core::Global::ServiceLocator::Get<Core::ResourceManagement::MaterialManager>().RegisterResource(materilaMap[i], tempMat);
				tempMat->SetBackfaceCulling(false);;
				tempMat->SetCastShadows(false);
				tempMat->SetReceiveShadows(false);
				tempMat->AddFeature("NORMAL_MAPPING");
				tempMat->SetShader(Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager[":Shaders\\Standard.ovfx"]);
				tempMat->SetProperty("u_Albedo", Maths::FVector4{ (float)pbr.baseColorFactor[0], (float)pbr.baseColorFactor[1], (float)pbr.baseColorFactor[2], (float)pbr.baseColorFactor[3] });
				// Albedo Texture
				if (pbr.baseColorTexture.index > -1) {
					auto albedo = Core::Global::ServiceLocator::Get<Core::ResourceManagement::TextureManager>().GetResource(textureMap[pbr.baseColorTexture.index], true);
					tempMat->SetProperty("u_AlbedoMap", albedo);
				}
				tempMat->SetProperty("u_AlphaClippingThreshold", static_cast<float>(gltfMaterial.alphaCutoff));
				tempMat->SetProperty("u_Roughness", (float)pbr.roughnessFactor);
				tempMat->SetProperty("u_Metallic", (float)pbr.metallicFactor);
				//if (strcmp(gltfMaterial.alphaMode.c_str(), "OPAQUE") == 0) material.alphaMode = AlphaMode::Opaque;
				//else if (strcmp(gltfMaterial.alphaMode.c_str(), "BLEND") == 0) material.alphaMode = AlphaMode::Blend;
				//else if (strcmp(gltfMaterial.alphaMode.c_str(), "MASK") == 0) material.alphaMode = AlphaMode::Mask;

				// MetallicRoughness Texture
				if (pbr.metallicRoughnessTexture.index > -1) {
					auto roughness = Core::Global::ServiceLocator::Get<Core::ResourceManagement::TextureManager>().GetResource(textureMap[pbr.metallicRoughnessTexture.index], true);
					tempMat->SetProperty("u_RoughnessMap", roughness);

					auto metallicMap = Core::Global::ServiceLocator::Get<Core::ResourceManagement::TextureManager>().GetResource(textureMap[pbr.metallicRoughnessTexture.index], true);
					tempMat->SetProperty("u_MetallicMap", metallicMap);
				}
				if (gltfMaterial.emissiveTexture.index > -1) {
					auto emission = Core::Global::ServiceLocator::Get<Core::ResourceManagement::TextureManager>().GetResource(textureMap[gltfMaterial.emissiveTexture.index], true);
					tempMat->SetProperty("u_EmissiveMap", emission);
				}
				// Normal Map Texture
				auto normalTex = Core::Global::ServiceLocator::Get<Core::ResourceManagement::TextureManager>().GetResource(textureMap[gltfMaterial.normalTexture.index], true);
				tempMat->SetProperty("u_NormalMap", normalTex);
				// Emission
				tempMat->SetProperty("u_EmissiveIntensity", 1.0f);
				tempMat->SetProperty("u_EmissiveColor", Maths::FVector3{ (float)gltfMaterial.emissiveFactor[0], (float)gltfMaterial.emissiveFactor[1], (float)gltfMaterial.emissiveFactor[2] });
				// KHR_materials_transmission
				if (gltfMaterial.extensions.find("KHR_materials_transmission") != gltfMaterial.extensions.end())
				{
					const auto& ext = gltfMaterial.extensions.find("KHR_materials_transmission")->second;
					if (ext.Has("transmissionFactor"))
					{
						tempMat->SetProperty("u_Transmission", (float)(ext.Get("transmissionFactor").Get<double>()));
					}
				}
			}
		}
		//Load Actor
		{
			const tinygltf::Scene gltfScene = gltfModel.scenes[gltfModel.defaultScene];

			for (int rootIdx = 0; rootIdx < gltfScene.nodes.size(); rootIdx++)
			{
				auto& rootActor = scene->CreateActor();
				rootActor.SetName("Root " + std::to_string(rootIdx));
				std::vector<int>indexStack;
				//std::vector<Mat4>transformStack;
				std::vector<int64_t>parentStack;
				indexStack.push_back(gltfScene.nodes[rootIdx]);
				//transformStack.push_back(formaa);
				parentStack.push_back(rootActor.GetID());
				while (!indexStack.empty())
				{
					int nodeIdx = indexStack.back(); indexStack.pop_back();
					//Mat4 parentMat = transformStack.back(); transformStack.pop_back();
					int parentActorIdx = parentStack.back(); parentStack.pop_back();


					tinygltf::Node gltfNode = gltfModel.nodes[nodeIdx];
					Mat4 localMat;
					if (gltfNode.matrix.size() > 0)
					{
						localMat.data[0][0] = gltfNode.matrix[0];
						localMat.data[0][1] = gltfNode.matrix[1];
						localMat.data[0][2] = gltfNode.matrix[2];
						localMat.data[0][3] = gltfNode.matrix[3];

						localMat.data[1][0] = gltfNode.matrix[4];
						localMat.data[1][1] = gltfNode.matrix[5];
						localMat.data[1][2] = gltfNode.matrix[6];
						localMat.data[1][3] = gltfNode.matrix[7];

						localMat.data[2][0] = gltfNode.matrix[8];
						localMat.data[2][1] = gltfNode.matrix[9];
						localMat.data[2][2] = gltfNode.matrix[10];
						localMat.data[2][3] = gltfNode.matrix[11];

						localMat.data[3][0] = gltfNode.matrix[12];
						localMat.data[3][1] = gltfNode.matrix[13];
						localMat.data[3][2] = gltfNode.matrix[14];
						localMat.data[3][3] = gltfNode.matrix[15];
					}
					else
					{
						Mat4 translate, rot, scale;

						if (gltfNode.translation.size() > 0)
						{
							translate.data[3][0] = gltfNode.translation[0];
							translate.data[3][1] = gltfNode.translation[1];
							translate.data[3][2] = gltfNode.translation[2];
						}

						if (gltfNode.rotation.size() > 0)
						{
							rot = Mat4::QuatToMatrix(gltfNode.rotation[0], gltfNode.rotation[1], gltfNode.rotation[2], gltfNode.rotation[3]);
						}

						if (gltfNode.scale.size() > 0)
						{
							scale.data[0][0] = gltfNode.scale[0];
							scale.data[1][1] = gltfNode.scale[1];
							scale.data[2][2] = gltfNode.scale[2];
						}

						localMat = scale * rot * translate;
						//localMat = translate * rot * scale;
					}

					//Mat4 xform = localMat * parentMat;
					//localMat = localMat.Transpose();
					// When at a leaf node, add an instance to the scene (if a mesh exists for it)
					if (gltfNode.children.size() == 0 && gltfNode.mesh != -1)
					{
						std::vector<primIndex> prims = meshPrimMap[gltfNode.mesh];

						// Write the instance data
						for (int i = 0; i < prims.size(); i++)
						{
							std::string name = gltfNode.name;
							// TODO: Better naming
							if (strcmp(name.c_str(), "") == 0)
								name = "Mesh " + std::to_string(gltfNode.mesh) + " Prim" + prims[i].path;
							auto& actor = scene->CreateActor();
							actor.SetParent(*scene->FindActorByID(parentActorIdx));
							actor.SetName(name);

							auto mesh = Core::Global::ServiceLocator::Get<Core::ResourceManagement::ModelManager>().GetResource(prims[i].path);
							actor.AddComponent<Core::ECS::Components::CModelRenderer>().SetModel(mesh);

							actor.GetComponent<Core::ECS::Components::CTransform>()->SetMatrix(localMat.data);
							auto& materilaRener = actor.AddComponent<Core::ECS::Components::CMaterialRenderer>();
							if (prims[i].materialIndex < 0) {
								assert(false);
							}

							auto mat = Core::Global::ServiceLocator::Get<Core::ResourceManagement::MaterialManager>().GetResource(materilaMap[prims[i].materialIndex < 0 ? 0 : prims[i].materialIndex]);
							materilaRener.SetMaterialAtIndex(0, *mat);
							materilaRener.UpdateMaterialList();

						}
					}
					else {
						std::string name = gltfNode.name;
						// TODO: Better naming
						if (strcmp(name.c_str(), "") == 0)
							name = "Mesh #";
						auto& actor = scene->CreateActor();
						actor.SetParent(*scene->FindActorByID(parentActorIdx));
						actor.SetName(name);
						actor.GetComponent<Core::ECS::Components::CTransform>()->SetMatrix(localMat.data);
						for (size_t i = 0; i < gltfNode.children.size(); i++)
						{
							indexStack.push_back(gltfNode.children[i]);
							//transformStack.push_back(xform);
							parentStack.push_back(actor.GetID());
						}
					}
				}
			}
		}
		return true;
	}
}
