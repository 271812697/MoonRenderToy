
#pragma once

#include <bullet/btBulletCollisionCommon.h>

#include <Maths/FTransform.h>
#include <Maths/FVector3.h>
#include <Maths/FQuaternion.h>

namespace Physics::Tools
{
	/**
	* Conversion helper to convert bullet maths to Maths
	*/
	class Conversion
	{
	public:
		Conversion() = delete;

		/**
		* Convert a FTransform to btTransform
		* @param p_transform
		*/
		static btTransform ToBtTransform(const Maths::FTransform& p_transform);

		/**
		* Convert a FVector3 to btVector3
		* @param p_vector
		*/
		static btVector3 ToBtVector3(const Maths::FVector3& p_vector);

		/**
		* Convert a FQuaternion to btQuaternion
		* @param p_quaternion
		*/
		static btQuaternion ToBtQuaternion(const Maths::FQuaternion& p_quaternion);

		/**
		* Convert a btTransform to FTransform
		* @param p_transform
		*/
		static Maths::FTransform ToTransform(const btTransform& p_transform);

		/**
		* Convert a btVector3 to FVector3
		* @param p_vector
		*/
		static Maths::FVector3 ToVector3(const btVector3& p_vector);

		/**
		* Convert a btQuaternion to FQuaternion
		* @param p_quaternion
		*/
		static Maths::FQuaternion ToOvQuaternion(const btQuaternion& p_quaternion);
	};
}