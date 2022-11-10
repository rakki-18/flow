#include <arpa/inet.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include "protocol.h"
#define PORT 8888


// Writing unit test as per the example given 
std::string unit_test(int current_order) {
    std::string messageStr = "";
    switch(current_order) {
        case 1:
        {
            struct NewOrder first = {1,100,1,10,10,'B'};
            std::cout<<"Sending the following new order: "<<std::endl;
            std::cout<<"side: "<<first.side<<" "<<" quantity: "<<first.orderQuantity<<" instrument: "<<first.listingId<<std::endl;
            messageStr = encode(first);
            break;

        }
        case 2:
        {
            struct NewOrder second = {1,200,2,10,15,'S'};
            std::cout<<"Sending the following new order: "<<std::endl;
            std::cout<<"side: "<<second.side<<" "<<" quantity: "<<second.orderQuantity<<" instrument: "<<second.listingId<<std::endl;
            messageStr = encode(second);
            break;
        }
        case 3:
        {
            struct NewOrder third = {1,200,3,15,4,'B'};
            std::cout<<"Sending the following new order: "<<std::endl;
            std::cout<<"side: "<<third.side<<" "<<" quantity: "<<third.orderQuantity<<" instrument: "<<third.listingId<<std::endl;
            messageStr = encode(third);
            break;
        }
        case 4:
        {
            struct NewOrder fourth = {1,200,4,15,20,'B'};
            std::cout<<"Sending the following new order: "<<std::endl;
            std::cout<<"side: "<<fourth.side<<" "<<" quantity: "<<fourth.orderQuantity<<" instrument: "<<fourth.listingId<<std::endl;
            messageStr = encode(fourth);
            break;
        }
        case 5:
        {
            struct Trade firstTrade = {4, 200, 3, -4, 15};
            std::cout<<"Sending the following trade: "<<std::endl;
            std::cout<<" quantity: "<<firstTrade.tradeQuantity<<" instrument: "<<firstTrade.listingId<<std::endl;
            messageStr = encode(firstTrade);
            break;
        }
        case 6:
        {
            struct DeleteOrder deleteTrade = {3, 200};
            std::cout<<"Sending the following deleteOrder: "<<std::endl;
            std::cout<<" orderId: "<<deleteTrade.orderId<<std::endl;
            messageStr = encode(deleteTrade);
            break;
        }
        default: {
            messageStr = "";
        }
    }
    return messageStr;

}

// Given response from server
// Prints the risk involved
void printServerResponse(char *message) {
    std::string messageStr(message);
    struct OrderResponse orderResponse = decodeToOrderResponse(messageStr);
    switch(orderResponse.status) {
        case Status::ACCEPTED:
            std::cout<<"Yes, this order can be sent to the exchange"<<std::endl;
            break;
        case Status::REJECTED:
            std::cout<<"No, this order can not be sent to the exchange"<<std::endl;
            break;
        case Status::NA:
            std::cout<<"NA"<<std::endl;
            break;
        default:
            std::cout<<"NA"<<std::endl;
    }

}

// Use actual trading techniques to get the order that you want to send to the exchange
// Here, we are using unit testing approach here for checking code correctness
std::string get_order(int current_order) {
    return unit_test(current_order);
}

int main(int argc, char const* argv[])
{
    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
    
    char buffer[1024] = { 0 };
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
 
    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
 
    if ((client_fd = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))< 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    /*
    * Calculate thresholds from command line arguments
    */
    std::string tradingThreshold = "";
    std::cout<<"Number of arguments: "<<argc<<std::endl;
    for(int i = 1; i < argc; i++) {
        tradingThreshold += conv64bit(atoi(argv[i]));
        std::cout<<"THis is the value: "<<atoi(argv[i])<<std::endl;
    }

    /*
    *
    * TCP Connection Initialization
    * Client -> encodes and sends thresholds to server
    * Server -> ACK
    */
    send(sock, tradingThreshold.c_str(), strlen(tradingThreshold.c_str()),0);
    valread = read(sock, buffer, 1024);
    puts(buffer);

    int current_order = 1;

    /*
    * Trader can keep asking for orders until they wish to
    * For the purpose of demo
    * Traders can ask 7 orders in this code
    *
    */
    while(true) {
        std::string order = get_order(current_order);
        send(sock, order.c_str(), strlen(order.c_str()), 0);
        printf("Order message sent\n");
        
        valread = read(sock, buffer, 1024);
        std::cout<<"Response from server: ";
        printServerResponse(buffer);
        std::cout<<std::endl<<std::endl;


        current_order++;
        if(current_order == 7)
            break;
    }
    
    // closing the connected socket
    close(client_fd);
    return 0;
}
