#include "Gizmo/GizmoType.h"
#include "Gizmo/MathUtil/MathUtil.h"
#include "Core/Global/ServiceLocator.h"
#include "Core/ResourceManagement/TextureManager.h"
#include "Rendering/Resources/Texture.h"
#include "Rendering/Resources/Mesh.h"
#include "Rendering/Resources/Model.h"
#include "renderer/Context.h"
#include <glad/glad.h>
namespace MOON {
	void Cell::clear() {
		vertex.clear();
		uv.clear();
	}
	Cell::Cell(const Eigen::Vector3f& v0, const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, const Eigen::Vector4<uint8_t>& c)
	{
		color = c;
		addPoint(v0,{0,0});
		addPoint(v1, { 0,0 });
		addPoint(v2, { 0,0 });
		n = (v1 - v0).cross(v2-v0).normalized();
	}
	void Cell::addPoint(const Eigen::Vector3f& v, const Eigen::Vector2f& tex)
	{
		vertex.push_back(v);
		uv.push_back(tex);
	}
	void Cell::addPointArray(const std::vector<Eigen::Vector3f>& v, const std::vector<Eigen::Vector2f>& tex)
	{
		vertex = v;
		uv = tex;
	}

	Cell Cell::transform(const Eigen::Matrix4f& mat, float offsetX, float offsetY)
	{
		Cell res;
		for (int i = 0; i < vertex.size();i++) {
			res.addPoint(MatrixMulPoint(mat, vertex[i]), { uv[i].x() + offsetX,uv[i].y() + offsetY });
		}
		res.n = MatrixMulDir(mat, n);
		return res;
	}
	void Cell::tranformUV(float u, float v)
	{
		for (int i = 0; i < vertex.size(); i++) {
			uv[i].x() += u;
			uv[i].y() += v;
		}
	}
	void Polygon::setCellColor(int index, const Eigen::Vector4<uint8_t>& color)
	{
		for (int i = 0;i < cellArray.size();i++) {
			if (i == index) {
				cellArray[index].color = color;
			}
			else
			{
				cellArray[i].color = { 255,255,255,255 };
			}
		}
		isDirty = true;
	}
	void Polygon::submit()
	{
		std::vector<VertexData> vData;
		edgeValue.clear();
		for (int i = 0;i < cellArray.size();i++) {
			Cell& cell = cellArray[i];
			for (int j = 2;j < cell.vertex.size();j++) {
				VertexData vd1(cell.vertex[0], cell.color, cell.n, cell.uv[0]);
				VertexData vd2(cell.vertex[j - 1], cell.color, cell.n, cell.uv[j - 1]);
				VertexData vd3(cell.vertex[j], cell.color, cell.n, cell.uv[j]);
				vData.push_back(vd1);
				vData.push_back(vd2);
				vData.push_back(vd3);
				if (drawEdge) {
					if (j == 2) {
						edgeValue.push_back(cell.vertex.size()==3?7:6);
					}
					else if (j == cell.vertex.size() - 1) {
						edgeValue.push_back( 3);
					}
					else
					{
						edgeValue.push_back( 2);
					}
				}
				else
				{
					edgeValue.push_back(0);
				}
			}
		}
		numVertex = vData.size();

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vData.size() * sizeof(VertexData), (GLvoid*)vData.data(), GL_STREAM_DRAW);
		
