

#pragma once

#include "Physics/Entities/PhysicalObject.h"

namespace Physics::Entities
{
	/**
	* PhysicalObject with a box shape
	*/
	class PhysicalBox : public PhysicalObject
	{
	public:
		/**
		* PhysicalBox constructor (Internal transform management)
		* @param p_radius
		* @param p_height
		*/
		PhysicalBox(const Maths::FVector3& p_size = {0.5f, 0.5f, 0.5f});

		/**
		* PhysicalBox constructor (External transform management)
		* @param p_transform
		* @param p_radius
		* @param p_height
		*/
		PhysicalBox(Maths::FTransform& p_transform, const Maths::FVector3& p_size = { 0.5f, 0.5f, 0.5f });

		/**
		* Defines the size of the physical box
		* @param p_size
		*/
		void SetSize(const Maths::FVector3& p_size);

		/**
		* Returns the size of the physical box
		*/
		Maths::FVector3 GetSize() const;

	private:
		void CreateCollisionShape(const Maths::FVector3& p_size);
		void RecreateCollisionShape(const Maths::FVector3& p_size);
		virtual void SetLocalScaling(const Maths::FVector3& p_scaling) override;

	private:
		Maths::FVector3 m_size;
	};
}