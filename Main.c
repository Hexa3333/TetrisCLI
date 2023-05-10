#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/utime.h>

#include <Windows.h>

LPCSTR TITLE = "TetrisCLI";

#define FIELD_WIDTH 12
#define FIELD_HEIGHT 18

int screenWidth, screenHeight;
int score, scorePerLine;

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
void CheckLinesFilled();
bool CanRotate();
bool CanMoveLeft();
bool CanMoveRight();
bool CanMoveDown();
void GameOver();

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

	scorePerLine = 10;
	int gameSpeed = 20;
	int gameSpeedCounter = 0;
	while (true)
	{
		// Timing
		Sleep(50);
		gameSpeedCounter++;
		bool bforceDown = (gameSpeed == gameSpeedCounter);
		if (bforceDown) gameSpeedCounter = 0;
		
		// +++++++++++ Input +++++++++++
		if (GetKeyState(' ') & 0x8000) Emplace();

		// --- Arrow Keys (Movement) ---
		if (GetKeyState(VK_LEFT) & 0x8000 && CanMoveLeft())
			curTetromino.pos.X--;

		else if (GetKeyState(VK_RIGHT) & 0x8000 && CanMoveRight())
			curTetromino.pos.X++;

		if (GetKeyState(VK_DOWN) & 0x8000 && CanMoveDown())
			curTetromino.pos.Y++;

		if (GetKeyState(VK_UP) & 0x8000 && CanRotate())
			Rotate();

		// --- Exit ---
		if (GetKeyState(0x1B) & 0x8000) // ESC Key
			break;

		// +++++++++++ Game Logic ++++++++++
		if (bforceDown)
		{
			if (CanMoveDown()) curTetromino.pos.Y++;
			else Emplace();
		}

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

		// --- the score ---
		sprintf(&screen[5 * screenWidth + FIELD_WIDTH + xFieldOffset + 10], "%d", score);


		// --- Display Buffer ---
		WriteConsoleOutputCharacterA(hConsole, screen, screenHeight * screenWidth, (COORD){0,0}, &dwCharsWritten);
	}

	CloseHandle(hConsole);
	free(screen);

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
				bField[(curTetromino.pos.Y + (y-yFieldOffset)) * FIELD_WIDTH + curTetromino.pos.X + (x-xFieldOffset)] = FILLED;

	// If you reach the top, you're dead
	for (int x = 1; x < FIELD_WIDTH-1; x++)
		if (bField[1 * FIELD_WIDTH + x]) GameOver();


	CheckLinesFilled();
	CreateNewTetromino();
}

void CheckLinesFilled()
{
	for (int curLineIndex = 0; curLineIndex < FIELD_HEIGHT-1; curLineIndex++)
	{
		for (int x = 0; x < FIELD_WIDTH; x++)
		{
			bool bBlock = bField[curLineIndex * FIELD_WIDTH + x];
			if (bBlock == EMPTY) goto loopY;
		}
		score += scorePerLine;

		// Clear the line
		for (int x = 1; x < FIELD_WIDTH-1; x++)
			bField[curLineIndex * FIELD_WIDTH + x] = EMPTY;

		// Shift Everything Down
		for (int y = curLineIndex; y > 0; y--)
		{
			for (int x = 0; x < FIELD_WIDTH; x++)
				bField[y * FIELD_WIDTH + x] = bField[(y-1) * FIELD_WIDTH + x];
		}
		loopY:
	}
}

bool CanRotate()
{
	char pseudoNextSprite[17];
	switch ((curTetromino.rotation+1) % 4)
	{
		case 0: // 0 deg
			for (int y = 0; y < 4; y++)
				for (int x = 0; x < 4; x++)
					pseudoNextSprite[y * 4 + x] = curTetromino.originalSprite[y * 4 + x];
			break;
			
		case 1: // 90 deg
			for (int y = 0; y < 4; y++)
				for (int x = 0; x < 4; x++)
					pseudoNextSprite[y * 4 + x] = curTetromino.originalSprite[12 + y - (x*4)];
			break;

		case 2: // 180 deg
			for (int y = 0; y < 4; y++)
				for (int x = 0; x < 4; x++)
					pseudoNextSprite[y * 4 + x] = curTetromino.originalSprite[15 - (y*4) - x];
			break;
			
		case 3: // 270 deg
			for (int y = 0; y < 4; y++)
				for (int x = 0; x < 4; x++)
					pseudoNextSprite[y * 4 + x] = curTetromino.originalSprite[3 - y + (x*4)];
			break;
	} 

	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if (pseudoNextSprite[y*4+x] == 'X' &&
				bField[(curTetromino.pos.Y - yFieldOffset + y) * FIELD_WIDTH + curTetromino.pos.X - xFieldOffset + x])
				return false;
		}
	}

	return true;
}

bool CanMoveLeft()
{
	for (int i = 0; i < 4; i++)
	{
		bool firstColumnFilled = (curTetromino.sprite[i * 4 + curTetromino.cellOffsetFromL] == 'X');
		if (firstColumnFilled && bField[curTetromino.pos.X - xFieldOffset + curTetromino.cellOffsetFromL - 1 + ((curTetromino.pos.Y - yFieldOffset + i) * FIELD_WIDTH)])
			return false;
	}

	return true;
}

bool CanMoveRight() // IT might be a little buggy, fix this one
{
	for (int i = 0; i < 4; i++)
	{
		bool lastColumnFilled = (curTetromino.sprite[i * 4 - 1 - curTetromino.cellOffsetFromR] == 'X');
		if (lastColumnFilled && bField[curTetromino.pos.X - xFieldOffset + (3 - curTetromino.cellOffsetFromR) + 1 + ((curTetromino.pos.Y - yFieldOffset + i) * FIELD_WIDTH)])
			return false;
	}

	return true;
}

bool CanMoveDown()
{
	for (int i = 0; i < 4; i++)
	{
		bool lastRowFilled = (curTetromino.sprite[(3 - curTetromino.cellOffsetFromBottom) * 4 + i] == 'X');
		if (lastRowFilled && bField[(curTetromino.pos.Y - yFieldOffset + 3 - curTetromino.cellOffsetFromBottom + 1) * FIELD_WIDTH + curTetromino.pos.X - xFieldOffset + i])
			return false;
	}
	// if (bField[(curTetromino.pos.Y - yFieldOffset + 3 + 1) * FIELD_WIDTH + curTetromino.pos.X - xFieldOffset])
	// 		return false;

    return true;
}

void GameOver()
{
	/*	Attach a new console screen buffer,
	*	present the game over screen.
	*	Wanna play again ?
	*/
}