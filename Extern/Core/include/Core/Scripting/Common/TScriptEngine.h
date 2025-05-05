/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <string>
#include <vector>

#include <Core/Scripting/Common/EScriptingLanguage.h>

namespace Core::ECS::Components
{
	class Behaviour;
	class CPhysicalObject;
}

namespace Core::Scripting
{
	/**
	* Interface for any scripting backend to implement.
	* This class defines the necessary methods that a scripting engine should provide
	* to interact with the engine's components and handle various lifecycle events.
	*/
	template<EScriptingLanguage Language, class Context>
	class TScriptEngine
	{
	public:
		/**
		* Constructor of the generic script engine
		*/
		TScriptEngine();

		/**
		* Destructor of the generic script engine (virtual to allow polymorphism)
		*/
		virtual ~TScriptEngine();

		/**
		* Defines the root location of the script folder
		* @param p_rootFolder
		*/
		void SetScriptRootFolder(const std::string& p_rootFolder);

		/**
		* Returns a list of valid extensions for scripts.
		*/
		std::vector<std::string> GetValidExtensions();

		/**
		* Returns the content for a default script
		* @param p_name Name of the class or object represented by this script
		*/
		std::string GetDefaultScriptContent(const std::string& p_name);

		/**
		* Returns the extension for a default script
		*/
		std::string GetDefaultExtension();

		/**
		* Adds a behaviour to the scripting engine.
		* @param p_toAdd The behaviour to add.
		*/
		void AddBehaviour(Core::ECS::Components::Behaviour& p_toAdd);

		/**
		* Removes a behaviour from the scripting engine.
		* @param p_toRemove The behaviour to remove.
		*/
		void RemoveBehaviour(Core::ECS::Components::Behaviour& p_toRemove);

		/**
		* Reloads the scripting engine.
		*/
		void Reload();

		/**
		* Checks if the scripting engine is in a good state.
		* @return True if the engine is okay, false otherwise.
		*/
		bool IsOk() const;

		/**
		* Called when a behaviour is awakened.
		* @param p_target The target behaviour.
		*/
		void OnAwake(Core::ECS::Components::Behaviour& p_target);

		/**
		* Called when a behaviour starts.
		* @param p_target The target behaviour.
		*/
		void OnStart(Core::ECS::Components::Behaviour& p_target);

		/**
		* Called when a behaviour is enabled.
		* @param p_target The target behaviour.
		*/
		void OnEnable(Core::ECS::Components::Behaviour& p_target);

		/**
		* Called when a behaviour is disabled.
		* @param p_target The target behaviour.
		*/
		void OnDisable(Core::ECS::Components::Behaviour& p_target);

		/**
		* Called when a behaviour is destroyed.
		* @param p_target The target behaviour.
		*/
		void OnDestroy(Core::ECS::Components::Behaviour& p_target);

		/**
		* Called every frame to update a behaviour.
		* @param p_target The target behaviour.
		* @param p_deltaTime The time elapsed since the last frame.
		*/
		void OnUpdate(Core::ECS::Components::Behaviour& p_target, float p_deltaTime);

		/**
		* Called every physics frame to update a behaviour.
		* @param p_target The target behaviour.
		* @param p_deltaTime The time elapsed since the last physics frame.
		*/
		void OnFixedUpdate(Core::ECS::Components::Behaviour& p_target, float p_deltaTime);

		/**
		* Called every frame after OnUpdate to perform late updates on a behaviour.
		* @param p_target The target behaviour.
		* @param p_deltaTime The time elapsed since the last frame.
		*/
		void OnLateUpdate(Core::ECS::Components::Behaviour& p_target, float p_deltaTime);

		/**
		* Called when a behaviour's owner enters a collision with another physical object.
		* @param p_target The target behaviour.
		* @param p_otherObject The other physical object involved in the collision.
		*/
		void OnCollisionEnter(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject);

		/**
		* Called when a behaviour's owner stays in collision with another physical object.
		* @param p_target The target behaviour.
		* @param p_otherObject The other physical object involved in the collision.
		*/
		void OnCollisionStay(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject);

		/**
		* Called when a behaviour's owner exits a collision with another physical object.
		* @param p_target The target behaviour.
		* @param p_otherObject The other physical object involved in the collision.
		*/
		void OnCollisionExit(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject);

		/**
		* Called when a behaviour's owner enters a trigger with another physical object.
		* @param p_target The target behaviour.
		* @param p_otherObject The other physical object involved in the trigger.
		*/
		void OnTriggerEnter(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject);

		/**
		* Called when a behaviour's owner stays in a trigger with another physical object.
		* @param p_target The target behaviour.
		* @param p_otherObject The other physical object involved in the trigger.
		*/
		void OnTriggerStay(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject);

		/**
		* Called when a behaviour's owner exits a trigger with another physical object.
		* @param p_target The target behaviour.
		* @param p_otherObject The other physical object involved in the trigger.
		*/
		void OnTriggerExit(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject);

	protected:
		Context m_context;
	};
}
