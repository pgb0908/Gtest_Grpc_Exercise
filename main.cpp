#include <iostream>
#include <gtest/gtest.h>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "generated/greetings.grpc.pb.h"
#include "generated/greetings.pb.h"
#include <thread>

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service {
    grpc::Status SayHello(grpc::ServerContext* context, const HelloRequest* request,
                    HelloReply* reply) override {
        std::string prefix("Hello ");
        reply->set_message(prefix + request->name());
        return grpc::Status::OK;
    }
};

class GreeterClient {
public:
    GreeterClient(std::shared_ptr<grpc::Channel> channel)
            : stub_(Greeter::NewStub(channel)) {}

    // Assembles the client's payload, sends it and presents the response back
    // from the server.
    std::string SayHello(const std::string& user) {
        // Data we are sending to the server.
        HelloRequest request;
        request.set_name(user);

        // Container for the data we expect from the server.
        HelloReply reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        grpc::ClientContext context;

        // The actual RPC.
        grpc::Status status = stub_->SayHello(&context, request, &reply);

        // Act upon its status.
        if (status.ok()) {
            return reply.message();
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return "RPC failed";
        }
    }

private:
    std::unique_ptr<Greeter::Stub> stub_;
};


TEST(grpc_server_test, client_check_test){

    std::thread server_th([](){
        std::cout << "-----GLOBAL ENV SETUP-----" << std::endl;

        // grpc 서버 부팅
        std::string server_address("0.0.0.0:50051");
        GreeterServiceImpl service;

        grpc::EnableDefaultHealthCheckService(true);
        grpc::reflection::InitProtoReflectionServerBuilderPlugin();
        grpc::ServerBuilder builder;
        // Listen on the given address without any authentication mechanism.
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        // Register "service" as the instance through which we'll communicate with
        // clients. In this case it corresponds to an *synchronous* service.
        builder.RegisterService(&service);
        // Finally assemble the server.
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << "Segrpc::Serveristening on " << server_address << std::endl;

        // Wait for the server to shutdown. Note that some other thread must be
        // responsible for shutting down the server for this call to ever return.
        server->Wait();
    });
    server_th.detach();
    sleep(5);


    std::string val;
    std::thread client_th([&val](){
        // 클라이언트 부팅
        // Instantiate the client. It requires a channel, out of which the actual RPCs
        // are created. This channel models a connection to an endpoint specified by
        // the argument "--target=" which is the only expected argument.
        // We indicate that the channel isn't authenticated (use of
        // InsecureChannelCredentials()).
        std::string target_str = "localhost:50051";

        GreeterClient greeter(
                grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
        std::string user("world");
        std::string reply = greeter.SayHello(user);
        std::cout << "Greeter received: " << reply << std::endl;

        EXPECT_TRUE(reply != "RPC failed");
    });


    client_th.join();

    // 값 비교



}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    //::testing::AddGlobalTestEnvironment(new GlobalEnv);
    return RUN_ALL_TESTS();
}
