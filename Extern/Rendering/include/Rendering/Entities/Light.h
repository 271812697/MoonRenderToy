/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Maths/FVector3.h>
#include <Maths/FMatrix4.h>
#include <Maths/FTransform.h>

#include "Rendering/Entities/Entity.h"
#include "Rendering/Settings/ELightType.h"
#include "Rendering/Resources/Texture.h"
#include "Rendering/HAL/Framebuffer.h"
#include <Rendering/Entities/Camera.h>

namespace Rendering::Entities
{
	/**
	* Data structure that can represent any type of light
	*/
	struct Light : public Entity
	{
		Maths::FVector3 color{ 1.f, 1.f, 1.f };
		float intensity = 1.f;
		float constant = 0.0f;
		float linear = 0.0f;
		float quadratic = 1.0f;
		float cutoff = 12.f;
		float outerCutoff = 15.f;
		Settings::ELightType type = Settings::ELightType::POINT;

		bool castShadows = false;
		float shadowAreaSize = 50.0f;
		bool shadowFollowCamera = true;
		int16_t shadowMapResolution = 8192;

		/**
		* Update the content of the shadow cache
		* @param p_shadowMapResolution
		* @param p_camera
		*/
		void UpdateShadowData(const Rendering::Entities::Camera& p_camera);

		/**
		* Returns the light space matrix
		*/
		const Maths::FMatrix4& GetLightSpaceMatrix() const;

		/**
		* Returns the framebuffer used to render the shadow map
		*/
		const Rendering::HAL::Framebuffer& GetShadowBuffer() const;

		/**
		* Generate the light matrix, ready to send to the GPU
		*/
		Maths::FMatrix4 GenerateMatrix() const;

		/**
		* Calculate the light effect range from the quadratic falloff equation
		*/
		float GetEffectRange() const;

		Maths::FMatrix4 lightSpaceMatrix;
		std::unique_ptr<Rendering::HAL::Framebuffer> shadowBuffer;
	};
}
