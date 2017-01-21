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
	cube = Engine.createCube(vec3(-3,300,-3),1.0f);
	Engine.createCube(vec3(3,300,-3),1.0f);
	Engine.createCylinder(vec3(0,300,-4),1,2,30);

	//position the default camera
	Engine.currentCamera->setPos(vec3(0,303,6));
	Engine.currentCamera->lookAt(vec3(0,-1,-3));

	//Position FPS camera
	fpsCam.setPos(vec3(0,180,0));
	fpsCam.lookAt(vec3(0,-1,-3));
	fpsOn = false;
	selectChk = i32vec3(0,0,0);	
	mouseLook = false;

	terrain.EXTsuperChunkIsEmpty.Set(this,&C3DtestApp::superChunkIsEmpty);
	terrain.EXTchunkExists.Set(this,&C3DtestApp::chunkExists);
	terrain.EXTfreeChunkModel.Set(&Engine,&CEngine::freeModel);
	terrain.EXTcreateChunkMesh.Set(this,&C3DtestApp::createChunkMesh);

	initChunkShell();

	//load chunkCheck shader
	Engine.loadShader(vertex,dataPath + "chunkCheck.vert");
	Engine.loadShader(geometry,dataPath + "chunkCheck.geom");
	hChunkCheckProg = Engine.linkShaders();
	Engine.setCurrentShader(hChunkCheckProg);
	hCCsamplePosVec = Engine.getShaderDataHandle("nwSamplePos");
	hCCloDscale = Engine.getShaderDataHandle("LoDscale");

	double t = Engine.Time.milliseconds();

	initChunkGrid(cubesPerChunkEdge);

	terrain.setSizes(chunksPerSuperChunkEdge,cubesPerChunkEdge,cubeSize);
	terrain.createLayers(4,2,1);

	terrain.createAllChunks(); //nearly 4/5 of time spent here!
	//goes down massively with chunks per superchunk, so it's definitel a number-of-calls issue

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
	hChunkLoDscale = Engine.getShaderDataHandle("LoDscale");
	hChunkColour = Engine.getShaderDataHandle("inColour");
	hChunkSamplePos = Engine.getShaderDataHandle("samplePos");
	hChunkTriTable = Engine.getShaderDataHandle("hTriTableTex");

	//Upload data texture for chunk shader
	hTriTableTex = Engine.createDataTexture(intTex,16,256,&triTable);
	Engine.uploadDataTexture(hChunkTriTable,hTriTableTex);

	oldTime = Engine.Time.milliseconds();

	supWire = false;
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
	Engine.setShaderValue(hChunkCubeSize,chunk.cubeSize);

	//float LoDscale = 4;//chunk.LoD;
	float LoDscale = float(1 << chunk.LoD-1);
	Engine.setShaderValue(hChunkLoDscale,LoDscale);
	Engine.setShaderValue(hChunkColour,chunk.colour);
	Engine.setShaderValue(hChunkSamplePos,chunk.samplePos);
	Engine.setDataTexture(hTriTableTex);

	unsigned int hFeedBackBuf; 
	int vertsPerPrimitive = 3 * chunk.nAttribs;
	int maxMCverts = 16; //The maximum vertices needed for a surface inside one MC cube.
	int nVertsOut = cubesPerChunkEdge * cubesPerChunkEdge * cubesPerChunkEdge * maxMCverts;

	chunk.nTris = Engine.acquireFeedbackModel(shaderChunkGrid,sizeof(vec4)*nVertsOut*chunk.nAttribs,vertsPerPrimitive,chunk);	
	terrain.totalTris += chunk.nTris;
}


bool C3DtestApp::superChunkIsEmpty(vec3& sampleCorner, int LoD) {
	//return false;
	Engine.setCurrentShader(hChunkCheckProg);
	float LoDscale = LoD * chunksPerSuperChunkEdge;
	Engine.setShaderValue(hCCsamplePosVec,sampleCorner);
	Engine.setShaderValue(hCCloDscale,LoDscale);

	unsigned int primitives = Engine.drawModelCount(chunkShell);
	//TO DO: chunkshell is coarse, create a SCshell with more points
	if ((primitives == 0) || (primitives == shellTotalVerts*3))
		return true; //outside surface
	return false;
}


