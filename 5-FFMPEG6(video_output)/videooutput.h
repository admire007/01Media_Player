#ifndef VIDEOOUTPUT_H
#define VIDEOOUTPUT_H

#include"avframqueue.h"

#ifdef __cplusplus ///
extern "C"
{
// 包含ffmpeg头文件
//#include<libavutil/avutil.h>
#include"SDL.h"

}
#endif


class video_output
{
public:
    video_output(AVFramQueue *frame_queue,int video_width,int video_hight);
    int Init();
    int MainLoop();//退出则整个程序退出
    void RefreshLoopWaitEvent(SDL_Event *event);

private:
    void videoRefresh(double *remaining_time);
    AVFramQueue *frame_queue_=NULL;
    SDL_Event even_t;//事件
    SDL_Rect rect_;//显示对应区域
    SDL_Window *win_=NULL;
    SDL_Renderer *renderer_=NULL;//渲染器
    SDL_Texture *texture_=NULL;//要渲染的纹理
    //SDL_Rectv *srcrect 要渲染的纹理部分

    int video_width_=0;
    int video_hight_=0;
    uint8_t *yuv_buf_=NULL;
    int yuv_buf_size_=0;

    //SDL_mutex mutex_;

};

#endif // VIDEOOUTPUT_H
