//
// Created by amir on 23.07.24.
//

#ifndef MULTITHREADEDSERVER_QUEUE_SAFE_H
#define MULTITHREADEDSERVER_QUEUE_SAFE_H

#include<queue>
#include<mutex>
#include<memory>


/*!
 * Очередь с блокировкой одновременного доступа
 * @tparam T
 */
template<typename T>
class queue_safe{
public:

    bool empty(){
        std::lock_guard<std::mutex> lk(mu);
        return queue.empty();
    }

    void push(T& data){
        std::lock_guard<std::mutex> lk(mu);
        queue.push(data);
    }

    std::shared_ptr<T> front_pop(){
        std::lock_guard<std::mutex> lk(mu);
        if(!queue.empty()){
            std::shared_ptr<T> tmp = std::make_shared<T>(queue.front());
            queue.pop();
            return tmp;
        }
        else
            return nullptr;
    }

    void pop(){
        std::lock_guard<std::mutex> lk(mu);
        queue.pop();
    }

    T& front(){
        std::lock_guard<std::mutex> lk(mu);
        queue.front();
    }

    size_t size(){
        std::lock_guard<std::mutex> lk(mu);
        return queue.size();
    }



private:
    std::mutex mu;
    std::queue<T> queue;
};

#endif //MULTITHREADEDSERVER_QUEUE_SAFE_H
