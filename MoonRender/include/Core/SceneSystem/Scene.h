#pragma once
#include <Core/API/ISerializable.h>
#include <Core/ECS/Actor.h>
#include <Core/ECS/Components/CCamera.h>
#include <Core/ECS/Components/CLight.h>
#include <Core/ECS/Components/CModelRenderer.h>
#include <Core/ECS/Components/CPostProcessStack.h>
#include <Core/ECS/Components/CReflectionProbe.h>
#include <Core/SceneSystem/BvhService.h>
#include <Rendering/Geometry/bbox.h>
#include <Rendering/Geometry/bvh.h>
#include <Rendering/Geometry/ray.h>

namespace Core::SceneSystem
{
	class Scene : public API::ISerializable
	{
	public:
		struct FastAccessComponents
		{
			std::vector<ECS::Components::CModelRenderer*> modelRenderers;
			std::vector<ECS::Components::CCamera*> cameras;
			std::vector<ECS::Components::CLight*> lights;
			std::vector<ECS::Components::CPostProcessStack*> postProcessStacks;
			std::vector<ECS::Components::CReflectionProbe*> reflectionProbes;
		};
		Scene();
		~Scene();
		void AddDefaultCamera();
		void AddDefaultLights();
		void AddDefaultReflections();
		void AddDefaultPostProcessStack();
		void AddDefaultSkysphere();
		void AddDefaultAtmosphere();
		void Play();
		bool IsPlaying() const;
		void Update(float p_deltaTime);
		void FixedUpdate(float p_deltaTime);
		void LateUpdate(float p_deltaTime);
		ECS::Actor& CreateActor();
		ECS::Actor& CreateActor(const std::string& p_name, const std::string& p_tag = "");
		bool DestroyActor(ECS::Actor& p_target);
		void CollectGarbages();
		ECS::Actor* FindActorByName(const std::string& p_name) const;
		ECS::Actor* FindActorByTag(const std::string& p_tag) const;
		ECS::Actor* FindActorByID(int64_t p_id) const;
		std::vector<std::reference_wrapper<ECS::Actor>> FindActorsByName(const std::string& p_name) const;
		std::vector<std::reference_wrapper<ECS::Actor>> FindActorsByTag(const std::string& p_tag) const;
		Core::ECS::Components::CCamera* FindMainCamera() const;
		void OnComponentAdded(ECS::Components::AComponent& p_compononent);
		void OnComponentRemoved(ECS::Components::AComponent& p_compononent);
		std::vector<Core::ECS::Actor*>& GetActors();
		const FastAccessComponents& GetFastAccessComponents() const;
		virtual void OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_root) override;
		virtual void OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_root) override;
		void BuildSceneBvh();
		
		bool RayHit(const ::Rendering::Geometry::Ray& ray, HitRes& outRes);
		bool RayIteratorHit(const ::Rendering::Geometry::Ray& ray, HitRes& outRes);
		::Rendering::Geometry::Bvh* GetBvh();
		BvhService* GetBvhService() { return bvhService; }
	private:
		int64_t m_availableID = 1;
		bool m_isPlaying = false;
		std::vector<ECS::Actor*> m_actors;
		FastAccessComponents m_fastAccessComponents;
		BvhService*bvhService=nullptr;
		::Rendering::Geometry::bbox m_sceneBoundingBox;
	};
}