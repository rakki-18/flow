#include <iostream>
#include "server_util.h"
#include "protocol.h"

struct OrderResponse calculateResponse(struct NewOrder &newOrder, std::vector<struct Trader> &client_socket, int client) {

    uint64_t listingId = newOrder.listingId;
    uint64_t orderQuantity = newOrder.orderQuantity;

    std::cout<<"For the user values are: "<<client_socket[client].buyThreshold<<" "<<client_socket[client].sellThreshold<<std::endl;


    // If we encounter an unidentified financial instrument
    if(client_socket[client].instruments.find(listingId) == client_socket[client].instruments.end()) {
        struct Instrument instrument = {0,0,0,0,0};
        client_socket[client].instruments[listingId] = instrument;
    }

    if(newOrder.side == 'B') {
        uint64_t tentativeBuyQty = client_socket[client].instruments[listingId].buyQty += orderQuantity;
        uint64_t tentativeWorstBuyQty = std::max(tentativeBuyQty, tentativeBuyQty + client_socket[client].instruments[listingId].netPos);
        if(tentativeWorstBuyQty > client_socket[client].buyThreshold) {
            struct OrderResponse orderResponse = {5,newOrder.orderId, Status::REJECTED};
            return orderResponse;
        } else {
            client_socket[client].instruments[listingId].buyQty += tentativeBuyQty;
            client_socket[client].instruments[listingId].worstBuyQty = tentativeWorstBuyQty;

            struct Order order = {listingId, newOrder.orderQuantity, newOrder.orderPrice, newOrder.side};
            client_socket[client].orders[newOrder.orderId] = order;
            struct OrderResponse orderResponse = {5,newOrder.orderId, Status::ACCEPTED};
            return orderResponse;
        }
    } else if(newOrder.side == 'S'){
        uint64_t tentativeSellQty = client_socket[client].instruments[listingId].sellQty += orderQuantity;
        uint64_t tentativeWorstSellQty = std::max(tentativeSellQty, tentativeSellQty - client_socket[client].instruments[listingId].netPos);
        if(tentativeWorstSellQty > client_socket[client].sellThreshold) {
            struct OrderResponse orderResponse = {5,newOrder.orderId, Status::REJECTED};
            return orderResponse;
        } else {
            client_socket[client].instruments[listingId].sellQty += tentativeSellQty;
            client_socket[client].instruments[listingId].worstSellQty = tentativeWorstSellQty;

            struct Order order = {listingId, newOrder.orderQuantity, newOrder.orderPrice, newOrder.side};
            client_socket[client].orders[newOrder.orderId] = order;
            struct OrderResponse orderResponse = {5,newOrder.orderId, Status::ACCEPTED};
            return orderResponse;
        }
    } else {
        std::cout<<"Input is erroneous"<<std::endl;
        struct OrderResponse orderResponse = {5,newOrder.orderId, Status::NA};
    }
}

struct OrderResponse calculateResponse(struct DeleteOrder &deleteOrder, std::vector<struct Trader> &client_socket, int client) {

    uint64_t orderId = deleteOrder.orderId;
    struct OrderResponse orderResponse = {5,orderId, Status::NA};

    // If order is not present
    // It is a meaningless order
    if(client_socket[client].orders.find(orderId) == client_socket[client].orders.end()) {
        std::cout<<"Input is erroneous"<<std::endl;
        return orderResponse;
    }

    struct Order order = client_socket[client].orders[orderId];
    client_socket[client].orders.erase(orderId);

    if(order.side == 'B') {
        uint64_t newBuyQty = client_socket[client].instruments[order.instrument].buyQty -= order.quantity;
        uint64_t newWorstBuyQty = std::max(newBuyQty, newBuyQty + client_socket[client].instruments[order.instrument].netPos);
        
        client_socket[client].instruments[order.instrument].buyQty = newBuyQty;
        client_socket[client].instruments[order.instrument].worstBuyQty = newWorstBuyQty;
        
    } else if(order.side == 'S') {
        uint64_t newSellQty = client_socket[client].instruments[order.instrument].sellQty -= order.quantity;
        uint64_t newWorstSellQty = std::max(newSellQty, newSellQty - client_socket[client].instruments[order.instrument].netPos);
        
        client_socket[client].instruments[order.instrument].sellQty = newSellQty;
        client_socket[client].instruments[order.instrument].worstSellQty = newWorstSellQty;

    } else {
        std::cout<<"Input is erroneous"<<std::endl;
    }
    return orderResponse;
}

struct OrderResponse calculateResponse(struct ModifyOrderQuantity &modifyOrder, std::vector<struct Trader> &client_socket, int client) {

    uint64_t orderId = modifyOrder.orderId;

    // If order is not present
    // It is a meaningless order
    if(client_socket[client].orders.find(orderId) == client_socket[client].orders.end()) {
        std::cout<<"Input is erroneous"<<std::endl;
        struct OrderResponse orderResponse = {5,orderId,Status::NA};
        return orderResponse;
    }
    struct Order order = client_socket[client].orders[orderId];

