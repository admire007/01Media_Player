#include<iostream>

#include "libavutil/avutil.h"
#include"demuxthread.h"
#include"avframqueue.h"
#include"decodethread.h"
#include"audiooutput.h"
#include"videooutput.h"


#include"log.h"


using namespace std;
#undef main //与ffmpeg  main冲突
int main(int argc,char *argv[])
{
    int ret=0;
    printf("Hello FFMPEG\n");
    LogInit();
    //声明队列 给下面new DemuxThread()传参
    AVPacketQueue audio_packet_queue;
    AVPacketQueue video_packet_queue;


    AVFramQueue audio_frame_queue;
    AVFramQueue video_frame_queue;

    //解复用
    DemuxThread *demux_thread=new DemuxThread(&audio_packet_queue,&video_packet_queue);
    ret=demux_thread->Init(argv[1]);//[0]全命令行 [1]文件名
    if(ret<0){
        LogError("demux_thread.Init  failed");
        return -1;
    }

    ret=demux_thread->Start();//[0]全命令行 [1]文件名
    if(ret<0){
        LogError("demux_thread.Start  failed");
        return -1;
    }


    //解码线程初始化(音频线程)
    DecodeThread *audio_decode_thread=new DecodeThread(&audio_packet_queue,&audio_frame_queue);

    ret=audio_decode_thread->Init(demux_thread->AudioCodecParameters());
    if(ret<0){
        LogError("audio_decode_thread->Init  failed");
        return -1;
    }

    ret=audio_decode_thread->Start();
    if(ret<0){
        LogError("audio_decode_thread->Start failed");
        return -1;
    }


    //解码线程初始化(视频线程)
    DecodeThread *video_decode_thread=new DecodeThread(&video_packet_queue,&video_frame_queue);

    ret=video_decode_thread->Init(demux_thread->VideoCodecParameters());
    if(ret<0){
        LogError("video_decode_thread->Init  failed");
        return -1;
    }

    ret=video_decode_thread->Start();
    if(ret<0){
        LogError("video_decode_thread->Start failed");
        return -1;
    }


    //初始化解码后的audio输出
    AudioPrams audio_prams={0};
    memset(&audio_prams,0,sizeof(AudioPrams));
    audio_prams.channels=demux_thread->AudioCodecParameters()->channels;//通道数
    audio_prams.channel_layout=demux_thread->AudioCodecParameters()->channel_layout;
    audio_prams.fmt=(enum AVSampleFormat)demux_thread->AudioCodecParameters()->format;
    audio_prams.freq=demux_thread->AudioCodecParameters()->sample_rate;
    audio_prams.frame_size=demux_thread->AudioCodecParameters()->frame_size;

    AudioOutput *audio_output =new AudioOutput(audio_prams,&audio_frame_queue);
    ret=audio_output->Init();
    if(ret<0){
        LogError("audio_output->Init() failed");
        return -1;
    }

    video_output *video_output_ = new  video_output(&video_frame_queue, demux_thread->VideoCodecParameters()->width,
                                                demux_thread->VideoCodecParameters()->height);

    ret = video_output_->Init();
    if(ret < 0) {
        LogError("video_output->Init() failed");
        return -1;
    }
    video_output_->MainLoop();//等待退出事件















    //休眠2秒
    std::this_thread::sleep_for(std::chrono::milliseconds(120*1000));

    LogInfo("demux_thread->stop");
    demux_thread->Stop();
    delete demux_thread;

    LogInfo("audio_decode_thread->stop");
    audio_decode_thread->Stop();
    LogInfo("delete audio_decode_thread");
    delete audio_decode_thread;

    LogInfo("video_decode_thread->stop");
    video_decode_thread->Stop();
    LogInfo("delete video_decode_thread");
    delete video_decode_thread;


    LogInfo("main finish");

    return 0;
}
