#include "audiooutput.h"
#include"log.h"



AudioOutput::AudioOutput(const AudioPrams &audio_prams, AVFramQueue *frame_queue)
    :src_tgt(audio_prams),frame_queue(frame_queue)
{

}

AudioOutput::~AudioOutput()
{

}

void fill_audio_pcm(void *udata,Uint8 *stream,int len){//len:读多少数据 拷贝到stream

//1.判断是否从frame—_queue读取解码后的数据，填充到stream
//2.len=4000 frame=6000字节，剩下的字节存储在？
    AudioOutput *is=(AudioOutput *)udata;//转换
    int len1=0;//剩余的长度
    int audio_size=0;

while(len>0){//stream没读完
        if(is->audio_buf1_index==is->audio_buf1_size){//读完了，再从frame_queue读数据
        is->audio_buf1_index=0;//重置

        AVFrame *frame=is->frame_queue->Pop(10);//超时10毫秒
        if(frame){//读到解码后的数据
                //如何判断要不要重采用？ 格式？采样率？通道布局
            if((frame->format!=is->dst_tgt.fmt)
                    ||(frame->sample_rate!=is->dst_tgt.freq)
                    ||(frame->channel_layout!=is->dst_tgt.channel_layout)
                    &&(!is->swr_ctx_)){//不支持重采样器（支支持一次重采样器）
                    //
                    is->swr_ctx_=swr_alloc_set_opts(NULL,
                                                  is->dst_tgt.channel_layout,
                                                  (enum AVSampleFormat)is->dst_tgt.fmt,
                                                  is->dst_tgt.freq,//输出格式
                                                  frame->channel_layout,//解码后源格式
                                                  (enum AVSampleFormat)frame->format,
                                                  frame->sample_rate,
                                                  0,NULL);

                    //判断能否正常打开
                    if(!is->swr_ctx_ || swr_init(is->swr_ctx_)<0){
                        LogError(
                            "Cannot creat sample rate converter for conversion of %d Hz  %s %d channels to %d Hz %s %d channels\n",
                            frame->sample_rate,
                            av_get_sample_fmt_name((enum AVSampleFormat)frame->format),
                            frame->channels,
                            is->dst_tgt.freq,
                            av_get_sample_fmt_name((enum AVSampleFormat)is->dst_tgt.fmt),
                            is->dst_tgt.channels);
                            swr_free((SwrContext **)(&is->swr_ctx_));
                            return;

                        }
                        if(is->swr_ctx_){//初始化重采样器，需重采样
                            const uint8_t **in=(const uint8_t **)frame->extended_data;//多通道>8
                            uint8_t **out=&is->audio_buf1_;//重采样后数据放到buf1

                            //重采样得到样本数
                            /*原：96K（1024个样本）---现：48K（512个样本）
                             *1024/96=10.6(ms)   512/48=10.6(ms)
                             *重采样不改变时间，改变频率，格式。。
                             */
                            int out_samples=frame->nb_samples*is->dst_tgt.freq/frame->sample_rate+256;//+256：会有缓存（缓冲）
                            //字节数
                            int out_bytes=av_samples_get_buffer_size(NULL,is->dst_tgt.channels,out_samples,is->dst_tgt.fmt,0);
                            if(out_bytes<0){
                                LogError("av_samples_get_buffer_size failed\n");
                                return ;
                            }
                            //输出的数据比原来多
                            av_fast_malloc(&is->audio_buf1_,&is->audio_buf1_size,out_bytes);//audio_buf1_size小于out_bytes,重新分配audio_buf1_
                            int len2=swr_convert(is->swr_ctx_,out,out_samples,in,frame->nb_samples);//返回样本数
                            if(len2<0){
                                LogError("swr_convert failed\n");
                                return;
                            }
                            is->audio_buf_=is->audio_buf1_;
                            is->audio_buf1_size=av_samples_get_buffer_size(NULL,is->dst_tgt.channels,len2,is->dst_tgt.fmt,1);//1:无需对齐，此处只是统计


                        }else{//没有重采样
                            //一帧内有多少个字节
                            audio_size=av_samples_get_buffer_size(NULL,frame->channels,frame->nb_samples,(enum AVSampleFormat)frame->format,1);
                            //frame buf传到buf
                            av_fast_malloc(&is->audio_buf1_,&is->audio_buf1_size,audio_size);
                            is->audio_buf_=is->audio_buf1_;
                            is->audio_buf_size=is->audio_buf1_size;//audio_buf1_size  可分配比buf_size更多内存
                            memcpy(is->audio_buf_,frame->data[0],audio_size);
                        }
                    }

                }
            av_frame_free(&frame);
            }else{
            //没有读到解码后数据
            is->audio_buf_=NULL;//指向空
            is->audio_buf_size=512;//填回0值
        }
    }
        len1=is->audio_buf1_size-is->audio_buf1_index;//剩余的长度：总长 - 读取到的索引位置
    if(len1>len)//只要len 却有len1
            len1=len;//拷贝len

    if(!is->audio_buf_){
        memset(stream,0,len1);
        }else{
            //真正拷贝有效的数据
            /*is->audio_buf_(从此处开始)+is->audio_buf1_index  拷贝len1长度到stream
             */
            memcpy(stream,is->audio_buf_+is->audio_buf1_index,len1);
        }
        len-=len1;
        stream+=len1;
        is->audio_buf1_index+=len1;
    }


int AudioOutput::Init()
{
    if(SDL_Init(SDL_INIT_AUDIO)!=0){
        LogError("SDL_Init failed\n");
        return -1;
    }

    SDL_AudioSpec wanted_spec,spec;//wanted_spec想要设置的参数 spc实际的
    wanted_spec.channels=src_tgt.channels;
    wanted_spec.freq=src_tgt.freq;//采样率
    wanted_spec.format=AUDIO_S16;//SDL支持格式（大小端问题）
    wanted_spec.silence=0;//静音
    wanted_spec.callback= fill_audio_pcm;//通过callback读取PCM数据
    wanted_spec.userdata=this;
    wanted_spec.samples=src_tgt.frame_size;//采样数量

    int ret=SDL_OpenAudio(&wanted_spec,&spec);
    if(ret<0){
        LogError("SDL_Init failed\n");
        return -1;
    }

    //保存
    dst_tgt.channels=spec.channels;
    dst_tgt.fmt=AV_SAMPLE_FMT_S16;
    dst_tgt.freq=spec.freq;
    dst_tgt.channel_layout=av_get_default_channel_layout(src_tgt.channels);
    //dst_tgt.frame_size=spec.size;??
    dst_tgt.frame_size=src_tgt.frame_size;//使用输入的src_tgt.frame_size
    SDL_PauseAudio(0);//回调后->开始
    LogInfo("AudioOutput::Init() leave\n");
}

int AudioOutput::DeInit()
{
    SDL_PauseAudio(1);
    SDL_CloseAudio();
    LogInfo("AudioOutput::DeInit() leave\n");
}
