IDirect3D9Ex* p_Object;
IDirect3DDevice9Ex* p_Device;
D3DPRESENT_PARAMETERS p_Params;
ID3DXLine* p_Line;
ID3DXFont* pFontSmall;
ID3DXSprite* pSprite;
IDirect3DTexture9* pBackGroundTexture;
IDirect3DTexture9* pHelicopterTexture;
IDirect3DTexture9* pObstacleTexture;
IDirect3DTexture9* pDropTexture;

const MARGINS Margin = { 0, 0, 800, 600 };
#define CText (DT_CENTER|DT_NOCLIP)
#define LText (DT_LEFT|DT_NOCLIP) 
#define RText (DT_RIGHT|DT_NOCLIP)
String GetExePath();

int DirectXInit(HWND hWnd)
{

	Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object);

	RECT tClient;
	GetClientRect(hWnd,&tClient);
	clientwidth=tClient.right-tClient.left;
	clientheight=tClient.bottom-tClient.top;

	ZeroMemory(&p_Params, sizeof(p_Params));    
	p_Params.Windowed = TRUE;   
	p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;    
	p_Params.hDeviceWindow = hWnd;    
	p_Params.BackBufferWidth = clientwidth;    
	p_Params.BackBufferHeight = clientheight;    

	p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device);


	if(!p_Line)
		D3DXCreateLine(p_Device, &p_Line);
	p_Line->SetAntialias(1); 

	D3DXCreateFontA(p_Device, 18, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Calibri", &pFontSmall);

	D3DXCreateSprite(p_Device, &pSprite);

	String path=GetExePath();
	
	String background=path;
	background.append("\\background.png");

	String heli=path;
	heli.append("\\helicopter.png");

	String obstalce=path;
	obstalce.append("\\obstacle.png");

	String target=path;
	target.append("\\target.png");

	D3DXCreateTextureFromFileA(p_Device,background.c_str(),&pBackGroundTexture);
	D3DXCreateTextureFromFileA(p_Device,heli.c_str(),&pHelicopterTexture);
	D3DXCreateTextureFromFileA(p_Device,obstalce.c_str(),&pObstacleTexture);
	D3DXCreateTextureFromFileA(p_Device,target.c_str(),&pDropTexture);
	return 0;
}

typedef struct _D3DTLVERTEX{
	double fX;
	double fY;
	double fZ;
	double fRHW;
	D3DCOLOR Color;
	double fU;
	double fV;
}D3DTLVERTEX,*PD3DTLVERTEX;

void FillRGB(double x, double y, double w, double h, int r, int g, int b, int a) 
{ 
	D3DXVECTOR2 vLine[2]; 

	p_Line->SetWidth( w ); 

	vLine[0].x = x + w/2; 
	vLine[0].y = y; 
	vLine[1].x = x + w/2; 
	vLine[1].y = y + h; 

	p_Line->Begin( ); 
	p_Line->Draw( vLine, 2, D3DCOLOR_RGBA( r, g, b, a ) ); 
	p_Line->End( ); 
} 

int DrawShadowString(const char* String, int x, int y, int r, int g, int b, ID3DXFont* ifont)
{
	RECT Font;
	Font.left = x;
	Font.top = y;
	RECT Fonts;
	Fonts.left = x+1;
	Fonts.top = y;
	RECT Fonts1;
	Fonts1.left = x-1;
	Fonts1.top = y;
	RECT Fonts2;
	Fonts2.left = x;
	Fonts2.top = y+1;
	RECT Fonts3;
	Fonts3.left = x;
	Fonts3.top = y-1;
	ifont->DrawTextA(0, String, strlen(String), &Fonts3, DT_NOCLIP , D3DCOLOR_ARGB(255, 1, 1, 1));
	ifont->DrawTextA(0, String, strlen(String), &Fonts2, DT_NOCLIP, D3DCOLOR_ARGB(255, 1, 1, 1));
	ifont->DrawTextA(0, String, strlen(String), &Fonts1, DT_NOCLIP, D3DCOLOR_ARGB(255, 1, 1, 1));
	ifont->DrawTextA(0, String, strlen(String), &Fonts, DT_NOCLIP, D3DCOLOR_ARGB(255, 1, 1, 1));
	ifont->DrawTextA(0, String, strlen(String), &Font, DT_NOCLIP, D3DCOLOR_ARGB(255, r, g, b));
	return 0;
}

VOID WINAPI BuildVertex( double X, double Y, DWORD COLOR, PD3DTLVERTEX PD3DTLV, UINT Index )
{
	PD3DTLV[Index].fX			= X;
	PD3DTLV[Index].fY				= Y;
	PD3DTLV[Index].fZ			= 0.0f;
	PD3DTLV[Index].fU			= 0.0f;
	PD3DTLV[Index].fV			= 0.0f;
	PD3DTLV[Index].fRHW		= 1.0f;
	PD3DTLV[Index].Color		= COLOR;
}

