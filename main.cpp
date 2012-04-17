#include <CL/cl.hpp>

#include <SDKCommandArgs.hpp>
#include "CLHelper.h"

#include "SimpleAddProgram.h"

// Object that handles command arguments
streamsdk::SDKCommandArgs commandArgs(0, NULL);

// Default values
std::string defaultVendor = "";
cl_device_type defaultDeviceType = CL_DEVICE_TYPE_DEFAULT;
std::string defaultDeviceTypeString = "DEFAULT";
cl_int defaultDeviceId = 0;

// Command line options
streamsdk::Option options[] = {{ "d", "device", "The device ID to use as default.", streamsdk::CA_ARG_INT, &defaultDeviceId },
                               { "t", "device-type", "The device type to use as default. ('GPU', 'CPU' or 'ALL')", streamsdk::CA_ARG_STRING, &defaultDeviceTypeString },
                               { "v", "vendor", "The vendor to use as default. (Examples: 'AMD', 'Intel')", streamsdk::CA_ARG_STRING, &defaultVendor },
                               { "h", "help", "Print this.", streamsdk::CA_NO_ARGUMENT, NULL }};

int main(int argc, char **argv) {

	// Add all options to SDKCommandArgs object
	commandArgs.AddOption(&options[0]);
	commandArgs.AddOption(&options[1]);
	commandArgs.AddOption(&options[2]);
	commandArgs.AddOption(&options[3]);
	// Parse arguments
	commandArgs.parse(++argv, --argc);

	if(commandArgs.isArgSet("h", true) || commandArgs.isArgSet("help", false)) {
		commandArgs.help();
		exit(0);
	}

	// Print all platforms and devices
	CLHelper::printAllPlatformsAndDevices();

	// Find specified devices and store them in 'deviceList' and related device info in 'deviceInfoList'
	std::vector<cl::Device> deviceList;
	std::vector<streamsdk::SDKDeviceInfo> deviceInfoList;
	CLHelper::findSpecifiedDevice(defaultVendor, defaultDeviceType, defaultDeviceId, &deviceList, &deviceInfoList);

	// Print selected devices
	std::cout << std::endl;
	std::cout << "Selected devices:" << std::endl;
	CLHelper::printDeviceInfoList(deviceInfoList);

	// Call specific OpenCL program with 'deviceList' and optionally 'deviceInfoList' as parameter
	runSimpleAddProgram(deviceList, deviceInfoList);

	return 0;
}