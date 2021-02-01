#include <thread>

#ifndef RENDERING_SERVER_WRAP_REMOTE_H
#define RENDERING_SERVER_WRAP_REMOTE_H

#include "rendering_server_default.h"

class RenderingServerWrapRemote : public RenderingServerDefault {
	private:
		std::thread _rpc_server_thread;
	public:
	void run_rpc_server();
	
	RenderingServerWrapRemote(){ // grpc server
		_rpc_server_thread = std::thread(&RenderingServerWrapRemote::run_rpc_server, this); //init 
	};
	~RenderingServerWrapRemote(){

	};
	

};

#endif
