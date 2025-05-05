/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include "Core/ECS/PhysicsWrapper.h"

#include "Core/Global/ServiceLocator.h"

#include <Physics/Core/PhysicsEngine.h>

std::optional<Core::ECS::PhysicsWrapper::RaycastHit> Core::ECS::PhysicsWrapper::Raycast(Maths::FVector3 p_origin, Maths::FVector3 p_direction, float p_distance)
{
	if (auto result = OVSERVICE(Physics::Core::PhysicsEngine).Raycast(p_origin, p_direction, p_distance))
	{
		RaycastHit finalResult;

		finalResult.FirstResultObject = std::addressof(result.value().FirstResultObject->GetUserData<std::reference_wrapper<Components::CPhysicalObject>>().get());
		for (auto object : result.value().ResultObjects)
			finalResult.ResultObjects.push_back(std::addressof(object->GetUserData<std::reference_wrapper<Components::CPhysicalObject>>().get()));

		return finalResult;
	}
	else
		return {};
}
