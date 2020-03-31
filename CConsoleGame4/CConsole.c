#include<stdio.h>
#include<Windows.h>
#include<time.h>
#include<stdlib.h>
#include<process.h>
#include"resource.h"

static HWND hWnd;
static HINSTANCE hInst;
clock_t Start, End ,Start2, End2, GaugeStart, GaugeEnd;

struct Stamp //시작부분의 스탬프 정보
{
	int StampX, StampY;
	int Stamping; //찍었을때 1
	int FStampX, FStampY;
};

struct StickMan //플레이어 애니메이션을 위한 비트맵 저장
{
	HBITMAP Bit[3]; 
	HBITMAP FBit[3];
};

struct Box //박스의 비트맵, 각각의 좌표 등
{
	HBITMAP BoxBit[24];
	int Boxx[24], Boxy[24];
	int PosOverLap[16][9]; //Collider나 중복처리를 위함
	int MiniBox[6];
	int Boxst;
	int Fault;
	POINT BoxPos[24];

	int GetBox;
	int GetBigBox;
	int GetFBox;

	int FirstScore;
};

struct StickManValue //플레이어의 좌표, 애니메이션 처리
{
	int City;
	int StickX, StickY;
	int StickAni, StickAni_sub;
	int GetBox;
	int GetBoxNum;
	int StickManSpeed;
};

struct Collider //Collider 처리를 위한 박스들의 최대 최소 좌표
{
	int MaxX[25], MaxY[25];
	int MinX[25], MinY[25];
};

//Phase2 Struct////////////////////////////////////////////////////////////////////////////////////////
struct Ph2Map //맵이 움직일때 필요한것들, 마지막부분 애니메이션 처리
{
	int MapSpeed;
	int Map1X, Map2X;
	int Truck12;

	int MapSlowDelay;
	int Finish;
	int MapFinishDraw;
};

struct Truck //트럭의 속도, 좌표, 게이지 칸수
{
	int TruckX;
	int TruckSpeed;
	int GaugeNum;
};

struct House //집의 좌표, 비트맵(랜덤이라서), 받았을때 애니메이션 처리
{
	HBITMAP House[60];
	int Ph2HouseX[60];
	int Ph2HouseY[60];
	int XR[60];
};

struct Ph2Box //던지는 박스
{
	HBITMAP BoxBit[3];
	int BoxX[50];
	int BoxY[50];
	int BoxSpeed[50];
	int BoxNum;
	int PushWS[50];

	int BoxKind[50];

	int LastScore;
};

struct Phone //좌측 아래 폰의 채팅 관리
{
	int Chat[50];
	int ChatOK[50];
	int ChatY[50];
};
///////////////////////////////////////////////////////////////////////////////////////////////////////
struct Save //이름 저장
{
	char Name[50][100];
};

void CursorView(char show)//커서숨기기
{
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);

	HANDLE hOutHan = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(hOutHan, &info);
	COORD new_size =
	{
		info.srWindow.Right - info.srWindow.Left + 1,
		info.srWindow.Bottom - info.srWindow.Top + 1
	};
	SetConsoleScreenBufferSize(hOutHan, new_size);
	HANDLE hConsole;
	CONSOLE_CURSOR_INFO ConsoleCursor;

	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	ConsoleCursor.bVisible = show;
	ConsoleCursor.dwSize = 1;

	SetConsoleCursorInfo(hConsole, &ConsoleCursor);
}

//Start/////////////////////////////////////////////////////////////////////////////////////////////////
void StampPos(struct Stamp *Stamp) //스탬프의 위치
{
	POINT Cur;
	GetCursorPos(&Cur);
	ScreenToClient(hWnd, &Cur);
	Stamp->StampX = Cur.x / 1535.0 * 1920; //해상도 문제인지 모르겠지만 정확한 위치를 위한 연산
	Stamp->StampY = Cur.y / 863.0 * 1080;
}

void StartRender(char *CityName, struct Stamp Stamp) //홈화면 이미지 출력과 배달할 지역 랜덤 출력
{
	HDC hDC, hMemDC;
	static HDC hBackDC;
	HBITMAP hBackBitmap, hOldBitmap, hNewBitmap;
	BITMAP Bitmap;
	RECT rt = { 0,470,1920,1080 };

	hDC = GetDC(hWnd);

	hMemDC = CreateCompatibleDC(hDC);
	hBackDC = CreateCompatibleDC(hDC);

	hBackBitmap = CreateCompatibleBitmap(hDC, 1920, 1080);
	hOldBitmap = (HBITMAP)SelectObject(hBackDC, hBackBitmap);

	hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_StartBack));
	GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
	SelectObject(hMemDC, hNewBitmap);
	GdiTransparentBlt(hBackDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
	DeleteObject(hNewBitmap);

	HFONT font = CreateFont(140, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"제주한라산");

	(HFONT)SelectObject(hBackDC, font);

	SetTextColor(hBackDC, RGB(0,0,0));
	SetBkMode(hBackDC, TRANSPARENT);

	DrawTextA(hBackDC,CityName, -1, &rt, DT_CENTER);

	if (Stamp.Stamping)
	{
		hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_Dojang));
		GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
		SelectObject(hMemDC, hNewBitmap);
		GdiTransparentBlt(hBackDC, Stamp.FStampX - 50, Stamp.FStampY - 10, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
		DeleteObject(hNewBitmap);
	}

	hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_StampCursor));
	GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
	SelectObject(hMemDC, hNewBitmap);
	GdiTransparentBlt(hBackDC, Stamp.StampX - 50, Stamp.StampY - 50, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
	DeleteObject(hNewBitmap);

	BitBlt(hDC, 0, 0, 1920, 1080, hBackDC, 0, 0, SRCCOPY);

	DeleteObject(SelectObject(hBackDC, hBackBitmap));
	DeleteObject(hNewBitmap);
	DeleteDC(hBackDC);
	DeleteDC(hMemDC);

	ReleaseDC(hWnd, hDC);
}

void StampAni(char* CityName, struct Stamp *Stamp) //스탬프를 찍었을때 애니메이션
{
	int StX = Stamp->StampX, StY = Stamp->StampY;

	while (Stamp->StampY < StY + 50)
	{
		Stamp->StampY += 8;
		StartRender(CityName, *Stamp);
	}
	Stamp->Stamping = 1;
	while (Stamp->StampY > StY - 50)
	{
		Stamp->StampY -= 15;
		StartRender(CityName, *Stamp);
	}
}
//phase 1///////////////////////////////////////////////////////////////////////////////////////////////
void Render(HBITMAP* Stick, struct StickManValue* StickVal, struct Box* box, int Dir) //1페이즈 박스, 맵, 플레이어 등 출력
{

	HDC hDC, hMemDC;
	static HDC hBackDC;
	HBITMAP hBackBitmap, hOldBitmap, hNewBitmap;
	BITMAP Bitmap;

	RECT rt = { 1400,30,1920,180 };

	char string[10];
	hDC = GetDC(hWnd);

	hMemDC = CreateCompatibleDC(hDC);
	hBackDC = CreateCompatibleDC(hDC);

	hBackBitmap = CreateCompatibleBitmap(hDC, 1920, 1080);
	hOldBitmap = (HBITMAP)SelectObject(hBackDC, hBackBitmap);

	hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_Map));
	GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
	SelectObject(hMemDC, hNewBitmap);
	GdiTransparentBlt(hBackDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
	DeleteObject(hNewBitmap);

	if (box->Fault)
	{
		hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_ClassStick));
		GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
		SelectObject(hMemDC, hNewBitmap);
		GdiTransparentBlt(hBackDC, 50, 30, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
		DeleteObject(hNewBitmap);
	}

	for (int k = 0; k < 6; k++)
	{
		if (box->MiniBox[k] == 1)
		{
			hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_MiniBox));
			GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
			SelectObject(hMemDC, hNewBitmap);
			GdiTransparentBlt(hBackDC, k % 2 == 0 ? 869 : 963, k < 2 ? 180 : (k < 4 ? 118 : 56), Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
			DeleteObject(hNewBitmap);
		}
	}

	for (int i = 0; i < 24; i++)
	{
		hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(box->BoxBit[i]));
		GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
		SelectObject(hMemDC, hNewBitmap);
		GdiTransparentBlt(hBackDC, box->Boxx[i], box->Boxy[i], Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
		DeleteObject(hNewBitmap);
	}
	if (Dir == 4)
	{
		hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(Stick[1]));
		GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
		SelectObject(hMemDC, hNewBitmap);
		GdiTransparentBlt(hBackDC, StickVal->StickX, StickVal->StickY, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
		DeleteObject(hNewBitmap);
	}
	else
	{
		hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(Stick[StickVal->StickAni % 2 + 1]));
		GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
		SelectObject(hMemDC, hNewBitmap);
		GdiTransparentBlt(hBackDC, StickVal->StickX, StickVal->StickY, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
		DeleteObject(hNewBitmap);
		StickVal->StickAni_sub++;
	}

	if (StickVal->StickAni_sub == 15)
	{
		StickVal->StickAni_sub = 0;
		StickVal->StickAni++;
	}

	HFONT font = CreateFont(140, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Digital-7 Mono");

	(HFONT)SelectObject(hBackDC, font);

	SetTextColor(hBackDC, RGB(255, 0, 0));
	SetBkMode(hBackDC, TRANSPARENT);
	
	sprintf(string, "%d",30 - (int)(((End2 - Start2) / CLOCKS_PER_SEC)));
	DrawTextA(hBackDC, string, -1, &rt, DT_CENTER);

	BitBlt(hDC, 0, 0, 1920, 1080, hBackDC, 0, 0, SRCCOPY);

	DeleteObject(SelectObject(hBackDC, hBackBitmap));
	DeleteObject(hNewBitmap);
	DeleteDC(hBackDC);
	DeleteDC(hMemDC);

	ReleaseDC(hWnd, hDC);
}

