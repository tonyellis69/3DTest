#include "3DtestApp.h"

#include <glew.h>
#include <algorithm>
#include <string>
#include <vector>
#include <stdio.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/perpendicular.hpp>
#include <glm/gtx/common.hpp>

#include "watch.h"

using namespace watch;

using namespace glm;

C3DtestApp::C3DtestApp() {

}

void C3DtestApp::onStart() {
	dataPath = homeDir + "Data\\";
	lastMousePos = glm::vec2(0,0);

	//test objects, temporary
	cube = Engine.createCube(vec3(-3,0,-3),1.0f);
	Engine.createCube(vec3(3,0,-3),1.0f);
	Engine.createCylinder(vec3(0,0,-4),1,2,30);

	//position the default camera
	Engine.currentCamera->setPos(glm::vec3(0,3,6));
	Engine.currentCamera->lookAt(vec3(0,-1,-3));

	//Position FPS camera
	fpsCam.setPos(vec3(0,-140,0));
	fpsCam.lookAt(vec3(0,-1,-3));
	fpsOn = false;
	selectChk = i32vec3(0,0,0);	
	mouseLook = false;

	terrain.EXTchunkExists.Set(this,&C3DtestApp::chunkExists);
	terrain.EXTregisterChunkModel.Set(&Engine,&CEngine::storeModel);
	terrain.EXTfreeChunkModel.Set(&Engine,&CEngine::freeModel);
	terrain.EXTcreateChunkMesh.Set(this,&C3DtestApp::createChunkMesh);
	terrain.EXTregisterIndexedModel.Set(this,&C3DtestApp::registerIndexedModel);

	initChunkShell();

	//load chunkCheck shader
	Engine.loadShader(vertex,dataPath + "chunkCheck.vert");
	Engine.loadShader(geometry,dataPath + "chunkCheck.geom");
	hChunkCheckProg = Engine.linkShaders();
	Engine.setCurrentShader(hChunkCheckProg);
	hCCsamplePosVec = Engine.getShaderDataHandle("nwSamplePos");

	double t = Engine.Time.milliseconds();

	terrain.setSizes(chunksPerSuperChunkEdge,cubesPerChunkEdge,cubeSize);
	terrain.create();

	t = Engine.Time.milliseconds() - t;
	cerr << "\n time " << t;

	//load wireframe shader
	Engine.loadShader(vertex,dataPath + "wire.vert");
	Engine.loadShader(frag,dataPath + "wire.frag");
	hWireProg =  Engine.linkShaders();

	//get wireframe shader data handles
	Engine.setCurrentShader(hWireProg);
	hWireMVPmatrix  = Engine.getShaderDataHandle("mvpMatrix");
	hWireScale = Engine.getShaderDataHandle("scale");
	hWireColour = Engine.getShaderDataHandle("colour");

	createBB();

	//load chunk shader
	Engine.loadShader(vertex,dataPath + "chunk.vert");
	Engine.loadShader(geometry,dataPath + "chunk.geom");
	hChunkProg = Engine.attachShaders();
	const char* feedbackVaryings[3];
	feedbackVaryings[0] = "gl_Position"; 
	feedbackVaryings[1] = "outColour";
	feedbackVaryings[2] = "normal";
	Engine.setFeedbackData(hChunkProg, 3, feedbackVaryings);
	Engine.linkShaders(hChunkProg);

	//get chunk shader data handles
	Engine.setCurrentShader(hChunkProg);
	hChunkCubeSize = Engine.getShaderDataHandle("cubeSize");
	hChunkColour = Engine.getShaderDataHandle("inColour");
	hChunkSamplePos = Engine.getShaderDataHandle("samplePos");
	hChunkTriTable = Engine.getShaderDataHandle("hTriTableTex");

	//Upload data texture for chunk shader
	hTriTableTex = Engine.createDataTexture(intTex,16,256,&triTable);
	Engine.uploadDataTexture(hChunkTriTable,hTriTableTex);

	oldTime = Engine.Time.milliseconds();
}

