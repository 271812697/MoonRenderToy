/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include "Core/ECS/Components/CAudioListener.h"
#include "Core/ECS/Actor.h"

Core::ECS::Components::CAudioListener::CAudioListener(ECS::Actor& p_owner) :
	AComponent(p_owner),
	m_audioListener(owner.transform.GetFTransform())
{
	m_audioListener.SetEnabled(false);
}

std::string Core::ECS::Components::CAudioListener::GetName()
{
	return "Audio Listener";
}

void Core::ECS::Components::CAudioListener::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
}

void Core::ECS::Components::CAudioListener::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
}



void Core::ECS::Components::CAudioListener::OnEnable()
{
	m_audioListener.SetEnabled(true);
}

void Core::ECS::Components::CAudioListener::OnDisable()
{
	m_audioListener.SetEnabled(false);
}
