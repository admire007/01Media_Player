#include "videooutput.h"
#include"log.h"
#include"thread"

video_output::video_output(AVSync *avsync,AVRational time_base,AVFramQueue *frame_queue, int video_width, int video_hight)
    :avsync_(avsync),time_base_(time_base),
    frame_queue_(frame_queue),video_width_(video_width),video_hight_(video_hight)
    {
}

    int video_output::Init()
{
    //？？
    if(SDL_Init(SDL_INIT_VIDEO)){
        LogError("SDL_Init failed\n");
        return -1;
    }

    win_=SDL_CreateWindow("player",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,
                            video_width_,video_hight_,SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if(!win_){
        return -1;
    }

    renderer_=SDL_CreateRenderer(win_,-1,0);
    if(!renderer_){
        return -1;
    }

    //SDL_PIXELFORMAT_IYUV =      /**< Planar mode: Y + U + V  (3 planes) */
    //    SDL_DEFINE_PIXELFOURCC('I', 'Y', 'U', 'V'),
    texture_=SDL_CreateTexture(renderer_,SDL_PIXELFORMAT_IYUV,SDL_TEXTUREACCESS_STREAMING,video_width_,video_hight_);
    if(!texture_){
        return -1;
    }

    yuv_buf_size_=video_width_*video_hight_*1.5;
    yuv_buf_=(uint8_t*)malloc(yuv_buf_size_);


//faild:{
        //释放资源
//}

}


int video_output::MainLoop()
{
    SDL_Event event;
    while (true) {
        // 读取事件
        RefreshLoopWaitEvent(&event);

        switch (event.type) {
        case SDL_KEYDOWN://键盘响应
            if(event.key.keysym.sym == SDLK_ESCAPE) {
                LogInfo("esc key down");
                return 0;
            }
            break;
        case SDL_QUIT://退出事件
            LogInfo("SDL_QUIT");
            return 0;
            break;
        default:
            break;
        }
    }
}


#define REFRESH_RATE 0.01
void video_output::RefreshLoopWaitEvent(SDL_Event *event)
{
    double remaining_time = 0.0;
    SDL_PumpEvents();
    while (!SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {//没有时间产生（退出等）
        if (remaining_time > 0.0)//秒  没到音视频同步
            std::this_thread::sleep_for(std::chrono::milliseconds(int64_t(remaining_time * 1000.0)));//休眠
        remaining_time = REFRESH_RATE;
        // 尝试刷新画面
        videoRefresh(&remaining_time);
        SDL_PumpEvents();
    }
}


//音视频同步
void video_output::videoRefresh(double *remaining_time)//每次传入0.01秒
{

    AVFrame *frame = NULL;
    frame = frame_queue_->Front();//有无消息
    if(frame) {
        double pts=frame->pts * av_q2d(time_base_);//通过time_base_转换成秒单位
        LogInfo("video pts:%0.3lf\n", pts);

        double diff = pts - avsync_->GetClock();

        if(diff > 0) {
            *remaining_time = FFMIN(*remaining_time, diff);//#define FFMIN(a,b) ((a) > (b) ? (b) : (a)) 取小
            return;
        }



        // 有就渲染
        rect_.x = 0;
        rect_.y = 0;
        rect_.w = video_width_;
        rect_.h = video_hight_;

        //支持yuv420p
        SDL_UpdateYUVTexture(texture_, &rect_, frame->data[0], frame->linesize[0],//Y
                             frame->data[1], frame->linesize[1],//U
                             frame->data[2], frame->linesize[2]);//V
        SDL_RenderClear(renderer_);
        SDL_RenderCopy(renderer_, texture_, NULL, &rect_);
        SDL_RenderPresent(renderer_);
        frame = frame_queue_->Pop(1);
        av_frame_free(&frame);
    }
}

