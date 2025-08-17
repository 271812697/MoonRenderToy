

#pragma once

#include "OvCore/API/IInspectorItem.h"

namespace OvCore::ECS { class Actor; }

namespace OvCore::ECS::Components
{


	/**
	* AComponent is the base class for any component.
	* A component is a set of data and behaviours (Entity-Component without systems) that is interpreted by the engine (Or the user)
	*/
	class AComponent : public API::IInspectorItem
	{
	public:
		/**
		* Constructor of a AComponent (Must be called by derived classes)
		* @param p_owner
		*/
		AComponent(ECS::Actor& p_owner);

		/**
		* Destroying a AComponent will call dervied classes destructors
		*/
		virtual ~AComponent();

		/**
		* Called when the scene start right before OnStart
		* It allows you to apply prioritized game logic on scene start
		*/
		virtual void OnAwake() {}

		/**
		* Called when the scene start right after OnAwake
		* It allows you to apply prioritized game logic on scene start
		*/
		virtual void OnStart() {}

		/**
		* Called when the components gets enabled (owner SetActive set to true) and after OnAwake() on scene starts
		*/
		virtual void OnEnable() {}

		/**
		* Called when the component gets disabled (owner SetActive set to false) and before OnDestroy() when the component get destroyed
		*/
		virtual void OnDisable() {}

		/**
		* Called when the components gets destroyed
		*/
		virtual void OnDestroy() {}

		/**
		* Called every frame
		* @param p_deltaTime
		*/
		virtual void OnUpdate(float p_deltaTime) {}

		/**
		* Called every physics frame
		* @param p_deltaTime
		*/
		virtual void OnFixedUpdate(float p_deltaTime) {}

		/**
		* Called every frame after OnUpdate
		* @param p_deltaTime
		*/
		virtual void OnLateUpdate(float p_deltaTime) {}

		/**
		* Returns the name of the component
		*/
		virtual std::string GetName() = 0;

	public:
		ECS::Actor& owner;
	};
}