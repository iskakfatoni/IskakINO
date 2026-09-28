/*
 * src/core/IskakINO_RingBuffer.h
 * Circular FIFO Ring Buffer zero-heap (alokasi statis berbasis template).
 *
 * Sangat berguna untuk antrean stream byte UART/Serial, buffer sensor data,
 * komunikasi antar interupsi/task, dan pipeline pengolahan data tanpa malloc().
 */

#ifndef ISKAKINO_RINGBUFFER_H
#define ISKAKINO_RINGBUFFER_H

#include <Arduino.h>

template <typename T, size_t Capacity = 32>
class IskakINO_RingBuffer {
  public:
    IskakINO_RingBuffer() : _head(0), _tail(0), _count(0) {}

    // Memasukkan satu item ke antrean. Return false jika antrean penuh.
    bool push(const T& item) {
        if (_count >= Capacity) return false;
        _buffer[_tail] = item;
        _tail = (_tail + 1) % Capacity;
        _count++;
        return true;
    }

    // Mengambil satu item dari antrean ke outItem. Return false jika antrean kosong.
    bool pop(T& outItem) {
        if (_count == 0) return false;
        outItem = _buffer[_head];
        _head = (_head + 1) % Capacity;
        _count--;
        return true;
    }

    // Mengambil satu item tanpa menyalin ke argumen luar
    bool pop() {
        if (_count == 0) return false;
        _head = (_head + 1) % Capacity;
        _count--;
        return true;
    }

    // Mengintip item terdepan tanpa menghapusnya dari antrean
    bool peek(T& outItem) const {
        if (_count == 0) return false;
        outItem = _buffer[_head];
        return true;
    }

    // Mengintip item terdepan secara langsung (pastikan !isEmpty() sebelum memanggil)
    T peek() const {
        if (_count == 0) return T();
        return _buffer[_head];
    }

    // Mengintip item pada indeks offset relatif dari depan antrean (0 = terdepan)
    bool peekAt(size_t index, T& outItem) const {
        if (index >= _count) return false;
        size_t actualIdx = (_head + index) % Capacity;
        outItem = _buffer[actualIdx];
        return true;
    }

    // Jumlah item yang saat ini ada di antrean
    inline size_t count() const { return _count; }
    inline size_t available() const { return _count; }

    // Kapasitas maksimum buffer
    inline constexpr size_t capacity() const { return Capacity; }

    // Sisa slot kosong yang masih dapat diisi
    inline size_t freeSpace() const { return (Capacity - _count); }

    inline bool isEmpty() const { return (_count == 0); }
    inline bool isFull() const { return (_count >= Capacity); }

    // Mengosongkan seluruh antrean seketika
    void clear() {
        _head = 0;
        _tail = 0;
        _count = 0;
    }

  private:
    T      _buffer[Capacity];
    size_t _head;
    size_t _tail;
    size_t _count;
};

#endif // ISKAKINO_RINGBUFFER_H
