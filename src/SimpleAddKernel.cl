__kernel
void simpleAddKernel(__global float* dataA, __global float* dataB, __global float* dataC, unsigned int dataSize)
{
	unsigned int threadId = get_global_id(0);

	if(threadId < dataSize)
		dataC[threadId] = dataA[threadId] + dataB[threadId];
}