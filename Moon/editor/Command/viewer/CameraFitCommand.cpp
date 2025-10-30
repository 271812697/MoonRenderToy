#include "CameraFitCommand.h"

#include "OvCore/Global/ServiceLocator.h"
#include "core/log.h"

#include "renderer/SceneView.h"
namespace MOON {
	CameraFitCommand::CameraFitCommand(QObject* parent, Mode mode) :Command(parent)
	{
		this->ReactionMode = mode;
		auto action = new QAction(this);
		setAction(action);
			
	}

	void CameraFitCommand::execute()
	{
		auto& view = GetService(OvEditor::Panels::SceneView);
	
		OvMaths::FVector3 dir = { 0,1,0 };
		switch (this->ReactionMode)
		{
		case RESET_CAMERA:

			break;
		case RESET_POSITIVE_X:
			dir = { 1,0,0 };
			break;
		case RESET_POSITIVE_Y:
			dir = { 0,1,0 };
			break;
		case RESET_POSITIVE_Z:
			dir = { 0,0,1 };
			break;
		case RESET_NEGATIVE_X:
			dir = { -1,0,0 };
			break;
		case RESET_NEGATIVE_Y:
			dir = { 0,-1,0 };
			break;
		case RESET_NEGATIVE_Z:
			dir = { 0,0,-1 };
			break;
		case APPLY_ISOMETRIC_VIEW:
			dir = { 1,1,1 };
			dir = OvMaths::FVector3::Normalize(dir);
			break;

		case ZOOM_TO_DATA:
			dir = view.GetCamera()->GetTransform().GetWorldForward();;
			break;
		case ROTATE_CAMERA_CW:
			break;
		case ROTATE_CAMERA_CCW:
			break;
		}
		view.FitToSelectedActor(dir);
	}
}




