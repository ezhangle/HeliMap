// HeliMap.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "HeliMap.h"
#include "GPSMathematics.h"
#include "DXDrawing.h"


#define MAX_LOADSTRING 100
bool painting=false;
// Global Variables:
HWND hWnd;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void GetGPSImage();

ObjectsToDraw* objects;
VOID CreateConsole( )
{
	int hConHandle = 0; 
	HANDLE lStdHandle = 0;
	FILE *fp = 0;
	AllocConsole( );
	lStdHandle = GetStdHandle( STD_OUTPUT_HANDLE );
	hConHandle = _open_osfhandle( PtrToUlong( lStdHandle ), _O_TEXT );
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );
}  

int Render()
{
	p_Device->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_XRGB(210,250,210), 1.0f, 0);
	p_Device->BeginScene();

	objects->CalculateScreens();
	Point Heli=objects->GetHeliScreen();
	
	if(!pHelicopterTexture)
	{
		DrawTextFormat(Heli.x,Heli.y,D3DCOLOR_RGBA(255,0,0,255),CText,"X");
	}else{ //if we've got an image
		pSprite->Begin(D3DXSPRITE_ALPHABLEND); 
		pSprite->Draw(pHelicopterTexture, 0,&D3DXVECTOR3(40,25,0), &D3DXVECTOR3(Heli.x,Heli.y,0),0xFFFFFFFF );
		pSprite->Flush();
		pSprite->End();	
	}
	

	for(int i=0;i<objects->GetObstacleScreenSize();i++)
	{
		Point ObstaclePoint=objects->GetObstacleScreenAt(i);
		if(!pObstacleTexture)
		{
			D3DDrawLineCircle(D3DXVECTOR2(ObstaclePoint.x,ObstaclePoint.y),5,D3DCOLOR_RGBA(255,0,0,255),50,p_Device);
			D3DDrawLineCircle(D3DXVECTOR2(ObstaclePoint.x,ObstaclePoint.y),10,D3DCOLOR_RGBA(255,0,0,255),50,p_Device);
		}else{
			pSprite->Begin(D3DXSPRITE_ALPHABLEND); 
			pSprite->Draw(pObstacleTexture, 0,&D3DXVECTOR3(12.5,12.5,0), &D3DXVECTOR3(ObstaclePoint.x,ObstaclePoint.y,0),0xFFFFFFFF );
			pSprite->Flush();
			pSprite->End();	
		}
		
		DrawTextFormat(ObstaclePoint.x,ObstaclePoint.y+15,D3DCOLOR_RGBA(0,0,255,255),CText,"Obstacle %d",i+1);
	}

	for (int i = 0; i < objects->GetDropScreenSize(); i++)
	{
		Point DropPoint=objects->GetDropScreenAt(i);

		pSprite->Begin(D3DXSPRITE_ALPHABLEND); 
		pSprite->Draw(pDropTexture, 0,&D3DXVECTOR3(12.5,12.5,0), &D3DXVECTOR3(DropPoint.x,DropPoint.y,0),0xFFFFFFFF );
		pSprite->Flush();
		pSprite->End();	
		DrawTextFormat(DropPoint.x,DropPoint.y+20,D3DCOLOR_RGBA(255,0,0,255),CText,"Drop %d",i+1);
	}

	Coordinate HeliGPS=objects->GetHeli();
	Coordinate Obst1=objects->GetObstalceAt(1);
	Point Obst1Screen=objects->GetObstacleScreenAt(1);
	
	float distance=GetDistance(HeliGPS,Obst1);
	float bearing=GetBearing(HeliGPS,Obst1);
	float degrees=abs(bearing-90);
	float x=(float) cos(degrees*(M_PI/180))*distance*3280.84f;
	float y=(float) sin(degrees*(M_PI/180))*distance*3280.84f;

	D3DDrawLine(Heli.x,Heli.y,Obst1Screen.x,Heli.y,D3DCOLOR_RGBA(255,0,0,255),p_Device);
	D3DDrawLine(Obst1Screen.x,Heli.y,Obst1Screen.x,Obst1Screen.y,D3DCOLOR_RGBA(255,0,0,255),p_Device);

	DrawTextFormat(Heli.x+((Obst1Screen.x-Heli.x)/2),Heli.y-5,D3DCOLOR_RGBA(0,0,255,255),CText,"%.1f",abs(x));
	DrawTextFormat(Obst1Screen.x,Heli.y+((Obst1Screen.y-Heli.y)/2),D3DCOLOR_RGBA(0,0,255,255),CText,"%.1f",abs(y));
	
	p_Device->EndScene();
	p_Device->PresentEx(0, 0, 0, 0, 0);

	return 0;
}

