#include "gamelib.h"

//structs
struct ObstacleData {
	float x, z;		//Obstacle's coordinates
	bool used;		//Being used or not
	int type;		//Type of obstacle
};

//Global variables
const int OBSTACLEMAX = 50;

int gameFont;

int playerModel, backgroundModel[3], obstacleModel, explosionModel, wormHoleModel;		//Store models
float playerX = -4.0f, playerZ = 0.0f;													//Model's initial position
float prevplayerX = -8.0f, startingX = 0;												//Record player position
float wormHoleX = 0.0f, wormHoleZ = 0.0f;												//Worm hole position
int typesOfObstacle = 5;																//Types of obstacle
float lastTime;																			//Record time
float loopTime = 0;																		//Time spent for running a game loop
float speed = 5.0f;																		//Player's speed
float angle = 90;																		//Player's angle
float turingSpeed = 90;																	//Turning 90 degrees per second
float obstacleSpeed = 2.5f;																//Obstacles' speed
float leftDeadEnd = -7.0f;																//Prevent player from going backward
float background1PosX = 0.0f, background2PosX = 18.0f;									//Backgrounds' positions
float backgroundDisplacement = 36.0f;													//Backgrounds' displacements

DWORD passedTime;
TCHAR timeString[30];
TCHAR distanceString[50];

BOOL demoOver = FALSE;
BOOL enablePressEnter = FALSE;
BOOL hit = FALSE;

ObstacleData obstacles[OBSTACLEMAX];

enum { GAMETITLE, GAMEMAIN, GAMEEND, GAMEOVER };
int gameMode = GAMETITLE;

//Load models
HRESULT LoadResources() {
	//Create fonts
	gameFont = CreateGameFont(_T("AR DARLING"), 65, FW_BOLD);
	if (gameFont == -1) {
		return E_FAIL;
	}

	gameFont = CreateGameFont(_T("AR DARLING"), 45, FW_BOLD);
	if (gameFont == -1) {
		return E_FAIL;
	}

	gameFont = CreateGameFont(_T("AR DARLING"), 60, FW_BOLD);
	if (gameFont == -1) {
		return E_FAIL;
	}

	gameFont = CreateGameFont(_T("AR DARLING"), 170, FW_BOLD);
	if (gameFont == -1) {
		return E_FAIL;
	}

	//Load models
	//Load player model
	playerModel = LoadModel(_T("res/spaceShip.x"));
	if (playerModel == -1) {
		return E_FAIL;
	}

	//Load background models
	backgroundModel[0] = LoadModel(_T("res/background.x"));
	if (backgroundModel[0] == -1) {
		return E_FAIL;
	}

	backgroundModel[1] = LoadModel(_T("res/background.x"));
	if (backgroundModel[1] == -1) {
		return E_FAIL;
	}

	backgroundModel[2] = LoadModel(_T("res/timeTravelBackground.x"));
	if (backgroundModel[2] == -1) {
		return E_FAIL;
	}

	//Load obstacle model
	obstacleModel = LoadModel(_T("res/obstacle.x"));
	if (obstacleModel == -1) {
		return E_FAIL;
	}

	//Load explosion model
	explosionModel = LoadModel(_T("res/explosion.x"));
	if (explosionModel == -1) {
		return E_FAIL;
	}

	//Load worm hole model
	wormHoleModel = LoadModel(_T("res/wormHole.x"));
	if (wormHoleModel == -1) {
		return E_FAIL;
	}

	return S_OK;
}

//set view perspective
void SetViews() {
	//Set ambient light
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0xffffffff);

	//Set camera
	D3DXVECTOR3 vEye(0.0f, 15.0f, 0.0f);
	D3DXVECTOR3 vLookAt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUp(0.0f, 0.0f, 1.0f);
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH(&matView, &vEye, &vLookAt, &vUp);
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, g_aspect, 1.0f, 100.0f);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

