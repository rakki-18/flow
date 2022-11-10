#ifndef SERVER_UTIL_H
#define SERVER_UTIL_H

#include <vector>
#include <map>

struct OrderResponse calculateResponse(struct NewOrder &newOrder, std::vector<struct Trader> &client_socket, int client);
struct OrderResponse calculateResponse(struct DeleteOrder &deleteOrder, std::vector<struct Trader> &client_socket, int client);
struct OrderResponse calculateResponse(struct ModifyOrderQuantity &modifyOrder, std::vector<struct Trader> &client_socket, int client);
struct OrderResponse calculateResponse(struct Trade &tradeOrder, std::vector<struct Trader> &client_socket, int client);



struct Instrument {
    int64_t netPos; // Net sum of all received trades quantity
    uint64_t buyQty; // Net sum off all buy orders quantity
    uint64_t sellQty; // Net sum off all sell orders quantity
    uint64_t worstBuyQty; // Worst possible buy quantity situation
    uint64_t worstSellQty; // Worst possible sell quantity situation
};

struct Order
{
    uint64_t instrument;
    uint64_t quantity;
    uint64_t price;
    char side;
};

struct Trader
{
    uint64_t buyThreshold; // Threshold above which buy quantities will be rejected
    uint64_t sellThreshold; // Threshold above which sell quantities will be rejected
    std::map<uint64_t,Instrument> instruments;
    std::map<uint64_t,Order> orders;
};

#endif