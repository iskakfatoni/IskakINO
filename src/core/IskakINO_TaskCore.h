#ifndef ISKAKINO_TASKCORE_H
#define ISKAKINO_TASKCORE_H

#include <Arduino.h>
#include "IskakINO_Platform.h"
#include "IskakINO_Logger.h"
#include "IskakINO_Result.h"

#if defined(ESP32)

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <functional>

#define ISKAK_CORE_0 0
#define ISKAK_CORE_1 1

/**
 * @brief Thread-safe Mutex wrapper untuk sinkronisasi data antar-core FreeRTOS.
 */
class IskakINO_Mutex {
  public:
    IskakINO_Mutex() {
        _mutex = xSemaphoreCreateMutex();
    }

    ~IskakINO_Mutex() {
        if (_mutex) {
            vSemaphoreDelete(_mutex);
            _mutex = nullptr;
        }
    }

    bool lock(uint32_t waitMs = portMAX_DELAY) {
        if (!_mutex) return false;
        TickType_t ticks = (waitMs == portMAX_DELAY) ? portMAX_DELAY : pdMS_TO_TICKS(waitMs);
        return (xSemaphoreTake(_mutex, ticks) == pdTRUE);
    }

    void unlock() {
        if (_mutex) {
            xSemaphoreGive(_mutex);
        }
    }

    SemaphoreHandle_t handle() const { return _mutex; }

  private:
    SemaphoreHandle_t _mutex = nullptr;
};

/**
 * @brief RAII LockGuard untuk auto-lock dan auto-unlock mutex saat keluar scope.
 */
class IskakINO_LockGuard {
  public:
    explicit IskakINO_LockGuard(IskakINO_Mutex& mutex, uint32_t waitMs = portMAX_DELAY)
        : _mutex(mutex), _locked(false) {
        _locked = _mutex.lock(waitMs);
    }

    ~IskakINO_LockGuard() {
        if (_locked) {
            _mutex.unlock();
        }
    }

    bool isLocked() const { return _locked; }

  private:
    IskakINO_Mutex& _mutex;
    bool _locked;
};

/**
 * @brief Thread-safe inter-core FIFO Queue wrapper.
 */
template<typename T, size_t Capacity = 10>
class IskakINO_Queue {
  public:
    IskakINO_Queue() {
        _queue = xQueueCreate(Capacity, sizeof(T));
    }

    ~IskakINO_Queue() {
        if (_queue) {
            vQueueDelete(_queue);
            _queue = nullptr;
        }
    }

    bool push(const T& item, uint32_t waitMs = 0) {
        if (!_queue) return false;
        TickType_t ticks = pdMS_TO_TICKS(waitMs);
        return (xQueueSend(_queue, &item, ticks) == pdTRUE);
    }

    bool pop(T& outItem, uint32_t waitMs = 0) {
        if (!_queue) return false;
        TickType_t ticks = pdMS_TO_TICKS(waitMs);
        return (xQueueReceive(_queue, &outItem, ticks) == pdTRUE);
    }

    bool peek(T& outItem, uint32_t waitMs = 0) const {
        if (!_queue) return false;
        TickType_t ticks = pdMS_TO_TICKS(waitMs);
        return (xQueuePeek(_queue, &outItem, ticks) == pdTRUE);
    }

    size_t count() const {
        if (!_queue) return 0;
        return (size_t)uxQueueMessagesWaiting(_queue);
    }

    bool isEmpty() const { return (count() == 0); }
    bool isFull() const { return (count() >= Capacity); }
    void clear() {
        if (_queue) xQueueReset(_queue);
    }

  private:
    QueueHandle_t _queue = nullptr;
};

/**
 * @brief Pengelola task background terdedikasi di Core 0 atau Core 1 ESP32.
 */
class IskakINO_TaskCore {
  public:
    IskakINO_TaskCore(uint8_t core = ISKAK_CORE_0, uint32_t stackSize = 4096, uint8_t priority = 1);
    virtual ~IskakINO_TaskCore();

    bool begin(const char* taskName = "IskakTask");
    bool run(std::function<void()> taskFunc);
    bool runEvery(uint32_t intervalMs, std::function<void()> taskFunc);
    bool runOnce(std::function<void()> taskFunc);

    void pause();
    void resume();
    void stop();

    bool isRunning() const { return _isRunning; }
    bool isPaused() const { return _isPaused; }
    uint8_t getCoreID() const { return _core; }
    uint32_t getFreeStack() const;

    TaskHandle_t handle() const { return _taskHandle; }

  private:
    uint8_t  _core;
    uint32_t _stackSize;
    uint8_t  _priority;
    char     _name[24];

    TaskHandle_t _taskHandle = nullptr;
    volatile bool _isRunning = false;
    volatile bool _isPaused = false;
    volatile bool _isPeriodic = false;
    volatile bool _isOnce = false;
    uint32_t     _intervalMs = 0;

    std::function<void()> _userFunc = nullptr;

    static void taskTrampoline(void* param);
};

#else // Non-ESP32 Fallback Wrapper (Universal No-Op / Scheduler mapping)

class IskakINO_Mutex {
  public:
    bool lock(uint32_t = 0) { return true; }
    void unlock() {}
};

class IskakINO_LockGuard {
  public:
    explicit IskakINO_LockGuard(IskakINO_Mutex&) {}
    bool isLocked() const { return true; }
};

template<typename T, size_t Capacity = 10>
class IskakINO_Queue {
  public:
    bool push(const T& item, uint32_t = 0) {
        if (_count >= Capacity) return false;
        _buffer[_tail] = item;
        _tail = (_tail + 1) % Capacity;
        _count++;
        return true;
    }

    bool pop(T& outItem, uint32_t = 0) {
        if (_count == 0) return false;
        outItem = _buffer[_head];
        _head = (_head + 1) % Capacity;
        _count--;
        return true;
    }

    size_t count() const { return _count; }
    bool isEmpty() const { return (_count == 0); }
    bool isFull() const { return (_count >= Capacity); }

  private:
    T _buffer[Capacity];
    size_t _head = 0;
    size_t _tail = 0;
    size_t _count = 0;
};

class IskakINO_TaskCore {
  public:
    IskakINO_TaskCore(uint8_t = 0, uint32_t = 0, uint8_t = 0) {}
    bool begin(const char* = nullptr) { return true; }
    bool run(void (*)()) { return true; }
    bool runEvery(uint32_t, void (*)()) { return true; }
    bool runOnce(void (*)()) { return true; }
    void pause() {}
    void resume() {}
    void stop() {}
    bool isRunning() const { return false; }
    bool isPaused() const { return false; }
    uint8_t getCoreID() const { return 0; }
    uint32_t getFreeStack() const { return 0; }
};

#endif // defined(ESP32)

#endif // ISKAKINO_TASKCORE_H
