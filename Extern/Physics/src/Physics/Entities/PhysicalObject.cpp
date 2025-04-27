
#include "Physics/Entities/PhysicalObject.h"

#include "Physics/Tools/Conversion.h"
#include "Debug/Logger.h"

using namespace Physics::Tools;
using namespace Physics::Settings;

::Tools::Eventing::Event<Physics::Entities::PhysicalObject&>	Physics::Entities::PhysicalObject::CreatedEvent;
Tools::Eventing::Event<Physics::Entities::PhysicalObject&>	Physics::Entities::PhysicalObject::DestroyedEvent;
Tools::Eventing::Event<btRigidBody&>							Physics::Entities::PhysicalObject::ConsiderEvent;
Tools::Eventing::Event<btRigidBody&>							Physics::Entities::PhysicalObject::UnconsiderEvent;

Physics::Entities::PhysicalObject::PhysicalObject() :
	m_transform(new Maths::FTransform()),
	m_internalTransform(true)
{
	CollisionStartEvent += [this](Physics::Entities::PhysicalObject& otherPhysicalObject)
	{
		UpdateBtTransform();
	};
}

Physics::Entities::PhysicalObject::PhysicalObject(Maths::FTransform& p_transform) :
	m_transform(&p_transform),
	m_internalTransform(false)
{

}

Physics::Entities::PhysicalObject::~PhysicalObject()
{
	DestroyBody();
	PhysicalObject::DestroyedEvent.Invoke(*this);

	if (m_internalTransform)
		delete m_transform;
}

void Physics::Entities::PhysicalObject::Init()
{
	PhysicalObject::CreatedEvent.Invoke(*this);
	CreateBody({});
}

void Physics::Entities::PhysicalObject::AddForce(const Maths::FVector3& p_force)
{
	m_body->applyCentralForce(Conversion::ToBtVector3(p_force));
}

void Physics::Entities::PhysicalObject::AddImpulse(const Maths::FVector3& p_impulse)
{
	m_body->applyCentralImpulse(Conversion::ToBtVector3(p_impulse));
}

void Physics::Entities::PhysicalObject::ClearForces()
{
	m_body->clearForces();
}

void Physics::Entities::PhysicalObject::AddFlag(btCollisionObject::CollisionFlags p_flag)
{
	m_body->setCollisionFlags(m_body->getCollisionFlags() | p_flag);
}

void Physics::Entities::PhysicalObject::RemoveFlag(btCollisionObject::CollisionFlags p_flag)
{
	m_body->setCollisionFlags(m_body->getCollisionFlags() & ~p_flag);
}

float Physics::Entities::PhysicalObject::GetMass() const
{
	return m_mass;
}

const Physics::Entities::PhysicalObject::ECollisionDetectionMode& Physics::Entities::PhysicalObject::GetCollisionDetectionMode() const
{
	return m_collisionMode;
}

float Physics::Entities::PhysicalObject::GetBounciness() const
{
	return m_body->getRestitution();
}

float Physics::Entities::PhysicalObject::GetFriction() const
{
	return m_body->getFriction();
}

Maths::FVector3 Physics::Entities::PhysicalObject::GetLinearVelocity() const
{
	return Conversion::ToVector3(m_body->getLinearVelocity());
}

Maths::FVector3 Physics::Entities::PhysicalObject::GetAngularVelocity() const
{
	return Conversion::ToVector3(m_body->getAngularVelocity());
}

Maths::FVector3 Physics::Entities::PhysicalObject::GetLinearFactor() const
{
	return Conversion::ToVector3(m_body->getLinearFactor());
}

Maths::FVector3 Physics::Entities::PhysicalObject::GetAngularFactor() const
{
	return Conversion::ToVector3(m_body->getAngularFactor());
}

bool Physics::Entities::PhysicalObject::IsTrigger() const
{
	return m_trigger;
}

bool Physics::Entities::PhysicalObject::IsKinematic() const
{
	return m_kinematic;
}

Physics::Entities::PhysicalObject::EActivationState Physics::Entities::PhysicalObject::GetActivationState() const
{
	return static_cast<EActivationState>(m_body->getActivationState());
}

Maths::FTransform& Physics::Entities::PhysicalObject::GetTransform()
{
	return *m_transform;
}

void Physics::Entities::PhysicalObject::SetMass(float p_mass)
{
	m_mass = p_mass;
	ApplyInertia();
}

void Physics::Entities::PhysicalObject::SetCollisionDetectionMode(ECollisionDetectionMode p_mode)
{
	m_collisionMode = p_mode;

	switch (m_collisionMode)
	{
	case ECollisionDetectionMode::DISCRETE:
		m_body->setCcdMotionThreshold(std::numeric_limits<float>::max());
		m_body->setCcdSweptSphereRadius(0.0f);
		break;
	case ECollisionDetectionMode::CONTINUOUS:
		m_body->setCcdMotionThreshold(static_cast<btScalar>(1e-7));
		m_body->setCcdSweptSphereRadius(0.5f);
		break;
	}
}

void Physics::Entities::PhysicalObject::SetBounciness(float p_bounciness)
{
	m_body->setRestitution(p_bounciness);
}

void Physics::Entities::PhysicalObject::SetFriction(float p_friction)
{
	m_body->setFriction(p_friction);
}

void Physics::Entities::PhysicalObject::SetLinearVelocity(const Maths::FVector3 & p_linearVelocity)
{
	m_body->setLinearVelocity(Conversion::ToBtVector3(p_linearVelocity));
}

void Physics::Entities::PhysicalObject::SetAngularVelocity(const Maths::FVector3 & p_angularVelocity)
{
	m_body->setAngularVelocity(Conversion::ToBtVector3(p_angularVelocity));
}