void RandomBoxPos(struct Box* box, struct Collider* Coll) //박스의 랜덤한 위치 지정(중복 X)
{
	srand(GetTickCount64());
	int Ver;
	int Hor;
	for (int j = 20; j < 24; j++)
	{
		Hor = rand() % 15;
		Ver = rand() % 2 + 6;

		if (box->PosOverLap[Hor][Ver] != 1 && box->PosOverLap[Hor + 1][Ver] != 1 && box->PosOverLap[Hor][Ver + 1] != 1 && box->PosOverLap[Hor + 1][Ver + 1] != 1)
		{
			box->PosOverLap[Hor][Ver] = 1;
			box->PosOverLap[Hor + 1][Ver] = 1;
			box->PosOverLap[Hor][Ver + 1] = 1;
			box->PosOverLap[Hor + 1][Ver + 1] = 1;

			box->BoxPos[j].x = Hor;
			box->BoxPos[j].y = Ver;

			box->Boxx[j] = 15 + Hor * 120;
			box->Boxy[j] = Ver * 120;

			Coll->MaxX[j] = box->Boxx[j] + 240;
			Coll->MinX[j] = box->Boxx[j];
			Coll->MaxY[j] = box->Boxy[j] + 240;
			Coll->MinY[j] = box->Boxy[j];
		}
		else
			j--;
	}

	for (int i = 0; i < 16; i++)
	{
		Ver = rand() % 3 + 6;
		Hor = rand() % 16;

		if (box->PosOverLap[Hor][Ver] != 1)
		{
			box->PosOverLap[Hor][Ver] = 1;
			box->Boxx[i] = 15 + Hor * 120;
			box->Boxy[i] = Ver * 120;

			box->BoxPos[i].x = Hor;
			box->BoxPos[i].y = Ver;

			Coll->MaxX[i] = box->Boxx[i] + 120;
			Coll->MinX[i] = box->Boxx[i];
			Coll->MaxY[i] = box->Boxy[i] + 120;
			Coll->MinY[i] = box->Boxy[i];
		}

		else
		{
			i--;
		}
	}

	for (int k = 16; k < 20; k++)
	{
		Ver = rand() % 3 + 6;
		Hor = rand() % 16;

		if (box->PosOverLap[Hor][Ver] != 1)
		{
			box->PosOverLap[Hor][Ver] = 1;

			box->Boxx[k] = 15 + Hor * 120;
			box->Boxy[k] = Ver * 120;

			box->BoxPos[k].x = Hor;
			box->BoxPos[k].y = Ver;

			Coll->MaxX[k] = box->Boxx[k] + 120;
			Coll->MinX[k] = box->Boxx[k];
			Coll->MaxY[k] = box->Boxy[k] + 120;
			Coll->MinY[k] = box->Boxy[k];
		}
		else
			k--;
	}
}

int ColliderCheck(struct StickManValue StickVal, struct Collider Coll, int Dir) //충돌 처리를 위한 좌표 모음
{
	switch (Dir)
	{
	case 1:
		for (int i = 0; i < 24; i++)
		{
			if(StickVal.StickX + 100 > 820 && StickVal.StickX + 100 < 1100 && StickVal.StickY + 150 - 5 < 246)
			{
				return 30;
			}

			if (Coll.MaxY[i] >= StickVal.StickY - 5 + 200 && Coll.MinY[i] <= StickVal.StickY - 5 + 200 && Coll.MaxX[i] >= StickVal.StickX + 100 && Coll.MinX[i] <= StickVal.StickX + 100)
			{
				return i;
			}
		}
		break;
	case 2:
		for (int i = 0; i < 24; i++)
		{
			if (Coll.MaxY[i] >= StickVal.StickY + 5 + 200 && Coll.MinY[i] <= StickVal.StickY + 5 + 200 && Coll.MaxX[i] >= StickVal.StickX + 100 && Coll.MinX[i] <= StickVal.StickX + 100)
			{
				return i;
			}
		}
		break;
	case 3:
		for (int i = 0; i < 24; i++)
		{
			if (StickVal.StickX + 100 - 5 > 820 && StickVal.StickX + 100 - 5 < 1100 && StickVal.StickY + 200 < 246)
			{
				return 30;
			}

			if (Coll.MaxY[i] >= StickVal.StickY + 200 && Coll.MinY[i] <= StickVal.StickY - 5 + 200 && Coll.MaxX[i] >= StickVal.StickX - 5 + 50 && Coll.MinX[i] <= StickVal.StickX - 5 + 50)
			{
				return i;
			}
		}
		break;
	case 4:
		for (int i = 0; i < 24; i++)
		{
			if (StickVal.StickX + 100 + 5 > 820 && StickVal.StickX + 100 + 5 < 1100 && StickVal.StickY + 200 < 246)
			{
				return 30;
			}

			if (Coll.MaxY[i] >= StickVal.StickY + 200 && Coll.MinY[i] <= StickVal.StickY - 5 + 200 && Coll.MaxX[i] >= StickVal.StickX + 5 + 200 && Coll.MinX[i] <= StickVal.StickX + 5 + 150)
			{
				return i;
			}
		}
		break;
	}
	return 50;
}

void StickManBoxSet(struct StickMan* Stick, struct Box* Box, struct StickManValue* StickVal, struct Stamp *Stamp) //게임 시작 시 모든 값들 초기화
{
	Stick[0].Bit[1] = IDB_StickMan02;
	Stick[0].Bit[2] = IDB_StickMan03;

	Stick[1].Bit[1] = IDB_StickMan12;
	Stick[1].Bit[2] = IDB_StickMan13;

	Stick[2].Bit[1] = IDB_StickMan22;
	Stick[2].Bit[2] = IDB_StickMan23;

	Stick[3].Bit[1] = IDB_StickMan32;
	Stick[3].Bit[2] = IDB_StickMan33;

	Stick[4].Bit[1] = IDB_StickMan11;
	Stick[4].Bit[2] = IDB_StickMan11;


	Stick[0].FBit[1] = IDB_FStickMan02;
	Stick[0].FBit[2] = IDB_FStickMan03;

	Stick[1].FBit[1] = IDB_FStickMan12;
	Stick[1].FBit[2] = IDB_FStickMan13;

	Stick[2].FBit[1] = IDB_FStickMan22;
	Stick[2].FBit[2] = IDB_FStickMan23;

	Stick[3].FBit[1] = IDB_FStickMan32;
	Stick[3].FBit[2] = IDB_FStickMan33;

	Stick[4].FBit[1] = IDB_FStickMan11;
	Stick[4].FBit[2] = IDB_FStickMan11;

	for (int i = 0; i < 4; i++)
	{
		Box->BoxBit[i] = IDB_Box1;
	}
	for (int i = 4; i < 8; i++)
	{
		Box->BoxBit[i] = IDB_Box2;
	}
	for (int i = 8; i < 12; i++)
	{
		Box->BoxBit[i] = IDB_Box3;
	}
	for (int i = 12; i < 16; i++)
	{
		Box->BoxBit[i] = IDB_Box4;
	}

	Box->BoxBit[20] = IDB_BigBox1;
	Box->BoxBit[21] = IDB_BigBox2;
	Box->BoxBit[22] = IDB_BigBox3;
	Box->BoxBit[23] = IDB_BigBox4;

	Box->BoxBit[16] = IDB_FBox1;
	Box->BoxBit[17] = IDB_FBox2;
	Box->BoxBit[18] = IDB_FBox3;
	Box->BoxBit[19] = IDB_FBox4;

	Box->Fault = 0;
	Box->FirstScore = 0;

	for (int j = 0; j < 6; j++)
	{
		Box->MiniBox[j] = 10;
	}
	Box->Boxst = 0;

	for (int k = 0; k < 24; k++)
	{
		Box->BoxPos[k].x = 50;
		Box->BoxPos[k].y = 50;
	}

	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 16; j++)
			Box->PosOverLap[j][i] = 0;

	Box->GetBox = 0;
	Box->GetFBox = 0;
	Box->GetBigBox = 0;

	StickVal->StickX = 500;
	StickVal->StickY = 500;
	StickVal->StickAni = 0;
	StickVal->StickAni_sub = 0;
	StickVal->GetBox = 0;
	StickVal->GetBoxNum = 50;
	StickVal->StickManSpeed = 7;

	Stamp->StampX = 0;
	Stamp->StampY = 0;
	Stamp->FStampX = 0;
	Stamp->FStampY = 0;
	Stamp->Stamping = 0;
}

