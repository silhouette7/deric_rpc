#include <memory>
#include <iostream>

#include "deric_test_service_api.h"
#include "rpc_client_entry.h"
#include "rpc_config.h"
#include "deric_process_entry.h"

int main() {
    deric::rpc::DericProcessEntry process;
    process.init();

    deric::rpc::RpcConfig clientConfig;
    (void)clientConfig.init("test_client.config");

    std::shared_ptr<deric::rpc::RpcClientEntry> clientEntry= std::make_shared<deric::rpc::RpcClientEntry>();
    clientEntry->init(clientConfig);
    clientEntry->connect(RPC_SERVICE_DERIC_TEST);

    int res;
    if (0 > clientEntry->invokeWithResultSync<int>(RPC_SERVICE_DERIC_TEST_ADD, res, 1, 2)) {
        std::cout << "call" << RPC_SERVICE_DERIC_TEST_ADD << "failed" << std::endl;
    }
    else {
        std::cout << res << std::endl;
    }

    int a = 3, b = 1;
    if (0 > clientEntry->invokeWithResultSync<int>(RPC_SERVICE_DERIC_TEST_MINOR, res, a, b)) {
        std::cout << "call" << RPC_SERVICE_DERIC_TEST_MINOR << "failed" << std::endl;
    }
    else {
        std::cout << res << std::endl;
    }

    if (0 > clientEntry->invoke(RPC_SERVICE_DERIC_TEST_HELLO, "what's up")) {
        std::cout << "call" << RPC_SERVICE_DERIC_TEST_HELLO << "failed" << std::endl;
    }

    clientEntry->disconnect();
    clientEntry->deinit();

    return 0;
}