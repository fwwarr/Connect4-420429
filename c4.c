/*
 * c4.c
 *
 *  Created on: 2010-11-17
 *      Author: fred
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

int printMatrix(int[10][7]);
int play(int[10][7], int, int);
int getMove(int, int);
void* multiCalc(void*);
int calcMove(int[10][7], int, int);
int analyzeMove(int[10][7], int, int, int);
int undo(int[10][7], int);

int NUM_THREADS = 1;
int grid[10][7] =   {
			{0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0}};
int p = 1; //The current player
int best;	// Holder for best move
int bestVal;// Holder for best move value

int main(int argc, char *argv[]){
	printf ("Welcome to Connect4-420429\n");

	char mode = 'c';
	int i;
	if(argc>1){
		mode = argv[1][0];
	}
	switch (mode) {
	case 'f':
		//Not implemented yet
		printf("Playing in file mode. Moves will be read from a file.\n\n");
		break;
	case 'r':
		printf("Playing in random mode. Moves will played randomly.\n\n");
		break;
	case 'a':
		//Not implemented yet
		printf("Playing in AI mode. Moves will chosen according to a winning strategy.\n\n");
		break;
	default:
		printf("Playing in console mode. Moves will be read from the console.\n\n");
		break;
	}


	if (argc>2){
		NUM_THREADS = strtod(argv[2], NULL);
	}


	srand(time(NULL));//Seeding the random number generator

	int w = 0; //The outcome of a move (positive: player wins, 0: OK, -1: invalid move)
	int c = 0; //The current move, i.e. the column the token was dropped in
	int m = 0; //The number of moves played so far

	while(w == 0 && m < 70){

		w = -1;
		while(w == -1){
			c = getMove(p, mode);
			w = play(grid,p,c);
		}
		printMatrix(grid);
		p = p%2 + 1;
		m++;

		// MULTI-PROCESSING SETUP - Calculate best move
		pthread_t threads[NUM_THREADS];
		int thread_args[NUM_THREADS];
	
		int rc, i;
		
		//Create Threads
		for (i=0; i<NUM_THREADS; ++i) {
			thread_args[i] = i;
			rc = pthread_create(&threads[i], NULL, multiCalc, (void *) &thread_args[i]);
			assert(0 == rc);
		}
		//Wait for all threads to complete
		for (i=0; i<NUM_THREADS; ++i) {
			rc = pthread_join(threads[i], NULL);
			assert(0 == rc);
		}



	}
	if(w > 0){
		printf("\nPlayer %d has won!\n", w);
	} else {
		printf("\nGame ended in a tie.\n");
	}


	return 0;
}


int printMatrix(int m[10][7]){

	int i = 0;
	int j = 0;
	putchar('\n');
	for( i = 0; i < 10; i++){
		for( j = 0; j < 7; j++){
			printf(" %d ", m[i][j]);
		}
		putchar('\n');
	}
	return 0;
}

int play(int m[10][7], int p, int c){

	int i = 9;
	for(;i>=0;i--){
		if(m[i][c] == 0){
			m[i][c] = p;
			break;
		}
	}

	//Check if the move is a winning move
	if(i>=0){

		int a = 0;
		int j = 0;
		int k = 0;

		//Vertical
		if(i<7){
			if(m[i][c] + m[i+1][c] + m[i+2][c] + m[i+3][c] == 4 * p){
				return p;
			}
		}
		//Horizontal
		for(;k<7;k++){
			if(m[i][k] == p){
				a++;
				if (a > 3){
					return p;
				}
			} else {
				a = 0;
			}
		}
		//Diagonal NW-SE
		a = 0;
		j = i;
		k = c;
		while(k > 0 && j > 0){
			j--;
			k--;
		}
		while(k < 7 && j < 10){
			if(m[j][k] == p){
				a++;
				if (a > 3){
					return p;
				}
			} else {
				a = 0;
			}
			j++;
			k++;
		}
		//Diagonal SW-NE
		a = 0;
		j = i;
		k = c;
		while(k > 0 && j < 9){
			j++;
			k--;
		}
		while(k < 7 && j >= 0){
			if(m[j][k] == p){
				a++;
				if (a > 3){
					return p;
				}
			} else {
				a = 0;
			}
			j--;
			k++;
		}
		return 0;
	}
	return -1;
}

int getMove(int p, int mode){

	int c;
	switch (mode) {
	case 'r':
		c = rand()%7;
		break;
	default:
		printf("Please enter a move for player %d:", p);
		scanf("%d", &c);
		break;
	}
	return c;
}

void *multiCalc(void *argument) {

	int tid;
	tid = *((int *) argument);

	int b = 0;
	
	b = calcMove(grid, p, tid);

	printf("Processor %d's best result is move %d\n", tid, b);
	
	}

//Calculates the 'best' possible move
int calcMove(int m[10][7], int p, int tid){
	
	printf("\n");
	int a = 0;//value of current move
	int v = 0;//value of best move
	int b = 0;//best move
	int c = 0;//column
	int m2[10][7];
	memcpy(m2,m,sizeof(m2));

	for(;c < 7; c++){
	
		if (c%NUM_THREADS == tid) {
			a = analyzeMove(m2,p,c,0);
			if(c==0) v = a;
			printf("Value of move in column %d: %d\n", c, a);
			if (a > v){
				v = a;
				b = c;
			}
		}
		
	}
	//printf("The best move is %d\n", b);
	return b;
}

int analyzeMove(int m[10][7], int p, int c, int ply){

	int v = 0;//Value of current move

	//Limit search depth
	if(ply > 2){
		return v;
	}
	else {

		int w = play(m,p,c);

		if(w == p){
			v = v - (ply%2*2-1)*7;
		} else if (w > 0){
			v = v + (ply%2*2-1)*7;
		} else if (w == -1){
			return v;//Invalid move
		}

		int i = 0;
		int a = 0;
		ply++;

		for(;i < 7; i++){
			a += analyzeMove(m,p%2+1,i,ply);
		}
		undo(m,c);
		return v*7*7+a;//Extra multiplications to add weight to sooner c4s.
	}
}

int undo(int m[10][7], int c){
	int i = 0;
	for(;i<10;i++){
		if(m[i][c] != 0){
			m[i][c] = 0;
			break;
		}
	}
	return 0;
}









