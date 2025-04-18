#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <new>

namespace Allocator {
struct Buffer {
  std::size_t size = 0;
  Buffer *next = 0;
  std::byte *ptr = 0;

  Buffer(std::size_t s, std::size_t alignment = 1024) {
    alignment = std::min(s, alignment);
    ptr = static_cast<std::byte *>(std::aligned_alloc(alignment, s));
    if (!ptr)
      throw std::bad_alloc();
    size = s;
  }
};

class ArenaAllocator {
public:
  explicit ArenaAllocator(std::size_t size) : capacity(size) {
    buffer = new Buffer(size);
    head = buffer;
  }

  void *alloc(std::size_t size,
              std::size_t alinment = alignof(std::max_align_t)) {
    if ((size > capacity / 4) || (alinment > capacity / 4)) {
      Buffer *large = new Buffer(size, alinment);
      large->next = buffer;
      buffer = large;
      return large->ptr;
    }

    while (true) {
      std::size_t aligned_offset = (offset + (alinment - 1)) & ~(alinment - 1);

      if (aligned_offset + size > capacity) {
        if (buffer->next) {
          buffer = buffer->next;
          offset = 0;
          continue;
        }
        buffer->next = new Buffer(capacity * 2);
        buffer = buffer->next;
        offset = 0;
        continue;
      }

      void *ptr = buffer->ptr + aligned_offset;
      offset = aligned_offset + size;
      return ptr;
    }
  }

  template <typename T, typename... Args> T *emplace(Args &&...args) {
    auto p = static_cast<T *>(alloc(sizeof(T), alignof(T)));
    new (p) T(std::forward<Args>(args)...);
    return p;
  }

  void reset() {
    offset = 0;
    buffer = head;
  }

  ~ArenaAllocator() {
    reset();
    while (buffer != nullptr) {
      auto next = buffer->next;
      free(buffer->ptr);
      delete buffer;
      buffer = next;
    }
  }

private:
  std::size_t capacity = 0;
  std::size_t offset = 0;
  Buffer *buffer = nullptr;
  Buffer *head = nullptr;
};
} // namespace Allocator
