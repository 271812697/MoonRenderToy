#include <tinyxml2.h>
#include <Core/ECS/Actor.h>
#include <Core/ECS/Components/CBatchMeshLine.h>
#include <Core/ECS/Components/CModelRenderer.h>
#include <Core/Global/ServiceLocator.h>
#include <Core/ResourceManagement/MaterialManager.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Rendering/Geometry/bvh.h>
#include <Rendering/Geometry/split_bvh.h>

namespace Core::ECS::Components
{
	class CBatchMeshLine::CBatchMeshLineInternal {
	public:
		CBatchMeshLineInternal(CBatchMeshLine* self) :mSelf(self){

		}
		~CBatchMeshLineInternal() {
			delete rootBvh;
			for (int i = 0; i < subMeshBvhs.size(); i++) {
				delete subMeshBvhs[i];
			}
		}
	private:
		friend class CBatchMeshLine;
		CBatchMeshLine* mSelf = nullptr;
		bool colorChange = false;
		int hoverIndex =-1;
		std::vector<int> candidatesIndex;
		Maths::FVector4 candidateColor;
		Maths::FVector4 hoverColor;

		std::vector<Maths::FVector4> m_defaultColors;
		::Rendering::Geometry::Bvh* rootBvh = nullptr;
		std::vector<::Rendering::Geometry::SplitBvh*>subMeshBvhs;
		std::vector<uint32_t> subMeshRanges;
	};
	CBatchMeshLine::CBatchMeshLine(ECS::Actor& p_owner) : AComponent(p_owner),mInternal(new CBatchMeshLineInternal(this))
	{
	}

	CBatchMeshLine::~CBatchMeshLine()
	{
		delete mInternal;
	}

	std::string CBatchMeshLine::GetName()
	{
		return "CBatchMesh";
	}

