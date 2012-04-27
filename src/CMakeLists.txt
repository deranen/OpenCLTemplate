cmake_minimum_required(VERSION 2.8)

include_directories(. /opt/AMDAPP/include)

add_executable(main
	CLHelper.cpp
	CLHelper.h
	CMakeLists.txt
	SDKCommandArgs.cpp
	SDKCommandArgs.hpp
	SDKCommon.cpp
	SDKCommon.hpp
	SDKFile.cpp
	SDKFile.hpp
	SimpleAddKernel.cl
	SimpleAddProgram.cpp
	SimpleAddProgram.h
	main.cpp
)

target_link_libraries(main
	OpenCL
	boost_program_options
)

file(COPY SimpleAddKernel.cl DESTINATION ./)
