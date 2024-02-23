cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 17)
set(OPENSSL_LIB_PATH vendor/OpenSSL/lib)
set(LIBS HTTP SocketStreams crypto ssl)
set(LIBRARY_TYPE STATIC)
option(BUILD_SHARED_LIBS "" OFF)
option(SHARED_NETWORKS "Build shared Networks with static linked dependencies" OFF)

if (UNIX)
	add_definitions(-D__LINUX__)

	set(OPENSSL_LIB_PATH ${OPENSSL_LIB_PATH}/Linux)
	add_compile_options(-fPIC)
else ()
	set(OPENSSL_LIB_PATH ${OPENSSL_LIB_PATH}/Windows)
	set(LIBS ${LIBS} crypt32)
endif ()

if (${BUILD_SHARED_LIBS} OR ${SHARED_NETWORKS})
	add_definitions(-DNETWORKS_DLL)
	set(LIBRARY_TYPE SHARED)
endif ()

project(Networks)

add_library(
	${PROJECT_NAME} ${LIBRARY_TYPE}
	src/HTTPNetwork.cpp
	src/HTTPSNetwork.cpp
	src/NetworksUtility.cpp
	src/Exceptions/SSLException.cpp
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

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR}/bin)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
	install(DIRECTORY src/ DESTINATION ${CMAKE_BINARY_DIR}/bin/include FILES_MATCHING PATTERN "*.h")
endif ()