//Create obstacle
void CreateObstacle() {
	for (int i = 0; i < OBSTACLEMAX; ++i)
	{
		if (!obstacles[i].used) {
			obstacles[i].x = (float)(rand() % 29) / 2 - 3 + playerX + (rand() % 15);
			switch (rand() % typesOfObstacle)
			{
			case 0:
				obstacles[i].z = 7;
				obstacles[i].type = 1;
				break;
			case 1:
				obstacles[i].z = -8;
				obstacles[i].type = 2;
				break;
			case 2:
				obstacles[i].z = 7;
				obstacles[i].type = 3;
				break;
			case 3:
				obstacles[i].z = -8;
				obstacles[i].type = 4;
				break;
			case 4:
				obstacles[i].x = playerX + 15;
				obstacles[i].z = (float)(rand() % 21) / 2 - 5;
				obstacles[i].type = 5;
				break;
			}
			obstacles[i].used = TRUE;
			break;
		}
	}
}

//Game content
void GameMain() {
	float velocity = 0;
	const char *keys = GetKeyState();

	if (keys != NULL) {
		if (gameMode == GAMEMAIN) {
			if (keys[DIK_UP] & 0x80) {
				velocity = speed * loopTime;
			}
			if (keys[DIK_DOWN] & 0x80) {
				velocity = -speed * loopTime;
			}
			if (keys[DIK_LEFT] & 0x80) {
				angle = angle - turingSpeed * loopTime;
			}
			if (keys[DIK_RIGHT] & 0x80) {
				angle = angle + turingSpeed * loopTime;
			}
			if (keys[DIK_SPACE] & 0x80) {
				//Fire missiles
			}
		}
	}

	//Set range of angle to 0~360
	if (angle < 0) {
		angle = angle + 360;
	}
	if (angle > 360) {
		angle = angle - 360;
	}

	float radRotationAngle = D3DXToRadian(angle);
	if (prevplayerX < playerX) {
		prevplayerX = playerX;
	}

	playerX = playerX + velocity * sinf(radRotationAngle);
	playerZ = playerZ + velocity * cosf(radRotationAngle);

	if (playerX >= 0) {
		if (prevplayerX < playerX) {
			leftDeadEnd += (playerX - prevplayerX);
		}
	}

	if (playerX < leftDeadEnd) {
		playerX = leftDeadEnd;
	}

	if (playerX >= 0) {
		if (!(prevplayerX > playerX)) {
			//Set camera
			D3DXVECTOR3 vEye(playerX, 15.0f, 0.0f);
			D3DXVECTOR3 vLookAt(playerX, 0.0f, 0.0f);
			D3DXVECTOR3 vUp(0.0f, 0.0f, 1.0f);
			D3DXMATRIXA16 matView;
			D3DXMatrixLookAtLH(&matView, &vEye, &vLookAt, &vUp);
			g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);
		}
	}

	if (playerX > 150 && gameMode == GAMEMAIN) {
		static float a = 1.0f;

		if (getPassedTime(2) > 1000) {
			//Create blinking effect
			if (a == 1.0f) {
				a = 0.0f;
			}
			else {
				a = 1.0f;
				enablePressEnter = TRUE;
			}

			SetTimer(2, 0);
		}

		RECT rt = { 2, 52, 642, 182 };
		g_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);
		g_pFonts[0]->DrawText(g_pSprite, _T("WARNING"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, a));
		SetRect(&rt, 0, 50, 640, 180);
		g_pFonts[0]->DrawText(g_pSprite, _T("WARNING"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, a));
		g_pSprite->End();

		if (playerX > 159.3) {
			gameMode = GAMEOVER;
			SetTimer(1, 0);		//Timer for game over scene
		}
	}

	if (playerZ < -6) {
		playerZ = -6;
	}
	if (playerZ > 6) {
		playerZ = 6;
	}

	D3DXMATRIXA16 matWorld1, matWorld2;

	if (gameMode == GAMEMAIN || (gameMode == GAMEOVER && !hit)) {
		//Display player model
		D3DXMatrixTranslation(&matWorld1, playerX, 0.0f, playerZ);
		D3DXMatrixRotationY(&matWorld2, radRotationAngle);
		matWorld2 *= matWorld1;
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld2);
		RenderModel(playerModel);
		
		//Remove obstacles and reset worm hole position when game starts
		if (!demoOver) {
			for (int i = 0; i < OBSTACLEMAX; ++i)
			{
				if (obstacles[i].used) {
					obstacles[i].used = FALSE;
				}
			}

			wormHoleX = 150.0f;
			wormHoleZ = (float)(rand() % 25) / 2 - 6;

			demoOver = TRUE;
		}
	}

	//Display background
	if (gameMode == GAMEMAIN) {
		//Move the position of background models
		if (playerX >= background1PosX + 18.0f) {
			background1PosX += backgroundDisplacement;
		}
		if (playerX >= background2PosX + 18.0f) {
			background2PosX += backgroundDisplacement;
		}
	}

	D3DXMatrixTranslation(&matWorld1, background1PosX, 0.0f, 0.0f);
	D3DXMatrixScaling(&matWorld2, 1.3f, 1.0f, 1.2f);
	matWorld2 *= matWorld1;
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld2);
	RenderModel(backgroundModel[0]);

	D3DXMatrixTranslation(&matWorld1, background2PosX, 0.0f, 0.0f);
	D3DXMatrixScaling(&matWorld2, 1.3f, 1.0f, 1.2f);
	matWorld2 *= matWorld1;
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld2);
	RenderModel(backgroundModel[1]);

	//Display worm hole
	D3DXMatrixTranslation(&matWorld1, wormHoleX, 0.0f, wormHoleZ);
	D3DXMatrixScaling(&matWorld2, 4.0f, 4.0f, 4.0f);
	matWorld2 *= matWorld1;
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld2);
	RenderModel(wormHoleModel);

	//Hit check
	if (sqrt(pow(wormHoleX - playerX, 2) + pow(wormHoleZ - playerZ, 2)) < 2 && gameMode == GAMEMAIN) {
		gameMode = GAMEEND;
		passedTime = getPassedTime(3);
	}

	//Create obstacle
	if (isTimerOver(0)) {
		CreateObstacle();
		if (playerX >= 30) {
			SetTimer(0, 1500);		//Reset timer for creating obstacle
			obstacleSpeed = 3.5f;	//Change the speed of obstacle
		}
		else if (playerX >= 50) {
			SetTimer(0, 1000);		//Reset timer for creating obstacle
			obstacleSpeed = 4.5f;	//Change the speed of obstacle
		}
		else {
			SetTimer(0, 2000);		//Reset timer for creating obstacle
		}
	}

	//Display and move obstacle
	for (int i = 0; i < OBSTACLEMAX; ++i) {
		if (obstacles[i].used) {
			switch (obstacles[i].type)
			{
				case 1:
					obstacles[i].z = obstacles[i].z - obstacleSpeed * loopTime;		//Move obstacle down vertically
					break;
				case 2:
					obstacles[i].z = obstacles[i].z + obstacleSpeed * loopTime;		//Move obstacle up vertically
					break;
				case 3:
					obstacles[i].x = obstacles[i].x - obstacleSpeed * loopTime;		//Move obstacle diagonally from top right to bottom left;
					obstacles[i].z = obstacles[i].z - obstacleSpeed * loopTime;
					break;
				case 4:
					obstacles[i].x = obstacles[i].x - obstacleSpeed * loopTime;		//Move obstacle diagonally from bottom right to top left;
					obstacles[i].z = obstacles[i].z + obstacleSpeed * loopTime;
					break;
				case 5:
					obstacles[i].x = obstacles[i].x - obstacleSpeed * loopTime;		//Move obstacle horizontally from right to left
					break;
			}

			if ((obstacles[i].z < -7.5 && (obstacles[i].type == 1 || obstacles[i].type == 3)) ||
				(obstacles[i].z > 8 && (obstacles[i].type == 2 || obstacles[i].type == 4)) ||
				(obstacles[i].x < leftDeadEnd && obstacles[i].type == 5)) {
				obstacles[i].used = FALSE;							//Remove obstacle once it leaves screen
			}

			D3DXMatrixTranslation(&matWorld1, obstacles[i].x, 0.0f, obstacles[i].z);
			D3DXMatrixRotationY(&matWorld2, (float)timeGetTime() / 1000);
			matWorld2 *= matWorld1;
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld2);
			RenderModel(obstacleModel);

			//Hit check
			if (sqrt(pow(obstacles[i].x - playerX, 2) + pow(obstacles[i].z - playerZ, 2)) < 2 && gameMode == GAMEMAIN) {
				gameMode = GAMEOVER;
				SetTimer(1, 0);		//Timer for game over scene
				hit = TRUE;
			}
		}
	}

	//Display explosion
	D3DXMatrixTranslation(&matWorld1, 0.0f, 0.0f, 0.0f);
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld1);
	//RenderModel(explosionModel);
}

