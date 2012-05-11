#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "CLHelper.h"

namespace fs = boost::filesystem;

cl_int CLHelper::findSpecifiedDevices(
	const std::string& defaultVendor,
	const cl_device_type defaultDeviceType,
	const cl_int defaultDeviceId,
	std::vector<cl::Device>* deviceList,
	std::vector<CLHelper::DeviceInfo>* deviceInfoList)
{
	cl_int err;

	std::vector<cl::Platform> platforms;
	err = cl::Platform::get(&platforms);
	CHECK_OPENCL_ERROR(err, "cl::Platform::get failed");

	std::vector<cl::Platform>::iterator platform;
	for(platform = platforms.begin(); platform != platforms.end(); platform++)
	{
		std::string platformVendorString;
		platform->getInfo(CL_PLATFORM_VENDOR, &platformVendorString);

		if(defaultVendor.length() > 0 && platformVendorString.find(defaultVendor) == std::string::npos) {
			continue;
		}

		std::vector<cl::Device> devices;
		cl_int anyDevicesFound = platform->getDevices(defaultDeviceType, &devices);
		if(anyDevicesFound != CL_SUCCESS) continue;

		bool foundSpecificDevice = false;
		cl_int deviceId = 0;
		std::vector<cl::Device>::iterator device;
		for(device = devices.begin(); device != devices.end(); device++, deviceId++)
		{
			if(deviceId == defaultDeviceId) {
				CLHelper::DeviceInfo deviceInfo;
				deviceInfo.setDeviceInfo((*device)());

				deviceList->push_back((*device)());
				deviceInfoList->push_back(deviceInfo);
				
				// Fix (possibly) faulty vendor string
				if(defaultVendor.length() > 0) {
					delete deviceInfo.vendorName;
					size_t len = platformVendorString.length();
					deviceInfo.vendorName = new char[len+1];
					deviceInfo.vendorName[len] = 0;
					memcpy(deviceInfo.vendorName, platformVendorString.c_str(), len);
				}
				// End of fix

				foundSpecificDevice = true;
				break;
			}
		}
		if(foundSpecificDevice)
			break;
	}

	if(deviceList->empty()) {
		std::cerr << "No devices found which match the criteria. Exiting..." << std::endl;
		exit(1);
	}

	return CL_SUCCESS;
}

cl_int CLHelper::loadKernelFileToString(std::string relativeFilePath, std::string* source)
{
	std::string filePathString = (fs::current_path() / relativeFilePath).string();
	boost::algorithm::replace_all(filePathString, "\"", "");

	std::ifstream sourceFile(filePathString.c_str(), std::ifstream::in);
	if(!sourceFile.good()) {
		std::cerr << std::endl;
		std::cerr << "Unable to open file \"" << filePathString << "\"." << std::endl;
		exit(1);
	}

	*source = std::string((std::istreambuf_iterator<char>(sourceFile)),
	                       std::istreambuf_iterator<char>());

	return CL_SUCCESS;
}

cl_int CLHelper::compileProgram(
	cl::Program& program,
	std::vector<cl::Device>& devices,
	const char* options,
	void (CL_CALLBACK * notifyFptr)(cl_program, void *),
	void* data)
{
	cl_int err;

	err = program.build(devices, options, NULL, NULL);
	if(err != CL_SUCCESS) {
		std::cout << "Build error! Showing build log:" << std::endl << std::endl;

		std::string errorLog;
		std::vector<cl::Device>::iterator device;
		for(device = devices.begin(); device != devices.end(); device++)
		{
			errorLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(*device);
			std::cout << errorLog << std::endl;
		}
		CHECK_OPENCL_ERROR(err, "cl::Program::build() failed.");
	}

	return CL_SUCCESS;
}