		//
		Rendering::Settings::TextureDesc desc;
		desc.isTextureBuffer = true;
		desc.internalFormat = Rendering::Settings::EInternalFormat::R8UI;
		desc.buffetLen = edgeValue.size() * sizeof(uint8_t);
		desc.mutableDesc= Rendering::Settings::MutableTextureDesc{
				.data=edgeValue.data()
		};
		edgeTexture = new Rendering::Resources::Texture();;//new  OvRendering::HAL::GLTexture(OvRendering::Settings::ETextureType::TEXTURE_BUFFER, "tbo");
		auto gltexture=new Rendering::HAL::GLTexture(Rendering::Settings::ETextureType::TEXTURE_BUFFER);
		gltexture->Allocate(desc);
		edgeTexture->SetTexture(std::unique_ptr<Rendering::HAL::Texture>(gltexture));
	}
	void Polygon::bind()
	{
		if (isDirty) {
			isDirty = false;
			submit();
		}
		glBindVertexArray(vao);
	}
	Eigen::Vector3f Polygon::getCellNormal(int index)
	{
		return cellArray[index].n;
	}
	Polygon::~Polygon()
	{
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}
	void Polygon::addMesh(Rendering::Resources::Mesh* mesh,const Maths::FMatrix4& matrix, const Eigen::Vector4<uint8_t>& c)
	{
		int vcnt=mesh->GetVertexCount();
		int icnt = mesh->GetIndexCount();
		int num = icnt > 0 ? icnt : vcnt;
		for (int i = 0; i < num;i+=3) {
			auto v0 = Maths::FMatrix4::MulPoint(matrix,mesh->GetVertexPosition(i));
			auto v1 = Maths::FMatrix4::MulPoint(matrix, mesh->GetVertexPosition(i+1));
			auto v2 = Maths::FMatrix4::MulPoint(matrix, mesh->GetVertexPosition(i+2));
			cellArray.push_back(Cell({ v0.x,v0.y,v0.z }, { v1.x,v1.y,v1.z }, { v2.x,v2.y,v2.z },c));
		}
	}
	void Polygon::addModel(Rendering::Resources::Model* model, const Maths::FMatrix4& matrix, const Eigen::Vector4<uint8_t>& c)
	{
		for (auto m:model->GetMeshes()) {
			addMesh(m,matrix,c);
		}
	}
	void Polygon::initGpuBuffer()
	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, positionSize));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, color));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, normal));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, uv));
		submit();
		glBindVertexArray(0);
	}
	int Polygon::hit(const Eigen::Matrix4f& viewProj, float u, float v)
	{
		int res = -1;
		float minDist = 100000.0f;
		Ray ray;
		ray.m_origin = { u,v,-2 };
		ray.m_direction = { 0,0,1.0 };
		for (int i = 0; i < cellArray.size(); i++) {
			Cell& cell = cellArray[i];
			std::vector<Eigen::Vector3f>ndcPos(cell.vertex.size());
			for (int j = 0;j < cell.vertex.size();j++) {
				ndcPos[j] = MatrixMulPoint(viewProj, cell.vertex[j]);
			}
			for (int j = 2; j < cell.vertex.size(); j++) {
				float tr;
				if (Intersect(ray, ndcPos[0], ndcPos[j - 1], ndcPos[j], tr)) {
					if (tr < minDist) {
						minDist = tr;
						res = i;
					}
				}
			}
		}
		return res;
	}
	Polygon& ViewCube()
	{
		static Polygon viewCube;
		if (viewCube.cellArray.size() == 0) {
			float halflen = 3.0f;
			float shift = 0.6f;
			float ratio = 0.5 * shift / halflen;
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
			Eigen::Vector3f F7 = { halflen - 2 * shift, halflen , halflen - shift };
			Eigen::Vector3f F8 = { -halflen + 2 * shift, halflen , halflen - shift };
			Eigen::Vector3f F9 = { halflen , -halflen + 2 * shift, halflen - shift };
			Eigen::Vector3f F10 = { halflen, halflen - 2 * shift, halflen - shift };
			Eigen::Vector3f F11 = { -halflen + 2 * shift, -halflen , halflen - shift };
			Eigen::Vector3f F12 = { halflen - 2 * shift, -halflen , halflen - shift };

			Cell cell;
			cell.addPoint(A1, { (2 * ratio) / 3,(1 - ratio) / 2 });
			cell.addPoint(A2, { (ratio) / 3,(1 - 2 * ratio) / 2 });
			cell.addPoint(D2, { (ratio) / 3,(2 * ratio) / 2 });
			cell.addPoint(D1, { (2 * ratio) / 3,(ratio) / 2 });
			cell.addPoint(C1, { (1 - 2 * ratio) / 3,(ratio) / 2 });
			cell.addPoint(C2, { (1 - ratio) / 3,(2 * ratio) / 2 });
			cell.addPoint(B2, { (1 - ratio) / 3,(1 - 2 * ratio) / 2 });
			cell.addPoint(B1, { (1 - 2 * ratio) / 3,(1 - ratio) / 2 });
			cell.n = n;
			cell.tranformUV(2.0 / 3, 0.0);
			auto& cellArr = viewCube.cellArray;
			cellArr.push_back(cell);
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 90, 0, 0 }), -1.0 / 3.0, 0.5));
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 180, 0, 0 }), 0.0, 0.5));
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 270, 0, 0 }), -1.0 / 3.0, 0));
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 0, 90, 0 }), -2.0 / 3.0, 0.0));
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 0, 270, 0 }), -2.0 / 3.0, 0.5));

			cell.clear();
			cell.n = Eigen::Vector3f(1, 1, 1).normalized();
			cell.addPointArray({ F1,F2,F3,F4,F5,F6 }, { { 0, 0 }, { 0,0 }, { 0,0 }, { 0,0 }, { 0,0 }, { 0,0 } });
			cellArr.push_back(cell);
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 0, 90, 0 })));
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 0, 180, 0 })));
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 0, 270, 0 })));
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 90, 0, 0 })));
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 180, 0, 0 })));
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 90, -90, 0 })));
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 90, -180, 0 })));
			cell.clear();
			cell.n = Eigen::Vector3f(0, 1, 1).normalized();
			cell.addPointArray({ A1,B1,F7,F8 }, { {0,0},{0,0 },{0,0} ,{0,0} });
			cellArr.push_back(cell);
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 0, 90, 0 })));
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 0, 180, 0 })));
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 0, 270, 0 })));
			cell.clear();
			cell.n = Eigen::Vector3f(1, 0, 1).normalized();
			cell.addPointArray({ C2,F9,F10,B2 }, { { 0, 0 }, { 0,0 }, { 0,0 }, { 0,0 } });
			cellArr.push_back(cell);
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 0, 90, 0 })));
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 0, 180, 0 })));
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 0, 270, 0 })));
			cell.clear();
			cell.n = Eigen::Vector3f(0, -1, 1).normalized();
			cell.addPointArray({ D1,F11,F12,C1 }, { { 0, 0 }, { 0,0 }, { 0,0 }, { 0,0 } });
			cellArr.push_back(cell);
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 0, 90, 0 })));
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 0, 180, 0 })));
			cellArr.push_back(cell.transform(EulerXYZToMatrix4Degree({ 0, 270, 0 })));			
			viewCube.initGpuBuffer();
			viewCube.texture = Core::Global::ServiceLocator::Get<Core::ResourceManagement::TextureManager>().GetResource(PROJECT_ENGINE_PATH"/Textures/XYZ.png", true);
		}
		return viewCube;
	}
	Polygon& ViewAxis()
	{
		static Polygon viewAxis;
		if (viewAxis.cellArray.size() == 0) {
			float halflen = 3.0f;
			Maths::FMatrix4 model =
				Maths::FMatrix4::Translation({ 0,0,0 }) *
				Maths::FMatrix4::Scaling({ 6,6,6 });
			viewAxis.drawEdge = false;
			
			auto arrow = GetService(Editor::Core::Context).editorResources->GetModel("Arrow_Translate");
			auto sphere= GetService(Core::ResourceManagement::ModelManager).LoadResource(":Models/Sphere.fbx");
			viewAxis.addModel(arrow, model, { 0,0,255,255 });
			viewAxis.addModel(arrow, model.RotateOnAxisY(-90), { 255,0,0,255 });
			viewAxis.addModel(arrow, model.RotateOnAxisX(-90), { 0,255,0,255 });
			viewAxis.addModel(sphere, Maths::FMatrix4::Translation({ 0,0,0 }) * Maths::FMatrix4::Scaling({ 0.5f,0.5f,0.5f }), { 255,255,0,255 });
			viewAxis.model = Eigen::Matrix4f::Identity();
			viewAxis.model(0, 3) = -halflen;
			viewAxis.model(1, 3) = -halflen;
			viewAxis.model(2, 3) = -halflen;
			viewAxis.initGpuBuffer();
			viewAxis.texture = Core::Global::ServiceLocator::Get<Core::ResourceManagement::TextureManager>().GetResource(PROJECT_ENGINE_PATH"/Textures/XYZ.png", true);
		}
		return viewAxis;
	}
	Polygon& GizmoAxis()
	{
		static Polygon poly;
		if (poly.cellArray.size() == 0) {
			float halflen = 3.0f;
			Maths::FMatrix4 model =
				Maths::FMatrix4::Translation({ 0,0,0 }) *
				Maths::FMatrix4::Scaling({ 6,6,6 });
			poly.drawEdge = false;

			auto arrow = GetService(Editor::Core::Context).editorResources->GetModel("Arrow_Translate");
			auto sphere = GetService(Core::ResourceManagement::ModelManager).LoadResource(":Models/Sphere.fbx");
			poly.addModel(arrow, model, { 0,0,255,255 });
			poly.addModel(arrow, model.RotateOnAxisY(-90), { 255,0,0,255 });
			poly.addModel(arrow, model.RotateOnAxisX(-90), { 0,255,0,255 });
			poly.addModel(sphere, Maths::FMatrix4::Translation({ 0,0,0 }) * Maths::FMatrix4::Scaling({ 0.5f,0.5f,0.5f }), { 255,255,0,255 });
			float quadlen = 3.0;
			float quadoffset = 1.0;
			//y axis
			Cell cell;
			cell.addPoint({ quadoffset,0,quadoffset }, { 0,0 });
			cell.addPoint({ quadoffset+quadlen,0,quadoffset  }, { 0,0 });
			cell.addPoint({ quadoffset + quadlen,0,quadoffset +quadlen }, { 0,0 });
			cell.addPoint({ quadoffset,0,quadoffset +quadlen }, { 0,0 });
			cell.n = {0,1,0};
			poly.cellArray.push_back(cell);
			
			//x axis
			cell.clear();
			cell.addPoint({ 0,quadoffset,quadoffset }, { 0,0 });
			cell.addPoint({ 0,quadoffset + quadlen,quadoffset  }, { 0,0 });
			cell.addPoint({ 0,quadoffset + quadlen,quadoffset + quadlen }, { 0,0 });
			cell.addPoint({ 0,quadoffset ,quadoffset + quadlen }, { 0,0 });
			cell.n = { 1,0,0 };
			poly.cellArray.push_back(cell);

			//z axis
			cell.clear();
			cell.addPoint({ quadoffset,quadoffset,0 }, { 0,0 });
			cell.addPoint({ quadoffset + quadlen,quadoffset,0}, { 0,0 });
			cell.addPoint({ quadoffset + quadlen,quadoffset + quadlen,0 }, { 0,0 });
			cell.addPoint({ quadoffset ,quadoffset + quadlen,0}, { 0,0 });
			cell.n = { 0,0,1 };
			poly.cellArray.push_back(cell);

			poly.model = Eigen::Matrix4f::Identity();
		
			poly.initGpuBuffer();
			poly.texture = Core::Global::ServiceLocator::Get<Core::ResourceManagement::TextureManager>().GetResource(PROJECT_ENGINE_PATH"/Textures/XYZ.png", true);
		}
		return poly;
	}
}