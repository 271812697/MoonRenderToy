#include "Interactive/ViewData.h"
namespace MOON {
	int WidgetViewData::addPoint(const Eigen::Vector3f& pos, float size, const Eigen::Vector4<uint8_t>& color) {
		points.emplace_back(pos, size, color);
		return points.size();
	}
	int WidgetViewData::addPoint(const std::vector<Eigen::Vector3f>& pos, float size, const Eigen::Vector4<uint8_t>& color) {
		for (int i = 0; i < pos.size(); i++) {
			points.emplace_back(pos[i], size, color);
		}
		return points.size();
	}
	int WidgetViewData::addEdge(const std::vector<Eigen::Vector3f>& l, const Eigen::Vector4<uint8_t>& c, const Eigen::Matrix4f& m ) {
		lines.emplace_back(l, c, m);
		return lines.size();
	}
	int WidgetViewData::addTriangleFace(const std::vector<Eigen::Vector3f>& f, const Eigen::Vector4<uint8_t>& c, const Eigen::Matrix4f& m ) {
		faces.emplace_back(f, c, m);
		return faces.size();
	}
	void WidgetViewData::setPoint(int id, const Eigen::Vector4<uint8_t>& color) {
		if (id < points.size()) {
			points[id].color = color;
		}
	}
	void WidgetViewData::setPoint(int id, float size) {
		if (id < points.size()) {
			points[id].size = size;
		}
	}
	void WidgetViewData::setPoint(int id, const Eigen::Vector3f& pos) {
		if (id < points.size()) {
			points[id].pos = pos;
		}
	}
	void WidgetViewData::setEdge(const std::string& name, const std::vector<Eigen::Vector3f>& l, const Eigen::Vector4<uint8_t>& c, const Eigen::Matrix4f& m ) {
		auto it = edgesIdMap.find(name);
		if (it != edgesIdMap.end()) {
			int id = it->second;
			lines[id].lines = l;
			lines[id].color = c;
			lines[id].model = m;
			return;
		}
		int id = addEdge(l, c, m);
		edgesIdMap[name] = id - 1;
		edgesStrMap[id - 1] = name;
	}
	void WidgetViewData::setEdge(const std::string& name, const std::vector<Eigen::Vector3f>& l) {
		auto it = edgesIdMap.find(name);
		if (it != edgesIdMap.end()) {
			int id = it->second;
			lines[id].lines = l;
			return;
		}
	}
	void WidgetViewData::setEdge(const std::string& name, const Eigen::Vector4<uint8_t>& c) {
		auto it = edgesIdMap.find(name);
		if (it != edgesIdMap.end()) {
			int id = it->second;
			lines[id].color = c;
			return;
		}
	}
	void WidgetViewData::setEdge(const std::string& name, const Eigen::Matrix4f& m) {
		auto it = edgesIdMap.find(name);
		if (it != edgesIdMap.end()) {
			int id = it->second;
			lines[id].model = m;
			return;
		}
	}
	void WidgetViewData::setTriangleFace(const std::string& name, const std::vector<Eigen::Vector3f>& f, const Eigen::Vector4<uint8_t>& c, const Eigen::Matrix4f& m) {
		auto it = facesIdMap.find(name);
		if (it != facesIdMap.end()) {
			int id = it->second;
			faces[id].faces = f;
			faces[id].color = c;
			faces[id].model = m;
			return;
		}
		int id = addTriangleFace(f, c, m);
		facesIdMap[name] = id - 1;
		facesStrMap[id - 1] = name;
	}
	void WidgetViewData::setTriangleFace(const std::string& name, const Eigen::Matrix4f& m) {
		auto it = facesIdMap.find(name);
		if (it != facesIdMap.end()) {
			int id = it->second;
			faces[id].model = m;
		}
	}
	void WidgetViewData::setTriangleFace(const std::string& name, const std::vector<Eigen::Vector3f>& f) {
		auto it = facesIdMap.find(name);
		if (it != facesIdMap.end()) {
			int id = it->second;
			faces[id].faces = f;
		}
	}
	void WidgetViewData::setTriangleFace(const std::string& name, const Eigen::Vector4<uint8_t>& c) {
		auto it = facesIdMap.find(name);
		if (it != facesIdMap.end()) {
			int id = it->second;
			faces[id].color = c;
		}
	}
	std::vector<Edge>& WidgetViewData::getEdge() {
		return lines;
	}
	std::vector<VertexPoint>& WidgetViewData::getPoints() {
		return points;
	}
	std::vector<TriangleFace>& WidgetViewData::getFaces() {
		return faces;
	}
	bool WidgetViewData::getTriangleFace(const std::string& name, TriangleFace*& f) {
		auto it = facesIdMap.find(name);
		if (it != facesIdMap.end()) {
			f = &faces[it->second];
			return true;
		}
		return false;
	}
	bool WidgetViewData::getEdgeLine(const std::string& name, Edge*& e) {
		auto it = edgesIdMap.find(name);
		if (it != edgesIdMap.end()) {
			e = &lines[it->second];
			return true;
		}
		return false;
	}
	void WidgetViewData::clearPoints() {
		points.clear();
	}
	void WidgetViewData::clearLines() {
		lines.clear();
		edgesIdMap.clear();
		edgesStrMap.clear();
	}
	void WidgetViewData::clearFaces() {
		faces.clear();
		facesIdMap.clear();
		facesStrMap.clear();
	}
	void WidgetViewData::clear() {
		lines.clear();
		points.clear();
		faces.clear();
		facesIdMap.clear();
		edgesIdMap.clear();
		facesStrMap.clear();
		edgesStrMap.clear();
	}
	std::string WidgetViewData::hitFace(const Ray& ray) {
		int res = -1;
		float minDist = 100000.0f;
		for (int i = 0; i < faces.size(); i++) {
			std::vector<Eigen::Vector3f>& tris = faces[i].faces;
			Eigen::Matrix4f& mat = faces[i].model;
			for (int j = 0; j < tris.size(); j += 3) {
				float tr;
				if (Intersect(ray, MatrixMulPoint(mat, tris[j]), MatrixMulPoint(mat, tris[j + 1]), MatrixMulPoint(mat, tris[j + 2]), tr)) {
					if (tr < minDist) {
						minDist = tr;
						res = i;
					}
				}
			}
		}
		if (res == -1) {
			return "";
		}
		return facesStrMap[res];
	}
	std::string WidgetViewData::hitEdge(const Eigen::Matrix4f& viewPortMat, const Eigen::Vector2f& pos) {
		int res = -1;
		float minDist = 100000.0f;
		float error = 3;
		for (int i = 0; i < lines.size(); i++) {
			std::vector<Eigen::Vector3f>& edges = lines[i].lines;
			Eigen::Matrix4f& mat = lines[i].model;
			std::vector<Eigen::Vector3f>screenPos(edges.size());
			for (int j = 0; j < screenPos.size(); j++) {
				screenPos[j] = MatrixMulPoint(viewPortMat, MatrixMulPoint(mat, edges[j]));
			}
			for (int j = 0; j < screenPos.size() - 1; j++) {

				float curDis = pointToSegmentDist(pos, screenPos[j].head<2>(), screenPos[j + 1].head<2>());
				if (curDis < error && curDis < minDist) {
					minDist = curDis;
					res = i;
				}
			}
		}
		if (res == -1) {
			return "";
		}
		return edgesStrMap[res];
	}
	int WidgetViewData::hitPoint(const Eigen::Matrix4f& viewPortMat, const Eigen::Vector2f& pos) {
		int res = -1;
		float minDist = 100000.0f;
		for (int i = 0; i < points.size(); i++) {
			Eigen::Vector3f p = points[i].pos;
			float size = points[i].size / 2.0;
			Eigen::Vector2f screenPos = MatrixMulPoint(viewPortMat, p).head<2>();
			float curDis = (screenPos - pos).norm();
			if (curDis < size && curDis < minDist) {
				minDist = curDis;
				res = i;
			}
		}
		return res;
	}

	float WidgetViewData::pointToSegmentDist(const Eigen::Vector2f& p, const Eigen::Vector2f& s, const Eigen::Vector2f& e) {
		Eigen::Vector2f se = e - s;
		Eigen::Vector2f sp = p - s;
		float t = sp.dot(se) / se.dot(se);
		if (t < 0.0) {
			return sp.norm();
		}
		if (t > 1.0) {
			return (p - e).norm();
		}
		Eigen::Vector2f proj = s + t * se;
		return (p - proj).norm();
	};
}