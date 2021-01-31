#ifndef SPLAB_RENDERING_SERVER
#define SPLAB_RENDERING_SERVER

#include "splab_grpc_service.grpc.pb.h"

class SPLabRenderingServerImpl final : public splab_grpc_service::SPLabRenderingServer::Service {
    public:
      explicit SPLabRenderingServerImpl(){};       
      void init_server();
  
};


#endif