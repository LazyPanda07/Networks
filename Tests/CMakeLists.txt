cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)
set(OPENSSL_LIB_PATH ../vendor/OpenSSL/lib)
set(LIBS Networks HTTP JSON SocketStreams ssl crypto)

project(Tests)

find_package(GTest)

if(NOT ${GTest_FOUND})
	set(GTEST_BOTH_LIBRARIES gtest gtest_main)
	set(GTEST_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/gtest/include/)
endif()

if (UNIX)
	set(OPENSSL_LIB_PATH ${OPENSSL_LIB_PATH}/Linux)
	set(SHARED_OBJECT ${PROJECT_SOURCE_DIR}/../Networks/lib/libNetworks.so)
	add_definitions(-D__LINUX__)
else ()
	set(OPENSSL_LIB_PATH ${OPENSSL_LIB_PATH}/Windows)
	set(LIBS ${LIBS} crypt32)
	set(SHARED_OBJECT ${PROJECT_SOURCE_DIR}/../Networks/dll/Networks.dll)
endif (UNIX)

add_executable(
	${PROJECT_NAME}
	main.cpp
)

target_include_directories(
	${PROJECT_NAME} PUBLIC
	../src/
	../HTTP/src/
	../HTTP/JSON/src/
	../SocketStreams/src/
	../vendor/OpenSSL/include/
	${GTEST_INCLUDE_DIRS}
)

target_link_directories(
	${PROJECT_NAME} PUBLIC
	../Networks/lib
	gtest/lib
	${OPENSSL_LIB_PATH}
)

target_link_libraries(
	${PROJECT_NAME} PUBLIC
	${LIBS}
	${GTEST_BOTH_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR}/bin)

if(EXISTS ${SHARED_OBJECT})
    install(FILES ${SHARED_OBJECT} DESTINATION ${CMAKE_BINARY_DIR}/bin)
endif()
