#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#include <tchar.h>
#include <dinput.h>
#include <objbase.h>

//structs
struct Model {
	LPD3DXMESH pMesh;					//Mesh
	D3DMATERIAL9 *pMaterials;			//Material array
	LPDIRECT3DTEXTURE9 *pTextures;		//Texture array
	DWORD numMaterials;					//Number of materials
	BOOL used;							//Being used or not
};

//Global variables
extern LPDIRECT3D9 g_pD3D;
extern LPDIRECT3DDEVICE9 g_pd3dDevice;
extern LPD3DXFONT g_pFonts[];
extern LPD3DXSPRITE g_pSprite;
extern float g_aspect;
extern Model g_models[];

//Function prototypes;
HRESULT InitD3DWindow(HINSTANCE, LPCTSTR, int, int);
int CreateGameFont(LPCTSTR, int, UINT);
int LoadModel(LPCTSTR);
void RenderModel(int);
const char *GetKeyState();
void SetTimer(int, DWORD);
BOOL isTimerOver(int);
DWORD getPassedTime(int);