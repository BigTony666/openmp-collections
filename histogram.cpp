/**********************************
 * DESCRIPTION: A program to calculate histogram using MPI
 *
 * Author: Kejie Zhang
 * LAST UPDATED: 03/25/2019
 *
 * USAGE:
 *   COMPILE: mpicxx -std=c++11 histogram.cpp -o histogram
 *   RUN: mpiexec -n < 4 or 8 > ./histogram
 *
 * USEFUL REFERENCE:
 *    -> MPI: https://computing.llnl.gov/tutorials/mpi/
**********************************/
#include <iostream>
#include <vector>
#include <random>
#include <mpi.h>

#define MIN_RANGE 1
#define MAX_RANGE 1000

using namespace std;

void populateVectorRandom(int data[], int length, int start, int end, int flag);

void setLocalData(
        int data[],
        int local_data[],
        int num_data,
        int local_num_data,
        int rank_id);

void setLocalBins(
        int bins[],
        int local_bins[],
        int local_num_bins);

void printArray(int data[], int length);

void histogram(
        int data[],
        int bins[],
        int local_data[],
        int local_bins[],
        int local_num_data,
        int local_num_bins);

void histogram2(
        int data[],
        int bins[],
        int local_data[],
        int local_bins[],
        int local_num_data,
        int local_num_bins,
        int num_bins,
        int rank_id);

void printResult(int bins[], int length);

