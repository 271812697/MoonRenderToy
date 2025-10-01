#include "CameraReaction.h"


namespace MOON {
	CameraReaction::CameraReaction(QAction* parent, Mode mode) :Superclass(parent)
	{
		this->ReactionMode = mode;
		this->updateEnableState();
	}
	void CameraReaction::resetPositiveX()
	{
	}
	void CameraReaction::resetPositiveY()
	{
	}
	void CameraReaction::resetPositiveZ()
	{
	}
	void CameraReaction::resetNegativeX()
	{
	}
	void CameraReaction::resetNegativeY()
	{
	}
	void CameraReaction::resetNegativeZ()
	{
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
			break;
		case ZOOM_TO_DATA:
			break;
		case ROTATE_CAMERA_CW:
			break;
		case ROTATE_CAMERA_CCW:
			break;
		}
	}
}




