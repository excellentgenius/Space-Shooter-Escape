#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include "gamelib.h"

//Constant variables
const int MAXFONTNUM = 16;		//Maximum number of font styles
const int MAXMODELNUM = 64;		//Maximun number of models
const int MAXTIMERNUM = 16;		//Maximum number of timers

//Global variables
LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
float g_aspect = 1.0f;
LPD3DXFONT g_pFonts[MAXFONTNUM];
LPD3DXSPRITE g_pSprite = NULL;

Model g_models[MAXMODELNUM];
LPDIRECTINPUT8 g_pDI = NULL;
LPDIRECTINPUTDEVICE8 g_pDIDevice = NULL;
char g_keys[256];
DWORD g_setTimer[MAXTIMERNUM];

//Function prototypes
LRESULT WINAPI MsgProc(HWND, UINT, WPARAM, LPARAM);

//Initialize window
HRESULT InitD3DWindow(HINSTANCE hInstance, LPCTSTR windowTitle, int windowWidth, int windowHeight) {
	//Initialize g_pFonts
	ZeroMemory(&g_pFonts, sizeof(LPD3DXFONT) * MAXFONTNUM);

	//Initialize g_models
	ZeroMemory(&g_models, sizeof(Model) * MAXMODELNUM);

	//Create the main application window
	WNDCLASS wc;

	//Set attributes for the main application window
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)MsgProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = _T("D3D Window Class");

	if (!RegisterClass(&wc)) {
		MessageBox(0, _T("ウインドウクラスの登録が失敗しました"), _T("Warning"), 0);
		return E_FAIL;
	}

	//Create window
	//Move the window to center of the screen
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int centerXPosition = screenWidth / 2 - windowWidth / 2;
	int centerYPosition = screenHeight / 2 - windowHeight / 2;

	HWND hWnd = CreateWindow(_T("D3D Window Class"), windowTitle, WS_EX_TOPMOST | WS_SYSMENU, centerXPosition, centerYPosition, windowWidth, windowHeight, NULL, NULL, wc.hInstance, NULL);

	if (!hWnd) {
		MessageBox(0, _T("ウインドウの作成が失敗しました"), _T("Warning"), 0);
		return E_FAIL;
	}

	//set aspect ratio for view port
	g_aspect = (float)windowWidth / (float)windowHeight;

	//Create D3D9
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) {
		MessageBox(0, _T("IDirect3D9インターフェイスの作成が失敗しました"), _T("Warning"), 0);
		return E_FAIL;
	}

	//Create D3D9 Device
	//Set attributes for the main application window
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice))) {
		if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice))) {
			MessageBox(0, _T("デバイスの作成が失敗しました"), _T("Warning"), 0);
			return E_FAIL;
		}
	}

	//Enable use of z buffer
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	ShowWindow(hWnd, SW_SHOWDEFAULT);

	//Initialize DirectInput
	if (FAILED(DirectInput8Create(wc.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_pDI, NULL))) {
		MessageBox(0, _T("DirectInput8の作成が失敗しました"), _T("Warning"), 0);
		return E_FAIL;
	}
	if (FAILED(g_pDI->CreateDevice(GUID_SysKeyboard, &g_pDIDevice, NULL))) {
		MessageBox(0, _T("DirectInput8デバイスの作成が失敗しました"), _T("Warning"), 0);
		return E_FAIL;
	}
	g_pDIDevice->SetDataFormat(&c_dfDIKeyboard);
	g_pDIDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	//Create sprite for text
	if (FAILED(D3DXCreateSprite(g_pd3dDevice, &g_pSprite))) {
		MessageBox(0, _T("スプライトの作成が失敗しました"), _T("Warning"), 0);
		return E_FAIL;
	}

	return S_OK;
}

//Create font
int CreateGameFont(LPCTSTR fontName, int height, UINT weight) {
	int index;
	//Check for unused slot in g_pFonts
	for (index = 0; index < MAXFONTNUM; ++index) {
		if (g_pFonts[index] == NULL) {
			break;
		}
	}

	if (index >= MAXFONTNUM) {
		return -1;
	}

	//Create font
	if (FAILED(D3DXCreateFont(g_pd3dDevice, -height, 0, weight, 1, FALSE, DEFAULT_CHARSET,
							  OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
							  fontName, &g_pFonts[index]))) {
		return -1;
	}

	return index;
}

