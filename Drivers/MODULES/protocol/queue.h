#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <stdbool.h>

// 定义通用缓冲区结构
#define BUFFER_SIZE 128
#define QUEUE_SIZE 8

typedef struct {
    uint8_t data[BUFFER_SIZE];
    uint16_t length;
    bool complete;  // 标记是否接收到完整数据包
} Buffer;

typedef struct {
    Buffer buffers[QUEUE_SIZE];
    uint8_t write_index;
    uint8_t read_index;
    uint8_t count;
} Queue;

// 函数声明
void queue_init(Queue* queue);
Buffer* get_current_buffer(Queue* queue);
bool queue_write_packet(Queue* queue, uint8_t* data, uint16_t length);
bool queue_write_bytes(Queue* queue, uint8_t* data, uint16_t length);
bool queue_read(Queue* queue, Buffer* buffer);

#endif // QUEUE_H