#include "precomp.h"
#include "AnimatedGameObject.h"

#include "Shader.h"


real::AnimatedGameObject::AnimatedGameObject() : GameObject()
{

}
real::AnimatedGameObject::~AnimatedGameObject()
{
	for (int i = 0; i < m_animations.size(); i++)
	{
		delete m_animations[i];
	}
	delete m_animator;
}

void real::AnimatedGameObject::Init(Model& _model, const char* _startAnimationPath, real::E_GENERATECOLLISIONSHAPEMODE _generateCollisionShape, float _mass, glm::vec3 _startPos, glm::vec3 _startScale, glm::quat _startRotation)
{
	real::GameObject::Init(_model, _generateCollisionShape, _mass, _startPos, _startScale, _startRotation);

	real::Animation& startAnim = LoadAnimation(_startAnimationPath, 0);
	m_animator = new Animator(&startAnim);
	m_animator->Init();
}


void real::AnimatedGameObject::Tick(float _deltaTime)
{
	m_animator->UpdateAnimation(_deltaTime);
	real::GameObject::Tick(_deltaTime);
}

real::Animation& real::AnimatedGameObject::LoadAnimation(const char* _animationFilePath, unsigned int _animationListPosition, bool _looping)
{
	if (m_model == nullptr)
	{
		std::cout << "ERROR:ANIMATION::LOADING::CANNOT LOAD ANIMATION WITHOUT MODEL";
		abort();
	}
	real::Animation& newAnimation = *new real::Animation(_animationFilePath, m_model, _looping);
	if (m_animations.size() < _animationListPosition) m_animations.resize(_animationListPosition);
	m_animations.insert(m_animations.begin() + _animationListPosition, &newAnimation);
	return newAnimation;
}

void real::AnimatedGameObject::SetupDraw(glm::mat4 _viewProjection, std::vector<ReflectionProbe*> _activeReflectionProbes, bool _wireframe)
{
	const Shader& shader = m_model->GetShader();
	shader.use();

	auto transforms = m_animator->GetFinalBoneMatrices();
	for (int i = 0; i < transforms.size(); ++i)
		shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

	GameObject::SetupDraw(_viewProjection, _activeReflectionProbes, _wireframe);
}