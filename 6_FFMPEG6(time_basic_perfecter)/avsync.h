#ifndef AVSYNC_H
#define AVSYNC_H

#include <chrono>
#include <ctime>
#include <math.h>
using namespace std::chrono;//获取时钟方式
class AVSync
{
public:
    AVSync() {

    }

    //1. 初始化时钟
    void InitClock() {
        SetClock(NAN); // 数学对比是一个无效值
    }

    //3. 设置时钟（单位--秒）
    void SetClockAt(double pts, double time) {
        pts_ = pts;
        pts_drift_ = pts_ - time;//time：实时时间
    }

    //4.  视频 动态 获取时钟
    double GetClock() {
        double time = GetMicroseconds() / 1000000.0;
        return pts_drift_ + time;
    }

    //2. 对外提供  以audio（pts转换为秒）为时钟设置
    void SetClock(double pts) {
        double time = GetMicroseconds() / 1000000.0; // us -> s
        SetClockAt(pts, time);
    }


    //获取微秒
    time_t GetMicroseconds() {
        system_clock::time_point time_point_new = system_clock::now();  // 时间一直动  获取当前时间
        system_clock::duration duration = time_point_new.time_since_epoch();//转成对应微秒

        time_t us = duration_cast<microseconds>(duration).count();//对应的微秒转换
        return us;
    }

    double pts_ = 0;
    double pts_drift_ = 0;//相对时间戳
};

#endif // AVSYNC_H
