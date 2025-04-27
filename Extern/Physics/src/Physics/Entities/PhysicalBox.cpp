

#include "Physics/Entities/PhysicalBox.h"
#include "Physics/Tools/Conversion.h"

Physics::Entities::PhysicalBox::PhysicalBox(const Maths::FVector3& p_size) : PhysicalObject()
{
	CreateCollisionShape(p_size);
	Init();
}

Physics::Entities::PhysicalBox::PhysicalBox(Maths::FTransform & p_transform, const Maths::FVector3& p_size) : PhysicalObject(p_transform)
{
	CreateCollisionShape(p_size);
	Init();
}

void Physics::Entities::PhysicalBox::SetSize(const Maths::FVector3& p_size)
{
	if (m_size != p_size)
		RecreateCollisionShape(p_size);
}

Maths::FVector3 Physics::Entities::PhysicalBox::GetSize() const
{
	return m_size;
}


void Physics::Entities::PhysicalBox::CreateCollisionShape(const Maths::FVector3& p_size)
{
	m_shape = std::make_unique<btBoxShape>(Physics::Tools::Conversion::ToBtVector3(p_size));
	m_size = p_size;
}

void Physics::Entities::PhysicalBox::RecreateCollisionShape(const Maths::FVector3& p_size)
{
	CreateCollisionShape(p_size);
	RecreateBody();
}

void Physics::Entities::PhysicalBox::SetLocalScaling(const Maths::FVector3 & p_scaling)
{
	m_shape->setLocalScaling(Physics::Tools::Conversion::ToBtVector3(p_scaling));
}