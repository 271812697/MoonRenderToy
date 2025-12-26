#include <algorithm>
#include <tinyxml2.h>

#include <Core/ECS/Actor.h>
#include <Core/ECS/Components/CAmbientBoxLight.h>
#include <Core/ECS/Components/CAmbientSphereLight.h>

#include <Core/ECS/Components/CCamera.h>
#include <Core/ECS/Components/CDirectionalLight.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/ECS/Components/CModelRenderer.h>

#include <Core/ECS/Components/CPointLight.h>
#include <Core/ECS/Components/CPostProcessStack.h>
#include <Core/ECS/Components/CReflectionProbe.h>
#include <Core/ECS/Components/CSpotLight.h>

Tools::Eventing::Event<Core::ECS::Actor&> Core::ECS::Actor::DestroyedEvent;
Tools::Eventing::Event<Core::ECS::Actor&> Core::ECS::Actor::CreatedEvent;
Tools::Eventing::Event<Core::ECS::Actor&, Core::ECS::Actor&> Core::ECS::Actor::AttachEvent;
Tools::Eventing::Event<Core::ECS::Actor&> Core::ECS::Actor::DettachEvent;

Core::ECS::Actor::Actor(int64_t p_actorID, const std::string& p_name, const std::string& p_tag, bool& p_playing) :
	m_actorID(p_actorID),
	m_name(p_name),
	m_tag(p_tag),
	m_playing(p_playing),
	transform(AddComponent<Components::CTransform>())
{
	CreatedEvent.Invoke(*this);
}

Core::ECS::Actor::~Actor()
{
	if (!m_sleeping)
	{
		if (IsActive())
			OnDisable();

		if (m_awaked && m_started)
			OnDestroy();
	}

	DestroyedEvent.Invoke(*this);

	std::vector<Actor*> toDetach = m_children;

	for (auto child : toDetach)
		child->DetachFromParent();

	toDetach.clear();

	DetachFromParent();

	std::for_each(m_components.begin(), m_components.end(), [&](std::shared_ptr<Components::AComponent> p_component) { ComponentRemovedEvent.Invoke(*p_component); });
	std::for_each(m_children.begin(), m_children.end(), [](Actor* p_element) { delete p_element; });
}

const std::string& Core::ECS::Actor::GetName() const
{
	return m_name;
}

const std::string& Core::ECS::Actor::GetTag() const
{
	return m_tag;
}

void Core::ECS::Actor::SetName(const std::string& p_name)
{
	m_name = p_name;
}

void Core::ECS::Actor::SetTag(const std::string& p_tag)
{
	m_tag = p_tag;
}

void Core::ECS::Actor::SetActive(bool p_active)
{
	if (p_active != m_active)
	{
		RecursiveWasActiveUpdate();
		m_active = p_active;
		RecursiveActiveUpdate();
	}
}

bool Core::ECS::Actor::IsSelfActive() const
{
	return m_active;
}

bool Core::ECS::Actor::IsActive() const
{
	return m_active && (m_parent ? m_parent->IsActive() : true);
}

void Core::ECS::Actor::SetID(int64_t p_id)
{
	m_actorID = p_id;
}

int64_t Core::ECS::Actor::GetID() const
{
	return m_actorID;
}

void Core::ECS::Actor::SetParent(Actor& p_parent)
{
	DetachFromParent();

	/* Define the given parent as the new parent */
	m_parent = &p_parent;
	m_parentID = p_parent.m_actorID;
	transform.SetParent(p_parent.transform);

	/* Store the actor in the parent children list */
	p_parent.m_children.push_back(this);

	AttachEvent.Invoke(*this, p_parent);
}

void Core::ECS::Actor::DetachFromParent()
{
	DettachEvent.Invoke(*this);

	/* Reme the actor from the parent children list */
	if (m_parent)
	{
		m_parent->m_children.erase(std::remove_if(m_parent->m_children.begin(), m_parent->m_children.end(), [this](Actor* p_element)
			{
				return p_element == this;
			}));
	}

	m_parent = nullptr;
	m_parentID = 0;

	transform.RemoveParent();
}

