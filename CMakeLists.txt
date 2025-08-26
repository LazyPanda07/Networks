cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)
set(OPENSSL_LIB_PATH vendor/OpenSSL/lib)
set(LIBS HTTP SocketStreams crypto ssl)
set(LIBRARY_TYPE STATIC)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install-dir CACHE STRING "")
option(BUILD_SHARED_LIBS "" OFF)
option(SHARED_NETWORKS "Build shared Networks with static linked dependencies" OFF)
option(WITH_NETWORKS_LOGGING "Enable logging via std::clog" OFF)

project(Networks VERSION 1.20.1)

if (${WITH_NETWORKS_LOGGING})
	add_definitions(-DNETWORKS_LOGGING)
endif()

if (UNIX)
	add_definitions(-D__LINUX__)

	set(CMAKE_POSITION_INDEPENDENT_CODE ON)

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
		add_definitions(-D__ANDROID__)

		set(OPENSSL_LIB_PATH ${OPENSSL_LIB_PATH}/Android)
	else()
		if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
			set(OPENSSL_LIB_PATH ${OPENSSL_LIB_PATH}/LinuxARM)
		else()
			set(OPENSSL_LIB_PATH ${OPENSSL_LIB_PATH}/Linux)
		endif()
	endif()

	install(DIRECTORY ${OPENSSL_LIB_PATH}/ DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/vendor/OpenSSL FILES_MATCHING PATTERN "*.a")
else ()
	set(OPENSSL_LIB_PATH ${OPENSSL_LIB_PATH}/Windows)
	set(LIBS ${LIBS} crypt32)

	install(DIRECTORY ${OPENSSL_LIB_PATH}/ DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/vendor/OpenSSL FILES_MATCHING PATTERN "*.lib")
endif ()

if (${BUILD_SHARED_LIBS} OR ${SHARED_NETWORKS})
	add_definitions(-DNETWORKS_DLL)
	set(LIBRARY_TYPE SHARED)
endif ()

add_library(
	${PROJECT_NAME} ${LIBRARY_TYPE}
	src/HTTPNetwork.cpp
	src/HTTPSNetwork.cpp
	src/NetworksUtility.cpp
	src/Exceptions/SSLException.cpp
	src/LargeBodyHandler.cpp
)

if (DEFINED ENV{MARCH} AND NOT "$ENV{MARCH}" STREQUAL "")
	target_compile_options(${PROJECT_NAME} PRIVATE -march=$ENV{MARCH})
endif()

target_include_directories(
	${PROJECT_NAME} PUBLIC
	src/
	HTTP/src
	HTTP/JSON/src
	SocketStreams/src
	vendor/OpenSSL/include
)

target_link_directories(
	${PROJECT_NAME} PUBLIC
	${OPENSSL_LIB_PATH}
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})

add_subdirectory(HTTP)
add_subdirectory(SocketStreams)

install(
	TARGETS ${PROJECT_NAME} 
	ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/dll
)

install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include FILES_MATCHING PATTERN "*.h")
install(DIRECTORY vendor/OpenSSL/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/vendor/OpenSSL/)
