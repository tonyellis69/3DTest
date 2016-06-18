#include "3DtestApp.h"

#include <glew.h>
#include <algorithm>
#include <string>
#include <vector>
#include <stdio.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

C3DtestApp::C3DtestApp() {
//	camera = NULL;
}

void C3DtestApp::onStart() {
	dataPath = homeDir + "Data\\";
	oldMouse = glm::vec2(0,0);
	turn = 0;

	cube = Engine.createCube(vec3(-3,0,-3),1.0f);
	Engine.createCube(vec3(3,0,-3),1.0f);


	Engine.currentCamera->setPos(glm::vec3(0,3,6));
	Engine.currentCamera->lookAt(vec3(0,-1,-3));
	
	yawAng = 0;

	Engine.createCylinder(vec3(0,0,-4),1,2,30);

	Engine.surfer.setPos(glm::vec3(0,0,-500));
	Engine.createVolume(dataPath + "map2.png");
	
}




void C3DtestApp::init() {
	//glGenVertexArrays(1, &vao);
	//glBindVertexArray(vao);

	
/*	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
*/
}

void C3DtestApp::keyCheck() {
	
	//player.moving = rNone;
	//player.upDownKey = none;
	//if ((Time - LastKeyCheck) < 20) //Prevent over-responsive controls
	//	return;
	
	if (KeyDown['A'] ) { 
		Engine.currentCamera->track(-5.1f);
	}

	if (KeyDown['D'] ) { 
		//glm::vec3 pos = Engine.currentCamera->getPos();
		//pos.x += 0.1f;
		Engine.currentCamera->track(5.1f);
	}

	if (KeyDown['W'] ) { 
		Engine.currentCamera->dolly(5.1f);
	}

	if (KeyDown['S'] ) { 
		Engine.currentCamera->dolly(-5.1f);
	}
	if (KeyDown[VK_SPACE] ) { 
		Engine.currentCamera->elevate(5.1f);
	}


	if (KeyDown['E'] ) { 
		yawAng = 1.1f;
		Engine.currentCamera->yaw(yawAng);
	}

	if (KeyDown['T'] ) { 
		yawAng = -1.1f;
		Engine.currentCamera->yaw(yawAng);
	}
	if (KeyDown['P']) {
		cube->rotate(2,glm::vec3(1,0,0));
	}
	if (KeyDown['Y']) {
		cube->rotate(2,glm::vec3(0,1,0));
	}
	if (KeyDown['R']) {
		cube->rotate(2,glm::vec3(0,0,1));
	}
}


void C3DtestApp::mouseMove(int x, int y, int key) {
	if (key == MK_LBUTTON) {
		glm::vec2 newMouse(x,-y);
		glm::vec2 dMouse = newMouse - oldMouse;
		oldMouse = newMouse;
		Engine.currentCamera->adjustTarget(dMouse);
	}

}


void C3DtestApp::onResize(int width, int height) {
	
}



void C3DtestApp::draw() {
	Engine.setDrawColour(engineWhite);
	Engine.drawString(50,50,"%d %d",(int)tmp.x,(int)tmp.y);

}



