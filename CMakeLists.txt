cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 17)
option(BUILD_SHARED_LIBS "" OFF)

if (UNIX)
	add_definitions(-D__LINUX__)

	set(VENDOR_LIBRARY_PATH vendor/Linux/Release-x64)
else ()
	set(VENDOR_LIBRARY_PATH vendor/Windows/Release-x64)
endif ()

if (${BUILD_SHARED_LIBS} STREQUAL "ON")
	add_definitions(-DNETWORKS_DLL)
endif ()

project(Networks)

add_library(
	${PROJECT_NAME}
	src/HTTPNetwork.cpp
	src/HTTPSNetwork.cpp
	src/Exceptions/SSLException.cpp
)

target_include_directories(
	${PROJECT_NAME} PUBLIC
	HTTP/src
	HTTP/JSON/src
	SocketStreams/src
	vendor/OpenSSL/include
)

target_link_directories(
	${PROJECT_NAME} PUBLIC
	${VENDOR_LIBRARY_PATH}
)

target_link_libraries(
	${PROJECT_NAME} PUBLIC
	HTTP
	crypto
	ssl
)

add_subdirectory(HTTP)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR}/bin)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
	install(DIRECTORY src/ DESTINATION ${CMAKE_BINARY_DIR}/bin/include FILES_MATCHING PATTERN "*.h")
endif ()
