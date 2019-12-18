#include <iostream>
#include <stdio.h>      /* printf, scanf, NULL */
#include <stdlib.h>     /* malloc, free, rand */
#include <ctime>		/* srand(time(NULL)); */
#include <chrono>		/* calculo de tiempos */
#include <omp.h>		/* paralelizacion con OMP */
//#include <math.h>
using namespace std;

/**** Variables auxiliares ****/
#define NUM_REGLAS 6
enum Status { DEAD, ALIVE };
enum Regla { _0, _1, _2, _3, _4, _5 };
/******************************/

/**************** METODOS PARA EL MANEJO DE MATRICES ****************/

/**** TIPS matriz 3D aplanada en matriz 1D: *****
* Donde hemos sacado el GameOfLife --> https://www.geeksforgeeks.org/program-for-conways-game-of-life/
* // https://www.techiedelight.com/dynamic-memory-allocation-in-c-for-2d-3d-array/#3D
* M = n�Columnas = n�Filas;
* NUM_REGLAS = profundidad
*
* k = reglaObjetivo = profundidadObjetiva
* i = filaObjetivo;
* j = columnaObjetivo;
*
* grid[k][i][j] -> grid[ (i * M * NUM_REGLAS) + (j * NUM_REGLAS) + k]
*************************************************/

/* Inicializa las matrices para cada regla */
void initMatrix(short* grid, const int M) {
	srand(time(NULL));
	short value;
	long int pos;
	long  numElem = M * M * NUM_REGLAS;
	#pragma omp parallel for schedule(auto) private (pos)
	for (pos = 0; pos < numElem; pos+= NUM_REGLAS){
		value = rand() % 2;
		grid[pos] = value;
		grid[pos + Regla::_1] = value;
		grid[pos + Regla::_2] = value;
		grid[pos + Regla::_3] = value;
		grid[pos + Regla::_4] = value;
		grid[pos + Regla::_5] = value;
	}
	#pragma omp barrier
	//cout << "\nSoy el hilo " << 
	/*
	for (int i = 0; i < M; i++){
		for (int j = 0; j < M; j++){ // Hacemos una copia de la matriz inicial por cada regla que tengamos
			value = rand() % 2;
			pos = (size_t)((i * M * NUM_REGLAS) + (j * NUM_REGLAS)); // [i][j]

			for (int k = 0; k < NUM_REGLAS; k++) {
				grid[pos + k] = value; // Regla K, Posici�n [i][j] ... 
			}
		}
	}*/
}

// Imprime la matriz de una regla
void printRegla(const short* grid, const int M, const int Regla) {
	printf("\n\tregla[%d]\n", Regla);

	for (int i = 0; i < M; i++){
		cout << "\t";
		for (int j = 0; j < M; j++){ // Hacemos una copia de la matriz inicial por cada regla que tengamos
			size_t pos = (size_t)((i * M * NUM_REGLAS) + (j * NUM_REGLAS) + Regla); // [i][j]
			cout << (grid[pos] == Status::DEAD ? '.' : '*');
		}
		cout << "\n";
	}
}

/* Imprime las matrices de cada regla */
void printMatrix(const short* grid, const int M) {
	for (int k = 0; k < NUM_REGLAS; k++) {
		printRegla(grid, M, k);
	}
}

/* Devuelve el módulo con su valor absoluto */
unsigned int mod(long a, long b) { return ((a%b) + b) % b; }

/* Obtiene los vecinos de una celda para todas las reglas */
void getAliveNeighbours(const short* grid, int aliveNeighbours[], const int l, const int m, int M, size_t pos){
	int i, j;
	for (i = -1; i <= 1; i++){
		for (j = -1; j <= 1; j++){
			/* Si se hace de esta manera falla dependiendo del compilador... */
			int aux1 = ((l + i) % M), //mod((l + i),M),
				aux2 = ((m + j) % M); //mod((m + j),M);
			size_t posNeig = (size_t)((aux1 * M * NUM_REGLAS) + (aux2 * NUM_REGLAS)); // [i][j]
			aliveNeighbours[Regla::_0] += grid[posNeig];
			aliveNeighbours[Regla::_1] += grid[posNeig + Regla::_1];
			aliveNeighbours[Regla::_2] += grid[posNeig + Regla::_2];
			aliveNeighbours[Regla::_3] += grid[posNeig + Regla::_3];
			aliveNeighbours[Regla::_4] += grid[posNeig + Regla::_4];
			aliveNeighbours[Regla::_5] += grid[posNeig + Regla::_5];
		}
	}
	// The cell needs to be subtracted from its neighbours as it was counted before
	aliveNeighbours[Regla::_0] -= grid[pos];
	aliveNeighbours[Regla::_1] -= grid[pos + Regla::_1];
	aliveNeighbours[Regla::_2] -= grid[pos + Regla::_2];
	aliveNeighbours[Regla::_3] -= grid[pos + Regla::_3];
	aliveNeighbours[Regla::_4] -= grid[pos + Regla::_4];
	aliveNeighbours[Regla::_5] -= grid[pos + Regla::_5];
}
/********************************************************************/