cl_int CLHelper::printAllPlatformsAndDevices()
{
	cl_int err;

	std::vector<cl::Platform> platforms;
	err = cl::Platform::get(&platforms);
	CHECK_OPENCL_ERROR(err, "cl::Platform::get() failed.");

	streamsdk::SDKCommon sdkCommon;
	std::cout << std::endl;
	std::cout << "Listing platform vendors and devices" << std::endl;
	std::cout << "===========================================" << std::endl;

	std::vector<cl::Platform>::iterator platform;
	for(platform = platforms.begin(); platform != platforms.end(); platform++) {
		err = CLHelper::printVendor(*platform);
		CHECK_OPENCL_ERROR(err, "CLHelper::printVendor() failed.");

		err = CLHelper::printDevices(*platform, CL_DEVICE_TYPE_ALL);
		CHECK_OPENCL_ERROR(err, "CLHelper::printDevices() failed.");

		std::cout << "===========================================" << std::endl;
	}

	return CL_SUCCESS;
}

cl_int CLHelper::printVendor(cl::Platform platform)
{
    cl_int err;

    std::string vendorString;
    err = platform.getInfo(CL_PLATFORM_VENDOR, &vendorString);
    CHECK_OPENCL_ERROR(err, "cl::Platform::getInfo() failed.");

    std::cout << "Platform Vendor : " << vendorString << std::endl;

    return CL_SUCCESS;
}

cl_int CLHelper::printDevices(cl::Platform platform, cl_device_type deviceType)
{
    cl_int err;

    std::vector<cl::Device> deviceList;
    err = platform.getDevices(deviceType, &deviceList);
    CHECK_OPENCL_ERROR(err, "cl::Platform::getDevices() failed.");

    int deviceNum;
    std::vector<cl::Device>::iterator device;
    for(deviceNum = 0, device = deviceList.begin(); device != deviceList.end(); deviceNum++, device++)
    {
    	std::string deviceName;
        err = device->getInfo(CL_DEVICE_NAME, &deviceName);
        CHECK_OPENCL_ERROR(err, "cl::Device::getInfo() failed.");

        std::cout << "Device " << deviceNum << " : " << deviceName << std::endl;
    }

    return CL_SUCCESS;
}

void CLHelper::printDeviceInfoList(std::vector<CLHelper::DeviceInfo>& deviceInfoList)
{
	std::vector<CLHelper::DeviceInfo>::iterator deviceInfo;
	for(deviceInfo = deviceInfoList.begin(); deviceInfo != deviceInfoList.end(); deviceInfo++)
	{
		std::cout << deviceTypeToString(deviceInfo->dType) << ": " << std::string(deviceInfo->name) << std::endl;
		std::cout << std::endl;
	}
}

std::string CLHelper::deviceTypeToString(cl_device_type type) {
	switch(type) {
	case CL_DEVICE_TYPE_DEFAULT:
		return "DEFAULT";
	case CL_DEVICE_TYPE_GPU:
		return "GPU";
	case CL_DEVICE_TYPE_CPU:
		return "CPU";
	case CL_DEVICE_TYPE_ACCELERATOR:
		return "ACCELERATOR";
	case CL_DEVICE_TYPE_ALL:
		return "ALL";
	default: {
		std::cerr << "Invalid device type provided: " << type;
		exit(1);
	}
	}
}

cl_device_type CLHelper::deviceStringToType(std::string deviceString) {
	if(deviceString.find("DEFAULT") != std::string::npos)
		return CL_DEVICE_TYPE_DEFAULT;
	else if(deviceString.find("GPU") != std::string::npos)
		return CL_DEVICE_TYPE_GPU;
	else if(deviceString.find("CPU") != std::string::npos)
		return CL_DEVICE_TYPE_CPU;
	else if(deviceString.find("ACCELERATOR") != std::string::npos)
		return CL_DEVICE_TYPE_ACCELERATOR;
	else if(deviceString.find("ALL") != std::string::npos)
		return CL_DEVICE_TYPE_ALL;
	else {
		std::cerr << "Invalid device string provided: " << deviceString;
		exit(1);
	}
}

