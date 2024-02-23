cmake_minimum_required(VERSION 3.27.0)

project(Tests)

set(CMAKE_CXX_STANDARD 17)
set(OPENSSL_LIB_PATH ../vendor/OpenSSL/lib)
set(LIBS Networks HTTP JSON SocketStreams gtest gtest_main ssl crypto)

if (UNIX)
	set(OPENSSL_LIB_PATH ${OPENSSL_LIB_PATH}/Linux)
	set(SHARED_OBJECT ${PROJECT_SOURCE_DIR}/../Networks/libNetworks.so)
	add_definitions(-D__LINUX__)
else ()
	set(OPENSSL_LIB_PATH ${OPENSSL_LIB_PATH}/Windows)
	set(LIBS ${LIBS} crypt32)
	set(SHARED_OBJECT ${PROJECT_SOURCE_DIR}/../Networks/Networks.dll)
endif (UNIX)

add_executable(
	${PROJECT_NAME}
	main.cpp
)

target_include_directories(
	${PROJECT_NAME} PUBLIC
	gtest/include/
	../src/
	../HTTP/src/
	../HTTP/JSON/src/
	../SocketStreams/src/
	../vendor/OpenSSL/include/
)

target_link_directories(
	${PROJECT_NAME} PUBLIC
	../Networks
	gtest/lib
	${OPENSSL_LIB_PATH}
)

target_link_libraries(
	${PROJECT_NAME} PUBLIC
	${LIBS}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR}/bin)

if(EXISTS ${SHARED_OBJECT})
    install(FILES ${SHARED_OBJECT} DESTINATION ${CMAKE_BINARY_DIR}/bin)
endif()
