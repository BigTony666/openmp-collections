/**********************************
 * DESCRIPTION: A program to do matrix inversion with the help of LU Decomposition
 * by Doolittle algorithm using pthreads.
 *
 * Author: Kejie Zhang
 * LAST UPDATED: 03/05/2019
 *
 * USAGE:
 *   COMPILE: g++ matrix_inverse.cpp -pthread -std=c++11 -O3 -o matrix_inverse
 *   RUN: ./matrix_inverse
 *
 * USEFUL REFERENCE:
 *    -> Doolittle: https://www.geeksforgeeks.org/doolittle-algorithm-lu-decomposition/
 *    -> Forward and Back Substitution: https://www.gaussianwaves.com/2013/05/solving-a-triangular-matrix-using-forward-backward-substitution/
**********************************/
#include <iostream>
#include <random>
#include <stdio.h>
#include <iomanip>
#include <time.h>
#include <stdlib.h>
#include <utility>
#include <vector>
#include <pthread.h>

using namespace std;

const int matrix_size = 1000;
const int num_threads = 8;

vector <vector<int>>
        A(matrix_size, vector<int>(matrix_size));
vector <vector<double>>
        U(matrix_size, vector<double>(matrix_size, 0)),
        L(matrix_size, vector<double>(matrix_size, 0)),
        C(matrix_size, vector<double>(matrix_size)),
        Inv(matrix_size, vector<double>(matrix_size, 0)),
        Identity(matrix_size, vector<double>(matrix_size));
vector<int>
        Prow(matrix_size),
        Pcol(matrix_size);

struct thread_data {
    int lo;
    int hi;
};

class Printer {
public:
    void print_matrix(vector <vector<int>> M, string const &message) {
        cout << endl;
        cout << message << endl;
        int i, j;
        for (i = 0; i < matrix_size; i++) {
            for (j = 0; j < matrix_size; j++)
                cout << setprecision(2) << M[i][j] << "\t";
            cout << endl;
        }
        cout << endl;
    }

    void print_matrix(vector <vector<double>> M, string const &message) {
        cout << endl;
        cout << message << endl;
        int i, j;
        for (i = 0; i < matrix_size; i++) {
            for (j = 0; j < matrix_size; j++)
                cout << setprecision(2) << M[i][j] << "\t";
            cout << endl;
        }
        cout << endl;
    }
};

class Clock {
private:
    timespec begin;
    timespec end;

public:
    void start() {
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &begin);
    }

    void stop() {
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
    }

    double getInterval() {
        if (&begin != NULL && &end != NULL)
            return ((end.tv_sec - begin.tv_sec) * 1e3 + (end.tv_nsec - begin.tv_nsec) * 1e-6) * 1e-3;
        else
            return -1;
    }
};

/**
 * LU Decomposition with Doolittle Algorithm
 */

void LUDecomposition(vector <vector<int>> &A, vector <vector<double>> &L, vector <vector<double>> &U) {
    for (int i = 0; i < matrix_size; i++) {

        // Upper Triangular
        for (int k = i; k < matrix_size; k++) {
            double sum = 0;
            for (int j = 0; j < i; j++)
                sum += (L[i][j] * U[j][k]);
            U[i][k] = A[i][k] - sum;
        }

        // Lower Triangular
        for (int k = i; k < matrix_size; k++) {
            if (i == k)
                L[i][i] = 1;
            else {
                double sum = 0;
                for (int j = 0; j < i; j++)
                    sum += (L[k][j] * U[j][i]);
                L[k][i] = (A[k][i] - sum) / U[i][i];
            }
        }
    }
}

/**
 * Forward Substitution
 * @param col
 * @param L
 * @param U
 */
void triangle_forward_sub(int col, vector <vector<double>> &L, vector <vector<double>> &U) {
    int _col = col;
    int i, j;
    if (Pcol[_col] != 0)
        C[_col][0] = 0;
    else C[_col][0] = 1 / L[0][0];
    for (j = 1; j < matrix_size; j++) {
        double sum = 0;
        for (i = 0; i < j; i++)
            sum += L[j][i] * C[_col][i];
        if (Pcol[_col] != j)
            C[_col][j] = -sum;
        else C[_col][j] = 1 - sum;
    }
}

/**
 * Backward substitution
 * @param col
 * @param U
 */
void triangle_back_sub(int col, vector <vector<double>> &U) {
    int _col = col;
    int i, j;
    Inv[matrix_size - 1][_col] = C[_col][matrix_size - 1] / U[matrix_size - 1][matrix_size - 1];
    for (i = matrix_size - 2; i > -1; i--) {
        double sum = 0;
        for (j = i + 1; j < matrix_size; j++)
            sum += U[i][j] * Inv[j][_col];
        Inv[i][_col] = (C[_col][i] - sum) / U[i][i];
    }
}

