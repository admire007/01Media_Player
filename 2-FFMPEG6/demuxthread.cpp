#include "demuxthread.h"
#include"log.h"

DemuxThread::DemuxThread(AVPacketQueue *video_queue,AVPacketQueue *audio_queue)
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
    LogInfo("DemuxThread::Init success.");

    ret=avformat_find_stream_info(ifmt_ctx_,NULL);
    if(ret<0){
        //打印错误信息
        av_strerror(ret,err2str,sizeof(err2str));
        LogError("avformat_open_input failed,ret:%d, err2str:%s\n",ret,err2str);

        return -1;
    }
    av_dump_format(ifmt_ctx_,0,url,0);//打印码流信息
    audio_index_=av_find_best_stream(ifmt_ctx_, AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);//填-1，自动查找
    video_index_=av_find_best_stream(ifmt_ctx_, AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
    if(video_index_<0||audio_index_<0){
        LogError("no audio or video");
        return -1;
    }

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
    LogInfo("Run into");
    int ret =0;
    AVPacket pkt;
    while(abort_!=1){//Thread 友元
        ret=av_read_frame(ifmt_ctx_,&pkt);
        if(ret<0){
            av_strerror(ret,err2str,sizeof(err2str));
            LogError("avformat_open_input failed,ret:%d, err2str:%s\n",ret,err2str);

            break;
        }
        if(pkt.stream_index==audio_index_){
            audio_queue_->Push(&pkt);
            LogInfo("audio pkt queue size:%d",audio_queue_->Size());
        }else if(pkt.stream_index==video_index_){
            video_queue_->Push(&pkt);
            LogInfo("video pkt queue size:%d",video_queue_->Size());
        }else{
            av_packet_unref(&pkt);//释放包
        }
    }
    LogInfo("run finish");

}
