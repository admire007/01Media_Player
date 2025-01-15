#ifndef DEMUXTHREAD_H
#define DEMUXTHREAD_H
#include<thread.h>
#include"avpacketqueue.h"

// 如果创建C++工程引用头文件的方式
#ifdef __cplusplus ///
    extern "C"
{
// 包含ffmpeg头文件
#include<libavutil/avutil.h>
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
    }
#endif



class DemuxThread:public Thread//友元
{
public:
    DemuxThread(AVPacketQueue *video_queue,AVPacketQueue *audio_queue);
    ~DemuxThread();
    int Init(const char *url_);//初始化
    int Start();
    int Stop();
    void Run();

    AVCodecParameters *AudioCodecParameters();//保存音视频流基本参数信息，通常会在AVCodecContext被填充使用
    AVCodecParameters *VideoCodecParameters();

private:
    char err2str[256]={0};//将错误信息返回数字转为字符串
    std::string url_;//文件名
    //音，视队列
    AVPacketQueue *audio_queue_=NULL;
    AVPacketQueue *video_queue_=NULL;

    AVFormatContext *ifmt_ctx_=NULL;
    int audio_index_=-1;//从0开始，-1表示没找到
    int video_index_=-1;



};

#endif // DEMUXTHREAD_H