void nextGeneration(short* grid, short *future, const int M, size_t totalBytes, size_t aliveCells[]);

void swap_pointer_address(short **a, short **b) {
	short *swap;
	swap = *a;
	*a = *b;
	*b = swap;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Parametros incorrectos Introduce el tama�o de la matriz y el numero de itereaciones.\n");
		exit(1);
	}
	// Indica el tama�o de la matriz M * M
	int M = atoi(argv[1]);
	// Indica el n�mero de veces que se aplicaran las reglas
	int iteraciones = atoi(argv[2]);

	size_t totalCells = (size_t)((M * M * NUM_REGLAS));
	size_t totalBytes = totalCells* sizeof(short);

	short* grid = (short*)malloc(totalBytes);
	short *future = (short*)malloc(totalBytes);


	if (grid) {
		printf("\nSe van a reservar %d Bytes utilizando tipo shorts en cada grid (En 2 grid hacen %d Bytes) con un total de %d elementos", totalBytes, 2 * totalBytes, totalCells);
		double t1 = omp_get_wtime();
		initMatrix(grid, M);


		size_t numRoundsWithAliveCells[NUM_REGLAS] = { 0, 0, 0, 0, 0, 0 };
		short ruleWithMostSurvivors = 0;
		
		//printMatrix(grid, M);

			for (int i = 0; i < iteraciones; i++) {
				size_t aliveCells[NUM_REGLAS] = { 0, 0, 0, 0, 0, 0 };
				printf("\nIteracion: %d", i);
				nextGeneration(grid, future, M, totalBytes, aliveCells);
				//copyMatrix(grid, future, M);
				swap(grid, future);


				for (int j = 0; j < NUM_REGLAS; j++) {

					numRoundsWithAliveCells[j] += ((aliveCells[j] == 0) ? 0 : 1);
					if (aliveCells[j] > aliveCells[ruleWithMostSurvivors])
						ruleWithMostSurvivors = j;
				}

				printf("\n\tLa regla con mas supervivientes es la %d con %d celulas vivas.\n", ruleWithMostSurvivors, aliveCells[ruleWithMostSurvivors]);
				//printMatrix(grid, M);
			}
		printf("\nLas reglas han sobrevivido");
		for (int i = 0; i < NUM_REGLAS; i++)
			printf(" [R%d = %drondas]", i, numRoundsWithAliveCells[i]);
		printf(" rondas respectivamente.\n");

		double t2 = omp_get_wtime();
		double tFinal = t2 - t1;

		free(grid); free(future);
		future = NULL; grid = NULL;

		printf("\nSe han analizado un total de %d elementos por iteracion.", totalCells);
		printf("\nFin de ejecuci�n en %.5fs.\n ", tFinal);
		
	}
	else {
		printf("\nError en el malloc del main, no hay espacio suficiente para %d Bytes", totalBytes);
		//getchar();
		//exit(1);
	}
	//getchar();
	return 0;
}

/**** Predefinici�n de las Posibles reglas a aplicar ****/
short reglaConway(const short &cell, const int aliveNeighbours);
short muerteLocomotorasNaves(const short &cell, const int aliveNeighbours);
short manchas(const short &cell, const int aliveNeighbours);
short vida34(const short &cell, const int aliveNeighbours);
short alfombras(const short &cell, const int aliveNeighbours);
short dimantesCatastrofes(const short &cell, const int aliveNeighbours);
/********************************************************/

// Funci�n para "avanzar" el estado de vida de las colonias
void nextGeneration(short *grid, short *future, const int M, size_t totalBytes, size_t aliveCells[])
{

	if (future) {
		size_t pos;
		// Loop through every cell
		int l,m;
		#pragma omp parallel for schedule(auto) collapse(2) private(l,m,pos)
		
		for (  l = 0; l < M; l++)
		{
			for (  m = 0; m < M; m++)
			{
				pos = (l * M * NUM_REGLAS) + (m * NUM_REGLAS); // [i][j]

				// finding num Of Neighbours that are alive
				int aliveNeighbours[NUM_REGLAS] = { 0, 0, 0, 0, 0, 0 };
				getAliveNeighbours(grid, aliveNeighbours, l, m, M, pos);

				size_t r0 = pos + Regla::_0,
					r1 = pos + Regla::_1, r2 = pos + Regla::_2, r3 = pos + Regla::_3, r4 = pos + Regla::_4, r5 = pos + Regla::_5;

				
					//printf("Hilo %d \n",omp_get_thread_num());
					future[r0] = reglaConway(grid[r0], aliveNeighbours[Regla::_0]);
					future[r1] = muerteLocomotorasNaves(grid[r1], aliveNeighbours[Regla::_1]);
					future[r2] = manchas(grid[r2], aliveNeighbours[Regla::_2]);
					future[r3] = vida34(grid[r3], aliveNeighbours[Regla::_3]);
					future[r4] = alfombras(grid[r4], aliveNeighbours[Regla::_4]);
					future[r5] = dimantesCatastrofes(grid[r5], aliveNeighbours[Regla::_5]);
				
				for (int i = 0; i < NUM_REGLAS; i++){
					aliveCells[i] += future[pos + i];
				}

			}
		}

		#pragma omp barrier
	}
	else {
		printf("\nError en el malloc de nextGeneration, no hay espacio suficiente para %d Bytes", totalBytes);
		exit(1);
	}
}

