#include "avframqueue.h"
#include"log.h"

AVFramQueue::AVFramQueue() {}

AVFramQueue::~AVFramQueue()
{

}

void AVFramQueue::Abort()
{
    release();//先清除
    queue_.Abort();//后退出
}

int AVFramQueue::Push(AVFrame *val)
{
    AVFrame *tmp_frame=av_frame_alloc();
    av_frame_move_ref(tmp_frame,val);
    return queue_.Push(tmp_frame);
}

AVFrame *AVFramQueue::Pop(const int timeout)
{
    AVFrame *tmp_frame=NULL;
    int ret=queue_.Pop(tmp_frame,timeout);
    if(ret<0){
        if(ret==-1){
            LogError("AVFrame Pop failed");
        }
    }
    return tmp_frame;

}

AVFrame *AVFramQueue::Front()
{
    AVFrame *tmp_frame=NULL;
    int ret=queue_.Front(tmp_frame);
    if(ret<0){
        if(ret==-1){
            LogError("AVFrame Front failed");
        }
    }
    return tmp_frame;
}

int AVFramQueue::Size()
{
    return queue_.Size();
}

void AVFramQueue::release()
{
    while(true){
        AVFrame *frame=NULL;
        int ret=queue_.Pop(frame,1);
        if(ret<0){
            break;
        }else{//==0有数据读
            av_frame_free(&frame);
            continue;
        }
    }

}