// Constructor
CLHelper::DeviceInfo::DeviceInfo() {
	dType = CL_DEVICE_TYPE_GPU;
	venderId = 0;
	maxComputeUnits = 0;
	maxWorkItemDims = 0;
	maxWorkItemSizes = NULL;
	maxWorkGroupSize = 0;
	preferredCharVecWidth = 0;
	preferredShortVecWidth = 0;
	preferredIntVecWidth = 0;
	preferredLongVecWidth = 0;
	preferredFloatVecWidth = 0;
	preferredDoubleVecWidth = 0;
	preferredHalfVecWidth = 0;
	nativeCharVecWidth = 0;
	nativeShortVecWidth = 0;
	nativeIntVecWidth = 0;
	nativeLongVecWidth = 0;
	nativeFloatVecWidth = 0;
	nativeDoubleVecWidth = 0;
	nativeHalfVecWidth = 0;
	maxClockFrequency = 0;
	addressBits = 0;
	maxMemAllocSize = 0;
	imageSupport = CL_FALSE;
	maxReadImageArgs = 0;
	maxWriteImageArgs = 0;
	image2dMaxWidth = 0;
	image2dMaxHeight = 0;
	image3dMaxWidth = 0;
	image3dMaxHeight = 0;
	image3dMaxDepth = 0;
	maxSamplers = 0;
	maxParameterSize = 0;
	memBaseAddressAlign = 0;
	minDataTypeAlignSize = 0;
	singleFpConfig = CL_FP_ROUND_TO_NEAREST | CL_FP_INF_NAN;
	doubleFpConfig = CL_FP_FMA |
					 CL_FP_ROUND_TO_NEAREST |
					 CL_FP_ROUND_TO_ZERO |
					 CL_FP_ROUND_TO_INF |
					 CL_FP_INF_NAN |
					 CL_FP_DENORM;
	globleMemCacheType = CL_NONE;
	globalMemCachelineSize = CL_NONE;
	globalMemCacheSize = 0;
	globalMemSize = 0;
	maxConstBufSize = 0;
	maxConstArgs = 0;
	localMemType = CL_LOCAL;
	localMemSize = 0;
	errCorrectionSupport = CL_FALSE;
	hostUnifiedMem = CL_FALSE;
	timerResolution = 0;
	endianLittle = CL_FALSE;
	available = CL_FALSE;
	compilerAvailable = CL_FALSE;
	execCapabilities = CL_EXEC_KERNEL;
	queueProperties = 0;
	platform = 0;
	name = NULL;
	vendorName = NULL;
	driverVersion = NULL;
	profileType = NULL;
	deviceVersion = NULL;
	openclCVersion = NULL;
	extensions = NULL;
}

// Copy constructor
CLHelper::DeviceInfo::DeviceInfo(const DeviceInfo &obj)
{
	assignAll(obj);
}

// Destructor
CLHelper::DeviceInfo::~DeviceInfo()
{
	deleteAll();
};

// Assignment operator
CLHelper::DeviceInfo& CLHelper::DeviceInfo::operator=(const DeviceInfo& obj)
{
	if(this != &obj)
	{
		deleteAll();
		assignAll(obj);
	}

	return *this;
}

