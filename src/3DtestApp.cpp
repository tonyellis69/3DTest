#include "3DtestApp.h"

#include <glew.h>
#include <algorithm>
#include <string>
#include <vector>
#include <stdio.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/perpendicular.hpp>

#include "watch.h"

using namespace watch;

using namespace glm;

C3DtestApp::C3DtestApp() {

}

void C3DtestApp::onStart() {
	dataPath = homeDir + "Data\\";
	lastMousePos = glm::vec2(0,0);
	count = 0;
	//test objects, temporary
	cube = Engine.createCube(vec3(-3,0,-3),1.0f);
	Engine.createCube(vec3(3,0,-3),1.0f);
	Engine.createCylinder(vec3(0,0,-4),1,2,30);

	//position the default camera
	Engine.currentCamera->setPos(glm::vec3(0,3,6));
	Engine.currentCamera->lookAt(vec3(0,-1,-3));
	
	//load terrain shader
	Engine.loadShader(vertex,dataPath + "noise.vert");
	Engine.loadShader(frag,dataPath + "noise.frag");
	hTerrainProg =  Engine.linkShaders();

	//get terrain shader data handles
	Engine.setCurrentShader(hTerrainProg);
	hWorldPosVec = Engine.getShaderDataHandle("worldPos");
	hTerrainNoCubesInt = Engine.getShaderDataHandle("noCubes");

	//create a buffer to store blocks of terrain data
	int nChunkVerts = (nChunkCubes+1) *(nChunkCubes+1)* (nChunkCubes+1);
	terrainDataBuf = new float [nChunkVerts ];

	chunkCheckBuf = new int [(nChunkCubes+1) *(nChunkCubes+1)];

	chunkDataTmp = new vec4[nChunkCubes * nChunkCubes * nChunkCubes * 16 * 3];


	terrain2.EXTgetSampleData.Set(this,&C3DtestApp::getTerrainData);
	terrain2.EXTchunkExists.Set(this,&C3DtestApp::chunkExists);
	terrain2.EXTregisterChunkModel.Set(&Engine,&CEngine::storeModel);
	terrain2.EXTfreeChunkModel.Set(&Engine,&CEngine::freeModel);
	terrain2.EXTcreateChunkData.Set(this,&C3DtestApp::createChunkData);

	
	initChunkShell();
	//load chunkCheck shader
	Engine.loadShader(vertex,dataPath + "chunkCheck2.vert");
	Engine.loadShader(geometry,dataPath + "chunkCheck2.geom");
	hChunkCheckProg = Engine.attachShaders();

	const GLchar* feedbackVaryings2[1];
	feedbackVaryings2[0] = "feedBack"; 
	//glTransformFeedbackVaryings(hChunkCheckProg, 1, feedbackVaryings2, GL_INTERLEAVED_ATTRIBS);
	Engine.linkShaders(hChunkCheckProg);

	//get chunkCheck shader data handles
	Engine.setCurrentShader(hChunkCheckProg);
	hCCsamplePosVec = Engine.getShaderDataHandle("nwSamplePos");


	double t = Engine.Time.milliseconds();

	int chunksPerTerrainEdge = 8;
	terrain2.setSizes(chunksPerTerrainEdge,nChunkCubes,cubeSize);

	terrain2.createChunks();

	t = Engine.Time.milliseconds() - t;
	cerr << "\n time " << t;

	//load wireframe shader
	Engine.loadShader(vertex,dataPath + "wire.vert");
	Engine.loadShader(frag,dataPath + "wire.frag");
	hWireProg =  Engine.linkShaders();

	//get wireframe shader data handles
	Engine.setCurrentShader(hWireProg);
	hMVPmatrix  = Engine.getShaderDataHandle("mvpMatrix");
	hScale = Engine.getShaderDataHandle("scale");
	hWireColour = Engine.getShaderDataHandle("colour");

	createBB();


	//load chunk shader
	Engine.loadShader(vertex,dataPath + "chunk.vert");
	Engine.loadShader(geometry,dataPath + "chunk.geom");
//	Engine.loadShader(frag,dataPath + "chunk.frag");
	hChunkProg = Engine.attachShaders();
	const GLchar* feedbackVaryings[3];
	feedbackVaryings[0] = "gl_Position"; 
	feedbackVaryings[1] = "outColour";
	feedbackVaryings[2] = "normal";

	glTransformFeedbackVaryings(hChunkProg, 3, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
	Engine.linkShaders(hChunkProg);

	//get chunk shader data handles
	Engine.setCurrentShader(hChunkProg);
	hChunkCubeScale = Engine.getShaderDataHandle("cubeScale");
	hChunkColour = Engine.getShaderDataHandle("inColour");
	hChunkSamplePos = Engine.getShaderDataHandle("samplePos");
	hChunkHfactor = Engine.getShaderDataHandle("hFactor");

	triTableTex = Engine.createDataTexture(intTex,16,256,&triTable);

	glUniform1i(Engine.getShaderDataHandle("triTableTex"),0); 

		oldTime = Engine.Time.milliseconds();
		lastPress = 0;

	mouseLook = false;

	//FPS camera
	fpsCam.setPos(vec3(0,-140,0));
	fpsCam.lookAt(vec3(0,-1,-3));

	fpsOn = false;
	selectChk = i32vec3(0,0,0);

	
}

/** Create a wireframe bounding box.*/
void C3DtestApp::createBB() {
	chunkBB.drawMode = GL_LINES;
	vec4 boxV[8]  = {	vec4(0,0,0,1),
							vec4(1,0,0,1),
							vec4(1,1,0,1),
							vec4(0,1,0,1),
							vec4(0,0,1,1),
							vec4(1,0,1,1),
							vec4(1,1,1,1),
							vec4(0,1,1,1)};
	unsigned short index[12 * 2] = {0,1,1,2,2,3,3,0,
									4,5,5,6,6,7,7,4,
									6,2,7,3,5,1,4,0};

	chunkBB.noVerts = 8;
	chunkBB.indexSize = 24;
	Engine.storeIndexedModel(&chunkBB,boxV,index);
}

/** Return a pointer to a 3D block of terrain data with the given NW bottom corner.*/
float* C3DtestApp::getTerrainData(vec3& corner) {
	//switch to noise shader
	Engine.setCurrentShader(hTerrainProg);

	//set noise shader parameters
	Engine.setShaderValue(hWorldPosVec,corner);
	Engine.setShaderValue(hTerrainNoCubesInt,nChunkCubes);

	//render to 3D buffer
	vec3 terrainDataBufDimensions(nChunkCubes+1,nChunkCubes+1,nChunkCubes+1);
	Engine.renderTo3DTexture(terrainDataBufDimensions,terrainDataBuf);
	Engine.setCurrentShader(Engine.defaultProgram);
//	for (int i=0;i<4913;i++)
//		cerr << "\npoint: " << terrainDataBuf[i];

	return terrainDataBuf;
}

void C3DtestApp::createChunkData(Chunk& chunk, vec3& samplePos) {

	unsigned int hFeedBack;
	int nVertOut = 4096 * 16;
	float hFactor = 4;
	Engine.setCurrentShader(hChunkProg);

	vec4 cubeScale((float)cubeSize,(float)cubeSize,(float)cubeSize,1);
	Engine.setShaderValue(hChunkCubeScale,cubeScale);
	Engine.setShaderValue(hChunkColour,chunk.colour);
	Engine.setShaderValue(hChunkSamplePos,samplePos);
	Engine.setShaderValue(hChunkHfactor,hFactor);

	
	Engine.setDataTexture(triTableTex);
	unsigned int noTris = Engine.getGeometryFeedback(terrain2.shaderChunkGrid,sizeof(vec4)*nVertOut*chunk.nAttribs ,(char*)chunkDataTmp,hFeedBack);
				
	if (noTris == 0)
		return;
	//register buffer data in the name of our model
	chunk.noVerts = noTris*3;// nVertOut; //sizeof(data);
	chunk.drawMode = GL_TRIANGLES;
	//Engine.storeModel((CModel*)&chunk,chunkDataTmp);
	chunk.hBuffer = hFeedBack;
	Engine.Renderer.storeVertexLayout(chunk.hVAO,hFeedBack,0,chunk.nAttribs);

	

}

/** Return false if no side of this potential chunk is penetratedby the isosurface.*/
/*
bool C3DtestApp::chunkExists(vec3& corner) {
	//return true;
	//switch to chunk check shader
	Engine.setCurrentShader(hChunkCheckProg);

	//set chunk check shader parameters
	Engine.setShaderValue(hCCworldPosVec,corner);
	Engine.setShaderValue(hCCnoCubesInt,nChunkCubes);

	//pick a side to check
	
	//render to buffer
	vec2 terrainDataSize(nChunkCubes+1,nChunkCubes+1);
	int sliceSize = (nChunkCubes+1) *(nChunkCubes+1);

	for (int face=0;face < 6;face++) {
		int chk = 0;
		Engine.setShaderValue(hCCfaceInt,face);
		Engine.renderTo2DTexture(terrainDataSize,chunkCheckBuf);
		for (int c=0;c < sliceSize; c++ ) 
			chk += chunkCheckBuf[c];
		if ((chk != 0) &&(chk < sliceSize))
			return true;

	}
	//Engine.setCurrentShader(0); //TO DO: scrap!
	//return true;
	return false;
}
*/
bool C3DtestApp::chunkExists(vec3& sampleCorner) {
	//return true;
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
	
	//player.moving = rNone;
	//player.upDownKey = none;
	//if ((Time - LastKeyCheck) < 20) //Prevent over-responsive controls
	//	return;
	float moveInc = dT * 0.5f;
	
//	if (KeyDown['A'] ) { 
	if (keyNow('A')) {
		Engine.currentCamera->track(-moveInc);
	}

	if (KeyDown['D'] ) { 
		Engine.currentCamera->track(moveInc);
	}

	//if (KeyDown['W'] ) { 
	if (keyNow('W')) {
		Engine.currentCamera->dolly(moveInc);
	}

	if (KeyDown['S'] ) { 
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

//	if (Time - lastPress > 300) {


		lastPress = Time;
	//}

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
			float angle = (2 * length(mousePos) ) ;

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
			//int i = terrain.getIndex(selectChk.x,selectChk.y,selectChk.z);
			terrain2.recalc(selectChk);
		}

		
		if (KeyDown['Z']) {
			terrain2.scroll(north);
			terrain2.scroll(west);
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
	glm::mat3 normMatrix(terrain2.worldMatrix); 
	Engine.setShaderValue(Engine.rNormalModelToCameraMatrix,normMatrix);
	mat4 relativePos, mvp; 
	ChunkNode* node;
	for (int s=0;s<terrain2.superChunk.size();s++) {
		terrain2.superChunk[s].initNodeWalk();
		while (node = terrain2.superChunk[s].nextNode3()) {
			if ((node->pChunk != NULL) && (node->pChunk->live)) {
				relativePos = node->pChunk->worldMatrix *   terrain2.worldMatrix;
				mvp = Engine.currentCamera->clipMatrix * relativePos; 
				Engine.setShaderValue(Engine.rMVPmatrix,mvp);
				Engine.setShaderValue(Engine.rNormalModelToCameraMatrix,mat3(relativePos));
				if (node->pChunk->hBuffer > 0)
				Engine.drawModel(*node->pChunk);
			}
		}
	}

/*
	//chunk shader drawing
	//load shader
	Engine.setCurrentShader(hTmpProg);
//	vec4 cubeScale((float)cubeSize,(float)cubeSize,(float)cubeSize,1);
//	Engine.setShaderValue(hChunkCubeScale,cubeScale);

	for (int s=0;s<terrain2.superChunk.size();s++) {
		terrain2.superChunk[s].initNodeWalk();
		while (node = terrain2.superChunk[s].nextNode3()) {
			if (node->pChunk) {
				//terrain2.shaderChunkGrid.setPos(node->pChunk->getPos());
			//	relativePos = terrain2.shaderChunkGrid.worldMatrix *   terrain2.worldMatrix;
			//	Engine.setShaderValue(hChunkMVPmatrix,Engine.currentCamera->clipMatrix * relativePos);					
				//Engine.drawModel(terrain2.shaderChunkGrid);
				relativePos = node->pChunk->worldMatrix *   terrain2.worldMatrix;
				mvp = Engine.currentCamera->clipMatrix * relativePos; 
				Engine.setShaderValue(hTmpMatrix,mvp);
			//	Engine.drawModel(*node->pChunk);
				
			}
		}
	}
*/




	//wireframe drawing:
	Engine.setCurrentShader(hWireProg);
	float chunkRealSize = terrain2.superChunk[0].cubesPerChunkEdge * terrain2.superChunk[0].cubeSize;
	Engine.setShaderValue(hScale,vec3(chunkRealSize,chunkRealSize,chunkRealSize));
	Engine.setShaderValue(hWireColour,vec4(0,1,0,0.4f));
	float half = terrain2.superChunk[0].chunkSize * terrain2.superChunk[0].sizeInChunks.x/2;
	vec3 offset(half);
	vec3 nwCorner;
	vec3 rowStartPos;
	vec3 layerStartPos;

	

	
	//draw bounding boxes
	for (int s=0;s<terrain2.superChunk.size();s++) {
		terrain2.superChunk[s].initNodeWalk();
		while (node = terrain2.superChunk[s].nextNode3()) {
			if (node->pChunk) {
				chunkBB.setPos(node->pChunk->getPos());
				Engine.setShaderValue(hWireColour,vec4(0,1,0,0.4f));
				if (node->boundary & 64)
						Engine.setShaderValue(hWireColour,vec4(1,1,0,1));
				else if (node->boundary & 128)
						Engine.setShaderValue(hWireColour,vec4(0,0,1,1));
				else if (node == terrain2.superChunk[s].nwRoot)
						Engine.setShaderValue(hWireColour,vec4(1,0,1,1));
						
				drawChunkBB(chunkBB);
			}
		}
	}





}

void C3DtestApp::drawChunkBB(CModel& model) {
	mat4 relativePos = model.worldMatrix *   terrain2.worldMatrix;

	Engine.setShaderValue(hMVPmatrix,Engine.currentCamera->clipMatrix * relativePos);


	//call engine drawWireFrame func
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

	vec3 movement = dir * float(10.0f);

	int chunkDist = nChunkCubes * cubeSize;

	terrain2.translate(movement);

	vec3 pos = terrain2.getPos();
	bvec3 insideChunkBoundary = glm::greaterThan(glm::abs(pos),vec3(chunkDist,chunkDist,chunkDist));

	if (glm::any(insideChunkBoundary)) {
		vec3 posMod;
	//	posMod = glm::mod(pos,vec3(chunkDist,chunkDist,chunkDist));
		//glm::mod seems to turn negative remainders positive
		posMod.x = fmod(pos.x,chunkDist);
		posMod.y = fmod(pos.y,chunkDist);
		posMod.z = fmod(pos.z,chunkDist);
		terrain2.setPos(posMod );
		terrain2.scroll(direction);
		//return;
	}
	

	terrain2.translate(movement);
}

void C3DtestApp::Update() {
	
	terrain2.update();

	if (fpsOn) {
		Tdirection direction = none;

		int chunkDist = nChunkCubes * cubeSize;

		vec3 pos = fpsCam.getPos();
		bvec3 outsideChunkBoundary = glm::greaterThan(glm::abs(pos),vec3(chunkDist,chunkDist,chunkDist));

		if (outsideChunkBoundary.x || outsideChunkBoundary.z) {
			vec3 posMod;
			posMod.x = fmod(pos.x,chunkDist);
			//posMod.y = fmod(pos.y,chunkDist);
			posMod.y = pos.y;
			posMod.z = fmod(pos.z,chunkDist);
			fpsCam.setPos(posMod );

			//work out direction
			if (outsideChunkBoundary.x) {
				if (pos.x > 0)
					direction = west;
				else
					direction = east;
				terrain2.scroll(direction);
				//return;
			}
			
			if (outsideChunkBoundary.z) {
				if (pos.z > 0)
					direction = south;
				else
					direction = north;
				terrain2.scroll(direction);
				//return;
			}

			
		//return;
		}
	
		
	}
}

void C3DtestApp::initChunkShell() {
	float vertsPerEdge = nChunkCubes+1;
	shellTotalVerts = std::pow(vertsPerEdge,3) - std::pow(vertsPerEdge-2,3);
	vec4* shell = new vec4[shellTotalVerts];
	int v=0;
	for(int y=0;y<vertsPerEdge;y++) {
		for(int x=0;x<vertsPerEdge;x++) {
			shell[v++] = vec4(x,y,0,1);
			shell[v++] = vec4(x,y,nChunkCubes,1);
		}
		for(int z=1;z<nChunkCubes;z++) {
			shell[v++] = vec4(0,z,y,1);
			shell[v++] = vec4(nChunkCubes,z,y,1);
		}
	}
		
	for(int x=1;x<nChunkCubes;x++) {
		for(int z=1;z<nChunkCubes;z++) {
			shell[v++] = vec4(x,z,0,1);
			shell[v++] = vec4(x,z,nChunkCubes,1);
		}
	}

//	for (int p=0;p<shellTotalVerts;p++)
	//	cerr << "\n" << shell[p].x << " " << shell[p].y << " " << shell[p].z;
	
	chunkShell.noVerts = shellTotalVerts;
	chunkShell.nAttribs = 1;
	chunkShell.indexSize = 0; 
	chunkShell.drawMode = GL_POINTS;
	Engine.storeModel(&chunkShell,shell);
	delete[] shell;

}


C3DtestApp::~C3DtestApp() {
	delete terrainDataBuf;
	delete chunkCheckBuf;
	delete chunkDataTmp;
}





