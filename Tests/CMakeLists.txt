cmake_minimum_required(VERSION 3.27.0)

project(Tests)

set(CMAKE_CXX_STANDARD 17)

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
)

target_link_libraries(
	${PROJECT_NAME} PUBLIC
	Networks
	HTTP
	JSON
	SocketStreams
	gtest
	gtest_main
)
