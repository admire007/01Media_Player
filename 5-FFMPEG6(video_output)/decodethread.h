#ifndef DECODETHREAD_H
#define DECODETHREAD_H
#include"thread.h"
#include"avpacketqueue.h"
#include"avframqueue.h"
/*解码器线程，与什么交互：AVPacketQueue  AVFramQueue
*/
class DecodeThread : public Thread
{
public:
    DecodeThread(AVPacketQueue *packet_queue,AVFramQueue *frame_queue);
    ~DecodeThread();
    int Init(AVCodecParameters *par);//初始化解码器相关的
    int Start();
    int Stop();
    void Run();//解码线程run
private:
    char err2str[256]={0};//将错误信息返回数字转为字符串
    AVCodecContext *codec_ctx_=NULL;//上下文
    //声明变量并初始化
    AVPacketQueue *packet_queue_=NULL;
    AVFramQueue *frame_queue_=NULL;

};

#endif // DECODETHREAD_H
