# Steps to run the server
### Server
```
g++ server.cpp protocol.cpp server_util.cpp -o server -std=c++17
./server
```

### Client
```
g++ client.cpp protocol.cpp  -o client -std=c++17
./client 20 15
```



