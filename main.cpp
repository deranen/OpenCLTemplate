#include <CL/cl.hpp>
#include <boost/program_options.hpp>

#include "CLHelper.h"
#include "SimpleAddProgram.h"

namespace po = boost::program_options;

int main(int argc, char **argv) {

	std::string defaultVendor, defaultDeviceTypeString;
	cl_device_type defaultDeviceType;
	cl_int defaultDeviceId;

// Specify options
	po::options_description desc("Allowed options");
	desc.add_options()
		("device,d",
			po::value<cl_int>(&defaultDeviceId)->default_value(0),
			"The device ID to use as default.")
		("device-type,t",
			po::value<std::string>(&defaultDeviceTypeString)->default_value("DEFAULT"),
			"The device type to use as default. ('GPU', 'CPU' or 'ALL')")
		("vendor,v",
			po::value<std::string>(&defaultVendor)->default_value(""),
			"The vendor to use as default. (Examples: 'AMD', 'Intel')")
		("help", "Print this.");


// Parse the command line
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

// Display help if requested
	if(vm.count("help")) {
		std::cout << desc << std::endl;
		exit(1);
	}

// Modify "AMD" string to correct one
	if(defaultVendor.compare("AMD") == 0) {
		defaultVendor = "Advanced Micro Devices, Inc.";
	}

// Convert device type string to type 'cl_device_type'
	defaultDeviceType = CLHelper::deviceStringToType(defaultDeviceTypeString);

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