void PutBox(struct StickManValue* StickVal, struct Box* Box, struct Collider* Coll, int Dir) //박스를 놓았을 때 값 변화
{
	switch (Dir)
	{
	case 0:
		for (int i = 0; i < 16; i++)
			for (int j = 1; j < 9; j++)
			{
				if (StickVal->StickX + 100 >= i * 120 && StickVal->StickX + 100 <= i * 120 + 120 && StickVal->StickY + 60 >= j * 120 && StickVal->StickY + 60 <= j * 120 + 120 && Box->PosOverLap[i][j] == 0)
				{
					if ((i == 7 || i == 8) && (j == 0 || j == 1))
					{
						if ((StickVal->City * 4 <= StickVal->GetBoxNum && StickVal->City * 4 + 4 > StickVal->GetBoxNum))
						{
							Box->MiniBox[Box->Boxst] = 1;
							Box->Boxst++;
							Box->GetBox++;
							Box->FirstScore += 1;
						}
						else if (StickVal->GetBoxNum == 16 + StickVal->City) //취급주의
						{
							Box->MiniBox[Box->Boxst] = 1;
							Box->Boxst++;
							Box->GetFBox++;
						}

						else if (StickVal->GetBoxNum == 20 + StickVal->City) //대형
						{
							Box->MiniBox[Box->Boxst] = 1;
							Box->Boxst++;
							Box->GetBigBox++;
						}

						else
						{
							if (Box->Fault == 0)
							{
								Start = clock();
							}
							Box->Fault = 1;
						}
						StickVal->StickManSpeed = 7;
						StickVal->GetBox = 0;
						StickVal->GetBoxNum = 50;
						continue;
					}
					if (StickVal->GetBoxNum >= 20 && StickVal->GetBoxNum < 24 && Box->PosOverLap[i+1][j-1] == 0 && Box->PosOverLap[i][j] == 0 && Box->PosOverLap[i+1][j] == 0)
					{
						Box->Boxx[StickVal->GetBoxNum] = 15 + 120 * i;
						Box->Boxy[StickVal->GetBoxNum] = 120 * j - 120;
						Coll->MaxX[StickVal->GetBoxNum] = 15 + 120 * i + 240;
						Coll->MaxY[StickVal->GetBoxNum] = 120 * j + 120;
						Coll->MinX[StickVal->GetBoxNum] = 15 + 120 * i;
						Coll->MinY[StickVal->GetBoxNum] = 120 * j - 120;

						Box->BoxPos[StickVal->GetBoxNum].x = i;
						Box->BoxPos[StickVal->GetBoxNum].y = j - 1;

						Box->PosOverLap[i][j - 1] = 1;
						Box->PosOverLap[i + 1][j - 1] = 1;
						Box->PosOverLap[i][j] = 1;
						Box->PosOverLap[i + 1][j] = 1;
						StickVal->StickManSpeed = 7;
						StickVal->GetBox = 0;
						StickVal->GetBoxNum = 50;
					}
					else if (StickVal->GetBoxNum >= 0 && StickVal->GetBoxNum < 20)
					{
						Box->Boxx[StickVal->GetBoxNum] = 15 + 120 * i;
						Box->Boxy[StickVal->GetBoxNum] = 120 * j;
						Coll->MaxX[StickVal->GetBoxNum] = 15 + 120 * i + 120;
						Coll->MaxY[StickVal->GetBoxNum] = 120 * j + 120;
						Coll->MinX[StickVal->GetBoxNum] = 15 + 120 * i;
						Coll->MinY[StickVal->GetBoxNum] = 120 * j;

						Box->BoxPos[StickVal->GetBoxNum].x = i;
						Box->BoxPos[StickVal->GetBoxNum].y = j;

						Box->PosOverLap[i][j] = 1;
						StickVal->StickManSpeed = 7;
						StickVal->GetBox = 0;
						StickVal->GetBoxNum = 50;
					}
				}
			}
		break;
	case 1:
		for (int i = 0; i < 16; i++)
			for (int j = 1; j < 9; j++)
			{
				if (StickVal->StickX + 100 > i * 120 && StickVal->StickX + 100 < i * 120 + 120 && StickVal->StickY + 320 > j * 120 && StickVal->StickY + 320 < j * 120 + 120 && Box->PosOverLap[i][j] == 0)
				{
					if ((i == 7 || i == 8) && (j == 0 || j == 1))
					{
						if ((StickVal->City * 4 <= StickVal->GetBoxNum && StickVal->City * 4 + 4 > StickVal->GetBoxNum))
						{
							Box->MiniBox[Box->Boxst] = 1;
							Box->Boxst++;
							Box->GetBox++;
							Box->FirstScore += 1;
						}
						else if (StickVal->GetBoxNum == 16 + StickVal->City) //취급주의
						{
							Box->MiniBox[Box->Boxst] = 1;
							Box->Boxst++;
							Box->GetFBox++;
						}

						else if (StickVal->GetBoxNum == 20 + StickVal->City) //대형
						{
							Box->MiniBox[Box->Boxst] = 1;
							Box->Boxst++;
							Box->GetBigBox++;
						}
						else
						{
							if (Box->Fault == 0)
							{
								Start = clock();
							}
							Box->Fault = 1;
						}
						StickVal->StickManSpeed = 7;
						StickVal->GetBox = 0;
						StickVal->GetBoxNum = 50;
						continue;
					}
					if (StickVal->GetBoxNum >= 20 && StickVal->GetBoxNum < 24 && Box->PosOverLap[i][j+1] == 0 && Box->PosOverLap[i+1][j] == 0 && Box->PosOverLap[i+1][j+1] == 0)
					{
						Box->Boxx[StickVal->GetBoxNum] = 15 + 120 * i;
						Box->Boxy[StickVal->GetBoxNum] = 120 * j;
						Coll->MaxX[StickVal->GetBoxNum] = 15 + 120 * i + 240;
						Coll->MaxY[StickVal->GetBoxNum] = 120 * j + 240;
						Coll->MinX[StickVal->GetBoxNum] = 15 + 120 * i;
						Coll->MinY[StickVal->GetBoxNum] = 120 * j;

						Box->BoxPos[StickVal->GetBoxNum].x = i;
						Box->BoxPos[StickVal->GetBoxNum].y = j;

						Box->PosOverLap[i][j] = 1;
						Box->PosOverLap[i + 1][j] = 1;
						Box->PosOverLap[i][j + 1] = 1;
						Box->PosOverLap[i + 1][j + 1] = 1;
						StickVal->StickManSpeed = 7;
						StickVal->GetBox = 0;
						StickVal->GetBoxNum = 50;
					}
					else if (StickVal->GetBoxNum >= 0 && StickVal->GetBoxNum < 20)
					{
						Box->Boxx[StickVal->GetBoxNum] = 15 + 120 * i;
						Box->Boxy[StickVal->GetBoxNum] = 120 * j;
						Coll->MaxX[StickVal->GetBoxNum] = 15 + 120 * i + 120;
						Coll->MaxY[StickVal->GetBoxNum] = 120 * j + 120;
						Coll->MinX[StickVal->GetBoxNum] = 15 + 120 * i;
						Coll->MinY[StickVal->GetBoxNum] = 120 * j;

						Box->BoxPos[StickVal->GetBoxNum].x = i;
						Box->BoxPos[StickVal->GetBoxNum].y = j;

						Box->PosOverLap[i][j] = 1;
						StickVal->StickManSpeed = 7;
						StickVal->GetBox = 0;
						StickVal->GetBoxNum = 50;
					}
				}
			}
		break;
	case 2:
		for (int i = 0; i < 16; i++)
			for (int j = 1; j < 9; j++)
			{
				if (StickVal->StickX - 60 > i * 120 && StickVal->StickX - 60 < i * 120 + 120 && StickVal->StickY + 200 > j * 120 && StickVal->StickY + 200 < j * 120 + 120 && Box->PosOverLap[i][j] == 0)
				{
					if ((i == 7 || i == 8) && (j == 0 || j == 1))
					{
						if ((StickVal->City * 4 <= StickVal->GetBoxNum && StickVal->City * 4 + 4 > StickVal->GetBoxNum))
						{
							Box->MiniBox[Box->Boxst] = 1;
							Box->Boxst++;
							Box->GetBox++;
							Box->FirstScore += 1;
						}
						else if (StickVal->GetBoxNum == 16 + StickVal->City) //취급주의
						{
							Box->MiniBox[Box->Boxst] = 1;
							Box->Boxst++;
							Box->GetFBox++;
						}

						else if (StickVal->GetBoxNum == 20 + StickVal->City) //대형
						{
							Box->MiniBox[Box->Boxst] = 1;
							Box->Boxst++;
							Box->GetBigBox++;
						}
						else
						{
							if (Box->Fault == 0)
							{
								Start = clock();
							}
							Box->Fault = 1;
						}
						StickVal->GetBox = 0;
						StickVal->GetBoxNum = 50;
						StickVal->StickManSpeed = 7;
						continue;
					}
					if (StickVal->GetBoxNum >= 20 && StickVal->GetBoxNum < 24 && Box->PosOverLap[i][j-1] == 0 && Box->PosOverLap[i-1][j] == 0 && Box->PosOverLap[i][j] == 0 && Box->PosOverLap[i-1][j-1] == 0)
					{
						Box->Boxx[StickVal->GetBoxNum] = 15 + 120 * i - 120;
						Box->Boxy[StickVal->GetBoxNum] = 120 * j - 120;
						Coll->MaxX[StickVal->GetBoxNum] = 15 + 120 * i + 120;
						Coll->MaxY[StickVal->GetBoxNum] = 120 * j + 120;
						Coll->MinX[StickVal->GetBoxNum] = 15 + 120 * i - 120;
						Coll->MinY[StickVal->GetBoxNum] = 120 * j - 120;

						Box->BoxPos[StickVal->GetBoxNum].x = i - 1;
						Box->BoxPos[StickVal->GetBoxNum].y = j - 1;

						Box->PosOverLap[i - 1][j - 1] = 1;
						Box->PosOverLap[i][j - 1] = 1;
						Box->PosOverLap[i - 1][j] = 1;
						Box->PosOverLap[i][j] = 1;
						StickVal->StickManSpeed = 7;
						StickVal->GetBox = 0;
						StickVal->GetBoxNum = 50;
					}
					else if (StickVal->GetBoxNum >= 0 && StickVal->GetBoxNum < 20)
					{
						Box->Boxx[StickVal->GetBoxNum] = 15 + 120 * i;
						Box->Boxy[StickVal->GetBoxNum] = 120 * j;
						Coll->MaxX[StickVal->GetBoxNum] = 15 + 120 * i + 120;
						Coll->MaxY[StickVal->GetBoxNum] = 120 * j + 120;
						Coll->MinX[StickVal->GetBoxNum] = 15 + 120 * i;
						Coll->MinY[StickVal->GetBoxNum] = 120 * j;

						Box->BoxPos[StickVal->GetBoxNum].x = i;
						Box->BoxPos[StickVal->GetBoxNum].y = j;

						Box->PosOverLap[i][j] = 1;
						StickVal->StickManSpeed = 7;
						StickVal->GetBox = 0;
						StickVal->GetBoxNum = 50;
					}
				}
			}
		break;
	case 3:
		for (int i = 0; i < 16; i++)
			for (int j = 1; j < 9; j++)
			{
				if (StickVal->StickX + 260 > i * 120 && StickVal->StickX + 260 < i * 120 + 120 && StickVal->StickY + 200 > j * 120 && StickVal->StickY + 200 < j * 120 + 120 && Box->PosOverLap[i][j] == 0)
				{
					if ((i == 7 || i == 8) && (j == 0 || j == 1))
					{
						if ((StickVal->City * 4 <= StickVal->GetBoxNum && StickVal->City * 4 + 4 > StickVal->GetBoxNum))
						{
							Box->MiniBox[Box->Boxst] = 1;
							Box->Boxst++;
							Box->GetBox++;
							Box->FirstScore += 1;
						}
						else if (StickVal->GetBoxNum == 16 + StickVal->City) //취급주의
						{
							Box->MiniBox[Box->Boxst] = 1;
							Box->Boxst++;
							Box->GetFBox++;
						}

						else if (StickVal->GetBoxNum == 20 + StickVal->City) //대형
						{
							Box->MiniBox[Box->Boxst] = 1;
							Box->Boxst++;
							Box->GetBigBox++;
						}
						else
						{
							if (Box->Fault == 0)
							{
								Start = clock();
							}
							Box->Fault = 1;
						}
						StickVal->GetBox = 0;
						StickVal->GetBoxNum = 50;
						StickVal->StickManSpeed = 7;
						continue;
					}
					if (StickVal->GetBoxNum >= 20 && StickVal->GetBoxNum < 24 && Box->PosOverLap[i+1][j-1] == 0 && Box->PosOverLap[i][j] == 0 && Box->PosOverLap[i+1][j] == 0)
					{
						Box->Boxx[StickVal->GetBoxNum] = 15 + 120 * i;
						Box->Boxy[StickVal->GetBoxNum] = 120 * j - 120;
						Coll->MaxX[StickVal->GetBoxNum] = 15 + 120 * i + 240;
						Coll->MaxY[StickVal->GetBoxNum] = 120 * j + 120;
						Coll->MinX[StickVal->GetBoxNum] = 15 + 120 * i;
						Coll->MinY[StickVal->GetBoxNum] = 120 * j - 120;

						Box->BoxPos[StickVal->GetBoxNum].x = i;
						Box->BoxPos[StickVal->GetBoxNum].y = j - 1;

						Box->PosOverLap[i][j - 1] = 1;
						Box->PosOverLap[i + 1][j - 1] = 1;
						Box->PosOverLap[i][j] = 1;
						Box->PosOverLap[i + 1][j] = 1;
						StickVal->GetBox = 0;
						StickVal->GetBoxNum = 50;
						StickVal->StickManSpeed = 7;
					}
					else if (StickVal->GetBoxNum >= 0 && StickVal->GetBoxNum < 20)
					{
						Box->Boxx[StickVal->GetBoxNum] = 15 + 120 * i;
						Box->Boxy[StickVal->GetBoxNum] = 120 * j;
						Coll->MaxX[StickVal->GetBoxNum] = 15 + 120 * i + 120;
						Coll->MaxY[StickVal->GetBoxNum] = 120 * j + 120;
						Coll->MinX[StickVal->GetBoxNum] = 15 + 120 * i;
						Coll->MinY[StickVal->GetBoxNum] = 120 * j;

						Box->BoxPos[StickVal->GetBoxNum].x = i;
						Box->BoxPos[StickVal->GetBoxNum].y = j;

						Box->PosOverLap[i][j] = 1;
						StickVal->StickManSpeed = 7;
						StickVal->GetBox = 0;
						StickVal->GetBoxNum = 50;
					}
				}
			}
		break;
	}
}