/** Return false if no side of this potential chunk is penetratedby the isosurface.*/
bool C3DtestApp::chunkExists(vec3& sampleCorner, int LoD) {
	//return true;
	//change to chunk test shader
	Engine.setCurrentShader(hChunkCheckProg);
	float LoDscale = LoD;
	//load shader values
	Engine.setShaderValue(hCCsamplePosVec,sampleCorner);
	Engine.setShaderValue(hCCloDscale,LoDscale);
	

	//Draw check grid 
	unsigned int primitives = Engine.drawModelCount(chunkShell);
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
	
	if (keyNow('8')) {
			advance(north);
		//	EatKeys();
		}
		if (keyNow('2') ) {
			advance(south);
			//EatKeys();
		}
		if (keyNow('6') ) {
			advance(east);
			//EatKeys();
		}
		if (keyNow('4')) {
			advance(west);
			//EatKeys();
		}
		if (KeyDown['5']) {
			advance(up);
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



		
		if (KeyDown['Z']) {
			terrain.advance(north);
			terrain.advance(west);
			EatKeys();
		}

		//if (keyNow('U') )
		if (KeyDown['U']) {
			supWire = !supWire;
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
	//Engine.setShaderValue(Engine.rNormalModelToCameraMatrix,normMatrix);
	mat4 mvp; 

	Chunk* chunk;

	int draw =0;

	for (int layer=0;layer<terrain.layers.size();layer++) {
		for (int sc=0;sc<terrain.layers[layer].superChunks.size();sc++) {
			for (int c=0;c<terrain.layers[layer].superChunks[sc]->chunkList.size();c++) {
				chunk = terrain.layers[layer].superChunks[sc]->chunkList[c];
				mvp = Engine.currentCamera->clipMatrix * chunk->worldMatrix; 
				Engine.setShaderValue(Engine.rMVPmatrix,mvp);
				Engine.setShaderValue(Engine.rNormalModelToCameraMatrix,mat3(chunk->worldMatrix));
				if ((chunk->hBuffer > 0) && (chunk->live)) {
						Engine.drawModel(*chunk);
					draw++;
				}
				if (chunk->tag == 131) {
					vec3 pos = chunk->getPos();
					watch::watch2 << pos.x << " " << pos.y <<" " << pos.z;


				}
			}
		} 
		
	}

	

	//wireframe drawing:
	Engine.setCurrentShader(hWireProg);
	float chunkRealSize = terrain.layers[0].superChunks[0]->cubesPerChunkEdge * terrain.layers[0].superChunks[0]->cubeSize;
	Engine.setShaderValue(hWireScale,vec3(chunkRealSize,chunkRealSize,chunkRealSize));
	Engine.setShaderValue(hWireColour,vec4(0,1,0,0.4f));

	//draw bounding boxes

		

	if (supWire) {
		//draw superchunk
		float siz; 
		
		for (int l=0;l<terrain.layers.size();l++) {
			Engine.setShaderValue(hWireColour,vec4(0,1,0,0.4f));
			siz = terrain.layers[l].cubeSize * cubesPerChunkEdge * chunksPerSuperChunkEdge;
			Engine.setShaderValue(hWireScale,vec3(siz));
			for (int s=0;s<terrain.layers[l].superChunks.size();s++) {
				chunkBB.setPos(terrain.layers[l].superChunks[s]->nwWorldPos);
				Engine.setShaderValue(hWireMVPmatrix,Engine.currentCamera->clipMatrix * chunkBB.worldMatrix);
				//if (terrain.layers[l].superChunks[s]->LoD == 1)
					Engine.drawModel(chunkBB);
				
			}
		
		}
	}


}

void C3DtestApp::drawChunkBB(CModel& model) {
	mat4 relativePos = model.worldMatrix *   terrain.worldMatrix;
	Engine.setShaderValue(hWireMVPmatrix,Engine.currentCamera->clipMatrix * relativePos);
	Engine.drawModel(model);
}

void C3DtestApp::advance(Tdirection direction) {
	//NB direction is the direction new terrain is scrolling *in* from
	//dir is the vector that moves the terrain *away* from that direction, to make space for new terrain 
	vec3 dir;
	switch (direction) {
		case north :	dir = vec3(0,0,1); //z=1 = out of the screen
						break;
		case south :	dir = vec3(0,0,-1); 
						break;
		case east :		dir = vec3(-1,0,0); 
						break;
		case west :		dir = vec3(1,0,0); 
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
		terrain.advance(direction); //
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
					direction = east;
				else
					direction = west;
				terrain.advance(direction);
			}
			
			if (outsideChunkBoundary.z) {
				if (pos.z > 0)
					direction = south;
				else
					direction = north;
				terrain.advance(direction);
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

/** Initialise a 3D grid of points to represent the cubes of a chunk in drawing. */
void C3DtestApp::initChunkGrid(int cubesPerChunkEdge) {
	int vertsPerEdge = cubesPerChunkEdge+1;
	int noVerts = vertsPerEdge * vertsPerEdge * vertsPerEdge;
	vec3* shaderChunkVerts = new vec3[noVerts];
	int i=0;
	for (float y=0;y<vertsPerEdge;y++) {
		for (float z=0;z<vertsPerEdge;z++) {
			for (float x=0;x<vertsPerEdge;x++) {
				shaderChunkVerts[i++] = vec3(x,y,z);
			}
		}
	}


	//create an index that creates a bottom square for each cube in the grid
	int noIndices = cubesPerChunkEdge * cubesPerChunkEdge *cubesPerChunkEdge * 5;

	unsigned short layer = vertsPerEdge * vertsPerEdge;
	unsigned short* index = new unsigned short[noIndices];
	i = 0;
	unsigned short vertNo = 0;
	do {
		index[i++] = vertNo;
		index[i++] = vertNo+1;
		index[i++] = vertNo+1+layer;
		index[i++] = vertNo+layer;

/*		index[i++] = vertNo+vertsPerEdge;
		index[i++] = vertNo+1+vertsPerEdge;
		index[i++] = vertNo+1+vertsPerEdge+layer;
		index[i++] = vertNo+vertsPerEdge+layer;
*/
		index[i++] = 65535; //signals the end of this line sequence

		vertNo++;
		if ( ((vertNo +1) % vertsPerEdge) == 0)
			vertNo++;
		if ( (vertNo % layer) == (vertsPerEdge * (vertsPerEdge-1)) )
			vertNo += vertsPerEdge;

	} while (i < noIndices);
	
	shaderChunkGrid.noVerts = noVerts;
	shaderChunkGrid.nAttribs = 1;
	shaderChunkGrid.indexSize = noIndices; 
	shaderChunkGrid.drawMode = GL_LINES_ADJACENCY;
	
	//Engine.(&shaderChunkGrid,shaderChunkVerts,index);
	Engine.storeIndexedModel(&shaderChunkGrid,shaderChunkVerts,index);

	delete[] shaderChunkVerts;
	delete[] index; 
}



C3DtestApp::~C3DtestApp() {

	
}





