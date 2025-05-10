#ifndef __NDI_INFO_InferenceH__
#define __NDI_INFO_InferenceH__
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
#include <vector>                      // std::vector

using namespace std;

#ifdef _WIN32
#include <windows.h>

#ifdef _WIN64
#pragma comment(lib, "Processing.NDI.Lib.Advanced.x64.lib")
#else // _WIN64
#pragma comment(lib, "Processing.NDI.Lib.Advanced.x86.lib")
#endif // _WIN64

#endif

#include <Processing.NDI.Advanced.h>

struct MY_JITTER_INFO
{
	double		m_ftime_avg;

	double		m_ftime_jitter;
};

struct MY_BLIP_INFO
{
	double		m_ftime_offset;

	double		m_fsmoothed_average;
};

//-------------------------------------------------------
//. initialaition of ndi sdk library
//-------------------------------------------------------
int					//. Return	0 if success.
My_NDI_Init();

//-------------------------------------------------------
//. Set jitter information for video and audio
//-------------------------------------------------------
int						//. Return	0 if success.
My_Set_Jitter_Blip(
	int		p_nOpt		//. 0 - All, 1 - Video , 2 - Audio , 3 - metadata
);

//-------------------------------------------------------
//. Get jitter information for video and audio
//-------------------------------------------------------
int								//. Count of jitter
My_Get_Jitter(
	int		p_nOpt				//. 1 - Video , 2 - Audio
);

//-------------------------------------------------------
//. Get blip information for video and audio
//-------------------------------------------------------
//std::vector<MY_BLIP_INFO>		//. Count of blip
int
My_Get_Blip(
	int		p_nOpt				//. 1 - Video , 2 - Audio
);


//-------------------------------------------------------
//. Close
//-------------------------------------------------------
void    My_NDI_Close();


#endif // __NDI_INFO_InferenceH__