void Physics::Entities::PhysicalObject::SetLinearFactor(const Maths::FVector3 & p_linearFactor)
{
	m_body->setLinearFactor(Conversion::ToBtVector3(p_linearFactor));
}

void Physics::Entities::PhysicalObject::SetAngularFactor(const Maths::FVector3 & p_angularFactor)
{
	m_body->setAngularFactor(Conversion::ToBtVector3(p_angularFactor));
}

void Physics::Entities::PhysicalObject::SetTrigger(bool p_trigger)
{
	if (p_trigger)
		AddFlag(btCollisionObject::CF_NO_CONTACT_RESPONSE);
	else
		RemoveFlag(btCollisionObject::CF_NO_CONTACT_RESPONSE);

	m_trigger = p_trigger;
}

void Physics::Entities::PhysicalObject::SetKinematic(bool p_kinematic)
{
	m_kinematic = p_kinematic;

	if (m_kinematic)
	{
		ClearForces();
		SetLinearVelocity(Maths::FVector3::Zero);
		SetAngularVelocity(Maths::FVector3::Zero);
	}

	RecreateBody();
}

void Physics::Entities::PhysicalObject::SetActivationState(EActivationState p_activationState)
{
	m_body->setActivationState(static_cast<int>(p_activationState));
}

void Physics::Entities::PhysicalObject::SetEnabled(bool p_enabled)
{
	m_enabled = p_enabled;

	if (!m_enabled)
		Unconsider();
	else
		Consider();
}

bool Physics::Entities::PhysicalObject::IsEnabled() const
{
	return m_enabled;
}

void Physics::Entities::PhysicalObject::UpdateBtTransform()
{
	m_body->setWorldTransform(Conversion::ToBtTransform(*m_transform));

	if (Maths::FVector3::Distance(m_transform->GetWorldScale(), m_previousScale) >= 0.01f)
	{
		m_previousScale = m_transform->GetWorldScale();
		SetLocalScaling({ abs(m_previousScale.x), abs(m_previousScale.y), abs(m_previousScale.z) });
		RecreateBody();
	}
}

void Physics::Entities::PhysicalObject::UpdateFTransform()
{
	if (!m_kinematic)
	{
		const btTransform& result = m_body->getWorldTransform();
		if (m_transform->HasParent()) {
			
			auto Local=Conversion::ToBtTransform(*(m_transform->m_parent)).inverse()*result;
			m_transform->SetLocalPosition(Conversion::ToVector3(Local.getOrigin()));
			m_transform->SetLocalRotation(Conversion::ToOvQuaternion(Local.getRotation()));

		}
		else {
		   m_transform->SetLocalPosition(Conversion::ToVector3(result.getOrigin()));
		   m_transform->SetLocalRotation(Conversion::ToOvQuaternion(result.getRotation()));
		}

	}
}

void Physics::Entities::PhysicalObject::RecreateBody()
{
	CreateBody(DestroyBody());
}

void Physics::Entities::PhysicalObject::ApplyInertia()
{
	m_body->setMassProps(m_kinematic ? 0.0f : std::max(0.0000001f, m_mass), m_kinematic ? btVector3(0.0f, 0.0f, 0.0f) : CalculateInertia());
}

void Physics::Entities::PhysicalObject::Consider()
{
	if (!m_considered)
	{
		m_considered = true;
		ConsiderEvent.Invoke(*m_body);
	}
}

void Physics::Entities::PhysicalObject::Unconsider()
{
	if (m_considered)
	{
		m_considered = false;
		UnconsiderEvent.Invoke(*m_body);
	}
}

void Physics::Entities::PhysicalObject::CreateBody(const Settings::BodySettings & p_bodySettings)
{
	m_motion = std::make_unique<btDefaultMotionState>(Tools::Conversion::ToBtTransform(*m_transform));

	m_body = std::make_unique<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo{ 0.0f, m_motion.get(), m_shape.get(), btVector3(0.0f, 0.0f, 0.0f) });

	ApplyInertia();

	m_body->setRestitution(p_bodySettings.restitution);
	m_body->setFriction(p_bodySettings.friction);
	m_body->setLinearVelocity(p_bodySettings.linearVelocity);
	m_body->setAngularVelocity(p_bodySettings.angularVelocity);
	m_body->setLinearFactor(p_bodySettings.linearFactor);
	m_body->setAngularFactor(p_bodySettings.angularFactor);
	m_body->setUserPointer(this);

	AddFlag(btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

	if (p_bodySettings.isTrigger)
		AddFlag(btCollisionObject::CF_NO_CONTACT_RESPONSE);

	SetActivationState(EActivationState::ALWAYS_ACTIVE); // TODO: Avoid using always active

	if (m_enabled)
		Consider();
}

Physics::Settings::BodySettings Physics::Entities::PhysicalObject::DestroyBody()
{
	BodySettings result
	{
		m_body->getLinearVelocity(),
		m_body->getAngularVelocity(),
		m_body->getLinearFactor(),
		m_body->getAngularFactor(),
		GetBounciness(),
		GetFriction(),
		IsTrigger(),
		IsKinematic()
	};

	Unconsider();

	m_body.reset();
	m_motion.reset();

	return result;
}

btVector3 Physics::Entities::PhysicalObject::CalculateInertia() const
{
	btVector3 result = { 0.f, 0.f, 0.f };

	if (m_mass != 0.0f)
		m_shape->calculateLocalInertia(m_mass, result);

	return result;
}

btRigidBody& Physics::Entities::PhysicalObject::GetBody()
{
	return *m_body;
}