cl_int CLHelper::DeviceInfo::setDeviceInfo(cl::Device device) {
    cl_int err = CL_SUCCESS;

    //Get device type
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_TYPE,
                    sizeof(cl_device_type),
                    &dType,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_TYPE) failed");

    //Get vender ID
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_VENDOR_ID,
                    sizeof(cl_uint),
                    &venderId,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_VENDOR_ID) failed");

    //Get max compute units
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_MAX_COMPUTE_UNITS,
                    sizeof(cl_uint),
                    &maxComputeUnits,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_MAX_COMPUTE_UNITS) failed");

    //Get max work item dimensions
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
                    sizeof(cl_uint),
                    &maxWorkItemDims,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS) failed");

    //Get max work item sizes
    delete maxWorkItemSizes;
    maxWorkItemSizes = new size_t[maxWorkItemDims];
    CHECK_ALLOCATION(maxWorkItemSizes, "Failed to allocate memory(maxWorkItemSizes)");

    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_MAX_WORK_ITEM_SIZES,
                    maxWorkItemDims * sizeof(size_t),
                    maxWorkItemSizes,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS) failed");

    // Maximum work group size
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_MAX_WORK_GROUP_SIZE,
                    sizeof(size_t),
                    &maxWorkGroupSize,
                    NULL);
   CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_MAX_WORK_GROUP_SIZE) failed");

    // Preferred vector sizes of all data types
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR,
                    sizeof(cl_uint),
                    &preferredCharVecWidth,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR) failed");

    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT,
                    sizeof(cl_uint),
                    &preferredShortVecWidth,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT) failed");

    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,
                    sizeof(cl_uint),
                    &preferredIntVecWidth,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT) failed");

    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,
                    sizeof(cl_uint),
                    &preferredLongVecWidth,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG) failed");

    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,
                    sizeof(cl_uint),
                    &preferredFloatVecWidth,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT) failed");

    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE,
                    sizeof(cl_uint),
                    &preferredDoubleVecWidth,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE) failed");

    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF,
                    sizeof(cl_uint),
                    &preferredHalfVecWidth,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF) failed");

    // Clock frequency
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_MAX_CLOCK_FREQUENCY,
                    sizeof(cl_uint),
                    &maxClockFrequency,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_MAX_CLOCK_FREQUENCY) failed");

    // Address bits
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_ADDRESS_BITS,
                    sizeof(cl_uint),
                    &addressBits,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_ADDRESS_BITS) failed");

    // Maximum memory alloc size
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                    sizeof(cl_ulong),
                    &maxMemAllocSize,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_MAX_MEM_ALLOC_SIZE) failed");

    // Image support
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_IMAGE_SUPPORT,
                    sizeof(cl_bool),
                    &imageSupport,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_IMAGE_SUPPORT) failed");

    // Maximum read image arguments
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_MAX_READ_IMAGE_ARGS,
                    sizeof(cl_uint),
                    &maxReadImageArgs,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_MAX_READ_IMAGE_ARGS) failed");

    // Maximum write image arguments
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_MAX_WRITE_IMAGE_ARGS,
                    sizeof(cl_uint),
                    &maxWriteImageArgs,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_MAX_WRITE_IMAGE_ARGS) failed");

    // 2D image and 3D dimensions
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_IMAGE2D_MAX_WIDTH,
                    sizeof(size_t),
                    &image2dMaxWidth,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_IMAGE2D_MAX_WIDTH) failed");

    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_IMAGE2D_MAX_HEIGHT,
                    sizeof(size_t),
                    &image2dMaxHeight,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_IMAGE2D_MAX_HEIGHT) failed");

    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_IMAGE3D_MAX_WIDTH,
                    sizeof(size_t),
                    &image3dMaxWidth,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_IMAGE3D_MAX_WIDTH) failed");

    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_IMAGE3D_MAX_HEIGHT,
                    sizeof(size_t),
                    &image3dMaxHeight,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_IMAGE3D_MAX_HEIGHT) failed");

    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_IMAGE3D_MAX_DEPTH,
                    sizeof(size_t),
                    &image3dMaxDepth,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_IMAGE3D_MAX_DEPTH) failed");

    // Maximum samplers
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_MAX_SAMPLERS,
                    sizeof(cl_uint),
                    &maxSamplers,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_MAX_SAMPLERS) failed");

    // Maximum parameter size
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_MAX_PARAMETER_SIZE,
                    sizeof(size_t),
                    &maxParameterSize,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_MAX_PARAMETER_SIZE) failed");

    // Memory base address align
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_MEM_BASE_ADDR_ALIGN,
                    sizeof(cl_uint),
                    &memBaseAddressAlign,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_MEM_BASE_ADDR_ALIGN) failed");

    // Minimum data type align size
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE,
                    sizeof(cl_uint),
                    &minDataTypeAlignSize,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE) failed");

    // Single precision floating point configuration
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_SINGLE_FP_CONFIG,
                    sizeof(cl_device_fp_config),
                    &singleFpConfig,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_SINGLE_FP_CONFIG) failed");

    // Double precision floating point configuration
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_DOUBLE_FP_CONFIG,
                    sizeof(cl_device_fp_config),
                    &doubleFpConfig,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_DOUBLE_FP_CONFIG) failed");

    // Global memory cache type
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_GLOBAL_MEM_CACHE_TYPE,
                    sizeof(cl_device_mem_cache_type),
                    &globleMemCacheType,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE) failed");

    // Global memory cache line size
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,
                    sizeof(cl_uint),
                    &globalMemCachelineSize,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE) failed");

    // Global memory cache size
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,
                    sizeof(cl_ulong),
                    &globalMemCacheSize,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE) failed");

    // Global memory size
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_GLOBAL_MEM_SIZE,
                    sizeof(cl_ulong),
                    &globalMemSize,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_GLOBAL_MEM_SIZE) failed");

    // Maximum constant buffer size
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,
                    sizeof(cl_ulong),
                    &maxConstBufSize,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE) failed");

    // Maximum constant arguments
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_MAX_CONSTANT_ARGS,
                    sizeof(cl_uint),
                    &maxConstArgs,
                    NULL);
   CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_MAX_CONSTANT_ARGS) failed");

    // Local memory type
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_LOCAL_MEM_TYPE,
                    sizeof(cl_device_local_mem_type),
                    &localMemType,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_LOCAL_MEM_TYPE) failed");

    // Local memory size
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_LOCAL_MEM_SIZE,
                    sizeof(cl_ulong),
                    &localMemSize,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_LOCAL_MEM_SIZE) failed");

    // Error correction support
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_ERROR_CORRECTION_SUPPORT,
                    sizeof(cl_bool),
                    &errCorrectionSupport,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_ERROR_CORRECTION_SUPPORT) failed");

    // Profiling timer resolution
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_PROFILING_TIMER_RESOLUTION,
                    sizeof(size_t),
                    &timerResolution,
                    NULL);
   CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_PROFILING_TIMER_RESOLUTION) failed");

    // Endian little
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_ENDIAN_LITTLE,
                    sizeof(cl_bool),
                    &endianLittle,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_ENDIAN_LITTLE) failed");

    // Device available
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_AVAILABLE,
                    sizeof(cl_bool),
                    &available,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_AVAILABLE) failed");

    // Device compiler available
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_COMPILER_AVAILABLE,
                    sizeof(cl_bool),
                    &compilerAvailable,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_COMPILER_AVAILABLE) failed");

    // Device execution capabilities
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_EXECUTION_CAPABILITIES,
                    sizeof(cl_device_exec_capabilities),
                    &execCapabilities,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_EXECUTION_CAPABILITIES) failed");

    // Device queue properities
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_QUEUE_PROPERTIES,
                    sizeof(cl_command_queue_properties),
                    &queueProperties,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_QUEUE_PROPERTIES) failed");

    // Platform
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_PLATFORM,
                    sizeof(cl_platform_id),
                    &platform,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_PLATFORM) failed");

    // Device name
    size_t tempSize = 0;
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_NAME,
                    0,
                    NULL,
                    &tempSize);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_NAME) failed");

    delete name;
    name = new char[tempSize];
    CHECK_ALLOCATION(name, "Failed to allocate memory(name)");

    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_NAME,
                    sizeof(char) * tempSize,
                    name,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_NAME) failed");

    // Vender name
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_VENDOR,
                    0,
                    NULL,
                    &tempSize);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_VENDOR) failed");

    delete vendorName;
    vendorName = new char[tempSize];
    CHECK_ALLOCATION(vendorName, "Failed to allocate memory(venderName)");

    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_VENDOR,
                    sizeof(char) * tempSize,
                    vendorName,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_VENDOR) failed");

    // Driver name
    err = clGetDeviceInfo(
                    device(),
                    CL_DRIVER_VERSION,
                    0,
                    NULL,
                    &tempSize);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DRIVER_VERSION) failed");

    delete driverVersion;
    driverVersion = new char[tempSize];
    CHECK_ALLOCATION(driverVersion, "Failed to allocate memory(driverVersion)");

    err = clGetDeviceInfo(
                    device(),
                    CL_DRIVER_VERSION,
                    sizeof(char) * tempSize,
                    driverVersion,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DRIVER_VERSION) failed");

    // Device profile
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_PROFILE,
                    0,
                    NULL,
                    &tempSize);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_PROFILE) failed");

    delete profileType;
    profileType = new char[tempSize];
    CHECK_ALLOCATION(profileType, "Failed to allocate memory(profileType)");

    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_PROFILE,
                    sizeof(char) * tempSize,
                    profileType,
                    NULL);
   CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_PROFILE) failed");

    // Device version
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_VERSION,
                    0,
                    NULL,
                    &tempSize);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_VERSION) failed");

    delete deviceVersion;
    deviceVersion = new char[tempSize];
    CHECK_ALLOCATION(deviceVersion, "Failed to allocate memory(deviceVersion)");

    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_VERSION,
                    sizeof(char) * tempSize,
                    deviceVersion,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_VERSION) failed");

    // Device extensions
    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_EXTENSIONS,
                    0,
                    NULL,
                    &tempSize);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_EXTENSIONS) failed");

    delete extensions;
    extensions = new char[tempSize];
    CHECK_ALLOCATION(extensions, "Failed to allocate memory(extensions)");

    err = clGetDeviceInfo(
                    device(),
                    CL_DEVICE_EXTENSIONS,
                    sizeof(char) * tempSize,
                    extensions,
                    NULL);
    CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_EXTENSIONS) failed");

    // Device parameters of OpenCL 1.1 Specification
