//
// Starting point for the GPU coursework. Please read coursework instructions before attempting this.
//


//
// Includes.
//
#include <stdio.h>
#include <stdlib.h>
#include "helper_cwk.h"			// Note this is not the same as the 'helper.h' used for examples.


//
// Main.
//
int main( int argc, char **argv )
{
	//
	// Initialisation.
	//
	
	// Initialise OpenCL. This is the same as the examples in lectures.
	cl_device_id device;
	cl_context context = simpleOpenContext_GPU(&device);

	cl_int status;
	cl_command_queue queue = clCreateCommandQueue( context, device, 0, &status );
	
	// Get the parameters (N = no. of nodes/gradients, M = no. of inputs). getCmdLineArgs() is in helper_cwk.h.
	int N, M;
	getCmdLineArgs( argc, argv, &N, &M );

	// Initialise host arrays. initialiseArrays() is defined in helper_cwk.h. DO NOT REMOVE or alter this routine;
	// it will be replaced with a different version as part of the assessment.
	float
		*gradients = (float*) malloc( N  *sizeof(float) ),
		*inputs    = (float*) malloc(   M*sizeof(float) ),
		*weights   = (float*) malloc( N*M*sizeof(float) );
	initialiseArrays( gradients, inputs, weights, N, M );			// DO NOT REMOVE.
	
	
	//
	// Implement the GPU solution to the problem.
	//

	// Allocate memory for the arrays
	cl_mem device_gradients = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, N*sizeof(float), gradients, &status);
	cl_mem device_inputs = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, M*sizeof(float), inputs, &status);
	cl_mem device_weights = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, M*N*sizeof(float), weights, &status);
	
	//
	// Perform calculation on the GPU
	//
	
	// Build the kernel code
	cl_kernel kernel = compileKernelFromFile("cwk3.cl", "weightsUpdate", context, device);
	
	// Specify args to the kernel
	status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &device_gradients);
	status = clSetKernelArg(kernel, 1, sizeof(cl_mem), &device_inputs);
	status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &device_weights);

	// Get maximum work group size for device
	size_t maxWorkItems;
	glGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &maxWorkItems, NULL);

	// Set up global problem size and the work group size
	size_t indexSpaceSize[1], workGroupSize[1];
	indexSpaceSize[0] = N;
	workGroupSize[0] = maxWorkItems;

	// Put kernel onto the command queue.
	status = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, indexSpaceSize, workGroupSize, 0, NULL, NULL);

	//
	// Output the result and clear up.
	//
	
	// Get result from device to host
	status = clEnqueueReadBuffer(queue, device_weights, CL_TRUE, 0, N*M*sizeof(float), weights, 0, NULL, NULL);
	
	

	// Output result to screen. DO NOT REMOVE THIS LINE (or alter displayWeights() in helper_cwk.h); this will be replaced
	// with a different displayWeights() for the the assessment, so any changes you might make will be lost.
	displayWeights( weights, N, M) ;								// DO NOT REMOVE.
	
	free( gradients );
	free( inputs    );
	free( weights   );

	clReleaseCommandQueue( queue   );
	clReleaseContext     ( context );

	return EXIT_SUCCESS;
}
