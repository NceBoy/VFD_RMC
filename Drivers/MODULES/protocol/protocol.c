#include "protocol.h"
#include <string.h>
#include <stdint.h>
#include <assert.h>


//malloc要实现以下
void* protocol_malloc(uint32_t len) {
    return NULL;
}

void protocol_free(void* ptr) {

}

void create_packet(Packet* packet, ActionCode action, MessageType type, uint16_t source_id, uint16_t target_id, uint16_t subtype, uint8_t* body, uint16_t body_length) {
    packet->header = HEADER_VALUE;
    packet->action = (uint8_t)action;
    packet->type = (uint8_t)type;
    packet->reserved = 0; // 预留字段
    packet->source_id = source_id; // 小端存储
    packet->target_id = target_id; // 小端存储
    packet->subtype = subtype; // 小端存储
    packet->body_length = body_length; // 小端存储
    if(packet->body_length)
    {
        packet->body = (uint8_t*)protocol_malloc(body_length);
        if(packet->body)
            memcpy(packet->body, body, body_length);  
        else 
            packet->body_length = 0;
    }

    // 计算CRC时，从 header 到 crc 之前的字段都需要参与计算
    uint8_t* data = (uint8_t*)packet;
    uint16_t crc = calculate_modbus_crc_2(data,body, 12,packet->body_length);
    packet->crc = crc; // 存储为小端
    packet->footer = FOOTER_VALUE;
}

int serialize_packet(const Packet* packet, uint8_t* buffer) {
    uint8_t* temp = buffer;
    *temp++ = packet->header;
    *temp++ = packet->type;
    *temp++ = packet->action;
    *temp++ = packet->reserved; // 预留字段
    // 源设备ID和目标设备ID以小端存储
    *temp++ = (packet->source_id >> 0) & 0xFF;  // 低字节
    *temp++ = (packet->source_id >> 8) & 0xFF;  // 高字节

    *temp++ = (packet->target_id >> 0) & 0xFF;  // 低字节
    *temp++ = (packet->target_id >> 8) & 0xFF;  // 高字节

    *temp++ = (packet->subtype >> 0) & 0xFF;    // 低字节
    *temp++ = (packet->subtype >> 8) & 0xFF;    // 高字节

    *temp++ = (packet->body_length >> 0) & 0xFF; // 低字节
    *temp++ = (packet->body_length >> 8) & 0xFF; // 高字节

    if(packet->body_length)
    {
        memcpy(temp, packet->body, packet->body_length);
        protocol_free(packet->body);
    }
    temp += packet->body_length;
    
    *temp++ = (packet->crc >> 0) & 0xFF;        // 低字节
    *temp++ = (packet->crc >> 8) & 0xFF;        // 高字节

    *temp++ = packet->footer;
    return temp - buffer;
}

uint16_t deserialize_packet_byte(CallbackRead funcRead, int timeout, Packet* packet) {
    uint8_t* data = (uint8_t*)packet;
    uint16_t expected_crc;
    if (funcRead == NULL)
        goto exit2;

    // 读取并判断包头
    if (funcRead(&packet->header, 1, timeout) != 1)
        goto exit2;
    if (packet->header != HEADER_VALUE)
        goto exit2;

    // 读取动作、类型、预留字段、源设备ID、目标设备ID、源设备ID和目标设备ID
    if (funcRead((uint8_t*)&packet->action, 11, timeout) != 11) // 包括 action, type, reserved, source_id, target_id,subtype, body_length
        goto exit2;

    // 读取消息体
    packet->body = (uint8_t*)protocol_malloc(packet->body_length);
    if (packet->body == NULL)
        goto exit2;
    if (funcRead(packet->body, packet->body_length, timeout) != packet->body_length)
        goto exit1;

    // 读取CRC和包尾
    uint8_t crc_buffer[2]; // 缓存两个字节的CRC
    if (funcRead(crc_buffer, 2, timeout) != 2)
        goto exit1;
    packet->crc = (crc_buffer[1] << 8) | crc_buffer[0]; 
    // 验证CRC，无需转换大小端
    data = (uint8_t*)packet;
    expected_crc = calculate_modbus_crc_2(data,packet->body, 12,packet->body_length);
    if (packet->crc != expected_crc)
        goto exit1;

    if (funcRead(&packet->footer, 1, timeout) != 1)
        goto exit1;

    // 验证包头和包尾
    if (packet->footer != FOOTER_VALUE)
        goto exit1;

    return packet->body_length + 15; // 包头、动作、类型、预留、源设备ID、目标设备ID、子类型、消息体长度、CRC、包尾
exit1:
    protocol_free(packet->body);
exit2:
    return 0;
}

