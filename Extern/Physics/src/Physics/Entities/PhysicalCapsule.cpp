

#include <algorithm>

#include "Physics/Entities/PhysicalCapsule.h"
#include "Physics/Tools/Conversion.h"

Physics::Entities::PhysicalCapsule::PhysicalCapsule(float p_radius, float p_height) : PhysicalObject()
{
	CreateCollisionShape(p_radius, p_height);
	Init();
}

Physics::Entities::PhysicalCapsule::PhysicalCapsule(Maths::FTransform & p_transform, float p_radius, float p_height) : PhysicalObject(p_transform)
{
	CreateCollisionShape(p_radius, p_height);
	Init();
}

void Physics::Entities::PhysicalCapsule::SetRadius(float p_radius)
{
	RecreateCollisionShape(p_radius, m_height);
}

void Physics::Entities::PhysicalCapsule::SetHeight(float p_height)
{
	RecreateCollisionShape(m_radius, p_height);
}

float Physics::Entities::PhysicalCapsule::GetRadius() const
{
	return m_radius;
}

float Physics::Entities::PhysicalCapsule::GetHeight() const
{
	return m_height;
}

void Physics::Entities::PhysicalCapsule::CreateCollisionShape(float p_radius, float p_height)
{
	m_shape = std::make_unique<btCapsuleShape>(p_radius, p_height);
	m_radius = p_radius;
	m_height = p_height;
}

void Physics::Entities::PhysicalCapsule::RecreateCollisionShape(float p_radius, float p_height)
{
	CreateCollisionShape(p_radius, p_height);
	RecreateBody();
}

void Physics::Entities::PhysicalCapsule::SetLocalScaling(const Maths::FVector3 & p_scaling)
{
	m_shape->setLocalScaling({ std::max(p_scaling.x, p_scaling.z), p_scaling.y, 1.0f });
}