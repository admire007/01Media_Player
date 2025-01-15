#ifndef AVFRAMQUEUE_H
#define AVFRAMQUEUE_H

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

class AVFramQueue
{
public:
    AVFramQueue();
    ~AVFramQueue();
    void Abort();
    int Push(AVFrame *val);
    AVFrame *Pop(const int timeout);
    AVFrame *Front();//只读取获取信息，无需删除
    int Size();//PCM YUV多少帧

private:
    void release();
    Queue<AVFrame *> queue_;
};

#endif // AVFRAMQUEUE_H
