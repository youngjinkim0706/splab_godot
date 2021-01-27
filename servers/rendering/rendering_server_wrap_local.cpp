#include "rendering_server_wrap_local.h"


void RenderingServerWrapLocal::draw(bool p_swap_buffers, double frame_step) {
    print_line("function hooked up");
    // create rpc proxy to send data to client stub
    RenderingServerDefault::draw(p_swap_buffers, frame_step);
}