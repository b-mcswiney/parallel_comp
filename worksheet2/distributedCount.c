//
// Simple distributed counting algorithm in MPI, to demonstrate some of the
// most commonly used collective communication routines: MPI_Bcast, MPI_Scatter
// and MPI_Gather.
//
// Compile with:
//
// mpicc -Wall -o distributedCount distributedCount.c
//


//
// Includes
//
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>


//
// Problem size (the size of the full data set). Will be rounded up to the
// nearest multiple of the number of processes at run time. Use a very large
// number so that the times are measurable, but not so many as to cause a
// memory allocation failure!
//
#define N 9999999


//
// Main
//
int main(int argc, char **argv) {
    int i, total, globalSize = -1, localSize;
    // int p // Not used in collective communication but kept for sake of comparison

    //
    // Initialisation
    //

    // Initialise MPI and get the rank and no. of processes.
    int rank, numProcs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Allocate memory for the full array on rank 0 only.
    int *globalData = NULL;
    if (rank == 0) {
        // Round up to the next-highest multiple of numProcs.
        globalSize = numProcs * ((N + numProcs - 1) / numProcs);

        // Try to allocate memory for the global data array.
        globalData = (int *) malloc(globalSize * sizeof(int));
        if (!globalData) {
            printf("Could not allocate memory for the global data array.\n");
            MPI_Finalize();
            return EXIT_FAILURE;
        }

        // Fill the array with random numbers in the range 0 to 99 inclusive. Not a very good way
        // of doing this (will generate a non-uniform distribution), but fine for this example.
        srand(time(NULL));
        for (i = 0; i < globalSize; i++)
            globalData[i] = rand() % 100;
    }

    // Start the timer.
    double startTime = MPI_Wtime();

    //
    // Step 1. All ranks must know the (dynamic) local array size. For this example they could all calculate
    // it independently, but imagine e.g. rank 0 read the data in from a file.
    //

    // All ranks (including rank 0) have a local array. However, they do not yet know the size, so cannot
    // allocate memory for the array, nor call MPI receive routines with the correct expected message size.

    // Point-to-point: Use a loop of send-and-receives.
//	if( rank==0 )
//	{
//		// Problem size per process, in principle not known to any rank except rank 0.
//		localSize = globalSize / numProcs;
//
//		// Note &localSize looks to the MPI function like an array of size 1.
//		for( p=1; p<numProcs; p++ )
//			MPI_Send( &localSize, 1, MPI_INT, p ,0, MPI_COMM_WORLD );
//	}
//	else
//	{
//		MPI_Recv( &localSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
//	}

    // collective communication implementation
    if(rank == 0) localSize = globalSize / numProcs; // rank 0 defines local size

    MPI_Bcast(&localSize, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // All ranks can now allocate memory for their local arrays.
    int *localData = (int *) malloc(localSize * sizeof(int));
    if (!localData) {
        printf("Could not allocate memory for the local data array on rank %d.\n", rank);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    //
    // Step 2. Distribute the global array to the local arrays on all processes.
    //

    // Point-to-point: Use a loop of send-and-receives. Note we copy the data from globalData to
    // localData for rank 0, to make replacing this with MPI_Scatter a bit easier.
//	if( rank==0 )
//	{
//		// Copy first segment to own localData (nb. never 'send' to self!)
//		for( i=0; i<localSize; i++ ) localData[i] = globalData[i];
//
//		// Send the remaining segments.
//		for( p=1; p<numProcs; p++ )
//			MPI_Send( &globalData[p*localSize], localSize, MPI_INT, p, 0, MPI_COMM_WORLD );
//	}
//	else
//	{
//		MPI_Recv( localData, localSize, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
//	}

    // Collective communication implementation
    MPI_Scatter(
            globalData, localSize, MPI_INT,
            localData, localSize, MPI_INT,
            0, MPI_COMM_WORLD
            );

    //
    // Step 3. Each process performs the count on its local data. This is purely computation and
    // doesn't need to be modified.
    //
    int count = 0;
    for (i = 0; i < localSize; i++)
        if (localData[i] < 10) count++;

    //
    // Step 4. Send all of the local counts back to rank 0, which calculates the total.
    //

    // Point-to-point: Use a loop of send-and-receives.
//    if (rank == 0) {
//        // Start the running total with rank 0's count.
//        total = count;
//
//        // Now add on all of the counts from the other processes.
//        for (p = 1; p < numProcs; p++) {
//            int next;
//            MPI_Recv(&next, 1, MPI_INT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//            total += next;
//        }
//    } else {
//        MPI_Send(&count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
//    }


    // Collective communication implementation
//    int *partials;
//    partials = (int *) malloc(numProcs * sizeof(int));
//
//    MPI_Gather(
//            &count, 1, MPI_INT,
//            partials, 1, MPI_INT,
//            0, MPI_COMM_WORLD
//            );

//    for(i=0; i<numProcs; i++)
//    {
//        total+=partials[i];
//    }

    // MPI_Reduce implementation of above loop and gather
    MPI_Reduce(&count, &total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    //
    // Check: Rank 0 performs an independent count on the global data. Rank 0 also outputs
    // how long it took (including allocation of the local arrays).
    //
    if (rank == 0) {
        printf("Time taken: %g s.\n", MPI_Wtime() - startTime);

        int check = 0;
        for (i = 0; i < globalSize; i++)
            if (globalData[i] < 10) check++;

        printf("Distributed count %d (cf. serial count %d).\n", total, check);
    }


    //
    // Clear up and quit.
    //
    if (rank == 0) free(globalData);
    free(localData);
    MPI_Finalize();
    return EXIT_SUCCESS;
}

