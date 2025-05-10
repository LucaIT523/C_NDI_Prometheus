//---------------------------------------------------------------------------


#include "ndi_info.h"
#include <iostream>
#include <fstream>
#include <vector>

NDIlib_recv_instance_t		gNDI_recv = NULL;


//. Jitter
std::chrono::high_resolution_clock::time_point last_audio_time;
std::chrono::high_resolution_clock::time_point last_video_time;
static const double average_smooth = 0.75;
double video_time_average = 0.0, video_time_jitter = 0.0;
double audio_time_average = 0.0, audio_time_jitter = 0.0;

std::vector<MY_JITTER_INFO>		g_stJitter_Video;
std::vector<MY_JITTER_INFO>		g_stJitter_Audio;


//. Blip
static const std::chrono::high_resolution_clock::time_point no_time;
std::chrono::high_resolution_clock::time_point last_audio_blip;
std::chrono::high_resolution_clock::time_point last_video_blip;
bool last_audio_was_blip = false;
bool last_video_was_blip = false;
float smoothed_average = 0.0f;

std::vector<MY_BLIP_INFO>		g_stBlip_Video;
std::vector<MY_BLIP_INFO>		g_stBlip_Audio;


int is_blip(const NDIlib_audio_frame_v2_t& audio_frame)
{
	// Cycle across the channel and look for the first sample above 0.5
	for (int sample_no = 0; sample_no < audio_frame.no_samples; sample_no++)
		if (std::abs(audio_frame.p_data[sample_no] > 0.5f))
			return sample_no;

	// This is not a blip.
	return -1;
}

bool is_blip(const NDIlib_video_frame_v2_t& video_frame)
{
	// Cycle across the middle line
	const uint8_t* p_line = video_frame.p_data + (video_frame.yres / 2) * video_frame.line_stride_in_bytes;

	// Compute the average luminance
	int avg = 0;
	for (int x = 0; x < video_frame.xres; x++)
		avg += p_line[x * 2 + 1];

	// Get whether this looks white or not
	return ((avg / video_frame.xres) > 128);
}

//-------------------------------------------------------
//. initialaition of ndi sdk library
//-------------------------------------------------------
int    My_NDI_Init()
{
	int				w_nSts = -1;
	char			w_szbuffInfo[256] = "__Windows_Test__";

	if (!NDIlib_initialize()){
		return w_nSts;
	}

	// We now have at least one source, so we create a receiver to look at it.
	NDIlib_recv_create_v3_t create_settings;
	create_settings.color_format = NDIlib_recv_color_format_fastest;
	create_settings.bandwidth = NDIlib_recv_bandwidth_highest;
	create_settings.source_to_connect_to.p_ndi_name = w_szbuffInfo;

	// Create the settings
	gNDI_recv = NDIlib_recv_create_v3(&create_settings);
	if (!gNDI_recv){
		return w_nSts;
	}

	//. OK
	w_nSts = 0;
	return w_nSts;
}


//-------------------------------------------------------
//. Close
//-------------------------------------------------------
void    
My_NDI_Close() 
{

	if (gNDI_recv) {
		// Destroy the receiver
		NDIlib_recv_destroy(gNDI_recv);
	}
	// Not required, but nice
	NDIlib_destroy();

	return;
}

