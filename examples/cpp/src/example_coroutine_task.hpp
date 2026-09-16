#ifndef QNBOT_CPP_EXAMPLE_COROUTINE_TASK_HPP
#define QNBOT_CPP_EXAMPLE_COROUTINE_TASK_HPP

#include <qnbot/coro.hpp>

#include <condition_variable>
#include <coroutine>
#include <deque>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

namespace example {

class QueueExecutor final : public qnbot::coro::DeferredExecutor {
private:
    struct State {
        std::mutex mutex;
        std::condition_variable ready;
        std::deque<std::function<void()>> tasks;
        bool stopping{false};
    };

public:
    QueueExecutor()
        : state_(std::make_shared<State>()),
          worker_([state = state_] { run(std::move(state)); }) {}

    ~QueueExecutor() override {
        {
            std::lock_guard<std::mutex> lock(state_->mutex);
            state_->stopping = true;
        }
        state_->ready.notify_one();
        if (worker_.get_id() == std::this_thread::get_id())
            worker_.detach();
        else
            worker_.join();
    }

    void post(std::function<void()> task) noexcept override {
        try {
            {
                std::lock_guard<std::mutex> lock(state_->mutex);
                state_->tasks.push_back(std::move(task));
            }
            state_->ready.notify_one();
        } catch (...) {
            std::terminate();
        }
    }

private:
    static void run(std::shared_ptr<State> state) {
        for (;;) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(state->mutex);
                state->ready.wait(lock, [&] {
                    return state->stopping || !state->tasks.empty();
                });
                if (state->tasks.empty() && state->stopping) return;
                task = std::move(state->tasks.front());
                state->tasks.pop_front();
            }
            task();
        }
    }

    std::shared_ptr<State> state_;
    std::thread worker_;
};

template <typename T> class Task {
public:
    struct promise_type {
        std::promise<T> result;

        Task get_return_object() { return Task(result.get_future()); }
        std::suspend_never initial_suspend() const noexcept { return {}; }
        std::suspend_never final_suspend() const noexcept { return {}; }
        void return_value(T value) { result.set_value(std::move(value)); }
        void unhandled_exception() {
            result.set_exception(std::current_exception());
        }
    };

    explicit Task(std::future<T> result) : result_(std::move(result)) {}
    T get() { return result_.get(); }

private:
    std::future<T> result_;
};

} // namespace example

#endif
