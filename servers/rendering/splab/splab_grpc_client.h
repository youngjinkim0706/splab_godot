#include <string>
#include <grpcpp/grpcpp.h>
#include "splab_grpc.grpc.pb.h"

class SPLabClient{
    public:
        SPLabClient(){
            std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel("0.0.0.0:50051", 
                grpc::InsecureChannelCredentials());
            stub_ = splab_grpc_service::SPLabRenderingServer::NewStub(channel);
        }
        
        // client에서 보낼 함수 선언
        bool init_rendering_server();
    
    private:
        std::unique_ptr<splab_grpc_service::SPLabRenderingServer::Stub> stub_;
};