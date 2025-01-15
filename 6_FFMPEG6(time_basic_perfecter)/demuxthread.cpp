#include "demuxthread.h"
#include"log.h"

DemuxThread::DemuxThread(AVPacketQueue *audio_queue,AVPacketQueue *video_queue)
    :audio_queue_(audio_queue),video_queue_(video_queue)//传递，初始化audio_queue_  video_queue_
{
    LogInfo("DemuxThread");



}

DemuxThread::~DemuxThread(){
    LogInfo("~DemuxThread");
    if(thread_){
        Stop();
    }

}

int DemuxThread::Init(const char *url) {
    LogInfo("url:%s",url);
    int ret=0;//判断返回值
    url_=url;

    ifmt_ctx_=avformat_alloc_context();//分配上下文

    ret=avformat_open_input(&ifmt_ctx_,url_.c_str(),NULL,NULL);//url_.c_str():返回指向空终止内容的常量指针
    if(ret!=0){//0为正常返回值
        av_strerror(ret,err2str,sizeof(err2str));
        LogError("avformat_open_input failed,ret:%d, err2str:%s\n",ret,err2str);
        return -1;
    }
    //LogInfo("Init success.");

    ret=avformat_find_stream_info(ifmt_ctx_,NULL);
    if(ret<0){
        //打印错误信息
        av_strerror(ret,err2str,sizeof(err2str));
        LogError("avformat_find_stream_info failed,ret:%d, err2str:%s\n",ret,err2str);

        return -1;
    }
    av_dump_format(ifmt_ctx_,0,url,0);//打印码流信息
    audio_index_=av_find_best_stream(ifmt_ctx_, AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);//填-1，自动查找
    video_index_=av_find_best_stream(ifmt_ctx_, AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
    if(video_index_<0||audio_index_<0){
        LogError("no audio or video");
        return -1;
    }
    return 0;
}

int DemuxThread::Start() {
    thread_ =new std::thread(&DemuxThread::Run,this);
    if(!thread_){
        LogError("new std::thread(&DemuxThread::Run;this)failed ");
        return -1;
    }
    return 0;
}

int DemuxThread::Stop() {
    Thread::Stop();
    avformat_close_input(&ifmt_ctx_);

    return 0;
}

void DemuxThread::Run() {
    LogInfo(" DemuxThread::Run into");
    int ret =0;
    AVPacket pkt;
    while(abort_!=1){//Thread 友元
        if(audio_queue_->Size()>100||video_queue_->Size()>100){//音视频队列读取限制，不可无限读取
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        ret=av_read_frame(ifmt_ctx_,&pkt);
        if(ret<0){
            av_strerror(ret,err2str,sizeof(err2str));
            LogError("av_read_frame failed\n"
                     "ret:%d, err2str:%s\n",ret,err2str);

            break;
        }

        if(pkt.stream_index==audio_index_){
            LogInfo("audio pkt_size:%d, queue size:%d\n", pkt.size, audio_queue_->Size());
           ret= audio_queue_->Push(&pkt);
            //av_packet_unref(&pkt);

        }else if(pkt.stream_index==video_index_){
            LogInfo("video pkt_size:%d, queue size:%d\n", pkt.size, video_queue_->Size());
            ret =video_queue_->Push(&pkt);
            //av_packet_unref(&pkt);
        }else{
            av_packet_unref(&pkt);//释放包
        }
    }
    LogInfo("run finish");

}

AVCodecParameters *DemuxThread::AudioCodecParameters()
{
    if(audio_index_!=-1){
        return ifmt_ctx_->streams[audio_index_]->codecpar;//Codec parameters associated with this stream.

    }else{//==-1 说明失败
        return NULL;
    }

}

AVCodecParameters *DemuxThread::VideoCodecParameters()
{
    if(video_index_!=-1){
       return ifmt_ctx_->streams[video_index_]->codecpar;//Codec parameters associated with this stream.

    }else{//==-1 说明失败
        return NULL;
    }

}

AVRational DemuxThread::AudioStreamTimebase()
{
    if(audio_index_ != -1) {
        return ifmt_ctx_->streams[audio_index_]->time_base;
    } else {
        return AVRational{0, 0};
    }
}



AVRational DemuxThread::VideoStreamTimebase()
{
    if(video_index_ != -1) {
        return ifmt_ctx_->streams[video_index_]->time_base;//传回去
    } else {
        return AVRational{0, 0};
    }
}
