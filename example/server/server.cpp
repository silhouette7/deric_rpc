#include <memory>
#include <iostream>
#include <sys/epoll.h>

#include "api/deric_test_service_api.h"
#include "rpc_service_entry.h"
#include "rpc_config.h"
#include "deric_process_entry.h"

class Caculator {
public:
    int add(int a, int b) {
        std::cout << a << " + " << b << " = " << a + b << std::endl;
        return a + b;
    }

    int minor(int a, int b) {
        std::cout << a << " - " << b << " = " << a + b << std::endl;
        return a - b;
    }
};

void hello(const std::string& str) {
    std::cout << str << std::endl;
}

int main() {
    deric::rpc::DericProcessEntry process;
    process.init();

    deric::rpc::RpcConfig serviceConfig;
    (void)serviceConfig.init("test_server.config");

    std::shared_ptr<deric::rpc::RpcServiceEntry> serviceEntry = std::make_shared<deric::rpc::RpcServiceEntry>(RPC_SERVICE_DERIC_TEST);
    serviceEntry->init(serviceConfig);

    serviceEntry->registerMethod(RPC_SERVICE_DERIC_TEST_HELLO, hello);

    Caculator test_cal;
    serviceEntry->registerMethod(RPC_SERVICE_DERIC_TEST_ADD, &Caculator::add, &test_cal);
    serviceEntry->registerMethod(RPC_SERVICE_DERIC_TEST_MINOR, &Caculator::minor, &test_cal);

    serviceEntry->start();

    const std::string endString = "exit";
    std::string inputString;
    while(1) {
        std::getline(std::cin, inputString);
        if (inputString.compare(endString) == 0) {
            std::cout << "close the server" << std::endl;

            serviceEntry->stop();
            serviceEntry->unregisterMethod(RPC_SERVICE_DERIC_TEST_HELLO);
            serviceEntry->unregisterMethod(RPC_SERVICE_DERIC_TEST_ADD);
            serviceEntry->deInit();
            process.deinit();
            break;
        }
    }

    return 0;
}