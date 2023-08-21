#if !defined(PARA_H_INCLUDED)
#define PARA_H_INCLUDED

/******************************************************************************

MIT License

Copyright (c) 2023 Andrea Griffini

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

******************************************************************************/

#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

struct Para {
    std::atomic_int npara;
    int started, done;
    std::condition_variable wcv;

    std::mutex mm;
    std::condition_variable mcv;

    struct Worker;
    std::vector<Worker *> workers;

    std::function<void()> f;

    Para() : npara(0) {
        for (int i=0; i<int(std::thread::hardware_concurrency()); i++) {
            workers.push_back(new Worker(this));
        }
    }

    Para(const Para&) = delete;
    Para(Para&&) = delete;
    Para& operator=(const Para&) = delete;
    Para& operator=(Para&&) = delete;

    struct Worker {
        Para *para;
        enum { Idle, Ready, Running, Stopping, Stopped } state;
        std::mutex m;
        std::thread t;

        Worker(Para *para) : para(para), state(Idle) {
            t = std::thread([this](){ run(); });
        }

        void run() {
            std::unique_lock<std::mutex> lk(m);
            for(;;) {
                para->wcv.wait(lk, [this]{ return state != Idle; });
                if (state == Stopping) {
                    state = Stopped;
                    std::lock_guard<std::mutex> g(para->mm);
                    if (++para->done == para->started) {
                        para->mcv.notify_one();
                    }
                    break;
                }
                state = Running;
                para->f();
                state = Idle;
                std::lock_guard<std::mutex> g(para->mm);
                if (++para->done == para->started) {
                    para->mcv.notify_one();
                }
            }
        }
    };

    void irun(std::function<void()> f) {
        if (++npara != 1) {
            f();
        } else {
            done = 0;
            this->f = f;
            started = workers.size();
            done = 0;
            for (auto& w : workers) {
                w->m.lock();
                w->state = Worker::Ready;
                w->m.unlock();
            }
            std::unique_lock<std::mutex> lk(mm);
            wcv.notify_all();
            mcv.wait(lk, [this](){ return done == started; });
        }
        --npara;
    }

    static void run(std::function<void()> f) {
        static Para para;
        para.irun(f);
    }

    ~Para() {
        started = workers.size();
        done = 0;
        for (auto& w : workers) {
            w->m.lock();
            w->state = Worker::Stopping;
            w->m.unlock();
        }
        std::unique_lock<std::mutex> lk(mm);
        wcv.notify_all();
        mcv.wait(lk, [this](){ return done == started; });
        for (auto& w : workers) {
            w->t.join();
            delete w;
        }
    }
};

#endif
