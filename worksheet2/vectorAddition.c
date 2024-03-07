//
// An example of vector addition in MPI-C. Compile with:
//
// mpicc -Wall -o vectorAddition vectorAddition.c
//
// and execute with (on one machine with 4 cores):
//
// mpiexec -n 4 ./vectorAddition
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
int main( int argc, char **argv )
{
	int i, p;

	// Initialise MPI and find the total number of processes and the rank of 'this' process.
	int rank, numProcs;
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &numProcs );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank     );

	// Check that the problem size is a multiple of the number of processes.
	if( N%numProcs )
	{
		if( rank==0 ) printf( "Problem size '%d' is not a multiple of the number of processes '%d.'\n", N, numProcs );
		MPI_Finalize();
		return EXIT_FAILURE;
	}

	// Initialise the full vectors a, b and c on rank 0 only.
	float *a=NULL, *b=NULL, *c=NULL;
	if( rank==0 )
	{
		// Allocate memory for the vectors.
		a = (float*) malloc( N*sizeof(float) );
		b = (float*) malloc( N*sizeof(float) );
		c = (float*) malloc( N*sizeof(float) );
		if( !a || !b || !c )
		{
			printf( "Could not allocate memory for the vectors.\n" );
			MPI_Finalize();
			return EXIT_FAILURE;
		}

		// Put values into vectors a and b only.
		for( i=0; i<N; i++ ) a[i] = b[i] = i+1;
	}

	// Initialise the local vectors on all processes except rank 0. Note that rank 0 can use the
	// same arrays a, b and c as for the full vectors, but it helps keep things easier to understand
	// to use new names for the local variables and arrays.
	int localSize = N / numProcs;
	float *local_a=NULL, *local_b=NULL, *local_c=NULL;
	if( rank>0 )
	{
		local_a = (float*) malloc( localSize*sizeof(float) ),
		local_b = (float*) malloc( localSize*sizeof(float) ),
		local_c = (float*) malloc( localSize*sizeof(float) );
	}

	//
	// Step 1. Rank 0 sends segments of the full vectors a and b to their respective processes;
	// those processes must in turn receive the data.
	//
	if( rank==0 )
	{
		for( p=1; p<numProcs; p++ )
		{
			MPI_Send( &a[p*localSize], localSize, MPI_FLOAT, p, 0, MPI_COMM_WORLD );
			MPI_Send( &b[p*localSize], localSize, MPI_FLOAT, p, 0, MPI_COMM_WORLD );
		}
	}
	else
	{
		MPI_Recv( local_a, localSize, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
		MPI_Recv( local_b, localSize, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
	}

	//
	// Step 2. Each process performs vector addition on their local segments of the full arrays.
	//
	if( rank==0 )
		for( i=0; i<localSize; i++ ) c[i] = a[i] + b[i];
	else
		for( i=0; i<localSize; i++ ) local_c[i] = local_a[i] + local_b[i];

	//
	// Step 3. Recombine the full c array back on rank 0.
	//
	if( rank==0 )
	{
		for( p=1; p<numProcs; p++ )
			MPI_Recv( &c[p*localSize], localSize, MPI_FLOAT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
	}
	else
	{
		MPI_Send( local_c, localSize, MPI_FLOAT, 0, 0, MPI_COMM_WORLD );
	}

	// Check the answer on rank 0. Only display the first few array elements if N is large.
	if( rank==0 )
	{
		for( i=0; i<(N>20?20:N); i++ )
			printf( "%g + %g = %g.\n", a[i], b[i], c[i] );
		if( N>20 )
			printf( "(remaining elements not displayed)\n" );
	}

	// Clear up.
	if( rank==0 )
	{
		free( a );
		free( b );
		free( c );
	}
	else
	{
		free( local_a );
		free( local_b );
		free( local_c );
	}
	MPI_Finalize();
	return EXIT_SUCCESS;
}
