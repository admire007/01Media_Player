#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H
#include"avframqueue.h"

#ifdef __cplusplus ///
extern "C"
{
// 包含ffmpeg头文件
//#include<libavutil/avutil.h>
#include"SDL.h"
#include"libswresample/swresample.h"//重采样
}
#endif

//定义结构体 重采样参数 ffmpeg acc(小数)   SDL acc(整数类型)
typedef struct AudioPrams
{
    int freq;//采样率
    int channels;
    int64_t channel_layout;//通道布局：立体声。。
    enum AVSampleFormat fmt;
    int frame_size;
}AudioPrams;

class AudioOutput
{
public:
    AudioOutput(const AudioPrams &audio_prams,AVFramQueue *frame_queue );
    ~AudioOutput();
    int Init();//初始化
    int DeInit();//释放


//private:
public:
    AudioPrams src_tgt;//解码后的参数 保存于audio_prams
    AudioPrams dst_tgt;//SDL实际输出的格式
    AVFramQueue *frame_queue=NULL;

    struct SwrContext *swr_ctx_=NULL;

    //一个索引buf读取到的位置 一个指针指向开头
    uint8_t *audio_buf_=NULL;
    uint8_t *audio_buf1_=NULL;

    //分配一些buf保存frame的PCM数据
    uint32_t audio_buf_size=0;
    uint32_t audio_buf1_size=0;
    uint32_t audio_buf1_index=0;// 索引




};

#endif // AUDIOOUTPUT_H
