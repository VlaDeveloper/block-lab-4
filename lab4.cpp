#include <cstdio>
#include <iostream>
#include <random>
#include "mpi.h"

// размер матрицы
const int N = 500;

// объявляем матрицы размером N
int m1[N][N];
int m2[N][N];
int m3[N][N];

// получаем рандомное число для заполнения матрицы
int GetRandom(int min, int max) {
	return rand() % (max - min + 1) + min;
}

int main(int argc, char *argv[])
{
        setlocale(LC_ALL, "Russian");
        //теги сообщений: мастер посылает 1, рабочие посылают 2
	int size, rank; // количество процессов и номер текущего процесса
	int rowsForA, remainingRows, rows; //дополнительные строки, если количество строк не делится на целое
	int buff = 0;
	MPI_Status status;
	double start, end; // время начала и окончания
	// заполняем нулями итоговую матрицу
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			m3[i][j] = 0;
		}

	}

	MPI_Init(&argc, &argv); // инициализируем параллельную часть программы
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); // узнаем номер текущего процесса
	MPI_Comm_size(MPI_COMM_WORLD, &size); // узнаем общее количество процессов
	MPI_Barrier(MPI_COMM_WORLD); // синхронизация барьера для всех членов группы
	if (rank == 0) { // мастер распределяет строки между процессами
		printf("Мастер\n");
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				m1[i][j] = GetRandom(-100, 100);
			}

		}

		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				m2[i][j] = GetRandom(-100, 100);
			}

		}

		start = MPI_Wtime(); // засекаем начало
		rowsForA = N / (size - 1); //общее количество процессов минус 1 
		remainingRows = N % (size-1); // распределяем оставшиеся строки
		for (int i = 1; i <= size - 1; i++) { //отправка начальный значений
			if (i <= remainingRows) {
				rows = rowsForA + 1;
			}
			else {
				rows = rowsForA;
			}
			MPI_Send(&buff, 1, MPI_INT, i, 1, MPI_COMM_WORLD);//начальный номер строки
			MPI_Send(&rows, 1, MPI_INT, i, 1, MPI_COMM_WORLD);//количество строк
			MPI_Send(&m1[buff][0], N * N, MPI_INT, i, 1, MPI_COMM_WORLD);//отправка строки
			MPI_Send(&m2, N * N, MPI_INT, i, 1, MPI_COMM_WORLD);
			buff += rows;
		}
		for (int i = 1; i <= size - 1; i++) { //получение результата
			MPI_Recv(&buff, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
			MPI_Recv(&rows, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
			MPI_Recv(&(m3[buff][0]), rows*N, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
		}
		end = MPI_Wtime(); // засекаем окончание
		double time = end - start; // получаем время выполнения
		printf("Время выполнения %f с\n", time);
	} else {
	        // все остальные процессы получают данные от мастера
		MPI_Recv(&buff, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
		MPI_Recv(&rows, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
		MPI_Recv(&m1[buff][0], N * N, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
		MPI_Recv(&m2, N * N, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

                // получаем элементы итоговой матрицы умножением элемента первой матрицы на элемент второй
		for (int i = buff; i < buff+rows; i++) {
			for (int k = 0; k < N; k++) {
				for (int j = 0; j < N; j++) {
					m3[i][j] += m1[i][k] * m2[k][j];
				}
			}
		}
		// посылаем данные мастеру
		MPI_Send(&buff, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
		MPI_Send(&rows, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
		MPI_Send(&(m3[buff][0]), rows * N, MPI_INT, 0, 2, MPI_COMM_WORLD);
	}
	// завершение параллельной части программы
	MPI_Finalize();

}
