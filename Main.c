#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <Windows.h>

#define FIELD_WIDTH 12
#define FIELD_HEIGHT 18
int screenWidth, screenHeight;
LPCSTR TITLE = "TetrisCLI";

typedef unsigned char uchar;

typedef enum
{
	typeI, typeleftL, typerightL, typeblock,
	typeleftZ, typerightZ, typeT
} Tetromino_Type;

typedef struct
{
	char sprite[16];
	char originalSprite[16];
	Tetromino_Type type;
	COORD pos;
	uint8_t rotation; // 0 - 1 - 2 - 3
	bool isPlaced;
} Tetromino;
Tetromino* curTetromino;

// Starting position of our tetrominos
COORD startingPosition;

// 4x4 Tetromino Sprites
const char spriteI	   		[]		= "..X...X...X...X.";
const char spriteleftL 		[] 		= ".....X...XXX....";
const char spriterightL		[] 		= "......X.XXX.....";
const char spriteblock		[] 		= ".....XX..XX.....";
const char spriterightZ		[] 		= "......XX.XX.....";
const char spriteleftZ		[] 		= ".....XX...XX....";
const char spriteT			[]		= "......X..XXX....";

const char message[] = "Keybindings (They're not case sensitive):\n\tArrow Keys: Movement\n\tC: Rotate Piece\n\tEsc/Q: Quit\n";
/*
*	Keybindings (They're not case sensitive):
*		Arrow Keys: Movement
*		C: Rotate Piece
*		Esc/Q: Quit
*/

void CreateNewTetromino();
void Rotate();

int main(void)
{
	newgame:
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0,
												NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hConsole, &csbi);
		screenWidth = csbi.dwSize.X;
		screenHeight = csbi.dwSize.Y;
	}	// This scope is only put here for deallocating the now useless csbi

	SetConsoleTitleA(TITLE);
	if (!SetConsoleActiveScreenBuffer(hConsole)) return EXIT_FAILURE;
	DWORD dwCharsWritten;

	int xFieldOffset = 4, yFieldOffset = 4;

	uchar* screen = (uchar*) malloc(screenWidth * screenHeight);
	for (int i = 0; i < screenWidth * screenHeight; i++) // Clearing the buffer
		screen[i] = ' ';

	startingPosition.X = (FIELD_WIDTH / 2) + xFieldOffset;
	startingPosition.Y = yFieldOffset;

	curTetromino = malloc(sizeof(Tetromino));
	CreateNewTetromino(); // We'll have to start with something

	bool keepRunning = true;
	while (keepRunning)
	{
		// Timing
		Sleep(50);

		// +++++++++++ Input +++++++++++
		
		// --- Arrow Keys (Movement) ---
		if (curTetromino->pos.X > xFieldOffset+1 && GetKeyState(VK_LEFT) & 0x8000)
			curTetromino->pos.X--;

		else if (curTetromino->pos.X < (FIELD_WIDTH-1) && GetKeyState(VK_RIGHT) & 0x8000)
			curTetromino->pos.X++;	

		if (curTetromino->pos.Y < FIELD_HEIGHT-1 && GetKeyState(VK_DOWN) & 0x8000)
			curTetromino->pos.Y++;

		if (GetKeyState(0x41) & 0x8000) // A KEY
			Rotate();

		// --- Exit ---
		if (GetKeyState(0x1B) & 0x8000) // ESC Key
			break;

		// +++++++++++ Game Logic ++++++++++


		// +++++++++++ Rendering +++++++++++

		// --- the field ---
		for (int y = 0; y < FIELD_HEIGHT; y++)
			for (int x = 0; x < FIELD_WIDTH; x++)
				// If the pixel is a border, put a #
				screen[(y + yFieldOffset) * screenWidth + (x + xFieldOffset)] = (x == 0 || x == FIELD_WIDTH - 1 || y == FIELD_HEIGHT - 1) ? '#' : ' ';

		// --- the tetromino ---
		// UNCHECKABLE; THINK OF A BETTER WAY, it's gonna have to do for now
		for (int i = 0; i < 4; i++)
		{
			memcpy(&screen[(curTetromino->pos.Y + i) * screenWidth + curTetromino->pos.X], &curTetromino->sprite[i*4], 4);
		}



		// --- Swap Buffers ---
		WriteConsoleOutputCharacterA(hConsole, screen, screenHeight * screenWidth, (COORD){0,0}, &dwCharsWritten);
	}

	CloseHandle(hConsole);
	free(screen);
	
	system("pause");
	#if 0
	goto newgame;
	#endif
	return EXIT_SUCCESS;
}

void CreateNewTetromino()
{
	curTetromino->pos = startingPosition;
	curTetromino->rotation = 0;

	srand(time(NULL));
	int random = rand() % 7;
	
	switch (random)
	{
		case 0:
			curTetromino->type = typeI;
			strcpy(curTetromino->sprite, spriteI);
			strcpy(curTetromino->originalSprite, spriteI); // The only way that worked for me...
			break;
		case 1:
			curTetromino->type = typeleftL;
			strcpy(curTetromino->sprite, spriteleftL);
			strcpy(curTetromino->originalSprite, spriteleftL);
			break;
		case 2:
			curTetromino->type = typerightL;
			strcpy(curTetromino->sprite, spriterightL);
			strcpy(curTetromino->originalSprite, spriterightL);
			break;
		case 3:
			curTetromino->type = typeblock;
			strcpy(curTetromino->sprite, spriteblock);
			strcpy(curTetromino->originalSprite, spriteblock);
			break;
		case 4:
			curTetromino->type = typeleftZ;
			strcpy(curTetromino->sprite, spriteleftZ);
			strcpy(curTetromino->originalSprite, spriteleftZ);
			break;
		case 5:
			curTetromino->type = typerightZ;
			strcpy(curTetromino->sprite, spriterightZ);
			strcpy(curTetromino->originalSprite, spriterightZ);
			break;
		case 6:
			curTetromino->type = typeT;
			strcpy(curTetromino->sprite, spriteT);
			strcpy(curTetromino->originalSprite, spriteT);
			break;
	}
}

void Rotate()
{
	curTetromino->rotation++;

	switch (curTetromino->rotation % 4)
	{
		case 0: // 0 deg
			for (int y = 0; y < 4; y++)
				for (int x = 0; x < 4; x++)
					curTetromino->sprite[y * 4 + x] = curTetromino->originalSprite[y * 4 + x];
			break;
			
		case 1: // 90 deg
			for (int y = 0; y < 4; y++)
				for (int x = 0; x < 4; x++)
					curTetromino->sprite[y * 4 + x] = curTetromino->originalSprite[12 + y - (x*4)];
			break;

		case 2: // 180 deg
			for (int y = 0; y < 4; y++)
				for (int x = 0; x < 4; x++)
					curTetromino->sprite[y * 4 + x] = curTetromino->originalSprite[15 - (y*4) - x];
			break;
			
		case 3: // 270 deg
			for (int y = 0; y < 4; y++)
				for (int x = 0; x < 4; x++)
					curTetromino->sprite[y * 4 + x] = curTetromino->originalSprite[3 - y + (x*4)];
			break;
	}
}