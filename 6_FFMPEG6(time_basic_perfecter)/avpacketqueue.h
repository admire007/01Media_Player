#ifndef AVPACKETQUEUE_H
#define AVPACKETQUEUE_H
#include"queue.h"

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

// class AVPavketQueue:public Queue//继承
class AVPacketQueue
{
public:
    AVPacketQueue();
    ~AVPacketQueue();
    void Abort();
    void Releses();//释放资源函数
    int Size();
    int Push(AVPacket *val);//取数据
    AVPacket *Pop(const int timeout);
private:
     Queue<AVPacket *>queue_;//多用组合，少用继承

};

#endif // AVPACKETQUEUE_H
