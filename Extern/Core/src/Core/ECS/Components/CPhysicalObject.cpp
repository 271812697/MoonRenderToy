
#include <Debug/Logger.h>

#include <Physics/Entities/PhysicalObject.h>

#include "Core/ECS/Components/CPhysicalObject.h"
#include "Core/ECS/Actor.h"

Core::ECS::Components::CPhysicalObject::CPhysicalObject(ECS::Actor & p_owner) : 
	AComponent(p_owner)
{
}

void Core::ECS::Components::CPhysicalObject::AddForce(const Maths::FVector3 & p_force)
{
	m_physicalObject->AddForce(p_force);
}

void Core::ECS::Components::CPhysicalObject::AddImpulse(const Maths::FVector3 & p_impulse)
{
	m_physicalObject->AddImpulse(p_impulse);
}

void Core::ECS::Components::CPhysicalObject::ClearForces()
{
	m_physicalObject->ClearForces();
}

void Core::ECS::Components::CPhysicalObject::SetCollisionDetectionMode(Physics::Entities::PhysicalObject::ECollisionDetectionMode p_mode)
{
	m_physicalObject->SetCollisionDetectionMode(p_mode);
}

Physics::Entities::PhysicalObject::ECollisionDetectionMode Core::ECS::Components::CPhysicalObject::GetCollisionDetectionMode() const
{
	return m_physicalObject->GetCollisionDetectionMode();
}

float Core::ECS::Components::CPhysicalObject::GetMass() const
{
	return m_physicalObject->GetMass();
}

float Core::ECS::Components::CPhysicalObject::GetBounciness() const
{
	return m_physicalObject->GetBounciness();
}

float Core::ECS::Components::CPhysicalObject::GetFriction() const
{
	return m_physicalObject->GetFriction();
}

Maths::FVector3 Core::ECS::Components::CPhysicalObject::GetLinearVelocity() const
{
	return m_physicalObject->GetLinearVelocity();
}

Maths::FVector3 Core::ECS::Components::CPhysicalObject::GetAngularVelocity() const
{
	return m_physicalObject->GetAngularVelocity();
}

Maths::FVector3 Core::ECS::Components::CPhysicalObject::GetLinearFactor() const
{
	return m_physicalObject->GetLinearFactor();
}

Maths::FVector3 Core::ECS::Components::CPhysicalObject::GetAngularFactor() const
{
	return m_physicalObject->GetAngularFactor();
}

bool Core::ECS::Components::CPhysicalObject::IsTrigger() const
{
	return m_physicalObject->IsTrigger();
}

bool Core::ECS::Components::CPhysicalObject::IsKinematic() const
{
	return m_physicalObject->IsKinematic();
}

Physics::Entities::PhysicalObject::EActivationState Core::ECS::Components::CPhysicalObject::GetActivationState() const
{
	return m_physicalObject->GetActivationState();
}

void Core::ECS::Components::CPhysicalObject::SetMass(float p_mass)
{
	m_physicalObject->SetMass(p_mass);
}

void Core::ECS::Components::CPhysicalObject::SetBounciness(float p_bounciness)
{
	m_physicalObject->SetBounciness(p_bounciness);
}

void Core::ECS::Components::CPhysicalObject::SetFriction(float p_friction)
{
	m_physicalObject->SetFriction(p_friction);
}

void Core::ECS::Components::CPhysicalObject::SetLinearVelocity(const Maths::FVector3 & p_linearVelocity)
{
	m_physicalObject->SetLinearVelocity(p_linearVelocity);
}

void Core::ECS::Components::CPhysicalObject::SetAngularVelocity(const Maths::FVector3 & p_angularVelocity)
{
	m_physicalObject->SetAngularVelocity(p_angularVelocity);
}

void Core::ECS::Components::CPhysicalObject::SetLinearFactor(const Maths::FVector3 & p_linearFactor)
{
	m_physicalObject->SetLinearFactor(p_linearFactor);
}

void Core::ECS::Components::CPhysicalObject::SetAngularFactor(const Maths::FVector3 & p_angularFactor)
{
	m_physicalObject->SetAngularFactor(p_angularFactor);
}

void Core::ECS::Components::CPhysicalObject::SetTrigger(bool p_trigger)
{
	m_physicalObject->SetTrigger(p_trigger);
}

void Core::ECS::Components::CPhysicalObject::SetKinematic(bool p_kinematic)
{
	m_physicalObject->SetKinematic(p_kinematic);
}

void Core::ECS::Components::CPhysicalObject::SetActivationState(Physics::Entities::PhysicalObject::EActivationState p_state)
{
	m_physicalObject->SetActivationState(p_state);
}

