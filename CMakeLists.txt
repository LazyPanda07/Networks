cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 17)
set(OPENSSL_LIB_PATH vendor/OpenSSL/lib)
set(LIBS HTTP SocketStreams crypto ssl)
set(LIBRARY_TYPE STATIC)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install-dir CACHE STRING "")
option(BUILD_SHARED_LIBS "" OFF)
option(SHARED_NETWORKS "Build shared Networks with static linked dependencies" OFF)

if (UNIX)
	add_definitions(-D__LINUX__)

	set(OPENSSL_LIB_PATH ${OPENSSL_LIB_PATH}/Linux)

	add_compile_options(-fPIC)

	install(DIRECTORY vendor/OpenSSL/lib/Linux/ DESTINATION ${CMAKE_INSTALL_PREFIX}/lib FILES_MATCHING PATTERN "*.a")
else ()
	set(OPENSSL_LIB_PATH ${OPENSSL_LIB_PATH}/Windows)
	set(LIBS ${LIBS} crypt32)

	install(DIRECTORY vendor/OpenSSL/lib/Windows/ DESTINATION ${CMAKE_INSTALL_PREFIX}/lib FILES_MATCHING PATTERN "*.lib")
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

install(
	TARGETS ${PROJECT_NAME} 
	ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/dll
)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
	install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include FILES_MATCHING PATTERN "*.h")
	install(DIRECTORY vendor/OpenSSL/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/vendor/OpenSSL/)
endif ()
