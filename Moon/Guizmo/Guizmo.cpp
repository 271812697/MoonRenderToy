#include "Guizmo.h"
#include "Guizmo/MathUtil/MathUtil.h"
#include "Settings/DebugSetting.h"
#include "Qtimgui/imgui/imgui.h"
#include "renderer/SceneView.h"
#include "OvCore/Global/ServiceLocator.h"
#include <OvRendering/Data/Material.h>
#include <OvRendering/Resources/Loaders/ShaderLoader.h>
#include <OvMaths/FMatrix4.h>
#include <glad/glad.h>
namespace MOON
{

	static OvMaths::FMatrix4 ToFMatrix4(const Eigen::Matrix4f& mat) {
		Eigen::Matrix4f temp = mat;
		temp.transposeInPlace();
		OvMaths::FMatrix4 ret;
		memcpy(ret.data, temp.data(), 16 * sizeof(float));
		return ret;
	}
	class GL2DRender
	{
	public:
		GL2DRender()
		{
		}
		virtual void init()
		{
		}
		virtual ~GL2DRender()
		{
			destory();
		}
		void Vertex(const Eigen::Vector2f& v, const Eigen::Vector4f& c)
		{
			vertices.push_back(v);
			color.push_back(c);
		}
		virtual void create(const char* V, const char* F)
		{
			unsigned vP = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vP, 1, &V, NULL);
			unsigned fP = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fP, 1, &F, NULL);
			char infolog[512];
			int success;
			glCompileShader(vP);
			glGetShaderiv(vP, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(vP, 512, NULL, infolog);
				assert(false);
			}
			glCompileShader(fP);
			glGetShaderiv(fP, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fP, 512, NULL, infolog);
				assert(false);
			}
			shader = glCreateProgram();
			glAttachShader(shader, vP);
			glAttachShader(shader, fP);
			glLinkProgram(shader);
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 512, NULL, infolog);
				assert(false);
			}
			glDeleteShader(vP);
			glDeleteShader(fP);
			projection = glGetUniformLocation(shader, "projectionMatrix");
			glGenVertexArrays(1, &vaoId);
			glBindVertexArray(vaoId);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glGenBuffers(2, bufferId);
			glBindBuffer(GL_ARRAY_BUFFER, bufferId[0]);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * 8, vertices.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, bufferId[1]);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
			glBufferData(GL_ARRAY_BUFFER, color.size() * 16, color.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
		virtual void destory()
		{
			glDeleteBuffers(2, bufferId);
			glDeleteVertexArrays(1, &vaoId);
			glDeleteProgram(shader);
		}
		virtual void draw()
		{
		}
		virtual void fflush()
		{
			if (vertices.size() == 0)
				return;
			glUseProgram(shader);
			float p[16] = { 0.0 };
			glUniformMatrix4fv(projection, 1, GL_FALSE, p);
			glBindVertexArray(vaoId);

			glBindBuffer(GL_ARRAY_BUFFER, bufferId[0]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * 8, vertices.data());
			glBindBuffer(GL_ARRAY_BUFFER, bufferId[1]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, color.size() * 16, color.data());
			draw();
			glBindVertexArray(0);
			glUseProgram(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	protected:
		unsigned int vaoId;
		unsigned int bufferId[2];
		unsigned int shader;
		std::vector<Eigen::Vector2f> vertices;
		std::vector<Eigen::Vector4f> color;
		int projection;
	};

	class GLRenderPoints : public GL2DRender
	{
	public:
		GLRenderPoints()
			: GL2DRender()
		{
		}
		virtual ~GLRenderPoints() override
		{
			glDeleteBuffers(1, &sizeBuffer);
		}
		virtual void init() override
		{
			const char* V = "#version 330\n"
				"uniform mat4 projectionMatrix;\n"
				"layout(location = 0) in vec2 v_position;\n"
				"layout(location = 1) in vec4 v_color;\n"
				"layout(location = 2) in float v_size;\n"
				"out vec4 f_color;\n"
				"void main(void)\n"
				"{\n"
				"	f_color = v_color;\n"
				"	gl_Position = projectionMatrix*vec4(v_position, 0.0f, 1.0f);\n"
				"   gl_PointSize = v_size;\n"
				"}\n";

			const char* F = "#version 330\n"
				"in vec4 f_color;\n"
				"out vec4 color;\n"
				"void main(void)\n"
				"{\n"
				"	color = f_color;\n"
				"}\n";
			create(V, F);
		}
		virtual void create(const char* V, const char* F) override
		{
			GL2DRender::create(V, F);
			glBindVertexArray(vaoId);
			glEnableVertexAttribArray(2);
			glGenBuffers(1, &sizeBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, sizeBuffer);
			glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
			glBufferData(GL_ARRAY_BUFFER, size.size() * 4, size.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
		void Vertex(const Eigen::Vector2f& v, const Eigen::Vector4f& c, float s)
		{
			size.push_back(s);
			GL2DRender::Vertex(v, c);
		}
		virtual void draw() override
		{
			glBindBuffer(GL_ARRAY_BUFFER, sizeBuffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, size.size() * 4, size.data());

			glEnable(GL_PROGRAM_POINT_SIZE);
			glDrawArrays(GL_POINTS, 0, vertices.size());
			glDisable(GL_PROGRAM_POINT_SIZE);
		}

	private:
		std::vector<float> size; // ��С
		unsigned int sizeBuffer;
	};

	class GLRenderLines : public GL2DRender
	{
	public:
		GLRenderLines()
		{
		}
		virtual void init() override
		{
			const char* V = "#version 330\n"
				"uniform mat4 projectionMatrix;\n"
				"layout(location = 0) in vec2 v_position;\n"
				"layout(location = 1) in vec4 v_color;\n"
				"out vec4 f_color;\n"
				"void main(void)\n"
				"{\n"
				"	f_color = v_color;\n"
				"	gl_Position =projectionMatrix*vec4(v_position, 0.0f, 1.0f);\n"

				"}\n";

			const char* F = "#version 330\n"
				"in vec4 f_color;\n"
				"out vec4 color;\n"
				"void main(void)\n"
				"{\n"
				"	color = f_color;\n"
				"}\n";
			create(V, F);
		}
		virtual void draw() override
		{
			glDrawArrays(GL_LINES, 0, vertices.size());
		}
	};

	class GLRenderTriangles : public GL2DRender
	{
	public:
		GLRenderTriangles()
			: GL2DRender()
		{
		}
		virtual void init() override
		{
			const char* V = "#version 330\n"
				"uniform mat4 projectionMatrix;\n"
				"layout(location = 0) in vec2 v_position;\n"
				"layout(location = 1) in vec4 v_color;\n"
				"out vec4 f_color;\n"
				"void main(void)\n"
				"{\n"
				"	f_color = v_color;\n"
				"	gl_Position = projectionMatrix*vec4(v_position, 0.0f, 1.0f);\n"

				"}\n";

			const char* F = "#version 330\n"
				"in vec4 f_color;\n"
				"out vec4 color;\n"
				"void main(void)\n"
				"{\n"
				"	color = f_color;\n"
				"}\n";
			create(V, F);
		}
		virtual void draw() override
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDrawArrays(GL_TRIANGLES, 0, vertices.size());
			glDisable(GL_BLEND);
		}
	};


	static constexpr unsigned int IdInvalid = 0;
	static constexpr unsigned int kFnv1aPrime32 = 0x01000193u;
	static Eigen::Vector4<uint8_t> Color_Gold = Eigen::Vector4<uint8_t>(255, 0, 215, 255);
	static Eigen::Vector4<uint8_t> Color_Red = Eigen::Vector4<uint8_t>(255, 0, 0, 255);
	static Eigen::Vector4<uint8_t> Color_Green = Eigen::Vector4<uint8_t>(255, 0, 255, 0);
	static Eigen::Vector4<uint8_t> Color_Blue = Eigen::Vector4<uint8_t>(255, 255, 0, 0);
	static Eigen::Vector4<uint8_t> Color_Brown = Eigen::Vector4<uint8_t>(255, 19, 69, 139);
	static Eigen::Vector4<uint8_t> Color_White = Eigen::Vector4<uint8_t>(255, 255, 255, 255);
	static Eigen::Vector4<uint8_t> Color_GB = Eigen::Vector4<uint8_t>(255, 255, 255, 0);
	static Eigen::Vector4<uint8_t> Color_AR = Eigen::Vector4<uint8_t>(255, 115, 115, 76);
	static Eigen::Vector4<uint8_t> Color_ARB = Eigen::Vector4<uint8_t>(255, 232, 162, 0);
	static unsigned int VertexArray;
	static unsigned int VertexBuffer;
	static constexpr const size_t MiB = 1024u * 1024u;
	Guizmo::Guizmo()
		: buffer(9 * MiB, 27 * MiB, false)
		, command(driver, buffer.getCircularBuffer())
	{
		command.test(8);
		command.queueCommand([]() {
			// std::cout << "say hello" << std::endl;
			}
		);

	}

	Guizmo& Guizmo::instance()
	{
		static Guizmo guizmo;
		return guizmo;
	}
	void Guizmo::init()
	{
		prepareGl();
		registerDebugSettings();
		// ResetId();
		sortCalled = false;
		endFrameCalled = false;
		primMode = PrimitiveModeNone;
		vertexDataIndex = 0; // = sorting disabled
		layerIndex = 0;
		firstVertThisPrim = 0;
		vertCountThisPrim = 0;
		gizmoLocal = false;
		gizmoMode = GizmoModeTranslation;

		// m_gizmoLocal = true;
		// m_gizmoMode = GizmoMode_Rotation;
		hotId = IdInvalid;
		activeId = IdInvalid;
		hotId2D = IdInvalid;
		activeId2D = IdInvalid;
		appId = IdInvalid;
		appActiveId = IdInvalid;
		appHotId = IdInvalid;
		hotDepth = FLT_MAX;
		gizmoHeightPixels = 64.0f;
		gizmoSizePixels = 5.0f;

		memset(&keyDownCurr, 0, sizeof(keyDownCurr));
		memset(&keyDownPrev, 0, sizeof(keyDownPrev));
		matrixStack.push_back(Eigen::Matrix4f::Identity());
		colorStack.push_back(Eigen::Vector4<uint8_t>(255, 255, 255, 255));
		alphaStack.push_back(1.0f);
		sizeStack.push_back(1.0f);
		pushEnableSorting(false);
		pushLayerId(0);
		pushId(0x811C9DC5u);
	}
	void Guizmo::preStoreMesh()
	{
		//std::string auxMeshFolder = Engine::instance()->getAssetPath() + "EditorAuxiliaryMesh\\";
		////MeshBuilder builder;
		////intrusive_ptr<Mesh> mesh = builder.build(auxMeshFolder+ "Sph.json");
		//mPreStoredMesh["Sphere"] = MeshManager::Ptr->getOrCreateMesh("Sphere");
		//mPreStoredMesh["Sphere"]->updateDrawData();
		//mPreStoredMesh["ClipCone"] = MeshManager::Ptr->getOrCreateMesh("ClipCone");
		//mPreStoredMesh["ClipCone"]->updateDrawData();
		//mPreStoredMesh["XCone"] = MeshManager::Ptr->getOrCreateMesh("XCone");
		//mPreStoredMesh["XCone"]->updateDrawData();

	}
	void Guizmo::prepareGl()
	{
		std::string shaderPath = std::string(PROJECT_ENGINE_PATH) + std::string("/Shaders/");
		mLineMaterial = new OvRendering::Data::Material(OvRendering::Resources::Loaders::ShaderLoader::Create(shaderPath + "/GizmoLine.ovfx"));
		mPointMaterial = new OvRendering::Data::Material(OvRendering::Resources::Loaders::ShaderLoader::Create(shaderPath + "/GizmoPoint.ovfx"));
		mTriangleMaterial = new OvRendering::Data::Material(OvRendering::Resources::Loaders::ShaderLoader::Create(shaderPath + "/GizmoTriangle.ovfx"));

		glGenBuffers(1, &VertexBuffer);
		glGenVertexArrays(1, &VertexArray);
		glBindVertexArray(VertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, positionSize));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, color));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, normal));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, uv));
		glBindVertexArray(0);
		create2DRender();
		preStoreMesh();
	}
	void Guizmo::registerDebugSettings()
	{
		DebugSettings::instance().registerFloatReference("View", "FixedscaleValue", debugSettings.scaleValue, 0.5f, 10.0f);
	}
	void Guizmo::terminate()
	{
		// free resource
		delete m_lines;
		delete m_points;
		delete m_triangles;
		delete mLineMaterial;
		delete mPointMaterial;
		delete mTriangleMaterial;
	}
	void Guizmo::begin(PrimitiveMode _mode)
	{
		assert(!endFrameCalled); // Begin*() called after EndFrame() but before NewFrame(), or forgot to call NewFrame()
		assert(primMode == PrimitiveModeNone); // forgot to call End()
		primMode = _mode;
		vertCountThisPrim = 0;
		switch (primMode)
		{
		case PrimitiveModePoints:
			primType = DrawPrimitivePoints;
			break;
		case PrimitiveModeLines:
		case PrimitiveModeLineStrip:
		case PrimitiveModeLineLoop:
			primType = DrawPrimitiveLines;
			break;
		case PrimitiveModeTriangles:
		case PrimitiveModeTriangleStrip:
			primType = DrawPrimitiveTriangles;
			break;
		default:
			break;
		};
		firstVertThisPrim = getCurrentVertexList()->size();
	}
	void Guizmo::end()
	{
		assert(primMode != PrimitiveModeNone); // End() called without Begin*()
		if (vertCountThisPrim > 0)
		{
			VertexList* vertexList = getCurrentVertexList();
			switch (primMode)
			{
			case PrimitiveModePoints:
				break;
			case PrimitiveModeLines:
				assert(vertCountThisPrim % 2 == 0);
				break;
			case PrimitiveModeLineStrip:
				assert(vertCountThisPrim > 1);
				break;
			case PrimitiveModeLineLoop:
				assert(vertCountThisPrim > 1);
				vertexList->push_back(vertexList->back());
				vertexList->push_back((*vertexList)[firstVertThisPrim]);
				break;
			case PrimitiveModeTriangles:
				assert(vertCountThisPrim % 3 == 0);
				break;
			case PrimitiveModeTriangleStrip:
				assert(vertCountThisPrim >= 3);
				break;
			default:
				break;
			};
		}
		primMode = PrimitiveModeNone;
		primType = DrawPrimitiveCount;
	}
	void Guizmo::vertex(const Eigen::Vector3f& p, const Eigen::Vector4f& c, const Eigen::Vector3f& n, const Eigen::Vector2f& tex)
	{
		litVertexArray.push_back(VertexData(p, c, n, tex));
	}

	void Guizmo::vertex(const Eigen::Vector3f& p, const Eigen::Vector4<uint8_t>& c, const Eigen::Vector3f& n, const Eigen::Vector2f& tex)
	{
		litVertexArray.push_back(VertexData(p, c, n, tex));
	}
	void Guizmo::vertex(const Eigen::Vector3f& _position, float _size, const Eigen::Vector4<uint8_t>& _color)
	{
		assert(primMode != PrimitiveModeNone); // Vertex() called without Begin*()

		VertexData vd(_position, _size, _color);

		if (matrixStack.size() > 1) // optim, skip the matrix multiplication when the stack size is 1
		{
			Eigen::Vector3f pos = MatrixMulPoint(matrixStack.back(), _position);

			vd.positionSize = Eigen::Vector4<float>(pos.x(), pos.y(), pos.z(), _size);
		}
		vd.color(0) = vd.color(0) * alphaStack.back();

		VertexList* vertexList = getCurrentVertexList();
		switch (primMode)
		{
		case PrimitiveModePoints:
		case PrimitiveModeLines:
		case PrimitiveModeTriangles:
			vertexList->push_back(vd);
			break;
		case PrimitiveModeLineStrip:
		case PrimitiveModeLineLoop:
			if (vertCountThisPrim >= 2)
			{
				vertexList->push_back(vertexList->back());
				++vertCountThisPrim;
			}
			vertexList->push_back(vd);
			break;
		case PrimitiveModeTriangleStrip:
			if (vertCountThisPrim >= 3)
			{
				vertexList->push_back(*(vertexList->end() - 2));
				vertexList->push_back(*(vertexList->end() - 2));
				vertCountThisPrim += 2;
			}
			vertexList->push_back(vd);
			break;
		default:
			break;
		};
		++vertCountThisPrim;
	}



	void Guizmo::drawOneMesh(Eigen::Vector3f& translation, Eigen::Matrix3f& rotation, Eigen::Vector3f& scale,
		const std::string& mesh, bool longterm)
	{
		//drawOneMesh(Coord3(translation, rotation, scale), mesh, longterm);
	}

	void Guizmo::drawOneMesh(Eigen::Vector3f& translation, Eigen::Matrix3f& rotation, Eigen::Vector3f& scale,
		Eigen::Vector3f& color, const std::string& mesh, bool longterm)
	{
		//drawOneMesh(Coord3(translation, rotation, scale), mesh, color, longterm);
	}

	void Guizmo::drawOneMesh(
		Eigen::Vector3f translation, Eigen::Vector3f scale, const std::string& mesh, bool longterm)
	{
		//drawOneMesh(Coord3(translation, Eigen::Matrix3f::Identity(), scale), mesh, longterm);
	}

	void Guizmo::drawOneMesh(
		Eigen::Vector3f translation, Eigen::Vector3f scale, Eigen::Vector3f& color, const std::string& mesh, bool longterm)
	{
		//drawOneMesh(Coord3(translation, Eigen::Matrix3f::Identity(), scale), mesh, color, longterm);
	}

	void Guizmo::drawOneMesh(Eigen::Matrix4f& model, const std::string& name, bool longterm)
	{
		if (longterm)
		{
			drawLongTermMeshList.push_back({ name, model });
		}
		else
		{
			drawMeshList.push_back({ name,model });
		}
	}

	void Guizmo::drawOneMesh(Eigen::Matrix4f& model, const std::string& mesh, Eigen::Vector3f& color, bool longterm)
	{
		if (longterm)
		{
			drawLongTermMeshList.push_back({ mesh, model,color });
		}
		else
		{
			drawMeshList.push_back({ mesh, model,color });
		}
	}

	void Guizmo::drawOneFixScaleMesh(Eigen::Matrix4f& model, const std::string& mesh, Eigen::Vector3f& color, bool longterm)
	{
		MeshInstance me = { mesh, model, color };
		me.fixScaled = true;
		if (longterm)
		{
			drawLongTermMeshList.push_back(me);
		}
		else
		{
			drawMeshList.push_back(me);
		}
	}

	void Guizmo::drawPoint(const Eigen::Vector3f& _position, float _size, Eigen::Vector4<uint8_t> _color)
	{
		begin(PrimitiveModePoints);
		vertex(_position, _size, _color);
		end();
	}

	void Guizmo::drawPoint(const Eigen::Vector3f& pos)
	{
		drawPoint(pos, sizeStack.back(), colorStack.back());
	}

	void Guizmo::drawPoint(const Eigen::Vector3f& pos, float size)
	{
		drawPoint(pos, size, colorStack.back());
	}

	void Guizmo::drawPointList(const std::vector<Eigen::Vector3f>& position, float size, Eigen::Vector4<uint8_t> color)
	{
		std::vector<VertexData> vd(position.size());
		float alpha = alphaStack.back();
		if (matrixStack.size() > 1) // optim, skip the matrix multiplication when the stack size is 1
		{
			for (int i = 0; i < position.size(); i++)
			{
				vd[i] = { MatrixMulPoint(matrixStack.back(), position[i]),size,color };
				vd[i].color[0] *= alpha;
			}
		}
		else
		{
			for (int i = 0; i < position.size(); i++)
			{
				vd[i] = { position[i], size, color };
				vd[i].color[0] *= alpha;
			}
		}
		VertexList* vertexList = vertexData[vertexDataIndex][layerIndex * DrawPrimitiveCount + DrawPrimitivePoints];
		vertexList->insert(vertexList->end(), vd.begin(), vd.end());
	}

	void Guizmo::drawLineList(const std::vector<Eigen::Vector3f>& position, float _size, Eigen::Vector4<uint8_t> _color)
	{
		std::vector<VertexData> vd(position.size());
		float alpha = alphaStack.back();
		if (matrixStack.size() > 1) // optim, skip the matrix multiplication when the stack size is 1
		{
			for (int i = 0; i < position.size(); i++)
			{
				vd[i] = { MatrixMulPoint(matrixStack.back(), position[i]), _size, _color };
				vd[i].color[0] *= alpha;
			}
		}
		else
		{
			for (int i = 0; i < position.size(); i++)
			{
				vd[i] = { position[i], _size, _color };
				vd[i].color[0] *= alpha;
			}
		}
		VertexList* vertexList = vertexData[vertexDataIndex][layerIndex * DrawPrimitiveCount + DrawPrimitiveLines];
		vertexList->insert(vertexList->end(), vd.begin(), vd.end());
	}

	void Guizmo::drawTriangleList(const std::vector<Eigen::Vector3f>& position, float _size, Eigen::Vector4<uint8_t> _color)
	{
		std::vector<VertexData> vd(position.size());
		float alpha = alphaStack.back();
		if (matrixStack.size() > 1) // optim, skip the matrix multiplication when the stack size is 1
		{
			for (int i = 0; i < position.size(); i++)
			{
				vd[i] = { MatrixMulPoint(matrixStack.back(), position[i]), _size, _color };
				vd[i].color[0] *= alpha;
			}
		}
		else
		{
			for (int i = 0; i < position.size(); i++)
			{
				vd[i] = { position[i], _size, _color };
				vd[i].color[0] *= alpha;
			}
		}
		VertexList* vertexList = vertexData[vertexDataIndex][layerIndex * DrawPrimitiveCount + DrawPrimitiveTriangles];
		vertexList->insert(vertexList->end(), vd.begin(), vd.end());
	}

	void Guizmo::drawLine(
		const Eigen::Vector3f& _a, const Eigen::Vector3f& _b, float _size, Eigen::Vector4<uint8_t> _color)
	{
		begin(PrimitiveModeLines);
		vertex(_a, _size, _color);
		vertex(_b, _size, _color);
		end();
	}

	void Guizmo::drawLine(const Eigen::Vector3f& _a, const Eigen::Vector3f& _b, float _size)
	{
		drawLine(_a, _b, _size, colorStack.back());
	}

	void Guizmo::drawLine(const Eigen::Vector3f& _a, const Eigen::Vector3f& _b)
	{
		drawLine(_a, _b, sizeStack.back());
	}

	void Guizmo::drawTriangle(const Eigen::Vector3f& a, const Eigen::Vector3f& b, const Eigen::Vector3f& c, const Eigen::Vector3f& n)
	{

		auto& matrix = matrixStack.back();
		Eigen::Vector3f mn = MatrixMulDir(matrix, n);
		litVertexArray.push_back({ MatrixMulPoint(matrix, a), colorStack.back(), mn });
		litVertexArray.push_back({ MatrixMulPoint(matrix, b), colorStack.back(), mn });
		litVertexArray.push_back({ MatrixMulPoint(matrix, c), colorStack.back(), mn });
	}

	void Guizmo::drawTriangle(const Eigen::Vector3f& _a, const Eigen::Vector3f& _b, const Eigen::Vector3f& _c)
	{
		drawTriangle(_a, _b, _c, colorStack.back());
	}

	void Guizmo::drawTriangle(const Eigen::Vector3f& _a, const Eigen::Vector3f& _b, const Eigen::Vector3f& _c,
		const Eigen::Vector4<uint8_t>& _color)
	{
		colorStack.push_back(_color);
		begin(PrimitiveModeTriangles);
		vertex(_a);
		vertex(_b);
		vertex(_c);
		end();
		colorStack.pop_back();
	}


	void Guizmo::drawQuadFilled(
		const Eigen::Vector3f& _origin, const Eigen::Vector3f& _normal, const Eigen::Vector2<float>& _size)
	{
		Eigen::Matrix4f matrix = matrixStack.back() * LookAt(_origin, _origin + _normal);
		matrixStack.push_back(matrix);
		drawQuadFilled(Eigen::Vector3f(-_size.x(), -_size.y(), 0.0f),
			Eigen::Vector3f(_size.x(), -_size.y(), 0.0f), Eigen::Vector3f(_size.x(), _size.y(), 0.0f),
			Eigen::Vector3f(-_size.x(), _size.y(), 0.0f));
		matrixStack.pop_back();
	}

	void Guizmo::drawQuad(const Eigen::Vector3f& _a, const Eigen::Vector3f& _b, const Eigen::Vector3f& _c,
		const Eigen::Vector3f& _d)
	{
		begin(PrimitiveModeLineLoop);
		vertex(_a);
		vertex(_b);
		vertex(_c);
		vertex(_d);
		end();
	}

	void Guizmo::drawQuad(
		const Eigen::Vector3f& _origin, const Eigen::Vector3f& _normal, const Eigen::Vector2<float>& _size)
	{
		matrixStack.push_back(matrixStack.back() * LookAt(_origin, _origin + _normal));
		drawQuad(Eigen::Vector3f(-_size.x(), _size.y(), 0.0f), Eigen::Vector3f(_size.x(), _size.y(), 0.0f),
			Eigen::Vector3f(_size.x(), -_size.y(), 0.0f), Eigen::Vector3f(-_size.x(), -_size.y(), 0.0f));
		matrixStack.pop_back();
	}

	void Guizmo::drawQuadFilled(const Eigen::Vector3f& _a, const Eigen::Vector3f& _b,
		const Eigen::Vector3f& _c, const Eigen::Vector3f& _d)
	{

		begin(PrimitiveModeTriangles);
		vertex(_a);
		vertex(_b);
		vertex(_c);
		vertex(_a);
		vertex(_c);
		vertex(_d);
		end();
	}
	void Guizmo::drawArrow(const Eigen::Vector3f& _start, const Eigen::Vector3f& _end, float _headLength,
		float axisThickness, float _headThickness)
	{
		sizeStack.push_back(axisThickness);
		drawArrow(_start, _end, _headLength, _headThickness);
		sizeStack.pop_back();
	}
	void Guizmo::drawArrow(
		const Eigen::Vector3f& _start, const Eigen::Vector3f& _end, float _headLength, float _headThickness)
	{

		if (_headThickness < 0.0f)
		{
			_headThickness = sizeStack.back() * 2.0f;
		}
		Eigen::Vector3f dir = _end - _start;
		float dirlen = dir.norm();
		if (_headLength < 0.0f)
		{
			_headLength = std::min(dirlen / 2.0f, pixelsToWorldSize(_end, _headThickness * 2.0f));
		}
		dir = dir / dirlen;

		Eigen::Vector3f head = _end - dir * _headLength;

		begin(PrimitiveModeLines);
		vertex(_start);
		vertex(head);
		vertex(head, _headThickness, colorStack.back());
		vertex(_end, 2.0f, colorStack.back()); // \hack \todo 2.0f here compensates for the shader antialiasing (which
		// reduces alpha when size < 2)
		end();
	}

	void Guizmo::drawArrow(unsigned int _id, const Eigen::Vector3f& _origin, const Eigen::Vector3f& _axis,
		float _worldHeight, Eigen::Vector4<uint8_t> _color)
	{

		Eigen::Vector3f viewDir =
			cameraParam.orthProj ? cameraParam.viewDirectioin : (cameraParam.eye - _origin);

		Eigen::Vector4<uint8_t> color = _color;
		if (_id == activeId)
		{
			color = Color_Gold;

			begin(PrimitiveModeLines);
			vertex(_origin - _axis * 999.0f, gizmoSizePixels * 0.5f, _color);
			vertex(_origin + _axis * 999.0f, gizmoSizePixels * 0.5f, _color);
			end();

		}
		else if (_id == hotId)
		{
			color = Color_Gold;
		}
		colorStack.push_back(color);
		sizeStack.push_back(2.0);

		drawArrow(_origin + _axis * _worldHeight * 0.1, _origin + _axis * _worldHeight, -1, 8);
		sizeStack.pop_back();
		colorStack.pop_back();
	}

	void Guizmo::drawCircleFilled(
		const Eigen::Vector3f& _origin, const Eigen::Vector3f& _normal, float _radius, int _detail)
	{
		if (_detail < 0)
		{
			_detail = estimateLevelOfDetail(_origin, _radius, 8, 64);
		}
		_detail = std::max(_detail, 3);

		matrixStack.push_back(matrixStack.back() * LookAt(_origin, _origin + _normal));

		begin(PrimitiveModeTriangles);
		float cp = _radius;
		float sp = 0.0f;
		for (int i = 1; i <= _detail; ++i)
		{
			vertex(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
			vertex(Eigen::Vector3f(cp, sp, 0.0f));
			float rad = TwoPi * ((float)i / (float)_detail);
			float c = cosf(rad) * _radius;
			float s = sinf(rad) * _radius;
			vertex(Eigen::Vector3f(c, s, 0.0f));
			cp = c;
			sp = s;
		}
		end();
		matrixStack.pop_back();
	}

	void Guizmo::drawTransparencyCircle(const Eigen::Vector3f& _origin, const Eigen::Vector3f& _normal, float _radius, int _detail)
	{
		alphaStack.push_back(0.5);
		colorStack.push_back(Color_Green);
		pushEnableSorting(true);
		drawCircleFilled(_origin, _normal, _radius, _detail);
		popEnableSorting();
		alphaStack.pop_back();
		colorStack.pop_back();
		sizeStack.push_back(3.0);
		colorStack.push_back(Color_Green);
		drawCircle(_origin, _normal, _radius, _detail);
		sizeStack.pop_back();
		colorStack.pop_back();
	}

	void Guizmo::drawSphere(const Eigen::Vector3f& _origin, float _radius, int _detail)
	{
		if (_detail < 0)
		{
			_detail = estimateLevelOfDetail(_origin, _radius, 12, 32);
		}
		_detail = std::max(_detail, 6);
		// xy circle
		begin(PrimitiveModeLineLoop);
		for (int i = 0; i < _detail; ++i)
		{
			float rad = TwoPi * ((float)i / (float)_detail);
			vertex(Eigen::Vector3f(cosf(rad) * _radius + _origin.x(), sinf(rad) * _radius + _origin.y(), 0.0f + _origin.z()));
		}
		end();
		// xz circle
		begin(PrimitiveModeLineLoop);
		for (int i = 0; i < _detail; ++i)
		{
			float rad = TwoPi * ((float)i / (float)_detail);
			vertex(
				Eigen::Vector3f(cosf(rad) * _radius + _origin.x(), 0.0f + _origin.y(), sinf(rad) * _radius + _origin.z()));
		}
		end();
		// yz circle
		begin(PrimitiveModeLineLoop);
		for (int i = 0; i < _detail; ++i)
		{
			float rad = TwoPi * ((float)i / (float)_detail);
			vertex(Eigen::Vector3f(0.0f + _origin.x(), cosf(rad) * _radius + _origin.y(), sinf(rad) * _radius + _origin.z()));
		}
		end();

	}

	void Guizmo::drawSphereFilled(const Eigen::Vector3f& _origin, float _radius, int _detail)
	{
		if (_detail < 0)
		{
			_detail = estimateLevelOfDetail(_origin, _radius, 12, 32);
		}
		_detail = std::max(_detail, 6);

		begin(PrimitiveModeTriangles);
		float yp = -_radius;
		float rp = 0.0f;
		auto& c = colorStack.back();
		for (int i = 1; i <= _detail / 2; ++i)
		{
			float y = ((float)i / (float)(_detail / 2)) * 2.0f - 1.0f;
			float r = cosf(y * HalfPi) * _radius;
			y = sinf(y * HalfPi) * _radius;
			float u = (float)i / (float)(_detail);
			float xp = 1.0f;
			float zp = 0.0f;
			for (int j = 1; j <= _detail; ++j)
			{
				float x = ((float)j / (float)(_detail)) * TwoPi;
				float z = sinf(x);
				float v = (float)j / (float)(_detail);
				x = cosf(x);
				if (enableLit)
				{
					vertex(Eigen::Vector3f(xp * rp + _origin.x(), yp + _origin.y(), zp * rp + _origin.z()), c, Eigen::Vector3f(xp * rp, yp, zp * rp).normalized(), { u, v });
					vertex(Eigen::Vector3f(xp * r + _origin.x(), y + _origin.y(), zp * r + _origin.z()), c, Eigen::Vector3f(xp * r, y, zp * r).normalized(), { u,v });
					vertex(Eigen::Vector3f(x * r + _origin.x(), y + _origin.y(), z * r + _origin.z()), c, Eigen::Vector3f(x * r, y, z * r).normalized(), { u, v });

					vertex(Eigen::Vector3f(xp * rp + _origin.x(), yp + _origin.y(), zp * rp + _origin.z()), c, Eigen::Vector3f(xp * rp, yp, zp * rp).normalized(), { u, v });
					vertex(Eigen::Vector3f(x * r + _origin.x(), y + _origin.y(), z * r + _origin.z()), c, Eigen::Vector3f(x * r, y, z * r).normalized(), { u, v });
					vertex(Eigen::Vector3f(x * rp + _origin.x(), yp + _origin.y(), z * rp + _origin.z()), c, Eigen::Vector3f(x * rp, yp, z * rp).normalized(), { u, v });
				}
				else
				{
					vertex(Eigen::Vector3f(xp * rp + _origin.x(), yp + _origin.y(), zp * rp + _origin.z()));
					vertex(Eigen::Vector3f(xp * r + _origin.x(), y + _origin.y(), zp * r + _origin.z()));
					vertex(Eigen::Vector3f(x * r + _origin.x(), y + _origin.y(), z * r + _origin.z()));

					vertex(Eigen::Vector3f(xp * rp + _origin.x(), yp + _origin.y(), zp * rp + _origin.z()));
					vertex(Eigen::Vector3f(x * r + _origin.x(), y + _origin.y(), z * r + _origin.z()));
					vertex(Eigen::Vector3f(x * rp + _origin.x(), yp + _origin.y(), z * rp + _origin.z()));
				}
				xp = x;
				zp = z;
			}
			yp = y;
			rp = r;
		}
		end();
	}

	void Guizmo::drawCircleFaceCamera(const Eigen::Vector3f& _origin)
	{
		sizeStack.push_back(2.8);
		drawCircle(_origin, cameraParam.viewDirectioin, pixelsToWorldSize(_origin, 7.5), 40);
		sizeStack.pop_back();
	}

	void Guizmo::drawCircle(const Eigen::Vector3f& _origin, const Eigen::Vector3f& _normal, float _radius, int _detail)
	{
		if (_detail < 0)
		{
			_detail = estimateLevelOfDetail(_origin, _radius, 8, 48);
		}
		_detail = std::max(_detail, 3);
		matrixStack.push_back(matrixStack.back() * LookAt(_origin, _origin + _normal));
		begin(PrimitiveModeLineLoop);
		for (int i = 0; i < _detail; ++i)
		{
			float rad = TwoPi * ((float)i / (float)_detail);
			vertex(Eigen::Vector3f(cosf(rad) * _radius, sinf(rad) * _radius, 0.0f));
		}
		end();
		matrixStack.pop_back();
	}

	void Guizmo::drawConeFilled(const Eigen::Vector3f& _origin, const Eigen::Vector3f& _normal, float height,
		float _radius, int _detail)
	{
		if (_detail < 0)
		{
			_detail = estimateLevelOfDetail(_origin + _normal * height / 2, height / 2, 8, 48);
		}
		_detail = std::max(_detail, 3);
		// cone bottom face
		drawCircleFilled(_origin, _normal, _radius, _detail);
		// cone side face
		matrixStack.push_back(matrixStack.back() * LookAt(_origin, _origin + _normal));
		begin(PrimitiveModeTriangles);
		float cp = _radius;
		float sp = 0.0f;
		for (int i = 1; i <= _detail; ++i)
		{
			vertex(Eigen::Vector3f(0, 0, 1) * height);
			vertex(Eigen::Vector3f(cp, sp, 0.0f));
			float rad = TwoPi * ((float)i / (float)_detail);
			float c = cosf(rad) * _radius;
			float s = sinf(rad) * _radius;
			vertex(Eigen::Vector3f(c, s, 0.0f));
			cp = c;
			sp = s;
		}
		end();
		matrixStack.pop_back();
	}

	void Guizmo::drawAlignedBox(const Eigen::Vector3f& _min, const Eigen::Vector3f& _max)
	{
		begin(PrimitiveModeLineLoop);
		vertex(Eigen::Vector3f(_min.x(), _min.y(), _min.z()));
		vertex(Eigen::Vector3f(_max.x(), _min.y(), _min.z()));
		vertex(Eigen::Vector3f(_max.x(), _min.y(), _max.z()));
		vertex(Eigen::Vector3f(_min.x(), _min.y(), _max.z()));
		end();
		begin(PrimitiveModeLineLoop);
		vertex(Eigen::Vector3f(_min.x(), _max.y(), _min.z()));
		vertex(Eigen::Vector3f(_max.x(), _max.y(), _min.z()));
		vertex(Eigen::Vector3f(_max.x(), _max.y(), _max.z()));
		vertex(Eigen::Vector3f(_min.x(), _max.y(), _max.z()));
		end();
		begin(PrimitiveModeLines);
		vertex(Eigen::Vector3f(_min.x(), _min.y(), _min.z()));
		vertex(Eigen::Vector3f(_min.x(), _max.y(), _min.z()));
		vertex(Eigen::Vector3f(_max.x(), _min.y(), _min.z()));
		vertex(Eigen::Vector3f(_max.x(), _max.y(), _min.z()));
		vertex(Eigen::Vector3f(_min.x(), _min.y(), _max.z()));
		vertex(Eigen::Vector3f(_min.x(), _max.y(), _max.z()));
		vertex(Eigen::Vector3f(_max.x(), _min.y(), _max.z()));
		vertex(Eigen::Vector3f(_max.x(), _max.y(), _max.z()));
		end();
	}

	void Guizmo::drawAlignedBoxFilled(const Eigen::Vector3f& _min, const Eigen::Vector3f& _max)
	{
		// x+
		drawQuadFilled(Eigen::Vector3f(_max.x(), _max.y(), _min.z()),
			Eigen::Vector3f(_max.x(), _max.y(), _max.z()), Eigen::Vector3f(_max.x(), _min.y(), _max.z()),
			Eigen::Vector3f(_max.x(), _min.y(), _min.z()));
		// x-
		drawQuadFilled(Eigen::Vector3f(_min.x(), _min.y(), _min.z()),
			Eigen::Vector3f(_min.x(), _min.y(), _max.z()), Eigen::Vector3f(_min.x(), _max.y(), _max.z()),
			Eigen::Vector3f(_min.x(), _max.y(), _min.z()));
		// y+
		drawQuadFilled(Eigen::Vector3f(_min.x(), _max.y(), _min.z()),
			Eigen::Vector3f(_min.x(), _max.y(), _max.z()), Eigen::Vector3f(_max.x(), _max.y(), _max.z()),
			Eigen::Vector3f(_max.x(), _max.y(), _min.z()));
		// y-
		drawQuadFilled(Eigen::Vector3f(_max.x(), _min.y(), _min.z()),
			Eigen::Vector3f(_max.x(), _min.y(), _max.z()), Eigen::Vector3f(_min.x(), _min.y(), _max.z()),
			Eigen::Vector3f(_min.x(), _min.y(), _min.z()));
		// z+
		drawQuadFilled(Eigen::Vector3f(_max.x(), _min.y(), _max.z()),
			Eigen::Vector3f(_max.x(), _max.y(), _max.z()), Eigen::Vector3f(_min.x(), _max.y(), _max.z()),
			Eigen::Vector3f(_min.x(), _min.y(), _max.z()));
		// z-
		drawQuadFilled(Eigen::Vector3f(_min.x(), _min.y(), _min.z()),
			Eigen::Vector3f(_min.x(), _max.y(), _min.z()), Eigen::Vector3f(_max.x(), _max.y(), _min.z()),
			Eigen::Vector3f(_max.x(), _min.y(), _min.z()));
	}

	void Guizmo::drawAlignedBoxFilled(
		const Eigen::Vector3f& _min, const Eigen::Vector3f& _max, const std::vector<Eigen::Vector4<uint8_t>>& color)
	{
		if (color.size() == 0)
		{
			drawAlignedBoxFilled(_min, _max);
		}
		else if (color.size() == 1)
		{
			colorStack.push_back(color[0]);
			drawAlignedBoxFilled(_min, _max);
			colorStack.pop_back();
		}
		else if (color.size() == 6)
		{
			colorStack.push_back(color[0]);
			// x+
			drawQuadFilled(Eigen::Vector3f(_max.x(), _max.y(), _min.z()), Eigen::Vector3f(_max.x(), _max.y(), _max.z()),
				Eigen::Vector3f(_max.x(), _min.y(), _max.z()), Eigen::Vector3f(_max.x(), _min.y(), _min.z()));
			colorStack.pop_back();
			colorStack.push_back(color[1]);
			// x-
			drawQuadFilled(Eigen::Vector3f(_min.x(), _min.y(), _min.z()), Eigen::Vector3f(_min.x(), _min.y(), _max.z()),
				Eigen::Vector3f(_min.x(), _max.y(), _max.z()), Eigen::Vector3f(_min.x(), _max.y(), _min.z()));
			colorStack.pop_back();
			colorStack.push_back(color[2]);
			// y+
			drawQuadFilled(Eigen::Vector3f(_min.x(), _max.y(), _min.z()), Eigen::Vector3f(_min.x(), _max.y(), _max.z()),
				Eigen::Vector3f(_max.x(), _max.y(), _max.z()), Eigen::Vector3f(_max.x(), _max.y(), _min.z()));
			colorStack.pop_back();

			colorStack.push_back(color[3]);
			// y-
			drawQuadFilled(Eigen::Vector3f(_max.x(), _min.y(), _min.z()), Eigen::Vector3f(_max.x(), _min.y(), _max.z()),
				Eigen::Vector3f(_min.x(), _min.y(), _max.z()), Eigen::Vector3f(_min.x(), _min.y(), _min.z()));
			colorStack.pop_back();
			colorStack.push_back(color[4]);
			// z+
			drawQuadFilled(Eigen::Vector3f(_max.x(), _min.y(), _max.z()), Eigen::Vector3f(_max.x(), _max.y(), _max.z()),
				Eigen::Vector3f(_min.x(), _max.y(), _max.z()), Eigen::Vector3f(_min.x(), _min.y(), _max.z()));
			colorStack.pop_back();
			colorStack.push_back(color[5]);
			// z-
			drawQuadFilled(Eigen::Vector3f(_min.x(), _min.y(), _min.z()), Eigen::Vector3f(_min.x(), _max.y(), _min.z()),
				Eigen::Vector3f(_max.x(), _max.y(), _min.z()), Eigen::Vector3f(_max.x(), _min.y(), _min.z()));
			colorStack.pop_back();
		}
	}

	void Guizmo::drawPlaneGrid(const Eigen::Vector3f& origin, const Eigen::Vector3f& normal, float scale, float gridSize)
	{
		// Draw plane grid

		const float gridHalf = (float)gridSize * 0.5f;
		Eigen::Vector3f up = abs(normal.y()) > 0.99 ? Eigen::Vector3f(1, 0, 0) : Eigen::Vector3f(0, 1, 0);
		Eigen::Vector3f xaxis = normal.cross(up).normalized();
		Eigen::Vector3f zaxis = xaxis.cross(normal).normalized();

		alphaStack.push_back(1.0f);
		sizeStack.push_back(1.0f);
		pushEnableSorting(true);
		begin(PrimitiveModeLines);
		for (int x = 0; x <= gridSize * scale; ++x)
		{
			float v = x * 1.0 / (scale);
			vertex(origin + xaxis * (gridHalf)+zaxis * (v - gridHalf), Eigen::Vector4<uint8_t>(255, 0, 0, 0));
			vertex(origin - xaxis * (gridHalf)+zaxis * (v - gridHalf), Color_Red);
		}
		for (int z = 0; z <= gridSize * scale; ++z)
		{
			float v = z * 1.0 / (scale);
			vertex(origin + zaxis * gridHalf + xaxis * (v - gridHalf), Eigen::Vector4<uint8_t>(255, 0, 0, 0));
			vertex(origin - zaxis * gridHalf + xaxis * (v - gridHalf), Color_Blue);
		}
		end();
		alphaStack.pop_back();
		sizeStack.pop_back();
		popEnableSorting();
	}

	bool Guizmo::drawManpulate(unsigned _id, Eigen::Matrix4f& model)
	{
		// Draw Ray hit point
		colorStack.push_back(Eigen::Vector4<uint8_t>(255, 0, 0, 255));
		matrixStack.push_back(Eigen::Matrix4f::Identity());
		drawSphereFilled(cameraParam.rayOrigin + cameraParam.rayDirection,
			pixelsToWorldSize(cameraParam.rayOrigin + cameraParam.rayDirection, 3.0));
		matrixStack.pop_back();
		colorStack.pop_back();

		matrixStack.push_back(model);
		bool ret = false;
		Eigen::Vector3f position = Eigen::Vector3f(model(0, 3), model(1, 3), model(2, 3));
		Eigen::Matrix3f rot = model.block(0, 0, 3, 3);
		switch (gizmoMode)
		{
		case GizmoModeTranslation:

			if (translation(_id, position, gizmoLocal))
			{
				model.block(0, 3, 3, 1) = position;
				ret = true;
			}
			break;
		case GizmoModeRotation: {

			if (rotation(_id, rot, gizmoLocal))
			{
				model.block(0, 0, 3, 3) = rot;
				ret = true;
			}
			break;
		}

		case GizmoModeScale:
			break;
		default:
			break;
		}
		matrixStack.pop_back();
		return ret;
	}

	bool Guizmo::drawManpulate(const char* _id, Eigen::Matrix4f& model)
	{
		return drawManpulate(makeId(_id), model);
	}
	void Guizmo::drawViewCube()
	{
		//return;
		//params to control
		float halflen = 3.0f;
		float shift = 0.6f;
		Eigen::Vector3f n = { 0, 0, 1 };
		Eigen::Vector3f A = { -halflen + shift,halflen - shift, halflen };
		Eigen::Vector3f A1 = { -halflen + 2 * shift, halflen - shift, halflen };
		Eigen::Vector3f A2 = { -halflen + shift, halflen - 2 * shift, halflen };
		Eigen::Vector3f B = { halflen - shift, halflen - shift, halflen };
		Eigen::Vector3f B1 = { halflen - 2 * shift, halflen - shift, halflen };
		Eigen::Vector3f B2 = { halflen - shift, halflen - 2 * shift, halflen };
		Eigen::Vector3f C = { halflen - shift, -halflen + shift, halflen };
		Eigen::Vector3f C1 = { halflen - 2 * shift, -halflen + shift, halflen };
		Eigen::Vector3f C2 = { halflen - shift, -halflen + 2 * shift, halflen };
		Eigen::Vector3f D = { -halflen + shift, -halflen + shift, halflen };
		Eigen::Vector3f D1 = { -halflen + 2 * shift, -halflen + shift, halflen };
		Eigen::Vector3f D2 = { -halflen + shift, -halflen + 2 * shift, halflen };
		Eigen::Vector3f F1 = B1;
		Eigen::Vector3f F2 = F1 + Eigen::Vector3f(0, shift, -shift);
		Eigen::Vector3f F3 = F2 + Eigen::Vector3f(shift, 0, -shift);
		Eigen::Vector3f F4 = F3 + Eigen::Vector3f(shift, -shift, 0);
		Eigen::Vector3f F5 = F4 + Eigen::Vector3f(0, -shift, +shift);
		Eigen::Vector3f F6 = B2;


		drawLine(A1, A2);
		drawLine(B1, B2);
		drawLine(C1, C2);
		drawLine(D1, D2);
		drawLine(A1, B1);
		drawLine(A2, D2);
		drawLine(C2, B2);
		drawLine(D1, C1);
		drawTriangle(A2, A1, B1, n);
		drawTriangle(A2, B1, B2, n);
		drawTriangle(A2, B2, C2, n);
		drawTriangle(A2, C2, C1, n);
		drawTriangle(A2, C1, D1, n);
		drawTriangle(A2, D1, D2, n);

		matrixStack.push_back(EulerXYZToMatrix4Degree({ 90, 0, 0 }));
		drawLine(A1, A2);
		drawLine(B1, B2);
		drawLine(C1, C2);
		drawLine(D1, D2);
		drawLine(A1, B1);
		drawLine(A2, D2);
		drawLine(C2, B2);
		drawLine(D1, C1);
		drawTriangle(A2, A1, B1, n);
		drawTriangle(A2, B1, B2, n);
		drawTriangle(A2, B2, C2, n);
		drawTriangle(A2, C2, C1, n);
		drawTriangle(A2, C1, D1, n);
		drawTriangle(A2, D1, D2, n);
		matrixStack.pop_back();


		matrixStack.push_back(EulerXYZToMatrix4Degree({ 180, 0, 0 }));
		drawLine(A1, A2);
		drawLine(B1, B2);
		drawLine(C1, C2);
		drawLine(D1, D2);
		drawLine(A1, B1);
		drawLine(A2, D2);
		drawLine(C2, B2);
		drawLine(D1, C1);
		drawTriangle(A2, A1, B1, n);
		drawTriangle(A2, B1, B2, n);
		drawTriangle(A2, B2, C2, n);
		drawTriangle(A2, C2, C1, n);
		drawTriangle(A2, C1, D1, n);
		drawTriangle(A2, D1, D2, n);
		matrixStack.pop_back();

		matrixStack.push_back(EulerXYZToMatrix4Degree({ 270, 0, 0 }));
		drawLine(A1, A2);
		drawLine(B1, B2);
		drawLine(C1, C2);
		drawLine(D1, D2);
		drawLine(A1, B1);
		drawLine(A2, D2);
		drawLine(C2, B2);
		drawLine(D1, C1);
		drawTriangle(A2, A1, B1, n);
		drawTriangle(A2, B1, B2, n);
		drawTriangle(A2, B2, C2, n);
		drawTriangle(A2, C2, C1, n);
		drawTriangle(A2, C1, D1, n);
		drawTriangle(A2, D1, D2, n);
		matrixStack.pop_back();

		matrixStack.push_back(EulerXYZToMatrix4Degree({ 0, 90, 0 }));
		drawLine(A1, A2);
		drawLine(B1, B2);
		drawLine(C1, C2);
		drawLine(D1, D2);
		drawLine(A1, B1);
		drawLine(A2, D2);
		drawLine(C2, B2);
		drawLine(D1, C1);
		drawTriangle(A2, A1, B1, n);
		drawTriangle(A2, B1, B2, n);
		drawTriangle(A2, B2, C2, n);
		drawTriangle(A2, C2, C1, n);
		drawTriangle(A2, C1, D1, n);
		drawTriangle(A2, D1, D2, n);
		matrixStack.pop_back();


		matrixStack.push_back(EulerXYZToMatrix4Degree({ 0, 270, 0 }));
		drawLine(A1, A2);
		drawLine(B1, B2);
		drawLine(C1, C2);
		drawLine(D1, D2);
		drawLine(A1, B1);
		drawLine(A2, D2);
		drawLine(C2, B2);
		drawLine(D1, C1);
		drawTriangle(A2, A1, B1, n);
		drawTriangle(A2, B1, B2, n);
		drawTriangle(A2, B2, C2, n);
		drawTriangle(A2, C2, C1, n);
		drawTriangle(A2, C1, D1, n);
		drawTriangle(A2, D1, D2, n);
		matrixStack.pop_back();

		{
			Eigen::Vector3f nn = Eigen::Vector3f(1, 1, 1).normalized();
			drawLine(F1, F2);
			drawLine(F2, F3);
			drawLine(F3, F4);
			drawLine(F4, F5);
			drawLine(F5, F6);
			drawLine(F6, F1);
			drawTriangle(F1, F2, F3, nn);
			drawTriangle(F1, F3, F4, nn);
			drawTriangle(F1, F4, F5, nn);
			drawTriangle(F1, F5, F6, nn);

			matrixStack.push_back(EulerXYZToMatrix4Degree({ 0, 90, 0 }));
			drawLine(F1, F2);
			drawLine(F2, F3);
			drawLine(F3, F4);
			drawLine(F4, F5);
			drawLine(F5, F6);
			drawLine(F6, F1);
			drawTriangle(F1, F2, F3, nn);
			drawTriangle(F1, F3, F4, nn);
			drawTriangle(F1, F4, F5, nn);
			drawTriangle(F1, F5, F6, nn);
			matrixStack.pop_back();
			matrixStack.push_back(EulerXYZToMatrix4Degree({ 0, 180, 0 }));
			drawLine(F1, F2);
			drawLine(F2, F3);
			drawLine(F3, F4);
			drawLine(F4, F5);
			drawLine(F5, F6);
			drawLine(F6, F1);
			drawTriangle(F1, F2, F3, nn);
			drawTriangle(F1, F3, F4, nn);
			drawTriangle(F1, F4, F5, nn);
			drawTriangle(F1, F5, F6, nn);
			matrixStack.pop_back();
			matrixStack.push_back(EulerXYZToMatrix4Degree({ 0, 270, 0 }));
			drawLine(F1, F2);
			drawLine(F2, F3);
			drawLine(F3, F4);
			drawLine(F4, F5);
			drawLine(F5, F6);
			drawLine(F6, F1);
			drawTriangle(F1, F2, F3, nn);
			drawTriangle(F1, F3, F4, nn);
			drawTriangle(F1, F4, F5, nn);
			drawTriangle(F1, F5, F6, nn);
			matrixStack.pop_back();


			matrixStack.push_back(EulerXYZToMatrix4Degree({ 90, 0, 0 }));
			drawLine(F1, F2);
			drawLine(F2, F3);
			drawLine(F3, F4);
			drawLine(F4, F5);
			drawLine(F5, F6);
			drawLine(F6, F1);
			drawTriangle(F1, F2, F3, nn);
			drawTriangle(F1, F3, F4, nn);
			drawTriangle(F1, F4, F5, nn);
			drawTriangle(F1, F5, F6, nn);
			matrixStack.pop_back();

			matrixStack.push_back(EulerXYZToMatrix4Degree({ 180, 0, 0 }));
			drawLine(F1, F2);
			drawLine(F2, F3);
			drawLine(F3, F4);
			drawLine(F4, F5);
			drawLine(F5, F6);
			drawLine(F6, F1);
			drawTriangle(F1, F2, F3, nn);
			drawTriangle(F1, F3, F4, nn);
			drawTriangle(F1, F4, F5, nn);
			drawTriangle(F1, F5, F6, nn);
			matrixStack.pop_back();

			matrixStack.push_back(EulerXYZToMatrix4Degree({ 90, -90, 0 }));
			drawLine(F1, F2);
			drawLine(F2, F3);
			drawLine(F3, F4);
			drawLine(F4, F5);
			drawLine(F5, F6);
			drawLine(F6, F1);
			drawTriangle(F1, F2, F3, nn);
			drawTriangle(F1, F3, F4, nn);
			drawTriangle(F1, F4, F5, nn);
			drawTriangle(F1, F5, F6, nn);
			matrixStack.pop_back();

			matrixStack.push_back(EulerXYZToMatrix4Degree({ 90, -180, 0 }));
			drawLine(F1, F2);
			drawLine(F2, F3);
			drawLine(F3, F4);
			drawLine(F4, F5);
			drawLine(F5, F6);
			drawLine(F6, F1);
			drawTriangle(F1, F2, F3, nn);
			drawTriangle(F1, F3, F4, nn);
			drawTriangle(F1, F4, F5, nn);
			drawTriangle(F1, F5, F6, nn);
			matrixStack.pop_back();

		}

		setEnableLit(true);


		setEnableLit(false);
	}

	void Guizmo::drawRayHitScreenPoint()
	{
		// Draw Ray hit point
		colorStack.push_back(Eigen::Vector4<uint8_t>(255, 0, 200, 255));
		matrixStack.push_back(Eigen::Matrix4f::Identity());
		drawPoint(cameraParam.rayOrigin + cameraParam.rayDirection, 8, (Eigen::Vector4<uint8_t>(255, 0, 200, 255)));

		matrixStack.pop_back();
		colorStack.pop_back();
	}
	bool Guizmo::lineEdit(unsigned int id, std::vector<Eigen::Vector3f>& line, Eigen::Vector4<uint8_t> _color)
	{
		if (line.size() < 2)
		{
			drawRayHitScreenPoint();
		}
		if (line.size() > 2)
		{
			return false;
		}

		appId = id;
		bool ret = false;

		Eigen::Vector3f planeOrigin(0, 0, 0);
		Eigen::Vector3f planeNormal = -cameraParam.viewDirectioin;
		unsigned int lineId[2] = { makeId("St"), makeId("En") };
		if (line.size() > 0)
		{
			planeOrigin = line.back();
			Eigen::Vector3f oldPoint = line[0];
			if (gizmoSpherePlaneTranslationBehavior(lineId[0], line[0], pixelsToWorldSize(line[0], 10), planeNormal,
				cameraParam.snapTranslation, &line[0]))
			{

				if (line.size() >= 2)
				{
					ret = true;
					line[1] = line[1] + line[0] - oldPoint;
				}
			}
		}
		if (line.size() < 2)
		{
			if (wasKeyReleased(ActionSelect) && isKeyDown(ActionControl))
			{
				Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
				float t0 = FLT_MAX;
				if (Intersect(ray, Plane(planeNormal, planeOrigin), t0))
				{
					Eigen::Vector3f intersectPoint = ray.m_origin + ray.m_direction * t0;
					line.push_back(intersectPoint);
				}
			}
		}
		for (int i = 0; i < line.size(); i++)
		{

			if (lineId[i] == hotId)
			{
				colorStack.push_back(Color_Gold);
				drawCircle(line[i], planeNormal, pixelsToWorldSize(line[i], 10), 20);
				colorStack.pop_back();
				alphaStack.push_back(0.3);
				colorStack.push_back(Color_Red);
				drawCircleFilled(line[i], planeNormal, pixelsToWorldSize(line[i], 10), 20);
				alphaStack.pop_back();
				colorStack.pop_back();
			}
			drawPoint(line[i], 10, lineId[i] == hotId ? Color_Gold : Color_White);
		}
		if (line.size() >= 2)
		{
			sizeStack.push_back(4.0);
			colorStack.push_back(_color);
			drawArrow(line[0], line[1], -1, 15);
			colorStack.pop_back();
			sizeStack.pop_back();
			ret |= gizmoSpherePlaneTranslationBehavior(lineId[1], line[1], pixelsToWorldSize(line[1], 10), planeNormal, cameraParam.snapTranslation, &line[1]);
		}
		return ret;
	}

	bool Guizmo::scaleEdit(unsigned int id, std::vector<Eigen::Vector3f>& line, Eigen::Vector4<uint8_t> _color, int& index)
	{
		if (line.size() > 3)
		{
			return false;
		}

		appId = id;
		bool ret = false;

		Eigen::Vector3f planeOrigin(0, 0, 0);
		Eigen::Vector3f planeNormal = -cameraParam.viewDirectioin;
		unsigned int lineId[3] = { makeId("Anchor"), makeId("Begin"), makeId("End") };
		if (line.size() > 0)
		{
			planeOrigin = line.back();
			Eigen::Vector3f oldPoint = line[0];
			if (gizmoSpherePlaneTranslationBehavior(lineId[0], line[0], pixelsToWorldSize(line[0], 10), planeNormal,
				cameraParam.snapTranslation, &line[0]))
			{
				ret = true;
				index = 0;
				if (line.size() >= 2)
				{
					line[1] = line[1] + line[0] - oldPoint;
				}
				if (line.size() >= 3)
				{
					line[2] = line[2] + line[0] - oldPoint;
				}
			}
		}
		if (line.size() < 3)
		{
			if (wasKeyReleased(ActionSelect) && isKeyDown(ActionControl))
			{
				Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
				float t0 = FLT_MAX;
				if (Intersect(ray, Plane(planeNormal, planeOrigin), t0))
				{
					Eigen::Vector3f intersectPoint = ray.m_origin + ray.m_direction * t0;
					line.push_back(intersectPoint);
				}
			}
		}
		for (int i = 0; i < line.size(); i++)
		{

			if (lineId[i] == hotId)
			{
				colorStack.push_back(Color_Gold);
				drawCircle(line[i], planeNormal, pixelsToWorldSize(line[i], 10), 20);
				colorStack.pop_back();
				alphaStack.push_back(0.3);
				colorStack.push_back(Color_Red);
				drawCircleFilled(line[i], planeNormal, pixelsToWorldSize(line[i], 10), 20);
				alphaStack.pop_back();
				colorStack.pop_back();
			}
			drawPoint(line[i], 10, lineId[i] == hotId ? Color_Gold : Color_White);
		}
		if (line.size() >= 2)
		{
			drawLine(line[0], line[1], 4, _color);
			Eigen::Vector3f oldPoint = line[1];
			if (gizmoSpherePlaneTranslationBehavior(lineId[1], line[1], pixelsToWorldSize(line[1], 10), planeNormal, cameraParam.snapTranslation, &line[1]))
			{
				ret = true;
				index = 1;
				if (line.size() >= 3)
				{
					line[2] = line[2] + line[1] - oldPoint;
				}
			}
		}
		if (line.size() >= 3)
		{
			sizeStack.push_back(4.0);
			colorStack.push_back(_color);
			drawArrow(line[1], line[2], -1, 15);
			colorStack.pop_back();
			sizeStack.pop_back();
			if (gizmoSpherePlaneTranslationBehavior(lineId[2], line[2], pixelsToWorldSize(line[2], 10), planeNormal, cameraParam.snapTranslation, &line[2]))
			{
				ret = true;
				index = 2;
			}

		}
		return ret;
	}

	bool Guizmo::planeEdit(unsigned int id, Eigen::Vector3f& _origin, Eigen::Vector3f& _normal)
	{
		appId = id;
		bool ret = false;
		Eigen::Vector3f* outNormal = new Eigen::Vector3f(_normal);
		Eigen::Vector3f* outOrigin = new Eigen::Vector3f(_origin);
		Eigen::Vector3f normal = *outNormal;
		Eigen::Vector3f planeOrigin = *outOrigin;

		unsigned int planeArrow = makeId("planeArrow");
		unsigned int planeOriginId = makeId("planeOrigin");
		unsigned int planeOriginCircle = makeId("planeCircle");

		Eigen::Vector3f mouse = cameraParam.rayOrigin + cameraParam.rayDirection;
		float wordSize = pixelsToWorldSize(mouse, 3);
		float worldHeight = pixelsToWorldSize(planeOrigin, 50);
		static float cirleDetectRadius = 5;//
		//pixelsToWorldSize(planeOrigin, 40);


		colorStack.push_back(Color_Red);
		drawCircleFilled(mouse, cameraParam.viewDirectioin, wordSize, 20);
		colorStack.pop_back();



		ret |= gizmoSpherePlaneTranslationBehavior(planeOriginId, planeOrigin, pixelsToWorldSize(planeOrigin, 15), normal, cameraParam.snapTranslation, outOrigin);
		ret |= gizmoCircleAxisTranslationBehavior(planeOriginCircle, planeOrigin, cirleDetectRadius, normal, cameraParam.snapTranslation, outOrigin);
		ret |= gizmoOperateNormalBehavior(planeArrow, planeOrigin, planeOrigin + normal * worldHeight, pixelsToWorldSize(planeOrigin, 3), outNormal);

		planeOrigin = *outOrigin;



		drawArrow(planeArrow, planeOrigin, normal, worldHeight, Color_Green);
		drawPoint(planeOrigin, 15, planeOriginId == hotId ? Color_Gold : Color_White);
		alphaStack.push_back(0.5);
		colorStack.push_back(planeOriginCircle == hotId ? Color_Green : Color_White);
		pushEnableSorting(true);
		drawCircleFilled(planeOrigin, normal, cirleDetectRadius, 40);
		popEnableSorting();
		sizeStack.push_back(3.0);
		colorStack.push_back(Color_Green);

		drawCircle(planeOrigin, normal, cirleDetectRadius, 40);

		sizeStack.pop_back();
		alphaStack.pop_back();
		colorStack.pop_back();
		colorStack.pop_back();
		Eigen::Vector3f boxMin = Eigen::Vector3f(std::min(-5.0f, planeOrigin.x()), std::min(-5.0f, planeOrigin.y()), std::min(-5.0f, planeOrigin.z()));
		Eigen::Vector3f boxMax = Eigen::Vector3f(std::max(5.0f, planeOrigin.x()), std::max(5.0f, planeOrigin.y()), std::max(5.0f, planeOrigin.z()));

		sizeStack.push_back(3.0);
		drawAlignedBox(boxMin, boxMax);
		sizeStack.pop_back();
		std::vector<Eigen::Vector3f> edges(std::move(clipBox(Plane(normal, planeOrigin), boxMin, boxMax)));
		for (int i = 0; i < edges.size() / 2; i++)
		{
			drawLine(edges[2 * i], edges[2 * i + 1], 4, Color_White);
		}

		Eigen::Vector3f up = abs(normal.y()) > 0.99 ? Eigen::Vector3f(1, 0, 0) : Eigen::Vector3f(0, 1, 0);
		Eigen::Vector3f xaxis = normal.cross(up).normalized();
		Eigen::Vector3f zaxis = xaxis.cross(normal).normalized();
		unsigned int pointId[4]{ makeId("p1"), makeId("p2"), makeId("p3"), makeId("p4") };
		Eigen::Vector3f pointPos[4] = { planeOrigin + xaxis * 1.0 * cirleDetectRadius,planeOrigin - xaxis * 1.0 * cirleDetectRadius, planeOrigin + zaxis * 1.0 * cirleDetectRadius,planeOrigin - zaxis * 1.0 * cirleDetectRadius };
		Eigen::Vector3f pointDir[4] = { xaxis, -xaxis, zaxis, -zaxis };

		for (int i = 0; i < 4; i++)
		{
			drawPoint(pointPos[i], 10, pointId[i] == hotId ? Color_Gold : Color_GB);
			Eigen::Vector3f* outFace = new Eigen::Vector3f(0, 0, 0);
			*outFace = pointPos[i];
			float size = pixelsToWorldSize(pointPos[i], 10);
			if (gizmoSphereAxisTranslationBehavior(pointId[i], pointPos[i], size, pointDir[i], cameraParam.snapTranslation, outFace))
			{

				cirleDetectRadius = (*outFace - planeOrigin).norm();

			}
			delete outFace;
		}

		_normal = *outNormal;
		_origin = *outOrigin;
		delete outNormal;
		delete outOrigin;
		return ret;
	}

	bool Guizmo::planeEdit(unsigned int id, std::vector<Eigen::Vector3f>& threePoints, int& index)
	{
		if (threePoints.size() < 3)
		{
			drawRayHitScreenPoint();
		}
		if (threePoints.size() == 3)
		{

			Eigen::Vector3f ac = threePoints[2] - threePoints[0];
			Eigen::Vector3f ab = threePoints[1] - threePoints[0];
			if (!ac.cross(ab).isApprox(Eigen::Vector3f::Zero()))
			{
				Eigen::Vector3f abXac = ab.cross(ac);
				Eigen::Vector3f abXacXab = abXac.cross(ab);
				Eigen::Vector3f acXabXac = ac.cross(abXac);
				Eigen::Vector3f toCircumsphere =
					(abXacXab * ac.squaredNorm() + acXabXac * ab.squaredNorm()) / (2.0f * abXac.squaredNorm());
				float radius = toCircumsphere.norm();
				Eigen::Vector3f center = threePoints[0] + toCircumsphere;
				colorStack.push_back(Color_Gold);
				drawCircle(center, abXac.normalized(), radius, 40);
				colorStack.pop_back();
				pushEnableSorting(true);
				alphaStack.push_back(0.7);
				colorStack.push_back(Color_GB);
				drawCircleFilled(center, abXac.normalized(), radius, 40);
				alphaStack.pop_back();
				colorStack.pop_back();
				popEnableSorting();
			}

		}
		if (threePoints.size() > 3)
		{
			return false;
		}

		appId = id;
		bool ret = false;

		Eigen::Vector3f planeOrigin(0, 0, 0);
		Eigen::Vector3f planeNormal = -cameraParam.viewDirectioin;
		unsigned int lineId[3] = { makeId("ThreeA"), makeId("ThreeB"), makeId("ThreeC") };

		if (threePoints.size() < 3)
		{
			if (wasKeyReleased(ActionSelect) && isKeyDown(ActionControl))
			{
				Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
				float t0 = FLT_MAX;
				if (Intersect(ray, Plane(planeNormal, planeOrigin), t0))
				{
					Eigen::Vector3f intersectPoint = ray.m_origin + ray.m_direction * t0;
					threePoints.push_back(intersectPoint);
				}
			}
		}
		for (int i = 0; i < threePoints.size(); i++)
		{
			if (gizmoSpherePlaneTranslationBehavior(lineId[i], threePoints[i], pixelsToWorldSize(threePoints[i], 10),
				planeNormal, cameraParam.snapTranslation, &threePoints[i]))
			{
				ret = true;
				index = i;
			}


			if (lineId[i] == hotId)
			{
				colorStack.push_back(Color_Gold);
				drawCircle(threePoints[i], planeNormal, pixelsToWorldSize(threePoints[i], 10), 20);
				colorStack.pop_back();
				alphaStack.push_back(0.3);
				colorStack.push_back(Color_Red);
				drawCircleFilled(threePoints[i], planeNormal, pixelsToWorldSize(threePoints[i], 10), 20);
				alphaStack.pop_back();
				colorStack.pop_back();
			}
			drawPoint(threePoints[i], 10, lineId[i] == hotId ? Color_Gold : Color_White);
		}

		return ret;
	}

	bool Guizmo::axisRotateEdit(unsigned int id, std::vector<Eigen::Vector3f>& line, float& angle)
	{
		// appId = id;  
		// pushEnableSorting(true);
		if (line.size() < 2)
		{
			drawRayHitScreenPoint();
		}
		bool ret = false;
		if (line.size() >= 2)
		{
			unsigned int yAxiasId = makeId("yAxias");

			axisRotateDraw(yAxiasId, line[0], (line[1] - line[0]).normalized(), pixelsToWorldSize(line[0], 80), Color_White);
			ret |= gizmoAxislAngleBehavior(yAxiasId, line[0], (line[1] - line[0]).normalized(), cameraParam.snapRotation, pixelsToWorldSize(line[0], 80), pixelsToWorldSize(line[0], gizmoSizePixels), &angle);

		}
		idStack.push_back(id);
		ret |= lineEdit(id, line, Color_ARB);
		idStack.pop_back();
		//popEnableSorting();
		return ret;
	}

	bool Guizmo::translation(unsigned int _id, Eigen::Vector3f& _translation_, bool _local)
	{
		bool ret = false;
		Eigen::Vector3f* outVec3 = new Eigen::Vector3f(_translation_.x(), _translation_.y(), _translation_.z());
		Eigen::Vector3f drawAt = *outVec3;

		float worldHeight = pixelsToWorldSize(drawAt, gizmoHeightPixels);
		pushId(_id);
		appId = _id;
		if (_local)
		{
			Eigen::Matrix4f localMatrix = matrixStack.back();

			matrixStack.push_back(localMatrix);
		}
		float planeSize = worldHeight * (0.5f * 0.5f);
		float planeOffset = worldHeight * 0.5f;
		float worldSize = pixelsToWorldSize(drawAt, gizmoSizePixels);

		struct AxisG
		{
			unsigned int m_id;
			Eigen::Vector3f m_axis;
			Eigen::Vector4<uint8_t> m_color;
		};
		AxisG axes[] = { {makeId("axisX"), Eigen::Vector3f(1.0f, 0.0f, 0.0f), Eigen::Vector4<uint8_t>(255, 0, 0, 255)},
			{makeId("axisY"), Eigen::Vector3f(0.0f, 1.0f, 0.0f), Eigen::Vector4<uint8_t>(255, 0, 255, 0)},
			{makeId("axisZ"), Eigen::Vector3f(0.0f, 0.0f, 1.0f), Eigen::Vector4<uint8_t>(255, 255, 0, 0)} };
		struct PlaneG
		{
			unsigned int m_id;
			Eigen::Vector3f m_origin;
		};

		PlaneG planes[] = { {makeId("planeYZ"), Eigen::Vector3f(0.0f, planeOffset, planeOffset)},
			{makeId("planeXZ"), Eigen::Vector3f(planeOffset, 0.0f, planeOffset)},
			{makeId("planeXY"), Eigen::Vector3f(planeOffset, planeOffset, 0.0f)},
			{makeId("planeV"), Eigen::Vector3f(0.0f, 0.0f, 0.0f)} };

		if (cameraParam.flipGizmoWhenBehind)
		{
			const Eigen::Vector3f viewDir =
				cameraParam.orthProj ? -cameraParam.viewDirectioin : (cameraParam.eye - *outVec3).normalized();

			for (int i = 0; i < 3; ++i)
			{
				const Eigen::Vector3f axis =
					_local ? Eigen::Vector3f(matrixStack.back().block(0, i, 3, 1)) : axes[i].m_axis;
				if (axis.dot(viewDir) < 0.0f)
				{
					axes[i].m_axis = -axes[i].m_axis;
					for (int j = 0; j < 3; ++j)
					{
						planes[j].m_origin[i] = -planes[j].m_origin[i];
					}
				}
			}
		}
		Sphere boundingSphere(*outVec3, worldHeight * 1.5f); // expand the bs to catch the planar subgizmos
		Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
		bool intersects = appHotId == appId || Intersects(ray, boundingSphere);
		// pushEnableSorting(true);

		if (_local)
		{
			// local planes need to be drawn with the pushed matrix for correct orientation
			for (int i = 0; i < 3; ++i)
			{
				const PlaneG& plane = planes[i];
				gizmoPlaneTranslationDraw(plane.m_id, plane.m_origin, axes[i].m_axis, planeSize, Color_Gold);
				axes[i].m_axis = matrixStack.back().block(0, 0, 3, 3) * axes[i].m_axis;
				if (intersects)
				{
					ret |= gizmoPlaneTranslationBehavior(plane.m_id, MatrixMulPoint(matrixStack.back(), plane.m_origin),
						axes[i].m_axis, cameraParam.snapTranslation, planeSize, outVec3);
				}
			}
		}
		else
		{
			matrixStack.push_back(Eigen::Matrix4f::Identity());
			for (int i = 0; i < 3; ++i)
			{
				const PlaneG& plane = planes[i];
				gizmoPlaneTranslationDraw(plane.m_id, drawAt + plane.m_origin, axes[i].m_axis, planeSize, Color_Gold);
				if (intersects)
				{
					ret |= gizmoPlaneTranslationBehavior(plane.m_id, drawAt + plane.m_origin, axes[i].m_axis,
						cameraParam.snapTranslation, planeSize, outVec3);
				}
			}
			matrixStack.pop_back();
		}
		matrixStack.push_back(Eigen::Matrix4f::Identity());

		if (intersects)
		{
			// view plane (store the normal when the gizmo becomes active)
			unsigned int currentId = activeId;
			Eigen::Vector3f storedViewNormal = gizmoStateMat3.col(0);
			Eigen::Vector3f viewNormal;
			if (planes[3].m_id == activeId)
			{
				viewNormal = storedViewNormal;
			}
			else
			{
				viewNormal = cameraParam.viewDirectioin;
			}
			ret |= gizmoPlaneTranslationBehavior(
				planes[3].m_id, drawAt, viewNormal, cameraParam.snapTranslation, worldSize, outVec3);
			if (currentId != activeId)
			{
				// gizmo became active, store the view normal
				storedViewNormal = viewNormal;
				gizmoStateMat3.block(0, 0, 3, 1) = storedViewNormal;
			}

			// highlight axes if the corresponding plane is hot
			if (planes[0].m_id == hotId) // YZ
			{
				axes[1].m_color = axes[2].m_color = Color_Gold;
			}
			else if (planes[1].m_id == hotId) // XZ
			{
				axes[0].m_color = axes[2].m_color = Color_Gold;
			}
			else if (planes[2].m_id == hotId) // XY
			{
				axes[0].m_color = axes[1].m_color = Color_Gold;
			}
			else if (planes[3].m_id == hotId) // view plane
			{
				axes[0].m_color = axes[1].m_color = axes[2].m_color = Color_Gold;
			}
		}

		// draw the view plane handle
		begin(PrimitiveModePoints);
		vertex(drawAt, gizmoSizePixels * 2.0f,
			planes[3].m_id == hotId ? Color_Gold : Eigen::Vector4<uint8_t>(255, 255, 255, 255));
		end();
		// axes
		for (int i = 0; i < 3; ++i)
		{
			AxisG& axis = axes[i];
			gizmoAxisTranslationDraw(axis.m_id, drawAt, axis.m_axis, worldHeight, worldSize, axis.m_color);
			if (intersects)
			{

				ret |= gizmoAxisTranslationBehavior(
					axis.m_id, drawAt, axis.m_axis, cameraParam.snapTranslation, worldHeight, worldSize, outVec3);
			}
		}
		matrixStack.pop_back();
		// popEnableSorting();

		if (_local)
		{
			matrixStack.pop_back();
		}

		popId();
		_translation_ = *outVec3;
		delete outVec3;
		return ret;
	}

	bool Guizmo::gizmoPlaneTranslationBehavior(const Eigen::Vector3f& _origin, const Eigen::Vector3f& _normal,
		float _snap, float _worldSize, Eigen::Vector3f* _out_)
	{
		// To Do;
		return false;
	}

	void Guizmo::gizmoAxisTranslationDraw(unsigned int _id, const Eigen::Vector3f& _origin,
		const Eigen::Vector3f& _axis, float _worldHeight, float _worldSize, Eigen::Vector4<uint8_t> _color)
	{
		Eigen::Vector3f viewDir =
			cameraParam.orthProj ? cameraParam.viewDirectioin : (cameraParam.eye - _origin).normalized();
		;
		float aligned = 1.0f - fabs(_axis.dot(viewDir));
		aligned = Remap(aligned, 0.05f, 0.1f);
		Eigen::Vector4<uint8_t> color = _color;
		if (_id == activeId)
		{
			color = Color_Gold;
			pushEnableSorting(false);
			begin(PrimitiveModeLines);
			vertex(_origin - _axis * 999.0f, gizmoSizePixels * 0.5f, _color);
			vertex(_origin + _axis * 999.0f, gizmoSizePixels * 0.5f, _color);
			end();
			popEnableSorting();
		}
		else if (_id == hotId)
		{
			color = Color_Gold;
			aligned = 1.0f;
		}
		color(0) = color(0) * aligned; // .setA(color.getA() * aligned);
		colorStack.push_back(color);
		sizeStack.push_back(gizmoSizePixels);

		drawArrow(_origin + _axis * (0.2f * _worldHeight), _origin + _axis * _worldHeight);
		sizeStack.pop_back();
		colorStack.pop_back();
	}

	bool Guizmo::gizmoAxisTranslationBehavior(unsigned int _id, const Eigen::Vector3f& _origin,
		const Eigen::Vector3f& _axis, float _snap, float _worldHeight, float _worldSize, Eigen::Vector3f* _out_)
	{

		if (_id != hotId)
		{
			// disable behavior when aligned
			Eigen::Vector3f viewDir =
				cameraParam.orthProj ? cameraParam.viewDirectioin : (cameraParam.eye - _origin).normalized();
			float aligned = 1.0f - fabs(_axis.dot(viewDir));
			if (aligned < 0.01f)
			{
				return false;
			}
		}

		Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
		Line axisLine(_origin, _axis);
		Capsule axisCapsule(_origin + _axis * (0.2f * _worldHeight), _origin + _axis * _worldHeight, _worldSize);

		Eigen::Vector3f& storedPosition = gizmoStateVec3;

		if (_id == activeId)
		{
			if (isKeyDown(ActionSelect))
			{
				float tr, tl;
				Nearest(ray, axisLine, tr, tl);

				*_out_ = Snap(*_out_ + _axis * tl - storedPosition, _snap);

				return true;
			}
			else
			{
				makeActive(IdInvalid);
			}
		}
		else if (_id == hotId)
		{
			if (Intersects(ray, axisCapsule))
			{
				if (isKeyDown(ActionSelect))
				{
					makeActive(_id);
					float tr, tl;
					Nearest(ray, axisLine, tr, tl);
					storedPosition = _axis * tl;
				}
			}
			else
			{
				resetId();
			}
		}
		else
		{
			float t0, t1;
			bool intersects = Intersect(ray, axisCapsule, t0, t1);
			if (makeHot(_id, t0, intersects))
			{
			}
		}
		return false;
	}

	void Guizmo::gizmoPlaneTranslationDraw(unsigned int _id, const Eigen::Vector3f& _origin,
		const Eigen::Vector3f& _normal, float _worldSize, Eigen::Vector4<uint8_t> _color)
	{
		Eigen::Vector3f viewDir =
			cameraParam.orthProj ? cameraParam.viewDirectioin : (cameraParam.eye - _origin).normalized();

		Eigen::Vector3f n =
			matrixStack.back().block(0, 0, 3, 3) * _normal; // _normal may be in local space, need to transform to world
		// space for the dot with viewDir to make sense
		float aligned = fabs(n.dot(viewDir));
		aligned = Remap(aligned, 0.1f, 0.2f);
		Eigen::Vector4<uint8_t> color = _color;
		color(0) = (color(0) * aligned);
		colorStack.push_back(color);
		alphaStack.push_back(_id == hotId ? 0.7f : 0.1f * alphaStack.back());

		drawQuadFilled(_origin, _normal, Eigen::Vector2<float>(_worldSize, _worldSize));
		alphaStack.pop_back();
		drawQuad(_origin, _normal, Eigen::Vector2<float>(_worldSize, _worldSize));
		colorStack.pop_back();
	}

	bool Guizmo::gizmoPlaneTranslationBehavior(unsigned int _id, const Eigen::Vector3f& _origin,
		const Eigen::Vector3f& _normal, float _snap, float _worldSize, Eigen::Vector3f* _out_)
	{
		Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
		Plane plane(_normal, _origin);

		float tr;
		bool intersects = Intersect(ray, plane, tr);
		if (!intersects)
		{
			return false;
		}
		Eigen::Vector3f intersection = ray.m_origin + ray.m_direction * tr;
		Eigen::Vector3f Abs = Eigen::Vector3f(
			abs(intersection.x() - _origin.x()), abs(intersection.y() - _origin.y()), abs(intersection.z() - _origin.z()));
		bool allLessFlag = Abs.x() < _worldSize && Abs.y() < _worldSize && Abs.z() < _worldSize;
		intersects &= allLessFlag;
		Eigen::Vector3f& storedPosition = gizmoStateVec3;

		if (_id == activeId)
		{
			if (isKeyDown(ActionSelect))
			{
				*_out_ = Snap(intersection + storedPosition, plane, _snap);
				return true;
			}
			else
			{
				makeActive(IdInvalid);
			}
		}
		else if (_id == hotId)
		{
			if (intersects)
			{
				if (isKeyDown(ActionSelect))
				{
					makeActive(_id);
					storedPosition = *_out_ - intersection;
				}
			}
			else
			{
				resetId();
			}
		}
		else
		{
			makeHot(_id, tr, intersects);
		}
		return false;
	}

	bool Guizmo::rotation(unsigned int _id, Eigen::Matrix3f& _rotation_, bool _local)
	{
		Eigen::Vector3f origin = matrixStack.back().block(0, 3, 3, 1);
		float worldRadius = pixelsToWorldSize(origin, gizmoHeightPixels);
		unsigned int currentId = activeId; // store currentId to detect if the gizmo becomes active during this call
		pushId(_id);
		appId = _id;

		bool ret = false;
		Eigen::Matrix3f& storedRotation = gizmoStateMat3;
		Eigen::Matrix3f* outMat3 = new Eigen::Matrix3f(_rotation_);
		Eigen::Vector3f euler = ToEulerXYZ(*outMat3);
		float worldSize = pixelsToWorldSize(origin, gizmoSizePixels);
		struct AxisG
		{
			unsigned int m_id;
			Eigen::Vector3f m_axis;
			Eigen::Vector4<uint8_t> m_color;
		};
		AxisG axes[] = { {makeId("axisX"), Eigen::Vector3f(1.0f, 0.0f, 0.0f), Eigen::Vector4<uint8_t>(255, 0, 0, 255)},
			{makeId("axisY"), Eigen::Vector3f(0.0f, 1.0f, 0.0f), Eigen::Vector4<uint8_t>(255, 0, 255, 0)},
			{makeId("axisZ"), Eigen::Vector3f(0.0f, 0.0f, 1.0f), Eigen::Vector4<uint8_t>(255, 255, 0, 0)} };
		unsigned int viewId = makeId("axisV");

		Sphere boundingSphere(origin, worldRadius);
		Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
		bool intersects = appHotId == appId || Intersects(ray, boundingSphere);

		// const AppData& appData = ctx.getAppData();
		if (_local)
		{
			// extract axes from the pushed matrix
			for (int i = 0; i < 3; ++i)
			{
				if (activeId == axes[i].m_id)
				{
					// use the stored matrix where the id is active, avoid rotating the axis frame during interaction (cause
					// numerical instability)
					axes[i].m_axis = storedRotation.col(i).normalized(); // Normalize( Eigen::Vector3f(storedRotation.getCol(i)));
				}
				else
				{
					// matrixStack.back().col(i).normalized();
					axes[i].m_axis = matrixStack.back().block(0, i, 3, 1).normalized();
				}
			}
		}

		matrixStack.push_back(Eigen::Matrix4f::Identity());
		// ctx.pushMatrix(Mat4(1.0f));
		for (int i = 0; i < 3; ++i)
		{
			if (i == 0 && (activeId == axes[1].m_id || activeId == axes[2].m_id || activeId == viewId))
			{
				continue;
			}
			if (i == 1 && (activeId == axes[2].m_id || activeId == axes[0].m_id || activeId == viewId))
			{
				continue;
			}
			if (i == 2 && (activeId == axes[0].m_id || activeId == axes[1].m_id || activeId == viewId))
			{
				continue;
			}

			AxisG& axis = axes[i];
			gizmoAxislAngleDraw(axis.m_id, origin, axis.m_axis, worldRadius * 0.9f, euler[i], axis.m_color, 0.0f);
			if (intersects && gizmoAxislAngleBehavior(axis.m_id, origin, axis.m_axis, cameraParam.snapRotation,
				worldRadius * 0.9f, worldSize, &euler[i]))
			{
				*outMat3 = RotationMatrix(axis.m_axis, euler[i] - gizmoStateFloat) * storedRotation;
				ret = true;
			}
		}
		if (!(activeId == axes[0].m_id || activeId == axes[1].m_id || activeId == axes[2].m_id))
		{
			Eigen::Vector3f viewNormal = cameraParam.viewDirectioin;
			float angle = 0.0f;
			if (intersects && gizmoAxislAngleBehavior(
				viewId, origin, viewNormal, cameraParam.snapRotation, worldRadius, worldSize, &angle))
			{
				*outMat3 = RotationMatrix(viewNormal, angle) * storedRotation;
				ret = true;
			}
			gizmoAxislAngleDraw(viewId, origin, viewNormal, worldRadius, angle,
				viewId == activeId ? Color_Gold : Eigen::Vector4<uint8_t>(255, 255, 255, 255), 1.0f);
		}
		matrixStack.pop_back();

		if (currentId != activeId)
		{
			// gizmo became active, store rotation matrix
			storedRotation = *outMat3;
		}
		idStack.pop_back();
		_rotation_ = *outMat3;

		return ret;
	}

	bool Guizmo::gizmoAxislAngleBehavior(unsigned int _id, const Eigen::Vector3f& _origin, const Eigen::Vector3f& _axis, float _snap, float _worldRadius, float _worldSize, float* _out_)
	{
		Eigen::Vector3f viewDir =
			cameraParam.orthProj ? cameraParam.viewDirectioin : (cameraParam.eye - _origin).normalized();

		float aligned = fabs(_axis.dot(viewDir));
		float tr = 0.0f;
		Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
		bool intersects = false;
		Eigen::Vector3f intersection;
		if (aligned < 0.05f)
		{
			// ray-plane intersection fails at grazing angles, use capsule interesection
			float t1;
			Eigen::Vector3f capsuleAxis = viewDir.cross(_axis);
			Capsule capsule(_origin + capsuleAxis * _worldRadius, _origin - capsuleAxis * _worldRadius, _worldSize * 0.5f);
			intersects = Intersect(ray, capsule, tr, t1);
			intersection = ray.m_origin + ray.m_direction * tr;
		}
		else
		{
			Plane plane(_axis, _origin);
			intersects = Intersect(ray, plane, tr);
			intersection = ray.m_origin + ray.m_direction * tr;
			float dist = (intersection - _origin).norm();
			intersects &= fabs(dist - _worldRadius) < (_worldSize + _worldSize * (1.0f - aligned) * 2.0f);
		}

		Eigen::Vector3f& storedVec = gizmoStateVec3;
		float& storedAngle = gizmoStateFloat;
		bool ret = false;

		// use a view-aligned plane intersection to generate the rotation delta
		Plane viewPlane(viewDir, _origin);
		Intersect(ray, viewPlane, tr);
		intersection = ray.m_origin + ray.m_direction * tr;

		if (_id == activeId)
		{
			if (isKeyDown(ActionSelect))
			{
				Eigen::Vector3f delta = (intersection - _origin).normalized();
				float sign = storedVec.cross(delta).dot(_axis);
				float angle = acosf(Clamp(delta.dot(storedVec), -1.0f, 1.0f));
				*_out_ = Snap(storedAngle + copysignf(angle, sign), _snap);
				return true;
			}
			else
			{
				makeActive(IdInvalid);
			}
		}
		else if (_id == hotId)
		{
			if (intersects)
			{
				if (isKeyDown(ActionSelect))
				{
					makeActive(_id);
					storedVec = (intersection - _origin).normalized();
					storedAngle = Snap(*_out_, cameraParam.snapRotation);
				}
			}
			else
			{
				resetId();
			}
		}
		else
		{
			makeHot(_id, tr, intersects);
		}
		return false;
	}

	void Guizmo::axisRotateDraw(unsigned int _id, const Eigen::Vector3f& _origin, const Eigen::Vector3f& _axis, float _worldRadius, Eigen::Vector4<uint8_t> _color)
	{
		Eigen::Vector3f viewDir = cameraParam.orthProj ? cameraParam.viewDirectioin : (cameraParam.eye - _origin).normalized();
		Eigen::Vector4<uint8_t> color = _color;

		if (_id == activeId)
		{
			color = Color_Green;
			Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
			Plane plane(_axis, _origin);
			float tr;
			if (Intersect(ray, plane, tr))
			{
				Eigen::Vector3f intersection = ray.m_origin + ray.m_direction * tr;
				Eigen::Vector3f delta = (intersection - _origin).normalized();

				colorStack.push_back(Color_Gold);
				sizeStack.push_back(gizmoSizePixels);
				drawArrow(_origin, _origin + delta * _worldRadius);
				sizeStack.pop_back();
				colorStack.pop_back();

				begin(PrimitiveModePoints);
				vertex(_origin, gizmoSizePixels * 2.0f, Color_Gold);
				end();
			}
		}
		else if (_id == hotId)
		{
			color = Color_Green;
		}

		colorStack.push_back(color);
		sizeStack.push_back(gizmoSizePixels);
		matrixStack.push_back(matrixStack.back() * LookAt(_origin, _origin + _axis));
		// pushMatrix(getMatrix() * LookAt(_origin, _origin + _axis, m_appData.m_worldUp));
		begin(PrimitiveModeLineLoop);
		const int detail = estimateLevelOfDetail(_origin, _worldRadius, 32, 128);
		for (int i = 0; i < detail; ++i)
		{
			float rad = TwoPi * ((float)i / (float)detail);
			vertex(Eigen::Vector3f(cosf(rad) * _worldRadius, sinf(rad) * _worldRadius, 0.0f));
		}
		end();
		alphaStack.push_back(0.3);
		drawCircleFilled({ 0,0,0 }, { 0, 0, 1 }, _worldRadius, 20);
		alphaStack.pop_back();
		matrixStack.pop_back();
		sizeStack.pop_back();
		colorStack.pop_back();
	}
	void Guizmo::gizmoAxislAngleDraw(unsigned int _id, const Eigen::Vector3f& _origin, const Eigen::Vector3f& _axis, float _worldRadius, float _angle, Eigen::Vector4<uint8_t> _color, float _minAlpha)
	{
		Eigen::Vector3f viewDir = cameraParam.orthProj ? cameraParam.viewDirectioin : (cameraParam.eye - _origin).normalized();
		float aligned = fabs(_axis.dot(viewDir));

		Eigen::Vector3f& storedVec = gizmoStateVec3;
		Eigen::Vector4<uint8_t> color = _color;

		if (_id == activeId)
		{
			color = Color_Gold;
			Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
			Plane plane(_axis, _origin);
			float tr;
			if (Intersect(ray, plane, tr))
			{
				Eigen::Vector3f intersection = ray.m_origin + ray.m_direction * tr;
				Eigen::Vector3f delta = (intersection - _origin).normalized();

				alphaStack.push_back(std::max(_minAlpha, Remap(aligned, 1.0f, 0.99f)));


				begin(PrimitiveModeLines);
				vertex(_origin - _axis * 999.0f, gizmoSizePixels * 0.5f, _color);
				vertex(_origin + _axis * 999.0f, gizmoSizePixels * 0.5f, _color);
				// vertex(_origin, m_gizmoSizePixels * 0.5f, Color_GizmoHighlight);
				// vertex(_origin + storedVec * _worldRadius, m_gizmoSizePixels * 0.5f, Color_GizmoHighlight);
				end();

				alphaStack.pop_back();
				// popAlpha();
				colorStack.push_back(Color_Gold);
				// pushColor(Color_GizmoHighlight);
				sizeStack.push_back(gizmoSizePixels);
				// pushSize(m_gizmoSizePixels);
				drawArrow(_origin, _origin + delta * _worldRadius);
				sizeStack.pop_back();
				colorStack.pop_back();

				begin(PrimitiveModePoints);
				vertex(_origin, gizmoSizePixels * 2.0f, Color_Gold);
				end();
			}
		}
		else if (_id == hotId)
		{
			color = Color_Gold;
		}
		aligned = std::max(Remap(aligned, 0.9f, 1.0f), 0.1f);
		if (activeId == _id)
		{
			aligned = 1.0f;
		}
		colorStack.push_back(color);
		sizeStack.push_back(gizmoSizePixels);
		matrixStack.push_back(matrixStack.back() * LookAt(_origin, _origin + _axis));
		// pushMatrix(getMatrix() * LookAt(_origin, _origin + _axis, m_appData.m_worldUp));
		begin(PrimitiveModeLineLoop);
		const int detail = estimateLevelOfDetail(_origin, _worldRadius, 32, 128);
		for (int i = 0; i < detail; ++i)
		{
			float rad = TwoPi * ((float)i / (float)detail);
			vertex(Eigen::Vector3f(cosf(rad) * _worldRadius, sinf(rad) * _worldRadius, 0.0f));

			// post-modify the alpha for parts of the ring occluded by the sphere
			VertexData& vd = getCurrentVertexList()->back();
			Eigen::Vector3f v = Eigen::Vector3f(vd.positionSize.x(), vd.positionSize.y(), vd.positionSize.z());
			float d = (_origin - v).normalized().dot(cameraParam.viewDirectioin);
			d = std::max(_minAlpha, std::max(Remap(d, 0.1f, 0.2f), aligned));
			vd.color(0) = vd.color(0) * d;
		}
		end();
		matrixStack.pop_back();
		sizeStack.pop_back();
		colorStack.pop_back();
	}

	int Guizmo::estimateLevelOfDetail(const Eigen::Vector3f& _position, float _worldSize, int _min, int _max)
	{
		if (cameraParam.orthProj)
		{
			return _max;
		}

		float d = (_position - cameraParam.eye).norm();
		float x = Clamp(2.0f * atanf(_worldSize / (2.0f * d)), 0.0f, 1.0f);
		float fmin = (float)_min;
		float fmax = (float)_max;

		return (int)(fmin + (fmax - fmin) * x);
	}

	bool Guizmo::gizmoSpherePlaneTranslationBehavior(unsigned _id, const Eigen::Vector3f& _origin, float _radius, const Eigen::Vector3f& _normal, float _snap, Eigen::Vector3f* _out_)
	{
		Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
		Plane plane(_normal, _origin);
		Sphere sphere(_origin, _radius);
		float tr = FLT_MAX, t0 = FLT_MAX, t1 = FLT_MAX;
		bool intersects = Intersect(ray, plane, tr) && Intersect(ray, sphere, t0, t1);
		Eigen::Vector3f intersection = ray.m_origin + ray.m_direction * tr;
		Eigen::Vector3f& storedPosition = gizmoStateVec3;
		if (_id == activeId)
		{
			if (isKeyDown(ActionSelect))
			{
				*_out_ = Snap(intersection + storedPosition, plane, _snap);
				return true;
			}
			else
			{
				makeActive(IdInvalid);
			}
		}
		else if (_id == hotId)
		{
			if (intersects)
			{
				if (isKeyDown(ActionSelect))
				{
					makeActive(_id);
					storedPosition = *_out_ - intersection;
				}
			}
			else
			{
				resetId();
			}
		}
		else
		{
			makeHot(_id, t0, intersects);
		}
		return false;
	}

	bool Guizmo::gizmoCircleAxisTranslationBehavior(unsigned int _id, const Eigen::Vector3f& _origin, float _radius,
		const Eigen::Vector3f& _normal, float _snap, Eigen::Vector3f* _out_)
	{
		Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
		Plane plane(_normal, _origin);
		Line axisLine(_origin, _normal);

		float tr;
		bool intersects = Intersect(ray, plane, tr);

		Eigen::Vector3f intersection = ray.m_origin + ray.m_direction * tr;

		if ((intersection - _origin).norm() > _radius)
		{
			intersects = false;
		}
		Eigen::Vector3f& storedPosition = gizmoStateVec3;

		if (_id == activeId)
		{
			if (isKeyDown(ActionSelect))
			{
				float tr, tl;
				Nearest(ray, axisLine, tr, tl);
				*_out_ = Snap(*_out_ + _normal * tl - storedPosition, _snap);
				return true;
			}
			else
			{
				makeActive(IdInvalid);
			}
		}
		else if (_id == hotId)
		{
			if (intersects)
			{

				updateDepth(tr);
				if (isKeyDown(ActionSelect))
				{
					makeActive(_id);
					float tr, t1;
					Nearest(ray, axisLine, tr, t1);
					storedPosition = _normal * t1;
				}
			}
			else
			{
				resetId();
			}
		}
		else
		{
			makeHot(_id, tr, intersects);
		}

		return false;
	}

	bool Guizmo::gizmoSphereAxisTranslationBehavior(unsigned int _id, const Eigen::Vector3f& _origin, float _radius,
		const Eigen::Vector3f& _normal, float _snap, Eigen::Vector3f* _out_)
	{
		Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
		Sphere sphere(_origin, _radius);
		Line axisLine(_origin, _normal);

		float tr, t1;
		bool intersects = Intersect(ray, sphere, tr, t1);
		Eigen::Vector3f intersection = ray.m_origin + ray.m_direction * tr;
		Eigen::Vector3f& storedPosition = gizmoStateVec3;

		if (_id == activeId)
		{
			if (isKeyDown(ActionSelect))
			{
				float tr, tl;
				Nearest(ray, axisLine, tr, tl);
				*_out_ = Snap(*_out_ + _normal * tl - storedPosition, _snap);
				return true;
			}
			else
			{
				makeActive(IdInvalid);
			}
		}
		else if (_id == hotId)
		{
			if (intersects)
			{
				if (isKeyDown(ActionSelect))
				{
					makeActive(_id);
					float tr, t1;
					Nearest(ray, axisLine, tr, t1);
					storedPosition = _normal * t1;
				}
			}
			else
			{
				resetId();
			}
		}
		else
		{
			makeHot(_id, tr, intersects);
		}

		return false;
	}

	bool Guizmo::gizmoOperateNormalBehavior(unsigned int _id, const Eigen::Vector3f& _origin, const Eigen::Vector3f& _end,
		float _worldSize, Eigen::Vector3f* _out_)
	{
		bool ret = false;
		// Operate normal
		Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);

		Capsule axisCapsule(_origin, _end, _worldSize);

		float t0 = FLT_MAX, t1 = FLT_MAX;
		bool intersects = Intersect(ray, axisCapsule, t0, t1); //

		Eigen::Vector2f& storeCursor = gizmoCursor;
		Eigen::Vector3f& storeNormal = gizmoNormal;

		if (_id == activeId)
		{
			if (isKeyDown(ActionSelect))
			{
				Eigen::Vector2<float> offset = 0.5 * (cameraParam.cursor - storeCursor) / 180.0 * Pi;
				offset.y() *= -1;
				Eigen::Matrix3f view = cameraParam.view.block(0, 0, 3, 3);
				Eigen::Vector3f axis = (view.row(1) * offset.x() - view.row(0) * offset.y()).normalized();
				Eigen::Matrix3f mat = RotationMatrix(axis, offset.norm());
				*_out_ = (mat * storeNormal).normalized();
				ret = true;
			}
			else
			{
				makeActive(IdInvalid);
			}
		}
		else if (_id == hotId)
		{
			if (intersects)
			{
				if (isKeyDown(ActionSelect))
				{

					storeCursor = cameraParam.cursor;
					storeNormal = (_end - _origin).normalized();
					makeActive(_id);
				}
			}
			else
			{
				resetId();
			}
		}
		else
		{

			if (intersects)
			{
				makeHot(_id, t0, intersects);
			}
		}
		return ret;
	}

	bool Guizmo::planeClip(unsigned int _id, Eigen::Vector3f& _translation_, Eigen::Vector3f& _normal, float& _scale,
		const Eigen::Vector3f& minBox, const Eigen::Vector3f& maxBox, bool opeartorNormal, bool drawCircleFlag)
	{
		bool ret = false;
		Eigen::Vector3f* outVec3 = new Eigen::Vector3f(0, 0, 0);
		*outVec3 = _translation_;
		Eigen::Vector3f* outNormal = new Eigen::Vector3f(_normal);
		Eigen::Vector3f drawAt = *outVec3;
		Eigen::Vector3f normal = *outNormal;
		// idStack.push_back(_id);
		appId = _id;
		unsigned int sphereId = makeId("Sphere");
		unsigned int circleId = makeId("Circle");
		unsigned int arrowId = makeId("Arrow");


		Eigen::Vector3f up = abs(normal.y()) > 0.99 ? Eigen::Vector3f(1, 0, 0) : Eigen::Vector3f(0, 1, 0);
		Eigen::Vector3f xaxis = normal.cross(up).normalized();
		Eigen::Vector3f zaxis = xaxis.cross(normal).normalized();
		unsigned int pointId[4]{ makeId("p1"), makeId("p2"), makeId("p3"), makeId("p4") };
		Eigen::Vector3f pointPos[4] = {
			drawAt + xaxis * 1.0 * _scale, drawAt - xaxis * 1.0 * _scale,
			drawAt + zaxis * 1.0 * _scale, drawAt - zaxis * 1.0 * _scale };
		Eigen::Vector3f pointDir[4] = { xaxis,-xaxis,zaxis,-zaxis };
		float worldSize = pixelsToWorldSize(drawAt, 3.0);

		// 0.1 / scale;
		// draw spheres
		//drawOneMesh(drawAt, { 0.03f * _scale, 0.03f * _scale, 0.03f * _scale },
		//    sphereId == hotId ? Eigen::Vector3f(0, 1, 0) : Eigen::Vector3f(1, 0, 0), "Sphere");

		auto rotation = RotationMatrixX(normal.normalized());
		Eigen::Vector3f scale = { 0.025f * _scale, 0.025f * _scale, 0.025f * _scale };
		Eigen::Vector3f translate1 = drawAt + normal * _scale;
		Eigen::Vector3f translate2 = drawAt - normal * _scale;
		//drawOneMesh(translate1, rotation, scale, arrowId == hotId ? Eigen::Vector3f(0, 1, 0) : Eigen::Vector3f(1, 0, 0),
		//    "ClipCone");
		//drawOneMesh(translate2, rotation, scale, arrowId == hotId ? Eigen::Vector3f(0, 1, 0) : Eigen::Vector3f(1, 0, 0),
		//    "ClipCone");

		ret |= gizmoSpherePlaneTranslationBehavior(sphereId, drawAt, 0.05 * _scale,
			normal, cameraParam.snapTranslation, outVec3);
		ret |= gizmoCircleAxisTranslationBehavior(
			circleId, drawAt, 1.0 * _scale, normal, cameraParam.snapTranslation, outVec3);

		// Operate normal
		/***/
		if (opeartorNormal)
		{
			Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);

			Capsule axisCapsule(drawAt - normal * _scale, drawAt + normal * _scale, worldSize);

			float t0 = FLT_MAX, t1 = FLT_MAX, t2 = FLT_MAX, t3 = FLT_MAX;
			bool intersects = Intersect(ray, axisCapsule, t0, t1); //
			intersects |= IntersectWithCone(ray, drawAt + normal * _scale, normal, 0.173 * _scale, 0.1 * _scale, t2);
			intersects |= IntersectWithCone(ray, drawAt + normal * (-1.173 * _scale), normal, 0.173 * _scale, 0.1 * _scale, t3);
			Eigen::Vector2<float>& storeCursor = gizmoCursor;
			Eigen::Vector3f& storeNormal = gizmoNormal;

			if (arrowId == activeId)
			{
				if (isKeyDown(ActionSelect))
				{

					Eigen::Vector2<float> offset = 0.5 * (cameraParam.cursor - storeCursor) / 180.0 * Pi;
					offset.y() *= -1;
					Eigen::Matrix3f view = cameraParam.view.block(0, 0, 3, 3);
					Eigen::Vector3f axis = (view.row(1) * offset.x() - view.row(0) * offset.y()).normalized();
					Eigen::Matrix3f mat = RotationMatrix(axis, offset.norm());
					*outNormal = (mat * storeNormal).normalized();
					ret = true;
				}
				else
				{
					makeActive(IdInvalid);
				}
			}
			else if (arrowId == hotId)
			{
				if (intersects)
				{
					if (isKeyDown(ActionSelect))
					{

						storeCursor = cameraParam.cursor;
						storeNormal = normal.normalized();
						makeActive(arrowId);
					}
				}
				else
				{
					resetId();
				}
			}
			else
			{
				float tr = std::fmin(t0, t1);
				tr = std::fmin(tr, t2);
				tr = std::fmin(tr, t3);
				if (intersects)
				{
					makeHot(arrowId, tr, intersects);
				}
			}
		}



		/***/
		if (arrowId == hotId || circleId == hotId)
		{
			colorStack.push_back(Eigen::Vector4<uint8_t>(255, 0, 255, 0));
		}
		else
		{
			colorStack.push_back(Eigen::Vector4<uint8_t>(255, 0, 0, 255));
		}

		sizeStack.push_back(4.0);
		pushEnableSorting(true);
		drawLine(drawAt - normal * _scale, drawAt + normal * _scale, 2.5);
		drawCircle(drawAt, normal, 1.0 * _scale, 40);

		popEnableSorting();

		for (int i = 0; i < 4; i++)
		{
			drawPoint(pointPos[i], 8, pointId[i] == hotId ? Color_Gold : Color_GB);
			Eigen::Vector3f* outFace = new Eigen::Vector3f(0, 0, 0);
			*outFace = pointPos[i];
			float size = pixelsToWorldSize(pointPos[i], 15);
			if (gizmoSphereAxisTranslationBehavior(pointId[i], pointPos[i], size, pointDir[i], cameraParam.snapTranslation, outFace))
			{

				_scale = (*outFace - drawAt).norm();

			}
			delete outFace;
		}
		pushEnableSorting(true);
		sizeStack.pop_back();
		colorStack.pop_back();
		sizeStack.push_back(1.50);
		Eigen::Vector3f lb = Eigen::Vector3f(std::min(_translation_.x(), minBox.x()), std::min(_translation_.y(), minBox.y()), std::min(_translation_.z(), minBox.z()));
		Eigen::Vector3f rt = Eigen::Vector3f(std::max(_translation_.x(), maxBox.x()), std::max(_translation_.y(), maxBox.y()), std::max(_translation_.z(), maxBox.z()));
		drawAlignedBox(lb, rt);
		sizeStack.pop_back();
		std::vector<Eigen::Vector3f> edges(std::move(clipBox(Plane(normal, drawAt), lb, rt)));

		for (int i = 0; i < edges.size() / 2; i++)
		{

			drawLine(edges[2 * i], edges[2 * i + 1], 2.0, circleId == hotId ? Color_Green : Color_White);
		}

		alphaStack.push_back(0.3);
		if (circleId == hotId)
		{
			colorStack.push_back(Eigen::Vector4<uint8_t>(255, 0, 255, 0));
		}
		else
		{
			colorStack.push_back(Eigen::Vector4<uint8_t>(255, 255, 255, 255));
		}
		if (drawCircleFlag)
		{
			drawCircleFilled(drawAt, normal, 1.0 * _scale, 40);
		}


		colorStack.pop_back();
		alphaStack.pop_back();
		popEnableSorting();
		// idStack.pop_back();
		_normal = *outNormal;
		_translation_ = *outVec3;
		delete outNormal;
		delete outVec3;
		return ret;
	}

	bool Guizmo::boxEdit(unsigned int _id, Eigen::Vector3f& _translation_, Eigen::Matrix3f& _rotation_, Eigen::Vector3f& _scale_)
	{
		bool ret = false;
		Eigen::Vector3f* outVec3 = new Eigen::Vector3f(0, 0, 0);
		*outVec3 = _translation_;
		Eigen::Vector3f drawAt = *outVec3;
		Eigen::Matrix3f* outMat3 = new Eigen::Matrix3f(_rotation_);
		Eigen::Matrix3f drawMat = *outMat3;
		Eigen::Vector3f* outScale = new Eigen::Vector3f(_scale_);
		Eigen::Vector3f drawScale = *outScale;

		Eigen::Vector3f xAxis((*outMat3)(0, 0), (*outMat3)(1, 0), (*outMat3)(2, 0));
		Eigen::Vector3f yAxis((*outMat3)(0, 1), (*outMat3)(1, 1), (*outMat3)(2, 1));
		Eigen::Vector3f zAxis((*outMat3)(0, 2), (*outMat3)(1, 2), (*outMat3)(2, 2));
		float worldHeight = pixelsToWorldSize(drawAt, 10);
		idStack.push_back(_id);
		appId = _id;
		unsigned int boxCenter = makeId("BoxCenter");
		unsigned int boundingBox = makeId("BoundingBox");
		bool canRotateBox = true;
		Ray camRay(cameraParam.rayOrigin, cameraParam.rayDirection);
		if (boundingBox != activeId && Intersects(camRay, Sphere(drawAt, pixelsToWorldSize(drawAt, 15))))
		{
			canRotateBox = false;
		}
		Eigen::Vector3f viewNormal = cameraParam.viewDirectioin;
		if (gizmoPlaneTranslationBehavior(boxCenter, drawAt, viewNormal, cameraParam.snapTranslation, worldHeight, outVec3))
		{
			ret = true;
		}
		struct FaceCenter
		{
			unsigned int centerId;
			Eigen::Vector3f pos;
			Eigen::Vector3f nor;
			float size;
			float* outS;
			FaceCenter(const unsigned int& c, const Eigen::Vector3f& v, const Eigen::Vector3f& n, float* s)
				: centerId(c)
				, pos(v)
				, nor(n)
				, outS(s)
			{
			}
		};
		FaceCenter faces[6] = {
			{makeId("RightFaceCenter"), Eigen::Vector3f(drawAt + xAxis * drawScale.x()), xAxis, &(outScale->x())},
			{makeId("LeftFaceCenter"), Eigen::Vector3f(drawAt - xAxis * drawScale.x()), xAxis, &(outScale->x())},
			{makeId("TopFaceCenter"), Eigen::Vector3f(drawAt + yAxis * drawScale.y()), yAxis, &(outScale->y())},
			{makeId("BottomFaceCenter"), Eigen::Vector3f(drawAt - yAxis * drawScale.y()), yAxis, &(outScale->y())},
			{makeId("FrontCenter"), Eigen::Vector3f(drawAt + zAxis * drawScale.z()), zAxis, &(outScale->z())},
			{makeId("BackFaceCenter"), Eigen::Vector3f(drawAt - zAxis * drawScale.z()), zAxis, &(outScale->z())},
		};

		for (int i = 0; i < 6; i++)
		{
			Eigen::Vector3f* outFace = new Eigen::Vector3f(0, 0, 0);
			*outFace = faces[i].pos;
			float size = pixelsToWorldSize(faces[i].pos, 10);
			if (gizmoSphereAxisTranslationBehavior(faces[i].centerId, faces[i].pos, size, faces[i].nor, cameraParam.snapTranslation, outFace))
			{
				ret = true;
				*outVec3 = *outVec3 + (*outFace - faces[i].pos) / 2;
				*faces[i].outS = (*outFace - *outVec3).norm();
			}
			if (boundingBox != activeId && Intersects(camRay, Sphere(faces[i].pos, size)))
			{
				canRotateBox = false;
			}
			delete outFace;
		}

		if (canRotateBox)
		{
			Eigen::Matrix4f mat = Coord3(drawAt, drawMat, drawScale);
			Eigen::Matrix4f matinverse = mat.inverse();
			Eigen::Vector3f o = MatrixMulPoint(matinverse, cameraParam.rayOrigin);
			Eigen::Vector4<float> temp = matinverse * Eigen::Vector4<float>(cameraParam.rayDirection.x(), cameraParam.rayDirection.y(), cameraParam.rayDirection.z(), 0.0);
			Eigen::Vector3f d = temp.head<3>().normalized();
			Ray ray(o, d);
			float tr = FLT_MAX;
			bool hit = IntersectBox(ray, Eigen::Vector3f(-1, -1, -1), Eigen::Vector3f(1, 1, 1), tr);
			if (hit)
			{

				tr = (MatrixMulPoint(mat, ray.m_origin + ray.m_direction * tr) - cameraParam.rayOrigin).norm();
			}

			Eigen::Vector2<float>& storeCursor = gizmoCursor;
			Eigen::Matrix3f& storeMat = gizmoStateMat3;

			if (boundingBox == activeId)
			{
				if (isKeyDown(ActionSelect))
				{
					Eigen::Vector2<float> offset;
					offset = 0.5 * (cameraParam.cursor - storeCursor) / 180.0 * Pi;
					offset.y() *= -1;

					Eigen::Matrix3f view = cameraParam.view.block(0, 0, 3, 3);
					Eigen::Vector3f axis = (view.row(1) * offset.x() - view.row(0) * offset.y()).normalized();

					Eigen::Vector2<float> debug = offset.normalized();

					Eigen::Vector3f s = drawAt + Eigen::Vector3f(100 * (view.row(0) * debug.y() - view.row(1) * debug.x()));
					Eigen::Vector3f e = drawAt + Eigen::Vector3f(100 * (-view.row(0) * debug.y() + view.row(1) * debug.x()));
					drawLine(s, e, 3, Color_Gold);
					*outMat3 = RotationMatrix(axis, offset.norm()) * storeMat;
					ret = true;
				}
				else
				{
					makeActive(IdInvalid);
				}
			}
			else if (boundingBox == hotId)
			{
				if (hit)
				{
					if (isKeyDown(ActionSelect))
					{
						storeCursor = cameraParam.cursor;
						storeMat = *outMat3;
						makeActive(boundingBox);
					}
					updateDepth(tr);
				}
				else
				{
					resetId();
				}
			}
			else
			{
				makeHot(boundingBox, tr, hit);
			}
		}
		else if (boundingBox == hotId)
		{
			resetId();
		}
		_translation_ = *outVec3;
		_rotation_ = *outMat3;
		_scale_ = *outScale;
		delete outVec3;
		delete outMat3;
		delete outScale;
		matrixStack.push_back(Coord3(_translation_, _rotation_, _scale_));

		struct Quad
		{
			Eigen::Vector3f a;
			Eigen::Vector3f b;
			Eigen::Vector3f c;
			Eigen::Vector3f d;
		};

		Eigen::Vector3f points[6] = { Eigen::Vector3f(1, 0, 0), Eigen::Vector3f(-1, 0, 0),
			Eigen::Vector3f(0, 1, 0), Eigen::Vector3f(0, -1, 0), Eigen::Vector3f(0, 0, 1),
			Eigen::Vector3f(0, 0, -1) };
		Quad quads[6] = { {Eigen::Vector3f(1, -1, -1), Eigen::Vector3f(1, 1, -1), Eigen::Vector3f(1, 1, 1),
							 Eigen::Vector3f(1, -1, 1)},
			{Eigen::Vector3f(-1, -1, -1), Eigen::Vector3f(-1, 1, -1), Eigen::Vector3f(-1, 1, 1),
				Eigen::Vector3f(-1, -1, 1)},
			{Eigen::Vector3f(-1, 1, -1), Eigen::Vector3f(1, 1, -1), Eigen::Vector3f(1, 1, 1),
				Eigen::Vector3f(-1, 1, 1)},
			{Eigen::Vector3f(-1, -1, -1), Eigen::Vector3f(1, -1, -1), Eigen::Vector3f(1, -1, 1),
				Eigen::Vector3f(-1, -1, 1)},
			{Eigen::Vector3f(-1, -1, 1), Eigen::Vector3f(1, -1, 1), Eigen::Vector3f(1, 1, 1),
				Eigen::Vector3f(-1, 1, 1)},
			{Eigen::Vector3f(-1, -1, -1), Eigen::Vector3f(1, -1, -1), Eigen::Vector3f(1, 1, -1),
				Eigen::Vector3f(-1, 1, -1)}

		};

		for (int i = 0; i < 6; i++)
		{
			drawPoint(points[i], 7, faces[i].centerId == hotId ? Color_Gold : Color_Green);
			if (faces[i].centerId == hotId)
			{
				alphaStack.push_back(0.3);
				colorStack.push_back(Color_Brown);
				drawQuadFilled(quads[i].a, quads[i].b, quads[i].c, quads[i].d);
				alphaStack.pop_back();
				colorStack.pop_back();
			}
		}

		drawPoint(Eigen::Vector3f(0, 0, 0), 7, boxCenter == hotId ? Color_Gold : Color_Green);
		drawLine(Eigen::Vector3f(-1, 0, 0), Eigen::Vector3f(1, 0, 0), 2, Color_White);
		drawLine(Eigen::Vector3f(0, -1, 0), Eigen::Vector3f(0, 1, 0), 2, Color_White);
		drawLine(Eigen::Vector3f(0, 0, -1), Eigen::Vector3f(0, 0, 1), 2, Color_White);
		if (boundingBox == activeId)
		{
			colorStack.push_back(Eigen::Vector4<uint8_t>(255, 82, 111, 108));

			alphaStack.push_back(0.4);
			drawAlignedBoxFilled(Eigen::Vector3f(-1, -1, -1), Eigen::Vector3f(1, 1, 1));
			alphaStack.pop_back();
			colorStack.pop_back();
		}

		sizeStack.push_back(4.0);

		colorStack.push_back((boxCenter == activeId || boundingBox == activeId) ? Color_Green : Color_White);
		drawAlignedBox(Eigen::Vector3f(-1, -1, -1), Eigen::Vector3f(1, 1, 1));
		colorStack.pop_back();
		sizeStack.pop_back();
		matrixStack.pop_back();
		popId();

		return ret;
	}

	bool Guizmo::boxEdit(unsigned int _id, Eigen::Vector3f& _translation_, Eigen::Matrix3f& _rotation_,
		Eigen::Vector3f& _scale_, bool translationFlag, bool rotateFlag, bool faceMoveFlag, int& mode)
	{
		bool ret = false;
		Eigen::Vector3f* outVec3 = new Eigen::Vector3f(0, 0, 0);
		*outVec3 = _translation_;
		Eigen::Vector3f drawAt = *outVec3;
		Eigen::Matrix3f* outMat3 = new Eigen::Matrix3f(_rotation_);
		Eigen::Matrix3f drawMat = *outMat3;
		Eigen::Vector3f* outScale = new Eigen::Vector3f(_scale_);
		Eigen::Vector3f drawScale = *outScale;

		Eigen::Vector3f xAxis((*outMat3)(0, 0), (*outMat3)(1, 0), (*outMat3)(2, 0));
		Eigen::Vector3f yAxis((*outMat3)(0, 1), (*outMat3)(1, 1), (*outMat3)(2, 1));
		Eigen::Vector3f zAxis((*outMat3)(0, 2), (*outMat3)(1, 2), (*outMat3)(2, 2));
		float worldHeight = pixelsToWorldSize(drawAt, 10);
		idStack.push_back(_id);
		appId = _id;
		unsigned int boxCenter = makeId("BoxCenter");
		unsigned int boundingBox = makeId("BoundingBox");
		bool canRotateBox = true;
		Ray camRay(cameraParam.rayOrigin, cameraParam.rayDirection);
		if (boundingBox != activeId && Intersects(camRay, Sphere(drawAt, pixelsToWorldSize(drawAt, 15))))
		{
			canRotateBox = false;
		}
		Eigen::Vector3f viewNormal = cameraParam.viewDirectioin;
		if (translationFlag)
		{
			if (gizmoPlaneTranslationBehavior(
				boxCenter, drawAt, viewNormal, cameraParam.snapTranslation, worldHeight, outVec3))
			{
				ret = true;
				mode = 0;
			}
		}

		struct FaceCenter
		{
			unsigned int centerId;
			Eigen::Vector3f pos;
			Eigen::Vector3f nor;
			float size;
			float* outS;
			FaceCenter(const unsigned int& c, const Eigen::Vector3f& v, const Eigen::Vector3f& n, float* s)
				: centerId(c)
				, pos(v)
				, nor(n)
				, outS(s)
			{
			}
		};
		FaceCenter faces[6] = {
			{makeId("RightFaceCenter"), Eigen::Vector3f(drawAt + xAxis * drawScale.x()), xAxis, &(outScale->x())},
			{makeId("LeftFaceCenter"), Eigen::Vector3f(drawAt - xAxis * drawScale.x()), xAxis, &(outScale->x())},
			{makeId("TopFaceCenter"), Eigen::Vector3f(drawAt + yAxis * drawScale.y()), yAxis, &(outScale->y())},
			{makeId("BottomFaceCenter"), Eigen::Vector3f(drawAt - yAxis * drawScale.y()), yAxis, &(outScale->y())},
			{makeId("FrontCenter"), Eigen::Vector3f(drawAt + zAxis * drawScale.z()), zAxis, &(outScale->z())},
			{makeId("BackFaceCenter"), Eigen::Vector3f(drawAt - zAxis * drawScale.z()), zAxis, &(outScale->z())},
		};
		if (faceMoveFlag)
		{
			for (int i = 0; i < 6; i++)
			{
				Eigen::Vector3f* outFace = new Eigen::Vector3f(0, 0, 0);
				*outFace = faces[i].pos;
				float size = pixelsToWorldSize(faces[i].pos, 10);
				if (gizmoSphereAxisTranslationBehavior(
					faces[i].centerId, faces[i].pos, size, faces[i].nor, cameraParam.snapTranslation, outFace))
				{
					ret = true;
					*outVec3 = *outVec3 + (*outFace - faces[i].pos) / 2;
					*faces[i].outS = (*outFace - *outVec3).norm();
					mode = 1;
				}
				if (boundingBox != activeId && Intersects(camRay, Sphere(faces[i].pos, size)))
				{
					canRotateBox = false;
				}
				delete outFace;
			}

		}


		if (canRotateBox && rotateFlag)
		{
			Eigen::Matrix4f mat = Coord3(drawAt, drawMat, drawScale);
			Eigen::Matrix4f matinverse = mat.inverse();
			Eigen::Vector3f o = MatrixMulPoint(matinverse, cameraParam.rayOrigin);
			Eigen::Vector4<float> temp = matinverse * Eigen::Vector4<float>(cameraParam.rayDirection.x(),
				cameraParam.rayDirection.y(), cameraParam.rayDirection.z(), 0.0);
			Eigen::Vector3f d = temp.head<3>().normalized();
			Ray ray(o, d);
			float tr = FLT_MAX;
			bool hit = IntersectBox(ray, Eigen::Vector3f(-1, -1, -1), Eigen::Vector3f(1, 1, 1), tr);
			if (hit)
			{

				tr = (MatrixMulPoint(mat, ray.m_origin + ray.m_direction * tr) - cameraParam.rayOrigin).norm();
			}

			Eigen::Vector2<float>& storeCursor = gizmoCursor;
			Eigen::Matrix3f& storeMat = gizmoStateMat3;

			if (boundingBox == activeId)
			{
				if (isKeyDown(ActionSelect))
				{
					Eigen::Vector2<float> offset;
					offset = 0.5 * (cameraParam.cursor - storeCursor) / 180.0 * Pi;
					offset.y() *= -1;

					Eigen::Matrix3f view = cameraParam.view.block(0, 0, 3, 3);
					Eigen::Vector3f axis = (view.row(1) * offset.x() - view.row(0) * offset.y()).normalized();

					Eigen::Vector2<float> debug = offset.normalized();

					Eigen::Vector3f s = drawAt + Eigen::Vector3f(100 * (view.row(0) * debug.y() - view.row(1) * debug.x()));
					Eigen::Vector3f e =
						drawAt + Eigen::Vector3f(100 * (-view.row(0) * debug.y() + view.row(1) * debug.x()));
					drawLine(s, e, 3, Color_Gold);
					*outMat3 = RotationMatrix(axis, offset.norm()) * storeMat;
					ret = true;
					mode = 2;
				}
				else
				{
					makeActive(IdInvalid);
				}
			}
			else if (boundingBox == hotId)
			{
				if (hit)
				{
					if (isKeyDown(ActionSelect))
					{
						storeCursor = cameraParam.cursor;
						storeMat = *outMat3;
						makeActive(boundingBox);
					}
					updateDepth(tr);
				}
				else
				{
					resetId();
				}
			}
			else
			{
				makeHot(boundingBox, tr, hit);
			}
		}
		else if (boundingBox == hotId)
		{
			resetId();
		}
		_translation_ = *outVec3;
		_rotation_ = *outMat3;
		_scale_ = *outScale;
		delete outVec3;
		delete outMat3;
		delete outScale;
		matrixStack.push_back(Coord3(_translation_, _rotation_, _scale_));

		struct Quad
		{
			Eigen::Vector3f a;
			Eigen::Vector3f b;
			Eigen::Vector3f c;
			Eigen::Vector3f d;
		};

		Eigen::Vector3f points[6] = { Eigen::Vector3f(1, 0, 0), Eigen::Vector3f(-1, 0, 0), Eigen::Vector3f(0, 1, 0),
			Eigen::Vector3f(0, -1, 0), Eigen::Vector3f(0, 0, 1), Eigen::Vector3f(0, 0, -1) };
		Quad quads[6] = {
			{Eigen::Vector3f(1, -1, -1), Eigen::Vector3f(1, 1, -1), Eigen::Vector3f(1, 1, 1), Eigen::Vector3f(1, -1, 1)},
			{Eigen::Vector3f(-1, -1, -1), Eigen::Vector3f(-1, 1, -1), Eigen::Vector3f(-1, 1, 1),
				Eigen::Vector3f(-1, -1, 1)},
			{Eigen::Vector3f(-1, 1, -1), Eigen::Vector3f(1, 1, -1), Eigen::Vector3f(1, 1, 1), Eigen::Vector3f(-1, 1, 1)},
			{Eigen::Vector3f(-1, -1, -1), Eigen::Vector3f(1, -1, -1), Eigen::Vector3f(1, -1, 1),
				Eigen::Vector3f(-1, -1, 1)},
			{Eigen::Vector3f(-1, -1, 1), Eigen::Vector3f(1, -1, 1), Eigen::Vector3f(1, 1, 1), Eigen::Vector3f(-1, 1, 1)},
			{Eigen::Vector3f(-1, -1, -1), Eigen::Vector3f(1, -1, -1), Eigen::Vector3f(1, 1, -1), Eigen::Vector3f(-1, 1, -1)}

		};

		for (int i = 0; i < 6; i++)
		{
			drawPoint(points[i], 10, faces[i].centerId == hotId ? Color_Gold : Color_Green);
			if (faces[i].centerId == hotId)
			{
				alphaStack.push_back(0.3);
				colorStack.push_back(Color_Brown);
				drawQuadFilled(quads[i].a, quads[i].b, quads[i].c, quads[i].d);
				alphaStack.pop_back();
				colorStack.pop_back();
			}
		}

		drawPoint(Eigen::Vector3f(0, 0, 0), 10, boxCenter == hotId ? Color_Gold : Color_Green);
		drawLine(Eigen::Vector3f(-1, 0, 0), Eigen::Vector3f(1, 0, 0), 2, Color_White);
		drawLine(Eigen::Vector3f(0, -1, 0), Eigen::Vector3f(0, 1, 0), 2, Color_White);
		drawLine(Eigen::Vector3f(0, 0, -1), Eigen::Vector3f(0, 0, 1), 2, Color_White);
		if (boundingBox == activeId)
		{
			colorStack.push_back(Eigen::Vector4<uint8_t>(255, 82, 111, 108));

			alphaStack.push_back(0.4);
			drawAlignedBoxFilled(Eigen::Vector3f(-1, -1, -1), Eigen::Vector3f(1, 1, 1));
			alphaStack.pop_back();
			colorStack.pop_back();
		}

		sizeStack.push_back(4.0);

		colorStack.push_back((boxCenter == activeId || boundingBox == activeId) ? Color_Green : Color_White);
		drawAlignedBox(Eigen::Vector3f(-1, -1, -1), Eigen::Vector3f(1, 1, 1));
		colorStack.pop_back();
		sizeStack.pop_back();
		matrixStack.pop_back();
		popId();

		return ret;
	}

	bool Guizmo::sphereEdit(unsigned int _id, Eigen::Vector3f& _translation_, float& r)
	{
		bool ret = false;
		Eigen::Vector3f* outVec3 = new Eigen::Vector3f(0, 0, 0);
		*outVec3 = _translation_;
		Eigen::Vector3f drawAt = *outVec3;
		idStack.push_back(_id);
		appId = _id;
		unsigned int centerId = makeId("Sphere");
		ret |= gizmoSpherePlaneTranslationBehavior(centerId, drawAt, pixelsToWorldSize(drawAt, 15),
			cameraParam.viewDirectioin, cameraParam.snapTranslation, outVec3);
		Eigen::Vector3f axis[6] = { Eigen::Vector3f(1, 0, 0), Eigen::Vector3f(-1, 0, 0), Eigen::Vector3f(0, 1, 0),
			Eigen::Vector3f(0, -1, 0), Eigen::Vector3f(0, 0, 1), Eigen::Vector3f(0, 0, -1) };

		struct PointCenter
		{
			unsigned int centerId;
			Eigen::Vector3f pos;
			Eigen::Vector3f nor;
			float size;
			// float* outS;
			PointCenter(const unsigned int& c, const Eigen::Vector3f& v, const Eigen::Vector3f& n)
				: centerId(c)
				, pos(v)
				, nor(n)
			{
			}
		};
		PointCenter points[6] = {
			{makeId("RightPointCenter"), Eigen::Vector3f(drawAt + axis[0] * r), axis[0]},
			{makeId("LeftPointCenter"), Eigen::Vector3f(drawAt + axis[1] * r), axis[1]},
			{makeId("TopPointCenter"), Eigen::Vector3f(drawAt + axis[2] * r), axis[2]},
			{makeId("BottomPointCenter"), Eigen::Vector3f(drawAt + axis[3] * r), axis[3]},
			{makeId("FrontPointCenter"), Eigen::Vector3f(drawAt + axis[4] * r), axis[4]},
			{makeId("BackPointCenter"), Eigen::Vector3f(drawAt + axis[5] * r), axis[5]},
		};

		for (int i = 0; i < 6; i++)
		{
			Eigen::Vector3f* outFace = new Eigen::Vector3f(0, 0, 0);
			*outFace = points[i].pos;
			float size = pixelsToWorldSize(points[i].pos, 25);
			if (gizmoCircleAxisTranslationBehavior(
				points[i].centerId, points[i].pos, size, points[i].nor, cameraParam.snapTranslation, outFace))
			{

				*outVec3 = *outVec3 + (*outFace - points[i].pos) / 2;
				r = (*outFace - *outVec3).norm();
				ret = true;
			}
			delete outFace;
		}
		sizeStack.push_back(3.0);
		drawPoint(drawAt, 10, centerId == hotId ? Color_Gold : Color_White);
		drawSphere(drawAt, r, 40);
		for (int i = 0; i < 6; i++)
		{
			drawPoint(drawAt + axis[i] * r, 8, points[i].centerId == hotId ? Color_Gold : Color_Green);
		}


		sizeStack.pop_back();
		idStack.pop_back();
		_translation_ = *outVec3;
		delete outVec3;
		return ret;
	}

	bool Guizmo::dragPointInPlane(unsigned int id, Eigen::Vector3f& translation, const Eigen::Vector3f& normal)
	{
		bool ret = false;
		Eigen::Vector3f* outVec3 = new Eigen::Vector3f(translation);

		Eigen::Vector3f drawAt = *outVec3;
		idStack.push_back(id);
		appId = id;

		ret |= gizmoSpherePlaneTranslationBehavior(id, drawAt, pixelsToWorldSize(drawAt, 15), normal, cameraParam.snapTranslation, outVec3);
		drawPoint(drawAt, 10);

		idStack.pop_back();
		translation = *outVec3;
		delete outVec3;
		return ret;
	}

	bool Guizmo::dragPointByAxis(unsigned int id, Eigen::Vector3f& translation, const Eigen::Vector3f& axis)
	{
		bool ret = false;
		Eigen::Vector3f* outVec3 = new Eigen::Vector3f(translation);

		Eigen::Vector3f drawAt = *outVec3;
		idStack.push_back(id);
		appId = id;
		ret |= gizmoSphereAxisTranslationBehavior(id, translation, pixelsToWorldSize(drawAt, 15), axis, cameraParam.snapTranslation, outVec3);

		drawPoint(drawAt, 10);

		idStack.pop_back();
		translation = *outVec3;
		delete outVec3;
		return ret;
	}

	bool Guizmo::clydinerEdit(unsigned int _id, Eigen::Vector3f& _translation_, Eigen::Vector3f& _normal, float& height,
		float& _tr, float& _br, bool same)
	{
		if (same)
		{
			_br = _tr;
		}


		bool ret = false;
		unsigned int centerId = makeId("Clydiner");
		//ret |= translation(_id, _translation_, false);

		Eigen::Vector3f* outVec3 = new Eigen::Vector3f(_translation_);
		Eigen::Vector3f* outNormal = new Eigen::Vector3f(_normal);
		Eigen::Vector3f drawAt = *outVec3;
		Eigen::Vector3f normal = *outNormal;
		ret |= gizmoSpherePlaneTranslationBehavior(centerId, _translation_, pixelsToWorldSize(_translation_, 15), _normal, cameraParam.snapTranslation, outVec3);

		//
		float worldHeight = pixelsToWorldSize(drawAt, 40);
		float worldSize = pixelsToWorldSize(drawAt, 5);
		idStack.push_back(_id);

		appId = _id;
		unsigned int topArrow = makeId("topArrow");
		unsigned int bottomArrow = makeId("bottomArrow");
		unsigned int opNormal = makeId("makeNormal");
		Eigen::Vector3f up = abs(normal.y()) > 0.99 ? Eigen::Vector3f(1, 0, 0) : Eigen::Vector3f(0, 1, 0);
		Eigen::Vector3f xaxis = normal.cross(up).normalized();

		Eigen::Vector3f zaxis = xaxis.cross(normal).normalized();
		Eigen::Vector3f axis[6] = { xaxis, -xaxis, zaxis, -zaxis, normal, -normal };
		float h = height / 2;
		float tr = _tr;
		float br = _br;
		struct PointCenter
		{
			unsigned int centerId;
			Eigen::Vector3f pos;
			Eigen::Vector3f nor;
			float size;
			// float* outS;
			PointCenter(const unsigned int& c, const  Eigen::Vector3f& v, const  Eigen::Vector3f& n)
				: centerId(c)
				, pos(v)
				, nor(n)
			{
			}
		};
		PointCenter topCircle[4] = { {makeId("TPX"), drawAt + normal * h + xaxis * tr, axis[0]},
			{makeId("TNX"), drawAt + normal * h - xaxis * tr, axis[1]},
			{makeId("TPZ"), drawAt + normal * h + zaxis * tr, axis[2]},
			{makeId("TNZ"), drawAt + normal * h - zaxis * tr, axis[3]} };
		PointCenter bottomCircle[4] = { {makeId("BPX"), drawAt - normal * h + xaxis * br, axis[0]},
			{makeId("BNX"), drawAt - normal * h - xaxis * br, axis[1]},
			{makeId("BPZ"), drawAt - normal * h + zaxis * br, axis[2]},
			{makeId("BNZ"), drawAt - normal * h - zaxis * br, axis[3]} };
		for (int i = 0; i < 4; i++)
		{
			Eigen::Vector3f* outFace = new  Eigen::Vector3f(0, 0, 0);
			*outFace = topCircle[i].pos;
			float size = pixelsToWorldSize(topCircle[i].pos, 15);
			if (gizmoCircleAxisTranslationBehavior(topCircle[i].centerId, topCircle[i].pos, size, topCircle[i].nor,
				cameraParam.snapTranslation, outFace))
			{
				ret = true;
				_tr = (*outFace - drawAt - normal * h).norm();
				if (same)
				{
					_br = _tr;
				}
			}
			delete outFace;
		}
		for (int i = 0; i < 4; i++)
		{
			Eigen::Vector3f* outFace = new  Eigen::Vector3f(0, 0, 0);
			*outFace = bottomCircle[i].pos;
			float size = pixelsToWorldSize(bottomCircle[i].pos, 15);
			if (gizmoCircleAxisTranslationBehavior(bottomCircle[i].centerId, bottomCircle[i].pos, size, bottomCircle[i].nor,
				cameraParam.snapTranslation, outFace))
			{
				ret = true;
				_br = (*outFace - drawAt + normal * h).norm();
				if (same)
				{
					_tr = _br;
				}
			}
			delete outFace;
		}
		Eigen::Vector3f* outFace = new  Eigen::Vector3f(drawAt + normal * h);
		if (gizmoAxisTranslationBehavior(
			topArrow, drawAt + normal * h, normal, cameraParam.snapTranslation, worldHeight, worldSize, outFace))
		{
			ret = true;
			float len = (*outFace - drawAt).norm();
			height = (height / 2 + len);
			*outVec3 = *outVec3 + (*outFace - drawAt - normal * h) / 2;
		}
		else
		{
			*outFace = drawAt - normal * h;
			if (gizmoAxisTranslationBehavior(bottomArrow, drawAt - normal * h, -normal, cameraParam.snapTranslation,
				worldHeight, worldSize, outFace))
			{
				ret = true;
				float len = (*outFace - drawAt).norm();
				height = (height / 2 + len);
				*outVec3 = *outVec3 + (*outFace - drawAt + normal * h) / 2;
			}

		}

		// Operate normal

		float sphereRadius = pixelsToWorldSize(drawAt + normal * h, 10.0);
		Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
		Sphere sphere(drawAt + normal * h, sphereRadius);
		float t0 = FLT_MAX, t1 = FLT_MAX;
		bool intersects = Intersect(ray, sphere, t0, t1);
		intersects |= Intersect(ray, Sphere(drawAt - normal * h, sphereRadius), t0, t1);
		Eigen::Vector2f& storeCursor = gizmoCursor;
		Eigen::Vector3f& storeNormal = gizmoNormal;

		if (opNormal == activeId)
		{
			if (isKeyDown(ActionSelect))
			{
				Eigen::Vector2<float> offset = 0.5 * (cameraParam.cursor - storeCursor) / 180.0 * Pi;
				offset.y() *= -1;
				Eigen::Matrix3f view = cameraParam.view.block(0, 0, 3, 3);
				Eigen::Vector3f axis = (view.row(1) * offset.x() - view.row(0) * offset.y()).normalized();
				Eigen::Matrix3f mat = RotationMatrix(axis, offset.norm());
				*outNormal = (mat * storeNormal).normalized();
				ret = true;
			}
			else
			{
				makeActive(IdInvalid);
			}
		}
		else if (opNormal == hotId)
		{
			if (intersects)
			{
				if (isKeyDown(ActionSelect))
				{

					storeCursor = cameraParam.cursor;

					storeNormal = normal.normalized();

					makeActive(opNormal);
				}
			}
			else
			{
				resetId();
			}
		}
		else
		{

			if (intersects)
			{
				makeHot(opNormal, t0, intersects);
			}
		}

		normal = *outNormal;
		drawPoint(drawAt, 10, centerId == hotId ? Color_Gold : Color_White);
		drawCircle(drawAt - normal * h, normal, br, 30);
		drawCircle(drawAt + normal * h, normal, tr, 30);
		drawLine(drawAt + normal * h + xaxis * tr, drawAt - normal * h + xaxis * br, 2, Color_Green);
		drawLine(drawAt + normal * h - xaxis * tr, drawAt - normal * h - xaxis * br, 2, Color_Green);
		drawLine(drawAt + normal * h + zaxis * tr, drawAt - normal * h + zaxis * br, 2, Color_Green);
		drawLine(drawAt + normal * h - zaxis * tr, drawAt - normal * h - zaxis * br, 2, Color_Green);
		drawPoint(drawAt + normal * h + xaxis * tr, 8, Color_Green);
		drawPoint(drawAt - normal * h + xaxis * br, 8, Color_Green);
		drawPoint(drawAt + normal * h - xaxis * tr, 8, Color_Green);
		drawPoint(drawAt - normal * h - xaxis * br, 8, Color_Green);
		drawPoint(drawAt + normal * h + zaxis * tr, 8, Color_Green);
		drawPoint(drawAt - normal * h + zaxis * br, 8, Color_Green);
		drawPoint(drawAt + normal * h - zaxis * tr, 8, Color_Green);
		drawPoint(drawAt - normal * h - zaxis * br, 8, Color_Green);
		drawPoint(drawAt + normal * (h), 8, opNormal == hotId ? Color_Gold : Color_Green);
		drawPoint(drawAt - normal * (h), 8, opNormal == hotId ? Color_Gold : Color_Green);
		drawArrow(topArrow, drawAt + normal * (height / 2), normal, worldHeight, Color_Green);
		drawArrow(bottomArrow, drawAt - normal * (height / 2), -normal, worldHeight, Color_Green);

		idStack.pop_back();


		static bool move = false;
		static Eigen::Vector3f moveToPoint;
		if (wasKeyReleased(ActionSelect) && isKeyDown(ActionControl))
		{

			float p[3] = { 0, 0, 0 };
			if (false)
			{
				move = true;
				moveToPoint = { p[0], p[1], p[2] };
			}

		}
		if (move)
		{
			*outVec3 = 0.85 * (*outVec3) + 0.15 * moveToPoint;
			if ((*outVec3 - moveToPoint).norm() < 0.03f)
			{
				*outVec3 = moveToPoint;
				move = false;

			}
			ret = true;
		}
		_translation_ = *outVec3;
		_normal = *outNormal;
		delete outVec3;
		delete outNormal;
		return ret;
	}

	bool Guizmo::PrismEdit(unsigned int _id, Eigen::Vector3f& _normal, Eigen::Vector3f& _origin, float* _height,
		std::vector<Eigen::Vector3f>& polygon, std::vector<Eigen::Vector2f>& polygon2D, bool flip)
	{
		bool ret = false;
		Eigen::Vector3f* outNormal = new Eigen::Vector3f(_normal);
		Eigen::Vector3f* outOrigin = new Eigen::Vector3f(_origin);
		Eigen::Vector3f normal = *outNormal;
		Eigen::Vector3f planeOrigin = *outOrigin;
		float height = *_height;

		unsigned int planeArrow = makeId("planeArrow");
		unsigned int planeOriginId = makeId("planeOrigin");
		unsigned int polyBaryTop = makeId("polyBaryTop");
		unsigned int polyBaryBottom = makeId("polyBaryBottom");

		Eigen::Vector3f mouse = cameraParam.rayOrigin + cameraParam.rayDirection;
		float wordSize = pixelsToWorldSize(mouse, 3);
		float worldHeight = pixelsToWorldSize(planeOrigin, 50);

		ret |= gizmoSphereAxisTranslationBehavior(planeOriginId, planeOrigin, pixelsToWorldSize(planeOrigin, 15), normal,
			cameraParam.snapTranslation, outOrigin);
		planeOrigin = *outOrigin;
		ret |= gizmoOperateNormalBehavior(
			planeArrow, planeOrigin, planeOrigin + normal * worldHeight, pixelsToWorldSize(planeOrigin, 3), outNormal);

		colorStack.push_back(Color_Red);
		drawCircleFilled(mouse, cameraParam.viewDirectioin, wordSize, 20);
		colorStack.pop_back();
		normal = *outNormal;
		Eigen::Vector3f Up = abs(normal.y()) > 0.99 ? Eigen::Vector3f(1, 0, 0) : Eigen::Vector3f(0, 1, 0);
		Eigen::Vector3f xaxis = normal.cross(Up).normalized();//Normalize(Cross(normal, Up));
		Eigen::Vector3f zaxis = xaxis.cross(normal).normalized();//Normalize(Cross(xaxis, normal));

		if (wasKeyReleased(ActionSelect) && isKeyDown(ActionControl))
		{
			Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
			float t0 = FLT_MAX;
			if (Intersect(ray, Plane(normal, planeOrigin), t0))
			{
				ret = true;
				Eigen::Vector3f intersectPoint = ray.m_origin + ray.m_direction * t0 - planeOrigin;
				polygon2D.push_back(Eigen::Vector2f(intersectPoint.dot(xaxis), intersectPoint.dot(zaxis)));
			}
		}
		drawArrow(planeArrow, planeOrigin, normal, worldHeight, Color_Green);
		drawPoint(planeOrigin, 15, planeOriginId == hotId ? Color_Gold : Color_White);
		for (int i = 0; i < polygon2D.size(); i++)
		{
			int j = (i + 1) % polygon2D.size();
			if (flip)
			{
				drawLine(xaxis * polygon2D[i].x() + zaxis * polygon2D[i].y() + planeOrigin,
					xaxis * polygon2D[i].x() + zaxis * polygon2D[i].y() + planeOrigin + normal * height, 2.0, Color_Blue);
				drawLine(xaxis * polygon2D[i].x() + zaxis * polygon2D[i].y() + planeOrigin,
					xaxis * polygon2D[j].x() + zaxis * polygon2D[j].y() + planeOrigin, 2.0, Color_Blue);
				drawLine(xaxis * polygon2D[i].x() + zaxis * polygon2D[i].y() + planeOrigin + normal * height,
					xaxis * polygon2D[j].x() + zaxis * polygon2D[j].y() + planeOrigin + normal * height, 2.0, Color_Blue);
			}
			else
			{
				drawLine(xaxis * polygon2D[i].x() + zaxis * polygon2D[i].y() + planeOrigin,
					xaxis * polygon2D[i].x() + zaxis * polygon2D[i].y() + planeOrigin - normal * height, 2.0, Color_Blue);
				drawLine(xaxis * polygon2D[i].x() + zaxis * polygon2D[i].y() + planeOrigin,
					xaxis * polygon2D[j].x() + zaxis * polygon2D[j].y() + planeOrigin,
					2.0, Color_Blue);
				drawLine(xaxis * polygon2D[i].x() + zaxis * polygon2D[i].y() + planeOrigin - normal * height,
					xaxis * polygon2D[j].x() + zaxis * polygon2D[j].y() + planeOrigin - normal * height, 2.0, Color_Blue);
			}

		}
		Eigen::Vector3f center(0, 0, 0);

		for (const auto& point : polygon2D)
		{
			Eigen::Vector3f temp = xaxis * point.x() + zaxis * point.y() + planeOrigin;
			center = center + temp;
			drawPoint(temp, 10, Color_Green);
			drawPoint(flip ? Eigen::Vector3f(temp + (normal * height)) : temp - normal * height, 10, Color_Green);
		}
		if (polygon2D.size() > 0)
		{
			Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
			float t0 = FLT_MAX;
			if (Intersect(ray, Plane(normal, planeOrigin), t0))
			{
				Eigen::Vector3f last = xaxis * polygon2D.back().x() + zaxis * polygon2D.back().y() + planeOrigin;
				Eigen::Vector3f dir = (ray.m_origin + ray.m_direction * t0 - last).normalized();

				drawArrow(makeId("No unsigned int"), last, dir, pixelsToWorldSize(last, 30), Color_Gold);
			}
		}
		if (polygon2D.size() > 2)
		{
			center = center / polygon2D.size();
			drawPoint(center, 10, Color_Brown);
			drawPoint(flip ? Eigen::Vector3f(center + normal * height) : center - normal * height, 10, Color_Brown);
			drawArrow(polyBaryTop, center, flip ? -normal : normal, worldHeight, Color_Green);
			drawArrow(polyBaryBottom, flip ? Eigen::Vector3f(center + normal * height) : center - normal * height, flip ? normal : -normal,
				worldHeight, Color_Green);

			Eigen::Vector3f s = *outOrigin;
			if (gizmoAxisTranslationBehavior(polyBaryTop, center, flip ? -normal : normal, cameraParam.snapTranslation,
				pixelsToWorldSize(center, 50), pixelsToWorldSize(center, 3), outOrigin))
			{
				s = *outOrigin - s;
				*_height = *_height + s.dot(flip ? -normal : normal);
				ret = true;
			}
			else
			{
				Eigen::Vector3f start = *outOrigin;
				Eigen::Vector3f en = *outOrigin;
				if (gizmoAxisTranslationBehavior(polyBaryBottom, flip ? Eigen::Vector3f(center + normal * height) : center - normal * height,
					flip ? normal : -normal,
					cameraParam.snapTranslation, pixelsToWorldSize(center, 50), pixelsToWorldSize(center, 3),
					&en))
				{
					en = en - start;
					*_height = *_height + en.dot(flip ? normal : -normal);
					ret = true;
				}
			}
			*_height = std::fmax(0, *_height);
		}
		_normal = *outNormal;
		_origin = *outOrigin;
		delete outNormal;
		delete outOrigin;

		if (ret)
		{
			polygon.clear();
			for (const auto& point : polygon2D)
			{
				polygon.push_back(xaxis * point.x() + zaxis * point.y() + planeOrigin);
			}
		}
		return ret;
	}

	bool Guizmo::CurveEdit(unsigned int _id, Eigen::Vector3f& _normal, Eigen::Vector3f& _origin, std::vector<Eigen::Vector3f>& polygon)
	{
		drawRayHitScreenPoint();
		bool ret = false;
		Eigen::Vector3f outNormal = Eigen::Vector3f(_normal);
		Eigen::Vector3f outOrigin = Eigen::Vector3f(_origin);
		Eigen::Vector3f normal = outNormal;
		Eigen::Vector3f planeOrigin = outOrigin;

		unsigned int planeArrow = makeId("planeArrow");
		unsigned int planeOriginId = makeId("planeOrigin");
		unsigned int planeOriginCircle = makeId("planeCircle");

		// follow camera
		outNormal = -cameraParam.viewDirectioin.normalized();

		Eigen::Vector3f mouse = cameraParam.rayOrigin + cameraParam.rayDirection;
		float wordSize = pixelsToWorldSize(mouse, 3);
		float worldHeight = pixelsToWorldSize(planeOrigin, 50);
		float cirleDetectRadius = pixelsToWorldSize(planeOrigin, 40);


		// colorStack.push_back(Color_Red);
		 //drawCircleFilled(mouse, cameraParam.viewDirectioin, wordSize, 20);
		// colorStack.pop_back();
		// normal = *outNormal;

		 //ret |= gizmoSphereAxisTranslationBehavior(planeOriginId, planeOrigin, pixelsToWorldSize(planeOrigin, 15), normal, cameraParam.snapTranslation, outOrigin);
		 //gizmoSpherePlaneTranslationBehavior(
		   //  planeOriginId, planeOrigin, pixelsToWorldSize(planeOrigin, 10), normal, cameraParam.snapTranslation, outOrigin);
		// gizmoCircleAxisTranslationBehavior(planeOriginCircle, planeOrigin, cirleDetectRadius, normal, cameraParam.snapTranslation, outOrigin);
		// gizmoOperateNormalBehavior(planeArrow, planeOrigin, planeOrigin + normal * worldHeight, pixelsToWorldSize(planeOrigin, 3), outNormal);

		planeOrigin = outOrigin;
		/*
		if( wasKeyReleased(ActionSelect) && isKeyDown(ActionControl) )
		{
			Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
			float t0 = FLT_MAX;
			if( Intersect(ray, Plane(normal, planeOrigin), t0) )
			{
				 Eigen::Vector3f intersectPoint = ray.m_origin + ray.m_direction * t0;
				 polygon.push_back(intersectPoint);
				 outOrigin = intersectPoint;
				 ret = true;
			}
		}

		*/


		std::vector<unsigned int> polygonId(polygon.size());
		for (int i = 0; i < polygon.size(); i++)
		{
			char id[20] = "\0";
			sprintf(id, "Polygon%d", i);
			polygonId[i] = makeId(id);
		}
		for (int i = 0; i < polygon.size(); i++)
		{
			Eigen::Vector3f pos = polygon[i];
			//ret|=gizmoPlaneTranslationBehavior(polygonId[i], polygon[i],normal, cameraParam.snapTranslation,pixelsToWorldSize(pos, 15), &pos);
			polygon[i] = pos;
		}
		/*
		Draw plane grid
		drawPlaneGrid(planeOrigin,normal,5,5);

		drawArrow(planeArrow, planeOrigin, normal, worldHeight, Color_Green);
		drawPoint(planeOrigin, 10, planeOriginId == hotId ? Color_Gold:Color_White );
		alphaStack.push_back(0.5);
		colorStack.push_back(planeOriginCircle == hotId ? Color_Gold : Color_Green);
		drawCircleFilled(planeOrigin, normal, cirleDetectRadius, 20);
		sizeStack.push_back(3.0);
		colorStack.push_back( Color_Green);
		drawCircle(planeOrigin, normal, cirleDetectRadius, 20);
		sizeStack.pop_back();
		alphaStack.pop_back();
		colorStack.pop_back();
		colorStack.pop_back();
		*/

		for (int i = 0; i < polygon.size(); i++)
		{
			if (i > 0)
			{
				//drawLine(polygon[i], polygon[i - 1], 2.0, Color_Blue);
			}
			drawPoint(polygon[i], 7, polygonId[i] == hotId ? Color_Gold : Color_Green);
			if (polygonId[i] == hotId)
			{
				colorStack.push_back(Color_Gold);
				drawCircle(polygon[i], cameraParam.viewDirectioin, pixelsToWorldSize(polygon[i], 10), 20);
				colorStack.pop_back();
				alphaStack.push_back(0.3);
				colorStack.push_back(Color_Red);
				drawCircleFilled(polygon[i], cameraParam.viewDirectioin, pixelsToWorldSize(polygon[i], 10), 20);
				alphaStack.pop_back();
				colorStack.pop_back();
			}
		}

		if (polygon.size() > 0)
		{
			Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
			float t0 = FLT_MAX;
			if (Intersect(ray, Plane(normal, planeOrigin), t0))
			{
				Eigen::Vector3f last = polygon.back();
				Eigen::Vector3f dir = (ray.m_origin + ray.m_direction * t0 - last).normalized();
				drawArrow(makeId("NoId"), last, dir, pixelsToWorldSize(last, 30), Color_Gold);
			}
		}
		_normal = outNormal;
		_origin = outOrigin;
		return ret;
	}

	bool Guizmo::CurveEdit(unsigned int _id, Eigen::Vector3f& _normal, Eigen::Vector3f& _origin, std::vector<std::pair<Eigen::Vector3f, Eigen::Vector3f>>& polygon)
	{
		bool ret = false;
		Eigen::Vector3f* outNormal = new Eigen::Vector3f(_normal);
		Eigen::Vector3f* outOrigin = new Eigen::Vector3f(_origin);
		Eigen::Vector3f normal = *outNormal;
		Eigen::Vector3f planeOrigin = *outOrigin;

		unsigned int planeArrow = makeId("planeArrow");
		unsigned int planeOriginId = makeId("planeOrigin");
		unsigned int planeOriginCircle = makeId("planeCircle");

		// follow camera
		//*outNormal = -cameraParam.viewDirectioin.normalized();
		Eigen::Vector3f mouse = cameraParam.rayOrigin + cameraParam.rayDirection;
		float wordSize = pixelsToWorldSize(mouse, 3);
		float worldHeight = pixelsToWorldSize(planeOrigin, 50);
		float cirleDetectRadius = pixelsToWorldSize(planeOrigin, 40);

		colorStack.push_back(Color_Red);
		drawCircleFilled(mouse, cameraParam.viewDirectioin, wordSize, 20);
		colorStack.pop_back();
		// normal = *outNormal;

		// ret |= gizmoSphereAxisTranslationBehavior(planeOriginId, planeOrigin, pixelsToWorldSize(planeOrigin, 15), normal,
		// cameraParam.snapTranslation, outOrigin);
		gizmoSpherePlaneTranslationBehavior(
			planeOriginId, planeOrigin, pixelsToWorldSize(planeOrigin, 10), normal, cameraParam.snapTranslation, outOrigin);
		gizmoCircleAxisTranslationBehavior(
			planeOriginCircle, planeOrigin, cirleDetectRadius, normal, cameraParam.snapTranslation, outOrigin);
		gizmoOperateNormalBehavior(
			planeArrow, planeOrigin, planeOrigin + normal * worldHeight, pixelsToWorldSize(planeOrigin, 3), outNormal);

		planeOrigin = *outOrigin;
		Eigen::Vector3f Up = abs(normal.y()) > 0.99 ? Eigen::Vector3f(1, 0, 0) : Eigen::Vector3f(0, 1, 0);
		Eigen::Vector3f xaxis = normal.cross(Up).normalized();    // Normalize(Cross(normal, Up));
		Eigen::Vector3f zaxis = xaxis.cross(normal).normalized(); // Normalize(Cross(xaxis, normal));

		if (wasKeyReleased(ActionSelect) && isKeyDown(ActionControl))
		{
			Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
			float t0 = FLT_MAX;
			if (Intersect(ray, Plane(normal, planeOrigin), t0))
			{
				Eigen::Vector3f intersectPoint = ray.m_origin + ray.m_direction * t0;
				Eigen::Vector3f tangent = polygon.empty() ? xaxis : (intersectPoint - polygon.back().first).normalized();
				polygon.push_back({ intersectPoint, tangent });
				ret = true;
			}
		}
		std::vector<std::pair<unsigned int, unsigned int>> polygonId(polygon.size());
		for (int i = 0; i < polygon.size(); i++)
		{
			char id[20] = "\0";
			char id1[20] = "\0";
			sprintf(id, "Polygon%d", i);
			sprintf(id1, "Tangent%d", i);
			polygonId[i].first = makeId(id);
			polygonId[i].second = makeId(id1);

		}
		for (int i = 0; i < polygon.size(); i++)
		{
			Eigen::Vector3f pos = polygon[i].first;
			//move polygon's points
			ret |= gizmoPlaneTranslationBehavior(polygonId[i].first, polygon[i].first, normal, cameraParam.snapTranslation,
				pixelsToWorldSize(pos, 15), &pos);
			polygon[i].first = pos;
			pos = polygon[i].first + polygon[i].second;
			//move tangent (normal)
			if (gizmoPlaneTranslationBehavior(polygonId[i].second, pos, normal, cameraParam.snapTranslation, pixelsToWorldSize(pos, 15), &pos))
			{
				ret |= true;
			}

			polygon[i].second = pos - polygon[i].first;

		}

		// Draw plane grid
		drawPlaneGrid(planeOrigin, normal, 5, 5);

		drawArrow(planeArrow, planeOrigin, normal, worldHeight, Color_Green);
		drawPoint(planeOrigin, 10, planeOriginId == hotId ? Color_Gold : Color_White);
		alphaStack.push_back(0.5);
		colorStack.push_back(planeOriginCircle == hotId ? Color_Gold : Color_Green);
		drawCircleFilled(planeOrigin, normal, cirleDetectRadius, 20);
		sizeStack.push_back(3.0);
		colorStack.push_back(Color_Green);
		drawCircle(planeOrigin, normal, cirleDetectRadius, 20);
		sizeStack.pop_back();
		alphaStack.pop_back();
		colorStack.pop_back();
		colorStack.pop_back();
		for (int i = 0; i < polygon.size(); i++)
		{
			if (i > 0)
			{
				drawLine(polygon[i].first, polygon[i - 1].first, 2.0, Color_Blue);
			}
			drawPoint(polygon[i].first, 10, polygonId[i].first == hotId ? Color_Gold : Color_Green);
			drawPoint(polygon[i].first + polygon[i].second, 10, (polygonId[i].first == hotId || polygonId[i].second == hotId) ? Color_Gold : Color_Red);
			drawPoint(polygon[i].first - polygon[i].second, 10, (polygonId[i].first == hotId || polygonId[i].second == hotId) ? Color_Gold : Color_Brown);
			drawLine(polygon[i].first, polygon[i].first + polygon[i].second, 2.0, Color_White);
			drawLine(polygon[i].first, polygon[i].first - polygon[i].second, 2.0, Color_White);

			if (polygonId[i].first == hotId)
			{
				colorStack.push_back(Color_Gold);
				drawCircle(polygon[i].first, cameraParam.viewDirectioin, pixelsToWorldSize(polygon[i].first, 10), 20);
				colorStack.pop_back();
				alphaStack.push_back(0.3);
				colorStack.push_back(Color_Red);
				drawCircleFilled(polygon[i].first, cameraParam.viewDirectioin, pixelsToWorldSize(polygon[i].first, 10), 20);
				alphaStack.pop_back();
				colorStack.pop_back();
			}
		}
		//Draw Arrow between last point and hit point
		if (polygon.size() > 0)
		{
			Ray ray(cameraParam.rayOrigin, cameraParam.rayDirection);
			float t0 = FLT_MAX;
			if (Intersect(ray, Plane(normal, planeOrigin), t0))
			{
				Eigen::Vector3f last = polygon.back().first;
				Eigen::Vector3f dir = (ray.m_origin + ray.m_direction * t0 - last).normalized();
				drawArrow(makeId("NoId"), last, dir, pixelsToWorldSize(last, 30), Color_Gold);
			}
		}
		_normal = *outNormal;
		_origin = *outOrigin;
		delete outNormal;
		delete outOrigin;
		return ret;
	}
	bool Guizmo::drawTranslate2D(unsigned int id, Eigen::Vector2f& pos)
	{

		auto c1 = ImGui::ColorConvertFloat4ToU32({ 0.8156f, 0.9215f, 1.0f, 1.0f });
		auto c2 = ImGui::ColorConvertFloat4ToU32({ 1, 1, 0, 1 });
		float radius = 4.0f;

		bool ret = false;
		bool hitFlag = (pos - cameraParam.cursor).norm() < radius;

		if (id == activeId2D)
		{
			if (isKeyDown(ActionSelect))
			{
				pos = cameraParam.cursor + gizmoVec2;
				ret = true;
			}
			else
			{
				makeActive2D(IdInvalid);
			}
		}
		else if (id == hotId2D)
		{
			if (hitFlag)
			{
				if (isKeyDown(ActionSelect))
				{
					makeActive2D(id);
					gizmoVec2 = pos - cameraParam.cursor;
				}
			}
			else
			{
				resetId2D();
			}
		}
		else
		{
			if (hitFlag)
			{
				makeHot2D(id);
			}
		}
		pos.x() = Clamp<float>(pos.x(), 0.0f, cameraParam.viewportWidth);
		pos.y() = Clamp<float>(pos.y(), 0.0f, cameraParam.viewportHeight);
		return ret;
	}
	CameraParam& Guizmo::getCameraParam()
	{
		return cameraParam;
	}

	void Guizmo::newFrame(const OvEditor::Panels::SceneView* sceneView)
	{
		litVertexArray.clear();
		drawMeshList.clear();
		renderView = const_cast<OvEditor::Panels::SceneView*>(sceneView);
		// all state stacks should be default here.
		assert(colorStack.size() == 1);
		assert(alphaStack.size() == 1);
		assert(sizeStack.size() == 1);
		assert(enableSortingStack.size() == 1);
		assert(layerIdStack.size() == 1);
		assert(matrixStack.size() == 1);
		assert(idStack.size() == 1);
		assert(primMode == PrimitiveModeNone);
		primMode = PrimitiveModeNone;
		primType = DrawPrimitiveCount;
		assert(vertexData[0].size() == vertexData[1].size());
		for (int i = 0; i < vertexData[0].size(); ++i)
		{
			vertexData[0][i]->clear();
			vertexData[1][i]->clear();
		}
		drawLists.clear();
		sortCalled = false;
		endFrameCalled = false;

		// copy keydown array internally so that we can make a delta to detect key presses
		memcpy(keyDownPrev, keyDownCurr, KeyCount); // \todo avoid this copy, use an index
		memcpy(keyDownCurr, cameraParam.keyDown, KeyCount); // must copy in case m_keyDown is updated after reset (e.g. by an app callback)

		// update gizmo modes
		if (wasKeyPressed(ActionGizmoTranslation))
		{
			gizmoMode = GizmoModeTranslation;
			resetId();
		}
		else if (wasKeyPressed(ActionGizmoRotation))
		{
			gizmoMode = GizmoModeRotation;
			resetId();
		}
		else if (wasKeyPressed(ActionGizmoScale))
		{
			gizmoMode = GizmoModeScale;
			resetId();
		}
		if (wasKeyPressed(ActionGizmoLocal))
		{
			gizmoLocal = !gizmoLocal;
			resetId();
		}

		// set up engine params
		auto& inputState = renderView->getInutState();
		auto& viewMatrix = renderView->GetCamera()->GetViewMatrix();
		Eigen::Matrix4f view;
		memcpy(view.data(), viewMatrix.data, 16 * sizeof(float));
		view.transposeInPlace();
		auto viewinverse = view.inverse();
		auto& projMat = renderView->GetCamera()->GetProjectionMatrix();
		Eigen::Matrix4f proj;
		memcpy(proj.data(), projMat.data, 16 * sizeof(float));
		proj.transposeInPlace();
		Eigen::Vector3f vieweye = Eigen::Vector3f(viewinverse(0, 3), viewinverse(1, 3), viewinverse(2, 3));
		Eigen::Vector3f viewforward = Eigen::Vector3f(viewinverse(0, 2), viewinverse(1, 2), viewinverse(2, 2)).normalized();
		Eigen::Matrix4f camWorld = viewinverse;

		auto [w, h] = renderView->GetSafeSize();
		auto [x, y] = inputState.GetMousePosition();
		Eigen::Vector2<float> cursorPos = Eigen::Vector2<float>(x, y);
		cursorPos = Eigen::Vector2<float>((cursorPos.x() / w) * 2.0f - 1.0f, (cursorPos.y() / h) * 2.0f - 1.0f);
		cursorPos(1) = -cursorPos(1);
		Eigen::Vector3f rayOrigin, rayDirection;
		//
		bool isOrth = renderView->GetCamera()->GetProjectionMode() == OvRendering::Settings::EProjectionMode::ORTHOGRAPHIC;

		if (isOrth)
		{
			rayOrigin(0) = cursorPos.x() / proj(0, 0) - proj(0, 3) / proj(0, 0);
			rayOrigin(1) = cursorPos.y() / proj(1, 1) - proj(1, 3) / proj(1, 1);
			rayOrigin(2) = 0.0f;
			Eigen::Vector4<float> it = camWorld * Eigen::Vector4<float>(rayOrigin.x(), rayOrigin.y(), rayOrigin.z(), 1.0f);
			rayOrigin = Eigen::Vector3f(it.x() / it.w(), it.y() / it.w(), it.z() / it.w());
			it = camWorld * Eigen::Vector4<float>(0.0f, 0.0f, -1.0f, 0.0f);
			rayDirection = Eigen::Vector3f(it.x(), it.y(), it.z());
		}
		else
		{
			rayOrigin = vieweye;
			rayDirection(0) = cursorPos.x() / proj(0, 0);
			rayDirection(1) = cursorPos.y() / proj(1, 1);
			rayDirection(2) = -1.0f;
			Eigen::Vector3f rayDirectionNormalized = rayDirection.normalized();
			Eigen::Vector4<float> it = camWorld * Eigen::Vector4<float>(rayDirectionNormalized.x(),
				rayDirectionNormalized.y(), rayDirectionNormalized.z(), 0.0f);
			rayDirection = Eigen::Vector3f(it.x(), it.y(), it.z());
		}

		cameraParam.viewportWidth = w;
		cameraParam.viewportHeight = h;
		///
		cameraParam.projectY = 2.0 / proj(1, 1);
		cameraParam.orthProj = true;
		cameraParam.eye = vieweye;
		/// for Orth
		cameraParam.viewDirectioin = viewforward;
		cameraParam.rayOrigin = rayOrigin;
		cameraParam.rayDirection = rayDirection;
		cameraParam.cursor.x() = x;
		cameraParam.cursor.y() = y;
		cameraParam.view = view;
		cameraParam.inverseView = view.inverse();
		cameraParam.proj = proj;
		cameraParam.viewProj = proj * view;

		bool ctrlDown = inputState.IsKeyPressed(OvEditor::Panels::Control);

		cameraParam.keyDown[MouseLeft] = inputState.IsMouseButtonPressed(OvEditor::Panels::MOUSE_BUTTON_LEFT);
		cameraParam.keyDown[ActionControl] = ctrlDown;

		cameraParam.keyDown[KeyL] = ctrlDown && inputState.IsKeyPressed(OvEditor::Panels::KEYL);

		cameraParam.keyDown[KeyT] = ctrlDown && inputState.IsKeyPressed(OvEditor::Panels::KEYT);
		cameraParam.keyDown[KeyR] = ctrlDown && inputState.IsKeyPressed(OvEditor::Panels::KEYR);
		cameraParam.keyDown[KeyS] = ctrlDown && inputState.IsKeyPressed(OvEditor::Panels::KEYS);

		cameraParam.snapTranslation = 0.0f;
		cameraParam.snapRotation = 0.0f;
		cameraParam.snapScale = 0.0f;
	}

	void Guizmo::test()
	{
		drawViewCube();
		colorStack.push_back(Color_Gold);
		drawPoint({ 0,0,0 }, 40);
		colorStack.push_back(Color_Red);
		drawPoint({ 1,1,1 }, 40);
		colorStack.pop_back();
		colorStack.pop_back();
		drawAlignedBox({ -1,-1,-1 }, { 1,1,1 });
		drawSphere({ 2,2,2 }, 2);
		static Eigen::Vector3f t = { 0,0,0 };
		static Eigen::Matrix3f rotation = Eigen::Matrix3f::Identity();
		static Eigen::Vector3f scale = { 1,1,1 };

		boxEdit(makeId("box edit"), t, rotation, scale);
	}

	void Guizmo::drawUnsort()
	{
		auto driver = OvCore::Global::ServiceLocator::Get<OvEditor::Core::Context>().driver.get();
		auto p_pso = driver->CreatePipelineState();
		p_pso.stencilTest = true;
		p_pso.stencilWriteMask = 0xFF;
		p_pso.stencilFuncRef = 1;
		p_pso.stencilFuncMask = 0xFF;
		p_pso.stencilOpFail = OvRendering::Settings::EOperation::REPLACE;
		p_pso.depthOpFail = OvRendering::Settings::EOperation::REPLACE;
		p_pso.bothOpFail = OvRendering::Settings::EOperation::REPLACE;
		p_pso.colorWriting.mask = 0x00;
		p_pso.depthWriting = true;
		p_pso.colorWriting.mask = 0xFF;
		p_pso.blending = true;
		p_pso.blendingEquation = OvRendering::Settings::EBlendingEquation::FUNC_ADD;
		p_pso.blendingSrcFactor = OvRendering::Settings::EBlendingFactor::SRC_ALPHA;
		p_pso.depthFunc = OvRendering::Settings::EComparaisonAlgorithm::ALWAYS;
		///p_pso.culling = ;
		p_pso.depthTest = true;

		drawLists.clear();
		// draw unsorted primitives first
		for (int i = 0; i < vertexData[0].size(); ++i)
		{
			if (vertexData[0][i]->size() > 0)
			{
				drawLists.push_back(DrawList());
				DrawList& dl = drawLists.back();
				dl.layerId = layerIdMap[i / DrawPrimitiveCount];
				dl.primType = (DrawPrimitiveType)(i % DrawPrimitiveCount);
				dl.vertexData = vertexData[0][i]->data();
				dl.vertexCount = vertexData[0][i]->size();
			}
		}
		driver->SetPipelineState(p_pso);
		glEnable(GL_POINT_SPRITE);
		//glEnable(GL_BLEND);
		//glBlendEquation(GL_FUNC_ADD);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_PROGRAM_POINT_SIZE);
		//glEnable(GL_DEPTH_TEST);
		//glDepthFunc(GL_ALWAYS);
		//glDisable(GL_CULL_FACE);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glEnable(GL_POLYGON_OFFSET_FILL); // 2.0f, 25.0f
		glViewport(0, 0, cameraParam.viewportWidth, cameraParam.viewportHeight);

		for (int i = 0, n = drawLists.size(); i < n; ++i)
		{
			const DrawList& drawList = drawLists[i];
			unsigned int prim;
			OvRendering::Data::Material* mat = nullptr;
			std::string passTag = "";
			switch (drawList.primType)
			{
			case DrawPrimitivePoints:
				prim = GL_POINTS;
				passTag = "point";
				mat = mPointMaterial;
				break;
			case DrawPrimitiveLines:
				prim = GL_LINES;
				passTag = "line";
				mat = mLineMaterial;
				break;
			case DrawPrimitiveTriangles:
				prim = GL_TRIANGLES;
				passTag = "triangle";
				mat = mTriangleMaterial;
				// glEnable(GL_CULL_FACE); // culling valid for triangles, but optional
				break;
			default:
				assert(false);
				return;
			};

			glBindVertexArray(VertexArray);
			glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)drawList.vertexCount * sizeof(VertexData),
				(GLvoid*)drawList.vertexData, GL_STREAM_DRAW);
			//mShader->setActiveVariant(passTag, {});
			//mShader->bindActiveProgram();
			//mShader->commitDescriptorSetToActiveProgram();
			if (drawList.primType == DrawPrimitiveLines) {
				mat->SetProperty("uViewport", OvMaths::FVector2{ cameraParam.viewportWidth,cameraParam.viewportHeight });

			}
			mat->SetProperty("uViewProjMatrix", ToFMatrix4(cameraParam.viewProj));
			mat->Bind();
			glDrawArrays(prim, 0, (GLsizei)drawList.vertexCount);
		}
	}

	void Guizmo::drawSort()
	{
		drawLists.clear();
		auto driver = OvCore::Global::ServiceLocator::Get<OvEditor::Core::Context>().driver.get();
		auto p_pso = driver->CreatePipelineState();
		p_pso.stencilTest = true;
		p_pso.stencilWriteMask = 0xFF;
		p_pso.stencilFuncRef = 1;
		p_pso.stencilFuncMask = 0xFF;
		p_pso.stencilOpFail = OvRendering::Settings::EOperation::REPLACE;
		p_pso.depthOpFail = OvRendering::Settings::EOperation::REPLACE;
		p_pso.bothOpFail = OvRendering::Settings::EOperation::REPLACE;
		p_pso.colorWriting.mask = 0x00;
		p_pso.depthWriting = true;
		p_pso.colorWriting.mask = 0xFF;
		p_pso.blending = true;
		p_pso.blendingEquation = OvRendering::Settings::EBlendingEquation::FUNC_ADD;
		///p_pso.culling = ;
		p_pso.blendingSrcFactor = OvRendering::Settings::EBlendingFactor::SRC_ALPHA;
		p_pso.blendingDestFactor = OvRendering::Settings::EBlendingFactor::ONE_MINUS_SRC_ALPHA;
		p_pso.depthTest = true;
		p_pso.depthFunc = OvRendering::Settings::EComparaisonAlgorithm::LESS_EQUAL;
		driver->SetPipelineState(p_pso);
		// Enable Depth Test
		//glEnable(GL_DEPTH_TEST);
		//glEnable(GL_POINT_SPRITE);
		//glEnable(GL_BLEND);
		//glBlendEquation(GL_FUNC_ADD);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glDepthFunc(GL_LEQUAL);
		for (int i = 0; i < vertexData[1].size(); ++i)
		{
			if (vertexData[1][i]->size() > 0)
			{
				drawLists.push_back(DrawList());
				DrawList& dl = drawLists.back();
				dl.layerId = layerIdMap[i / DrawPrimitiveCount];
				dl.primType = (DrawPrimitiveType)(i % DrawPrimitiveCount);
				dl.vertexData = vertexData[1][i]->data();
				dl.vertexCount = vertexData[1][i]->size();
			}
		}
		for (int i = 0, n = drawLists.size(); i < n; ++i)
		{
			const DrawList& drawList = drawLists[i];
			unsigned int prim;

			std::string passTag = "";
			OvRendering::Data::Material* mat = nullptr;
			switch (drawList.primType)
			{
			case DrawPrimitivePoints:
				prim = GL_POINTS;
				mat = mPointMaterial;
				passTag = "point";
				break;
			case DrawPrimitiveLines:
				prim = GL_LINES;
				mat = mLineMaterial;
				passTag = "line";
				break;
			case DrawPrimitiveTriangles:
				prim = GL_TRIANGLES;
				mat = mTriangleMaterial;
				passTag = "triangle";
				// glEnable(GL_CULL_FACE); // culling valid for triangles, but optional
				break;
			default:
				assert(false);
				return;
			};

			glBindVertexArray(VertexArray);
			glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)drawList.vertexCount * sizeof(VertexData),
				(GLvoid*)drawList.vertexData, GL_STREAM_DRAW);
			if (drawList.primType == DrawPrimitiveLines) {
				mat->SetProperty("uViewport", OvMaths::FVector2{ cameraParam.viewportWidth,cameraParam.viewportHeight });

			}
			mat->SetProperty("uViewProjMatrix", ToFMatrix4(cameraParam.viewProj));;
			mat->Bind();
			glDrawArrays(prim, 0, (GLsizei)drawList.vertexCount);

		}

		//draw lit vertex data
		{
			//glBindVertexArray(VertexArray);
			//glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
			//glBufferData(GL_ARRAY_BUFFER, litVertexArray.size() * sizeof(VertexData), (GLvoid*)litVertexArray.data(), GL_STREAM_DRAW);

			////mShader->setActiveVariant("Lit", {});
			////mShader->bindActiveProgram();
			//glDrawArrays(GL_TRIANGLES, 0, (GLsizei)litVertexArray.size());
			////mShader->unBindActiveProgram();
		}

	}

	void Guizmo::drawMesh()
	{


	}
	void Guizmo::executeCommand()
	{
		buffer.flush();
		auto ranges = buffer.waitForCommands();
		for (auto& item : ranges)
		{
			if (item.begin)
			{
				command.execute(item.begin);
				buffer.releaseBuffer(item);
			}
		}
	}

	void Guizmo::endFrame()
	{
		executeCommand();
		runDrawTask();
		drawWidgets();

		//test();
		assert(!endFrameCalled);
		endFrameCalled = true;

		drawMesh();
		drawSort();
		drawUnsort();
	}
	void Guizmo::clear()
	{
		//to do something here.
	}
	void Guizmo::updateDepth(float _depth)
	{
		hotDepth = _depth;

	}
	bool Guizmo::makeHot(unsigned int _id, float _depth, bool _intersects)
	{
		if (activeId == IdInvalid && _depth < hotDepth && _intersects && !isKeyDown(ActionSelect))
		{
			hotId = _id;
			appHotId = appId;
			hotDepth = _depth;

			return true;
		}

		return false;
	}
	bool Guizmo::makeHot2D(unsigned int id)
	{
		hotId2D = id;
		return true;
	}
	bool Guizmo::isHot(unsigned int id)
	{
		return id == hotId;
	}
	bool Guizmo::isHot2D(unsigned int id)
	{
		return id == hotId2D;
	}
	bool Guizmo::isAppActive(unsigned int id)
	{
		return appActiveId == id;
	}
	void Guizmo::makeActive(unsigned int id)
	{
		activeId = id;
		appActiveId = id == IdInvalid ? IdInvalid : appId;
	}
	void Guizmo::makeActive2D(unsigned int _id)
	{
		activeId2D = _id;
	}
	void Guizmo::resetId()
	{
		activeId = hotId = appActiveId = appHotId = IdInvalid;
		hotDepth = FLT_MAX;
	}
	void Guizmo::resetId2D()
	{
		activeId2D = hotId2D = IdInvalid;
	}
	unsigned int Guizmo::makeId(const char* str)
	{
		unsigned int ret = idStack.size();
		while (*str)
		{
			ret ^= (unsigned int)*str++;
			ret *= kFnv1aPrime32;
		}
		return ret;
	}
	void Guizmo::pushEnableSorting(bool enable)
	{
		assert(primMode == PrimitiveModeNone); // can't change sort mode mid-primitive
		vertexDataIndex = enable ? 1 : 0;
		enableSortingStack.push_back(enable);
	}
	void Guizmo::popEnableSorting()
	{
		assert(primMode == PrimitiveModeNone); // can't change sort mode mid-primitive
		enableSortingStack.pop_back();
		vertexDataIndex = enableSortingStack.back() ? 1 : 0;
	}
	void Guizmo::setEnableSorting(bool _enable)
	{
		assert(primMode == PrimitiveModeNone); // can't change sort mode mid-primitive
		vertexDataIndex = _enable ? 1 : 0;
		enableSortingStack.back() = _enable;
	}
	void Guizmo::setEnableLit(bool enable)
	{
		enableLit = enable;
	}
	void Guizmo::pushLayerId(unsigned int layer)
	{
		assert(primMode == PrimitiveModeNone); // can't change layer mid-primitive
		int idx = findLayerIndex(layer);
		if (idx == -1) // not found, push new layer
		{
			idx = layerIdMap.size();
			layerIdMap.push_back(layer);
			for (int i = 0; i < DrawPrimitiveCount; ++i)
			{
				vertexData[0].push_back(new VertexList());
				*(vertexData[0].back()) = std::vector<VertexData>();
				vertexData[1].push_back(new VertexList());
				*(vertexData[1].back()) = std::vector<VertexData>();
			}
		}
		layerIdStack.push_back(layer);
		layerIndex = idx;
	}
	void Guizmo::popLayerId()
	{
		assert(layerIdStack.size() > 1);
		layerIdStack.pop_back();
		layerIndex = findLayerIndex(layerIdStack.back());
	}
	Guizmo::~Guizmo()
	{
		for (int i = 0; i < vertexData[0].size(); i++)
		{
			delete vertexData[0][i];
			delete vertexData[1][i];
		}
		terminate();
	}
	float Guizmo::pixelsToWorldSize(const Eigen::Vector3f& position, float pixels)
	{
		float d = cameraParam.orthProj ? 1.0f : (position - cameraParam.eye).norm();
		return cameraParam.projectY * d * (pixels / cameraParam.viewportHeight);
	}
	Eigen::Vector3f Guizmo::screenToWorld(const Eigen::Vector2f& pos)
	{
		Eigen::Vector3f p = { 2.0f * pos.x() / cameraParam.viewportWidth - 1.0f, 2.0f * (1.0f - pos.y() / cameraParam.viewportHeight) - 1.0f, 0.0f };
		return MatrixMulPoint(cameraParam.viewProj.inverse(), p);
	}
	void Guizmo::runDrawTask()
	{
		for (auto& drawTask : mDrawTaskMap)
		{
			drawTask.second();
		}
		for (auto& item : cancelList)
		{
			if (mDrawTaskMap.find(item) != mDrawTaskMap.end())
			{
				mDrawTaskMap.erase(item);
			}
		}
		cancelList.clear();
	}
	void Guizmo::drawWidgets()
	{

	}
	void Guizmo::placeDrawTask(const std::string& name, std::function<void()> task)
	{
		if (mDrawTaskMap.find(name) != mDrawTaskMap.end())
		{
			mDrawTaskMap.erase(name);
		}
		mDrawTaskMap.emplace(name, task);
	}

	void Guizmo::cancleDrawTask(const std::string& name)
	{
		cancelList.push_back(name);
	}
	void Guizmo::create2DRender()
	{
		this->m_points = new GLRenderPoints();
		this->m_lines = new GLRenderLines();
		this->m_triangles = new GLRenderTriangles();
		this->m_points->init();
		this->m_lines->init();
		this->m_triangles->init();
	}
	int Guizmo::findLayerIndex(unsigned int _id) const
	{
		for (int i = 0; i < (int)layerIdMap.size(); ++i)
		{
			if (layerIdMap[i] == _id)
			{
				return i;
			}
		}
		return -1;
	}
	VertexList* Guizmo::getCurrentVertexList()
	{
		return vertexData[vertexDataIndex][layerIndex * DrawPrimitiveCount + primType];
	}
}