// 23/3 (complejo) "Juego de la Vida de Conway"
short reglaConway(const short &cell, const int aliveNeighbours) {
	short  newStatus;
	// Cell is lonely and dies
	if ((cell == 1) && (aliveNeighbours < 2))
		newStatus = Status::DEAD;

	// Cell dies due to over population
	else if ((cell == 1) && (aliveNeighbours > 3))
		newStatus = Status::DEAD;

	// A new cell is born
	else if ((cell == 0) && (aliveNeighbours == 3))
		newStatus = Status::ALIVE;

	// Remains the same
	else
		newStatus = cell;
	return newStatus;
}

// 245/368 (estable) muerte, locomotoras y naves
short muerteLocomotorasNaves(const short &cell, const int aliveNeighbours) {
	short  newStatus;
	// Si la celula esta viva
	if ((cell == 1)) {
		// Sobrevive si tiene 2, 4, 5 vecinos vivos
		if (aliveNeighbours == 2 || aliveNeighbours == 4 || aliveNeighbours == 5)
			newStatus = cell;
		else // Sino, muere
			newStatus = Status::DEAD;
	} // Si la celula esta muerta
	else {
		// Nace si tiene 3, 6, 8 vecinos vivos
		if (aliveNeighbours == 3 || aliveNeighbours == 6 || aliveNeighbours == 8)
			newStatus = Status::ALIVE;
		else // Sino, continua muerta
			newStatus = cell;
	}
	return newStatus;
}

// 235678/3678 (estable) mancha de tinta que se seca r�pidamente
short manchas(const short &cell, const int aliveNeighbours){
	short newStatus;

	// Si la celula esta viva
	if ((cell == 1)) {
		// Sobrevive si tiene 2, 3, 5, 6, 7, 8 vecinos vivos
		if (aliveNeighbours == 2 || aliveNeighbours == 3 || aliveNeighbours >= 5)
			newStatus = cell;
		else // Sino, muere
			newStatus = Status::DEAD;
	} // Si la celula esta muerta
	else {
		// Nace si tiene 3, 6, 7, 8 vecinos vivos
		if (aliveNeighbours == 3 || aliveNeighbours >= 6)
			newStatus = Status::ALIVE;
		else // Sino, continua muerta
			newStatus = cell;
	}
	return newStatus;

}

// 34/34 (crece) "Vida 34"
short vida34(const short &cell, const int aliveNeighbours){
	short newStatus;

	// Si la celula esta viva
	if ((cell == 1)) {
		// Sobrevive si tiene 3, 4 vecinos vivos
		if (aliveNeighbours == 3 || aliveNeighbours == 4)
			newStatus = cell;
		else // Sino, muere
			newStatus = Status::DEAD;
	} // Si la celula esta muerta
	else {
		// Nace si tiene 3, 4 vecinos vivos
		if (aliveNeighbours == 3 || aliveNeighbours == 4)
			newStatus = Status::ALIVE;
		else // Sino, continua muerta
			newStatus = cell;
	}
	return newStatus;

}

// 4/2 (crece) generador de patrones de alfombras
short alfombras(const short &cell, const int aliveNeighbours){
	short newStatus;

	// Si la celula esta viva
	if ((cell == 1)) {
		// Sobrevive si tiene 4 vecinos vivos
		if (aliveNeighbours == 4)
			newStatus = cell;
		else // Sino, muere
			newStatus = Status::DEAD;
	} // Si la celula esta muerta
	else {
		// Nace si tiene 2 vecinos vivos
		if (aliveNeighbours == 2)
			newStatus = Status::ALIVE;
		else // Sino, continua muerta
			newStatus = cell;
	}
	return newStatus;

}

// 5678/35678 (ca�tico) diamantes, cat�strofes
short dimantesCatastrofes(const short &cell, const int aliveNeighbours){
	short newStatus;

	// Si la celula esta viva
	if ((cell == 1)) {
		// Sobrevive si tiene 5, 6, 7, 8 vecinos vivos
		if (aliveNeighbours >= 5)
			newStatus = cell;
		else // Sino, muere
			newStatus = Status::DEAD;
	} // Si la celula esta muerta
	else {
		// Nace si tiene 3, 5, 6, 7, 8 vecinos vivos
		if (aliveNeighbours == 3 || aliveNeighbours >= 5)
			newStatus = Status::ALIVE;
		else // Sino, continua muerta
			newStatus = cell;
	}
	return newStatus;

}