	void CBatchMeshLine::OnUpdate(float p_deltaTime)
	{
		if (mInternal->colorChange) {
			mInternal->colorChange = false;
			auto mat = owner.GetComponent<CMaterialRenderer>()->GetMaterialAtIndex(0);
			if (mat) {
				const ::Rendering::Data::MaterialProperty prop = mat->GetProperty("lineColorTex").value();
				::Rendering::HAL::GLTexture* triangleInfoTex = nullptr;
				auto tex = std::get<::Rendering::HAL::TextureHandle*>(prop.value);

				if (tex) {
					std::vector<Maths::FVector4> colors = mInternal->m_defaultColors;
	
					for (auto& idx : mInternal->candidatesIndex) {
						colors[idx] = mInternal->candidateColor;
					}
					if (mInternal->hoverIndex != -1) {
						colors[mInternal->hoverIndex] = mInternal->hoverColor;
					}
					::Rendering::Settings::TextureDesc desc;
					desc.isTextureBuffer = true;
					desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA32F;
					desc.buffetLen = colors.size() * sizeof(Maths::FVector4);
					desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
						.data = colors.data()
					};
					static_cast<::Rendering::HAL::GLTexture*>(tex)->Allocate(desc);
				}
			}
		}
	}

	void CBatchMeshLine::SetColors(const std::vector<Maths::FVector4>& colors)
	{
		mInternal->m_defaultColors = colors;
	}

	void CBatchMeshLine::SetColor(const std::vector<int>& index, const Maths::FVector4& color)
	{
		//mInternal->candidatesIndex.clear();;
		mInternal->candidatesIndex = index;
		mInternal->colorChange = true;
		mInternal->candidateColor = color;
		//for (int i = 0; i < index.size(); i++) {
		//	int idx = index[i];
		//	if (idx >= 0 && idx < mInternal->m_defaultColors.size()) {
		//		mInternal->colorChange = true;
		//		mInternal->candidatesIndex.push_back(index);
		//	}
		//}
	}

	void CBatchMeshLine::SetHoverColor(int index, const Maths::FVector4& color)
	{
		mInternal->hoverIndex = index;
		mInternal->hoverColor = color;
		mInternal->colorChange = true;
	}

	std::vector<Maths::FVector3> CBatchMeshLine::getLineSeg(int index)
	{
		std::vector<Maths::FVector3>res;
		auto model = owner.GetComponent<Core::ECS::Components::CModelRenderer>();
		auto mesh = model->GetModel()->GetMeshes()[0];
		auto& indices = mesh->GetIndices();
		auto& vertex = mesh->GetVerticesBVH();
		uint32_t i1=mInternal->subMeshRanges[index];
		uint32_t i2 = mInternal->subMeshRanges[index+1];
		for (int i = i1;i < i2;i += 2) {
			res.push_back(vertex[indices[i]].position);
			res.push_back(vertex[indices[i+1]].position);
		}

		return res;
	}

	void CBatchMeshLine::BuildBvh( const std::vector<uint32_t>& subMeshRanges)
	{
		mInternal->rootBvh = new ::Rendering::Geometry::Bvh(10.0f, 64, false);
		std::vector<::Rendering::Geometry::bbox> boxs;
		//mInternal->rootBvh->Build(boxs.data(),boxs.size());//::Rendering::Geometry::Bvh rootBvh;
		auto model=owner.GetComponent<Core::ECS::Components::CModelRenderer>();
		auto mesh=model->GetModel()->GetMeshes()[0];
		auto& indices =mesh->GetIndices();
		auto& vertex=mesh->GetVerticesBVH();
		uint32_t ioffset = 0;
		boxs.reserve(subMeshRanges.size());
		for (int i = 0; i < subMeshRanges.size(); i++) {
			int numLines = (subMeshRanges[i] - ioffset) / 2;
			auto bvh = new ::Rendering::Geometry::SplitBvh(2.0f, 64, 0, 0.001f, 0);
			//为所有的线段构建包围盒，然后在对所有的包围盒构建bvh
			std::vector<::Rendering::Geometry::bbox> bounds(numLines);
			for (int k = 0; k < numLines; k++)
			{

				bounds[k].grow(vertex[indices[2 * k+ ioffset]].position);
				bounds[k].grow(vertex[indices[2 * k + 1 + ioffset]].position);
				
			}
			
			bvh->Build(&bounds[0], numLines);
			boxs.emplace_back(bvh->m_bounds);
			mInternal->subMeshBvhs.push_back(bvh);
			ioffset = subMeshRanges[i];
		}
		mInternal->subMeshRanges.resize(subMeshRanges.size());
		for (int i = 0; i < subMeshRanges.size(); i++) {
			if (i == 0) {
				mInternal->subMeshRanges[i] = 0;
			}
			else
			{
				mInternal->subMeshRanges[i] = subMeshRanges[i-1];
			}
		}
		mInternal->rootBvh->Build(boxs.data(), boxs.size());
	}

	bool CBatchMeshLine::PointPick(const Maths::FMatrix4& viewPortMatrix, int x, int y, float tolerance, Core::SceneSystem::PointPickRes& out)
	{
		bool res = false;
		float pixelDis = 100;
		auto model = owner.GetComponent<Core::ECS::Components::CModelRenderer>();
		int actorId = owner.GetID();
		auto mesh = model->GetModel()->GetMeshes()[0];
		auto matrix = owner.GetComponent<Core::ECS::Components::CTransform>()->GetWorldMatrix();
		auto& indices = mesh->GetIndices();
		auto& vertex = mesh->GetVerticesBVH();
		std::vector<::Rendering::Geometry::Bvh::Node*>stack;
		stack.push_back(mInternal->rootBvh->m_root);
		while (!stack.empty()) {
			auto cur = stack.back(); stack.pop_back();
			if (!cur)continue;
			auto tbox = cur->bounds.transform(viewPortMatrix);
			bool flag1 = (x >= (tbox.pmin.x - tolerance)) && (x <= (tbox.pmax.x + tolerance));
			bool flag2 = (y >= (tbox.pmin.y - tolerance)) && (y <= (tbox.pmax.y + tolerance));
			if (flag1 && flag2) {
				if (cur->type == ::Rendering::Geometry::Bvh::kInternal) {
					stack.push_back(cur->lc);
					stack.push_back(cur->rc);
				}
				else if (cur->type == ::Rendering::Geometry::Bvh::kLeaf) {
					for (int i = cur->startidx; i < cur->startidx + cur->numprims; i++) {
						int index = mInternal->rootBvh->m_packed_indices[i];
						uint32_t ioffset = mInternal->subMeshRanges[index];
						auto meshBvh = mInternal->subMeshBvhs[index];
						std::vector<::Rendering::Geometry::Bvh::Node*>meshBvhStack;
						meshBvhStack.push_back(meshBvh->m_root);
						bool isHit = false;
						while (!meshBvhStack.empty()) {
							auto meshBvhCur = meshBvhStack.back(); meshBvhStack.pop_back();
							if (!meshBvhCur)continue;
							auto mbox = meshBvhCur->bounds.transform(matrix).transform(viewPortMatrix);
							bool mflag1 = (x >= (mbox.pmin.x - tolerance)) && (x <= (mbox.pmax.x + tolerance));
							bool mflag2 = (y >= (mbox.pmin.y - tolerance)) && (y <= (mbox.pmax.y + tolerance));
							if (mflag1 && mflag2) {
								if (meshBvhCur->type == ::Rendering::Geometry::Bvh::kInternal) {
									meshBvhStack.push_back(meshBvhCur->lc);
									meshBvhStack.push_back(meshBvhCur->rc);
								}
								else if (meshBvhCur->type == ::Rendering::Geometry::Bvh::kLeaf) {
									for (int j = meshBvhCur->startidx; j < meshBvhCur->startidx + meshBvhCur->numprims; j++) {
										int segIndex = meshBvh->m_packed_indices[j];
										::Rendering::Geometry::VertexBVH v0 = vertex[indices[segIndex * 2 + ioffset]];// mesh->GetVertexBVH(segIndex * 2+ioffset);
										::Rendering::Geometry::VertexBVH v1 = vertex[indices[segIndex * 2 + 1 + ioffset]];
										Maths::FVector3 ndcV0 = viewPortMatrix.MulPoint(matrix.MulPoint(v0.position));
										Maths::FVector3 ndcV1 = viewPortMatrix.MulPoint(matrix.MulPoint(v1.position));
										float dis= ::Core::SceneSystem::pointToSegmentDistance({ x*1.0f,y*1.0f }, { ndcV0.x,ndcV0.y }, { ndcV1.x,ndcV1.y });
										if (dis<tolerance&&dis<pixelDis) {
											pixelDis = dis;
											isHit = true;
											res = true;
											out.actorId = actorId;
											out.subMeshId= (int)v0.texCoords.x;
											break;
										}
									}
								}
							}
							if (isHit) {
								break;
							}
						}
					}
				}
			}
		}
		return res;
	}

	void CBatchMeshLine::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
	{
	}

	void CBatchMeshLine::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
	{
	}
}
