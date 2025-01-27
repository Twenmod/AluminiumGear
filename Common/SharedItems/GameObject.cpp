#include "precomp.h"
#include "Shader.h"
#include "TextureManager.h"
#include "GameObject.h"

#include "Model.hpp"

#include "ImGui/imgui.h"


real::GameObject::GameObject()
{
	btTransform startTransform;
	startTransform.setIdentity();
	BodyMotionState* motionState = new BodyMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(0, motionState, nullptr);
	m_body = new btRigidBody(rbInfo);
	m_body->setUserPointer(this);
}
real::GameObject::~GameObject()
{
	if (m_body != nullptr)
	{
		delete m_body->getMotionState();
		delete m_body;
	}
}

void real::GameObject::Init(float _mass, glm::vec3 _startPos, glm::vec3 _startScale, glm::quat _startRotation)
{
	SetPosition(_startPos);
	SetScale(_startScale);
	SetRotation(_startRotation);
	m_body->setMassProps(_mass, btVector3(0, 0, 0));

}

void real::GameObject::Init(Model& _objectModel, E_GENERATECOLLISIONSHAPEMODE _generateCollisionShape, float _mass, glm::vec3 _startPos, glm::vec3 _startScale, glm::quat _startRotation, glm::vec3 _collisionShapeLoadOffset)
{
	m_model = &_objectModel;

	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(GlmVecToBtVec(_startPos));
	startTransform.setRotation(GlmQuatToBtQuat(_startRotation));

	m_scale = _startScale;
	btVector3 inertia(0, 0, 0);
	if (_generateCollisionShape != E_GENERATECOLLISIONSHAPEMODE::DONT_GENERATE)
	{
		bool convex = _generateCollisionShape == E_GENERATECOLLISIONSHAPEMODE::GENERATE_HULL;
		btCompoundShape* shape = m_model->loadModelShape(convex, _collisionShapeLoadOffset);
		shape->setLocalScaling(GlmVecToBtVec(_startScale));
		m_body->setCollisionShape(shape);
		shape->calculateLocalInertia(_mass, inertia);
	}
	SetPosition(_startPos);
	SetScale(_startScale);
	SetRotation(_startRotation);
	m_body->setMassProps(_mass, inertia);
	m_body->setFriction(1.f);

}

void real::GameObject::SetupDraw(glm::mat4 _viewProjection, std::vector<ReflectionProbe*> _activeReflectionProbes, bool)
{
	if (m_model != nullptr)
	{
		//Calculate model
		glm::mat4 transform = glm::mat4(1);
		btTransform trans = GetGraphicTransform();
		btVector3 origin = trans.getOrigin();
		glm::vec3 objectposition(origin.getX(), origin.getY(), origin.getZ());
		if (!m_modelOffsetIsLocal) objectposition += m_modelOffset;
		transform = glm::translate(transform, objectposition);

		transform *= glm::mat4_cast(GetRotation());
		if (m_modelOffsetIsLocal) transform = glm::translate(transform, m_modelOffset);
		transform = glm::scale(transform, GetScale());
		//Set shader info
		const Shader& shader = m_model->GetShader();
		shader.use();
		shader.setMat4("Model", transform);
		shader.setMat3("NormalMatrix", glm::mat3(glm::transpose(glm::inverse(transform))));
		shader.setMat4("VP", _viewProjection);
	}
}

void real::GameObject::Tick(float)
{
	//dynamic_cast<BodyMotionState*>(body->getMotionState())->Tick(deltaTime);
}

void real::GameObject::DrawImGuiProperties()
{
	float mass = m_body->getMass();
	bool modifiedMass = ImGui::InputFloat("Mass", &mass);
	if (modifiedMass) m_body->setMassProps(modifiedMass, btVector3(0, 0, 0));

	float* pos[3];
	pos[0] = const_cast<float*>(&m_body->getWorldTransform().getOrigin().getX());
	pos[1] = const_cast<float*>(&m_body->getWorldTransform().getOrigin().getX());
	pos[2] = const_cast<float*>(&m_body->getWorldTransform().getOrigin().getX());
	bool modifiedPos = ImGui::InputFloat3("Position", *pos);
	if (modifiedPos)
	{
		m_body->setLinearVelocity(btVector3(0, 0, 0));
	}

	float* rot[3];
	glm::vec3 eulerRot = glm::degrees(glm::eulerAngles(GetRotation()));
	rot[0] = &eulerRot.x;
	rot[1] = &eulerRot.y;
	rot[2] = &eulerRot.z;
	bool modifiedRot = ImGui::InputFloat3("Rotation", *rot);
	if (modifiedRot)
	{
		SetRotation(glm::quat(glm::radians(eulerRot)));
	}

	if (m_body->getCollisionShape() != nullptr)
	{
		float* iscale[3];
		iscale[0] = const_cast<float*>(&m_body->getCollisionShape()->getLocalScaling().getX());
		iscale[1] = const_cast<float*>(&m_body->getCollisionShape()->getLocalScaling().getY());
		iscale[2] = const_cast<float*>(&m_body->getCollisionShape()->getLocalScaling().getZ());
		ImGui::InputFloat3("Scale", *iscale);
	}
}