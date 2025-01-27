#include "precomp.h"
#include "BTDebugDrawer.h"

#include "ShaderManager.h"
#include "Camera.h"

real::BtDebugDrawer::BtDebugDrawer() : m_debugMode(DBG_DrawWireframe)
{
	glGenBuffers(1, &m_vbo);
}

void real::BtDebugDrawer::Init(real::ShaderManager& _shaderManager)
{
	m_shaderManager = &_shaderManager;
}

void real::BtDebugDrawer::Draw()
{
	if (m_lineData.size() <= 0) return;

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_lineData.size(), &m_lineData[0], GL_DYNAMIC_DRAW);

	Shader& shader = *m_shaderManager->GetShader("Line");
	shader.use();

	// Set up the vertex attributes (position)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));

	shader.setMat4("MVP", m_vp);

	glLineWidth(1);
	glDrawArrays(GL_LINES, 0, static_cast<int>(m_lineData.size()/6));

	//Reset data
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	unsigned int lineSize = static_cast<unsigned int>(m_lineData.size());
	m_lineData.clear();
	m_lineData.reserve(lineSize);

}

void real::BtDebugDrawer::drawLine(const btVector3& _from, const btVector3& _to, const btVector3& _color)
{
	std::initializer_list<GLfloat>lineVertices = {
		_from.getX(), _from.getY(), _from.getZ(), _color.getX(), _color.getY(), _color.getZ(),
		_to.getX(), _to.getY(), _to.getZ(), _color.getX(), _color.getY(), _color.getZ()
	};
	m_lineData.insert(m_lineData.end(), lineVertices);
}