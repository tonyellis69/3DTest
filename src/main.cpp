#include "3DtestApp.h"


#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW 
#endif


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow) {

	C3DtestApp* testApp = new C3DtestApp;

	testApp->SetWindow(1200,800,"3D test app :-)",WIN_NORMAL);//GAME_SCREEN_WIDTH,GAME_SCREEN_HEIGHT,"Asteroids :-)",WIN_BORDERLESS);

	testApp->start();

	delete testApp;
	_CrtDumpMemoryLeaks();
	return true;
}