bool Core::ECS::Actor::IsDescendantOf(const Actor* p_actor) const
{
	const Actor* currentParentActor = m_parent;

	while (currentParentActor != nullptr)
	{
		if (currentParentActor == p_actor)
		{
			return true;
		}
		currentParentActor = currentParentActor->GetParent();
	}

	return false;
}

bool Core::ECS::Actor::HasParent() const
{
	return m_parent;
}

Core::ECS::Actor* Core::ECS::Actor::GetParent() const
{
	return m_parent;
}

int64_t Core::ECS::Actor::GetParentID() const
{
	return m_parentID;
}

std::vector<Core::ECS::Actor*>& Core::ECS::Actor::GetChildren()
{
	return m_children;
}

void Core::ECS::Actor::MarkAsDestroy()
{
	m_destroyed = true;

	for (auto child : m_children)
		child->MarkAsDestroy();
}

bool Core::ECS::Actor::IsAlive() const
{
	return !m_destroyed;
}

void Core::ECS::Actor::SetSleeping(bool p_sleeping)
{
	m_sleeping = p_sleeping;
}

void Core::ECS::Actor::OnAwake()
{
	m_awaked = true;
	std::for_each(m_components.begin(), m_components.end(), [](auto element) { element->OnAwake(); });

}

void Core::ECS::Actor::OnStart()
{
	m_started = true;
	std::for_each(m_components.begin(), m_components.end(), [](auto element) { element->OnStart(); });

}

void Core::ECS::Actor::OnEnable()
{
	std::for_each(m_components.begin(), m_components.end(), [](auto element) { element->OnEnable(); });

}

void Core::ECS::Actor::OnDisable()
{
	std::for_each(m_components.begin(), m_components.end(), [](auto element) { element->OnDisable(); });

}

void Core::ECS::Actor::OnDestroy()
{
	std::for_each(m_components.begin(), m_components.end(), [](auto element) { element->OnDestroy(); });

}

void Core::ECS::Actor::OnUpdate(float p_deltaTime)
{
	if (IsActive())
	{
		std::for_each(m_components.begin(), m_components.end(), [&](auto element) { element->OnUpdate(p_deltaTime); });
	}
}

void Core::ECS::Actor::OnFixedUpdate(float p_deltaTime)
{
	if (IsActive())
	{
		std::for_each(m_components.begin(), m_components.end(), [&](auto element) { element->OnFixedUpdate(p_deltaTime); });

	}
}

void Core::ECS::Actor::OnLateUpdate(float p_deltaTime)
{
	if (IsActive())
	{
		std::for_each(m_components.begin(), m_components.end(), [&](auto element) { element->OnLateUpdate(p_deltaTime); });

	}
}

bool Core::ECS::Actor::RemoveComponent(Core::ECS::Components::AComponent& p_component)
{
	for (auto it = m_components.begin(); it != m_components.end(); ++it)
	{
		if (it->get() == &p_component)
		{
			ComponentRemovedEvent.Invoke(p_component);
			m_components.erase(it);
			return true;
		}
	}

	return false;
}

std::vector<std::shared_ptr<Core::ECS::Components::AComponent>>& Core::ECS::Actor::GetComponents()
{
	return m_components;
}

void Core::ECS::Actor::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_actorsRoot)
{
	tinyxml2::XMLNode* actorNode = p_doc.NewElement("actor");
	p_actorsRoot->InsertEndChild(actorNode);

	Core::Helpers::Serializer::SerializeString(p_doc, actorNode, "name", m_name);
	Core::Helpers::Serializer::SerializeString(p_doc, actorNode, "tag", m_tag);
	Core::Helpers::Serializer::SerializeBoolean(p_doc, actorNode, "active", m_active);
	Core::Helpers::Serializer::SerializeInt64(p_doc, actorNode, "id", m_actorID);
	Core::Helpers::Serializer::SerializeInt64(p_doc, actorNode, "parent", m_parentID);

	tinyxml2::XMLNode* componentsNode = p_doc.NewElement("components");
	actorNode->InsertEndChild(componentsNode);

	for (auto& component : m_components)
	{
		/* Current component root */
		tinyxml2::XMLNode* componentNode = p_doc.NewElement("component");
		componentsNode->InsertEndChild(componentNode);

		/* Component type */
		Core::Helpers::Serializer::SerializeString(p_doc, componentNode, "type", typeid(*component).name());

		/* Data node (Will be passed to the component) */
		tinyxml2::XMLElement* data = p_doc.NewElement("data");
		componentNode->InsertEndChild(data);

		/* Data serialization of the component */
		component->OnSerialize(p_doc, data);
	}

	tinyxml2::XMLNode* behavioursNode = p_doc.NewElement("behaviours");
	actorNode->InsertEndChild(behavioursNode);

}

