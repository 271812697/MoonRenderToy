#include "Gizmo/Widgets/SplitScreen.h"
#include "Gizmo/Gizmo.h"
#include "Gizmo/MathUtil/MathUtil.h"
#include "renderer/SceneView.h"


namespace MOON {
	class SplitScreen::SplitScreenInternal {
	public:
		SplitScreenInternal(SplitScreen* clip):mSelf(clip) {
			id = mSelf->renderer->makeId("testline");
		}
		~SplitScreenInternal() {
		}
	private:
		friend class SplitScreen;
		SplitScreen* mSelf = nullptr;
		Eigen::Vector2f a = {100,100};
		Eigen::Vector2f b = { 600,600 };
		unsigned int id = 0;
	};

	SplitScreen::SplitScreen(const std::string& name) :GizmoWidget(name)
	, m_internal(new SplitScreenInternal(this)){
	
	}
	SplitScreen::~SplitScreen()
	{
		delete m_internal;
	}
	void SplitScreen::onUpdate()
	{
		renderer->drawLineSplit(m_internal->id, m_internal->a, m_internal->b);
	}
	void SplitScreen::getLineEquation(float* out)
	{
		Eigen::Vector2f a = { m_internal->a.x(),renderer->getCameraParam().viewportHeight - m_internal->a.y() };
		Eigen::Vector2f b = { m_internal->b.x(),renderer->getCameraParam().viewportHeight - m_internal->b.y() };
		Eigen::Vector2f v=b - a;
		v = v.normalized();
		v = { v.y(),-v.x() };
		
		out[0] = v.x();
		out[1] = v.y();
		out[2] =-a.dot(v);
	}
}