// Implement the kernel (or kernels) for coursework 3 in this file.

__kernel
void  weightsUpdate(__global float *gradients, __global float *inputs, __global float *weights, __constant int *device_M)
{
	// Global id tells us the index for this thread
	int gid = get_global_id(0);
	int M = *device_M;
	
	// Perform the weights editing loop
	int j;
	for( j=0; j<M; j++) weights[gid*M+j] += gradients[gid] * inputs[j];
}
