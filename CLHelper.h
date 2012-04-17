#pragma once

#include <CL/cl.hpp>
#include <SDKCommon.hpp>
#include <string>

class CLHelper
{
public:
static cl_int CLHelper::findSpecifiedDevice(
	const std::string& defaultVendor,
	const cl_device_type defaultDeviceType,
	const cl_int defaultDeviceId,
	std::vector<cl::Device>* deviceList,
	std::vector<streamsdk::SDKDeviceInfo>* deviceInfoList);

static cl_int loadKernelFileToString(std::string relativeFilePath, std::string* source);

static cl_int compileProgram(
	cl::Program& program,
	std::vector<cl::Device>& devices,
	const char* options = NULL,
	void (CL_CALLBACK * notifyFptr)(cl_program, void *) = NULL,
	void* data = NULL);

static cl_int printAllPlatformsAndDevices();

static void CLHelper::printDeviceInfoList(std::vector<streamsdk::SDKDeviceInfo>& deviceInfoList);

static std::string CLHelper::deviceTypeToString(cl_device_type type);

static cl_device_type CLHelper::deviceStringToType(std::string deviceString);
};