/** Create a wireframe bounding box.*/
void C3DtestApp::createBB() {
	chunkBB.drawMode = GL_LINES;
	vec3 boxV[8]  = {	vec3(0,0,0),
							vec3(1,0,0),
							vec3(1,1,0),
							vec3(0,1,0),
							vec3(0,0,1),
							vec3(1,0,1),
							vec3(1,1,1),
							vec3(0,1,1)};
	unsigned short index[12 * 2] = {0,1,1,2,2,3,3,0,
									4,5,5,6,6,7,7,4,
									6,2,7,3,5,1,4,0};

	chunkBB.noVerts = 8;
	chunkBB.indexSize = 24;
	Engine.storeIndexedModel(&chunkBB,boxV,index);
}


/*  Create a mesh for this chunk, and register it with the renderer.  */
void C3DtestApp::createChunkMesh(Chunk& chunk) {
	Engine.setCurrentShader(hChunkProg);
	Engine.setShaderValue(hChunkCubeSize,cubeSize);
	Engine.setShaderValue(hChunkColour,chunk.colour);
	Engine.setShaderValue(hChunkSamplePos,chunk.samplePos);
	Engine.setDataTexture(hTriTableTex);

	unsigned int hFeedBackBuf; 
	int vertsPerPrimitive = 3 * chunk.nAttribs;
	int nVertsOut = cubesPerChunkEdge * cubesPerChunkEdge * cubesPerChunkEdge * maxMCverts;

	Engine.getFeedbackModel(terrain.shaderChunkGrid,sizeof(vec4)*nVertsOut*chunk.nAttribs,vertsPerPrimitive,chunk);			
}

/** Return false if no side of this potential chunk is penetratedby the isosurface.*/
bool C3DtestApp::chunkExists(vec3& sampleCorner) {
	//change to chunk test shader
	Engine.setCurrentShader(hChunkCheckProg);

	//load shader values
	Engine.setShaderValue(hCCsamplePosVec,sampleCorner);

	GLuint query;
	glGenQueries(1, &query);
	glBeginQuery(GL_PRIMITIVES_GENERATED, query);

	//Draw check grid 
	Engine.drawModel(chunkShell);

	glEndQuery(GL_PRIMITIVES_GENERATED);
	GLuint primitives;
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives);

	if ((primitives == 0) || (primitives == shellTotalVerts*3))
		return false; //outside surface
	return true;

}



void C3DtestApp::keyCheck() {
	
	float moveInc = dT * 0.5f;
	
	if (keyNow('A')) {
		Engine.currentCamera->track(-moveInc);
	}

	if (keyNow('D') ) { 
		Engine.currentCamera->track(moveInc);
	}
 
	if (keyNow('W')) {
		Engine.currentCamera->dolly(moveInc);
	}

	if (keyNow('S') ) { 
		Engine.currentCamera->dolly(-moveInc);
	}
	if (KeyDown[VK_SPACE] ) { 
		Engine.currentCamera->elevate(moveInc);
	}


	if (KeyDown['E'] ) { 
		Engine.currentCamera->yaw(yawAng * dT);
	}

	if (KeyDown['T'] ) { 
		Engine.currentCamera->yaw(-yawAng *dT);
	}

	float rot = dT * 0.2f;
	if (KeyDown['P']) {
		cube->rotate(rot,glm::vec3(1,0,0));
	}
	if (KeyDown['Y']) {
		cube->rotate(rot,glm::vec3(0,1,0));
	}
	if (KeyDown['R']) {
		cube->rotate(rot,glm::vec3(0,0,1));
	}



	if (mouseKey == MK_LBUTTON)
	{	
		if (!mouseLook) {
			mouseLook = true;
			//mouse capture on
			mouseCaptured(true);
			oldMousePos = vec2(mouseX,mouseY);
			showMouse(false);
			//centre mouse
			setMousePos(-1,-1);

		} else {
			//find mouse movement
			glm::vec2 mousePos((mouseX - (viewWidth/2)), ((viewHeight/2) - mouseY)  );
			if (mousePos.x == 0 && mousePos.y == 0)
				return;
			float angle = (0.1f * length(mousePos) ) ;

			//move camera
			vec3 perp = normalize(vec3(mousePos.y,-mousePos.x,0));
			Engine.currentCamera->freeRotate(perp,angle );

			setMousePos(-1,-1);
		}
				
	}
	else { //mouselook key not down so
		if (mouseLook) {
			mouseCaptured(false);
			mouseLook = false;
			setMousePos(oldMousePos.x,oldMousePos.y);
			showMouse(true);

		}

	}
	
	if (KeyDown['8']) {
			scroll(north);
			EatKeys();
		}
		if (KeyDown['2']) {
			scroll(south);
			EatKeys();
		}
		if (KeyDown['6']) {
			scroll(east);
			EatKeys();
		}
		if (KeyDown['4']) {
			scroll(west);
			EatKeys();
		}
		if (KeyDown['5']) {
			scroll(up);
		}

		if (KeyDown['1']) {
			fpsOn = !fpsOn;
			if (fpsOn)
				Engine.currentCamera = &fpsCam;
			else
				Engine.currentCamera = Engine.defaultCamera;
			EatKeys();

		}


		if (KeyDown['J']) { 
			selectChk.x--;
			EatKeys();
		}
		if (KeyDown['L']) { 
			selectChk.x++;
			EatKeys();
		}
		if (KeyDown['I']) {
			selectChk.z--;
			EatKeys();
		}
		if (KeyDown['K']) { 
			selectChk.z++;
			EatKeys();
		}
		if (KeyDown['H']) { 
			selectChk.y++;
			EatKeys();
		}
		if (KeyDown['N']) { 
			selectChk.y--;
			EatKeys();
		}

		if (KeyDown['F']) {
			terrain.recalc(selectChk);
		}

		
		if (KeyDown['Z']) {
			terrain.scroll(north);
			terrain.scroll(west);
			EatKeys();
		}


		selectChk = glm::mod(vec3(selectChk),vec3(15,5,15));
			
}

