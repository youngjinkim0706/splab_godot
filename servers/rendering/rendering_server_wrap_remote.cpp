#include "rendering_server_wrap_remote.h"

#include <grpcpp/grpcpp.h>
#include "splab/splab_grpc_service.h"
void RenderingServerWrapRemote::run_rpc_server(){
    //initiate grpc server for rpc call
    std::string server_address("0.0.0.0:50051");
    SPLabRenderingServerImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    
    print_line("rpc server start");
    server->Wait();
}

void RenderingServerWrapRemote::init(){
    print_line("remote server");
}


void RenderingServerWrapRemote::draw(bool p_swap_buffers, double frame_step){
    
}
