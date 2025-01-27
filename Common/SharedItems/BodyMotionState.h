#pragma once

//I thought i needed this class for interpolating the position with low tick rates hence the commented code, but somehow it is already interpolating it.

class BodyMotionState : public btMotionState
{
public:
	BodyMotionState(const btTransform& _initialTransform) : m_newTransform(_initialTransform)
	{
	}

	void getWorldTransform(btTransform& _worldTrans) const override
	{
		_worldTrans = m_newTransform;
	}

	void setWorldTransform(const btTransform& _worldTrans) override
	{
		//oldTransform = newTransform;
		m_newTransform = _worldTrans;
		//graphicTransform = newTransform;
		//timer = 0;
	}

	void setWorldTransform(const btTransform& _worldTrans, bool _teleport)
	{
		if (_teleport)
		{
			//oldTransform = worldTrans;
			m_newTransform = _worldTrans;
			//graphicTransform = newTransform;
			//timer = 0;
		}
		else setWorldTransform(_worldTrans);
	}



//	//Calculates the interpolated transform
//	btTransform getInterpolatedTransform() const
//	{
//		float alpha = 1;
//		btTransform interpolated = oldTransform;
//		interpolated.setOrigin(oldTransform.getOrigin() * (1 - alpha) + newTransform.getOrigin() * alpha);
//		return interpolated;
//	}
//
	//void Tick(float deltaTime)
	//{
	//	graphicTransform = newTransform;
	//}
   //btTransform graphicTransform;
private:
//	float timer{ 0 };
//	btTransform oldTransform;
	btTransform m_newTransform;
};