#ifdef CL_VERSION_1_1
    std::string deviceVerStr(deviceVersion);
    size_t vStart = deviceVerStr.find(" ", 0);
    size_t vEnd = deviceVerStr.find(" ", vStart + 1);
    std::string vStrVal = deviceVerStr.substr(vStart + 1, vEnd - vStart - 1);
    if(vStrVal.compare("1.0") > 0)
    {
        // Native vector sizes of all data types
        err = clGetDeviceInfo(
                        device(),
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR,
                        sizeof(cl_uint),
                        &nativeCharVecWidth,
                        NULL);
        CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR) failed");

        err = clGetDeviceInfo(
                        device(),
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT,
                        sizeof(cl_uint),
                        &nativeShortVecWidth,
                        NULL);
        CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT) failed");

        err = clGetDeviceInfo(
                        device(),
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_INT,
                        sizeof(cl_uint),
                        &nativeIntVecWidth,
                        NULL);
        CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_NATIVE_VECTOR_WIDTH_INT) failed");

        err = clGetDeviceInfo(
                        device(),
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG,
                        sizeof(cl_uint),
                        &nativeLongVecWidth,
                        NULL);
        CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG) failed");

        err = clGetDeviceInfo(
                        device(),
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT,
                        sizeof(cl_uint),
                        &nativeFloatVecWidth,
                        NULL);
        CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT) failed");

        err = clGetDeviceInfo(
                        device(),
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE,
                        sizeof(cl_uint),
                        &nativeDoubleVecWidth,
                        NULL);
        CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE) failed");

        err = clGetDeviceInfo(
                        device(),
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF,
                        sizeof(cl_uint),
                        &nativeHalfVecWidth,
                        NULL);
        CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF) failed");

        // Host unified memory
        err = clGetDeviceInfo(
                        device(),
                        CL_DEVICE_HOST_UNIFIED_MEMORY,
                        sizeof(cl_bool),
                        &hostUnifiedMem,
                        NULL);
        CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_HOST_UNIFIED_MEMORY) failed");

        // Device OpenCL C version
        err = clGetDeviceInfo(
                        device(),
                        CL_DEVICE_OPENCL_C_VERSION,
                        0,
                        NULL,
                        &tempSize);
        CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_OPENCL_C_VERSION) failed");

        delete openclCVersion;
        openclCVersion = new char[tempSize];
        CHECK_ALLOCATION(openclCVersion, "Failed to allocate memory(openclCVersion)");

        err = clGetDeviceInfo(
                        device(),
                        CL_DEVICE_OPENCL_C_VERSION,
                        sizeof(char) * tempSize,
                        openclCVersion,
                        NULL);
        CHECK_OPENCL_ERROR(err, "clGetDeviceIDs(CL_DEVICE_OPENCL_C_VERSION) failed");
    }
