//
// Created by nano on 07/03/24.
//


//
// An example of vector addition in MPI-C. Compile with:
//
// mpicc -Wall -o vectorDouble vectorDouble.c
//
// and execute with (on one machine with 4 cores):
//
// mpiexec -n 4 -oversubscribe ./vectorDouble
//
// For more instructions see the notes at the end of Lecture 8.
//


//
// Includes
//
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


//
// Problem size.
//
#define N 100


//
// Main
//
int main(int argc, char **argv) {
    int i, p;

    // Initialise MPI and find total number of processes and rank of current process
    int rank, numProcs;
    MPI_Init(&argc, &argv); // initialise
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs); //get size of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //find rank of current

    // Check problem size is multiple of processes
    if (N % numProcs) {
        if (rank == 0) printf("Problem size not multiple of processes");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    // Initialise full vectors
    float *a = NULL, *b = NULL;

    if (rank == 0) {
        // Allocate memory for full vectors
        a = (float *) malloc(N * sizeof(float));
        b = (float *) malloc(N * sizeof(float));

        if (!a || !b) {
            printf("Could not allocate memory for vectors\n");
            MPI_Finalize();
            return EXIT_FAILURE;
        }

        // put values into vector to be doubled
        for (i = 0; i < N; i++) a[i] = i + 1;
    }

    // Initialise local vectors for all other processes
    int localsize = N / numProcs;
    float *local_a = NULL, *local_b = NULL;

    if (rank > 0) {
        local_a = (float *) malloc(localsize * sizeof(float));
        local_b = (float *) malloc(localsize * sizeof(float));
    }

    // Step 1. Rank 0 sends segments of vectors to all processes.
    // other processes then receive this data
    if (rank == 0) {
        for (p = 1; p < numProcs; p++) {
            MPI_Send(&a[p * localsize], localsize, MPI_FLOAT, p, 0, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(local_a, localsize, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Step 2 perform doubling on each process
    if (rank == 0) {
        for (i = 0; i < localsize; i++) b[i] = a[i] * 2;
    } else {
        for (i = 0; i < localsize; i++) local_b[i] = local_a[i] * 2;
    }

    // step 3. Recombine full array back on rank 0
    if (rank == 0) {
        for(p=1; p<numProcs; p++)
            MPI_Recv(&b[p*localsize], localsize, MPI_FLOAT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    else {
        MPI_Send(local_b, localsize, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }

    // Check answer on rank 0.
    if(rank == 0)
    {
        for(i=0; i<(N>20?20:N); i++)
            printf("%g * 2 = %g. \n", a[i], b[i]);
        if( N>20 )
            printf("(Remaining elements not displayed)\n");
    }

    // Clean up
    if(rank == 0)
    {
        free(a);
        free(b);
    }
    else
    {
        free(local_a);
        free(local_b);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
