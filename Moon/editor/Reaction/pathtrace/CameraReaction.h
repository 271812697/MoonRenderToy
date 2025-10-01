#pragma once
#include "editor/Reaction/pqReaction.h"
namespace MOON {
	class  CameraReaction : public pqReaction
	{
		Q_OBJECT
			typedef pqReaction Superclass;
	public:
		enum Mode
		{
			RESET_CAMERA,
			RESET_POSITIVE_X,
			RESET_POSITIVE_Y,
			RESET_POSITIVE_Z,
			RESET_NEGATIVE_X,
			RESET_NEGATIVE_Y,
			RESET_NEGATIVE_Z,
			APPLY_ISOMETRIC_VIEW,
			ZOOM_TO_DATA,
			ROTATE_CAMERA_CW,
			ROTATE_CAMERA_CCW,
		};
		CameraReaction(QAction* parent, Mode mode);
		void resetPositiveX();
		void resetPositiveY();
		void resetPositiveZ();
		void resetNegativeX();
		void resetNegativeY();
		void resetNegativeZ();
	public Q_SLOTS: // NOLINT(readability-redundant-access-specifiers)
		void updateEnableState() override;
	protected:
		void onTriggered() override;
	private:
		Q_DISABLE_COPY(CameraReaction)
			Mode ReactionMode;
	};

}

