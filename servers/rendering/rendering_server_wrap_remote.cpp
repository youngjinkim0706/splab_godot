#include "rendering_server_wrap_remote.h"

//void RenderingServerWrapRemote::draw(bool p_swap_buffers, double frame_step) {
//	print_line("function hooked up");
//	// create rpc proxy to send data to client stub
//	RenderingServerDefault::draw(p_swap_buffers, frame_step);
//}

//RenderingServerWrapRemote::RenderingServerWrapRemote() {
//	//RSG::canvas = memnew(RendererCanvasCull);
//	//RSG::viewport = memnew(RendererViewport);
//	//RendererSceneCull *sr = memnew(RendererSceneCull);
//	//RSG::scene = sr;
//	//RSG::rasterizer = RendererCompositor::create();
//	//RSG::storage = RSG::rasterizer->get_storage();
//	//RSG::canvas_render = RSG::rasterizer->get_canvas();
//	//sr->set_scene_render(RSG::rasterizer->get_scene());
//
//	//frame_profile_frame = 0;
//
//	//for (int i = 0; i < 4; i++) {
//	//	black_margin[i] = 0;
//	//	black_image[i] = RID();
//	//}
//}
void RenderingServerWrapRemote::RunServer(){
    grpc::ServerBuilder builder;
    std::string server_address("0.0.0.0:50051");
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // builder.RegisterService(&service);
    // std::unique_ptr<Server> server(builder.BuildAndStart());
    // std::cout << "Server listening on " << server_address << std::endl;
    // server->Wait();
}
