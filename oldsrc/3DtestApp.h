#pragma once

#include "BaseApp.h"
#include <glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

	


class C3DtestApp : public  CBaseApp {
public:
	C3DtestApp();
	void onStart();
	void keyCheck();
	void mouseMove(int x, int y, int key);

	void onResize(int width, int height);
	void draw();
	
	void createCube();
	void init();

	GLuint vertexBufferObject;
	GLuint theProgram;
	GLuint vao;
	string dataPath;

	GLuint perspectiveMatrixUnif;
	
	GLuint modelToWorldMatrixUnif;
	GLuint worldToCameraMatrixUnif;
	GLuint cameraToClipMatrixUnif;

	glm::mat4 modelMatrix;
	glm::mat4 camMatrix;

	glm::mat4 perspectiveMatrix;
	CCamera* camera;
	
	CModel* cube;
	
	
	GLuint indexBufferObject;

	glm::vec2 oldMouse;
	glm::vec2 tmp;
	float yawAng;
	float turn;
};
