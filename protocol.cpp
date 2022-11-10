#include <string>
#include <cstring>
#include <iostream>
#include <chrono>
#include <cassert>
#include "protocol.h"
#define VERSION 5
#define SEQNO 100


uint8_t convTo8bit(std::string message) {
    uint16_t value = 0;
    assert(message.size() == 8 && "Message size should be 8 bit");
    for(int i = 0; i < 8; i++) {
        assert(message[i] == '0' || message[i] == '1');
        value += ((1<<i)*(message[i] - '0'));
    }
    return value;
}

std::string conv8bit(uint8_t number) {
    std::string bit_conversion = "";
    for(int i = 0; i < 8; i++) 
        bit_conversion += '0';
    int index = 0;
    while(number) {
        if(number & 1) bit_conversion[index] = '1';
        number /= 2;
        index++;
    }
    return bit_conversion;
}

uint16_t convTo16bit(std::string message) {
    uint16_t value = 0;
    assert(message.size() == 16 && "Message size should be 16bit");
    for(int i = 0; i < 16; i++) {
        assert(message[i] == '0' || message[i] == '1');
        value += ((1<<i)*(message[i] - '0'));
    }
    return value;
}

std::string conv16bit(uint16_t number) {
    std::string bit_conversion = "";
    for(int i = 0; i < 16; i++) 
        bit_conversion += '0';
    int index = 0;
    while(number != 0) {
        if(number & 1) bit_conversion[index] = '1';
        number /= 2;
        index++;
    }
    return bit_conversion;
}

uint32_t convTo32bit(std::string message) {
    uint32_t value = 0;
    assert(message.size() == 32 && "Message size should be 32bit");
    for(int i = 0; i < 32; i++) {
        assert(message[i] == '0' || message[i] == '1');
        value += ((1<<i)*(message[i] - '0'));
    }
    return value;
}

std::string conv32bit(uint32_t number) {
    std::string bit_conversion = "";
    for(int i = 0; i < 32; i++) 
        bit_conversion += '0';
    int index = 0;
    while(number) {
        if(number & 1) bit_conversion[index] = '1';
        number /= 2;
        index++;
    }
    return bit_conversion;
}

uint64_t convTo64bit(std::string message) {
    uint64_t value = 0;
    assert(message.size() == 64 && "Message size should be 64bit");
    for(int i = 0; i < 64; i++) {
        assert(message[i] == '0' || message[i] == '1');
        value += ((1<<i)*(message[i] - '0'));
    }
    return value;
}

std::string conv64bit(uint64_t number) {
    std::string bit_conversion = "";
    for(int i = 0; i < 64; i++) 
        bit_conversion += '0';
    int index = 0;
    while(number) {
        if(number & 1) bit_conversion[index] = '1';
        number /= 2;
        index++;
    }
    return bit_conversion;
}
uint64_t getTimestamp() {
    std::chrono::high_resolution_clock m_clock;
    return std::chrono::duration_cast<std::chrono::milliseconds>
              (m_clock.now().time_since_epoch()).count();
}
struct Header getNewHeader(uint16_t payloadSize, uint32_t seqNo) {
    uint64_t timestamp = getTimestamp();
    struct Header header = {VERSION, payloadSize, SEQNO,timestamp};
    return header;
}

uint16_t getMessageType(std::string message) {
    uint16_t messageStartBit = 128, messageTypeSize = 16;
    return convTo16bit(message.substr(messageStartBit, messageTypeSize));
}

std::string encode(struct Header header) {
    std::string encoded_str = "";
    encoded_str += conv16bit(header.version);
    encoded_str += conv16bit(header.payloadSize);
    encoded_str += conv32bit(header.sequenceNumber);
    encoded_str += conv64bit(header.timestamp);
    return encoded_str;
}

