#include "queue.h"
#include <string.h>  // 添加 memcpy 函数的头文件


// 静态变量
static bool packet_start = false;

// 初始化队列
void queue_init(Queue* queue) {
    queue->write_index = 0;
    queue->read_index = 0;
    queue->count = 0;
    
    for (int i = 0; i < QUEUE_SIZE; i++) {
        queue->buffers[i].length = 0;
        queue->buffers[i].complete = false;
    }
    
    packet_start = false;
}

// 获取当前写入缓冲区
Buffer* get_current_buffer(Queue* queue) {
    return &queue->buffers[queue->write_index];
}

// 将完整的数据包写入队列
bool queue_write_packet(Queue* queue, uint8_t* data, uint16_t length) {
    // 如果队列已满，则无法写入
    if (queue->count >= QUEUE_SIZE) {
        return false;
    }
    
    // 检查数据长度是否超出缓冲区大小
    if (length > BUFFER_SIZE) {
        return false;
    }
    
    Buffer* current_buffer = &queue->buffers[queue->write_index];
    
    // 复制数据到缓冲区
    memcpy(current_buffer->data, data, length);
    current_buffer->length = length;
    current_buffer->complete = true;
    
    // 移动到下一个缓冲区
    queue->write_index = (queue->write_index + 1) % QUEUE_SIZE;
    queue->count++;
    
    return true;
}

// 将接收到的多个字节写入队列
bool queue_write_bytes(Queue* queue, uint8_t* data, uint16_t length) {
    // 如果队列已满且当前缓冲区已完成，则无法写入
    if (queue->count >= QUEUE_SIZE && queue->buffers[queue->write_index].complete) {
        return false;
    }
    
    Buffer* current_buffer = &queue->buffers[queue->write_index];
    
    // 检查缓冲区剩余空间
    uint16_t remaining_space = BUFFER_SIZE - current_buffer->length;
    
    // 如果剩余空间不足
    if (remaining_space < length) {
        // 先填充当前缓冲区
        if (remaining_space > 0) {
            memcpy(&current_buffer->data[current_buffer->length], data, remaining_space);
            current_buffer->length += remaining_space;
        }
        
        // 标记当前缓冲区为完成
        current_buffer->complete = true;
        
        // 移动到下一个缓冲区
        queue->write_index = (queue->write_index + 1) % QUEUE_SIZE;
        if (queue->count < QUEUE_SIZE) {
            queue->count++;
        } else {
            // 队列已满，无法继续写入剩余数据
            return false;
        }
        
        // 获取新的缓冲区
        current_buffer = &queue->buffers[queue->write_index];
        if (current_buffer->complete) {
            // 新缓冲区已被占用，无法继续写入
            return false;
        }
        
        // 重置新缓冲区
        current_buffer->length = 0;
        
        // 写入剩余数据
        uint16_t remaining_data = length - remaining_space;
        if (remaining_data > BUFFER_SIZE) {
            // 剩余数据仍然超出缓冲区大小，只写入部分
            memcpy(current_buffer->data, data + remaining_space, BUFFER_SIZE);
            current_buffer->length = BUFFER_SIZE;
            return false; // 数据未完全写入
        } else {
            // 写入所有剩余数据
            memcpy(current_buffer->data, data + remaining_space, remaining_data);
            current_buffer->length = remaining_data;
        }
    } else {
        // 缓冲区空间足够，直接写入
        memcpy(&current_buffer->data[current_buffer->length], data, length);
        current_buffer->length += length;
    }
    
    return true;
}

// 从队列中读取一个完整的数据包
bool queue_read(Queue* queue, Buffer* buffer) {
    if (queue->count == 0) {
        return false;
    }
    
    Buffer* read_buffer = &queue->buffers[queue->read_index];
    if (!read_buffer->complete) {
        return false;
    }
    
    // 复制数据（使用memcpy替代循环）
    buffer->length = read_buffer->length;
    memcpy(buffer->data, read_buffer->data, read_buffer->length);
    buffer->complete = true;
    
    // 清空已读取的缓冲区
    read_buffer->length = 0;
    read_buffer->complete = false;
    
    // 更新读索引
    queue->read_index = (queue->read_index + 1) % QUEUE_SIZE;
    queue->count--;
    
    return true;
}