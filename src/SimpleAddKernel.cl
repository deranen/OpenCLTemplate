#define DATA_SIZE 1024

typedef float DataType;

__kernel
void simpleAddKernel(__global DataType* dataA, __global DataType* dataB, __global DataType* dataC)
{
	int threadId = get_global_id(0);

	if(threadId < DATA_SIZE)
		dataC[threadId] = dataA[threadId] + dataB[threadId];
}