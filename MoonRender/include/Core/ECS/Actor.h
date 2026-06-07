#pragma once
#include <unordered_map>
#include <memory>
#include <Tools/Eventing/Event.h>

#include "Core/ECS/Components/AComponent.h"
#include "Core/ECS/Components/CTransform.h"

#include "Core/API/ISerializable.h"

namespace Core::ECS
{
	class Actor : public API::ISerializable
	{
	public:
		Actor(int64_t p_actorID, const std::string& p_name, const std::string& p_tag, bool& p_playing);
		virtual ~Actor() override;
		const std::string& GetName() const;
		const std::string& GetTag() const;
		void SetName(const std::string& p_name);
		void SetTag(const std::string& p_tag);
		void SetActive(bool p_active);
		bool IsSelfActive() const;
		bool IsActive() const;
		void SetID(int64_t p_id);
		int64_t GetID() const;
		void SetParent(Actor& p_parent);
		void DetachFromParent();
		bool IsDescendantOf(const Actor* p_actor) const;
		bool HasParent() const;
		Actor* GetParent() const;
		Actor* GetChild(const std::string& p_name) const;
		int64_t GetParentID() const;
		int GetChildId( Actor*child)const;
		std::vector<Actor*>& GetChildren();

		void MarkAsDestroy();
		bool IsAlive() const;
		void SetSleeping(bool p_sleeping);
		void OnAwake();
		void OnStart();
		void OnEnable();
		void OnDisable();
		void OnDestroy();
		void OnUpdate(float p_deltaTime);
		void OnFixedUpdate(float p_deltaTime);
		void OnLateUpdate(float p_deltaTime);
		template<typename T, typename ... Args>
		T& AddComponent(Args&&... p_args);
		template<typename T>
		bool RemoveComponent();
		bool RemoveComponent(Core::ECS::Components::AComponent& p_component);
		bool HasComponent(const std::string& compName)const;
		template<typename T>
		T* GetComponent() const;
		std::vector<std::shared_ptr<Components::AComponent>>& GetComponents();
		virtual void OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_actorsRoot) override;
		virtual void OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_actorsRoot) override;

	private:
		Actor(const Actor& p_actor) = delete;

		void RecursiveActiveUpdate();
		void RecursiveWasActiveUpdate();

	public:
		/* Some events that are triggered when an action occur on the actor instance */
		Tools::Eventing::Event<Components::AComponent&>	ComponentAddedEvent;
		Tools::Eventing::Event<Components::AComponent&>	ComponentRemovedEvent;


		/* Some events that are triggered when an action occur on any actor */
		static Tools::Eventing::Event<Actor&>				DestroyedEvent;
		static Tools::Eventing::Event<Actor&>				CreatedEvent;
		static Tools::Eventing::Event<Actor&, Actor&>		AttachEvent;
		static Tools::Eventing::Event<Actor&>				DettachEvent;

	private:
		/* Settings */
		std::string		m_name;
		std::string		m_tag;
		bool			m_active = true;
		bool& m_playing;

		/* Internal settings */
		int64_t	m_actorID;
		bool	m_destroyed = false;
		bool	m_sleeping = true;
		bool	m_awaked = false;
		bool	m_started = false;
		bool	m_wasActive = false;

		/* Parenting system stuff */
		int64_t					m_parentID = 0;
		Actor* m_parent = nullptr;
		std::vector<Actor*>		m_children;
		std::unordered_map<Actor*, int>m_childrenId;

		/* Actors components */
		std::vector<std::shared_ptr<Components::AComponent>> m_components;


	public:
		Components::CTransform& transform;
	};
}

#include "Core/ECS/Actor.inl"