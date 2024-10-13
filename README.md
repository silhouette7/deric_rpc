# DERIC_RPC
> a simple rpc lib

## 1. Function
- Support remote function call and server event broadcast
- Configuration by config file

## 2. Run the example
Refer to example code in the dir ```./example```.
### 2.1 Server
1. Build the server lib
   ```
   mkdir deric_rpc/build_server
   cd deric_rpc/build_server
   cmake .. -DSERVER_BUILD=ON
   make
   ```
2. Build the server bin
   ```
   mkdir test/rpc/server/build
   cd test/rpc/server/build
   cmake ..
   make
   ```
3. Run the server
   ```
   cp test/rpc/server/build/server test/rpc/server/
   ./test/rpc/server/server
   ```
### 2.2 Client
1. Build the client lib
   ```
   mkdir deric_rpc/build_client
   cd deric_rpc/build_client
   cmake .. -DCLIENT_BUILD=ON
   make
   ```
2. Build the client bin
   ```
   mkdir test/rpc/client/build
   cd test/rpc/client/build
   cmake ..
   make
   ```
3. Run the client
   ```
   cp test/rpc/client/build test/rpc/client
   ./test/rpc/client/client
   ```

## 3. Usage
### 3.1 Dependency
1. C++ 17
2. Msgpack
   > https://github.com/msgpack/msgpack-c/tree/cpp_master
3. Boost
   > Boost was required by msgpack

### 3.2 Server
1. Define service api with string.
   > Refer to ```test/rpc/server/deric_test_service.api.h```
2. Config the service by a config file.
   > Refer to ```test/rpc/server/test_server.config```
3. Create an instance of class ```DericProcessEntry``` and initialize the process.
4. Create an instance of class ```RpcServiceEntry``` to hold the service.
5. Register the service ip and port into the service book.
   > Refer to ```test/rpc/test_service_book```

### 3.3 Client
1. Config the client by a config file.
   > Refer to ```test/rpc/client/test_client.config```
2. Create an instance of class ```DericProcessEntry``` and initialize the process.
3. Create an instance of class ```RpcClientEntry``` to hold a connection to the service, and connect it to the target service that defined in the service api files.

## 4. Implementation
### 4.1 Function register
All functions are config to an standard format defined in ```ServiceInterface::ServiceFunctionType``` by ```functionexecutor```, which receives the origin input string and return the result string.

The ```functionexecutor``` traits the typies of the arguments and return value of the register function by ```function_traits```. Then it deserializes the arguments from the input string, by ```RpcSerialer```, and feeds them to the register funtion to execute it.

### 4.2 Serialization/Deserilization
All serialization/deserilization jobs are done by msgpack.

### 4.3 IO
The inter process communication is done by TCP.

The ```IoModule``` class holds all the io event of a process by epoll.

A ```TcpIoServer``` instance holds a tcp server, which can handle the tcp connection request from the network.

A ```TcpIoConnection``` instance holds a tcp connection.