int main(int argc, char *argv[]) {
    /** Variables */
    int rank_id,
            num_procs,
            i,
            name_len,
            num_data,
            num_bins,
            local_num_data,
            local_num_bins,
            question;
    double start,
            stop;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_id);

    /** Get the input */
    if (rank_id == 0) {
        cout << "Please choose which question? [1] or [2]?" << endl;
        cin >> question;
        cout << "Please input the number of data:" << endl;
        cin >> num_data;
        cout << "Please input the number of bins(classes):" << endl;
        cin >> num_bins;

        local_num_data = num_data / num_procs;
        num_data = local_num_data * num_procs;

        local_num_bins = num_bins / num_procs;
        num_bins = local_num_bins * num_procs;

        if(question == 1) {
            local_num_bins = num_bins;
        } else if (question == 2) {
            local_num_data = num_data;
            local_num_bins = num_bins;
        } else {
            return -1;
        }
    }
    MPI_Bcast(&num_data, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&num_bins, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&local_num_data, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&local_num_bins, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /** Populate the random data */
    int* data = new int[num_data];
    int* bins = new int[num_bins];
    int* local_data = new int[local_num_data];
    int* local_bins = new int[local_num_bins];

    /** Start the core components*/
    start = MPI_Wtime();
    setLocalData(data, local_data, num_data, local_num_data, rank_id);
//    printArray(local_data, local_num_data);
    setLocalBins(bins, local_bins, local_num_bins);
//    histogram(data, bins, local_data, local_bins, local_num_data, local_num_bins);
    histogram2(data, bins, local_data, local_bins, local_num_data, local_num_bins, num_bins, rank_id);
//    printArray(local_bins, local_num_bins);
    MPI_Reduce(local_bins, bins, num_bins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if(rank_id == 0) {
        stop = MPI_Wtime();
        printResult(bins, num_bins);
        cout << "Total running time is: " << stop - start << endl;
    }

    /** Clean */
    MPI_Finalize();
    return 0;
}

/**
 * Helper function to populate vector randomly in a range.
 * @param data
 * @param start
 * @param end
 * @param flag
 */
void populateVectorRandom(int data[], int length, int start, int end, int flag) {
    /** Generate random sequence*/
    random_device rd;
    mt19937 engine(rd());
    uniform_int_distribution<> u(start, end);
    normal_distribution<double> n(500, 1.0);

    //Populate random numbers into vectors
    if (flag == 1) {
        for (int i = 0; i < length; i++) {
            data[i] = u(engine);
        }
    } else if (flag == 2) {
        for (int i = 0; i < length; i++) {
            data[i] = lround(n(engine));
        }
    }
}

/**
 * Print Vector
 * @param data
 */
void printArray(int data[], int length) {
    int n = 0;
    for (int i = 0; i < length; i++) {
        // Set a threshold to print
        if (n >= 20) {
            cout << "only showing " << n << " elements, omit remaining......";
            break;
        }
        cout << data[i] << "\t";

        //Each line with 5 elements.
        if (++n % 5 == 0)
            cout << "\n";
    }
    cout << endl;
}

/**
 * Set the local data in nodes
 * @param data
 * @param local_data
 * @param local_num_data
 * @param flag
 */
void setLocalData(
        int data[],
        int local_data[],
        int num_data,
        int local_num_data,
        int rank_id) {
    if (rank_id == 0) {
        populateVectorRandom(data, num_data, MIN_RANGE, MAX_RANGE, 1);
    }
    MPI_Scatter(
            data,
            local_num_data,
            MPI_INT,
            local_data,
            local_num_data,
            MPI_INT,
            0,
            MPI_COMM_WORLD);
    if(rank_id == 0) {
        free(data);
    }
}

/**
 * Set the local bins in nodes
 * @param bins
 * @param local_bins
 * @param local_num_bins
 * @param flag
 */
void setLocalBins(
        int bins[],
        int local_bins[],
        int local_num_bins) {

//    int range = (MAX_RANGE - MIN_RANGE) / bins.size();

    for(int i = 0; i < local_num_bins; i++) {
        bins[i] = 0;
        local_bins[i] = 0;
    }
}

/**
 * Do the histogram
 * @param data
 * @param bins
 * @param local_data
 * @param local_bins
 * @param local_num_data
 * @param local_num_bins
 */
void histogram(
        int data[],
        int bins[],
        int local_data[],
        int local_bins[],
        int local_num_data,
        int local_num_bins) {

    int range,
            bin;

    if((MAX_RANGE - MIN_RANGE + 1) % local_num_bins != 0) {
        range = (MAX_RANGE - MIN_RANGE + 1) / local_num_bins + 1;
    } else {
        range = (MAX_RANGE - MIN_RANGE + 1) / local_num_bins;
    }

    for(int i = 0; i < local_num_data; i++) {
        bin = (local_data[i] - MIN_RANGE) / range;
        local_bins[bin]++;
    }

}

/**
 * Do the histogram for question 2
 * @param data
 * @param bins
 * @param local_data
 * @param local_bins
 * @param local_num_data
 * @param local_num_bins
 * @param rank_id
 */
void histogram2(
        int data[],
        int bins[],
        int local_data[],
        int local_bins[],
        int local_num_data,
        int local_num_bins,
        int num_bins,
        int rank_id) {

    int range,
            bin,
            lo,
            hi;

    if((MAX_RANGE - MIN_RANGE + 1) % num_bins != 0) {
        range = (MAX_RANGE - MIN_RANGE + 1) / num_bins + 1;
    } else {
        range = (MAX_RANGE - MIN_RANGE + 1) / num_bins;
    }

    lo = range * local_num_bins * rank_id + MIN_RANGE;
    hi = range * local_num_bins * (rank_id+1) - 1 + MIN_RANGE;

    for(int i = 0; i < local_num_data; i++) {
        int temp = local_data[i] - MIN_RANGE;
        if(temp > hi || temp < lo) continue;
        bin = temp / range;
        local_bins[bin]++;
    }

}

/**
 * Print the result
 * @param bins
 * @param length
 */
void printResult(int bins[], int length) {

    int range;

    if((MAX_RANGE - MIN_RANGE + 1) % length != 0) {
        range = (MAX_RANGE - MIN_RANGE + 1) / length + 1;
    } else {
        range = (MAX_RANGE - MIN_RANGE + 1) / length;
    }

    for(int i = 0; i < length; i++) {
        cout << "[" << (range * i + MIN_RANGE)
             << " - " << (range * (i+1) - 1 + MIN_RANGE)
             << "]: " << bins[i] << endl;
    }
}
