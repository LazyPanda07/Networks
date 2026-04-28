cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)
set(PLATFORM_NAME "")
set(LIBS Networks HTTP JSON SocketStreams ssl crypto)
set(BASE_TCP_SERVER_VERSION 1.17.2)
set(GTEST_VERSION 1.17.0)
set(UTILITY_LIBRARY_VERSION 0.12.0)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})

project(Tests)

include(FetchContent)

FetchContent_Declare(
    BaseTCPServer
    GIT_REPOSITORY https://github.com/LazyPanda07/BaseTCPServer.git
    GIT_TAG v${BASE_TCP_SERVER_VERSION}
)
FetchContent_Declare(
	UtilityLibrary
	GIT_REPOSITORY https://github.com/LazyPanda07/UtilityLibrary.git
	GIT_TAG v${UTILITY_LIBRARY_VERSION}
)
FetchContent_Declare(
	GTest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG v${GTEST_VERSION}
)

FetchContent_MakeAvailable(BaseTCPServer)
FetchContent_MakeAvailable(UtilityLibrary)
FetchContent_MakeAvailable(GTest)

if (UNIX)
	add_definitions(-D__LINUX__)
	set(SHARED_OBJECT ${PROJECT_SOURCE_DIR}/../Networks/lib/libNetworks.so)

	if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
		set(PLATFORM_NAME LinuxARM)
	else()
		set(PLATFORM_NAME Linux)
	endif()
else ()
	set(PLATFORM_NAME Windows)
	set(LIBS ${LIBS} crypt32)
	set(SHARED_OBJECT ${PROJECT_SOURCE_DIR}/../Networks/dll/Networks.dll)
endif (UNIX)

add_executable(
	${PROJECT_NAME}
	main.cpp
	server.cpp
)

target_include_directories(
	${PROJECT_NAME} PUBLIC
	../include
	../HTTP/include
	../HTTP/JSON/include
	../SocketStreams/include
	../vendor/OpenSSL/${PLATFORM_NAME}/include
	${basetcpserver_SOURCE_DIR}/include
)

target_link_directories(
	${PROJECT_NAME} PUBLIC
	../Networks/lib
	../vendor/OpenSSL/${PLATFORM_NAME}/lib
)

target_link_libraries(
	${PROJECT_NAME} PUBLIC
	${LIBS}
	BaseTCPServer
	UtilityLibrary
	GTest::gtest
	GTest::gtest_main
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES web_socket_server.py DESTINATION bin)

if(EXISTS ${SHARED_OBJECT})
    install(FILES ${SHARED_OBJECT} DESTINATION bin)
endif()
