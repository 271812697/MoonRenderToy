#pragma once
#include <Eigen/Core>
#include <Maths/FMatrix4.h>
namespace Rendering {
	namespace Resources {
		class Texture;
		class Mesh;
		class Model;
	}

}

namespace MOON
{
	class Gizmo;
	struct VertexData
	{
		Eigen::Vector4<float> positionSize; // xyz = position, w = size
		Eigen::Vector4<uint8_t> color;      // rgba8 (MSB = r)
		Eigen::Vector3<float> normal;
		Eigen::Vector2<float> uv;
		VertexData()
		{
		}
		VertexData(const Eigen::Vector3f& _position, float _size, Eigen::Vector4<uint8_t> _color)
			: positionSize(_position.x(), _position.y(), _position.z(), _size)
			, color(_color)
		{
		}
		VertexData(const Eigen::Vector3f& p, const Eigen::Vector4f& c, const Eigen::Vector3f& n, const Eigen::Vector2f& texCoord)
			: positionSize(p.x(), p.y(), p.z(), 1.0)
			, normal(n)
			, uv(texCoord)
			, color(c.w() * 255, c.z() * 255, c.y() * 255, c.x() * 255)
		{
		}
		VertexData(const Eigen::Vector3f& p, const Eigen::Vector4f& c, const Eigen::Vector3f& n)
			: positionSize(p.x(), p.y(), p.z(), 1.0)
			, normal(n)
			, uv(1.0, 1.0)
			, color(c.w() * 255, c.z() * 255, c.y() * 255, c.x() * 255)
		{
		}
		VertexData(const Eigen::Vector3f& p, const Eigen::Vector4<uint8_t>& c, const Eigen::Vector3f& n)
			: positionSize(p.x(), p.y(), p.z(), 1.0)
			, normal(n)
			, uv({ 1.0,1.0 })
			, color(c)
		{

		}
		VertexData(const Eigen::Vector3f& p, const Eigen::Vector4<uint8_t>& c, const Eigen::Vector3f& n, const Eigen::Vector2f& texCoord)
			: positionSize(p.x(), p.y(), p.z(), 1.0)
			, normal(n)
			, uv(texCoord)
			, color(c)
		{
		}
		VertexData(const Eigen::Vector3f& p, float blockId,const Eigen::Vector4<uint8_t>& c, const Eigen::Vector3f& n, const Eigen::Vector2f& texCoord)
			: positionSize(p.x(), p.y(), p.z(), blockId)
			, normal(n)
			, uv(texCoord)
			, color(c)
		{
		}
	};
	typedef std::vector<VertexData> VertexList;
	enum DrawPrimitiveType
	{
		// order here determines the order in which unsorted primitives are drawn
		DrawPrimitivePoints,
		DrawPrimitiveLines,
		DrawPrimitiveTriangles,
		DrawPrimitiveCount
	};

	struct DrawList
	{
		unsigned int layerId;
		DrawPrimitiveType primType;
		const VertexData* vertexData;
		unsigned int vertexCount;
	};

	enum Key
	{
		MouseLeft,
		MouseMiddle,
		MouseRight,
		KeyL,
		KeyR,
		KeyS,
		KeyT,
		KeyControl,
		KeyCount,
		ActionSelect = MouseLeft,
		ActionControl = KeyControl,
		ActionGizmoLocal = KeyL,
		ActionGizmoRotation = KeyR,
		ActionGizmoScale = KeyS,
		ActionGizmoTranslation = KeyT,

		ActionCount
	};
	enum PrimitiveMode
	{
		PrimitiveModeNone,
		PrimitiveModePoints,
		PrimitiveModeLines,
		PrimitiveModeLineStrip,
		PrimitiveModeLineLoop,
		PrimitiveModeTriangles,
		PrimitiveModeTriangleStrip
	};
	enum GizmoMode
	{
		GizmoModeTranslation,
		GizmoModeRotation,
		GizmoModeScale
	};
	struct CameraParam
	{
		bool keyDown[KeyCount] = { false };
		float viewportWidth;
		float viewportHeight;
		float projectY = 1.0;
		bool orthProj = false;
		Eigen::Vector3f eye;
		Eigen::Vector3f viewDirectioin;
		Eigen::Vector3f rayOrigin;
		Eigen::Vector3f rayDirection;
		Eigen::Vector2<float> cursor;
		Eigen::Matrix4f view;
		Eigen::Matrix4f inverseView;
		Eigen::Matrix4f proj;
		Eigen::Matrix4f viewProj;
		float snapTranslation = 0.0f; // Snap value for translation gizmos (world units). 0 = disabled.
		float snapRotation = 0.0f;    // Snap value for rotation gizmos (radians). 0 = disabled.
		float snapScale = 0.0f;
		bool flipGizmoWhenBehind = true;
	};
	struct Cell
	{
		std::vector<Eigen::Vector3f>vertex;
		std::vector<Eigen::Vector2f>uv;
		int blockId=0;
		Eigen::Vector3f n;
		Eigen::Vector4<uint8_t> color = { 255,255,255,255 };
		void clear();
		Cell() = default;

