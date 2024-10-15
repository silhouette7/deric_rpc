#include <memory>
#include <iostream>
#include <sys/epoll.h>

#include "api/deric_test_service_api.h"
#include "rpc_service_entry.h"
#include "rpc_config.h"
#include "deric_process_entry.h"

void helloWorld() {
    std::cout << "hello world" << std::endl;
}

void hello(const std::string& str) {
    std::cout << str << std::endl;
}

int count() {
    static int i = 0;
    return ++i;
}

class Caculator {
public:
    void init() {
        std::cout << "init" << std::endl;
    }

    int add(int a, int b) {
        std::cout << a << " + " << b << " = " << a + b << std::endl;
        return a + b;
    }

    int minor(int a, int b) {
        std::cout << a << " - " << b << " = " << a + b << std::endl;
        return a - b;
    }
};

int main() {
    deric::DericProcessEntry process;
    process.init();

    deric::rpc::RpcConfig serviceConfig;
    (void)serviceConfig.init("test_server.config");

    std::unique_ptr<deric::rpc::RpcServiceEntry> serviceEntry = std::make_unique<deric::rpc::RpcServiceEntry>(RPC_SERVICE_DERIC_TEST);
    serviceEntry->init(serviceConfig);

    serviceEntry->registerMethod(RPC_SERVICE_DERIC_TEST_HELLO_WORLD, helloWorld);
    serviceEntry->registerMethod(RPC_SERVICE_DERIC_TEST_HELLO, hello);
    serviceEntry->registerMethod(RPC_SERVICE_DERIC_TEST_COUNT, count);

    Caculator test_cal;
    serviceEntry->registerMethod(RPC_SERVICE_DERIC_TEST_CACULATOR_INIT, &Caculator::init, &test_cal);
    serviceEntry->registerMethod(RPC_SERVICE_DERIC_TEST_ADD, &Caculator::add, &test_cal);
    serviceEntry->registerMethod(RPC_SERVICE_DERIC_TEST_MINOR, &Caculator::minor, &test_cal);

    serviceEntry->registerEvent(RPC_SERVICE_EVENT_DERIC_TEST_STOP);

    serviceEntry->start();

    const std::string endString = "exit";
    std::string inputString;
    while(1) {
        std::getline(std::cin, inputString);
        if (inputString.compare(endString) == 0) {
            std::cout << "close the server" << std::endl;
            serviceEntry->postEvent(RPC_SERVICE_EVENT_DERIC_TEST_STOP);

            serviceEntry->stop();
            serviceEntry->unregisterMethod(RPC_SERVICE_DERIC_TEST_MINOR);
            serviceEntry->unregisterMethod(RPC_SERVICE_DERIC_TEST_ADD);
            serviceEntry->unregisterMethod(RPC_SERVICE_DERIC_TEST_CACULATOR_INIT);
            serviceEntry->unregisterMethod(RPC_SERVICE_DERIC_TEST_COUNT);
            serviceEntry->unregisterMethod(RPC_SERVICE_DERIC_TEST_HELLO);
            serviceEntry->unregisterMethod(RPC_SERVICE_DERIC_TEST_HELLO_WORLD);
            serviceEntry->deInit();
            process.deinit();
            break;
        }
    }

    return 0;
}