uint16_t deserialize_packet(const uint8_t* buffer, uint16_t length , Packet* packet) {

    if(length < 15)
        return 0;

    const uint8_t* temp = buffer;
    uint16_t body_length = temp[10] | (temp[11] << 8);
    //验证包长
    if(length != (body_length + 15))
        return 0;
    //验证包头和包尾
    if (temp[0] != HEADER_VALUE || temp[length -1] != FOOTER_VALUE) 
        return 0;
    //验证CRC
    uint16_t crc_pack = temp[12 + body_length] | (temp[13 + body_length] << 8);
    uint16_t expected_crc = calculate_modbus_crc(temp, 12 + body_length);
    if (crc_pack != expected_crc)
        return 0;

    packet->header = *temp++;
    packet->type = *temp++;
    packet->action = *temp++;

    packet->reserved = *temp++; // 预留字段

    // 源设备ID和目标设备ID
    packet->source_id = (temp[1] << 8) | temp[0];
    temp += 2;
    packet->target_id = (temp[1] << 8) | temp[0];
    temp += 2;

    // 子类型和消息体长度
    packet->subtype = (temp[1] << 8) | temp[0];
    temp += 2;
    packet->body_length = (temp[1] << 8) | temp[0];
    temp += 2;

    if(packet->body_length != 0)
    {
        packet->body = (uint8_t*)protocol_malloc(packet->body_length);
        memcpy(packet->body, temp, packet->body_length);        
    }

    temp += packet->body_length;

    // CRC和包尾
    packet->crc = (temp[1] << 8) | temp[0];
    temp += 2;
    packet->footer = *temp++;


    return 1;
}

uint16_t calculate_modbus_crc(const uint8_t* data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    uint16_t i, j;
    for (i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    // 返回小端格式的CRC值（十六进制字节顺序：低字节在前）
    return crc;
}

uint16_t calculate_modbus_crc_2(const uint8_t* data,uint8_t* data_body, uint16_t length,uint16_t lenth_body) {
    uint16_t crc = 0xFFFF;
    uint16_t i, j;
    for (i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    for (i = 0; i < lenth_body; i++) {
        crc ^= (uint16_t)data_body[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    // 返回小端格式的CRC值（十六进制字节顺序：低字节在前）
    return crc;
}

void print_packet(const Packet* packet) {
    //printf("Packet:\n");
    //printf("  Header: 0x%02X\n", packet->header);
    //printf("  Action: 0x%02X\n", packet->action);
    //printf("  Type: 0x%02X\n", packet->type);
    //printf("  Reserved: 0x%02X\n", packet->reserved);
    //printf("  Source ID: 0x%04X\n", packet->source_id);
    //printf("  Target ID: 0x%04X\n", packet->target_id);
    //printf("  Subtype: 0x%04X\n", packet->subtype);
    //printf("  Body Length: %d\n", packet->body_length);
    //printf("  Body: ");
    for (uint16_t i = 0; i < packet->body_length; i++) {
        //printf("%02X ", packet->body[i]);
    }
    //printf("\n");
    //printf("  CRC: 0x%04X\n", packet->crc);
    //printf("  Footer: 0x%02X\n", packet->footer);
}

void protocol_mem_init() {
  
}

// 新增函数：分配Packet内存
Packet* packet_allocate(uint16_t body_length) {
    if (body_length == 0) {
        return NULL;
    }
    Packet* packet = (Packet*)protocol_malloc(sizeof(Packet));
    if (packet == NULL) {
        return NULL;
    }
    packet->body = NULL;
    return packet;
}

// 新增函数：释放Packet内存
void packet_free(Packet* packet) {
    if (packet == NULL) {
        return;
    }
    if(packet->body != NULL)
        protocol_free(packet->body);
    protocol_free(packet);
}

void packet_body_free(Packet* packet)
{
    if(packet->body != NULL)
        protocol_free(packet->body);    
}
