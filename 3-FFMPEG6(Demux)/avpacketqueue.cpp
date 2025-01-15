#include "avpacketqueue.h"
#include"log.h"

AVPacketQueue::AVPacketQueue() {}

AVPacketQueue::~AVPacketQueue()
{


}

void AVPacketQueue::Abort()
{
    Releses();
    queue_.Abort();

}

void AVPacketQueue::Releses()
{
    while(true){
        AVPacket *pkt=NULL;
        int ret=queue_.Pop(pkt,1);//超一毫秒就退出
        if(ret<0){
            break;
        }else{
            av_packet_free(&pkt);//释放
            continue;
        }
    }
}

int AVPacketQueue::Size()
{
    queue_.Size();
    return 0;
}

int AVPacketQueue::Push(AVPacket *val)
{
    AVPacket *tmp_pkt = av_packet_alloc(); // 申请
    if (!tmp_pkt) {
        return -1; // 处理内存分配失败
    }
    av_packet_move_ref(tmp_pkt, val); // 将 val 的内容移动到 tmp_pkt
    return queue_.Push(tmp_pkt);

}

// AVPacket *AVPacketQueue::Pop(const int timeout)
// {
//     AVPacket *tmp_pkt=NULL;
//     int ret=queue_.Pop(tmp_pkt,timeout);
//     //判断
//     if(ret<0){
//         if(ret==-1){
//             LogError("AVPacketQueue :Pop failed");
//         }
//     }

//     return tmp_pkt;
// }