#endif

    return SDK_SUCCESS;
}

void CLHelper::DeviceInfo::deleteAll()
{
	delete maxWorkItemSizes;
	delete name;
	delete vendorName;
	delete driverVersion;
	delete profileType;
	delete deviceVersion;
	delete openclCVersion;
	delete extensions;
}

void CLHelper::DeviceInfo::assignAll(const DeviceInfo& obj)
{
	dType = obj.dType;
	venderId = obj.venderId;
	maxComputeUnits = obj.maxComputeUnits;
	maxWorkItemDims = obj.maxWorkItemDims;

	maxWorkItemSizes = new size_t[obj.maxWorkItemDims];
	for(cl_uint i = 0; i < obj.maxWorkItemDims; i++) {
		maxWorkItemSizes[i] = obj.maxWorkItemSizes[i];
	}

	maxWorkGroupSize = obj.maxWorkGroupSize;
	preferredCharVecWidth = obj.preferredCharVecWidth;
	preferredShortVecWidth = obj.preferredShortVecWidth;
	preferredIntVecWidth = obj.preferredIntVecWidth;
	preferredLongVecWidth = obj.preferredLongVecWidth;
	preferredFloatVecWidth = obj.preferredFloatVecWidth;
	preferredDoubleVecWidth = obj.preferredDoubleVecWidth;
	preferredHalfVecWidth = obj.preferredHalfVecWidth;
	nativeCharVecWidth = obj.nativeCharVecWidth;
	nativeShortVecWidth = obj.nativeShortVecWidth;
	nativeIntVecWidth = obj.nativeIntVecWidth;
	nativeLongVecWidth = obj.nativeLongVecWidth;
	nativeFloatVecWidth = obj.nativeFloatVecWidth;
	nativeDoubleVecWidth = obj.nativeDoubleVecWidth;
	nativeHalfVecWidth = obj.nativeHalfVecWidth;
	maxClockFrequency = obj.maxClockFrequency;
	addressBits = obj.addressBits;
	maxMemAllocSize = obj.maxMemAllocSize;
	imageSupport = obj.imageSupport;
	maxReadImageArgs = obj.maxReadImageArgs;
	maxWriteImageArgs = obj.maxWriteImageArgs;
	image2dMaxWidth = obj.image2dMaxWidth;
	image2dMaxHeight = obj.image2dMaxHeight;
	image3dMaxWidth = obj.image3dMaxWidth;
	image3dMaxHeight = obj.image3dMaxHeight;
	image3dMaxDepth = obj.image3dMaxDepth;
	maxSamplers = obj.maxSamplers;
	maxParameterSize = obj.maxParameterSize;
	memBaseAddressAlign = obj.memBaseAddressAlign;
	minDataTypeAlignSize = obj.minDataTypeAlignSize;
	singleFpConfig = obj.singleFpConfig;
	doubleFpConfig = obj.doubleFpConfig;
	globleMemCacheType = obj.globleMemCacheType;
	globalMemCachelineSize = obj.globalMemCachelineSize;
	globalMemCacheSize = obj.globalMemCacheSize;
	globalMemSize = obj.globalMemSize;
	maxConstBufSize = obj.maxConstBufSize;
	maxConstArgs = obj.maxConstArgs;
	localMemType = obj.localMemType;
	localMemSize = obj.localMemSize;
	errCorrectionSupport = obj.errCorrectionSupport;
	hostUnifiedMem = obj.hostUnifiedMem;
	timerResolution = obj.timerResolution;
	endianLittle = obj.endianLittle;
	available = obj.available;
	compilerAvailable = obj.compilerAvailable;
	execCapabilities = obj.execCapabilities;
	queueProperties = obj.queueProperties;
	platform = obj.platform;

	name = new char[strlen(obj.name) + 1];
	strcpy(name, obj.name);

	vendorName = new char[strlen(obj.vendorName) + 1];
	strcpy(vendorName, obj.vendorName);

	driverVersion = new char[strlen(obj.driverVersion) + 1];
	strcpy(driverVersion, obj.driverVersion);

	profileType = new char[strlen(obj.profileType) + 1];
	strcpy(profileType, obj.profileType);

	deviceVersion = new char[strlen(obj.deviceVersion) + 1];
	strcpy(deviceVersion, obj.deviceVersion);

	openclCVersion = new char[strlen(obj.openclCVersion) + 1];
	strcpy(openclCVersion, obj.openclCVersion);

	extensions = new char[strlen(obj.extensions) + 1];
	strcpy(extensions, obj.extensions);
}
