#ifndef RENDERING_SERVER_WRAP_LOCAL_H
#define RENDERING_SERVER_WRAP_LOCAL_H


#include "rendering_server_default.h"
#include "splab/splab_grpc_client.h"

#include <string>

#include <grpcpp/grpcpp.h>

class RenderingServerWrapLocal : public RenderingServerDefault{	
	public:
		RenderingServerWrapLocal(){
		};
		
		~RenderingServerWrapLocal(){};
    	SPLabClient rpc_client = SPLabClient();

		virtual void init();
};

#endif
