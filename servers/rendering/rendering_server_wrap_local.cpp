#include "rendering_server_wrap_local.h"
#include <string>


void RenderingServerWrapLocal::init(){
    // rpc server의 init()함수를 호출해서 edge의 rendering_server->init() 호출

    bool status = rpc_client.init_rendering_server();
    if (status){
        print_line("rpc succeeded");
    }else{
        print_line("rpc is failed");
    }
    
    // cloud의 init() 실행. 테스트용
    RenderingServerDefault::init();
}