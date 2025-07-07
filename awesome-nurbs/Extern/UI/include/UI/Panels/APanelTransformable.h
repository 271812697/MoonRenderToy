#pragma once

#include <vector>
#include <memory>
#include <Maths/FVector2.h>
#include "tools/Event.h"
#include "UI/Panels/APanel.h"
#include "UI/Settings/Alignment.h"

namespace UI::Panels
{

	class APanelTransformable : public APanel
	{
	public:

		APanelTransformable
		(
			const Maths::FVector2& p_defaultPosition = Maths::FVector2(-1.f, -1.f),
			const Maths::FVector2& p_defaultSize = Maths::FVector2(-1.f, -1.f),
			Settings::EHorizontalAlignment p_defaultHorizontalAlignment = Settings::EHorizontalAlignment::LEFT,
			Settings::EVerticalAlignment p_defaultVerticalAlignment = Settings::EVerticalAlignment::TOP,
			bool p_ignoreConfigFile = false
		);


		void SetPosition(const Maths::FVector2& p_position);


		void SetSize(const Maths::FVector2& p_size);


		void SetAlignment(Settings::EHorizontalAlignment p_horizontalAlignment, Settings::EVerticalAlignment p_verticalAligment);


		const Maths::FVector2& GetPosition() const;


		const Maths::FVector2& GetSize() const;

		Settings::EHorizontalAlignment GetHorizontalAlignment() const;


		Settings::EVerticalAlignment GetVerticalAlignment() const;

	protected:
		void Update();
		virtual void _Draw_Impl() = 0;

	private:
		Maths::FVector2 CalculatePositionAlignmentOffset(bool p_default = false);

		void UpdatePosition();
		void UpdateSize();
		void CopyImGuiPosition();
		void CopyImGuiSize();

	public:
		bool autoSize = true;

	protected:
		Maths::FVector2 m_defaultPosition;
		Maths::FVector2 m_defaultSize;
		Settings::EHorizontalAlignment m_defaultHorizontalAlignment;
		Settings::EVerticalAlignment m_defaultVerticalAlignment;
		bool m_ignoreConfigFile;

		Maths::FVector2 m_position = Maths::FVector2(0.0f, 0.0f);
		Maths::FVector2 m_size = Maths::FVector2(0.0f, 0.0f);

		bool m_positionChanged = false;
		bool m_sizeChanged = false;

		Settings::EHorizontalAlignment m_horizontalAlignment = Settings::EHorizontalAlignment::LEFT;
		Settings::EVerticalAlignment m_verticalAlignment = Settings::EVerticalAlignment::TOP;

		bool m_alignmentChanged = false;
		bool m_firstFrame = true;
	};
}