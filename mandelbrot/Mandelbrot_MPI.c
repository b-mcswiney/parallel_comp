//
// Simple C code for generating the Mandelbrot set, with parallelisation in OpenMP.
// A makefile it provided to aid compilation; just type 'make' and execute with './Mandelbrot'
// Purely meant for demonstration purposes and the OpenGL in particular could be optimised.
// - DAH/28/11/2017.
//
// Updated to use GLFW (rather than GLUT) for both Linux and Macs.
// - DAH/26/11/2019.
//
// Updated to use the terms 'main process' and 'worker'.
// - DAH/6/1/2021.
//



// Standard includes.
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

// For OpenGL windows. Should run on Linux (after loading the glfw module), or Macs (once glfw installed via homebrew),
// but may require changes for specific installations of glfw.
#include <GLFW/glfw3.h>


//
// Parameters.
//

// Comment out to use the strip partitioning version. Note even with the partitioned version,
// still use basic work pool structure to re-use as much code as possible.
//#define WORK_POOL

// MPI rank and number of processes are global variables (for simplicity).
int rank, numProcs;

// Window / view paraneters.
const int windowSize_x = 600;
const int windowSize_y = 600;

// Number of pixels to calculate; can be less than the window size.
#define numPixels_x 200
#define numPixels_y 200

// The maximum number iterations per pixel. Small values result in faster code but less well defined images.
const int maxIters = 300000;

// The colour arrays.
float red  [numPixels_x][numPixels_y];
float green[numPixels_x][numPixels_y];
float blue [numPixels_x][numPixels_y];

// Pointer to the GLFW struct.
GLFWwindow* window;



//
// Calculation routines and conversion from number of iterations to a colour.
//

// Calculates the value (number of iterations) for the given pixel.
int numIterations( int i, int j )
{
	// Initialise the variables (would be complex variables c and z)
	float
		cx = -2.0f + 4.0f * i / numPixels_x,
		cy = -2.0f + 4.0f * j / numPixels_y,
		zx = 0.0f,
		zy = 0.0f,
		ztemp;

	// The main loop.
	int numIters = 0;
	do
	{
		ztemp = zx*zx - zy*zy + cx;
		zy    = 2*zx*zy + cy;
		zx    = ztemp;
	}
	while( ++numIters<maxIters && zx*zx+zy*zy<4.0f );

	return numIters;
}

// Sets the colour of pixel (i,j) according to the passed value (number of iterations).
void setPixelColour( int i, int j, int numIters )
{
	// Colour based on the number of iterations. Cycle through RGB at different rates.
	if( numIters < maxIters )
	{
		red  [i][j] = 0.1f  * ( numIters%11 );
		green[i][j] = 0.05f * ( numIters%21 );
		blue [i][j] = 0.02f * ( numIters%51 );
	}
}


