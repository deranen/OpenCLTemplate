#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "CLHelper.h"

namespace fs = boost::filesystem;

cl_int CLHelper::findSpecifiedDevice(
	const std::string& defaultVendor,
	const cl_device_type defaultDeviceType,
	const cl_int defaultDeviceId,
	std::vector<cl::Device>* deviceList,
	std::vector<streamsdk::SDKDeviceInfo>* deviceInfoList)
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
				streamsdk::SDKDeviceInfo deviceInfo;
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
	CHECK_OPENCL_ERROR(err, "cl::Platform::get failed");

	streamsdk::SDKCommon sdkCommon;
	std::cout << std::endl;
	std::cout << "Listing platform vendors and devices" << std::endl;
	std::cout << "************************************" << std::endl;

	std::vector<cl::Platform>::iterator platform;
	for(platform = platforms.begin(); platform != platforms.end(); platform++) {
		sdkCommon.displayDevices((*platform)(), CL_DEVICE_TYPE_ALL);
		std::cout << "************************************" << std::endl;
	}

	return CL_SUCCESS;
}

void CLHelper::printDeviceInfoList(std::vector<streamsdk::SDKDeviceInfo>& deviceInfoList)
{
	std::vector<streamsdk::SDKDeviceInfo>::iterator deviceInfo;
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
