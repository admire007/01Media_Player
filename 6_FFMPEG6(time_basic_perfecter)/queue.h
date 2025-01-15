#ifndef QUEUE_H
#define QUEUE_H
#include<mutex>//支持互斥
#include<condition_variable>//唤醒
#include<queue>


template<typename T>//模板

class Queue
{
public:
    Queue(){}
    ~ Queue(){}
    void Abort(){
        abort_=1;
        cond_.notify_all();//多线程唤醒，此次为一线程读一线程写
    }

    int Push(T val){
        //获取锁
        std::lock_guard<std::mutex>lock(mutex_);//自动释放
        if(1==abort_){//说明已经请求abort_不能插入
            return -1;
        }
        queue_.push(val);
        cond_.notify_one();//唤醒下一个线程
        return 0;
    }

    int Pop(T &val,const int timeout=0){//超时多久
        //获取锁
        //std::lock_guard<std::mutex>lock(mutex_);//自动释放
        std::unique_lock< std::mutex>lock(mutex_);
        if(queue_.empty()){
            //等待唤醒或超时唤醒
            cond_.wait_for(lock,std::chrono::milliseconds(timeout),[this]{
                return !queue_.empty() | abort_;
            });
        }
        if(1==abort_){
            return -1;
        }
        if(queue_.empty()){//还是为空
            return -2;
        }
        val=queue_.front();
        queue_.pop();
        return 0;
    }

    int Front(T &val){//查是否有数据可读
        //获取锁
        std::lock_guard<std::mutex>lock(mutex_);//自动释放
        if(1==abort_){
            return -1;
        }
        if(queue_.empty()){//还是为空
            return -2;
        }
        val=queue_.front();//尝试取 不用pop
        return 0;
    }

    int Size(){
        std::lock_guard<std::mutex>lock(mutex_);
        return queue_.size();
    }

private:
    int abort_=0;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::queue<T> queue_;
};

#endif // QUEUE_H
