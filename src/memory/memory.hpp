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

  static Buffer *create(std::size_t s, std::size_t alignment = 1024) {
    alignment = std::min(s, alignment);
    // Align the whole block to max(alignment, alignof(Buffer))
    std::size_t buffer_align = std::max(alignment, alignof(Buffer));
    std::size_t total_size = sizeof(Buffer) + s + buffer_align;

    void *raw = std::aligned_alloc(buffer_align, total_size);
    if (!raw)
      throw std::bad_alloc();

    // Placement-new the Buffer object
    Buffer *buf = new (raw) Buffer();
    std::byte *raw_bytes = reinterpret_cast<std::byte *>(new (raw) Buffer());

    // Compute aligned pointer for buffer data after Buffer struct
    std::size_t struct_end = reinterpret_cast<std::size_t>(raw_bytes + sizeof(Buffer));
    std::size_t aligned_data = (struct_end + alignment - 1) & ~(alignment - 1);
    buf->ptr = reinterpret_cast<std::byte *>(aligned_data);
    buf->size = s;

    return buf;
  }
};

class ArenaAllocator {
public:
  explicit ArenaAllocator(std::size_t size) {
    buffer = Buffer::create(size);
    head = buffer;
  }

  void *alloc(std::size_t size, std::size_t alinment = alignof(std::max_align_t)) {
    if ((size > buffer->size / 4) || (alinment > buffer->size / 4)) {
      Buffer *new_buffer = Buffer::create(size, alinment);
      new_buffer->next = buffer->next;
      buffer->next = new_buffer;
      return new_buffer->ptr;
    }

    while (true) {
      std::size_t aligned_offset = (offset + (alinment - 1)) & ~(alinment - 1);

      if (aligned_offset + size > buffer->size) {
        if (buffer->next) {
          buffer = buffer->next;
          offset = 0;
          continue;
        }
        buffer->next = Buffer::create(buffer->size);
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
      std::free(buffer);
      buffer = next;
    }
  }

private:
  std::size_t offset = 0;
  Buffer *buffer = nullptr;
  Buffer *head = nullptr;
};
} // namespace Allocator
