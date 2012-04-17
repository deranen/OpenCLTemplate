#include "SimpleAddProgram.h"

void CL_CALLBACK contextCallbackFunction(const char* errorinfo, const void* private_info_size, size_t cb, void* user_data);

cl_int runSimpleAddProgram(std::vector<cl::Device>& deviceList, std::vector<streamsdk::SDKDeviceInfo>& deviceInfoList)
{
	cl_int err;
	streamsdk::SDKCommon timer;

// Create a Context from the list of devices
	cl::Context context(deviceList, NULL, &contextCallbackFunction, NULL, &err);
	CHECK_OPENCL_ERROR(err, "cl::Context::Context() failed.");

// Load the .cl file into a string
	std::string source;
	CLHelper::loadKernelFileToString("SimpleAddKernel.cl", &source);

// Add the source code to a Source object using make_pair
	cl::Program::Sources sources;
	sources.push_back(std::make_pair(source.c_str(), source.length()));

// Create a Program object from the Context and the Source
	cl::Program program(context, sources, &err);
	CHECK_OPENCL_ERROR(err, "cl::Program::Program() failed.");

// Compile the program, optionally giving arguments to the compiler
	CLHelper::compileProgram(program, deviceList, "");

// Pick out a specific kernel function from the compiled Program object
	cl::Kernel simpleAddKernel(program, "simpleAddKernel", &err);
	CHECK_OPENCL_ERROR(err, "cl::Kernel::Kernel() failed.");

// Allocate input and output arrays
	DataType* h_dataA = new DataType[DATA_SIZE];
	DataType* h_dataB = new DataType[DATA_SIZE];
	DataType* h_dataC = new DataType[DATA_SIZE];

// Initialize the input arrays
	for(size_t i = 0; i < DATA_SIZE; i++)
	{
		h_dataA[i] = (DataType) i;
		h_dataB[i] = (DataType) i;
		h_dataC[i] = (DataType) 0;
	}

// Create input and output Buffer objects using the host pointes
	cl::Buffer d_dataA(context, CL_MEM_READ_ONLY  | CL_MEM_USE_HOST_PTR, DATA_SIZE*sizeof(DataType), h_dataA, &err);
	CHECK_OPENCL_ERROR(err, "cl::Buffer::Buffer() failed.");
	cl::Buffer d_dataB(context, CL_MEM_READ_ONLY  | CL_MEM_USE_HOST_PTR, DATA_SIZE*sizeof(DataType), h_dataB, &err);
	CHECK_OPENCL_ERROR(err, "cl::Buffer::Buffer() failed.");
	cl::Buffer d_dataC(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, DATA_SIZE*sizeof(DataType), h_dataC, &err);
	CHECK_OPENCL_ERROR(err, "cl::Buffer::Buffer() failed.");

// Create command queues for all devices and put them into 'commQueueList'
	std::vector<cl::CommandQueue> commQueueList;
	std::vector<cl::Device>::iterator device;
	for(device = deviceList.begin(); device != deviceList.end(); device++)
	{
		commQueueList.push_back(cl::CommandQueue(context, *device, 0, &err));
		CHECK_OPENCL_ERROR(err, "cl::CommandQueue::CommandQueue() failed.");
	}

// For the rest of the program, pick the first command queue from the list (and ignore the rest of the devices, if any)
	cl::CommandQueue commQueue = commQueueList.front();

// Use enqueueWriteBuffer to write the data to the buffers (not sure if this is correct, maybe use enqueueMapBuffer?)
	err  = commQueue.enqueueWriteBuffer(d_dataA, true, 0, DATA_SIZE*sizeof(DataType), h_dataA, NULL, NULL);
	err |= commQueue.enqueueWriteBuffer(d_dataB, true, 0, DATA_SIZE*sizeof(DataType), h_dataB, NULL, NULL);
	CHECK_OPENCL_ERROR(err, "cl::CommandQueue::enqueueWriteBuffer() failed.");

// Set the kernel arguments
	err  = simpleAddKernel.setArg(0, d_dataA);
	err |= simpleAddKernel.setArg(1, d_dataB);
	err |= simpleAddKernel.setArg(2, d_dataC);
	CHECK_OPENCL_ERROR(err, "cl::Kernel::setArg() failed.");

// Get device information from deviceInfoList
	streamsdk::SDKDeviceInfo deviceInfo = deviceInfoList.front();
	size_t workItemsPerWorkGroup = deviceInfo.maxWorkGroupSize;

// Keep halving workItemsPerWorkgroup until it divides perfectly into DATA_SIZE
	while((DATA_SIZE % workItemsPerWorkGroup) != 0) {
		workItemsPerWorkGroup /= 2;
	}
	
// Execute the kernel on the command queue
	cl::Event clEvent;
	err = commQueue.enqueueNDRangeKernel(
		simpleAddKernel,
		cl::NullRange,
		cl::NDRange(DATA_SIZE),
		cl::NDRange(workItemsPerWorkGroup), NULL, &clEvent);
	CHECK_OPENCL_ERROR(err, "cl::CommandQueue::enqueueNDRangeKernel() failed.");

// Wait until the kernel returns
	err = clEvent.wait();
	CHECK_OPENCL_ERROR(err, "cl::Event::wait() failed."); 

// Read the result from the device into a host pointer
	err = commQueue.enqueueReadBuffer(d_dataC, true, 0, DATA_SIZE*sizeof(DataType), h_dataC, NULL, NULL);
	CHECK_OPENCL_ERROR(err, "cl::CommandQueue::enqueueReadBuffer() failed.");

	std::cout << "Result: " << h_dataC[DATA_SIZE-1] << std::endl;

// Free memory
	delete[] h_dataA;
	delete[] h_dataB;
	delete[] h_dataC;
}

void CL_CALLBACK contextCallbackFunction(const char* errorinfo, const void* private_info_size, size_t cb, void* user_data)
{
	std::cerr << "contextCallbackFunction called!" << std::endl;
	std::cerr << errorinfo << std::endl;
}