std::string encode(struct NewOrder newOrder) {
    struct Header header = getNewHeader(35, SEQNO);
    std::string encoded_str = encode(header);
    encoded_str += conv16bit(newOrder.messageType);
    encoded_str += conv64bit(newOrder.listingId);
    encoded_str += conv64bit(newOrder.orderId);
    encoded_str += conv64bit(newOrder.orderQuantity);
    encoded_str += conv64bit(newOrder.orderPrice);
    encoded_str += conv8bit(int(newOrder.side));
    return encoded_str;
}

struct NewOrder decodeToNewOrder(std::string message) {
    uint64_t listingId = convTo64bit(message.substr(128+16,64));
    uint64_t orderId = convTo64bit(message.substr(128+16+64,64));
    uint64_t orderQuantity = convTo64bit(message.substr(128+16+64+64,64));
    uint64_t orderPrice = convTo64bit(message.substr(128+16+64+64+64,64));
    uint8_t sideInt = convTo8bit(message.substr(128+16+64+64+64+64,8));
    char side = char(sideInt);
    struct NewOrder newOrder = {1,listingId,orderId,orderQuantity,orderPrice,side};
    return newOrder;
}

std::string encode(struct DeleteOrder deleteOrder) {
    struct Header header = getNewHeader(10, SEQNO);
    std::string encoded_str = encode(header);
    encoded_str += conv16bit(deleteOrder.messageType);
    encoded_str += conv64bit(deleteOrder.orderId);
    return encoded_str;
}

struct DeleteOrder decodeToDeleteOrder(std::string message) {
    uint64_t orderId = convTo64bit(message.substr(128+16,64));
    struct DeleteOrder deleteOrder = {2,orderId};
    return deleteOrder;
}

std::string encode(struct ModifyOrderQuantity modifyOrder) {
    struct Header header = getNewHeader(18, SEQNO);
    std::string encoded_str = encode(header);
    encoded_str += conv16bit(modifyOrder.messageType);
    encoded_str += conv64bit(modifyOrder.orderId);
    encoded_str += conv64bit(modifyOrder.newQuantity);
    return encoded_str;
}

struct ModifyOrderQuantity decodeToModifyOrder(std::string message) {
    uint64_t orderId = convTo64bit(message.substr(128+16,64));
    uint64_t newQuantity = convTo64bit(message.substr(128+16+64,64));
    struct ModifyOrderQuantity modifyOrder = {3,orderId,newQuantity};
    return modifyOrder;
}

std::string encode(struct Trade tradeOrder) {
    struct Header header = getNewHeader(34, SEQNO);
    std::string encoded_str = encode(header);
    encoded_str += conv16bit(tradeOrder.messageType);
    encoded_str += conv64bit(tradeOrder.listingId);
    encoded_str += conv64bit(tradeOrder.tradeId);
    encoded_str += conv64bit(tradeOrder.tradeQuantity);
    encoded_str += conv64bit(tradeOrder.tradePrice);
    return encoded_str;
}

struct Trade decodeToTradeOrder(std::string message) {
    uint64_t listingId = convTo64bit(message.substr(128+16,64));
    uint64_t tradeId = convTo64bit(message.substr(128+16+64,64));
    uint64_t tradeQuantity = convTo64bit(message.substr(128+16+64+64,64));
    uint64_t tradePrice = convTo64bit(message.substr(128+16+64+64+64,64));
    struct Trade tradeOrder = {1,listingId,tradeId,tradeQuantity,tradePrice};
    return tradeOrder;
}

std::string encode(struct OrderResponse orderResponse) {
    struct Header header = getNewHeader(12, SEQNO);
    std::string encoded_str = encode(header);
    encoded_str += conv16bit(orderResponse.messageType);
    encoded_str += conv64bit(orderResponse.orderId);
    encoded_str += conv16bit(static_cast<uint16_t>(orderResponse.status));
    return encoded_str;
}

struct OrderResponse decodeToOrderResponse(std::string message) {
    uint64_t orderId = convTo64bit(message.substr(128+16,64));
    Status status = static_cast<Status>(convTo16bit(message.substr(128+16+64,16)));
    struct OrderResponse orderResponse = {1,orderId,status};
    return orderResponse;
}