void C3DtestApp::OnKeyDown(unsigned int wParam, long lParam) {

	

};


/** Called when mouse moves. */
void C3DtestApp::mouseMove(int x, int y, int key) {
	return;

}


void C3DtestApp::onResize(int width, int height) {
	
}



void C3DtestApp::draw() {

	//draw chunk
	Engine.setStandard3dShader();
	glm::mat3 normMatrix(terrain.worldMatrix); 
	Engine.setShaderValue(Engine.rNormalModelToCameraMatrix,normMatrix);
	mat4 relativePos, mvp; 
	ChunkNode* node;
	for (int s=0;s<terrain.superChunk.size();s++) {
		terrain.superChunk[s].initNodeWalk();
		while (node = terrain.superChunk[s].nextNode3()) {
			if ((node->pChunk != NULL) && (node->pChunk->live)) {
				relativePos = node->pChunk->worldMatrix *   terrain.worldMatrix;
				mvp = Engine.currentCamera->clipMatrix * relativePos; 
				Engine.setShaderValue(Engine.rMVPmatrix,mvp);
				Engine.setShaderValue(Engine.rNormalModelToCameraMatrix,mat3(relativePos));
				if (node->pChunk->hBuffer > 0)
					Engine.drawModel(*node->pChunk);
			}
		}
	}



	//wireframe drawing:
	Engine.setCurrentShader(hWireProg);
	float chunkRealSize = terrain.superChunk[0].cubesPerChunkEdge * terrain.superChunk[0].cubeSize;
	Engine.setShaderValue(hWireScale,vec3(chunkRealSize,chunkRealSize,chunkRealSize));
	Engine.setShaderValue(hWireColour,vec4(0,1,0,0.4f));

	//draw bounding boxes
	for (int s=0;s<terrain.superChunk.size();s++) {
		terrain.superChunk[s].initNodeWalk();
		while (node = terrain.superChunk[s].nextNode3()) {
			if (node->pChunk) {
				chunkBB.setPos(node->pChunk->getPos());
				Engine.setShaderValue(hWireColour,vec4(0,1,0,0.4f));
				if (node->boundary & 64)
						Engine.setShaderValue(hWireColour,vec4(1,1,0,1));
				else if (node->boundary & 128)
						Engine.setShaderValue(hWireColour,vec4(0,0,1,1));
				else if (node == terrain.superChunk[s].nwRoot)
						Engine.setShaderValue(hWireColour,vec4(1,0,1,1));
						
				drawChunkBB(chunkBB);
			}
		}
	}





}

void C3DtestApp::drawChunkBB(CModel& model) {
	mat4 relativePos = model.worldMatrix *   terrain.worldMatrix;
	Engine.setShaderValue(hWireMVPmatrix,Engine.currentCamera->clipMatrix * relativePos);
	Engine.drawModel(model);
}

