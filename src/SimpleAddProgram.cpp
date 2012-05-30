#include "SimpleAddProgram.h"
#include <boost/timer.hpp>

void CL_CALLBACK contextCallbackFunction(const char* errorinfo, const void* private_info_size, size_t cb, void* user_data);

#define DATA_SIZE 1048576

typedef cl_float DataType;

cl_int runSimpleAddProgram(std::vector<cl::Device>& deviceList, std::vector<CLHelper::DeviceInfo>& deviceInfoList)
{
	cl_int err;
	boost::timer timer;

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

// Create input and output Buffer objects using the host pointers
	cl::Buffer d_dataA(context, CL_MEM_READ_ONLY  | CL_MEM_USE_HOST_PTR, DATA_SIZE*sizeof(DataType), h_dataA, &err);
	CHECK_OPENCL_ERROR(err, "cl::Buffer::Buffer() failed.");
	cl::Buffer d_dataB(context, CL_MEM_READ_ONLY  | CL_MEM_USE_HOST_PTR, DATA_SIZE*sizeof(DataType), h_dataB, &err);
	CHECK_OPENCL_ERROR(err, "cl::Buffer::Buffer() failed.");
	cl::Buffer d_dataC(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, DATA_SIZE*sizeof(DataType), h_dataC, &err);
	CHECK_OPENCL_ERROR(err, "cl::Buffer::Buffer() failed.");

// Set the kernel arguments
	err  = simpleAddKernel.setArg(0, d_dataA);
	err |= simpleAddKernel.setArg(1, d_dataB);
	err |= simpleAddKernel.setArg(2, d_dataC);
	err |= simpleAddKernel.setArg(3, DATA_SIZE);
	CHECK_OPENCL_ERROR(err, "cl::Kernel::setArg() failed.");

// Get device information from deviceInfoList
	CLHelper::DeviceInfo deviceInfo = deviceInfoList.front();
	size_t workGroupSize = deviceInfo.maxWorkGroupSize;

// Keep halving workGroupSize until it divides perfectly into DATA_SIZE
	while((DATA_SIZE % workGroupSize) != 0) {
		workGroupSize /= 2;
	}
	
	timer.restart();

// Execute the kernel on the command queue
	cl::Event clEvent;
	err = commQueue.enqueueNDRangeKernel(
		simpleAddKernel,
		cl::NullRange,
		cl::NDRange(DATA_SIZE),
		cl::NDRange(workGroupSize), NULL, &clEvent);
	CHECK_OPENCL_ERROR(err, "cl::CommandQueue::enqueueNDRangeKernel() failed.");

// Wait until the kernel returns
	err = clEvent.wait();
	CHECK_OPENCL_ERROR(err, "cl::Event::wait() failed.");

	std::cout << "Time to run kernel: " << timer.elapsed() << std::endl;

// Map a host pointer to the Buffer
	DataType* result =
			(DataType*) commQueue.enqueueMapBuffer(d_dataC, true, CL_MAP_READ, 0, DATA_SIZE*sizeof(DataType), NULL, NULL, &err);
	CHECK_OPENCL_ERROR(err, "cl::CommandQueue::enqueueReadBuffer() failed.");

	std::cout << "Result: " << result[DATA_SIZE-1] << std::endl;

// Unmap the host pointer when done
	commQueue.enqueueUnmapMemObject(d_dataC, result);

// Free memory
	delete[] h_dataA;
	delete[] h_dataB;
	delete[] h_dataC;

	return CL_SUCCESS;
}

void CL_CALLBACK contextCallbackFunction(const char* errorinfo, const void* private_info_size, size_t cb, void* user_data)
{
	std::cerr << "contextCallbackFunction called!" << std::endl;
	std::cerr << errorinfo << std::endl;
}
