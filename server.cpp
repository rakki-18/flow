//Handle multiple socket connections with select and fd_set on Linux 
#include <iostream>
#include <cstring>
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>   //close 
#include <arpa/inet.h>    //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include "protocol.h"
#include "server_util.h"

      
#define PORT 8888 
#define MAX_CLIENTS 30


/*
* A complex verification scheme can be carried out by using sequence numbers if required
*/
bool verify(char *message) {
    return true;
}

/*
* Input: Encoded threshold message
* Output: Decoded thresholds for the client/trader
*/
std::pair<uint64_t, uint64_t> getThresholds(char *message) {
    std::string messageStr(message);

    uint64_t buyThreshold = convTo64bit(messageStr.substr(0,64));
    uint64_t sellThreshold = convTo64bit(messageStr.substr(64,64));
    return {buyThreshold, sellThreshold}; 
}

/*
* Main placeholder function to calculate risk of the order
*/
struct OrderResponse calculateResponse(char *message, std::vector<struct Trader> &client_socket, int client) {
    std::string message_str(message);
    uint16_t messageType = getMessageType(message_str);
    std::cout<<"Message Type is: "<<messageType<<std::endl;
    std::cout<<"Length of message is: "<<message_str.size()<<std::endl;
    struct OrderResponse orderResponse;

    switch(messageType) {
        case 1: 
        {
            struct NewOrder newOrder =  decodeToNewOrder(message_str);
            std::cout<<"Printing details of the order: "<<newOrder.orderId<<" "<<newOrder.orderQuantity<<" "<<newOrder.side<<std::endl;
            orderResponse = calculateResponse(newOrder, client_socket, client);
            break;
        }
        case 2:
        {
            struct DeleteOrder deleteOrder =  decodeToDeleteOrder(message_str);
            orderResponse = calculateResponse(deleteOrder, client_socket, client);
            break;
        }
        case 3:
        {
            struct ModifyOrderQuantity modifyOrder =  decodeToModifyOrder(message_str);
            orderResponse = calculateResponse(modifyOrder, client_socket, client);
            break;
        }
        case 4:
        {
            struct Trade tradeOrder =  decodeToTradeOrder(message_str);
            orderResponse = calculateResponse(tradeOrder, client_socket, client);
            break;
        }
        default:
            std::cout<<"Erroneous message"<<std::endl;
    }
    return orderResponse;
}

int main(int argc , char *argv[])  
{  

    for(int i = 0; i < argc; i++) {
        std::cout<<argv[i]<<std::endl;
    }
    int opt = TRUE;  
    int master_socket , addrlen , new_socket , max_clients = 30 , activity, i , valread , sd; 
    int socket_free[max_clients]; 
    int max_sd;  
    struct sockaddr_in address;
    std::vector<struct Trader> client_socket(max_clients);
         
    char buffer[1025];  //data buffer of 1K 

    //set of socket descriptors 
    fd_set readfds;  
         
     
    //initialise all client_socket[] to 0 so not checked 
    for (i = 0; i < max_clients; i++)  
    {  
        socket_free[i] = 0; 
    }  
         
    //create a master socket 
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)  
    {  
        perror("socket failed");  
        exit(EXIT_FAILURE);  
    }  
     
    //set master socket to allow multiple connections , 
    //this is just a good habit, it will work without this 
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
          sizeof(opt)) < 0 )  
    {  
        perror("setsockopt");  
        exit(EXIT_FAILURE);  
    }  
     
    //type of socket created 
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons( PORT );  
         
    //bind the socket to localhost port 8888 
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)  
    {  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    }  
    printf("Listener on port %d \n", PORT);  
         
    //try to specify maximum of 3 pending connections for the master socket 
    if (listen(master_socket, 3) < 0)  
    {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }  
         
    //accept the incoming connection 
    addrlen = sizeof(address);  
    puts("Waiting for connections ...");  
         
    while(TRUE)  
    {  
        //clear the socket set 
        FD_ZERO(&readfds);  
     
        //add master socket to set 
        FD_SET(master_socket, &readfds);  
        max_sd = master_socket;  
             
        //add child sockets to set 
        for ( i = 0 ; i < max_clients ; i++)  
        {  
            //socket descriptor 
            sd = socket_free[i];  
                 
            //if valid socket descriptor then add to read list 
            if(sd > 0)  
                FD_SET( sd , &readfds);  
                 
            //highest file descriptor number, need it for the select function 
            if(sd > max_sd)  
                max_sd = sd;  
        }  
     
        //wait for an activity on one of the sockets , timeout is NULL , 
        //so wait indefinitely 
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  
       
        if ((activity < 0) && (errno!=EINTR))  
        {  
            printf("select error");  
        }  
             
        //If something happened on the master socket , 
        //then its an incoming connection 
        if (FD_ISSET(master_socket, &readfds))  
        {  
            if ((new_socket = accept(master_socket, 
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }  
             
            //inform user of socket number - used in send and receive commands 
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
           
                 
            //add new socket to array of sockets 
            for (i = 0; i < max_clients; i++)  
            {  
                //if position is empty 
                if( socket_free[i] == 0 )  
                {  
                    socket_free[i] = new_socket;  
                    printf("Adding to list of sockets as %d\n" , i);

                    /*
                    * TCP Connection Initialized
                    * Get the thresholds
                    * Send ACK
                    */
                    valread = read( new_socket , buffer, 1024);
                    std::pair<uint64_t, uint64_t> clientThreshold = getThresholds(buffer);
                    
                    struct Trader trader;
                    trader.buyThreshold = clientThreshold.first;
                    trader.sellThreshold = clientThreshold.second;
                    client_socket[i] = trader;
                    std::cout<<client_socket[i].buyThreshold<<std::endl;
                    char *hello = "Hello";
                    send(new_socket, hello, strlen(hello),0);
                    break;  
                }  
            }  
        }  
        

        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)  
        {  
            sd = socket_free[i];  
                 
            if (FD_ISSET( sd , &readfds))  
            {  
                //Check if it was for closing , and also read the 
                //incoming message 
                if ((valread = read( sd , buffer, 1024)) == 0)  
                {  
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);  
                    printf("Host disconnected , ip %s , port %d \n" , 
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
                         
                    //Close the socket and mark as 0 in list for reuse 
                    // Remove all information related to the client
                    close( sd );  
                    socket_free[i] = 0;  
                }  
                     
                else 
                {  
                    //set the string terminating NULL byte on the end
                    //of the data read 
                    buffer[valread] = '\0';
                    std::cout<<"This is the message received by the server: ";
                    puts(buffer);
                    if(verify(buffer) == false) {
                        std::cout<<"Packet is erroneous"<<std::endl;
                    } else {
                        /*
                        * Calculate risk and display to the client
                        */
                        struct OrderResponse orderResponse = calculateResponse(buffer, client_socket, i);
                        std::string orderResponseStr = encode(orderResponse);
                        send(sd , orderResponseStr.c_str() , strlen(buffer) , 0 );  
                    }
                    
                }  
            }  
        }  
    }  
         
    return 0;  
}  