//Display game title scene
void GameTitle() {
	GameMain();

	const char *keys = GetKeyState();

	//Initialize variables
	playerX = -4.0f;
	playerZ = 0.0f;
	angle = 90.0f;
	background1PosX = 0.0f;
	background2PosX = 18.0f;
	leftDeadEnd = -7.0f;
	wormHoleX = 0.0f;
	wormHoleZ = 0.0f;
	obstacleSpeed = 2.5f;
	prevplayerX = -8.0f;
	demoOver = FALSE;
	hit = FALSE;

	SetViews();

	static float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;

	if (getPassedTime(2) > 1000) {
		//Change game title font color
		switch (rand() % 3)
		{
		case 0:
			r += 0.01f;
			break;
		case 1:
			g += 0.01f;
			break;
		case 2:
			b += 0.01f;
			break;
		}

		if (r == 1.0f || g == 1.0f || b == 1.0f) {
			r = g = b = 0.0f;
		}
	}

	if (getPassedTime(2) > 1500) {
		//Create blinking effect
		if (a == 1.0f) {
			a = 0.0f;
		}
		else {
			a = 1.0f;
			enablePressEnter = TRUE;
		}

		SetTimer(2, 0);
	}

	if (enablePressEnter) {
		if (keys != NULL) {
			if (keys[DIK_RETURN] & 0x80) {
				gameMode = GAMEMAIN;
				SetTimer(3, 0);			//Timer for recording gameplay time
				SetTimer(2, 0);
			}
		}
	}

	//Display game title
	RECT rt = { 2, 32, 642, 132 };
	g_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);
	g_pFonts[0]->DrawText(g_pSprite, _T("SPACE SHOOTER"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 1.0f));
	SetRect(&rt, 0, 30, 640, 130);
	g_pFonts[0]->DrawText(g_pSprite, _T("SPACE SHOOTER"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(r, g, b, 1.0f));
	SetRect(&rt, 2, 112, 642, 192);
	g_pFonts[1]->DrawText(g_pSprite, _T("~ESCAPE~"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 1.0f));
	SetRect(&rt, 0, 110, 640, 190);
	g_pFonts[1]->DrawText(g_pSprite, _T("~ESCAPE~"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(r, g, b, 1.0f));


	SetRect(&rt, 0, 300, 640, 390);
	g_pFonts[1]->DrawText(g_pSprite, _T("PRESS ENTER TO START"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, a));
	g_pSprite->End();
}

//Display game result
void GameEnd() {
	const char *keys = GetKeyState();
	
	if (keys != NULL) {
		if (keys[DIK_RETURN] & 0x80) {
			gameMode = GAMETITLE;
			enablePressEnter = FALSE;

			//Initialize obstacle's array
			ZeroMemory(obstacles, sizeof(ObstacleData) * OBSTACLEMAX);
		}
	}
	
	static float a;
	D3DXMATRIXA16 matWorld1, matWorld2;
	angle = 80;
	float radRotationAngle = D3DXToRadian(angle);
	playerX = -3;
	playerZ = -2;
	SetViews();

	//Display player model
	D3DXMatrixTranslation(&matWorld1, playerX, 0.0f, playerZ);
	D3DXMatrixRotationX(&matWorld2, radRotationAngle);
	matWorld2 *= matWorld1;
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld2);
	RenderModel(playerModel);

	//Display game end background
	D3DXMatrixTranslation(&matWorld1, 0.0f, 0.0f, 0.0f);
	D3DXMatrixScaling(&matWorld2, 3.0f, 1.0f, 3.0f);
	matWorld2 *= matWorld1;
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld2);
	RenderModel(backgroundModel[2]);

	if (getPassedTime(2) > 1500) {
		//Create blinking effect
		if (a == 1.0f) {
			a = 0.0f;
		}
		else {
			a = 1.0f;
		}

		SetTimer(2, 0);
	}

	g_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);
	RECT rt = { 3, 63, 643, 193 };
	_stprintf_s(timeString, _T("%.2f SECONDS\nTO CLEAR"), (float)(passedTime / 1000.0f));
	g_pFonts[2]->DrawText(g_pSprite, timeString, -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 1.0f));
	SetRect(&rt, 0, 60, 640, 190);
	g_pFonts[2]->DrawText(g_pSprite, timeString, -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f));
	if (passedTime <= 20000) {
		SetRect(&rt, 3, 193, 643, 383);
		g_pFonts[3]->DrawText(g_pSprite, _T("S"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 1.0f));
		SetRect(&rt, 0, 190, 640, 380);
		g_pFonts[3]->DrawText(g_pSprite, _T("S"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f));
	}
	else if (passedTime <= 30000) {
		SetRect(&rt, 3, 193, 643, 383);
		g_pFonts[3]->DrawText(g_pSprite, _T("A"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f));
		SetRect(&rt, 0, 190, 640, 380);
		g_pFonts[3]->DrawText(g_pSprite, _T("A"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f));
	}
	else if (passedTime <= 40000) {
		SetRect(&rt, 3, 193, 643, 383);
		g_pFonts[3]->DrawText(g_pSprite, _T("B"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f));
		SetRect(&rt, 0, 190, 640, 380);
		g_pFonts[3]->DrawText(g_pSprite, _T("B"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f));
	}
	else if (passedTime <= 50000) {
		SetRect(&rt, 3, 193, 643, 383);
		g_pFonts[3]->DrawText(g_pSprite, _T("C"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f));
		SetRect(&rt, 0, 190, 640, 380);
		g_pFonts[3]->DrawText(g_pSprite, _T("C"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f));
	}
	else {
		SetRect(&rt, 3, 193, 643, 383);
		g_pFonts[3]->DrawText(g_pSprite, _T("F"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f));
		SetRect(&rt, 0, 190, 640, 380);
		g_pFonts[3]->DrawText(g_pSprite, _T("F"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f));
	}

	SetRect(&rt, 0, 340, 640, 460);
	g_pFonts[1]->DrawText(g_pSprite, _T("PRESS ENTER TO RETURN"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, a));
	g_pSprite->End();
}