int Moving(struct StickManValue* StickVal, struct Collider* Coll, struct StickMan* Stick, struct Box* Box, UINT VK, int MoveX, int MoveY, int MoveDir) //움직일때의 모션 모두 처리
{
	int StoreI;
	if (GetAsyncKeyState(VK))
	{
		StoreI = ColliderCheck(*StickVal, *Coll, MoveDir + 1);
		if (StoreI == 50)
		{
			if (GetAsyncKeyState(VK_SPACE) && StickVal->GetBox == 1)
			{
				while (GetAsyncKeyState(VK_SPACE))
				{

				}
				PutBox(StickVal, Box, Coll, MoveDir);
			}

			if (StickVal->GetBox == 1)
			{
				Render(Stick[MoveDir].FBit, StickVal, Box, MoveDir);
			}
			else Render(Stick[MoveDir].Bit, StickVal, Box, MoveDir);
			StickVal->StickX += MoveX;
			StickVal->StickY += MoveY;
			return 1;
		}

		else if (StoreI == 30 && GetAsyncKeyState(VK_SPACE) && StickVal->GetBox == 1)
		{
			while (GetAsyncKeyState(VK_SPACE))
			{

			}
			PutBox(StickVal, Box, Coll, MoveDir);
		}

		else if (StoreI < 30 && GetAsyncKeyState(VK) && GetAsyncKeyState(VK_SPACE) && StickVal->GetBox == 0)
		{
			StickVal->GetBox = 1;
			StickVal->GetBoxNum = StoreI;
			Box->Boxx[StoreI] = 5000;
			Box->Boxy[StoreI] = 5000;

			Coll->MaxX[StoreI] = 5000;
			Coll->MinX[StoreI] = 5000;
			Coll->MaxY[StoreI] = 5000;
			Coll->MinY[StoreI] = 5000;

			if (StickVal->GetBoxNum >= 20 && StickVal->GetBoxNum < 24)
			{
				StickVal->StickManSpeed = 5;
				Box->PosOverLap[Box->BoxPos[StoreI].x][Box->BoxPos[StoreI].y] = 0;
				Box->PosOverLap[Box->BoxPos[StoreI].x][Box->BoxPos[StoreI].y + 1] = 0;
				Box->PosOverLap[Box->BoxPos[StoreI].x + 1][Box->BoxPos[StoreI].y] = 0;
				Box->PosOverLap[Box->BoxPos[StoreI].x + 1][Box->BoxPos[StoreI].y + 1] = 0;
			}

			else
			{
				Box->PosOverLap[Box->BoxPos[StoreI].x][Box->BoxPos[StoreI].y] = 0;
			}

			Box->BoxPos[StoreI].x = 50;
			Box->BoxPos[StoreI].y = 50;

			Render(Stick[MoveDir].FBit, StickVal, Box, MoveDir);
			while (GetAsyncKeyState(VK_SPACE))
			{

			}
		}

		/*else if (StoreI == 30 && GetAsyncKeyState(VK) && GetAsyncKeyState(VK_SPACE) && StickVal->GetBox == 1)
		{
			StickVal->GetBox = 0;

			Box->Boxx[StickVal->GetBoxNum] = 5000;
			Box->Boxy[StickVal->GetBoxNum] = 5000;

			Coll->MaxX[StickVal->GetBoxNum] = 5000;
			Coll->MinX[StickVal->GetBoxNum] = 5000;
			Coll->MaxY[StickVal->GetBoxNum] = 5000;
			Coll->MinY[StickVal->GetBoxNum] = 5000;

			Box->BoxPos[StickVal->GetBoxNum].x = 50;
			Box->BoxPos[StickVal->GetBoxNum].y = 50;

			Render(Stick[MoveDir].Bit, StickVal, Box, MoveDir);

			return 1;
		}*/
	}
	return 0;
}
//Phase2/////////////////////////////////////////////////////////////////////////////////////////////////
void BoxFBigSet(struct Box Box, struct Ph2Box *Ph2Box) //박스의 종류 랜덤한 위치에 배정
{
	int Rand;
	srand(GetTickCount64());
	for (int i = 0; i < Box.GetFBox * 4; i++)
	{
		Ph2Box->BoxKind[rand() % 50] = 1;
	}

	for (int i = 0; i < Box.GetBigBox * 4; i++)
	{
		Ph2Box->BoxKind[rand() % 50] = 2;
	}
	Ph2Box->LastScore = 0;
}