void Core::ECS::Components::CPhysicalObject::OnSerialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_node)
{
	Helpers::Serializer::SerializeBoolean(p_doc, p_node, "is_trigger", IsTrigger());
	Helpers::Serializer::SerializeBoolean(p_doc, p_node, "is_kinematic", IsKinematic());
	Helpers::Serializer::SerializeFloat(p_doc, p_node, "bounciness", GetBounciness());
	Helpers::Serializer::SerializeFloat(p_doc, p_node, "mass", GetMass());
	Helpers::Serializer::SerializeFloat(p_doc, p_node, "friction", GetFriction());
	Helpers::Serializer::SerializeVec3(p_doc, p_node, "linear_factor", GetLinearFactor());
	Helpers::Serializer::SerializeVec3(p_doc, p_node, "angular_factor", GetAngularFactor());
	Helpers::Serializer::SerializeInt(p_doc, p_node, "collision_mode", static_cast<int>(GetCollisionDetectionMode()));
}

void Core::ECS::Components::CPhysicalObject::OnDeserialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_node)
{
	SetTrigger(Helpers::Serializer::DeserializeBoolean(p_doc, p_node, "is_trigger"));
	SetKinematic(Helpers::Serializer::DeserializeBoolean(p_doc, p_node, "is_kinematic"));
	SetBounciness(Helpers::Serializer::DeserializeFloat(p_doc, p_node, "bounciness"));
	SetMass(Helpers::Serializer::DeserializeFloat(p_doc, p_node, "mass"));
	SetFriction(Helpers::Serializer::DeserializeFloat(p_doc, p_node, "friction"));
	SetLinearFactor(Helpers::Serializer::DeserializeVec3(p_doc, p_node, "linear_factor"));
	SetAngularFactor(Helpers::Serializer::DeserializeVec3(p_doc, p_node, "angular_factor"));
	SetCollisionDetectionMode(static_cast<Physics::Entities::PhysicalObject::ECollisionDetectionMode>(Helpers::Serializer::DeserializeInt(p_doc, p_node, "collision_mode")));
}


void Core::ECS::Components::CPhysicalObject::Init()
{
	m_physicalObject->SetEnabled(false);
}

void Core::ECS::Components::CPhysicalObject::BindListener()
{
	/* Collision Events */
	m_physicalObject->CollisionStartEvent += [this](Physics::Entities::PhysicalObject& otherPhysicalObject)
	{
		auto& otherObject = otherPhysicalObject.GetUserData<std::reference_wrapper<CPhysicalObject>>().get();

		CollisionEnterEvent.Invoke(otherObject);
		owner.OnCollisionEnter(otherObject);
	};
	m_physicalObject->CollisionStayEvent += [this](Physics::Entities::PhysicalObject& otherPhysicalObject)
	{
		auto& otherObject = otherPhysicalObject.GetUserData<std::reference_wrapper<CPhysicalObject>>().get();

		CollisionStayEvent.Invoke(otherObject);
		owner.OnCollisionStay(otherObject);
	};
	m_physicalObject->CollisionStopEvent += [this](Physics::Entities::PhysicalObject& otherPhysicalObject)
	{
		auto& otherObject = otherPhysicalObject.GetUserData<std::reference_wrapper<CPhysicalObject>>().get();

		CollisionExitEvent.Invoke(otherObject);
		owner.OnCollisionExit(otherObject);
	};

	/* Trigger Events */
	m_physicalObject->TriggerStartEvent += [this](Physics::Entities::PhysicalObject& otherPhysicalObject)
	{
		auto& otherObject = otherPhysicalObject.GetUserData<std::reference_wrapper<CPhysicalObject>>().get();

		TriggerEnterEvent.Invoke(otherObject);
		owner.OnTriggerEnter(otherObject);
	};
	m_physicalObject->TriggerStayEvent += [this](Physics::Entities::PhysicalObject& otherPhysicalObject)
	{
		auto& otherObject = otherPhysicalObject.GetUserData<std::reference_wrapper<CPhysicalObject>>().get();

		TriggerStayEvent.Invoke(otherObject);
		owner.OnTriggerStay(otherObject);
	};
	m_physicalObject->TriggerStopEvent += [this](Physics::Entities::PhysicalObject& otherPhysicalObject)
	{
		auto& otherObject = otherPhysicalObject.GetUserData<std::reference_wrapper<CPhysicalObject>>().get();

		TriggerExitEvent.Invoke(otherObject);
		owner.OnTriggerExit(otherObject);
	};
}

void Core::ECS::Components::CPhysicalObject::OnEnable()
{
	m_physicalObject->SetEnabled(true);
}

void Core::ECS::Components::CPhysicalObject::OnDisable()
{
	m_physicalObject->SetEnabled(false);
}
