#ifndef RENDERING_SERVER_WRAP_REMOTE_H
#define RENDERING_SERVER_WRAP_REMOTE_H

#include "rendering_server_default.h"

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>

class RenderingServerWrapRemote : public RenderingServerDefault {
	//virtual void draw(bool p_swap_buffers, double frame_step);
	//virtual void init();
	//RenderingServerWrapRemote();
	//~RenderingServerWrapRemote();
	void RunServer();
};

#endif
