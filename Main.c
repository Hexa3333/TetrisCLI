#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <Windows.h>

LPCSTR TITLE = "TetrisCLI";

#define FIELD_WIDTH 12
#define FIELD_HEIGHT 18

int screenWidth, screenHeight;
int score;

bool bField[FIELD_WIDTH * FIELD_HEIGHT];
int xFieldOffset = 4, yFieldOffset = 4;

typedef unsigned char uchar;
#define FILLED true
#define EMPTY false

typedef enum
{
	typeI, typeleftL, typerightL, typeblock,
	typeleftZ, typerightZ, typeT
} Tetromino_Type;

struct _Tetromino
{
	char sprite[16];
	char originalSprite[16];
	int cellOffsetFromL, cellOffsetFromR, cellOffsetFromBottom;
	Tetromino_Type type;
	COORD pos;
	uint8_t rotation; // 0 - 1 - 2 - 3
	bool isPlaced;
} curTetromino;

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
void Emplace();
bool CanMoveDown();

void dbgPrintField()
{
	FILE* fpTable = fopen("Field.txt", "w");
	for (int y = 0; y < FIELD_HEIGHT; y++)
	{
		for (int x = 0; x < FIELD_WIDTH; x++)
		{
			if (bField[y * FIELD_WIDTH + x]) fputc('#', fpTable);
			else                             fputc(' ', fpTable);
		}
		fputc('\n', fpTable);
	}
	fclose(fpTable);
}
int main(void)
{
	newgame:
	score = 0;
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0,
												NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hConsole, &csbi);
		screenWidth = csbi.dwSize.X;
		screenHeight = csbi.dwSize.Y;
	}

	SetConsoleTitleA(TITLE);
	if (!SetConsoleActiveScreenBuffer(hConsole)) return EXIT_FAILURE;
	DWORD dwCharsWritten;

	uchar* screen = (uchar*) malloc(screenWidth * screenHeight);
	for (int i = 0; i < screenWidth * screenHeight; i++) // Clearing the buffer
		screen[i] = ' ';

	startingPosition.X = (FIELD_WIDTH / 2) + xFieldOffset;
	startingPosition.Y = yFieldOffset;

	for (int y = 0; y < FIELD_HEIGHT; y++)
			for (int x = 0; x < FIELD_WIDTH; x++)
				bField[y * FIELD_WIDTH + x] = (x == 0 || x == FIELD_WIDTH - 1 || y == FIELD_HEIGHT - 1) ? FILLED : EMPTY;

	CreateNewTetromino(); // We'll have to start with something
	
	bool keepRunning = true;
	while (keepRunning)
	{
		// Timing
		Sleep(30);

		// +++++++++++ Input +++++++++++
		if (GetKeyState(VK_SHIFT) & 0x8000)
		{
			FILE* testerfp = fopen("test.txt", "a+");
			if (CanMoveDown())
				fprintf(testerfp, "YES ");
			else fprintf(testerfp, "NO ");
			fclose(testerfp);
			dbgPrintField();
		}
		if (GetKeyState(' ') & 0x8000) Emplace();

		// --- Arrow Keys (Movement) ---
		if (curTetromino.pos.X > (xFieldOffset+1 - curTetromino.cellOffsetFromL) && GetKeyState(VK_LEFT) & 0x8000)
			curTetromino.pos.X--;
		else if (curTetromino.pos.X < (FIELD_WIDTH-1 + curTetromino.cellOffsetFromR) && GetKeyState(VK_RIGHT) & 0x8000)
			curTetromino.pos.X++;

		if (curTetromino.pos.Y < (FIELD_HEIGHT-1 + curTetromino.cellOffsetFromBottom) && GetKeyState(VK_DOWN) & 0x8000)
			curTetromino.pos.Y++;

		if (GetKeyState('A') & 0x8000)
			Rotate();

		// --- Exit ---
		if (GetKeyState(0x1B) & 0x8000) // ESC Key
			break;

		// +++++++++++ Game Logic ++++++++++
		if (curTetromino.pos.Y + 4 - curTetromino.cellOffsetFromBottom == FIELD_HEIGHT + yFieldOffset - 1) Emplace();

		//curTetromino.pos.Y++;
		// +++++++++++ Rendering +++++++++++

		// --- the field ---
		for (int y = 0; y < FIELD_HEIGHT; y++)
			for (int x = 0; x < FIELD_WIDTH; x++)
				// If the pixel is filled, put a #
				screen[(y + yFieldOffset) * screenWidth + (x + xFieldOffset)] = (bField[y * FIELD_WIDTH + x]) ? '#' : ' ';

		// --- the current tetromino ---
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
			{
				if (curTetromino.sprite[i * 4 + j] == 'X')
					screen[(curTetromino.pos.Y + i) * screenWidth + curTetromino.pos.X + j] = curTetromino.sprite[i * 4 + j];
			}



		// --- Display Buffer ---
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
	curTetromino.pos = startingPosition;
	curTetromino.rotation = 0;

	srand(time(NULL));
	int random = rand() % 7;
	
	switch (random)
	{
		case 0:
			curTetromino.type = typeI;
			strcpy(curTetromino.sprite, spriteI);
			strcpy(curTetromino.originalSprite, spriteI);
			break;
		case 1:
			curTetromino.type = typeleftL;
			strcpy(curTetromino.sprite, spriteleftL);
			strcpy(curTetromino.originalSprite, spriteleftL);
			break;
		case 2:
			curTetromino.type = typerightL;
			strcpy(curTetromino.sprite, spriterightL);
			strcpy(curTetromino.originalSprite, spriterightL);
			break;
		case 3:
			curTetromino.type = typeblock;
			strcpy(curTetromino.sprite, spriteblock);
			strcpy(curTetromino.originalSprite, spriteblock);
			break;
		case 4:
			curTetromino.type = typeleftZ;
			strcpy(curTetromino.sprite, spriteleftZ);
			strcpy(curTetromino.originalSprite, spriteleftZ);
			break;
		case 5:
			curTetromino.type = typerightZ;
			strcpy(curTetromino.sprite, spriterightZ);
			strcpy(curTetromino.originalSprite, spriterightZ);
			break;
		case 6:
			curTetromino.type = typeT;
			strcpy(curTetromino.sprite, spriteT);
			strcpy(curTetromino.originalSprite, spriteT);
			break;
	}

	// IMPORTANT:
	// Ordering is important! By going with an x outer and y inner loop, we are checking column by column in the 4x4 matrix instead of row by row

	// Left
	for (int x = 0; x < 4; x++)
		for (int y = 0; y < 4; y++)
			if (curTetromino.sprite[y * 4 + x] == 'X')
			{
				curTetromino.cellOffsetFromL = x;
				goto offsetLExit;
			}
	offsetLExit:

	// Right
	for (int x = 3; x >= 0; x--)
		for (int y = 3; y >= 0; y--)
			if (curTetromino.sprite[y * 4 + x] == 'X')
			{
				curTetromino.cellOffsetFromR = 3 - x;
				goto offsetRExit;
			}
	offsetRExit:

	// Bottom
	for (int y = 3; y >= 0; y--)
		for (int x = 3; x >= 0; x--)
			if (curTetromino.sprite[y * 4 + x] == 'X')
			{
				curTetromino.cellOffsetFromBottom = 3 - y;
				goto offsetBExit;
			}
	offsetBExit:
}

