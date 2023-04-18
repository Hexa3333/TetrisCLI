CC=gcc
SRC=Main.c
BIN=Tetris.exe

all:
	${CC} ${SRC} -o ${BIN}