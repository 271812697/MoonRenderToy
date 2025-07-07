

#include <iostream>

#include "InputManager.h"
Windowing::Inputs::InputManager* instance = nullptr;
Windowing::Inputs::InputManager* Windowing::Inputs::InputManager::GetInputManager() {

	return instance;
}

Windowing::Inputs::InputManager::InputManager(Window& p_window) : m_window(p_window)
{

	instance = this;
    //挂载回调
	m_keyPressedListener = m_window.KeyPressedEvent.AddListener(std::bind(&InputManager::OnKeyPressed, this, std::placeholders::_1));
	m_keyReleasedListener = m_window.KeyReleasedEvent.AddListener(std::bind(&InputManager::OnKeyReleased, this, std::placeholders::_1));
	m_mouseButtonPressedListener = m_window.MouseButtonPressedEvent.AddListener(std::bind(&InputManager::OnMouseButtonPressed, this, std::placeholders::_1));
	m_mouseButtonReleasedListener = m_window.MouseButtonReleasedEvent.AddListener(std::bind(&InputManager::OnMouseButtonReleased, this, std::placeholders::_1));
    m_window.MouseScroolEvent.AddListener(std::bind(&InputManager::OnMouseScroll,this,std::placeholders::_1));
	m_window.CursorMoveEvent.AddListener(std::bind(&InputManager::OnCursorMove,this,std::placeholders::_1,std::placeholders::_2));
}

Windowing::Inputs::InputManager::~InputManager()
{
	m_window.KeyPressedEvent.RemoveListener(m_keyPressedListener);
	m_window.KeyReleasedEvent.RemoveListener(m_keyReleasedListener);
	m_window.MouseButtonPressedEvent.RemoveListener(m_mouseButtonPressedListener);
	m_window.MouseButtonReleasedEvent.RemoveListener(m_mouseButtonReleasedListener);
}

Windowing::Inputs::EKeyState Windowing::Inputs::InputManager::GetKeyState(EKey p_key) const
{
	switch (glfwGetKey(m_window.GetGlfwWindow(), static_cast<int>(p_key)))
	{
		case GLFW_PRESS:	return EKeyState::KEY_DOWN;
		case GLFW_RELEASE:	return EKeyState::KEY_UP;
	}

	return EKeyState::KEY_UP;
}

Windowing::Inputs::EMouseButtonState Windowing::Inputs::InputManager::GetMouseButtonState(EMouseButton p_button) const
{
	switch (glfwGetMouseButton(m_window.GetGlfwWindow(), static_cast<int>(p_button)))
	{
		case GLFW_PRESS:	return EMouseButtonState::MOUSE_DOWN;
		case GLFW_RELEASE:	return EMouseButtonState::MOUSE_UP;
	}

	return EMouseButtonState::MOUSE_UP;
}

bool Windowing::Inputs::InputManager::IsKeyPressed(EKey p_key) const
{
	return m_keyEvents.find(p_key) != m_keyEvents.end() && m_keyEvents.at(p_key) == EKeyState::KEY_DOWN;
}

bool Windowing::Inputs::InputManager::IsKeyReleased(EKey p_key) const
{
	return m_keyEvents.find(p_key) != m_keyEvents.end() && m_keyEvents.at(p_key) == EKeyState::KEY_UP;
}

bool Windowing::Inputs::InputManager::IsMouseButtonPressed(EMouseButton p_button) const
{
	return m_mouseButtonEvents.find(p_button) != m_mouseButtonEvents.end() && m_mouseButtonEvents.at(p_button) == EMouseButtonState::MOUSE_DOWN;
}

bool Windowing::Inputs::InputManager::IsMouseButtonReleased(EMouseButton p_button) const
{
	return m_mouseButtonEvents.find(p_button) != m_mouseButtonEvents.end() && m_mouseButtonEvents.at(p_button) == EMouseButtonState::MOUSE_UP;
}

std::pair<double, double> Windowing::Inputs::InputManager::GetMousePosition() const
{
	std::pair<double, double> result;
	glfwGetCursorPos(m_window.GetGlfwWindow(), &result.first, &result.second);

	return result;
}

std::pair<int, int> Windowing::Inputs::InputManager::GetCursoreOffset() 
{
	auto ans= std::pair<int, int>(horizon_X,vertical_Y);
	horizon_X = 0;
	vertical_Y = 0;
	return ans;
}

void Windowing::Inputs::InputManager::ClearEvents()
{
	m_keyEvents.clear();
	m_mouseButtonEvents.clear();
    wheel = 0;
}

float Windowing::Inputs::InputManager::GetMouseScrollOffset() const
{
    return wheel;
}

void Windowing::Inputs::InputManager::OnCursorMove(int p_x, int p_y)
{
	static int x = 0;
	static int y =0;
	horizon_X = p_x - x;
	vertical_Y = p_y - y;
    x = p_x;
	y = p_y;

}

void Windowing::Inputs::InputManager::OnKeyPressed(int p_key)
{
	m_keyEvents[static_cast<EKey>(p_key)] = EKeyState::KEY_DOWN;
}

void Windowing::Inputs::InputManager::OnKeyReleased(int p_key)
{
	m_keyEvents[static_cast<EKey>(p_key)] = EKeyState::KEY_UP;
}

void Windowing::Inputs::InputManager::OnMouseButtonPressed(int p_button)
{
	m_mouseButtonEvents[static_cast<EMouseButton>(p_button)] = EMouseButtonState::MOUSE_DOWN;
}

void Windowing::Inputs::InputManager::OnMouseButtonReleased(int p_button)
{
	m_mouseButtonEvents[static_cast<EMouseButton>(p_button)] = EMouseButtonState::MOUSE_UP;
}

void Windowing::Inputs::InputManager::OnMouseScroll(double delta)
{
    wheel += delta;
}
