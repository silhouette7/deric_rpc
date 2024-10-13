#include "deric_test_service_api.h"
#include "deric_process_entry.h"
#include "rpc_client_entry.h"
#include "rpc_config.h"

#include <memory>
#include <iostream>

int main() {
    deric::DericProcessEntry process;
    process.init();

    deric::rpc::RpcConfig clientConfig;
    clientConfig.init("test_client.config");

    std::unique_ptr<deric::rpc::RpcClientEntry> clientEntry= std::make_unique<deric::rpc::RpcClientEntry>();
    clientEntry->init(clientConfig);
    clientEntry->connect(RPC_SERVICE_DERIC_TEST);

    clientEntry->subscribeToEvent(RPC_SERVICE_EVENT_DERIC_TEST_STOP, [](){std::cout << "service close" << std::endl;});

    auto resHelloWorld = clientEntry->invoke<void>(RPC_SERVICE_DERIC_TEST_HELLO_WORLD);
    auto resHello = clientEntry->invoke<void>(RPC_SERVICE_DERIC_TEST_HELLO, "what's up");
    auto resAdd = clientEntry->invoke<int>(RPC_SERVICE_DERIC_TEST_ADD, 1, 2); 
    int a = 3, b = 1;
    auto resMinor = clientEntry->invoke<int>(RPC_SERVICE_DERIC_TEST_MINOR, a, b);

    try {
        resHello.get();
        resHelloWorld.get();
    }
    catch(...) {
        std::cout << "invoke hello function failed" << std::endl;
    }

    try {
        std::cout << RPC_SERVICE_DERIC_TEST_ADD << " res: " << resAdd.get() << std::endl;
        std::cout << RPC_SERVICE_DERIC_TEST_MINOR << " res: " << resMinor.get() << std::endl;
    }
    catch(...) {
        std::cout << "invoke algorithm function failed" << std::endl;
    }

    const std::string endString = "exit";
    std::string inputString;
    while(1) {
        std::getline(std::cin, inputString);
        if (inputString.compare(endString) == 0) {
            std::cout << "close the client" << std::endl;
            break;
        }
    }

    clientEntry->unsubscribeToEvent(RPC_SERVICE_EVENT_DERIC_TEST_STOP);

    clientEntry->disconnect();
    clientEntry->deinit();

    return 0;
}