void Ph2Render(struct Ph2Map *Ph2Map, struct Truck *Truck, struct House *House, struct Ph2Box *Ph2Box, struct Phone *Phone,  HBITMAP hBackGroundBitmap) //2페이즈의 모든 이미지 출력 처리
{
	HDC hDC, hMemDC, hBackGroundDC;
	static HDC hBackDC;
	HBITMAP hBackBitmap, hOldBitmap, hNewBitmap;
	BITMAP Bitmap;

	hDC = GetDC(hWnd);

	hMemDC = CreateCompatibleDC(hDC);
	hBackGroundDC = CreateCompatibleDC(hDC);
	hBackDC = CreateCompatibleDC(hDC);

	hBackBitmap = CreateCompatibleBitmap(hDC, 1920, 1080);
	hOldBitmap = (HBITMAP)SelectObject(hBackDC, hBackBitmap);

	if ((House->Ph2HouseX[59] < 0))
	{
		Ph2Map->Finish = 1;
		if(Ph2Map->MapSpeed > 2)
			Ph2Map->MapSlowDelay += 1;
		if (Ph2Map->MapSlowDelay > 100)
		{
			Ph2Map->MapSlowDelay = 0;
			Ph2Map->MapSpeed -= 2;
		}

		if (Ph2Map->Map2X > 1900)
		{
			Ph2Map->MapFinishDraw = 1;
		}
	}

	if (Ph2Map->Map1X < -1920)
		Ph2Map->Map1X = -1;
	if (Ph2Map->Map1X < 0)
		Ph2Map->Map2X = 1920 + Ph2Map->Map1X;

	SelectObject(hBackGroundDC, hBackGroundBitmap);
	BitBlt(hBackDC, Ph2Map->Map1X, 0, 1920, 1080, hBackGroundDC, 0, 0, SRCCOPY);
	Ph2Map->Map1X -= Ph2Map->MapSpeed;

	if (Ph2Map->MapFinishDraw != 1)
	{
		SelectObject(hBackGroundDC, hBackGroundBitmap);
		BitBlt(hBackDC, Ph2Map->Map2X, 0, 1920, 1080, hBackGroundDC, 0, 0, SRCCOPY);
	}
	else if (Ph2Map->MapFinishDraw == 1)
	{
		hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_MapLast));
		SelectObject(hMemDC, hNewBitmap);
		BitBlt(hBackDC, Ph2Map->Map2X, 0, 1920, 1080, hMemDC, 0, 0, SRCCOPY);
		DeleteObject(hNewBitmap);
		if (Ph2Map->Map2X < 10)
		{
			Ph2Map->MapSpeed = 0;
			while (Truck->TruckX < 1500)
			{
				hDC = GetDC(hWnd);

				hMemDC = CreateCompatibleDC(hDC);
				hBackGroundDC = CreateCompatibleDC(hDC);
				hBackDC = CreateCompatibleDC(hDC);

				hBackBitmap = CreateCompatibleBitmap(hDC, 1920, 1080);
				hOldBitmap = (HBITMAP)SelectObject(hBackDC, hBackBitmap);

				hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_MapLast));
				SelectObject(hMemDC, hNewBitmap);
				BitBlt(hBackDC, Ph2Map->Map2X, 0, 1920, 1080, hMemDC, 0, 0, SRCCOPY);
				DeleteObject(hNewBitmap);

				Truck->TruckX += 2;
				if (Ph2Map->Truck12 < 10)
				{
					hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_Truck2));
					GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
					SelectObject(hMemDC, hNewBitmap);
					GdiTransparentBlt(hBackDC, Truck->TruckX, 430, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
					DeleteObject(hNewBitmap);
					Ph2Map->Truck12++;
				}
				else if (Ph2Map->Truck12 >= 10 && Ph2Map->Truck12 < 20)
				{
					hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_Truck1));
					GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
					SelectObject(hMemDC, hNewBitmap);
					GdiTransparentBlt(hBackDC, Truck->TruckX, 430, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
					DeleteObject(hNewBitmap);
					Ph2Map->Truck12++;
				}
				if (Ph2Map->Truck12 == 20)
					Ph2Map->Truck12 = 0;

				BitBlt(hDC, 0, 0, 1920, 1080, hBackDC, 0, 0, SRCCOPY);

				DeleteObject(SelectObject(hBackDC, hBackBitmap));
				DeleteObject(hNewBitmap);
				DeleteDC(hBackDC);
				DeleteDC(hMemDC);
				DeleteDC(hBackGroundDC);

				ReleaseDC(hWnd, hDC);
			}
			Ph2Map ->Finish = 2;
		}
	}

	if (Ph2Map->Truck12 < 10 || Ph2Map->MapSpeed == 0)
	{
		hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_Truck2));
		GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
		SelectObject(hMemDC, hNewBitmap);
		GdiTransparentBlt(hBackDC, Truck->TruckX, 430, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
		DeleteObject(hNewBitmap);
		Ph2Map->Truck12++;
	}
	else if (Ph2Map->Truck12 >= 10 && Ph2Map->Truck12 < 20 && Ph2Map->MapSpeed != 0)
	{
		hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_Truck1));
		GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
		SelectObject(hMemDC, hNewBitmap);
		GdiTransparentBlt(hBackDC, Truck->TruckX, 430, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
		DeleteObject(hNewBitmap);
		Ph2Map->Truck12++;
	}

	if (Ph2Box->BoxNum < 50)
	{
		hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(Ph2Box->BoxBit[Ph2Box->BoxKind[Ph2Box->BoxNum]]));
		GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
		SelectObject(hMemDC, hNewBitmap);
		GdiTransparentBlt(hBackDC, Truck->TruckX + 100, 530, 70, 70, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
		DeleteObject(hNewBitmap);
	}

	if (Ph2Map->Truck12 == 20)
		Ph2Map->Truck12 = 0;

	for (int i = 0; i < 60; i++)
	{
		if (House->Ph2HouseX[i] > -400 && House->Ph2HouseX[i] < 2320)
		{
			hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(House->House[i]));
			SelectObject(hMemDC, hNewBitmap);
			GdiTransparentBlt(hBackDC, House->Ph2HouseX[i], i < 30 ? 80 : 550, 400,400, hMemDC, 0, 0,400,400, RGB(255, 0, 228));
			DeleteObject(hNewBitmap);

			for (int k = 0; k < 50; k++)
				if (Ph2Box->BoxY[k] > 0 && Ph2Box->BoxY[k] < 2320)
				{
					if ((Ph2Box->BoxX[k] + 50 > House->Ph2HouseX[i] + 100) && (Ph2Box->BoxX[k] + 50 < House->Ph2HouseX[i] + 400 - 120) && (Ph2Box->BoxY[k] + 50 > House->Ph2HouseY[i] + 112) && (Ph2Box->BoxY[k] + 50 < House->Ph2HouseY[i] + 500))
					{
						Phone->Chat[k] = 1;
						Ph2Box->LastScore += 1;
						if ((Ph2Box->BoxKind[k] == 1 && (GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState(VK_RIGHT))) || (Ph2Box->BoxSpeed[k] > 18 && Ph2Box->BoxKind[k] == 1))
						{
							switch (House->XR[i])
							{
							case -1:
								House->House[i] = IDB_HouseFX1;
								Phone->Chat[k] = 2;
								Ph2Box->LastScore -= 3;
								break;
							case 0:
								House->House[i] = IDB_HouseFX3;
								Phone->Chat[k] = 2;
								Ph2Box->LastScore -= 3;
								break;
							case 1:
								House->House[i] = IDB_HouseFX2;
								Phone->Chat[k] = 2;
								Ph2Box->LastScore -= 3;
								break;
							}
						}
						else
						{
							switch (House->XR[i])
							{
							case -1:
								House->House[i] = IDB_Ph2House1OK;
								break;
							case 0:
								House->House[i] = IDB_Ph2House3OK;
								break;
							case 1:
								House->House[i] = IDB_Ph2House2OK;
								break;
							}
						}
						if (Ph2Box->BoxKind[k] == 2)
							Ph2Box->LastScore += 1;
						Ph2Box->BoxY[k] = 3000;
					}
					if (Ph2Box->BoxY[k] < 80 && Phone->Chat[k] != 1)
						Phone->Chat[k] = 2;
					if (Ph2Box->BoxY[k] > 1100 && Phone->Chat[k] != 1)
						Phone->Chat[k] = 2;
					if (Ph2Box->BoxX[k] < -100 && Phone->Chat[k] != 1 && Ph2Box->BoxKind[k] == 2)
					{
						Phone->Chat[k] = 2;
						Ph2Box->BoxY[k] = 3000;
					}
				}
		}
		House->Ph2HouseX[i] -= Ph2Map->MapSpeed;
	}

	hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_Gauge));
	GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
	SelectObject(hMemDC, hNewBitmap);
	GdiTransparentBlt(hBackDC, Truck->TruckX + 21, 490, Bitmap.bmWidth - (Truck->GaugeNum * 16), Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth - (Truck->GaugeNum * 16), Bitmap.bmHeight, RGB(255, 0, 228));
	DeleteObject(hNewBitmap);

	for (int i = 0; i < 50; i++)
	{
		if (Ph2Box->BoxY[i] > 0 && Ph2Box->BoxY[i] < 2200 && Phone->Chat[i] == 0)
		{
			if (Ph2Box->BoxX[i] != 0)
			{
				hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_Box1));
				GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
				SelectObject(hMemDC, hNewBitmap);
				GdiTransparentBlt(hBackDC, Ph2Box->BoxX[i], Ph2Box->BoxY[i], Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
				DeleteObject(hNewBitmap);
			}
			Ph2Box->BoxX[i] -= Ph2Map->MapSpeed;
			if (Ph2Box->PushWS[i] == 1)
				Ph2Box->BoxY[i] -= Ph2Box->BoxSpeed[i];
			else
				Ph2Box->BoxY[i] += Ph2Box->BoxSpeed[i];
		}
	}

	hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_Phone));
	GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
	SelectObject(hMemDC, hNewBitmap);
	GdiTransparentBlt(hBackDC, -62,563, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
	DeleteObject(hNewBitmap);

	for (int i = 0; i < 50; i++)
		if (Phone->Chat[i] == 1 || Phone->Chat[i] == 2)
		{
			if (Phone->ChatY[i] > 656)
			{
				hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(Phone->Chat[i] == 1 ? IDB_ChatGood : IDB_ChatSad));
				GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
				SelectObject(hMemDC, hNewBitmap);
				GdiTransparentBlt(hBackDC, 42, Phone->ChatY[i], Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
				DeleteObject(hNewBitmap);
			}

			if (Phone->ChatOK[i] == 0)
			{
				for (int k = 0; k < i; k++)
				{
					Phone->ChatY[k] -= 73;
				}
			}
			Phone->ChatOK[i] = 1;
		}
	hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_TruckNext));
	GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
	SelectObject(hMemDC, hNewBitmap);
	GdiTransparentBlt(hBackDC, 1450, 851, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
	DeleteObject(hNewBitmap);

	for (int i = 0; i < 3; i++)
	{
		if (Ph2Box->BoxNum + i < 50)
		{
			hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(Ph2Box->BoxBit[Ph2Box->BoxKind[Ph2Box->BoxNum + i]]));
			GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
			SelectObject(hMemDC, hNewBitmap);
			GdiTransparentBlt(hBackDC, 1470 + i * 96, 914, 86, 86, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
			DeleteObject(hNewBitmap);
		}
	}

	BitBlt(hDC, 0, 0, 1920, 1080, hBackDC, 0, 0, SRCCOPY);

	DeleteObject(SelectObject(hBackDC, hBackBitmap));
	DeleteObject(hNewBitmap);
	DeleteDC(hBackDC);
	DeleteDC(hMemDC);
	DeleteDC(hBackGroundDC);

	ReleaseDC(hWnd, hDC);
}

