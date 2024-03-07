//
// Code to measure t_{\rm comm} in MPI, the idea being to
// test the usual linear function with a startup and a transmission.
//
// Compile with:
//
// mpicc -Wall -o measure_tComm measure_tComm.c
//
// and execute on multiple machines (instructions on worksheet notes and in lectures),
// remembering to use more than one physical machine.
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
int main( int argc, char** argv )
{
	//
	//	Set up MPI
	//
	int rank, numProcs;
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &numProcs );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank     );

	//
	//	Start the timing runs.
	//
	double *sendData=NULL, *recvData=NULL;
	int i, m = 100;					// Starting message size (no. of doubles).
	do
	{
		// Set up array sizes for each m (use the same data each time).
		if( rank==0 )
		{
			sendData = (double*) malloc( sizeof(double)*m );
			for( i=0; i<m; i++ ) sendData[i] = i;				// Could actually leave with initial (garbage) values.
		}
		if( rank==numProcs-1 )
		{
			recvData = (double*) malloc( sizeof(double)*m );
		}

		// Start the timing.
		double startTime = MPI_Wtime();

		int sample, numSamples = 1000;							// No. samples to average over.
		for( sample=0; sample<numSamples; sample++ )
		{
			if( rank==0          ) MPI_Send( sendData, m, MPI_DOUBLE, numProcs-1, 0, MPI_COMM_WORLD                    );
			if( rank==numProcs-1 ) MPI_Recv( recvData, m, MPI_DOUBLE, 0,          0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

			MPI_Barrier( MPI_COMM_WORLD );						// Don't allow overlapping communications.
		}

		// Total time.
		double timeElapsed = MPI_Wtime() - startTime;

		// Output results.
		if( rank==0 )
			printf( "%i\t%g\n", m, timeElapsed/numSamples );

		// Deallocate memory used by the arrays before resizing them.
		if( rank==0          ) free( sendData );
		if( rank==numProcs-1 ) free( recvData );

		m *= 2;					// Increase message size by a factor of 2.

	} while( m < 30000 );

	//
	//	Clear up and quit
	//
	MPI_Finalize();
	return EXIT_SUCCESS;
}