BOOL WINAPI D3DDrawLine( double X1, double Y1, double X2, double Y2, DWORD COLOR, LPDIRECT3DDEVICE9 pDev )
{
	D3DTLVERTEX D3DTLV[20];

	BuildVertex( X1, Y1, COLOR, PD3DTLVERTEX(&D3DTLV), 0 );
	BuildVertex( X2, Y2, COLOR, PD3DTLVERTEX(&D3DTLV), 1 );

	pDev->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );

	pDev->DrawPrimitiveUP( D3DPT_LINELIST, 2, &D3DTLV, sizeof(D3DTLVERTEX) );

	return TRUE;
}

BOOL WINAPI D3DDrawFillRect( double X, double Y, DWORD COLOR, UINT Width, UINT Height, LPDIRECT3DDEVICE9 pDev)
{
	D3DTLVERTEX D3DTLV[20];

	BuildVertex( X-Width, Y+Height, COLOR, PD3DTLVERTEX(&D3DTLV), 0 );
	BuildVertex( X-Width, Y-Height, COLOR, PD3DTLVERTEX(&D3DTLV), 1 );
	BuildVertex( X+Width, Y-Height, COLOR, PD3DTLVERTEX(&D3DTLV), 2 );

	BuildVertex( X+Width, Y-Height, COLOR, PD3DTLVERTEX(&D3DTLV), 3 );
	BuildVertex( X+Width, Y+Height, COLOR, PD3DTLVERTEX(&D3DTLV), 4 );
	BuildVertex( X-Width, Y+Height, COLOR, PD3DTLVERTEX(&D3DTLV), 5 );

	pDev->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );

	pDev->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 6, &D3DTLV, sizeof(D3DTLVERTEX) );

	return TRUE;
}

BOOL WINAPI D3DDrawFillCircle( double X, double Y, DWORD COLOR,LPDIRECT3DDEVICE9 pDev, double Radius)
{
	D3DTLVERTEX D3DTLV[400];

	double X1	= X;
	double Y1	= Y;

	for( int i=0;i<=363;i+=3 )
	{
		double angle		= ( i / 57.3f );   

		double X2		= X + ( Radius * sin( angle ) );
		double Y2		= Y + ( Radius * cos( angle ) );        

		BuildVertex( X, Y, COLOR, PD3DTLVERTEX(&D3DTLV), i );
		BuildVertex( X1, Y1, COLOR, PD3DTLVERTEX(&D3DTLV), i + 1 );
		BuildVertex( X2, Y2, COLOR, PD3DTLVERTEX(&D3DTLV), i + 2 );

		Y1 = Y2;
		X1 = X2;
	}

	pDev->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID);
	pDev->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );
	pDev->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 363, &D3DTLV, sizeof(D3DTLVERTEX) );

	return TRUE;
}

BOOL WINAPI D3DDrawLineCircle( const D3DXVECTOR2 &Center,double Radius,DWORD color,unsigned short Sides /*= 30*/,LPDIRECT3DDEVICE9 pDev )
{
	double Angle = (360.0f/Sides)*(3.1415926f/180); //to radians

	double Cos = cos(Angle);
	double Sin = sin(Angle);

	D3DXVECTOR2 vec(Radius,0);

	for(unsigned short i = 0;i < Sides;++i)
	{
		D3DXVECTOR2 rot( Cos*vec.x - Sin*vec.y , Sin*vec.x + Cos*vec.y );
		rot += Center;
		vec += Center;
		//	DrawLine(vec,rot,color);
		D3DDrawLine(vec.x,vec.y,rot.x,rot.y,color,pDev);
		vec = rot - Center;
	}
	return TRUE;
}

D3DXVECTOR3 GetCirclePoint( D3DXVECTOR3 center, double radius, int index, int vertexCount )
{
	D3DXVECTOR3 point = center;
	point.x += sin( index * ( 2.0f * D3DX_PI ) / vertexCount ) * radius;
	point.z += cos( index * ( 2.0f * D3DX_PI ) / vertexCount ) * radius;

	return point;
}

void DrawTextFormat(int x, int y, DWORD Color, DWORD Style, const char *Format, ...)
{
	RECT rect,temprect;

	char Buffer[1024] = { '\0' };
	va_list va_alist;
	va_start(va_alist, Format);
	vsprintf_s(Buffer, Format, va_alist);
	va_end(va_alist);
	int height=	pFontSmall->DrawTextA(NULL,Buffer,-1,NULL, DT_CALCRECT,0xFFFFFFFF);
	SetRect(&rect, x, y-(height/2), x, y);
	pFontSmall ->DrawTextA(NULL, Buffer, -1, &rect, Style, Color);
	return;
}

String GetExePath() {
	char buffer[MAX_PATH];
	GetModuleFileName( NULL, buffer, MAX_PATH );
	String::size_type pos = String( buffer ).find_last_of( "\\/" );
	return String( buffer ).substr( 0, pos);
}
