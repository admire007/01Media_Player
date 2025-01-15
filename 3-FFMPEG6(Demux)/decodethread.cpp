#include "decodethread.h"
#include"log.h"
#include"demuxthread.h"


DecodeThread::DecodeThread(AVPacketQueue *packet_queue, AVFramQueue *frame_queue)
    :packet_queue_(packet_queue),frame_queue_(frame_queue)
{

}

DecodeThread::~DecodeThread()
{
    if(thread_){//线程还没有释放
        Stop();
    }
    if(codec_ctx_){
        avcodec_close(codec_ctx_);

    }
}

int DecodeThread::Init(AVCodecParameters *par)
{
    //先判断为空？后分配
    if(!par){
    LogError("Init par is null\n");
        return -1;
}
    codec_ctx_=avcodec_alloc_context3(NULL);

    //将传递进来的AVCodecParameters 拷贝到 codec_ctx_
    int ret=avcodec_parameters_to_context(codec_ctx_,par);
    if(ret<0){
        av_strerror(ret,err2str,sizeof(err2str));
        LogError("avcodec_parameters_to_context failed,ret:%d, err2str:%s\n",ret,err2str);

        return -1;
    }

    //找解码器
    //codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    /*codec = avcodec_find_decoder_by_name("h264_cuvid");
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }*/
    AVCodec *codec=avcodec_find_decoder(codec_ctx_->codec_id);//作业，实现指定解码器，硬件解码
    if(!codec){
        av_strerror(ret,err2str,sizeof(err2str));
        LogError("avcodec_find_decoder failed,ret:%d ,err2str:%s\n",ret,err2str);

        return -1;
    }

    ret=avcodec_open2(codec_ctx_,codec,NULL);
    if(ret<0){
        av_strerror(ret,err2str,sizeof(err2str));
        LogError("avcodec_open2 failed,ret:%d, err2str:%s\n",ret,err2str);

        return -1;
    }

    LogInfo("Init finished\n");
    return 0;


}

int DecodeThread::Start()
{
    thread_=new std::thread(&DecodeThread::Run,this);//绑定线程
    if(!thread_){
        LogInfo("new std::thread(&DecodeThread::Run,this) failed\n");

        return -1;
    }

}

int DecodeThread::Stop()
{
    Thread::Stop();
}

void DecodeThread::Run()
{
    AVFrame *frame=av_frame_alloc();
    LogInfo("DecodeThread::Run info\n");

    while(abort_!=1){
        if(frame_queue_->Size()>10){//队列读取限制，不可无限读取
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        AVPacket *pkt=packet_queue_->Pop(10);
        if(pkt){//pkt有数据
            int ret=avcodec_send_packet(codec_ctx_,pkt);//传包
            //注意！！
            av_packet_free(&pkt);
            LogInfo("ret=%d",ret);
            if(ret<0){
                av_strerror(ret,err2str,sizeof(err2str));
                LogError("avcodec_send_packet failed,ret:%d, err2str:%s\n",ret,err2str);

                break;
            }
            //读解码后数据frame
            while(true){
                ret=avcodec_receive_frame(codec_ctx_,frame);

            if(ret==0){//==0正常
                frame_queue_->Push(frame);//转移到队列
                LogInfo("%s frame_queue_->Push(frame) size:%d\n",codec_ctx_->codec->name,frame_queue_->Size());
                continue;
            }else if(AVERROR(EAGAIN)==ret){//如果需要继续调用avcodec_send_packet
                break;//有些帧无法读到，跳出while循环
            }else{
                abort_=1;
                av_strerror(ret,err2str,sizeof(err2str));
                LogError("avcodec_receive_frame failed,ret:%d, err2str:%s\n",ret,err2str);

                break;
            }
            }
        }else{
            LogInfo("no get packet\n");
        }
    }
LogInfo("DecodeThread::Run finished\n");
}
