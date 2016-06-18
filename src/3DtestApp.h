#pragma once

#include "BaseApp.h"
#include "superChunk.h" //TO replace with terrain.h
#include "terrain.h"
#include <glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

	


class C3DtestApp : public  CBaseApp {
public:
	C3DtestApp();
	~C3DtestApp();
	void onStart();
	void createBB();
	void keyCheck();
	void mouseMove(int x, int y, int key);
	void OnKeyDown(unsigned int wParam, long lParam);

	void onResize(int width, int height);
	void draw();
	void drawChunkBB(CModel& model);
	void scroll(Tdirection direction);
	void Update();
	
	float* getTerrainData(glm::vec3& corner);
	void createChunkData(Chunk& chunk, glm::vec3& samplePos);
	bool chunkExists(glm::vec3& corner);

	string dataPath; ///<Filepath to the Data folder
	
	CModel* cube; //TO DO: get rid of this
	
	glm::vec2 lastMousePos; ///<Used to track current mouse movement

	int hTerrainProg; ///<Handle for noise-generating shader.
	int hWireProg; ///<Handle for wireframe shader.
	int hMVPmatrix;
	int hScale;
	int hWireColour;
	int hChunkCheckProg;
	int hCCworldPosVec;
	int hCCfaceInt;
	int hCCnoCubesInt;
	int hTerrainNoCubesInt;

	int hChunkProg;
	int hChunkCubeScale;
	int hChunkColour;
	int hChunkSamplePos;
	int hChunkHfactor;

	int hTmpProg,hTmpMatrix;


	CTerrain terrain2;
	float* terrainDataBuf; ///<Points to buffer for noise data.
	int hWorldPosVec; ///<Handle for position vector in noise shader.
	int hChunkSampleSize; ///<Handle for size of sample in noise shader.

	CModel chunkBB; ///<Wireframe bounding box for chunks.

	int* chunkCheckBuf; ///<Points to buffer for chunk check data.

	double oldTime;
	double lastPress;

	bool mouseLook; ///<True if mouselook mode on.
	glm::vec2 oldMousePos;


	CCamera fpsCam;
	bool fpsOn; ///<True when we're using the fps came

	glm::i32vec3 selectChk; ///<Index of the chunk we're selecting

	glm::vec4* chunkDataTmp; ///<Reusable tmp store for genenerated chunk data.
};

const float yawAng = 0.22f;
const int nChunkCubes = 16; ///<Number of cubes along a chunk edge
const int cubeSize = 5; ///<Size of cubes in pixels.