void Rotate()
{
	curTetromino.rotation++;

	switch (curTetromino.rotation % 4)
	{
		case 0: // 0 deg
			for (int y = 0; y < 4; y++)
				for (int x = 0; x < 4; x++)
					curTetromino.sprite[y * 4 + x] = curTetromino.originalSprite[y * 4 + x];
			break;
			
		case 1: // 90 deg
			for (int y = 0; y < 4; y++)
				for (int x = 0; x < 4; x++)
					curTetromino.sprite[y * 4 + x] = curTetromino.originalSprite[12 + y - (x*4)];
			break;

		case 2: // 180 deg
			for (int y = 0; y < 4; y++)
				for (int x = 0; x < 4; x++)
					curTetromino.sprite[y * 4 + x] = curTetromino.originalSprite[15 - (y*4) - x];
			break;
			
		case 3: // 270 deg
			for (int y = 0; y < 4; y++)
				for (int x = 0; x < 4; x++)
					curTetromino.sprite[y * 4 + x] = curTetromino.originalSprite[3 - y + (x*4)];
			break;
	}

	// Recalculate Colissions
	// Left
	for (int x = 0; x < 4; x++)
		for (int y = 0; y < 4; y++)
			if (curTetromino.sprite[y * 4 + x] == 'X')
			{
				curTetromino.cellOffsetFromL = x;
				goto offsetLExit;
			}
	offsetLExit:

	// Right
	for (int x = 3; x >= 0; x--)
		for (int y = 3; y >= 0; y--)
			if (curTetromino.sprite[y * 4 + x] == 'X')
			{
				curTetromino.cellOffsetFromR = 3 - x;
				goto offsetRExit;
			}
	offsetRExit:

	// Bottom
	for (int y = 3; y >= 0; y--)
		for (int x = 3; x >= 0; x--)
			if (curTetromino.sprite[y * 4 + x] == 'X')
			{
				curTetromino.cellOffsetFromBottom = 3 - y;
				goto offsetBExit;
			}
	offsetBExit:

}

void Emplace()
{
	for (int y = 0; y < 4; y++)
		for (int x = 0; x < 4; x++)
			if (curTetromino.sprite[y * 4 + x] == 'X')
				bField[(curTetromino.pos.Y + (y-4)) * FIELD_WIDTH + curTetromino.pos.X + (x-4)] = FILLED;
	
	CreateNewTetromino();
}

bool CanMoveDown()
{
	bool lastRowFilled[4];
	for (int i = 0; i < 4; i++)
	{
		lastRowFilled[i] = (curTetromino.sprite[(3 - curTetromino.cellOffsetFromBottom) * 4 + i] == 'X');
		if (lastRowFilled[i] && bField[(curTetromino.pos.Y - yFieldOffset + 3 - curTetromino.cellOffsetFromBottom + 1) * FIELD_WIDTH + curTetromino.pos.X - xFieldOffset + i])
			return false;
	}
	// if (bField[(curTetromino.pos.Y - yFieldOffset + 3 + 1) * FIELD_WIDTH + curTetromino.pos.X - xFieldOffset])
	// 		return false;

    return true;
}