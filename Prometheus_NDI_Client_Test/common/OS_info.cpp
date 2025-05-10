
#include "OS_info.h"

//#include <unistd.h>
#include <fstream>
#include <sstream>
#include <Pdh.h>


#pragma comment(lib,"pdh.lib")

// Function to get CPU usage
double getCpuUsage() {
    //. Linux
    //std::ifstream statFile("/proc/stat");
    //std::string line;
    //std::getline(statFile, line);
    //std::istringstream iss(line);
    //std::string cpuLabel;
    //unsigned long user, nice, system, idle, iowait, irq, softirq, steal, guest, guestNice;
    //iss >> cpuLabel >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guestNice;
    //unsigned long idleTime = idle + iowait;
    //unsigned long nonIdleTime = user + nice + system + irq + softirq + steal;
    //unsigned long totalTime = idleTime + nonIdleTime;
    //double cpuUsage = (totalTime > 0) ? (1.0 - (static_cast<double>(idleTime) / totalTime)) : 0.0;
    //return cpuUsage;

    double              w_fCpuUsage = 0.0;
    PDH_HQUERY          query;
    PDH_HCOUNTER        counter;

    // Initialize the Performance Data Helper (PDH) library
    PdhOpenQuery(NULL, NULL, &query);
    PdhAddCounter(query, TEXT("\\Processor(_Total)\\% Processor Time"), NULL, &counter);
    PdhCollectQueryData(query);

    // Retrieve CPU usage
    PDH_FMT_COUNTERVALUE counterValue;
    PdhCollectQueryData(query);
    PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &counterValue);

    // Print CPU usage
//    std::cout << "CPU Usage: " << counterValue.doubleValue << "%" << std::endl;
    w_fCpuUsage = counterValue.doubleValue;

    // Close the query and cleanup
    PdhCloseQuery(query);

    return w_fCpuUsage;
}

// Function to get memory usage
double getMemoryUsage() {
    //. Linux
    //std::ifstream meminfoFile("/proc/meminfo");
    //std::string line;
    //std::getline(meminfoFile, line);
    //std::istringstream iss(line);
    //std::string memTotalLabel;
    //unsigned long memTotal;
    //iss >> memTotalLabel >> memTotal;
    //std::getline(meminfoFile, line);
    //std::istringstream iss2(line);
    //std::string memFreeLabel;
    //unsigned long memFree;
    //iss2 >> memFreeLabel >> memFree;
    //double memoryUsage = (memTotal > 0) ? (1.0 - (static_cast<double>(memFree) / memTotal)) : 0.0;
    //return memoryUsage;


    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    // Retrieve memory information
    GlobalMemoryStatusEx(&memoryStatus);
    double memoryUsage = 0.0;

    // Print memory usage
//    std::cout << "Total Physical Memory: " << memoryStatus.ullTotalPhys / (1024 * 1024) << " MB" << std::endl;
//    std::cout << "Available Physical Memory: " << memoryStatus.ullAvailPhys / (1024 * 1024) << " MB" << std::endl;
//    std::cout << "Total Virtual Memory: " << memoryStatus.ullTotalVirtual / (1024 * 1024) << " MB" << std::endl;
//    std::cout << "Available Virtual Memory: " << memoryStatus.ullAvailVirtual / (1024 * 1024) << " MB" << std::endl;

    memoryUsage = (memoryStatus.ullTotalPhys > 0) ? ( (memoryStatus.ullTotalPhys - memoryStatus.ullAvailPhys) * 100.0 / memoryStatus.ullTotalPhys) : 0.0;
    return memoryUsage;
}

// Function to get gpu usage
double getGpuUsage()
{


    return 0.0;
}