void HouseSetter(struct House *House) //집의 위치 랜덤하게 시작부분에서 배정
{
	srand(GetTickCount64());
	
	for (int i = 0; i < 30; i++)
	{
		House->XR[i] = rand() % 3 - 1;
		House->Ph2HouseX[i] = (1200 * (i + 1)) + House->XR[i] * 450;
		House->Ph2HouseY[i] = 80;
		switch (House->XR[i])
		{
		case -1:
			House->House[i] = IDB_Ph2House1;
			break;
		case 0:
			House->House[i] = IDB_Ph2House2;
			break;
		case 1:
			House->House[i] = IDB_Ph2House3;
			break;
		}
	}

	for (int i = 30; i < 60; i++)
	{
		House->XR[i] = rand() % 3 - 1;
		House->Ph2HouseX[i] = (1200 * (i - 30 + 1)) + House->XR[i] * 450;
		House->Ph2HouseY[i] = 550;
		switch (House->XR[i])
		{
		case -1:
			House->House[i] = IDB_Ph2House1;
			break;
		case 0:
			House->House[i] = IDB_Ph2House2;
			break;
		case 1:
			House->House[i] = IDB_Ph2House3;
			break;
		}
	}
}

void TruckMapSet(struct Ph2Map *Ph2Map, struct Truck *Truck, struct Ph2Box *Ph2Box, struct Phone *Phone, struct StickManValue StickVal)//2페이즈의 모든 변수 초기화
{
	switch (StickVal.City)
	{
	case 0:
		Ph2Box->BoxBit[0] = IDB_Box1;
		Ph2Box->BoxBit[1] = IDB_FBox1;
		Ph2Box->BoxBit[2] = IDB_BigBox1;
		break;
	case 1:
		Ph2Box->BoxBit[0] = IDB_Box2;
		Ph2Box->BoxBit[1] = IDB_FBox2;
		Ph2Box->BoxBit[2] = IDB_BigBox2;
		break;
	case 2:
		Ph2Box->BoxBit[0] = IDB_Box3;
		Ph2Box->BoxBit[1] = IDB_FBox3;
		Ph2Box->BoxBit[2] = IDB_BigBox3;
		break;
	case 3:
		Ph2Box->BoxBit[0] = IDB_Box4;
		Ph2Box->BoxBit[1] = IDB_FBox4;
		Ph2Box->BoxBit[2] = IDB_BigBox4;
		break;
	}
	Ph2Map->Map1X = 0;
	Ph2Map->Map2X = 1920;
	Ph2Map->MapSpeed = 0;
	Ph2Map->Truck12 = 0;
	Ph2Map->MapSlowDelay = 0;
	Ph2Map->Finish = 0;

	Ph2Map->MapFinishDraw = 0;

	Truck->TruckSpeed = 4;
	Truck->TruckX = 400;
	Truck->GaugeNum = 0;
	
	Ph2Box->BoxNum = 0;

	for (int i = 0; i < 50; i++)
	{
		Ph2Box->BoxX[i] = 3000;
		Ph2Box->BoxY[i] = 3000;
		Ph2Box->PushWS[i] = 0;
		Phone->Chat[i] = 0;
		Phone->ChatY[i] = 944;
		Phone->ChatOK[i] = 0;
		Ph2Box->BoxSpeed[i] = 8;
		Ph2Box->BoxKind[i] = 0;
	}
}

