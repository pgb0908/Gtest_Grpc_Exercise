//
// Created by bong on 21. 10. 13..
//

#include <iostream>
#include <gtest/gtest.h>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <thread>





TEST(otel_grpc_server_test, client_check_test){


}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    //::testing::AddGlobalTestEnvironment(new GlobalEnv);
    return RUN_ALL_TESTS();
}