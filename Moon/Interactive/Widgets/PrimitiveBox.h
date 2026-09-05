#pragma once
#include "Interactive/Widgets/PrimitiveShape.h"

namespace MOON
{
	class PrimitiveBox : public PrimitiveShape
	{
	public:
		PrimitiveBox(const std::string& name);
		virtual ~PrimitiveBox()override;
		virtual void onUpdate()override;
		virtual void onLeftMousePressed()override;
		virtual void onLeftMouseReleased()override;
		virtual void onMouseMove()override;
		virtual void createTopoShape()override;

	private:
		enum class BoxPart
		{
			None,
			Center,
			FacePlusX,
			FaceMinusX,
			FacePlusY,
			FaceMinusY,
			FacePlusZ,
			FaceMinusZ,
			Body
		};
		enum class DragOp
		{
			None,
			Translate,
			ScaleFace,
			Rotate
		};

		void drawGizmo();
		void updateHover();
		void beginDrag();
		void updateDrag();
		Eigen::Vector3f faceCenter(BoxPart part) const;

		BoxPart m_hover = BoxPart::None;
		DragOp m_dragOp = DragOp::None;
		BoxPart m_dragPart = BoxPart::None;
		int m_dragAxis = 0;      // 0=x, 1=y, 2=z for face drags
		float m_dragSign = 1.0f; // which side of the center the face is on
		Eigen::Vector3f m_dragNormal;
		float m_startAxisT = 0.0f;
		Eigen::Vector3f m_startTranslation;
		Eigen::Matrix3f m_startRotation;
		Eigen::Vector3f m_startScale;
		Eigen::Vector3f m_planeHitStart;
		Eigen::Vector2f m_cursorDown;
		Eigen::Vector3f m_rotateAxis = Eigen::Vector3f(0, 0, 0);

		Eigen::Vector3f translation;
		Eigen::Vector3f scale;
		Eigen::Matrix3f rot;
	};
}
