#include "renderer/InputState.h"
#include <QKeyEvent>
namespace OvEditor::Panels {

	OvEditor::Panels::InputState::InputState()
	{
	}
	KeyState OvEditor::Panels::InputState::GetKeyState(KeyBoard p_key)
	{
		return m_keyEvents[p_key];
	}

	MouseButtonState InputState::GetMouseButtonState(MouseButton p_button)
	{
		return m_mouseButtonEvents[p_button];
	}

	bool InputState::IsKeyPressed(KeyBoard p_key)
	{
		return m_keyEvents.find(p_key) != m_keyEvents.end() && m_keyEvents.at(p_key) == KeyState::Down;
	}

	bool InputState::IsKeyReleased(KeyBoard p_key)
	{
		return m_keyEvents.find(p_key) != m_keyEvents.end() && m_keyEvents.at(p_key) == KeyState::Up;
	}

	bool InputState::IsMouseButtonPressed(MouseButton p_button)
	{
		return m_mouseButtonEvents.find(p_button) != m_mouseButtonEvents.end() && m_mouseButtonEvents.at(p_button) == MouseButtonState::MOUSE_DOWN;
	}

	bool InputState::IsMouseButtonReleased(MouseButton p_button)
	{
		return m_mouseButtonEvents.find(p_button) != m_mouseButtonEvents.end() && m_mouseButtonEvents.at(p_button) == MouseButtonState::MOUSE_UP;
	}

	std::pair<double, double> InputState::GetMousePosition()
	{
		return std::pair<double, double>(mouseX, mouseY);
	}

	std::pair<double, double> InputState::GetMouseScroll()
	{
		return m_scrollData;
	}

	void InputState::ClearEvents()
	{
		//m_keyEvents.clear();
		//m_mouseButtonEvents.clear();
		m_scrollData = { 0.0, 0.0 };
	}