		Cell(const Eigen::Vector3f& v0, const Eigen::Vector3f& v1, const Eigen::Vector3f& v2,const Eigen::Vector4<uint8_t>& c);
		void addPoint(const Eigen::Vector3f& v, const Eigen::Vector2f& tex);
		void addPointArray(const std::vector<Eigen::Vector3f>& v, const std::vector<Eigen::Vector2f>& tex);
		Cell transform(const Eigen::Matrix4f& mat, float offsetX = 0.0f, float offsetY = 0.0f);
		void tranformUV(float u, float v);
	};
	struct PolygonInstance
	{
		PolygonInstance(const std::string& n, const Eigen::Matrix4f& m)
			: mesh(n)
			, model(m)
		{
			color = { 1.0f, 1.0f, 1.0f };
		}
		PolygonInstance(const std::string& n, const Eigen::Matrix4f& m, const Eigen::Vector3f& c)
			: mesh(n)
			, model(m)
			, color(c)
		{
		}
		PolygonInstance(std::string&& n, Eigen::Matrix4f&& m)
			: mesh(std::move(n))
			, model(std::move(m))
		{
			color = { 1.0f, 1.0f, 1.0f };
		}
		PolygonInstance(std::string&& n, Eigen::Matrix4f&& m, Eigen::Vector3f&& c)
			: mesh(std::move(n))
			, model(std::move(m))
			, color(c)
		{

		}
		void setFixed(bool flag) {
			fixScaled = flag;
		}
		float* getModelData()
		{
			return model.data();
		}
		std::string mesh;
		Eigen::Matrix4f model;
		Eigen::Vector3f color;
		bool fixScaled = false;
	};


	struct Polygon {
		std::vector<Cell>cellArray;
		std::vector<uint8_t>edgeValue;
		std::vector<Maths::FVector4>blockColor;
		std::unordered_map<std::string, int>blockNameToIndex;
		unsigned int vao = 0;
		unsigned int vbo = 0;
		unsigned int numVertex = 0;
		unsigned int nextBlockId=0;
		bool isDirty = false;
		bool blockColorDirty = true;
		bool drawEdge = true;
		Rendering::Resources::Texture* texture = nullptr;
		Rendering::Resources::Texture* edgeTexture = nullptr;
		Rendering::Resources::Texture* blockTexture = nullptr;
		Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
		void setCellColor(int index, const Eigen::Vector4<uint8_t>& color);
		int getBlockId(const std::string& name);
		void setBlockColor(int index,const Maths::FVector4& color);
		void addCell(const Cell& cell);
		void switchNextBlock(const Maths::FVector4& color={1,1,1,1},const std::string& name="empty");
		void submit();
		void bind();
		Eigen::Vector3f getCellNormal(int index);
		Polygon() = default;
		Polygon(const std::vector<Cell>& cells) :cellArray(cells) {
			vao = 0;
			vbo = 0;
			initGpuBuffer();
		}
		Polygon(const std::vector<Cell>&& cells) :cellArray(std::move(cells)) {
			vao = 0;
			vbo = 0;
			initGpuBuffer();
		}
		~Polygon();
		void addMesh(Rendering::Resources::Mesh*mesh, const Maths::FMatrix4& matrix , const Eigen::Vector4<uint8_t>& c);
		void addModel(Rendering::Resources::Model*model,const Maths::FMatrix4& matrix,const Eigen::Vector4<uint8_t>& c);
		void initGpuBuffer();
		int hit(const Eigen::Matrix4f& viewProj, float u, float v);
	};
	Polygon& ViewCube();
	Polygon& ViewAxis();
	Polygon& GizmoAxis();
}