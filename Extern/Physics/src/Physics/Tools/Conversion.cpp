
#include "Physics/Tools/Conversion.h"

btTransform Physics::Tools::Conversion::ToBtTransform(const Maths::FTransform& p_transform)
{
	return btTransform(ToBtQuaternion(p_transform.GetWorldRotation()), ToBtVector3(p_transform.GetWorldPosition()));
}

btVector3 Physics::Tools::Conversion::ToBtVector3(const Maths::FVector3& p_vector)
{
	return btVector3(p_vector.x, p_vector.y, p_vector.z);
}

btQuaternion Physics::Tools::Conversion::ToBtQuaternion(const Maths::FQuaternion& p_quaternion)
{
	return btQuaternion(p_quaternion.x, p_quaternion.y, p_quaternion.z, p_quaternion.w);
}

Maths::FTransform Physics::Tools::Conversion::ToTransform(const btTransform& p_transform)
{
	Maths::FQuaternion q = ToOvQuaternion(p_transform.getRotation());
	Maths::FVector3  v = ToVector3(p_transform.getOrigin());
	return Maths::FTransform(v, q);
}

Maths::FVector3 Physics::Tools::Conversion::ToVector3(const btVector3& p_vector)
{
	return Maths::FVector3(p_vector.getX(), p_vector.getY(), p_vector.getZ());
}

Maths::FQuaternion Physics::Tools::Conversion::ToOvQuaternion(const btQuaternion& p_quaternion)
{
	return Maths::FQuaternion(p_quaternion.getX(), p_quaternion.getY(), p_quaternion.getZ(), p_quaternion.getW());
}
