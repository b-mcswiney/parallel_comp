// Implement the kernel (or kernels) for coursework 3 in this file.

__kernel
void  weightsUpdate(__global float *gradients, __global float *inputs, __global float *weights, int device_M, int device_N)
{
	// Global id tells us the index for this thread
	int gid = get_global_id(0);


	
	// Perform the weights editing
	weights[gid] += gradients[gid / device_M] * inputs[gid % device_M];
}
