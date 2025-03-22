#pragma once
#include <vector>
#include <map>
#include <memory>
#include <string>
#include "MathUtil.h"
namespace asset {
	class FBO;
	class Shader;
	class UBO;
	class Texture;
}
namespace PathTrace
{


	struct RenderOptions
	{
		RenderOptions()
		{
			renderResolution = iVec2(1280, 720);
			windowResolution = iVec2(1280, 720);
			uniformLightCol = Vec3(0.3f, 0.3f, 0.3f);
			backgroundCol = Vec3(1.0f, 1.0f, 1.0f);
			tileWidth = 100;
			tileHeight = 100;
			maxDepth = 2;
			maxSpp = -1;
			RRDepth = 2;
			texArrayWidth = 2048;
			texArrayHeight = 2048;
			denoiserFrameCnt = 20;
			enableRR = true;
			enableDenoiser = false;
			enableTonemap = true;
			enableAces = false;
			openglNormalMap = true;
			enableEnvMap = false;
			enableUniformLight = false;
			hideEmitters = false;
			enableBackground = false;
			transparentBackground = false;
			independentRenderSize = false;
			enableRoughnessMollification = false;
			enableVolumeMIS = false;

			envMapIntensity = 1.0f;
			envMapRot = 0.0f;
			roughnessMollificationAmt = 0.0f;
		}

		iVec2 renderResolution;
		iVec2 windowResolution;
		Vec3 uniformLightCol;
		Vec3 backgroundCol;
		int tileWidth;
		int tileHeight;
		int maxDepth;
		int maxSpp;
		int RRDepth;
		int texArrayWidth;
		int texArrayHeight;
		int denoiserFrameCnt;
		bool enableRR;
		bool enableDenoiser;
		bool enableTonemap;
		bool enableAces;
		bool simpleAcesFit;
		bool openglNormalMap;
		bool enableEnvMap;
		bool enableUniformLight;
		bool hideEmitters;
		bool enableBackground;
		bool transparentBackground;
		bool independentRenderSize;
		bool enableRoughnessMollification;
		bool enableVolumeMIS;
		float envMapIntensity;
		float envMapRot;
		float roughnessMollificationAmt;
	};

	class Scene;
	class Quad;

	class Renderer
	{
	protected:
		Quad* quad;
		Scene* scene;

		// GPU数据
		unsigned int BVHBuffer;
		unsigned int BVHTex;
		unsigned int vertexIndicesBuffer;
		unsigned int vertexIndicesTex;
		unsigned int verticesBuffer;
		unsigned int verticesTex;
		unsigned int normalsBuffer;
		unsigned int normalsTex;
		unsigned int materialsTex;
		unsigned int transformsTex;
		unsigned int lightsTex;
		unsigned int textureMapsArrayTex;
		unsigned int envMapTex;
		unsigned int envMapCDFTex;

		// 帧缓冲
		/*
		pathTracefbo        绘制一个瓦片的结果
		pathTraceFBOLowRes  粗略预览
		accumFBO            存放光线的累加和
		outputFBO           将路径的累加和做平均和色调映射
		*/
		std::shared_ptr<asset::FBO> pathTracefbo;
		std::shared_ptr<asset::FBO> pathTraceFBOLowRes;
		std::shared_ptr<asset::FBO> accumFBO;
		std::shared_ptr<asset::FBO> outputFBO;
		std::shared_ptr<asset::FBO> rasterMsaaFBO;
		std::shared_ptr<asset::FBO> rasterFBO;

		// Shaders
		std::string shadersDirectory;

		std::shared_ptr<asset::Shader> pathTraceShader;
		std::shared_ptr<asset::Shader> pathTraceShaderLowRes;
		std::shared_ptr<asset::Shader> outputShader;
		std::shared_ptr<asset::Shader> tonemapShader;
		std::shared_ptr<asset::Shader> pbrShader;
		std::shared_ptr<asset::Shader> lineShader;
		std::map<unsigned int, asset::UBO> UBOs;  // indexed by uniform buffer's binding point
		//预计算IBL
		std::shared_ptr<asset::Texture> irradiance_map;
		std::shared_ptr<asset::Texture> prefiltered_map;
		std::shared_ptr<asset::Texture> BRDF_LUT;


		// 贴图
		unsigned int pathTraceTextureLowRes;
		unsigned int pathTraceTexture;
		unsigned int accumTexture;
		unsigned int tileOutputTexture[2];
		unsigned int denoisedTexture;


		iVec2 renderSize;
		iVec2 windowSize;


		iVec2 tile;
		iVec2 numTiles;
		Vec2 invNumTiles;
		int tileWidth;
		int tileHeight;
		int currentBuffer;
		int frameCounter;
		int sampleCounter;
		float pixelRatio;

		float* denoiserInputFramePtr;
		float* frameOutputPtr;
		bool denoised;

		bool initialized;


	public:
		Renderer(Scene* scene, const std::string& shadersDirectory);
		~Renderer();

		void ResizeRenderer();
		void ReloadShaders();
		void RenderPBR();
		void Render();
		void Present();
		void PresentPBR();
		void SaveFrame();
		void SaveScene();
		void Update(float secondsElapsed);
		float GetProgress();
		int GetSampleCount();
		void GetOutputBuffer(unsigned char**, int& w, int& h);

	private:
		void InitGPUDataBuffers();
		void InitFBOs();
		void InitShaders();
		void PreRaster();
	};
}