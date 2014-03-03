#pragma once


#include "resource.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <D3DX11core.h>
#include <D3DX11tex.h>
#include <d3d9.h>
#include <d3dx10.h>
#include <D3DX10math.h>
#include <d3dx9core.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <math.h>
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <limits>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#ifndef UNICODE  
typedef std::string String; 
#else
typedef std::wstring String; 
#endif

int clientwidth;
int clientheight;