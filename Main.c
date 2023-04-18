#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <Windows.h>

#define FIELD_WIDTH 12
#define FIELD_HEIGHT 18
int screenWidth, screenHeight;

LPCSTR TITLE = "TetrisCLI";

typedef unsigned char uchar;

enum Tetromino_Type
{
	typeI, typeleftL, typerightL, typeblock,
	typeleftZ, typerightZ, typeT
};

typedef struct
{
	COORD position;
	int type;
	char sprite[16]; // 4x4
} Tetromino;
Tetromino* curT;
int curRotation;

// 4x4 Tetromino Sprites
const char* spriteI 			= "..X...X...X...X.";
const char* spriteleftL 		= ".....X...XXX....";
const char* spriterightL 		= "......X.XXX.....";
const char* spriteblock 		= ".....XX..XX.....";
const char* spriterightZ 		= "......XX.XX.....";
const char* spriteleftZ 		= ".....XX...XX....";
const char* spriteT 			= "......X..XXX....";

Tetromino CreateTetromino()
{
	Tetromino newTetromino;

	srand(time(NULL));
	int randomizer = rand() % 7;
	switch (randomizer)
	{
		case 0:
			memcpy(newTetromino.sprite, spriteI, 16);
			newTetromino.type = typeI;
			break;
		case 1:
			memcpy(newTetromino.sprite, spriteleftL, 16);
			newTetromino.type = typeleftL;
			break;
		case 2:
			memcpy(newTetromino.sprite, spriterightL, 16);
			newTetromino.type = typerightL;
			break;
		case 3:
			memcpy(newTetromino.sprite, spriteblock, 16);
			newTetromino.type = typeblock;
			break;
		case 4:
			memcpy(newTetromino.sprite, spriterightZ, 16);
			newTetromino.type = typerightZ;
			break;
		case 5:
			memcpy(newTetromino.sprite, spriteleftZ, 16);
			newTetromino.type = typeleftZ;
			break;
		case 6:
			memcpy(newTetromino.sprite, spriteT, 16);
			newTetromino.type = typeT;
			break;
	}
	
	newTetromino.position.X = FIELD_WIDTH / 2;
	newTetromino.position.Y = 5;

	return newTetromino;
}

int GetIndexerFormula(int x, int y, int r)
{
	switch (r % 4)
	{
		case 0: return y * 4 + x;
		case 1: return 12 + y - (x*4);
		case 2: return 15 - (y*4) - x;
		case 3: return 3 + y * (x*4);
	}

	// You'll never reach here
	return -1;
}

void Rotate90()
{
	for (int y = 0; y < 4; y++)
		for (int x = 0; x < 4; x++)
		{
			curT->sprite[GetIndexerFormula(x,y,curRotation)] = curT->sprite[GetIndexerFormula(x,y,curRotation+1)];
		}

	curRotation++;
}

enum Movement
{
	MOV_LEFT, MOV_RIGHT, MOV_DOWN
};

bool CanMove(enum Movement movement)
{
	COORD toMove;
	switch (movement)
	{
		case MOV_LEFT:
			toMove.X = curT->position.X - 1;
			toMove.Y = curT->position.Y;
			break;
		case MOV_RIGHT:
			toMove.X = curT->position.X + 1;
			toMove.Y = curT->position.Y;
			break;
		case MOV_DOWN:
			toMove.X = curT->position.X;
			toMove.Y = curT->position.Y + 1;
			break;
	}



	for (int y = 0; y < 4; y++)
		for (int x = 0; x < 4; x++)
		{
			//if (curT->sprite[GetIndexerFormula(x,y)] == 'X' && )
		}

	return true;
}



int main(void)
{
	{
		CONSOLE_SCREEN_BUFFER_INFO CSBI;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &CSBI);
		screenWidth = CSBI.dwSize.X;
		screenHeight = CSBI.dwSize.Y;
	}

	uchar field[FIELD_WIDTH * FIELD_HEIGHT]; // Field width x Field Height

start:
	// Initialise the field
	for (int y = 0; y < FIELD_HEIGHT; y++)
		for (int x = 0; x < FIELD_WIDTH; x++)
			field[y * FIELD_WIDTH + x] = (x == 0 || x == FIELD_WIDTH - 1 || y == FIELD_HEIGHT - 1) ? 5 : 0; // 5 for Wall, 0 for nothing


	uchar* screen = (uchar*)malloc(screenWidth * screenHeight);
	for (int i = 0; i < screenWidth * screenHeight; i++)
		screen[i] = ' ';

	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
												0, NULL,
												CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleTitleA(TITLE);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	curT = malloc(sizeof(Tetromino));
	*curT = CreateTetromino();

	bool bRunning = true;
	while (bRunning)
	{
		// Frame Check
		Sleep(50);

		// =============== Input ===============
		if (GetAsyncKeyState(VK_ESCAPE)) bRunning = false;

		if (GetAsyncKeyState(0x41) & 0x8000) // A Key, used for rotation
		{
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{

			// Move Right
			if (CanMove(MOV_RIGHT))
			{
				curT->position.X += 1;
			}

		} else if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			// Move Left
			if (CanMove(MOV_LEFT))
			{
				curT->position.X -= 1;
			}
		}
		if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			// Accelerate downwards
			if (CanMove(MOV_DOWN))
			{
				curT->position.Y += 1;
			}
		}

		// =============== Rendering ===============



		// Draw Borders / Clear
		for (int y = 0; y < FIELD_HEIGHT; y++)
			for (int x = 0; x < FIELD_WIDTH; x++)
				if (screen[(y+5) * screenWidth + (x+5)] == '.') screen[(y+5) * screenWidth + (x+5)] = ' ';
				else screen[(y+5) * screenWidth + (x+5)] = (field[y * FIELD_WIDTH + x] == 5) ? '#' : ' ';

		// Copy Current Tetromino's Data To Screen
		for (int i = 0; i < 4; i++)
		{
			memcpy(&screen[screenWidth * (curT->position.Y + i) + curT->position.X], &curT->sprite[i*4], 4);
		}


		// Display New Buffer
		WriteConsoleOutputCharacterA(hConsole, screen, screenWidth * screenHeight, (COORD){0,0}, &dwBytesWritten);
	}
	

	CloseHandle(hConsole);
	free(screen);

	int playAgain;
prompt:
	printf("Play Again ?\n[1]: Yes\t[2]: Quit\n");
	scanf_s("%i", &playAgain);
	if (playAgain == 1)
		goto start;
	else if (playAgain == 2)
		return 0;
	else
	{
		printf("Invalid Option!\n");
		goto prompt;
	}
}