/**
 * Helper function to populate vector randomly or uniformly in a range.
 * @param A
 * @param start
 * @param end
 * @param flag
 */
void populateVectorRandom(vector <vector<int>> &A, double start, double end, int flag) {
    int i,
            j;
    /** Generate random sequence*/
    random_device rd;
    mt19937 engine(rd());
    uniform_int_distribution<> u(start, end);
    bernoulli_distribution b;

    /** Populate random numbers into vectors uniformly */
    if (flag == 1) {
        for (i = 0; i < A.size(); i++) {
            for (j = 0; j < A[0].size(); j++) {
                A[i][j] = u(engine);
            }
        }
    }

    /** Generate Sparse Matrix */
    if (flag == 2) {
        for (i = 0; i < A.size(); i++) {
            for (j = 0; j < A[0].size(); j++) {
                if(b(engine) == 1) {
                    A[i][j] = u(engine);
                } else {
                    A[i][j] = 0;
                }
            }
        }
    }
}

void *triangle_inverse(void *param) {
    int lo = ((thread_data *) param)->lo;
    int hi = ((thread_data *) param)->hi;
    int i;
    for (i = lo; i < hi; i++) {
        triangle_forward_sub(i, L, U);
        triangle_back_sub(i, U);
    }
    pthread_exit(NULL);
    return 0;
}

/**
 * Check the correctness of matrix inversion.
 * Use kij format for a cache-friendly matrix multiplication
 * @param A
 * @param Inv
 * @param Identity
 * @return bool
 */
int check(vector <vector<int>> &A, vector <vector<double>> &Inv, vector <vector<double>> &Identity) {
    int i, j, k, r, size;
    size = A.size();
    for (k = 0; k < size; k++) {
        for (i = 0; i < size; i++) {
            r = A[i][k];
            for (j = 0; j < size; j++) {
                Identity[i][j] += r * Inv[k][j];
            }
        }
    }

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            Identity[i][j] = lround(Identity[i][j]);
        }
    }

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            if ((i == j && Identity[i][j] != 1) || (i != j && Identity[i][j] != 0)) {
                return 1;
            }
        }
    }
    return 0;
}

/**
 * Main function
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {
    Printer printer;
    Clock clock;
    int i,
            partition,
            rc;
    double time;

    partition = matrix_size / num_threads;
    pthread_t workers[num_threads];
    thread_data thread_data_array[num_threads];

    /** Initialization */
    cout << "Initializing...";
    thread_data_array[num_threads - 1].lo = partition * (num_threads - 1);
    thread_data_array[num_threads - 1].hi = matrix_size;
    for (i = 0; i < num_threads - 1; i++) {
        thread_data_array[i].hi = partition * (i + 1);
        thread_data_array[i].lo = partition * i;
    }
    populateVectorRandom(A, 0, 100, 2);
    cout << "[DONE]" << endl;

    cout << "Running the LU Decomposition...";
    clock.start();
    for (i = 0; i < matrix_size; i++)
        Prow[i] = i;

    LUDecomposition(A, L, U);

    clock.stop();
    time = clock.getInterval();
    cout << "[DONE]" << endl;
    printf("LU Decomposition running time is...[%f]\n", time);

    cout << "Running the Forward and Backward Substitution...";
    clock.start();
    int sum = 0;
    for (i = 0; i < matrix_size; i++) {
        Pcol[Prow[i]] = sum;
        sum++;
    }

    pthread_create(&workers[num_threads - 1], NULL, triangle_inverse, &thread_data_array[num_threads - 1]);
    for (i = 0; i < num_threads - 1; i++)
        pthread_create(&workers[i], NULL, triangle_inverse, &thread_data_array[i]);
    for (i = 0; i < num_threads; i++)
        pthread_join(workers[i], NULL);

    clock.stop();
    time = clock.getInterval();
    cout << "[DONE]" << endl;
    printf("Forward and Backward substitution running time is...[%f]\n", time);

//    printer.print_matrix(A, "Matrix A is:\n");
//    printer.print_matrix(Inv, "Matrix Inv is:\n");
    cout << "Checking the calculation result...";
    rc = check(A, Inv, Identity);
    if (rc == 0) {
        cout << "[CORRECT]" << endl;
    } else {
        cout << "[WRONG]" << endl;
    }
    return 0;
}