void Core::ECS::Actor::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_actorsRoot)
{
	Core::Helpers::Serializer::DeserializeString(p_doc, p_actorsRoot, "name", m_name);
	Core::Helpers::Serializer::DeserializeString(p_doc, p_actorsRoot, "tag", m_tag);
	Core::Helpers::Serializer::DeserializeBoolean(p_doc, p_actorsRoot, "active", m_active);
	Core::Helpers::Serializer::DeserializeInt64(p_doc, p_actorsRoot, "id", m_actorID);
	Core::Helpers::Serializer::DeserializeInt64(p_doc, p_actorsRoot, "parent", m_parentID);

	{
		tinyxml2::XMLNode* componentsRoot = p_actorsRoot->FirstChildElement("components");
		if (componentsRoot)
		{
			tinyxml2::XMLElement* currentComponent = componentsRoot->FirstChildElement("component");

			while (currentComponent)
			{
				std::string componentType = currentComponent->FirstChildElement("type")->GetText();
				Core::ECS::Components::AComponent* component = nullptr;

				// TODO: Use component name instead of typeid (unsafe)
				if (componentType == typeid(Components::CTransform).name())			component = &transform;

				else if (componentType == typeid(Components::CModelRenderer).name())			component = &AddComponent<Core::ECS::Components::CModelRenderer>();
				else if (componentType == typeid(Components::CCamera).name())				component = &AddComponent<Core::ECS::Components::CCamera>();
				else if (componentType == typeid(Components::CMaterialRenderer).name())		component = &AddComponent<Core::ECS::Components::CMaterialRenderer>();
				else if (componentType == typeid(Components::CPointLight).name())			component = &AddComponent<Core::ECS::Components::CPointLight>();
				else if (componentType == typeid(Components::CDirectionalLight).name())		component = &AddComponent<Core::ECS::Components::CDirectionalLight>();
				else if (componentType == typeid(Components::CSpotLight).name())			component = &AddComponent<Core::ECS::Components::CSpotLight>();
				else if (componentType == typeid(Components::CAmbientBoxLight).name())		component = &AddComponent<Core::ECS::Components::CAmbientBoxLight>();
				else if (componentType == typeid(Components::CAmbientSphereLight).name())	component = &AddComponent<Core::ECS::Components::CAmbientSphereLight>();
				else if (componentType == typeid(Components::CPostProcessStack).name())		component = &AddComponent<Core::ECS::Components::CPostProcessStack>();
				else if (componentType == typeid(Components::CReflectionProbe).name())		component = &AddComponent<Core::ECS::Components::CReflectionProbe>();

				if (component)
					component->OnDeserialize(p_doc, currentComponent->FirstChildElement("data"));

				currentComponent = currentComponent->NextSiblingElement("component");
			}
		}
	}
}

void Core::ECS::Actor::RecursiveActiveUpdate()
{
	bool isActive = IsActive();

	if (!m_sleeping)
	{
		if (!m_wasActive && isActive)
		{
			if (!m_awaked)
				OnAwake();

			OnEnable();

			if (!m_started)
				OnStart();
		}

		if (m_wasActive && !isActive)
			OnDisable();
	}

	for (auto child : m_children)
		child->RecursiveActiveUpdate();
}

void Core::ECS::Actor::RecursiveWasActiveUpdate()
{
	m_wasActive = IsActive();
	for (auto child : m_children)
		child->RecursiveWasActiveUpdate();
}
