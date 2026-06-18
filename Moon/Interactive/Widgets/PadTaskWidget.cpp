#include "Interactive/Widgets/PadTaskWidget.h"
#include "Interactive/Im3DRenderer.h"
#include "Interactive/MathUtil/MathUtil.h"

namespace MOON {
	class PadTaskWidget::Internal {
	public:
		Internal(PadTaskWidget*s):self(s){}
		~Internal(){}

	private:
		friend PadTaskWidget;
		PadTaskWidget* self = nullptr;
		Eigen::Vector3f center = {0,0,0};
		Eigen::Vector3f normal = { 1,0,0 };
		Eigen::Vector3f originCenter = {0,0,0};
	};
	PadTaskWidget::PadTaskWidget(const std::string& name):EventWidget(name), mInternal(new Internal(this))
	{
	}
	PadTaskWidget::~PadTaskWidget()
	{
		delete mInternal;
	}
	void PadTaskWidget::onUpdate()
	{
		mPreflag = mCurflag;
		renderer->drawPoint(mInternal->center, 30);
		mCurflag=renderer->gizmoAxisTranslationBehavior(renderer->makeId("pad"),
			mInternal->center,mInternal->normal,0, 20.0, 10.0,&mInternal->center);
		renderer->gizmoAxisTranslationDraw(renderer->makeId("pad"),
			mInternal->center, mInternal->normal, 20.0, 10.0, {255,255,255,0});
		if (mPreflag && !mCurflag) {
			this->InvokeEvent(PadTaskEvent::LengthChange);
		}
	}
	void PadTaskWidget::setUpOrigin(float x, float y, float z)
	{
		mInternal->center = Eigen::Vector3f(x, y, z);
		mInternal->originCenter = mInternal->center;
	}
	void PadTaskWidget::setUpDir(float x, float y, float z)
	{
		mInternal->normal = Eigen::Vector3f(x,y,z);
	}
	float PadTaskWidget::getLength()
	{
		return (mInternal->originCenter - mInternal->center).norm();
	}
	void PadTaskWidget::setLength(float len)
	{
		mInternal->center = len * mInternal->normal + mInternal->originCenter;
	}
}