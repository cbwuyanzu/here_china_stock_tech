#ifndef MY_FIFO_QUEUE_NL
#define MY_FIFO_QUEUE_NL

#include <atomic>
#include <chrono>
#include <thread>

template<typename T>
class MyFifoQueueNL {
private:
    T* buffer_;
    size_t capacity_;

    // 读写索引
    std::atomic<size_t> read_index_;
    std::atomic<size_t> write_index_;

public:
    explicit MyFifoQueueNL(size_t capacity = 4096)
        : capacity_(capacity)  // 额外多分配一个槽位来区分空和满
        , read_index_(0)
        , write_index_(0) {

        buffer_ = new T[capacity_];
    }

    ~MyFifoQueueNL() {
        delete[] buffer_;
    }

    MyFifoQueueNL(const MyFifoQueueNL&) = delete;
    MyFifoQueueNL& operator=(const MyFifoQueueNL&) = delete;

    // 判断是否为空
    bool empty() const noexcept {
        // 使用memory_order_acquire确保看到最新的write_index_
        return read_index_.load(std::memory_order_acquire) ==
               write_index_.load(std::memory_order_acquire);
    }

    // 判断是否已满
    bool full() const noexcept {
        size_t read_idx = read_index_.load(std::memory_order_acquire);
        size_t write_idx = write_index_.load(std::memory_order_acquire);

        // 下一个写位置是否等于读位置
        return ((write_idx + 1) % capacity_) == read_idx;
    }

    // 获取队列大小
    size_t size() const noexcept {
        size_t write_idx = write_index_.load(std::memory_order_acquire);
        size_t read_idx = read_index_.load(std::memory_order_acquire);

        if (write_idx >= read_idx) {
            return write_idx - read_idx;
        } else {
            return capacity_ - (read_idx - write_idx);
        }
    }

    // 清空队列
    void clear() noexcept {
        read_index_.store(0, std::memory_order_release);
        write_index_.store(0, std::memory_order_release);
    }

    // 入队（拷贝）
    bool push(const T& value) {
        size_t write_idx = write_index_.load(std::memory_order_relaxed);
        size_t next_write_idx = (write_idx + 1) % capacity_;
        size_t read_idx = read_index_.load(std::memory_order_acquire);

        // 检查队列是否已满
        if (next_write_idx == read_idx) {
            return false;
        }

        // 写入数据
        buffer_[write_idx] = value;

        // 更新写索引
        write_index_.store(next_write_idx, std::memory_order_release);
        return true;
    }

    // 入队（移动）
    bool push(T&& value) {
        size_t write_idx = write_index_.load(std::memory_order_relaxed);
        size_t next_write_idx = (write_idx + 1) % capacity_;
        size_t read_idx = read_index_.load(std::memory_order_acquire);

        if (next_write_idx == read_idx) {
            return false;
        }

        buffer_[write_idx] = std::move(value);
        write_index_.store(next_write_idx, std::memory_order_release);
        return true;
    }


    // 出队（立即返回）
    bool pop(T& value) noexcept {
        size_t read_idx = read_index_.load(std::memory_order_relaxed);
        size_t write_idx = write_index_.load(std::memory_order_acquire);

        // 检查队列是否为空
        if (read_idx == write_idx) {
            return false;
        }

        // 读取数据
        value = std::move(buffer_[read_idx]);

        // 更新读索引
        size_t next_read_idx = (read_idx + 1) % capacity_;
        read_index_.store(next_read_idx, std::memory_order_release);
        return true;
    }

    // 带超时的出队
    bool try_pop(T& value, int timeoutMs) noexcept{
        auto timeout = std::chrono::milliseconds(timeoutMs);
        auto start_time = std::chrono::steady_clock::now();

        while (true) {
            if (pop(value)) {
                return true;
            }

            // 检查是否超时
            auto current_time = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                current_time - start_time);

            if (elapsed >= timeout) {
                return false;
            }

            // 简单等待
            std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
        }
    }

    // 查看队首元素（不弹出）
    bool front(T& value) const noexcept {
        size_t read_idx = read_index_.load(std::memory_order_acquire);
        size_t write_idx = write_index_.load(std::memory_order_acquire);

        if (read_idx == write_idx) {
            return false;
        }

        value = buffer_[read_idx];
        return true;
    }

};

#endif // MY_FIFO_QUEUE_NL