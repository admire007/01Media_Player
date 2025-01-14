#include<iostream>

#include "libavutil/avutil.h"
#include"demuxthread.h"


#include"log.h"


using namespace std;

int main(int argc,char *argv[])
{
    int ret=0;
    printf("Hello FFMPEG\n");
    LogInit();


    //解复用
    DemuxThread demux_thread;
    ret=demux_thread.Init(argv[1]);//[0]全命令行 [1]文件名
    if(ret<0){
        LogError("demux_thread.Init  failed");
        return -1;
    }

    ret=demux_thread.Start();//[0]全命令行 [1]文件名
    if(ret<0){
        LogError("demux_thread.Start  failed");
        return -1;
    }

    //休眠2秒
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));











    return 0;
}
