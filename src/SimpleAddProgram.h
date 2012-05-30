#ifndef _SIMPLEADDPROGRAM_H
#define _SIMPLEADDPROGRAM_H

#include "CLHelper.h"

#define DATA_SIZE 1048576

typedef float DataType;

cl_int runSimpleAddProgram(std::vector<cl::Device>& deviceList, std::vector<CLHelper::DeviceInfo>& deviceInfoList);

#endif