//Load model (X File & Textures)
int LoadModel(LPCTSTR filename) {
	//Check for unused slot in g_models
	int index;
	for (index = 0; index < MAXMODELNUM; ++index) {
		if (!g_models[index].used) {
			break;
		}
	}

	if (index >= MAXMODELNUM) {
		return -1;
	}

	LPD3DXBUFFER pD3DXMtrlBuffer;	//Temporary buffer for material

	//Load X File
	if (FAILED(D3DXLoadMeshFromX(filename, D3DXMESH_SYSTEMMEM, g_pd3dDevice, NULL, &pD3DXMtrlBuffer, NULL, &g_models[index].numMaterials, &g_models[index].pMesh))) {
		MessageBox(NULL, _T("Xファイルが見つかりません"), _T("Warning"), MB_OK);
		return -1;
	}

	//Create arrays for materials and textures
	D3DXMATERIAL *d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	int num = g_models[index].numMaterials;
	g_models[index].pMaterials = new D3DMATERIAL9[num];
	if (g_models[index].pMaterials == NULL) {
		return -1;
	}
	g_models[index].pTextures = new LPDIRECT3DTEXTURE9[num];
	if (g_models[index].pTextures == NULL) {
		return -1;
	}

	for (int i = 0; i < num; ++i) {
		//Copy material information from the temporary buffer
		g_models[index].pMaterials[i] = d3dxMaterials[i].MatD3D;
		//Set ambient color
		g_models[index].pMaterials[i].Ambient = g_models[index].pMaterials[i].Diffuse;

		//Load textures
		g_models[index].pTextures[i] = NULL;
		if (d3dxMaterials[i].pTextureFilename != NULL && lstrlenA(d3dxMaterials[i].pTextureFilename) > 0) {
			if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice, d3dxMaterials[i].pTextureFilename, &g_models[index].pTextures[i]))) {
				MessageBox(NULL, _T("テクスチャが見つかりません"), _T("Warning"), MB_OK);
				return -1;
			}
		}
	}

	pD3DXMtrlBuffer->Release();
	g_models[index].used = TRUE;

	return index;
}

//Render model
void RenderModel(int index) {
	if (!g_models[index].used) {
		return;
	}

	for (DWORD i = 0; i < g_models[index].numMaterials; ++i) {
		g_pd3dDevice->SetMaterial(&g_models[index].pMaterials[i]);
		g_pd3dDevice->SetTexture(0, g_models[index].pTextures[i]);
		g_models[index].pMesh->DrawSubset(i);
	}
}

//Get control of keyboard & get state of keyboard
const char *GetKeyState() {
	HRESULT hr = g_pDIDevice->Acquire();
	if ((hr == DI_OK) || (hr == S_FALSE)) {
		g_pDIDevice->GetDeviceState(sizeof(g_keys), &g_keys);
		return g_keys;
	}
	return NULL;
}

//Set timer
void SetTimer(int index, DWORD time) {
	//Restrict the number of timers
	if (index > MAXTIMERNUM) {
		return;
	}

	g_setTimer[index] = timeGetTime() + time;
}

//Check whether the assigned time has passed or not
BOOL isTimerOver(int index) {
	if (g_setTimer[index] > timeGetTime()) {
		return FALSE;
	}

	return TRUE;
}

//Look up the passed time
DWORD getPassedTime(int index) {
	return timeGetTime() - g_setTimer[index];
}

//Clean up memory
void CleanupD3D() {
	for (int i = 0; i < MAXFONTNUM; ++i) {
		if (g_pFonts[i]) {
			g_pFonts[i]->Release();
		}
	}

	if (g_pSprite) {
		g_pSprite->Release();
	}

	for (int i = 0; i < MAXMODELNUM; ++i) {
		if (g_models[i].used) {
			if (g_models[i].pMaterials != NULL) {
				delete[] g_models[i].pMaterials;
			}
			if (g_models[i].pTextures != NULL) {
				for (DWORD j = 0; j < g_models[i].numMaterials; ++j)
				{
					g_models[i].pTextures[j]->Release();
				}
				delete[] g_models[i].pTextures;
			}
			if (g_models[i].pMesh != NULL) {
				g_models[i].pMesh->Release();
			}
		}
	}

	if (g_pd3dDevice != NULL) {
		g_pd3dDevice->Release();
	}

	if (g_pD3D != NULL) {
		g_pD3D->Release();
	}

	if (g_pDIDevice != NULL) {
		g_pDIDevice->Unacquire();
		g_pDIDevice->Release();
	}

	if (g_pDI != NULL) {
		g_pDI->Release();
	}
}

//Window Procedure
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_DESTROY:
			CleanupD3D();
			PostQuitMessage(0);
			return 0;
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE) {
				::DestroyWindow(hWnd);
				return 0;
			}
	}

	//Handle the unhandled messages
	return DefWindowProc(hWnd, msg, wParam, lParam);
}