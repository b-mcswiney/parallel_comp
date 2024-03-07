//
// MPI implementation of the heat equation (aka the diffusion equation) in 2D,
// using the standard approach of domain partitioning with ghost cells.
//
// Compile with:
//
// mpicc -Wall -o heatEqn heatEqn.c
//
// and launch with a square number of processes, i.e. 4, 9, ...
//
// mpiexec -n 4 ./heatEqn
//
// In addition to being a square number, the number of domains in both directions
// must divide the global grid size L. Therefore running on 9 processes won't work
// unless you also change L to (say) 18 (L is a #define near the start of this file).
//


//
// Includes.
//
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


//
// Parameters and global variables.
//
#define L 8
#define NUM_ITERATIONS 10

int local_L;														// The dimensions of the local grids. Convenient to make it global.


//
// Function prototypes; definitions after main().
//
int  _index( int row, int col );									// Returns the grid index for the given global row and column.
void initialiseGrid( float *grid, int rank, int p );				// Fills the initial grid.
void displayGrid   ( float *grid, int rank, int p );				// Displays the current grid.


//
// Main.
//
int main( int argc, char **argv )
{
	//
	// Initialisation.
	//

	// Initialise MPI and get the rank and total number of processes.
	int rank, numProcs;
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &numProcs );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank     );

	// Check first that the number of processes is a square number (p*p).
	int p = 1;
	while( p*p < numProcs ) p++;
	if( p*p != numProcs )
	{
		if( rank==0 ) printf( "Must execute using a square number of processes (4,9,...).\n" );
		MPI_Finalize();
		return EXIT_FAILURE;
	}

	// Now check that the domains are commensurate with the number of processes per side.
	if( L%p )
	{
		if( rank==0 ) printf( "Grid dimension %d needs to be a multiple of the number of processes per side %d.\n", L, p );
		MPI_Finalize();
		return EXIT_FAILURE;
	}

	// Initialise the local grids for each process (not there is no 'global grid' here).
	local_L = L / p;
	float *grid = (float*) malloc( (local_L+2)*(local_L+2)*sizeof(float) );			// local_L+2 so we also include the ghost cells.

	// Fill in the original grid.
	initialiseGrid( grid, rank, p );

	// Display the initial grid.
	if( rank==0 ) printf( "Initial grid:\n" );
	displayGrid( grid, rank, p );

	// For left and right boundaries, use a temporary array to extract single columns.
	float *column = (float*) malloc( local_L*sizeof(float) );

	// Start the timer.
	double startTime = MPI_Wtime();

	//
	// Iteration.
	//
	int iter, row, col;
	for( iter=0; iter<NUM_ITERATIONS; iter++ )
	{
		// Get the row and column for 'this' rank in the partitioned domain.
		int rowBlock = rank / p, colBlock = rank % p;

		//
		// Synchronise the ghost cells.
		//

		// Upper boundary.
		if( rowBlock>0   ) MPI_Send( &grid[_index(        1,1)], local_L, MPI_FLOAT, rank-p, 0, MPI_COMM_WORLD );
		if( rowBlock<p-1 ) MPI_Recv( &grid[_index(local_L+1,1)], local_L, MPI_FLOAT, rank+p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

		// Lower boundary.
		if( rowBlock<p-1 ) MPI_Send( &grid[_index(local_L,1)], local_L, MPI_FLOAT, rank+p, 0, MPI_COMM_WORLD );
		if( rowBlock>0   ) MPI_Recv( &grid[_index(      0,1)], local_L, MPI_FLOAT, rank-p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

		// Left boundary.
		if( colBlock>0 )
		{
			for( row=1; row<local_L+1; row++ ) column[row-1] = grid[_index(row,1)];
			MPI_Send( column, local_L, MPI_FLOAT, rank-1, 0, MPI_COMM_WORLD );
		}
		if( colBlock<p-1 )
		{
			MPI_Recv( column, local_L, MPI_FLOAT, rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			for( row=1; row<local_L+1; row++ ) grid[_index(row,local_L+1)] = column[row-1];
		}

		// Right boundary.
		if( colBlock<p-1 )
		{
			for( row=1; row<local_L+1; row++ ) column[row-1] = grid[_index(row,local_L)];
			MPI_Send( column, local_L, MPI_FLOAT, rank+1, 0, MPI_COMM_WORLD );
		}
		if( colBlock>0 )
		{
			MPI_Recv( column, local_L, MPI_FLOAT, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			for( row=1; row<local_L+1; row++ ) grid[_index(row,0)] = column[row-1];
		}

		//
		// Perform the calculations, split into interior and edge points to help with the conversion to non-blocking.
		//

		// First update the interior grid points (using a Jacobi iteration).
		for( row=2; row<local_L; row++ )
			for( col=2; col<local_L; col++ )
				grid[_index(row,col)] = 0.25 * (
										  grid[_index(row+1,col  )]
										+ grid[_index(row-1,col  )]
										+ grid[_index(row  ,col+1)]
										+ grid[_index(row  ,col-1)]
									);

		// Now update the edge cells, i.e. those that need to read the ghost cells.
		for( row=1; row<local_L+1; row++ )
			for( col=1; col<local_L+1; col++ )
				if( row==1 || row==local_L || col==1 || col==local_L )
					grid[_index(row,col)] = 0.25 * (
											  grid[_index(row+1,col  )]
											+ grid[_index(row-1,col  )]
											+ grid[_index(row  ,col+1)]
											+ grid[_index(row  ,col-1)]
										);
	}

	// Calculate how long the calculation took.
	double endTime = MPI_Wtime();

	// Display the final grid and the time taken.
	if( rank==0 ) printf( "\nFinal grid:\n" );
	displayGrid( grid, rank, p );
	if( rank==0 ) printf( "\nTime taken: %g s.\n", endTime - startTime );

	//
	// Clear up and quit.
	//
	free( grid );
	free( column );
	MPI_Finalize();
	return EXIT_SUCCESS;
}


//
// Functions.
//

// Have used 1D arrays (rather than 2D), so perform the indexing 'by hand.'
int _index( int row, int col ) { return row*(local_L+2) + col; }

// Initialise the local grid for this process.
void initialiseGrid( float *grid, int rank, int p )
{
	int i, j, l=local_L;

	// Set all nodes to zero. This will be our boundary condition for this example.
	for( i=0; i<l+2; i++ )
		for( j=0; j<l+2; j++ )
			grid[_index(i,j)] = 0.0f;

	// Now overwrite the internal nodes with some values.
	for( i=1; i<l+1; i++ )
		for( j=1; j<l+1; j++ )
			grid[_index(i,j)] = rank+1;
}

// Displays the current grid. Uses point-to-point communication to send everything to rank 0,
// which then prints in order.
void displayGrid( float *grid, int rank, int p )
{
	int rowBlock, colBlock, row, col, source, l=local_L;
	MPI_Status status;

	// Only display if small enough.
	if( L>32 )
	{
		if( rank==0 ) printf( "Not displaying grid; too big.\n" );
		return;
	}

	// Scratch array for reading in one row at a time.
	float *scratch = (float*) malloc( l*sizeof(float) );

	// Print the upper row of (zero) boundary conditions, and a divider.
	if( rank==0 )
	{
		printf( "%6.3f | ", 0.0f );
		for( colBlock=0; colBlock<p; colBlock++ )
		{
			for( col=1; col<l+1; col++ ) printf( "%6.3f ", 0.0f );
			printf( "| " );
		}
		printf( "%6.3f\n", 0.0f );

		for( col=0; col<7*(p*l+2)+2*(p+1)-1; col++ ) printf( "-" );
		printf( "\n" );
	}

	// Loop over processes in blocks of rows.
	for( rowBlock=0; rowBlock<p; rowBlock++ )
	{

		// Loop over all rows in this rowBlock
		for( row=1; row<l+1; row++ )
		{
			// Print the zero boundary value first, with a divider.
			if( rank==0 ) printf( "%6.3f | ", 0.0f );

			// Now loop over the blocks of columns.
			for( colBlock=0; colBlock<p; colBlock++ )
			{
				// The rank of the process with the data we need.
				source = p*rowBlock + colBlock;

				// If source matches this rank, we need to send data to rank 0 (unless we already are rank 0).
				if( rank!=0 )
					if( source==rank )
						MPI_Send( &grid[_index(row,1)], l, MPI_FLOAT, 0, 0, MPI_COMM_WORLD );

				// Display the data always from rank 0, to preserve ordering.
				if( rank==0 )
				{
					if( source==0 )
					{
						for( col=1; col<l+1; col++ ) printf( "%6.3f ", grid[_index(row,col)] );
					}
					else
					{
						MPI_Recv( scratch, l, MPI_FLOAT, source, 0, MPI_COMM_WORLD, &status );
						for( col=1; col<l+1; col++ ) printf( "%6.3f ", scratch[col-1] );
					}
				}

				 // Add vertical lines between blocks.
				 if( rank==0 ) printf( "| " );
			}

			// End of row; print the right hand boundary value and a newline.
			if( rank==0 ) printf( "%6.3f\n", 0.0f );
		}

		// End of row block; print a divider (if not the last one).
		if( rowBlock!=p-1 && rank==0 )
		{
			for( col=0; col<7*(p*l+2)+2*(p+1)-1; col++ ) printf( "-" );
			printf( "\n" );
		}
	}

	// Plot last divider and final row of zero boundaries.
	if( rank==0 )
	{
		for( col=0; col<7*(p*l+2)+2*(p+1)-1; col++ ) printf( "-" );
		printf( "\n" );

		printf( "%6.3f | ", 0.0f );
		for( colBlock=0; colBlock<p; colBlock++ )
		{
			for( col=1; col<l+1; col++ ) printf( "%6.3f ", 0.0f );
			printf( "| " );
		}
		printf( "%6.3f\n", 0.0f );
	}

	// Clear up and return.
	free( scratch );
}

