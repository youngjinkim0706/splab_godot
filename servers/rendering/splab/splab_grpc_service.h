#ifndef SPLAB_RENDERING_SERVER
#define SPLAB_RENDERING_SERVER

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "splab_grpc.grpc.pb.h"

class SPLabRenderingServerImpl final : public splab_grpc_service::SPLabRenderingServer::Service {
    public:
      explicit SPLabRenderingServerImpl(){};       

      grpc::Status init_rendering_server(grpc::ServerContext* context, const google::protobuf::Empty* request,
                                  google::protobuf::Empty* reply) override; // rendering_server init

};


#endif