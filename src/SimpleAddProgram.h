#ifndef _SIMPLEADDPROGRAM_H
#define _SIMPLEADDPROGRAM_H

#include "CLHelper.h"

#define DATA_SIZE 1024

typedef float DataType;

cl_int runSimpleAddProgram(std::vector<cl::Device>& deviceList, std::vector<streamsdk::SDKDeviceInfo>& deviceInfoList);

#endif
