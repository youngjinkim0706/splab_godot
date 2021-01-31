#include "core/object/reference.h"


#ifndef SPLAB_RENDERING_SERVER
#define SPLAB_RENDERING_SERVER


class SPLabRenderingServerImpl : public Reference, public SPLabRenderingServer::Service{
    GDCLASS(SPLabRenderingServerImpl, Reference);

protected:
    static void _bind_methods();

public:
    explicit SPLabRenderingServerImpl();

    void init_server();
};


#endif