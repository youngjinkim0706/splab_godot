#include "zmq_server.h"

std::mutex ZMQServer::_mutex;
ZMQServer *ZMQServer::_instance = nullptr;
int ZMQServer::lock_test;
void ZMQServer::log() {
	std::cout << ZMQServer::socket << std::endl;
}

void ZMQServer::init_zmq() {
	// ZMQServer::socket = zmq::socket_t(ZMQServer::ctx, zmq::socket_type::req);
	ZMQServer::socket = zmq::socket_t(ZMQServer::ctx, zmq::socket_type::req);
}

void ZMQServer::add_count() {
	_mutex.lock();
	ZMQServer::lock_test++;
	_mutex.unlock();
}

int ZMQServer::get_count() {
	std::cout << &(ZMQServer::lock_test) << std::endl;
	return ZMQServer::lock_test;
}

// zmq::socket_t ZMQServer::get_socket() {
// 	return ZMQServer::socket;
// }