void EndingRender(HBITMAP Bit) //마지막 엔딩에서의 이미지 출력 처리
{
	HDC hDC, hMemDC;
	HBITMAP hBackBitmap, hOldBitmap, hNewBitmap;
	BITMAP Bitmap;

	hDC = GetDC(hWnd);

	hMemDC = CreateCompatibleDC(hDC);

	hBackBitmap = CreateCompatibleBitmap(hDC, 1920, 1080);

	hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(Bit));
	GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
	SelectObject(hMemDC, hNewBitmap);
	DeleteObject(hNewBitmap);

	BitBlt(hDC, 0, 0, 1920, 1080, hMemDC, 0, 0, SRCCOPY);

	DeleteObject(SelectObject(hMemDC, hDC));
	DeleteObject(hNewBitmap);
	DeleteObject(hBackBitmap);
	DeleteDC(hMemDC);

	ReleaseDC(hWnd, hDC);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void SaveRender(char Name[], struct Save Save, char Job[]) //저장할때의 이미지 출력 처리
{
	HDC hDC, hMemDC;
	static HDC hBackDC;
	HBITMAP hBackBitmap, hOldBitmap, hNewBitmap;
	BITMAP Bitmap;

	RECT Namert = { 700,456,1222,501 };
	RECT Jobrt = {1222, 456,1332,500};

	hDC = GetDC(hWnd);

	hMemDC = CreateCompatibleDC(hDC);
	hBackDC = CreateCompatibleDC(hDC);

	hBackBitmap = CreateCompatibleBitmap(hDC, 1920, 1080);
	hOldBitmap = (HBITMAP)SelectObject(hBackDC, hBackBitmap);

	hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LeaderBoardPut));
	GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
	SelectObject(hMemDC, hNewBitmap);
	GdiTransparentBlt(hBackDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, RGB(255, 0, 228));
	DeleteObject(hNewBitmap);

	HFONT font = CreateFont(33, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"여기어때 잘난체");

	(HFONT)SelectObject(hBackDC, font);

	SetTextColor(hBackDC, RGB(0, 0, 0));
	SetBkMode(hBackDC, TRANSPARENT);

	DrawTextA(hBackDC, Name, -1, &Namert, DT_VCENTER | DT_CENTER);

	DrawTextA(hBackDC, Job, -1, &Jobrt, DT_VCENTER | DT_CENTER);

	BitBlt(hDC, 0, 0, 1920, 1080, hBackDC, 0, 0, SRCCOPY);

	DeleteObject(SelectObject(hBackDC, hBackBitmap));
	DeleteObject(hNewBitmap);
	DeleteDC(hBackDC);
	DeleteDC(hMemDC);

	ReleaseDC(hWnd, hDC);
}

void LeaderBoardRender() //점수판(파일입출력)할때의 이미지 및 글 출력 처리
{
	char chSub[300];
	char ch[100][100];
	char c;
	FILE* inp = fopen("out.txt", "r");

	HDC hDC, hMemDC;
	static HDC hBackDC;
	HBITMAP hBackBitmap, hOldBitmap, hNewBitmap;
	BITMAP Bitmap;

	RECT Job = { 725,308,781,343 };
	RECT Name = { 787,308,1207,343 };

	hDC = GetDC(hWnd);

	hMemDC = CreateCompatibleDC(hDC);
	hBackDC = CreateCompatibleDC(hDC);

	hBackBitmap = CreateCompatibleBitmap(hDC, 1920, 1080);
	hOldBitmap = (HBITMAP)SelectObject(hBackDC, hBackBitmap);

	hNewBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LeaderBoard));
	GetObject(hNewBitmap, sizeof(BITMAP), &Bitmap);
	SelectObject(hMemDC, hNewBitmap);
	BitBlt(hBackDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, SRCCOPY);
	DeleteObject(hNewBitmap);

	HFONT font = CreateFont(23, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"여기어때 잘난체");

	(HFONT)SelectObject(hBackDC, font);

	SetTextColor(hBackDC, RGB(0, 0, 0));

	for (int i = 0; i < 100; i++)
	{
		fgets(ch[i], 100, inp);
		char* ptr = strtok(ch[i], " ");

		DrawTextA(hBackDC, ptr, -1, &Job, DT_VCENTER | DT_CENTER);
		ptr = strtok(NULL, " ");
		DrawTextA(hBackDC, ptr, -1, &Name, DT_VCENTER | DT_CENTER);
		Job.top += 38;
		Job.bottom += 38;
		Name.top += 38;
		Name.bottom += 38;

		if ((c = fgetc(inp)) == EOF)
			break;
		fseek(inp, -1, SEEK_CUR);
	}
	fclose(inp);
	
	BitBlt(hDC, 0, 0, 1920, 1080, hBackDC, 0, 0, SRCCOPY);

	DeleteObject(SelectObject(hBackDC, hBackBitmap));
	DeleteObject(hNewBitmap);
	DeleteDC(hBackDC);
	DeleteDC(hMemDC);

	ReleaseDC(hWnd, hDC);
}

