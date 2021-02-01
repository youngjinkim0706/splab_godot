#ifndef SPLAB_RENDERING_SERVER
#define SPLAB_RENDERING_SERVER

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

// #include "splab_grpc_service.grpc.pb.h"
#include "helloworld.grpc.pb.h"

class SPLabRenderingServerImpl final : public helloworld::Greeter::Service {
    public:
      explicit SPLabRenderingServerImpl(){};       
      // void init_server();
      grpc::Status SayHello(grpc::ServerContext* context, const helloworld::HelloRequest* request, helloworld::HelloReply* reply) override{
        std::string prefix("Hello ");
        reply->set_message(prefix + request->name());
        return grpc::Status::OK;
      }
};


#endif