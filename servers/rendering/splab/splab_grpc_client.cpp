#include "splab_grpc_client.h"

bool SPLabClient::init_rendering_server(){
    grpc::ClientContext context;
    google::protobuf::Empty request;
    google::protobuf::Empty reply;
    
    grpc::Status status = stub_->init_rendering_server(&context, request, &reply); //실제로 rpc를 호출하는 코드

    if (status.ok()){
        return true;
    } else{
        return false;
    }
}