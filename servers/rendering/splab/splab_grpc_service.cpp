#include "splab_grpc_service.h"

#include "servers/rendering/rendering_server_wrap_remote.h"

grpc::Status SPLabRenderingServerImpl::init_rendering_server(grpc::ServerContext* context, const google::protobuf::Empty* request,
                                  google::protobuf::Empty* reply) {
    // rendering_server initialize 시작
    RenderingServer::get_singleton()->init();

    return grpc::Status::OK;
}