    // Order definitely is accepted if new quantity is less than original quantity
    if(modifyOrder.newQuantity <= order.quantity) {
        client_socket[client].orders[orderId].quantity = modifyOrder.newQuantity;
        if(order.side == 'B') {
            uint64_t newBuyQty = client_socket[client].instruments[order.instrument].buyQty -= (order.quantity - modifyOrder.newQuantity);
            uint64_t newWorstBuyQty = std::max(newBuyQty, newBuyQty + client_socket[client].instruments[order.instrument].netPos);
            
            client_socket[client].instruments[order.instrument].buyQty = newBuyQty;
            client_socket[client].instruments[order.instrument].worstBuyQty = newWorstBuyQty;
            struct OrderResponse orderResponse = {5,orderId,Status::ACCEPTED};
            return orderResponse;
        } else if(order.side == 'S') {
            uint64_t newSellQty = client_socket[client].instruments[order.instrument].sellQty -= (order.quantity - modifyOrder.newQuantity);
            uint64_t newWorstSellQty = std::max(newSellQty, newSellQty - client_socket[client].instruments[order.instrument].netPos);
            
            client_socket[client].instruments[order.instrument].sellQty = newSellQty;
            client_socket[client].instruments[order.instrument].worstSellQty = newWorstSellQty;
            struct OrderResponse orderResponse = {5,orderId,Status::ACCEPTED};
            return orderResponse;

        } else {
            std::cout<<"Input is erroneous"<<std::endl;
            struct OrderResponse orderResponse = {5,orderId,Status::NA};
            return orderResponse;
        }
    } else {
        // Order can get rejected here
        uint64_t orderQuantity = (modifyOrder.newQuantity - order.quantity);
        uint64_t listingId = order.instrument;

        if(order.side == 'B') {
            uint64_t tentativeBuyQty = client_socket[client].instruments[listingId].buyQty += orderQuantity;
            uint64_t tentativeWorstBuyQty = std::max(tentativeBuyQty, tentativeBuyQty + client_socket[client].instruments[listingId].netPos);
            if(tentativeWorstBuyQty > client_socket[client].buyThreshold) {
                struct OrderResponse orderResponse = {5,orderId, Status::REJECTED};
                return orderResponse;
            } else {
                client_socket[client].instruments[listingId].buyQty += tentativeBuyQty;
                client_socket[client].instruments[listingId].worstBuyQty = tentativeWorstBuyQty;

                client_socket[client].orders[orderId].quantity = modifyOrder.newQuantity;
                struct OrderResponse orderResponse = {5,orderId, Status::ACCEPTED};
                return orderResponse;
            }
        } else if(order.side == 'S'){
            uint64_t tentativeSellQty = client_socket[client].instruments[listingId].sellQty += orderQuantity;
            uint64_t tentativeWorstSellQty = std::max(tentativeSellQty, tentativeSellQty - client_socket[client].instruments[listingId].netPos);
            if(tentativeWorstSellQty > client_socket[client].sellThreshold) {
                struct OrderResponse orderResponse = {5,orderId, Status::REJECTED};
                return orderResponse;
            } else {
                client_socket[client].instruments[listingId].sellQty += tentativeSellQty;
                client_socket[client].instruments[listingId].worstSellQty = tentativeWorstSellQty;

                client_socket[client].orders[orderId].quantity = modifyOrder.newQuantity;
                struct OrderResponse orderResponse = {5,orderId, Status::ACCEPTED};
                return orderResponse;
            }
        } else {
            std::cout<<"Input is erroneous"<<std::endl;
            struct OrderResponse orderResponse = {5,orderId, Status::NA};
            return orderResponse;
        }
    }
    
    // Note that the code wont reach here
    static OrderResponse dummyResponse;
    return dummyResponse;
}


struct OrderResponse calculateResponse(struct Trade &tradeOrder, std::vector<struct Trader> &client_socket, int client) {

    uint64_t tradeId = tradeOrder.tradeId;
    uint64_t listingId = tradeOrder.listingId;
    int64_t tradeQuantity = tradeOrder.tradeQuantity;
    struct OrderResponse orderResponse = {5,tradeId, Status::NA};

    // If original order id is not present
    // trade is meaningless
    if(client_socket[client].orders.find(tradeId) == client_socket[client].orders.end()) {
        std::cout<<"Input is erroneous"<<std::endl;
        return orderResponse;
    }

    struct Order order = client_socket[client].orders[tradeId];

    int64_t netPos = client_socket[client].instruments[listingId].netPos + tradeQuantity;
    client_socket[client].instruments[listingId].worstBuyQty = std::max(client_socket[client].instruments[listingId].buyQty, client_socket[client].instruments[listingId].buyQty + netPos);
    client_socket[client].instruments[listingId].worstSellQty = std::max(client_socket[client].instruments[listingId].sellQty, client_socket[client].instruments[listingId].sellQty - netPos);
    client_socket[client].instruments[listingId].netPos = netPos;

    return orderResponse;
}