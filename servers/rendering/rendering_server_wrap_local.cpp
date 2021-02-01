#include "rendering_server_wrap_local.h"

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>

#include "splab/splab_grpc_service.h"

RenderingServerWrapLocal::RenderingServerWrapLocal() {
}