	void InputState::ReceiveEvent(QEvent* e)
	{
		const QEvent::Type t = e->type();
		if (t == QEvent::KeyPress) {
			QKeyEvent* e2 = static_cast<QKeyEvent*>(e);
			Qt::Key key = static_cast<Qt::Key>(e2->key());
			if (!e2->isAutoRepeat()) {
				switch (key)
				{
				case Qt::Key_W: { m_keyEvents[KEYW] = KeyState::Down; }break;
				case Qt::Key_A: { m_keyEvents[KEYA] = KeyState::Down; }break;
				case Qt::Key_S: { m_keyEvents[KEYS] = KeyState::Down; }break;
				case Qt::Key_D: { m_keyEvents[KEYD] = KeyState::Down; }break;
				case Qt::Key_Q: { m_keyEvents[KEYQ] = KeyState::Down; }break;
				case Qt::Key_E: { m_keyEvents[KEYE] = KeyState::Down; }break;
				case Qt::Key_R: { m_keyEvents[KEYR] = KeyState::Down; }break;
				case Qt::Key_L: { m_keyEvents[KEYL] = KeyState::Down; }break;
				case Qt::Key_T: { m_keyEvents[KEYT] = KeyState::Down; }break;
				case Qt::Key_F: { m_keyEvents[KEYF] = KeyState::Down; }break;
				case Qt::Key_Alt: { m_keyEvents[ALTA] = KeyState::Down; }break;
				case Qt::Key_Right: { m_keyEvents[RIGHT] = KeyState::Down; }break;
				case Qt::Key_Up: { m_keyEvents[UP] = KeyState::Down; }break;
				case Qt::Key_Down: { m_keyEvents[DOWN] = KeyState::Down; }break;
				case Qt::Key_Left: { m_keyEvents[LEFT] = KeyState::Down; }break;
				case Qt::Key_PageUp: { m_keyEvents[PageUp] = KeyState::Down; }break;
				case Qt::Key_PageDown: { m_keyEvents[PageDown] = KeyState::Down; }break;
				case Qt::Key_Control: { m_keyEvents[Control] = KeyState::Down; }break;
				case Qt::Key_Shift: { m_keyEvents[Shift] = KeyState::Down; }break;
				default:
					break;
				}
			}
		}
		else if (t == QEvent::KeyRelease) {
			QKeyEvent* e2 = static_cast<QKeyEvent*>(e);
			Qt::Key key = static_cast<Qt::Key>(e2->key());
			if (!e2->isAutoRepeat()) {
				switch (key)
				{
				case Qt::Key_W: { m_keyEvents[KEYW] = KeyState::Up; }break;
				case Qt::Key_A: { m_keyEvents[KEYA] = KeyState::Up; }break;
				case Qt::Key_S: { m_keyEvents[KEYS] = KeyState::Up; }break;
				case Qt::Key_D: { m_keyEvents[KEYD] = KeyState::Up; }break;
				case Qt::Key_Q: { m_keyEvents[KEYQ] = KeyState::Up; }break;
				case Qt::Key_E: { m_keyEvents[KEYE] = KeyState::Up; }break;
				case Qt::Key_R: { m_keyEvents[KEYR] = KeyState::Up; }break;
				case Qt::Key_L: { m_keyEvents[KEYL] = KeyState::Up; }break;
				case Qt::Key_T: { m_keyEvents[KEYT] = KeyState::Up; }break;
				case Qt::Key_F: { m_keyEvents[KEYF] = KeyState::Up; }break;
				case Qt::Key_Alt: { m_keyEvents[ALTA] = KeyState::Up; }break;
				case Qt::Key_Right: { m_keyEvents[RIGHT] = KeyState::Up; }break;
				case Qt::Key_Up: { m_keyEvents[UP] = KeyState::Up; }break;
				case Qt::Key_Down: { m_keyEvents[DOWN] = KeyState::Up; }break;
				case Qt::Key_Left: { m_keyEvents[LEFT] = KeyState::Up; }break;
				case Qt::Key_PageUp: { m_keyEvents[PageUp] = KeyState::Up; }break;
				case Qt::Key_PageDown: { m_keyEvents[PageDown] = KeyState::Up; }break;
				case Qt::Key_Control: { m_keyEvents[Control] = KeyState::Up; }break;
				case Qt::Key_Shift: { m_keyEvents[Shift] = KeyState::Up; }break;
				default:
					break;
				}
			}
			e->accept();
		}
		else if (t == QEvent::MouseMove) {
			QMouseEvent* e2 = static_cast<QMouseEvent*>(e);
			mouseX = e2->x();
			mouseY = e2->y();
		}
		else if (t == QEvent::MouseButtonRelease) {
			QMouseEvent* e2 = static_cast<QMouseEvent*>(e);
			switch (e2->button())
			{
			case Qt::LeftButton: { m_mouseButtonEvents[MOUSE_BUTTON_LEFT] = MouseButtonState::MOUSE_UP; }break;
			case Qt::RightButton: { m_mouseButtonEvents[MOUSE_BUTTON_RIGHT] = MouseButtonState::MOUSE_UP; }break;
			case Qt::MiddleButton: { m_mouseButtonEvents[MOUSE_BUTTON_MIDDLE] = MouseButtonState::MOUSE_UP; }break;
			default:
				break;
			}
		}
		else if (t == QEvent::MouseButtonPress) {
			QMouseEvent* e2 = static_cast<QMouseEvent*>(e);
			switch (e2->button())
			{
			case Qt::LeftButton: { m_mouseButtonEvents[MOUSE_BUTTON_LEFT] = MouseButtonState::MOUSE_DOWN; }break;
			case Qt::RightButton: { m_mouseButtonEvents[MOUSE_BUTTON_RIGHT] = MouseButtonState::MOUSE_DOWN; }break;
			case Qt::MiddleButton: { m_mouseButtonEvents[MOUSE_BUTTON_MIDDLE] = MouseButtonState::MOUSE_DOWN; }break;
			default:
				break;
			}
		}
		else if (t == QEvent::Wheel) {
			constexpr float kUnitsPerScroll = 1.0f;
			QWheelEvent* e2 = static_cast<QWheelEvent*>(e);
			m_scrollData.second = 0.002 * static_cast<float>(e2->angleDelta().y());
		}
	}
}



