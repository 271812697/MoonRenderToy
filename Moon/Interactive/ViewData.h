#pragma once 
#include "Interactive/MathUtil/MathUtil.h"
namespace MOON {
	struct TriangleFace
	{
		TriangleFace(std::vector<Eigen::Vector3f>f,
			Eigen::Vector4<uint8_t> c) :faces(f), color(c) {

		}
		TriangleFace(std::vector<Eigen::Vector3f>f,
			Eigen::Vector4<uint8_t> c, const Eigen::Matrix4f& m) :model(m), faces(f), color(c) {

		}
		std::vector<Eigen::Vector3f>faces;
		Eigen::Vector4<uint8_t> color;
		Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
	};
	struct VertexPoint
	{
		VertexPoint(const Eigen::Vector3f& p, float s, const Eigen::Vector4<uint8_t>& c) :pos(p), size(s), color(c)
		{

		}
		Eigen::Vector3f pos;
		float size;
		Eigen::Vector4<uint8_t> color;
	};
	struct Edge {
		Edge(const std::vector<Eigen::Vector3f>& l, const Eigen::Vector4<uint8_t>& c) :lines(l), color(c) {}
		Edge(const std::vector<Eigen::Vector3f>& l, const Eigen::Vector4<uint8_t>& c, const Eigen::Matrix4f& m) :model(m), lines(l), color(c) {}
		std::vector<Eigen::Vector3f>lines;
		Eigen::Vector4<uint8_t> color;
		Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
	};
	class  WidgetViewData {
	public:
		WidgetViewData() = default;
		~WidgetViewData() {

		}
		int addPoint(const Eigen::Vector3f& pos, float size, const Eigen::Vector4<uint8_t>& color);
		int addPoint(const std::vector<Eigen::Vector3f>& pos, float size, const Eigen::Vector4<uint8_t>& color);
		int addEdge(const std::vector<Eigen::Vector3f>& l, const Eigen::Vector4<uint8_t>& c, const Eigen::Matrix4f& m = Eigen::Matrix4f::Identity());
		int addTriangleFace(const std::vector<Eigen::Vector3f>& f, const Eigen::Vector4<uint8_t>& c, const Eigen::Matrix4f& m = Eigen::Matrix4f::Identity());
		void setPoint(int id, const Eigen::Vector4<uint8_t>& color);
		void setPoint(int id, float size);
		void setPoint(int id, const Eigen::Vector3f& pos);
		void setEdge(const std::string& name, const std::vector<Eigen::Vector3f>& l, const Eigen::Vector4<uint8_t>& c, const Eigen::Matrix4f& m = Eigen::Matrix4f::Identity());
		void setEdge(const std::string& name, const std::vector<Eigen::Vector3f>& l);
		void setEdge(const std::string& name, const Eigen::Vector4<uint8_t>& c);
		void setEdge(const std::string& name, const Eigen::Matrix4f& m);
		void setTriangleFace(const std::string& name, const std::vector<Eigen::Vector3f>& f, const Eigen::Vector4<uint8_t>& c, const Eigen::Matrix4f& m = Eigen::Matrix4f::Identity());
		void setTriangleFace(const std::string& name, const Eigen::Matrix4f& m);
		void setTriangleFace(const std::string& name, const std::vector<Eigen::Vector3f>& f);
		void setTriangleFace(const std::string& name, const Eigen::Vector4<uint8_t>& c);
		void setTriangleFaceScale(const std::string& name,float scale);
		std::vector<Edge>& getEdge();
		std::vector<VertexPoint>& getPoints();
		std::vector<TriangleFace>& getFaces();
		bool getTriangleFace(const std::string& name, TriangleFace*& f);
		bool getEdgeLine(const std::string& name, Edge*& e);
		void clearPoints();
		void clearLines();
		void clearFaces();
		void clear();
		std::string hitFace(const Ray& ray, float scale = 1.0f);
		std::string hitEdge(const Eigen::Matrix4f& viewPortMat, const Eigen::Vector2f& pos);
		int hitPoint(const Eigen::Matrix4f& viewPortMat, const Eigen::Vector2f& pos);
	private:
		float pointToSegmentDist(const Eigen::Vector2f& p, const Eigen::Vector2f& s, const Eigen::Vector2f& e);
	private:
		std::vector<Edge>lines;
		std::vector<VertexPoint>points;
		std::vector<TriangleFace>faces;
		std::unordered_map<std::string, int>facesIdMap;
		std::unordered_map<int, std::string>facesStrMap;
		std::unordered_map<std::string, int>edgesIdMap;
		std::unordered_map<int, std::string>edgesStrMap;
	};

}