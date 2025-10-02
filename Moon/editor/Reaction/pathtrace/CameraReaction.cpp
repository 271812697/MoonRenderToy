#include "CameraReaction.h"
#include "pathtrace/PathTrace.h"
#include "pathtrace/Scene.h"
#include "pathtrace/Camera.h"

namespace MOON {
	CameraReaction::CameraReaction(QAction* parent, Mode mode) :Superclass(parent)
	{
		this->ReactionMode = mode;
		this->updateEnableState();
	}
	void CameraReaction::resetPositiveX()
	{
		auto& instance = PathTrace::PathTraceRender::instance();
		auto scene = instance.GetScene();
		int selectId = scene->getSelectInstanceId();

		if (selectId != -1) {
			auto camera = scene->getCamera();
			auto box = scene->getMeshInstanceBox(selectId);
			Vec3 center = box.center();
			Vec3 extent = box.extents();
			camera->lookAt(Vec3(center.x + Vec3::Length(extent), center.y, center.z), center);
			scene->setDirty(true);
		}
	}
	void CameraReaction::resetPositiveY()
	{
		auto& instance = PathTrace::PathTraceRender::instance();
		auto scene = instance.GetScene();
		int selectId = scene->getSelectInstanceId();

		if (selectId != -1) {
			auto camera = scene->getCamera();
			auto box = scene->getMeshInstanceBox(selectId);
			Vec3 center = box.center();
			Vec3 extent = box.extents();
			camera->lookAt(Vec3(center.x, center.y + Vec3::Length(extent), center.z), center);
			scene->setDirty(true);
		}
	}
	void CameraReaction::resetPositiveZ()
	{
		auto& instance = PathTrace::PathTraceRender::instance();
		auto scene = instance.GetScene();
		int selectId = scene->getSelectInstanceId();

		if (selectId != -1) {
			auto camera = scene->getCamera();
			auto box = scene->getMeshInstanceBox(selectId);
			Vec3 center = box.center();
			Vec3 extent = box.extents();
			camera->lookAt(Vec3(center.x, center.y, center.z + Vec3::Length(extent)), center);
			scene->setDirty(true);
		}
	}
	void CameraReaction::resetNegativeX()
	{
		auto& instance = PathTrace::PathTraceRender::instance();
		auto scene = instance.GetScene();
		int selectId = scene->getSelectInstanceId();

		if (selectId != -1) {
			auto camera = scene->getCamera();
			auto box = scene->getMeshInstanceBox(selectId);
			Vec3 center = box.center();
			Vec3 extent = box.extents();
			camera->lookAt(Vec3(center.x - Vec3::Length(extent), center.y, center.z), center);
			scene->setDirty(true);
		}
	}
	void CameraReaction::resetNegativeY()
	{
		auto& instance = PathTrace::PathTraceRender::instance();
		auto scene = instance.GetScene();
		int selectId = scene->getSelectInstanceId();

		if (selectId != -1) {
			auto camera = scene->getCamera();
			auto box = scene->getMeshInstanceBox(selectId);
			Vec3 center = box.center();
			Vec3 extent = box.extents();
			camera->lookAt(Vec3(center.x, center.y - Vec3::Length(extent), center.z), center);
			scene->setDirty(true);
		}
	}
	void CameraReaction::resetNegativeZ()
	{
		auto& instance = PathTrace::PathTraceRender::instance();
		auto scene = instance.GetScene();
		int selectId = scene->getSelectInstanceId();

		if (selectId != -1) {
			auto camera = scene->getCamera();
			auto box = scene->getMeshInstanceBox(selectId);
			Vec3 center = box.center();
			Vec3 extent = box.extents();
			camera->lookAt(Vec3(center.x, center.y, center.z - Vec3::Length(extent)), center);
			scene->setDirty(true);
		}
	}
	void CameraReaction::resetIsometriview()
	{
		auto& instance = PathTrace::PathTraceRender::instance();
		auto scene = instance.GetScene();
		int selectId = scene->getSelectInstanceId();

		if (selectId != -1) {
			auto camera = scene->getCamera();
			auto box = scene->getMeshInstanceBox(selectId);
			Vec3 center = box.center();
			Vec3 extent = box.extents();
			camera->lookAt(center + extent, center);
			scene->setDirty(true);
		}
	}
	void CameraReaction::resetZoomToSelect()
	{
		auto& instance = PathTrace::PathTraceRender::instance();
		auto scene = instance.GetScene();
		int selectId = scene->getSelectInstanceId();
		if (selectId != -1) {
			auto camera = scene->getCamera();
			auto box = scene->getMeshInstanceBox(selectId);
			auto forward = camera->getForward();
			Vec3 center = box.center();
			float extent = Vec3::Length(box.extents());
			camera->lookAt(center - extent * forward, center);
			scene->setDirty(true);
		}
	}
	void CameraReaction::updateEnableState() {

	}
	void CameraReaction::onTriggered()
	{
		switch (this->ReactionMode)
		{
		case RESET_CAMERA:

			break;
		case RESET_POSITIVE_X:
			this->resetPositiveX();
			break;
		case RESET_POSITIVE_Y:
			this->resetPositiveY();
			break;
		case RESET_POSITIVE_Z:
			this->resetPositiveZ();
			break;
		case RESET_NEGATIVE_X:
			this->resetNegativeX();
			break;
		case RESET_NEGATIVE_Y:
			this->resetNegativeY();
			break;
		case RESET_NEGATIVE_Z:
			this->resetNegativeZ();
			break;
		case APPLY_ISOMETRIC_VIEW:
			this->resetIsometriview();
			break;

		case ZOOM_TO_DATA:
			resetZoomToSelect();
			break;
		case ROTATE_CAMERA_CW:
			break;
		case ROTATE_CAMERA_CCW:
			break;
		}
	}
}