//Display gameover scene
void GameOver() {
	GameMain();
	if (getPassedTime(1) < 1500 && hit == TRUE) {
		float passedTime = (float)getPassedTime(1);

		//Alpha Blending
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BLENDFACTOR);
		g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		float colorValue = 1.0f - passedTime / 1500;
		g_pd3dDevice->SetRenderState(D3DRS_BLENDFACTOR, D3DCOLOR_COLORVALUE(colorValue, colorValue, colorValue, colorValue));

		D3DXMATRIXA16 matWorld1, matWorld2, matWorld3;
		//Display player model
		D3DXMatrixTranslation(&matWorld1, playerX, 0.0f, playerZ);
		D3DXMatrixRotationY(&matWorld2, D3DXToRadian(angle));
		D3DXMatrixScaling(&matWorld3, 1.0f + passedTime / 5000.0f, 1.0f + passedTime / 5000.0f, 1.0f + passedTime / 5000.0f);
		matWorld3 = matWorld3 * matWorld2 * matWorld1;
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld3);
		RenderModel(playerModel);

		//Display explosion model
		D3DXMatrixScaling(&matWorld3, 1.0f + passedTime / 100.0f, 1.0f + passedTime / 100.0f, 1.0f + passedTime / 100.0f);
		D3DXMatrixRotationY(&matWorld2, (float)timeGetTime() / 1000);
		matWorld3 = matWorld3 * matWorld2 * matWorld1;
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld3);
		RenderModel(explosionModel);

		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}

	if (getPassedTime(1) > 2500) {
		float distance = (float)(wormHoleX - playerX);
		RECT rt = { 2, 52, 642, 182 };
		if (distance > 0) {
			_stprintf_s(distanceString, _T("TOO BAD...\n%.2f METERS TO GO"), distance);
		}
		else {
			_stprintf_s(distanceString, _T("TOO BAD...\nWENT OVER %.1f METERS\nOUT OF FUEL"), abs(distance));
		}
		g_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);
		g_pFonts[0]->DrawText(g_pSprite, _T("GAME OVER"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 1.0f));
		SetRect(&rt, 0, 50, 640, 180);
		g_pFonts[0]->DrawText(g_pSprite, _T("GAME OVER"), -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f));
		SetRect(&rt, 2, 222, 642, 402);
		g_pFonts[1]->DrawText(g_pSprite, distanceString, -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 1.0f));
		SetRect(&rt, 0, 220, 640, 400);
		g_pFonts[1]->DrawText(g_pSprite, distanceString, -1, &rt, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f));
		g_pSprite->End();
	}

	if (getPassedTime(1) > 10000) {
		gameMode = GAMETITLE;

		//Initialize obstacle's array
		ZeroMemory(obstacles, sizeof(ObstacleData) * OBSTACLEMAX);

		SetViews();
	}
}

