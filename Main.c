#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <Windows.h>

#define SCR_WIDTH 16
#define SCR_HEIGTH 18
LPCSTR TITLE = "TetrisCLI";

// INDEXES
// 1 - I, 2 - leftL, 3 - rightL, 4 - 4x4, 5 - leftZ, 6 - rightZ, 7 - _i_

typedef struct { uint8_t x, y; } Position;
enum Tetromino_Type
{
	typeI, typeleftL, typerightL, typeblock,
	typeleftZ, typerightZ, typeT
};

typedef struct
{
	char sprite[16];
	Position position;
	enum Tetromino_Type type;
} Tetromino;

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
	int randomizer = rand() % 7;
	switch (randomizer)
	{
		case 0:
			memcpy(&newTetromino, spriteI, 16);
			newTetromino.type = typeI;
			break;
		case 1:
			memcpy(&newTetromino, spriteleftL, 16);
			newTetromino.type = typeleftL;
			break;
		case 2:
			memcpy(&newTetromino, spriterightL, 16);
			newTetromino.type = typerightL;
			break;
		case 3:
			memcpy(&newTetromino, spriteblock, 16);
			newTetromino.type = typeblock;
			break;
		case 4:
			memcpy(&newTetromino, spriterightZ, 16);
			newTetromino.type = typerightZ;
			break;
		case 5:
			memcpy(&newTetromino, spriteleftZ, 16);
			newTetromino.type = typeleftZ;
			break;
		case 6:
			memcpy(&newTetromino, spriteT, 16);
			newTetromino.type = typeT;
			break;
	}
	
	newTetromino.position.x = SCR_WIDTH / 2;
	newTetromino.position.y = SCR_HEIGTH - 4;

	return newTetromino;
}

int main(void)
{
	const char* screen;
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
												0, NULL,
												CONSOLE_TEXTMODE_BUFFER, NULL);
	COORD ScreenBufferCoord = { SCR_WIDTH, SCR_HEIGTH };
	SetConsoleTitleA(TITLE);
	SetConsoleScreenBufferSize(hConsole, ScreenBufferCoord);
	SetConsoleActiveScreenBuffer(hConsole);
	
	bool bRunning = true;
	while (bRunning)
	{
		Tetromino* inUsageTetromino;

		// Time/Intervals
		Sleep(50);

		// Input
		if (GetAsyncKeyState(0x41) & 0x8000) // 
		{
		}

		if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			// Rotate Right
		} else if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			// Rotate Left
		}

		if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			// Accelerate downwards
		}

		// Rendering
		for (int h = 0; h < SCR_HEIGTH; h++)
		{
			for (int w = 0; w < SCR_WIDTH; w++)
			{

			}
		}
	}
	
	CloseHandle(hConsole);
	return 0;
}