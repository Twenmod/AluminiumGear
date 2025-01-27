#pragma once
#include "RealMath.h"
#include "BodyMotionState.h"

namespace real
{
	enum class E_GENERATECOLLISIONSHAPEMODE
	{
		DONT_GENERATE,
		GENERATE_PRECISE,
		GENERATE_HULL
	};

	class Model;
	class Shader;
	class ObjectManager;
	class ReflectionProbe;

	class GameObject
	{
	public:
		GameObject();
		virtual ~GameObject();
		virtual void Init(float mass = 0, glm::vec3 startPos = glm::vec3(0), glm::vec3 startScale = glm::vec3(1), glm::quat startRotation = glm::identity<glm::quat>());
		virtual void Init(Model& model, E_GENERATECOLLISIONSHAPEMODE generateCollisionShape = E_GENERATECOLLISIONSHAPEMODE::DONT_GENERATE, float mass = 0, glm::vec3 startPos = glm::vec3(0), glm::vec3 startScale = glm::vec3(1), glm::quat startRotation = glm::identity<glm::quat>(), glm::vec3 collisionShapeLoadOffset = glm::vec3(0));
		virtual void SetupDraw(glm::mat4 viewProjection, std::vector<ReflectionProbe*> activeReflectionProbes = std::vector<ReflectionProbe*>(), bool wireframe = false);
		virtual void Tick(float deltaTime);


		//Getter and setters
		void SetPosition(glm::vec3 _position, bool _teleport = true)
		{
			btTransform trans = GetGraphicTransform();
			trans.setOrigin(GlmVecToBtVec(_position));
			m_body->getMotionState()->setWorldTransform(trans);
			m_body->setWorldTransform(trans);
			if (_teleport) m_body->setInterpolationWorldTransform(trans);
		};
		glm::vec3 GetPosition() const { 
			return BtVecToGlmVec(GetGraphicTransform().getOrigin());
		}
		void SetScale(glm::vec3 _scale)
		{
			m_scale = _scale;
			if (m_body->getCollisionShape() != nullptr) m_body->getCollisionShape()->setLocalScaling(GlmVecToBtVec(_scale));
		};
		glm::vec3 GetScale() const
		{
			return m_scale;
		}

		void Rotate(float _angle, glm::vec3 _axis)
		{
			btTransform transform = GetGraphicTransform();
			transform.setRotation(GlmQuatToBtQuat(glm::rotate(BtQuatToGlmQuat(GetGraphicTransform().getRotation()), _angle, _axis)));
			m_body->getMotionState()->setWorldTransform(transform);
			m_body->setWorldTransform(transform);
			m_body->setInterpolationWorldTransform(transform);
		}
		virtual void SetRotation(glm::quat _quaternionRotation) { 
			btTransform transform = m_body->getInterpolationWorldTransform();
			transform.setRotation(GlmQuatToBtQuat(_quaternionRotation)); 
			m_body->getMotionState()->setWorldTransform(transform);
			m_body->setWorldTransform(transform);
			m_body->setInterpolationWorldTransform(transform);
		}
		glm::quat GetRotation() const { return BtQuatToGlmQuat(GetGraphicTransform().getRotation()); }


		void DrawImGuiProperties();

		Model* GetModel() { return m_model; }

		void SetModelOffset(glm::vec3 _modelOffset, bool _local = true) { m_modelOffset = _modelOffset;  m_modelOffsetIsLocal = _local; }

		std::string m_name = "GameObject";
		btRigidBody* m_body{ nullptr };
	protected:
		btTransform GetGraphicTransform() const
		{
			btTransform transform;
			m_body->getMotionState()->getWorldTransform(transform);
			return transform;
		}
		Model* m_model{ nullptr };
		btTransform* m_tranform{ nullptr };
		glm::vec3 m_modelOffset{ glm::vec3(0) };
		bool m_modelOffsetIsLocal = false;
		glm::vec3 m_scale{ glm::vec3(1) };
	};
}