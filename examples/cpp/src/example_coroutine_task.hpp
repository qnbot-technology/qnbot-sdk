#ifndef QNBOT_CPP_EXAMPLE_COROUTINE_TASK_HPP
#define QNBOT_CPP_EXAMPLE_COROUTINE_TASK_HPP

#include <qnbot/coro.hpp>

#include <condition_variable>
#include <cstddef>
#include <coroutine>
#include <deque>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
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

    ~QueueExecutor() noexcept override {
        {
            std::lock_guard<std::mutex> lock(state_->mutex);
            state_->stopping = true;
        }
        state_->ready.notify_one();
        if (!worker_.joinable()) return;
        if (worker_.get_id() == std::this_thread::get_id()) {
            worker_.detach();
        } else {
            worker_.join();
        }
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
    static void run(std::shared_ptr<State> state) noexcept {
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

class DispatchState {
public:
    void begin() noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        posted_ = true;
        ++in_flight_;
    }

    void complete() noexcept {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            --in_flight_;
        }
        returned_.notify_all();
    }

    void wait_returned() noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!posted_) return;
        returned_.wait(lock, [&] { return in_flight_ == 0; });
    }

private:
    std::mutex mutex_;
    std::condition_variable returned_;
    std::size_t in_flight_{0};
    bool posted_{false};
};

class GatedExecutor final : public qnbot::coro::DeferredExecutor {
public:
    GatedExecutor(std::shared_ptr<QueueExecutor> executor,
                  std::shared_ptr<DispatchState> state) noexcept
        : executor_(std::move(executor)), state_(std::move(state)) {}

    void post(std::function<void()> task) noexcept override {
        state_->begin();
        auto state = state_;
        executor_->post([state = std::move(state),
                         task = std::move(task)]() mutable noexcept {
            struct Returned {
                std::shared_ptr<DispatchState> state;
                ~Returned() { state->complete(); }
            } returned{state};
            task();
        });
    }

private:
    std::shared_ptr<QueueExecutor> executor_;
    std::shared_ptr<DispatchState> state_;
};

struct TaskDispatch {
    std::shared_ptr<qnbot::coro::DeferredExecutor> executor;
    std::shared_ptr<DispatchState> state;
};

inline TaskDispatch
make_task_dispatch(std::shared_ptr<QueueExecutor> executor) {
    auto state = std::make_shared<DispatchState>();
    auto gated = std::make_shared<GatedExecutor>(std::move(executor), state);
    return {std::move(gated), std::move(state)};
}

template <typename T> class Task {
public:
    struct promise_type;
    using Handle = std::coroutine_handle<promise_type>;

    explicit Task(Handle handle) noexcept : handle_(handle) {}
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    Task(Task&& other) noexcept
        : handle_(std::exchange(other.handle_, {})),
          dispatch_(std::move(other.dispatch_)) {}
    Task& operator=(Task&&) = delete;

    ~Task() {
        if (handle_) {
            wait_until_frame_safe();
            destroy_frame();
        }
    }

    void bind_dispatch(std::shared_ptr<DispatchState> dispatch) noexcept {
        dispatch_ = std::move(dispatch);
    }

    T get() {
        wait_until_frame_safe();
        auto& promise = handle_.promise();
        const auto error = promise.error;
        auto value = std::move(promise.value);
        destroy_frame();
        handle_ = {};
        dispatch_.reset();
        if (error) std::rethrow_exception(error);
        return std::move(*value);
    }

    struct promise_type {
        std::mutex mutex;
        std::condition_variable finished;
        std::optional<T> value;
        std::exception_ptr error;
        bool done{false};

        Task get_return_object() noexcept {
            return Task(Handle::from_promise(*this));
        }
        std::suspend_never initial_suspend() const noexcept { return {}; }

        struct FinalSuspend {
            bool await_ready() const noexcept { return false; }
            void await_suspend(Handle handle) const noexcept {
                auto& promise = handle.promise();
                {
                    std::lock_guard<std::mutex> lock(promise.mutex);
                    promise.done = true;
                }
                promise.finished.notify_all();
            }
            void await_resume() const noexcept {}
        };

        FinalSuspend final_suspend() const noexcept { return {}; }
        void return_value(T result) { value.emplace(std::move(result)); }
        void unhandled_exception() noexcept {
            error = std::current_exception();
        }
    };

private:
    void destroy_frame() noexcept { handle_.destroy(); }

    void wait_until_frame_safe() noexcept {
        auto& promise = handle_.promise();
        {
            std::unique_lock<std::mutex> lock(promise.mutex);
            promise.finished.wait(lock, [&] { return promise.done; });
        }
        if (dispatch_) dispatch_->wait_returned();
    }

    Handle handle_;
    std::shared_ptr<DispatchState> dispatch_;
};

template <typename T>
Task<T> bind_task_dispatch(Task<T> task,
                           const TaskDispatch& dispatch) noexcept {
    task.bind_dispatch(dispatch.state);
    return task;
}

} // namespace example

#endif
