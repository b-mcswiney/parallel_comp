//
// An example of a cyclic communication pattern that results in deadlock for large messages,
// i.e. those that are too large to fit into the buffer.
//
// Compile with:
//
// mpicc -Wall -o cyclicSendAndReceive cyclicSendAndReceive.c
//
// When executing, requires a single command line argument: The array size for each process.
// Increasing this beyond the buffer size (which depends on the MPI implementation) should
// cause deadlock. See also the question on the worksheet. For example:
//
// mpiexec -n 4 ./cyclicSendAndReceive 100
//
// will run with an array size of 100 for each process on a 4-core machine.
//


//
// Includes
//
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


//
// Main
//
int main( int argc, char **argv )
{
	// Initialise MPI.
	MPI_Init( &argc, &argv );

	// Get the total number of processes, and the rank (i.e. the process ID for 'this' process).
	int numProcs, rank;
	MPI_Comm_size( MPI_COMM_WORLD, &numProcs );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank     );

	// Get the command line argument corresponding to the array size. This is required.
	// Do this for all ranks (i.e. processes); we could alternatively only let 1 process
	// do the parsing, and let it send the result to all other processes.
	if( argc != 2 )			// Need argc==2, since the first argument is the name of the executable.
	{
		// Only output the error message once, i.e. by one rank.
		if( rank==0 ) printf( "Error: Need single argument for the message size.\n");
		MPI_Finalize();
		return EXIT_FAILURE;
	}

	// Get the message size from the command line. Must be positive.
	int N = atoi( argv[1] );
	if( N <= 0 )
	{
		if( rank==0 ) printf( "Error: Message size must be positive.\n" );
		MPI_Finalize();
		return EXIT_FAILURE;
	}

	//
	//	Main part of code.
	//

	// Allocate memory for the arrays (same size on every process). Use int's for this example.
	int
		*sendData = (int*) malloc( sizeof(int)*N ),
		*recvData = (int*) malloc( sizeof(int)*N );

	// Fill the sendData array with some numbers; for the purpose of this exercise it doesn't matter what,
	// although it helps debugging if each rank's array contains different values.
	int i;
	for( i=0; i<N; i++ ) sendData[i] = (rank+1)*(i+1);

	// Send data 'cyclically to the right' (i.e. to rank+1, with wrap-around).
//	MPI_Send( sendData, N, MPI_INT, ( rank==numProcs-1 ? 0 : rank+1 ), 0, MPI_COMM_WORLD );


	// Receive data 'from the left'. Here use a status object but do nothing with it (could also replace &status with MPI_STATUS_IGNORE).
	MPI_Status status;
//	MPI_Recv( recvData, N, MPI_INT, ( rank==0 ? numProcs-1 : rank-1 ), 0, MPI_COMM_WORLD, &status );

    // Staggered send and receive
    if(rank % 2)
    {
        MPI_Recv( recvData, N, MPI_INT, ( rank==0 ? numProcs-1 : rank-1), 0, MPI_COMM_WORLD, &status);
        MPI_Send( sendData, N, MPI_INT, (rank==numProcs-1 ? 0 : rank+1), 0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Send( sendData, N, MPI_INT, (rank==numProcs-1 ? 0 : rank+1), 0, MPI_COMM_WORLD);
        MPI_Recv( recvData, N, MPI_INT, (rank==0 ? numProcs-1 : rank-1), 0, MPI_COMM_WORLD, &status);
    }

	//
	// Check the result
	//

	// Print a few values of recvData on rank==0; should correspond to sendData on rank==numProcs-1.
	if( rank==0 )
		for( i=0; i<(N<10?N:10); i++ ) printf( "%i\t%i\n", i, recvData[i] );

	// Deallocate memory.
	free( sendData );
	free( recvData );

	// Clear up MPI.
	MPI_Finalize();

	// Quit.
	return EXIT_SUCCESS;
}
