#pragma once

#include <string>

#ifdef NETWORKS_DLL
#ifdef __LINUX__
#define NETWORKS_API __attribute__((visibility("default")))
#else
#define NETWORKS_API __declspec(dllexport)

#define NETWORKS_API_FUNCTION extern "C" NETWORKS_API

#endif
#else
#define NETWORKS_API
#define NETWORKS_API_FUNCTION
#endif // NETWORKS_DLL

namespace web
{
	NETWORKS_API_FUNCTION std::string getNetworksVersion();
}