int main()
{
RESTART: //다시시작을 위한 goto문
	CursorView(0);

	hWnd = GetConsoleWindow();
	hInst = GetModuleHandle(NULL);

	struct StickMan Stick[5];
	struct StickManValue StickVal;
	struct Box Box;
	struct Collider Coll;
	struct Stamp Stamp;

	struct Ph2Map Ph2Map;
	struct Truck Truck;
	struct House House;
	struct Ph2Box Ph2Box;
	struct Phone Phone;
	struct Save Save;
	srand(GetTickCount64());

	char CityName[4][10] = { {"용산구"}, {"강남구"}, {"영등포"}, {"마포구"} };

	int StoreI = 0;

	int NowOnW = 0;
	int NowOnS = 0;

	char* Job[] = { "사퇴" };

	char Name[100] = { 0 };

	int Double = 0;


	HBITMAP hBackGroundBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_Ph2Map));

	for (int k = 0; k < 24; k++)
	{
		Coll.MaxX[k] = 0;
		Coll.MinX[k] = 0;
		Coll.MaxY[k] = 0;
		Coll.MinY[k] = 0;
	}

	StickVal.City = rand() % 4;
	int NameNum = 0;
	StickManBoxSet(Stick, &Box, &StickVal, &Stamp);
	HouseSetter(&House);
	TruckMapSet(&Ph2Map, &Truck, &Ph2Box, &Phone, StickVal);

	RandomBoxPos(&Box, &Coll);

	hBackGroundBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_Ph2Map));
	
	NameNum = 0;

	Double = 0;

	srand(GetTickCount64());

	///////여기까지 게임 시작 전 모든부분 셋팅

	while (1) //홈화면
	{
		StampPos(&Stamp);
		StartRender(CityName[StickVal.City], Stamp);
		if (GetAsyncKeyState(MK_LBUTTON) && Stamp.StampX > 693 && Stamp.StampX < 1250 && Stamp.StampY > 142 && Stamp.StampY < 800)
		{
			Stamp.FStampX = Stamp.StampX;
			Stamp.FStampY = Stamp.StampY;

			StampAni(CityName[StickVal.City], &Stamp);
			break;
		}//시작부분

		if (GetAsyncKeyState(MK_LBUTTON) && Stamp.StampX > 1455 && Stamp.StampX < 1670 && Stamp.StampY > 193 && Stamp.StampY < 400)
		{
			Stamp.FStampX = Stamp.StampX;
			Stamp.FStampY = Stamp.StampY;

			StampAni(CityName[StickVal.City], &Stamp);
			Start = clock();
			End = clock();

			while ((float)(End - Start) / CLOCKS_PER_SEC < 0.5)
			{
				End = clock();
			}
			Start2 = clock();
			while (1)
			{
				StampPos(&Stamp);
				LeaderBoardRender();
				if (GetAsyncKeyState(MK_LBUTTON) & 0x8000 && Stamp.StampX > 255 && Stamp.StampX < 438 && Stamp.StampY > 717 && Stamp.StampY < 900)
				{
					goto RESTART;
				}
			}
		}//점수판(파일입출력)

		if (GetAsyncKeyState(MK_LBUTTON) && Stamp.StampX > 1522 && Stamp.StampX < 1650 && Stamp.StampY > 593 && Stamp.StampY < 860)
		{
			Stamp.FStampX = Stamp.StampX;
			Stamp.FStampY = Stamp.StampY;

			StampAni(CityName[StickVal.City], &Stamp);

			return 0;
		}//나가기
	}
	Start = clock();
	End = clock();

	while ((float)(End - Start) / CLOCKS_PER_SEC < 1)//시작 전 딜레이
	{
		End = clock();
	}	 
	Start2 = clock();

	//while (1) //1페이즈 
	//{
	//	End2 = clock();
	//	if (Start != 0)
	//	{
	//		End = clock();
	//		if ((float)(End - Start) / CLOCKS_PER_SEC > 2)
	//		{
	//			Box.Fault = 0;
	//			Start = 0;
	//			End = 0;
	//		}
	//	}
	//	if (30 < (int)((End2 - Start2) / CLOCKS_PER_SEC))
	//		break;
	//	if (Moving(&StickVal, &Coll, Stick, &Box, VK_UP, 0, -StickVal.StickManSpeed, 0)) continue; //움직이는 부분

	//	if (Moving(&StickVal, &Coll, Stick, &Box, VK_DOWN, 0, StickVal.StickManSpeed, 1)) continue;

	//	if (Moving(&StickVal, &Coll, Stick, &Box, VK_LEFT, -StickVal.StickManSpeed, 0, 2)) continue;

	//	if (Moving(&StickVal, &Coll, Stick, &Box, VK_RIGHT, StickVal.StickManSpeed, 0, 3)) continue;

	//	else
	//	{
	//		if (StickVal.GetBox == 1) //박스를 집었을때 플레이어 이미지 변경
	//		{
	//			Render(Stick[4].FBit, &StickVal, &Box, 4);
	//		}
	//		else Render(Stick[4].Bit, &StickVal, &Box, 4);
	//	}
	//}
	BoxFBigSet(Box, &Ph2Box); //2페이즈 시작 전 셋팅

	Ph2Box.LastScore += Box.FirstScore; //시작부분의 점수 누적

	while (Ph2Map.Finish != 2) //2페이즈 시작
	{
		GaugeEnd = clock();

		Truck.GaugeNum = 11 - ((float)(GaugeEnd - GaugeStart) / CLOCKS_PER_SEC) * 18;
			
		if (Truck.GaugeNum < 0) Truck.GaugeNum = 0;

		Ph2Render(&Ph2Map, &Truck, &House, &Ph2Box, &Phone, hBackGroundBitmap);

		if (Ph2Map.MapSpeed < 11 && GetAsyncKeyState(VK_RIGHT) && Ph2Map.Finish == 0)
		{
			Double++;
			Ph2Map.MapSpeed = Double/10;
		}
		else if (Ph2Map.MapSpeed >= 11 && GetAsyncKeyState(VK_RIGHT) && Truck.TruckX + Truck.TruckSpeed < 1600 && Ph2Map.Finish == 0)
		{
			Truck.TruckX += Truck.TruckSpeed;
		}
		else if (Ph2Map.MapSpeed >= 11 && GetAsyncKeyState(VK_LEFT) && Truck.TruckX + Truck.TruckSpeed > 50 && Ph2Map.Finish == 0)
		{
			Truck.TruckX -= Truck.TruckSpeed;
		}

		if (Ph2Box.BoxNum < 50)
		{
			if ((GetAsyncKeyState('w') & 0x8000  || GetAsyncKeyState('W') & 0x8000) && (!GetAsyncKeyState('s')) && (!GetAsyncKeyState('S')) && NowOnW != 1)
			{
				GaugeStart = clock();
				NowOnW = 1;
				continue;
			}

			if ((GetAsyncKeyState('s') & 0x8000 || GetAsyncKeyState('S') & 0x8000) && ((!(GetAsyncKeyState('w'))) && (!GetAsyncKeyState('W'))) && NowOnS != 1)
			{
				GaugeStart = clock();
				NowOnS = 1;
				continue;
			}
		}

		if ((!GetAsyncKeyState('s')) && (!GetAsyncKeyState('S')) && Ph2Box.BoxNum < 50)
		{
			if (NowOnS == 1)
			{
				Ph2Box.BoxX[Ph2Box.BoxNum] = Truck.TruckX;
				Ph2Box.BoxY[Ph2Box.BoxNum] = 600;
				Ph2Box.PushWS[Ph2Box.BoxNum] = 2;

				if (Ph2Box.BoxKind[Ph2Box.BoxNum] == 2)
				{
					Ph2Box.BoxSpeed[Ph2Box.BoxNum] = (((float)(GaugeEnd - GaugeStart) / CLOCKS_PER_SEC) * 10);
					if (Ph2Box.BoxSpeed[Ph2Box.BoxNum] < 3)
						Ph2Box.BoxSpeed[Ph2Box.BoxNum] = 0;
				}
				else
					Ph2Box.BoxSpeed[Ph2Box.BoxNum] = 10 + (((float)(GaugeEnd - GaugeStart) / CLOCKS_PER_SEC) * 18);

				if (Ph2Box.BoxSpeed[Ph2Box.BoxNum] > 23)
					Ph2Box.BoxSpeed[Ph2Box.BoxNum] = 23;
				Ph2Box.BoxNum++;
			}
			NowOnS = 0;
		}
		if (((!(GetAsyncKeyState('w'))) && (!GetAsyncKeyState('W'))) && Ph2Box.BoxNum < 50)
		{
			if (NowOnW == 1)
			{
				Ph2Box.BoxX[Ph2Box.BoxNum] = Truck.TruckX;
				Ph2Box.BoxY[Ph2Box.BoxNum] = 430;
				Ph2Box.PushWS[Ph2Box.BoxNum] = 1;

				if (Ph2Box.BoxKind[Ph2Box.BoxNum] == 2)
				{
					Ph2Box.BoxSpeed[Ph2Box.BoxNum] = (((float)(GaugeEnd - GaugeStart) / CLOCKS_PER_SEC) * 10);
					if (Ph2Box.BoxSpeed[Ph2Box.BoxNum] < 3)
						Ph2Box.BoxSpeed[Ph2Box.BoxNum] = 0;
				}
				else
					Ph2Box.BoxSpeed[Ph2Box.BoxNum] = 10 + (((float)(GaugeEnd - GaugeStart) / CLOCKS_PER_SEC) * 18);
				
				if (Ph2Box.BoxSpeed[Ph2Box.BoxNum] > 23)
					Ph2Box.BoxSpeed[Ph2Box.BoxNum] = 23;
				Ph2Box.BoxNum++;
			}
			NowOnW = 0;
		}

		if ((!GetAsyncKeyState('s')) && (!GetAsyncKeyState('S')) && ((!(GetAsyncKeyState('w'))) && (!GetAsyncKeyState('W'))))
			GaugeStart = clock();
	}

	while (1) //끝난 뒤 엔딩
	{
		int X, Y;
		POINT Cur;
		GetCursorPos(&Cur);
		ScreenToClient(hWnd, &Cur);
		X = Cur.x / 1535.0 * 1920;
		Y = Cur.y / 863.0 * 1080;
		if (X > 291 && X < 665 && Y > 580 && Y < 746 && GetAsyncKeyState(MK_LBUTTON) & 0x8000) //다시하기, 저장
		{
			if (Name[0] != 0) //저장을 안할 시 패스
			{
				FILE* inp = fopen("out.txt", "a");
				fprintf(inp, "%s", Job[0]);
				fprintf(inp, "%s\n", Name);
				fclose(inp);
			}

			goto RESTART;
		}
		
		if (X > 291 && X < 665 && Y > 786 && Y < 952 && GetAsyncKeyState(MK_LBUTTON) & 0x8000)// 저장(이름 입력 및 직급 저장)
		{
			while (_kbhit()) _getch();
			X = 0, Y = 0;
			NameNum=0;
			for (int i = 0; i < 100; i++)
			{
				Name[i] = 0;
			}
			while (1)
			{
				if (_kbhit())
				{
					char Name1 = getch();
					if (Name1 == 8)
					{
						if (NameNum == 0)
							continue;
						Name[--NameNum] = 0;
						continue;
					}

					if (Name1 == 13)
					{
						break;
					}

					if (NameNum > 90)
						continue;
					Name[NameNum] = Name1;
					NameNum++;
				}
				SaveRender(Name, Save,Job[0]);
			}
		}

		if (Ph2Box.LastScore < 20)
		{
			EndingRender(IDB_EndMap3);
			Job[0] = "사퇴 ";
		}

		else if (Ph2Box.LastScore < 45)
		{
			EndingRender(IDB_EndMap2);
			Job[0] = "직원 ";
		}

		else if (Ph2Box.LastScore >= 45)
		{
			EndingRender(IDB_EndMap1);
			Job[0] = "사장 ";
		}

		//점수에 따라 직급 분배
	}
}