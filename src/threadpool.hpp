#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include "util.hpp"


class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
    std::condition_variable finished_condition;
    std::atomic<int> active_tasks;
public:
    ThreadPool(int num_threads) : stop(false) {
        for(int i = 0; i < num_threads; ++i) {
            workers.emplace_back([this]() {
                while (true)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                        });
                        if(this->stop && this->tasks.empty()) return;

                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                        ++active_tasks;
                    }
                    task();
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        --active_tasks;
                        if(tasks.empty() && active_tasks == 0) {
                            finished_condition.notify_all();
                        }
                    }
                }                
            });
        }
    }
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;


    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for(std::thread &worker : workers) {
            worker.join();
        }
    }

    template<class F, class... Args>
    void enqueue(F &&f, Args &&...args) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace([f, args...]() {
                f(args...);
            });
        }

        condition.notify_one();
    }

    void wait() {
        std::unique_lock<std::mutex> lock(queue_mutex);
        finished_condition.wait(lock, [this]() {
            return tasks.empty() && active_tasks == 0;
        });
    }


};




#endif // !THREADPOOL_HPP