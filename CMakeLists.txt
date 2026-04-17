cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)
set(PLATFORM_NAME "")
set(LIBS HTTP JSON SocketStreams crypto ssl)
set(LIBRARY_TYPE STATIC)
option(BUILD_SHARED_LIBS "" OFF)
option(SHARED_NETWORKS "Build shared Networks with static linked dependencies" OFF)

project(Networks VERSION 1.26.0)

if (UNIX)
	add_definitions(-D__LINUX__)

	set(CMAKE_POSITION_INDEPENDENT_CODE ON)

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
		add_definitions(-D__ANDROID__)

		set(PLATFORM_NAME Android)
	else()
		if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
			set(PLATFORM_NAME LinuxARM)
		else()
			set(PLATFORM_NAME Linux)
		endif()
	endif()
else()
	set(PLATFORM_NAME Windows)
	set(LIBS ${LIBS} crypt32)
endif()

if (${BUILD_SHARED_LIBS} OR ${SHARED_NETWORKS})
	add_definitions(-DNETWORKS_DLL)
	set(LIBRARY_TYPE SHARED)
endif ()

add_library(
	${PROJECT_NAME} ${LIBRARY_TYPE}
	src/HttpNetwork.cpp
	src/HttpsNetwork.cpp
	src/NetworksUtility.cpp
	src/Exceptions/SslException.cpp
	src/LargeBodyHandler.cpp
)

if (DEFINED ENV{MARCH} AND NOT "$ENV{MARCH}" STREQUAL "")
	target_compile_options(${PROJECT_NAME} PRIVATE -march=$ENV{MARCH})
endif()

if (NOT TARGET HTTP)
	add_subdirectory(HTTP)
endif()

if (NOT TARGET SocketStreams)
	add_subdirectory(SocketStreams)
endif()

target_include_directories(
	${PROJECT_NAME} PUBLIC
	include
	vendor/OpenSSL/${PLATFORM_NAME}/include
)

target_link_directories(
	${PROJECT_NAME} PUBLIC
	vendor/OpenSSL/${PLATFORM_NAME}/lib
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})

install(
	TARGETS ${PROJECT_NAME} 
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION dll
)

install(DIRECTORY include DESTINATION .)
install(DIRECTORY vendor/OpenSSL/${PLATFORM_NAME}/lib/ DESTINATION lib)
install(DIRECTORY vendor/OpenSSL/${PLATFORM_NAME}/include/ DESTINATION include)
install(DIRECTORY vendor/OpenSSL/license/ DESTINATION license)
