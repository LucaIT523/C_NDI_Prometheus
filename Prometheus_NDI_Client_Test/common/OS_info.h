#ifndef __OS_INFO_InferenceH__
#define __OS_INFO_InferenceH__
//---------------------------------------------------------------------------

#include <cassert>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <atomic>
#include <chrono>
#include <thread>
#include <iostream>                    // std::cout
#include <fstream>                     // std::ofstream
#include <filesystem>                  // std::filesystem
#include <string>                      // std::string


// Function to get CPU usage
double getCpuUsage();

// Function to get memory usage
double getMemoryUsage();

// Function to get gpu usage
double getGpuUsage();


#endif // __OS_INFO_InferenceH__