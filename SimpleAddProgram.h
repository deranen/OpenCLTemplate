#pragma once

#include "CLHelper.h"

#define DATA_SIZE 1024

typedef float DataType;

cl_int runSimpleAddProgramProgram(std::vector<cl::Device>& deviceList, std::vector<streamsdk::SDKDeviceInfo>& deviceInfoList);