//
// Worker process: Receive row requests, calculates values and returns.
//
void workerProcess()
{
	int i;

	// A single row of pixels, plus the row number.
	int rowData[numPixels_x+1];

#ifdef WORK_POOL
	// Receive the (first) row request. Note are assuming there are more rows than workers!
	int rowToCalc;
	MPI_Recv( &rowToCalc, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

	// A row value of -1 means terminate.
	while( rowToCalc>=0 )
	{
		// Fill the row data.
		rowData[0] = rowToCalc;
		for( i=0; i<numPixels_x; i++ )
			rowData[i+1] = numIterations( i, rowToCalc );

		// Send to the main process.
		MPI_Send( rowData, numPixels_x+1, MPI_INT, 0, 0, MPI_COMM_WORLD );

		// Wait for the next row to calculate, or the request to terminate.
		MPI_Recv( &rowToCalc, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
	}
#else
	// Number of rows per (worker) process, rounded up.
	int numPerProc = (numPixels_y+numProcs-2)/(numProcs-1);

	int row;
	for( row=(rank-1)*numPerProc; row<rank*numPerProc; row++ )
		if( row<numPixels_y )
		{
			// Same as the work pool version for each individual row, except just send (no receive).
			rowData[0] = row;
			for( i=0; i<numPixels_x; i++ )
				rowData[i+1] = numIterations( i, row );

			MPI_Send( rowData, numPixels_x+1, MPI_INT, 0, 0, MPI_COMM_WORLD );
		}
#endif
}


//
// Main process. Also handles graphics output.
//

// Displays the current Mandelbrot set.
void displayImage(void)
{
	// Clear the display
	glClear( GL_COLOR_BUFFER_BIT );

	// Step sizes. The actual image is fixed at [-1,1] in both directions, but for the Mandelbrot set we want the range [-2,2].
	float dx = 2.0f / numPixels_x, dy = 2.0f / numPixels_y;

	// Display the image using a very basic double loop.
	int i, j;
	for( i=0; i<numPixels_x; i++ )
		for( j=0; j<numPixels_y; j++ )
		{
			glColor3f( red[i][j], green[i][j], blue[i][j] );
			glBegin( GL_POLYGON );
			glVertex3f( -1.0f +  i   *dx, -1.0f +  j   *dy, 0.0f );
			glVertex3f( -1.0f + (i+1)*dx, -1.0f +  j   *dy, 0.0f );
			glVertex3f( -1.0f + (i+1)*dx, -1.0f + (j+1)*dy, 0.0f );
			glVertex3f( -1.0f +  i   *dx, -1.0f + (j+1)*dy, 0.0f );
			glEnd();
		}
}

// Call back functions for GLFW.
void graphicsErrorCallBack( int errorCode, const char* message )
{
    printf( "Error with OpenGL/GLFW: code %i, message %s\n", errorCode, message );
}

void keyboardCallBack( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    // Close if the escape key or 'q' is pressed.
    if( (key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q) && action==GLFW_PRESS )
        glfwSetWindowShouldClose( window, 1 );
}

// Initial call to the main process.
void mainProcess()
{
	int i, j;

	//
	// Initialisation
	//

	// Set up the GLFW window.
	if( !glfwInit() ) return;
 	window = glfwCreateWindow( windowSize_x, windowSize_y, "Mandelbrot set generator: 'q' or <ESC> to quit", NULL, NULL );
	if( !window )
	{
		printf( "Could not open a GLFW window." );
		glfwTerminate();
		return;
    	}
	glfwSetErrorCallback( graphicsErrorCallBack );
	glfwSetKeyCallback( window, keyboardCallBack );
	glfwMakeContextCurrent(window);

	// Initialise the image to all pixels black.
	for( j=0; j<numPixels_y; j++ )
		for( i=0; i<numPixels_x; i++ )
 			red[i][j] = green[i][j] = blue[i][j] = 0.0f;

	// Initialisation for work pool or strip partition variables.

#ifdef WORK_POOL
	printf( "Using a work pool with %d workers and %d iterations maximum per pixel.\n", numProcs-1, maxIters );

	// Keep track of the rows that have been sent to the main process for calculation, and the number still to return.
	int workPool_row       = 0;
	int workPool_numActive = 0;

	// Send requests for the first rows for each process.
	int p;
	for( p=1; p<numProcs; p++ )
	{
		MPI_Send( &workPool_row, 1, MPI_INT, p, 0, MPI_COMM_WORLD );
		workPool_numActive++;
		workPool_row++;
	}

#else
	printf( "Using strip partitioning with %d partitions and %d iterations maximum per pixel.\n", numProcs-1, maxIters );
#endif

	//
	// Main loop for the main process.
	//

	// Start time.
	double startTime = MPI_Wtime();

	// The work pool variant uses the MPI_Status struct to determine which rank the row was sent from.
	MPI_Status status;

	// Storage of each row as received, before conversion to colours. First element is the row index.
 	int rowData[numPixels_x+1];

	// Display the image until quitting.
	int numRowsPlotted = 0;

	while( !glfwWindowShouldClose(window) )
	{
		// Get one row of pixels, and display after each new row is calculated.
		if( numRowsPlotted<numPixels_y )
		{
			// Both strip partition and work pool variants wait until receiving the next row from a worker.
			MPI_Recv( rowData, numPixels_x+1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status );

#ifdef WORK_POOL

			// Have we finished, or do we need to send out more requests?
			workPool_numActive--;
			if( workPool_row < numPixels_y )
			{
				int workerRank = status.MPI_SOURCE;				// The rank of the worker process that just sent the data.
				MPI_Send( &workPool_row, 1, MPI_INT, workerRank, 0, MPI_COMM_WORLD );
				workPool_numActive++;
				workPool_row++;
			}

			// Have we completely finished, i.e. sent out all row request and no active ones remaining?
			if( workPool_row==numPixels_y && workPool_numActive==0 )
			{
				// All done; send a termination request to each worker.
				int p, terminationRequest = -1;
				for( p=1; p<numProcs; p++ )
					MPI_Send( &terminationRequest, 1, MPI_INT, p, 0, MPI_COMM_WORLD );
			}
#endif
			// No code here for the strip partition version, as each worker already knows how many rows to send,
			// so the main process just needs to receive and display until all rows have been calculated.

			// Plot the row just received (i.e. convert number of iterations to colours).
			for( i=0; i<numPixels_x; i++ ) setPixelColour( i, rowData[0], rowData[i+1] );

			// Increment the number of rows plotted, and output the total time taken if finished.
			if( ++numRowsPlotted == numPixels_y )
				printf( "Time taken: %g secs.\n", MPI_Wtime() - startTime );
		}

		// Display the current Mandelbrot set until the user quits.
		glClear( GL_COLOR_BUFFER_BIT );
		displayImage();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//
	// Clear up and return.
	//
	glfwDestroyWindow( window );
	glfwTerminate();
}

//
// Main.
//
int main( int argc, char **argv )
{
 	// Initialise MPI. For simplicity, rank and numProcs are global variales for this demonstration.
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &numProcs );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank     );

	// Check for a valid number of processes.
	if( numProcs<2 )
	{
		printf( "Need at least 2 processes to work.\n" );
		MPI_Finalize();
		return EXIT_FAILURE;
	}
	if( numProcs -1 > numPixels_y )
	{
		printf( "Cannot have more processes than rows to plot!\n" );
		MPI_Finalize();
		return EXIT_FAILURE;
	}

	// How to proceed depends on whether we are the main, or one of the workers.
	if( rank==0 )
		mainProcess();
	else
		workerProcess();

	// Finalise and quit.
	MPI_Finalize();

	return EXIT_SUCCESS;
}
