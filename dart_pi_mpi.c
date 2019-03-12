/**********************************
 * DESCRIPTION: A program to calculate PI using Monte Carlo method in MPI.
 *
 * Author: Kejie Zhang
 * LAST UPDATED: 03/5/2019
 *
 * USAGE:
 *   COMPILE: mpiCC dart_pi_mpi.c -o dart_pi_mpi
 *   RUN: mpirun -np <number of processes> ./dart_pi_mpi
 *
 * USEFUL REFERENCE:
 *    -> MPI: https://computing.llnl.gov/tutorials/openMP/
 *    -> Monte Carlo: http://www.thephysicsmill.com/2014/05/03/throwing-darts-pi/
**********************************/
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define NUM_THROWS 200000000
#define ROUNDS 10
#define PI 3.141592653589793

int main(int argc, char* argv[]) {
    int rank_id,
        num_procs,
        i,
        name_len;
    double x,
           y,
           local_pi,
           aver_pi,
           sum_pi,
           start,
           stop;
    unsigned long count,
                  j;
    char proc_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank_id);
    MPI_Get_processor_name(proc_name, &name_len);

    /** Start the work*/
    printf("Processor %s, rank %d out of %d processors starts to work\n",
            proc_name, rank_id, num_procs);
    start = MPI_Wtime();
    srandom(rank_id * rank_id);

    for(i = 0; i < ROUNDS; i++) {
        /**
         * Calculate the pi
         */
        count = 0;
        for(j = 0; j <= NUM_THROWS; j++) {
            x = (2.0 * (double)random()/RAND_MAX) - 1.0;
            y = (2.0 * (double)random()/RAND_MAX) - 1.0;

            if(x * x + y * y <= 1.0) {
                count++;
            }
        }

        local_pi = (double)(4.0 * count / NUM_THROWS);

        /** Reduce the result*/
        MPI_Reduce(&local_pi, &sum_pi, 1, MPI_DOUBLE, MPI_SUM,
                        0, MPI_COMM_WORLD);

        if(rank_id == 0) {
            aver_pi = ((aver_pi * i) + sum_pi / num_procs) / (i + 1);
            printf("ROUND %d, the value of PI is %.15f\n", i, aver_pi);
        }
    }

    if(rank_id == 0) {
        stop = MPI_Wtime();
        printf("\nThe final value of PI is %.15f\n", aver_pi);
        printf("The real value of PI is %.15f\n", PI);
        printf("Total running time: %f\n", stop - start);
        fflush(stdout);
    }
    MPI_Finalize();
    return 0;
}