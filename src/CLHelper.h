#ifndef _CLHELPER_H
#define _CLHELPER_H

#include <CL/cl.hpp>
#include <SDKCommon.hpp>
#include <string>

namespace CLHelper
{
	class DeviceInfo {

	public:
		cl_device_type dType;				/* dType device type */
		cl_uint venderId;					/* vendorId VendorId of device */
		cl_uint maxComputeUnits;			/* maxComputeUnits maxComputeUnits of device */
		cl_uint maxWorkItemDims;			/* maxWorkItemDims maxWorkItemDimensions VendorId of device */
		size_t* maxWorkItemSizes;			/* maxWorkItemSizes maxWorkItemSizes of device */
		size_t maxWorkGroupSize;			/* maxWorkGroupSize max WorkGroup Size of device */
		cl_uint preferredCharVecWidth;		/* preferredCharVecWidth preferred Char VecWidth of device */
		cl_uint preferredShortVecWidth;		/* preferredShortVecWidth preferred Short VecWidth of device */
		cl_uint preferredIntVecWidth;		/* preferredIntVecWidth preferred Int VecWidth of device */
		cl_uint preferredLongVecWidth;		/* preferredLongVecWidth preferred Long VecWidth of device */
		cl_uint preferredFloatVecWidth;		/* preferredFloatVecWidth preferredFloatVecWidth of device */
		cl_uint preferredDoubleVecWidth;	/* preferredDoubleVecWidth preferred Double VecWidth of device */
		cl_uint preferredHalfVecWidth;		/* preferredHalfVecWidth preferred Half VecWidth of device */
		cl_uint nativeCharVecWidth;		 	/* nativeCharVecWidth native Char VecWidth of device */
		cl_uint nativeShortVecWidth;		/* nativeShortVecWidth nativeShortVecWidth of device */
		cl_uint nativeIntVecWidth;			/* nativeIntVecWidth nativeIntVecWidth of device */
		cl_uint nativeLongVecWidth;			/* nativeLongVecWidth nativeLongVecWidth of device */
		cl_uint nativeFloatVecWidth;		/* nativeFloatVecWidth native Float VecWidth of device */
		cl_uint nativeDoubleVecWidth;		/* nativeDoubleVecWidth native Double VecWidth of device */
		cl_uint nativeHalfVecWidth;			/* nativeHalfVecWidth native Half VecWidth of device */
		cl_uint maxClockFrequency;			/* maxClockFrequency max Clock Frequency of device */
		cl_uint addressBits;				/* addressBits address Bits of device */
		cl_ulong maxMemAllocSize;			/* maxMemAllocSize max Mem Alloc Size of device */
		cl_bool imageSupport;				/* imageSupport image Support of device */
		cl_uint maxReadImageArgs;			/* maxReadImageArgs max ReadImage Args of device */
		cl_uint maxWriteImageArgs;			/* maxWriteImageArgs max Write Image Args of device */
		size_t image2dMaxWidth;				/* image2dMaxWidth image 2dMax Width of device */
		size_t image2dMaxHeight;			/* image2dMaxHeight image 2dMax Height of device */
		size_t image3dMaxWidth;				/* image3dMaxWidth image3d MaxWidth of device */
		size_t image3dMaxHeight;			/* image3dMaxHeight image 3dMax Height of device */
		size_t image3dMaxDepth;				/* image3dMaxDepth image 3dMax Depth of device */
		size_t maxSamplers;					/* maxSamplers maxSamplers of device */
		size_t maxParameterSize;			/* maxParameterSize maxParameterSize of device */
		cl_uint memBaseAddressAlign;		/* memBaseAddressAlign memBase AddressAlign of device */
		cl_uint minDataTypeAlignSize;		/* minDataTypeAlignSize minDataType AlignSize of device */
		cl_device_fp_config singleFpConfig;	/* singleFpConfig singleFpConfig of device */
		cl_device_fp_config doubleFpConfig; /* doubleFpConfig doubleFpConfig of device */
		cl_device_mem_cache_type globleMemCacheType; /* globleMemCacheType globleMem CacheType of device */
		cl_uint globalMemCachelineSize;		/* globalMemCachelineSize globalMem Cacheline Size of device */
		cl_ulong globalMemCacheSize;		/* globalMemCacheSize globalMem CacheSize of device */
		cl_ulong globalMemSize;				/* globalMemSize globalMem Size of device */
		cl_ulong maxConstBufSize;			/* maxConstBufSize maxConst BufSize of device */
		cl_uint maxConstArgs;				/* maxConstArgs max ConstArgs of device */
		cl_device_local_mem_type localMemType;/* localMemType local MemType of device */
		cl_ulong localMemSize;				/* localMemSize localMem Size of device */
		cl_bool errCorrectionSupport;		/* errCorrectionSupport errCorrectionSupport of device */
		cl_bool hostUnifiedMem;				/* hostUnifiedMem hostUnifiedMem of device */
		size_t timerResolution;				/* timerResolution timerResolution of device */
		cl_bool endianLittle;				/* endianLittle endian Little of device */
		cl_bool available;					/* available available of device */
		cl_bool compilerAvailable;			/* compilerAvailable compilerAvailable of device */
		cl_device_exec_capabilities execCapabilities;/* execCapabilities exec Capabilities of device */
		cl_command_queue_properties queueProperties;/* queueProperties queueProperties of device */
		cl_platform_id platform;			/* platform platform of device */
		char* name;							/* name name of device */
		char* vendorName;					/* venderName vender Name of device */
		char* driverVersion;				/* driverVersion driver Version of device */
		char* profileType;					/* profileType profile Type of device */
		char* deviceVersion;				/* deviceVersion device Version of device */
		char* openclCVersion;				/* openclCVersion opencl C Version of device */
		char* extensions;					/* extensions extensions of device */

		DeviceInfo();
		DeviceInfo(const DeviceInfo &obj);
		~DeviceInfo();

		DeviceInfo& operator=(const DeviceInfo& obj);

		cl_int setDeviceInfo(cl::Device device);
	private:
		void deleteAll();
		void assignAll(const DeviceInfo& obj);

	};

	cl_int findSpecifiedDevices(
		const std::string& defaultVendor,
		const cl_device_type defaultDeviceType,
		const cl_int defaultDeviceId,
		std::vector<cl::Device>* deviceList,
		std::vector<DeviceInfo>* deviceInfoList);

	cl_int loadKernelFileToString(std::string relativeFilePath, std::string* source);

	cl_int compileProgram(
		cl::Program& program,
		std::vector<cl::Device>& devices,
		const char* options = NULL,
		void (CL_CALLBACK * notifyFptr)(cl_program, void *) = NULL,
		void* data = NULL);

	cl_int printVendor(cl::Platform platform);
	cl_int printDevices(cl::Platform platform, cl_device_type deviceType);
	cl_int printAllPlatformsAndDevices();

	void printDeviceInfoList(std::vector<DeviceInfo>& deviceInfoList);

	std::string deviceTypeToString(cl_device_type type);
	cl_device_type deviceStringToType(std::string deviceString);
};

#endif