//Render model to screen
void Render() {
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);
	
	if (SUCCEEDED(g_pd3dDevice->BeginScene())) {
		switch (gameMode) {
			case GAMETITLE:
				GameTitle();
				break;
			case GAMEMAIN:
				GameMain();
				break;
			case GAMEEND:
				GameEnd();
				break;
			case GAMEOVER:
				GameOver();
				break;
		}
		g_pd3dDevice->EndScene();
	}

	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

//WinMain function (Program Entry Point)
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT) {
	srand((unsigned int)timeGetTime());

	if (SUCCEEDED(InitD3DWindow(hInst, _T("SPACE SHOOTER ~ESCAPE~"), 640, 480))) {

		//Load model
		if (FAILED(LoadResources())) {
			return 0;
		}

		SetViews();

		//Set a 3 seconds timer (Timer for creating obstacle)
		SetTimer(0, 3000);

		//Set a timer (Timer for creating blinking text effect)
		SetTimer(2, 0);

		//Initialize obstacle's array
		ZeroMemory(obstacles, sizeof(ObstacleData) * OBSTACLEMAX);

		//Message Loop
		lastTime = (float)timeGetTime();
		MSG msg = { 0 };
		while (msg.message != WM_QUIT) {
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				Render();

				float curTime = (float)timeGetTime();
				loopTime = (float)(curTime - lastTime) / 1000.0f;
				lastTime = curTime;
			}
		}
	}

	//Unregister class
	UnregisterClass(_T("D3D Window Class"), GetModuleHandle(NULL));
	
	return 0;
}