//-------------------------------------------------------
//. Set jitter information for video and audio
//-------------------------------------------------------
int						//. Return	0 if success.
My_Set_Jitter_Blip(
	int		p_nOpt		//. 0 - All, 1 - Video , 2 - Audio , 3 - metadata
){
	int				w_nSts = -1;

	if (gNDI_recv == NULL) {
		return -1;
	}

	// The descriptors
	NDIlib_video_frame_v2_t video_frame;
	NDIlib_audio_frame_v2_t audio_frame;

	NDIlib_frame_type_e w_nType = NDIlib_recv_capture_v2(gNDI_recv, &video_frame, &audio_frame, nullptr, 5000);

	if (w_nType == NDIlib_frame_type_none) {
		//. OK
		w_nSts = 0;
		goto L_EXIT;
	}
	//. Video
	if (w_nType == NDIlib_frame_type_video && (p_nOpt == 1 || p_nOpt == 0)) {

		// Get the current time
		const std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
		// Skip the first frame so we have an accurate measurement
		if (last_video_time != std::chrono::high_resolution_clock::time_point()) {
			// Get the time since the last sample.
			const int64_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_video_time).count();

			// Compute the average frame time
			const double smooth = (video_time_average == 0.0) ? 0.0 : average_smooth;
			video_time_average = smooth * video_time_average + (1.0 - smooth) * (double)microseconds;

			// Compute the average jitter
			const double jitter = std::abs((double)microseconds - video_time_average);
			video_time_jitter = smooth * video_time_jitter + (1.0 - smooth) * (double)jitter;

			MY_JITTER_INFO		w_stJitterInfo;
			w_stJitterInfo.m_ftime_avg = video_time_average / 1000.0;
			w_stJitterInfo.m_ftime_jitter = video_time_jitter / 1000.0;
			g_stJitter_Video.push_back(w_stJitterInfo);

		}
		// This is now the last time seen
		last_video_time = current_time;

		//. Blip
		//current_time = std::chrono::high_resolution_clock::now();
		// Does this frame have a video blip
		if (is_blip(video_frame)) {	// If this was not a blip
			if (!last_video_was_blip) {	// This is the current video blip
				last_video_blip = current_time;

				// Display the AV latency
				if (last_audio_blip != no_time) {	// Get the time
					const int64_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>(last_video_blip - last_audio_blip).count();

					// Get the smoothed average
					smoothed_average = smoothed_average ? (0.9f * smoothed_average + 0.1f * (float)microseconds) : microseconds;

					// Display the offset
					MY_BLIP_INFO		w_stBlipInfo;
					w_stBlipInfo.m_ftime_offset = (float)microseconds / 1000.0f;
					w_stBlipInfo.m_fsmoothed_average = (float)smoothed_average / 1000.0f;
					g_stBlip_Video.push_back(w_stBlipInfo);
					//.
					last_video_blip = last_audio_blip = no_time;
				}
				// If the last frame was a blip
				last_video_was_blip = true;
			}
		}
		else {	// Half a second in we always reset the time
			if ((last_video_blip != no_time) && (current_time - last_video_blip > std::chrono::milliseconds(500)))
				last_video_blip = no_time;
			// No blip
			last_video_was_blip = false;
		}
		// Free the video packet
		NDIlib_recv_free_video_v2(gNDI_recv, &video_frame);

	}
	//. Audio
	if (w_nType == NDIlib_frame_type_audio && (p_nOpt == 2 || p_nOpt == 0)) {
		// Get the current time
		const std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
		// Skip the first frame so we have an accurate measurement
		if (last_audio_time != std::chrono::high_resolution_clock::time_point()) {
			// Get the time since the last sample.
			const int64_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_audio_time).count();
			// Compute the average frame time
			const double smooth = (audio_time_average == 0.0) ? 0.0 : average_smooth;
			audio_time_average = smooth * audio_time_average + (1.0 - smooth) * (double)microseconds;

			// Compute the average jitter
			const double jitter = std::abs((double)microseconds - audio_time_average);
			audio_time_jitter = smooth * audio_time_jitter + (1.0 - smooth) * (double)jitter;

			MY_JITTER_INFO		w_stJitterInfo;
			w_stJitterInfo.m_ftime_avg = audio_time_average / 1000.0;
			w_stJitterInfo.m_ftime_jitter = audio_time_jitter / 1000.0;
			g_stJitter_Audio.push_back(w_stJitterInfo);
		}
		// This is now the last time seen
		last_audio_time = current_time;

		//. Blip
		//const std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
		// Does this frame have a blip
		const int blip_time = is_blip(audio_frame);
		if (blip_time >= 0) {	// If the last frame was a blip
			if (!last_audio_was_blip) {	// This is the current audio blip
				last_audio_blip = current_time + std::chrono::microseconds(((int64_t)blip_time * 1000000ll) / (int64_t)audio_frame.sample_rate);

				// Display the AV latency
				if (last_video_blip != no_time) {	// Get the time
					const int64_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>(last_video_blip - last_audio_blip).count();

					// Get the smoothed average
					smoothed_average = smoothed_average ? (0.9f * smoothed_average + 0.1f * (float)microseconds) : microseconds;

					MY_BLIP_INFO		w_stBlipInfo;
					w_stBlipInfo.m_ftime_offset = (float)microseconds / 1000.0f;
					w_stBlipInfo.m_fsmoothed_average = (float)smoothed_average / 1000.0f;
					g_stBlip_Audio.push_back(w_stBlipInfo);
					last_video_blip = last_audio_blip = no_time;
				}
				// It was a blip
				last_audio_was_blip = true;
			}
		}
		else {	// Half a second in we always reset the time
			if ((last_audio_blip != no_time) && (current_time - last_audio_blip > std::chrono::milliseconds(500)))
				last_audio_blip = no_time;
			// It was a blip
			last_audio_was_blip = false;
		}

		// Free the audio packet
		NDIlib_recv_free_audio_v2(gNDI_recv, &audio_frame);
	}
	//. MetaData
	if (w_nType == NDIlib_frame_type_metadata && (p_nOpt == 3 || p_nOpt == 0)) {
		//. TO DO


	}

	//. OK
	w_nSts = 0;
L_EXIT:
	return w_nSts;
}
//-------------------------------------------------------
//.
//-------------------------------------------------------
int								//. Count of jitter
My_Get_Jitter(
	int		p_nOpt				//. 1 - Video , 2 - Audio
) {
//	std::vector<MY_JITTER_INFO>	w_stRtn;
	int			w_nCnt = 0;

	if (p_nOpt == 1) {
		w_nCnt = g_stJitter_Video.size();
		g_stJitter_Video.clear();
	}
	else if (p_nOpt == 2) {
		w_nCnt = g_stJitter_Audio.size();
		g_stJitter_Audio.clear();
	}
	else {
		//.

	}
	return w_nCnt;
}
//-------------------------------------------------------
//.
//-------------------------------------------------------
int								//. Count of blip
My_Get_Blip(
	int		p_nOpt				//. 1 - Video , 2 - Audio
) {
//	std::vector<MY_BLIP_INFO>	w_stRtn;
	int			w_nCnt = 0;

	if (p_nOpt == 1) {
		w_nCnt = g_stBlip_Video.size();
		g_stBlip_Video.clear();
	}
	else if (p_nOpt == 2) {
		w_nCnt = g_stBlip_Audio.size();
		g_stBlip_Audio.clear();
	}
	else {
		//.

	}
	return w_nCnt;
}