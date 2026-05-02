cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 23)
set(PLATFORM_NAME "")
set(LIBS HTTP JSON SocketStreams crypto ssl)

project(Networks VERSION 1.28.1)

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

add_library(
	${PROJECT_NAME} ${LIBRARY_TYPE}
	src/Http/HttpNetwork.cpp
	src/Http/HttpsNetwork.cpp
	src/WebSocket/WsNetwork.cpp
	src/WebSocket/WssNetwork.cpp
	src/WebSocket/Frame.cpp
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

target_link_libraries(
	${PROJECT_NAME} PUBLIC 
	${LIBS}
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)

install(DIRECTORY include DESTINATION .)
install(DIRECTORY vendor/OpenSSL/${PLATFORM_NAME}/lib/ DESTINATION lib)
install(DIRECTORY vendor/OpenSSL/${PLATFORM_NAME}/include/ DESTINATION include)
install(DIRECTORY vendor/OpenSSL/license/ DESTINATION license)