void Initialize()
{
	Coordinate Helicoord=Coordinate(40.828223,-79.914226);

	Coordinate Gate1A=Coordinate(40.828223,-79.914226);
	Coordinate Gate1B=Coordinate(40.828376,-79.914217);
	Coordinate Gate2A=Coordinate(40.828189,-79.914397);
	Coordinate Gate2B=Coordinate(40.828189,-79.914621);
	Coordinate Gate3A= Coordinate(40.828246,-79.914888);
	Coordinate Gate3B=Coordinate(40.828244, -79.915044);
	Coordinate Gate4A=Coordinate(40.828478,-79.914754);
	Coordinate Gate4B=Coordinate(40.828536 ,-79.914618);

	Coordinate Drop1=Coordinate(40.828281,-79.914337);
	Coordinate Drop2=Coordinate(40.828010,-79.914470);
	Coordinate Drop3=Coordinate(40.828303,-79.915040);
	Coordinate Drop4=Coordinate(40.828431,-79.914580);

	std::vector<Coordinate>obstacles;
	obstacles.push_back(Gate1A);
	obstacles.push_back(Gate1B);
	obstacles.push_back(Gate2A);
	obstacles.push_back(Gate2B);
	obstacles.push_back(Gate3A);
	obstacles.push_back(Gate3B);
	obstacles.push_back(Gate4A);
	obstacles.push_back(Gate4B);

	std::vector<Coordinate> Drops;
	Drops.push_back(Drop1);
	Drops.push_back(Drop2);
	Drops.push_back(Drop3);
	Drops.push_back(Drop4);

	objects=new ObjectsToDraw(obstacles,Drops,Helicoord);
}
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,_In_ LPTSTR lpCmdLine,_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_HELIMAP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HELIMAP));

	Initialize();
	// Main message loop:
	while(true)
	{
		if(PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Render();
	}

	return (int) msg.wParam;
}

void GetGPSImage()
{
	char buffer[1024];
	//lat [float], lon [float], zoom [int], sizex [int, sizey [int]
	sprintf_s(buffer,"http://maps.googleapis.com/maps/api/staticmap?center=%f,%f&zoom=%d&size=%dx%d&sensor=false",40.300281,-79.75767,21,clientwidth,clientheight);

}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HELIMAP));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_HELIMAP);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   DirectXInit(hWnd);
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   
   CreateConsole();
   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		

		RECT tClient,tSize;
		GetClientRect(hWnd,&tClient);
		clientwidth=tClient.right-tClient.left;
		clientheight=tClient.bottom-tClient.top;

		GetWindowRect(hWnd, &tSize);
		MoveWindow(hWnd,tSize.left,tSize.top,tSize.right-tSize.left,tSize.right-tSize.left,TRUE);

		ZeroMemory(&p_Params, sizeof(p_Params));    
		p_Params.Windowed = TRUE;   
		p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;    
		p_Params.hDeviceWindow = hWnd;    
		p_Params.BackBufferWidth = clientwidth;    
		p_Params.BackBufferHeight = clientheight;    
		
		p_Line->OnLostDevice();
		pFontSmall->OnLostDevice();
		pSprite->OnLostDevice();
		
	

		p_Device->Reset(&p_Params);
		
		p_Line->OnResetDevice();
		pFontSmall->OnResetDevice();
		pSprite->OnResetDevice();
		if(!p_Line)
		D3DXCreateLine(p_Device, &p_Line);

		if(!pFontSmall)
		D3DXCreateFontA(p_Device, 18, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Calibri", &pFontSmall);

		if(!pSprite)
		D3DXCreateSprite(p_Device, &pSprite);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
