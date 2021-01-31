#include "rendering_server_wrap_local.h"

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>

// #include "splab_grpc_service.pb.h"
#include "modules/splab/splab_grpc_service.h"

RenderingServerWrapLocal::RenderingServerWrapLocal() {
	std::string server_address("0.0.0.0:50051");
    SPLabRenderingServerImpl::SPLabRenderingServerImpl service();

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

RenderingServerWrapLocal::init(){}