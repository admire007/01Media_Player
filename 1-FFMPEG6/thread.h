#ifndef THREAD_H
#define THREAD_H

#include<thread>

class Thread
{
public:
    Thread(){}//线程函数
    ~Thread(){
        if(thread_){//如果析构thread时线程非空，则忘记调用stop函数
            Thread::Stop();//调用自己的函数

        }
    }//析构
    int Start(){}
    int Stop(){
        int abort_=1;//通过abort_控制线程退出
        if(thread_){
            thread_->join();//?
            delete thread_;
            thread_ =NULL;
        }
        return 0;
    }
    virtual void Run()=0;//运行执行函数

protected:
    int abort_=0;
    std::thread *thread_=nullptr;//线程指针
};

#endif // THREAD_H
