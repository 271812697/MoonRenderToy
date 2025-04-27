

#pragma once

#include <Maths/FVector3.h>
#include <Maths/FMatrix4.h>
#include <Maths/FTransform.h>



namespace Rendering::Entities
{
	/**
	* Data structure that can represent any type of light
	*/
	struct Light
	{
		/**
		* Light types
		*/
		enum class Type { POINT, DIRECTIONAL, SPOT, AMBIENT_BOX, AMBIENT_SPHERE };

		/**
		* Create the light with the given transform and type
		* @param p_transform
		* @param p_type
		*/
		Light(Maths::FTransform& p_tranform, Type p_type);

		/**
		* Generate the light matrix, ready to send to the GPU
		*/
		Maths::FMatrix4 GenerateMatrix() const;

		/**
		* Calculate the light effect range from the quadratic falloff equation
		*/
		float GetEffectRange() const;

		/**
		* Returns the light transform
		*/
		const Maths::FTransform& GetTransform() const;

		Maths::FVector3	color		= { 1.f, 1.f, 1.f };
		float				intensity	= 1.f;
		float				constant	= 0.0f;
		float				linear		= 0.0f;
        float               range       = 10.0f;
		float				quadratic	= 1.0f;
		float				cutoff		= 12.f;
		float				outerCutoff = 15.f;
		float				type		= 0.0f;
        float               shadowindex = 0.0f;

	protected:
		Maths::FTransform& m_transform;
	};
}