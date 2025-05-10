#include <array>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>
#include <iostream>

#include <prometheus/gauge.h>
#include "prometheus/client_metric.h"
#include "prometheus/counter.h"
#include "prometheus/exposer.h"
#include "prometheus/family.h"
#include "prometheus/info.h"
#include "prometheus/registry.h"

#include "OS_info.h"
#include "ndi_info.h"

#ifdef _WIN32
#include <windows.h>
#endif


int main()
{
    using namespace prometheus;

    try {
        //.
        My_NDI_Init();

        // create a http server running on port 8080
        Exposer exposer{ "127.0.0.1:8080" };

        // create a metrics registry
        // @note it's the users responsibility to keep the object alive
        auto registry = std::make_shared<Registry>();

        auto& video_blip_packet_counter = prometheus::BuildCounter()
            .Name("ndi_packet_video_blip")
            .Help("blip number of video packets")
            .Register(*registry);

        // add and remember dimensional data, incrementing those is very cheap
        auto& video_blip_counter = video_blip_packet_counter.Add({ {"video_packet", "blip"} });


        auto& audio_blip_packet_counter = prometheus::BuildCounter()
            .Name("ndi_packet_audio_blip")
            .Help("blip number of audio packets")
            .Register(*registry);

        // add and remember dimensional data, incrementing those is very cheap
        auto& audio_blip_counter = audio_blip_packet_counter.Add({ {"audio_packet", "blip"} });

        //.
        auto& video_jitter_packet_counter = prometheus::BuildCounter()
            .Name("ndi_packet_video_jitter")
            .Help("jitter number of video packets")
            .Register(*registry);

        // add and remember dimensional data, incrementing those is very cheap
        auto& video_jitter_counter = video_jitter_packet_counter.Add({ {"video_packet", "jitter"} });


        auto& audio_jitter_packet_counter = prometheus::BuildCounter()
            .Name("ndi_packet_audio_jitter")
            .Help("jitter number of audio packets")
            .Register(*registry);

        // add and remember dimensional data, incrementing those is very cheap
        auto& audio_jitter_counter = audio_jitter_packet_counter.Add({ {"audio_packet", "jitter"} });

        //. Loop NDI SDK 
        std::thread updateThread([]() {
            while (true) {
                //. Video and audio
                My_Set_Jitter_Blip(0);
            }
            });

        // Create gauge metrics for CPU, memory, and network state
        auto& cpuGauge = prometheus::BuildGauge()
            .Name("cpu_usage")
            .Help("CPU usage in percentage")
            .Register(*registry);

        auto& cpu_gauge_opr{ cpuGauge.Add({{"number", "1"}}) };


        auto& memoryGauge = prometheus::BuildGauge()
            .Name("memory_usage")
            .Help("Memory usage in bytes")
            .Register(*registry);

        auto& memory_gauge_opr{ memoryGauge.Add({{"number", "1"}}) };

        // ask the exposer to scrape the registry on incoming HTTP requests
        exposer.RegisterCollectable(registry);

        int    w_Cnt = 0;

        std::cout << "program run...." << std::endl;

        for (;;) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            double cpuUsage = getCpuUsage();;
            double memoryUsage = getMemoryUsage();
            cpu_gauge_opr.Set(cpuUsage);
            memory_gauge_opr.Set(memoryUsage);

            //. Get NDI Libaray
            //. video jitter
            w_Cnt = My_Get_Jitter(1);
            for (int i = 0; i < w_Cnt; i++) {
                video_jitter_counter.Increment();
                std::cout << "video_jitter_counter.Increment() " << std::endl;
            }
            //. audio jitter
            w_Cnt = My_Get_Jitter(2);
            for (int i = 0; i < w_Cnt; i++) {
                audio_jitter_counter.Increment();
                std::cout << "audio_jitter_counter.Increment() " << std::endl;
            }
            //. video blip
            w_Cnt = My_Get_Blip(1);
            for (int i = 0; i < w_Cnt; i++) {
                video_blip_counter.Increment();
                std::cout << "video_blip_counter.Increment() " << std::endl;
            }
            //. audio jitter
            w_Cnt = My_Get_Blip(2);
            for (int i = 0; i < w_Cnt; i++) {
                audio_blip_counter.Increment();
                std::cout << "audio_blip_counter.Increment() " << std::endl;
            }

        }

        My_NDI_Close();

    }
    catch (const std::exception& error) {
        std::cout << error.what() << std::endl;
    }
    return 0;
}


