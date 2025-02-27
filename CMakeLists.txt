cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)
set(OPENSSL_LIB_PATH vendor/OpenSSL/lib)
set(LIBS HTTP SocketStreams crypto ssl)
set(LIBRARY_TYPE STATIC)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install-dir CACHE STRING "")
option(BUILD_SHARED_LIBS "" OFF)
option(SHARED_NETWORKS "Build shared Networks with static linked dependencies" OFF)

project(Networks VERSION 1.14.1)

if (UNIX)
	add_definitions(-D__LINUX__)

	set(CMAKE_POSITION_INDEPENDENT_CODE ON)

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
		add_definitions(-D__ANDROID__)

		set(OPENSSL_LIB_PATH ${OPENSSL_LIB_PATH}/Android)

		install(DIRECTORY vendor/OpenSSL/lib/Android/ DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/vendor/OpenSSL FILES_MATCHING PATTERN "*.a")
	else()
		set(OPENSSL_LIB_PATH ${OPENSSL_LIB_PATH}/Linux)

		install(DIRECTORY vendor/OpenSSL/lib/Linux/ DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/vendor/OpenSSL FILES_MATCHING PATTERN "*.a")
	endif()
else ()
	set(OPENSSL_LIB_PATH ${OPENSSL_LIB_PATH}/Windows)
	set(LIBS ${LIBS} crypt32)

	install(DIRECTORY vendor/OpenSSL/lib/Windows/ DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/vendor/OpenSSL FILES_MATCHING PATTERN "*.lib")
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