void C3DtestApp::scroll(Tdirection direction) {
	vec3 dir;
	switch (direction) {
		case north :	dir = vec3(0,0,1); 
						break;
		case south :	dir = vec3(0,0,-1); 
						break;
		case east :		dir = vec3(1,0,0); 
						break;
		case west :		dir = vec3(-1,0,0); 
						break;
	}

	//Move terrain in given direction
	vec3 movement = dir * float(10.0f); 
	terrain.translate(movement);
	vec3 pos = terrain.getPos();


	int chunkDist = cubesPerChunkEdge * cubeSize; //span of a chunk in world space
	bvec3 outsideChunkBoundary = glm::greaterThan(glm::abs(pos),vec3(chunkDist,chunkDist,chunkDist));

	//If terrain has moved by the length of a chunk
	if (glm::any(outsideChunkBoundary)) {
		vec3 posMod;
		posMod = glm::mod(pos,vec3(chunkDist,chunkDist,chunkDist)); //glm::mod seems to turn negative remainders positive
		posMod.x = fmod(pos.x,chunkDist);
		posMod.y = fmod(pos.y,chunkDist);
		posMod.z = fmod(pos.z,chunkDist);
		terrain.setPos(posMod ); //secretly move terrain back before scrolling to ensure it scrolls on the spot
		terrain.scroll(direction); //
	}
}

/** Called every frame. Mainly use this to scroll terrain if we're moving in first-person mode*/
void C3DtestApp::Update() {
	
	terrain.update();

	if (fpsOn) {
		Tdirection direction = none;

		int chunkDist = cubesPerChunkEdge * cubeSize;

		vec3 pos = fpsCam.getPos();
		bvec3 outsideChunkBoundary = glm::greaterThan(glm::abs(pos),vec3(chunkDist,chunkDist,chunkDist));

		//has viewpoint moved beyond the length of one chunk?
		if (outsideChunkBoundary.x || outsideChunkBoundary.z) {
			vec3 posMod;
			posMod.x = fmod(pos.x,chunkDist);
			posMod.y = pos.y;
			posMod.z = fmod(pos.z,chunkDist);
			fpsCam.setPos(posMod ); //secretly reposition viewpoint prior to scrolling terrain

			//work out direction to scroll-in new terrain from
			if (outsideChunkBoundary.x) {
				if (pos.x > 0)
					direction = west;
				else
					direction = east;
				terrain.scroll(direction);
			}
			
			if (outsideChunkBoundary.z) {
				if (pos.z > 0)
					direction = south;
				else
					direction = north;
				terrain.scroll(direction);
			}
		}	
	}
}

/** Prepare a hollow shell of vertices to use in checks for empty chunks. */
void C3DtestApp::initChunkShell() {
	float vertsPerEdge = cubesPerChunkEdge+1;
	shellTotalVerts = std::pow(vertsPerEdge,3) - std::pow(vertsPerEdge-2,3);
	vec3* shell = new vec3[shellTotalVerts];
	int v=0;
	for(int y=0;y<vertsPerEdge;y++) {
		for(int x=0;x<vertsPerEdge;x++) {
			shell[v++] = vec3(x,y,0);
			shell[v++] = vec3(x,y,cubesPerChunkEdge);
		}
		for(int z=1;z<cubesPerChunkEdge;z++) {
			shell[v++] = vec3(0,z,y);
			shell[v++] = vec3(cubesPerChunkEdge,z,y);
		}
	}
		
	for(int x=1;x<cubesPerChunkEdge;x++) {
		for(int z=1;z<cubesPerChunkEdge;z++) {
			shell[v++] = vec3(x,z,0);
			shell[v++] = vec3(x,z,cubesPerChunkEdge);
		}
	}

	
	chunkShell.noVerts = shellTotalVerts;
	chunkShell.nAttribs = 1;
	chunkShell.indexSize = 0; 
	chunkShell.drawMode = GL_POINTS;
	Engine.storeModel(&chunkShell,shell);
	delete[] shell;

}

void C3DtestApp::registerIndexedModel(CModel* model, vec3* verts, unsigned short* index) {
	Engine.storeIndexedModel(model,verts,index);
}


C3DtestApp::~C3DtestApp() {

	
}





