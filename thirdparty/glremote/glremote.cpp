// #include <tbb/concurrent_queue.h>
#include <chrono>
#include <iostream>
#include <string>
#include <type_traits>
#include <zmq.hpp>
// #include <GL/glew.h>
#include "gl_commands.h"
#include "glremote/glremote.h"

#define GL_SET_COMMAND(PTR, FUNCNAME)                                                \
	gl_##FUNCNAME##_t *PTR = (gl_##FUNCNAME##_t *)malloc(sizeof(gl_##FUNCNAME##_t)); \
	// PTR->cmd = GLSC_##FUNCNAME

GLint global_pack_alignment = 4;
GLint global_unpack_alignment = 4;
int sequence_number = 0;
int cache_hit = 0;
size_t total_size = 0;
size_t data_size = 0;
size_t more_data_size = 0;
size_t cmd_size = 0;

auto last_time = std::chrono::steady_clock::now();

std::map<cache_key, std::size_t> data_cache;
std::map<cache_key, std::size_t> more_data_cache;

cache_key cache_key_gen(unsigned char cmd, int sequence_number){
	std::bitset<32> cmd_bit(cmd);
	std::bitset<32> seq_bit(sequence_number);
	cmd_bit <<= 24;
	return (cmd_bit |= seq_bit).to_ulong();
} 

std::string alloc_cached_data(zmq::message_t &data_msg){
	std::string cache_data;
	cache_data.resize(data_msg.size());
	memcpy((void *)cache_data.data(), data_msg.data(), data_msg.size());
	return cache_data;
}

uint32_t calc_pixel_data_size(GLenum type, GLenum format, GLsizei width, GLsizei height){
	uint32_t pixelbytes, linebytes, datasize;

	switch (type) {
	case GL_UNSIGNED_BYTE:
		switch (format) {
			case GL_ALPHA:
				pixelbytes = 1;
				break;
			case GL_RGB:
				pixelbytes = 3;
				break;
			case GL_RG:
				pixelbytes = 2;
				break;
			case GL_RGBA:
				pixelbytes = 4;
				break;
			case GL_LUMINANCE:
				pixelbytes = 1;
				break;
			case GL_LUMINANCE_ALPHA:
				pixelbytes = 2;
				break;
			default:
				pixelbytes = 4;
				break;
		}
		break;
	case GL_UNSIGNED_SHORT_5_6_5:
		pixelbytes = 2;
		break;
	case GL_UNSIGNED_SHORT_4_4_4_4:
		pixelbytes = 2;
		break;
	case GL_UNSIGNED_SHORT_5_5_5_1:
		pixelbytes = 2;
		break;
	default:
		pixelbytes = 4;
		break;
	}
	linebytes = (pixelbytes * width + global_unpack_alignment - 1) & (~(global_unpack_alignment - 1)); // 4 willbe replaced with pixelstorei param
	datasize = linebytes * height;
	return datasize;
}

bool insert_or_check_cache(std::map<cache_key, std::size_t> &cache, cache_key key, zmq::message_t &data_msg){
	bool cached = false;
	std::string cache_data = alloc_cached_data(data_msg);
	std::size_t hashed_data = std::hash<std::string>{}(cache_data);

	auto res = cache.insert(std::make_pair(key, hashed_data));
	
	// true : missed, false : hit
	// if (!res.second){
	// 	if(res.first->second == hashed_data){
	// 		cache_hit++;
	// 		cached = true;
	// 		return cached;
	// 	}
	// }
	total_size += data_msg.size();
	return cached;
}

zmq::message_t send_data(unsigned char cmd, void *cmd_data, int size, bool hasReturn = false) {

	ZMQServer *zmq_server = ZMQServer::get_instance();

	gl_command_t c = {
		cmd
	};
	// if (hasReturn)

	zmq::message_t msg(sizeof(c));
	total_size += msg.size();

	cache_key key = cache_key_gen(cmd, sequence_number);	

	switch (cmd) {
		case (unsigned char) GL_Server_Command::GLSC_glClearBufferfv: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glClearBufferfv_t *more_data = (gl_glClearBufferfv_t *)cmd_data;
			zmq::message_t buffer_data(sizeof(GLfloat) * 4); //
			memcpy(buffer_data.data(), more_data->value, sizeof(GLfloat) * 4);
			is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
			if(is_cached){
				buffer_data.rebuild(0);	
			}
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);
			
			// wait recv if hasReturn true
			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glDeleteFramebuffers: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glDeleteFramebuffers_t *more_data = (gl_glDeleteFramebuffers_t *)cmd_data;
			zmq::message_t buffer_data(sizeof(GLuint) * more_data->n); //
			memcpy(buffer_data.data(), more_data->framebuffers, sizeof(GLuint) * more_data->n);
			is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
			if(is_cached){
				buffer_data.rebuild(0);	
			}
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);

			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glDeleteRenderbuffers: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glDeleteRenderbuffers_t *more_data = (gl_glDeleteRenderbuffers_t *)cmd_data;
			zmq::message_t buffer_data(sizeof(GLuint) * more_data->n); //
			memcpy(buffer_data.data(), more_data->renderbuffers, sizeof(GLuint) * more_data->n);
			is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
			if(is_cached){
				buffer_data.rebuild(0);	
			}
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);

			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glDeleteTextures: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glDeleteTextures_t *more_data = (gl_glDeleteTextures_t *)cmd_data;
			zmq::message_t buffer_data(sizeof(GLuint) * more_data->n);
			memcpy(buffer_data.data(), more_data->textures, sizeof(GLuint) * more_data->n);
			is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
			if(is_cached){
				buffer_data.rebuild(0);	
			}
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);

			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glShaderSource: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glShaderSource_t *more_data = (gl_glShaderSource_t *)cmd_data;
			for (int i = 0; i < more_data->count; i++) {
				zmq::message_t string_data;
				if (strlen(more_data->string[i]) > 0) {
					string_data.rebuild(strlen(more_data->string[i]));
					memcpy(string_data.data(), more_data->string[i], strlen(more_data->string[i]));
				}

				if (i == more_data->count - 1) {
					zmq_server->socket.send(string_data, zmq::send_flags::none);
				} else {
					zmq_server->socket.send(string_data, zmq::send_flags::sndmore);
				}
			}

			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glTransformFeedbackVaryings: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glTransformFeedbackVaryings_t *more_data = (gl_glTransformFeedbackVaryings_t *)cmd_data;
			// std::
			for (int i = 0; i < more_data->count; i++) {
				zmq::message_t string_data;
				if (strlen(more_data->varyings[i]) > 0) {
					string_data.rebuild(strlen(more_data->varyings[i]));
					memcpy(string_data.data(), more_data->varyings[i], strlen(more_data->varyings[i]));
				}

				if (i == more_data->count - 1) {
					zmq_server->socket.send(string_data, zmq::send_flags::none);
				} else {
					zmq_server->socket.send(string_data, zmq::send_flags::sndmore);
				}
			}

			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glBufferData: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glBufferData_t *more_data = (gl_glBufferData_t *)cmd_data;
			zmq::message_t buffer_data;
			if (more_data->data != NULL) {
				buffer_data.rebuild(more_data->size);
				memcpy(buffer_data.data(), more_data->data, more_data->size);
				is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
				if(is_cached){
					buffer_data.rebuild(0);	
				}
			}	
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);

			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glBufferSubData: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glBufferSubData_t *more_data = (gl_glBufferSubData_t *)cmd_data;
			zmq::message_t buffer_data;
			if (more_data->data != NULL) {
				buffer_data.rebuild(more_data->size);
				memcpy(buffer_data.data(), more_data->data, more_data->size);
				is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
				if(is_cached){
					buffer_data.rebuild(0);	
				}
			}
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);
			
			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glUniformMatrix4fv: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glUniformMatrix4fv_t *more_data = (gl_glUniformMatrix4fv_t *)cmd_data;
			zmq::message_t buffer_data;
			if (more_data->value != NULL) {
				buffer_data.rebuild(more_data->count * 16 * sizeof(GLfloat));
				memcpy(buffer_data.data(), more_data->value, more_data->count * 16 * sizeof(GLfloat));
				is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
				if(is_cached){
					buffer_data.rebuild(0);	
				}
			}
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);

			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glUniform4fv: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glUniform4fv_t *more_data = (gl_glUniform4fv_t *)cmd_data;
			zmq::message_t buffer_data;
			if (more_data->value != NULL) {
				buffer_data.rebuild(more_data->count * 4 * sizeof(GLfloat));
				memcpy(buffer_data.data(), more_data->value, more_data->count * 4 * sizeof(GLfloat));
				is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
				if(is_cached){
					buffer_data.rebuild(0);	
				}
			}
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);

			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glVertexAttrib4fv: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glVertexAttrib4fv_t *more_data = (gl_glVertexAttrib4fv_t *)cmd_data;
			zmq::message_t buffer_data;
			if (more_data->v != NULL) {
				buffer_data.rebuild(sizeof(GLfloat) * 4);
				memcpy(buffer_data.data(), more_data->v, sizeof(GLfloat) * 4);
				is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
				if(is_cached){
					buffer_data.rebuild(0);	
				}
			}
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);
			
			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glUniform2fv: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glUniform2fv_t *more_data = (gl_glUniform2fv_t *)cmd_data;
			zmq::message_t buffer_data;
			if (more_data->value != NULL) {
				buffer_data.rebuild(more_data->count * sizeof(GLfloat) * 2);
				memcpy(buffer_data.data(), more_data->value, more_data->count * sizeof(GLfloat));
				is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
				if(is_cached){
					buffer_data.rebuild(0);	
				}
			}
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);	

			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glDrawBuffers: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glDrawBuffers_t *more_data = (gl_glDrawBuffers_t *)cmd_data;
			zmq::message_t buffer_data(more_data->n * sizeof(GLenum));
			memcpy(buffer_data.data(), more_data->bufs, more_data->n * sizeof(GLenum));
			is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
			if(is_cached){
				buffer_data.rebuild(0);	
			}
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);
			
			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glDeleteVertexArrays: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glDeleteVertexArrays_t *more_data = (gl_glDeleteVertexArrays_t *)cmd_data;
			zmq::message_t buffer_data(more_data->n * sizeof(GLuint));
			memcpy(buffer_data.data(), more_data->arrays, more_data->n * sizeof(GLuint));
			is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
			if(is_cached){
				buffer_data.rebuild(0);	
			}
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);
			
			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glDeleteBuffers: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glDeleteBuffers_t *more_data = (gl_glDeleteBuffers_t *)cmd_data;
			zmq::message_t buffer_data(more_data->n * sizeof(GLuint));
			memcpy(buffer_data.data(), more_data->buffers, more_data->n * sizeof(GLuint));
			is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
			if(is_cached){
				buffer_data.rebuild(0);	
			}
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);

			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glGetAttribLocation: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glGetAttribLocation_t *more_data = (gl_glGetAttribLocation_t *)cmd_data;
			zmq::message_t buffer_data(strlen(more_data->name));
			memcpy(buffer_data.data(), more_data->name, strlen(more_data->name));
			is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
			if(is_cached){
				buffer_data.rebuild(0);	
			}
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);
			
			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glBindAttribLocation: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glBindAttribLocation_t *more_data = (gl_glBindAttribLocation_t *)cmd_data;
			zmq::message_t buffer_data(strlen(more_data->name));
			memcpy(buffer_data.data(), more_data->name, strlen(more_data->name));
			is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
			if(is_cached){
				buffer_data.rebuild(0);	
			}
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);
			
			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glGetUniformLocation: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glGetUniformLocation_t *more_data = (gl_glGetUniformLocation_t *)cmd_data;
			zmq::message_t buffer_data(strlen(more_data->name));
			memcpy(buffer_data.data(), more_data->name, strlen(more_data->name));
			is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
			if(is_cached){
				buffer_data.rebuild(0);	
			}
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);
			
			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glGetUniformBlockIndex: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glGetUniformBlockIndex_t *more_data = (gl_glGetUniformBlockIndex_t *)cmd_data;
			zmq::message_t buffer_data(strlen(more_data->name));
			memcpy(buffer_data.data(), more_data->name, strlen(more_data->name));
			is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
			if(is_cached){
				buffer_data.rebuild(0);	
			}
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);
			
			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glTexSubImage2D: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);
			
			// send more data
			gl_glTexSubImage2D_t *more_data = (gl_glTexSubImage2D_t *)cmd_data;
			uint32_t datasize = calc_pixel_data_size(more_data->type, more_data->format, more_data->width, more_data->height);

			zmq::message_t buffer_data;
			if (more_data->pixels != NULL) {
				buffer_data.rebuild(datasize);
				memcpy(buffer_data.data(), more_data->pixels, datasize);
				is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
				if(is_cached){
					buffer_data.rebuild(0);	
				}
			}	
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);

			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glTexImage2D: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);
			
			// send more data
			gl_glTexImage2D_t *more_data = (gl_glTexImage2D_t *)cmd_data;
			uint32_t datasize = calc_pixel_data_size(more_data->type, more_data->format, more_data->width, more_data->height);

			zmq::message_t buffer_data;
			if (more_data->pixels != NULL) {
				buffer_data.rebuild(datasize);
				memcpy(buffer_data.data(), more_data->pixels, datasize);
				is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
				if(is_cached){
					buffer_data.rebuild(0);	
				}
			}	
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);
			
			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glCompressedTexImage2D: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			gl_glCompressedTexImage2D_t *more_data = (gl_glCompressedTexImage2D_t *)cmd_data;
			zmq::message_t buffer_data;
			if (more_data->pixels != NULL) {
				buffer_data.rebuild(more_data->imageSize);
				memcpy(buffer_data.data(), more_data->pixels, more_data->imageSize);
				is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
				if(is_cached){
					buffer_data.rebuild(0);	
				}
			}	
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);
			
			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glTexSubImage3D: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);

			// send more data
			gl_glTexSubImage3D_t *more_data = (gl_glTexSubImage3D_t *)cmd_data;
			uint32_t datasize = calc_pixel_data_size(more_data->type, more_data->format, more_data->width, more_data->height) * more_data->depth;

			zmq::message_t buffer_data;
			if (more_data->pixels != NULL) {
				buffer_data.rebuild(datasize);
				memcpy(buffer_data.data(), more_data->pixels, datasize);
				is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
				if(is_cached){
					buffer_data.rebuild(0);	
				}
			}	
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);
			
			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);
			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_glTexImage3D: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			if(is_cached){
				data_msg.rebuild(0);	
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::sndmore);
			
			// send more data	
			gl_glTexImage3D_t *more_data = (gl_glTexImage3D_t *)cmd_data;
			uint32_t datasize = calc_pixel_data_size(more_data->type, more_data->format, more_data->width, more_data->height) * more_data->depth;

			zmq::message_t buffer_data;
			if (more_data->pixels != NULL) {
				buffer_data.rebuild(datasize);
				memcpy(buffer_data.data(), more_data->pixels, datasize);
				is_cached = insert_or_check_cache(more_data_cache, key, buffer_data);
				if(is_cached){
					buffer_data.rebuild(0);	
				}
			}	
			more_data_size += buffer_data.size();
			zmq_server->socket.send(buffer_data, zmq::send_flags::none);
			
			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);
			break;
		}
		case (unsigned char) GL_Server_Command::GLSC_BREAK:
		case (unsigned char) GL_Server_Command::GLSC_bufferSwap: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::none); 

			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);
			
			float cache_hit_ratio = cache_hit / sequence_number;
			sequence_number = 0;
			cache_hit = 0;

			break;		
		}
		default: {
			// send cmd
			memcpy(msg.data(), (void *)&c, sizeof(c));
			zmq_server->socket.send(msg, zmq::send_flags::sndmore); 

			// send data
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			bool is_cached = insert_or_check_cache(data_cache, key, data_msg);
			
			if(is_cached){
				data_msg.rebuild(0);
			}
			zmq_server->socket.send(data_msg, zmq::send_flags::none);
			
			if (hasReturn)
				zmq_server->socket.recv(msg, zmq::recv_flags::none);

			break;
		}
	}
	auto current_time = std::chrono::steady_clock::now();
	if (std::chrono::duration_cast<std::chrono::seconds>(current_time - last_time).count() >=1){
		std::cout << "----- data :" << total_size <<std::endl;
		total_size = 0;
		last_time = current_time;
	}

	sequence_number++;

	return msg;
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
	GL_SET_COMMAND(c, glViewport);
	
	c->x = x;
	c->y = y;
	c->width = width;
	c->height = height;

	send_data((unsigned char) GL_Server_Command::GLSC_glViewport, (void *)c, sizeof(gl_glViewport_t));
	// std::cout << __func__ << std::endl;
}

void glClear(GLbitfield mask) {
	GL_SET_COMMAND(c, glClear);
	c->mask = mask;
	
	send_data((unsigned char) GL_Server_Command::GLSC_glClear, (void *)c, sizeof(gl_glClear_t));
	// std::cout << __func__ << std::endl;
}

void glBegin(GLenum mode) {
	GL_SET_COMMAND(c, glBegin);
	c->mode = mode;
	
	send_data((unsigned char) GL_Server_Command::GLSC_glBegin, (void *)c, sizeof(gl_glBegin_t));
	// std::cout << __func__ << std::endl;
}
void glColor3f(GLfloat red, GLfloat green, GLfloat blue) {
	GL_SET_COMMAND(c, glColor3f);
	c->red = red;
	c->green = green;
	c->blue = blue;
	
	send_data((unsigned char) GL_Server_Command::GLSC_glColor3f, (void *)c, sizeof(gl_glColor3f_t));
	// std::cout << __func__ << std::endl;
}
void glVertex3f(GLfloat x, GLfloat y, GLfloat z) {
	GL_SET_COMMAND(c, glVertex3f);
	c->x = x;
	c->y = y;
	c->z = z;
	
	send_data((unsigned char) GL_Server_Command::GLSC_glVertex3f, (void *)c, sizeof(gl_glVertex3f_t));
	// std::cout << __func__ << std::endl;
}

void glEnd() {
	GL_SET_COMMAND(c, glEnd);
	
	send_data((unsigned char) GL_Server_Command::GLSC_glEnd, (void *)c, sizeof(gl_glEnd_t));
	// std::cout << __func__ << std::endl;
}

void glFlush() {
	GL_SET_COMMAND(c, glFlush);
	
	send_data((unsigned char) GL_Server_Command::GLSC_glFlush, (void *)c, sizeof(gl_glFlush_t));
	// std::cout << __func__ << std::endl;
}

void glBreak() {
	gl_command_t *c = (gl_command_t *)malloc(sizeof(gl_command_t));
	send_data((unsigned char) GL_Server_Command::GLSC_BREAK, (void *)c, sizeof(gl_command_t));
	// std::cout << __func__ << std::endl;
}
void glSwapBuffer() {
	gl_command_t *c = (gl_command_t *)malloc(sizeof(gl_command_t));
	send_data((unsigned char) GL_Server_Command::GLSC_bufferSwap, (void *)c, sizeof(gl_command_t), true);
	// std::cout << __func__ << std::endl;
}
GLuint glCreateShader(GLenum type) {
	GL_SET_COMMAND(c, glCreateShader);
	
	c->type = type;
	zmq::message_t result = send_data((unsigned char) GL_Server_Command::GLSC_glCreateShader, (void *)c, sizeof(gl_glCreateShader_t), true);
	GLuint *ret = (GLuint *)result.data();
	return (GLuint)*ret;
	// std::cout << __func__ << std::endl;
}
GLenum glGetError(void) {
	GL_SET_COMMAND(c, glGetError);
	
	zmq::message_t result = send_data((unsigned char) GL_Server_Command::GLSC_glGetError, (void *)c, sizeof(gl_glGetError_t), true);
	unsigned int *ret = (GLenum *)result.data();
	return static_cast<GLenum>(*ret);
}

void glShaderSource(GLuint shader, GLuint count, const GLchar *const *string, const GLint *length) {
	GL_SET_COMMAND(c, glShaderSource);
	
	c->shader = shader;
	c->count = count;
	c->string = string;
	c->length = length;
	send_data((unsigned char) GL_Server_Command::GLSC_glShaderSource, (void *)c, sizeof(gl_glShaderSource_t));
	//SEND_MORE
	// std::cout << __func__ << std::endl;
}
void glCompileShader(GLuint shader) {
	GL_SET_COMMAND(c, glCompileShader);
	
	c->shader = shader;
	send_data((unsigned char) GL_Server_Command::GLSC_glCompileShader, (void *)c, sizeof(gl_glCompileShader_t));
	// std::cout << __func__ << std::endl;
}

void glGetShaderiv(GLuint shader, GLenum pname, GLint *param) {
	GL_SET_COMMAND(c, glGetShaderiv);
	
	c->shader = shader;
	c->pname = pname;
	zmq::message_t result = send_data((unsigned char) GL_Server_Command::GLSC_glGetShaderiv, (void *)c, sizeof(gl_glGetShaderiv_t), true);
	GLint *ret = (GLint *)result.data();
	memcpy((void *)param, (void *)ret, sizeof(GLuint));
	// std::cout << __func__ << std::endl;
}

GLuint glCreateProgram() {
	GL_SET_COMMAND(c, glCreateProgram);
	
	zmq::message_t result = send_data((unsigned char) GL_Server_Command::GLSC_glCreateProgram, (void *)c, sizeof(gl_glCreateProgram_t), true);
	GLuint *ret = (GLuint *)result.data();
	return *(GLuint *)ret;
	// std::cout << __func__ << std::endl;
}

void glAttachShader(GLuint program, GLuint shader) {
	GL_SET_COMMAND(c, glAttachShader);
	
	c->program = program;
	c->shader = shader;
	send_data((unsigned char) GL_Server_Command::GLSC_glAttachShader, (void *)c, sizeof(gl_glAttachShader_t));
	// std::cout << __func__ << std::endl;
}

void glLinkProgram(GLuint program) {
	GL_SET_COMMAND(c, glLinkProgram);
	
	c->program = program;
	send_data((unsigned char) GL_Server_Command::GLSC_glLinkProgram, (void *)c, sizeof(gl_glLinkProgram_t));
	// std::cout << __func__ << std::endl;
}

void glGetProgramiv(GLuint program, GLenum pname, GLint *param) {
	GL_SET_COMMAND(c, glGetProgramiv);
	
	c->program = program;
	c->pname = pname;
	zmq::message_t result = send_data((unsigned char) GL_Server_Command::GLSC_glGetProgramiv, (void *)c, sizeof(gl_glGetProgramiv_t), true);
	GLint *ret = (GLint *)result.data();
	memcpy((void *)param, (void *)ret, sizeof(GLint));
	// std::cout << __func__ << std::endl;
}

void glGenBuffers(GLsizei n, GLuint *buffers) {
	GL_SET_COMMAND(c, glGenBuffers);
	
	c->n = n;

	ZMQServer *zmq_server = ZMQServer::get_instance();
	GLuint *indices = new GLuint[n];
	for (int i = 0; i < n; i++) {
		zmq_server->glGenBuffers_i++;
		indices[i] = zmq_server->glGenBuffers_i;
	}
	c->last_index = indices[n - 1];
	send_data((unsigned char) GL_Server_Command::GLSC_glGenBuffers, (void *)c, sizeof(gl_glGenBuffers_t));

	memcpy((void *)buffers, (void *)indices, sizeof(GLuint) * n);
	// std::cout << __func__ << std::endl;
}

void glBindBuffer(GLenum target, GLuint id) {
	GL_SET_COMMAND(c, glBindBuffer);
	
	c->target = target;
	c->id = id;
	send_data((unsigned char) GL_Server_Command::GLSC_glBindBuffer, (void *)c, sizeof(gl_glBindBuffer_t));
	// std::cout << __func__ << std::endl;
}

void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
	GL_SET_COMMAND(c, glBufferData);
	
	c->target = target;
	c->size = size;
	c->data = data;
	c->usage = usage;
	send_data((unsigned char) GL_Server_Command::GLSC_glBufferData, (void *)c, sizeof(gl_glBufferData_t));
	// std::cout << __func__ << std::endl;
}

void glGenVertexArrays(GLsizei n, GLuint *arrays) {
	GL_SET_COMMAND(c, glGenVertexArrays);
	
	c->n = n;
	ZMQServer *zmq_server = ZMQServer::get_instance();
	GLuint *indices = new GLuint[n];
	for (int i = 0; i < n; i++) {
		zmq_server->glGenVertexArrays_i++;
		indices[i] = zmq_server->glGenVertexArrays_i;
	}
	c->last_index = indices[n - 1];
	send_data((unsigned char) GL_Server_Command::GLSC_glGenVertexArrays, (void *)c, sizeof(gl_glGenVertexArrays_t));

	memcpy((void *)arrays, (void *)indices, sizeof(GLuint) * n);

	// std::cout << __func__ << std::endl;
}

void glBindVertexArray(GLuint array) {
	GL_SET_COMMAND(c, glBindVertexArray);
	
	c->array = array;
	send_data((unsigned char) GL_Server_Command::GLSC_glBindVertexArray, (void *)c, sizeof(gl_glBindVertexArray_t));
	// std::cout << __func__ << std::endl;
}

GLint glGetAttribLocation(GLuint programObj, const GLchar *name) {
	GL_SET_COMMAND(c, glGetAttribLocation);
	
	c->programObj = programObj;
	c->name = name;
	zmq::message_t result = send_data((unsigned char) GL_Server_Command::GLSC_glGetAttribLocation, (void *)c, sizeof(gl_glGetAttribLocation_t), true);
	GLint *ret = (GLint *)result.data();
	return (GLint)*ret;
	// std::cout << __func__ << std::endl;
}
void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) {
	GL_SET_COMMAND(c, glVertexAttribPointer);
	
	c->index = index;
	c->size = size;
	c->type = type;
	c->normalized = normalized;
	c->stride = stride;
	c->pointer = (int64_t)pointer;
	// std::cout << (void *)pointer << std::endl;
	send_data((unsigned char) GL_Server_Command::GLSC_glVertexAttribPointer, (void *)c, sizeof(gl_glVertexAttribPointer_t));
	// std::cout << __func__ << std::endl;
}

void glEnableVertexAttribArray(GLuint index) {
	GL_SET_COMMAND(c, glEnableVertexAttribArray);
	
	c->index = index;
	send_data((unsigned char) GL_Server_Command::GLSC_glEnableVertexAttribArray, (void *)c, sizeof(gl_glEnableVertexAttribArray_t));
	// std::cout << __func__ << std::endl;
}
void glUseProgram(GLuint program) {
	GL_SET_COMMAND(c, glUseProgram);
	
	c->program = program;
	send_data((unsigned char) GL_Server_Command::GLSC_glUseProgram, (void *)c, sizeof(gl_glUseProgram_t));
	// std::cout << __func__ << std::endl;
}

void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
	GL_SET_COMMAND(c, glClearColor);
	
	c->red = red;
	c->green = green;
	c->blue = blue;
	c->alpha = alpha;
	send_data((unsigned char) GL_Server_Command::GLSC_glClearColor, (void *)c, sizeof(gl_glClearColor_t));
	// std::cout << __func__ << std::endl;
}
void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
	GL_SET_COMMAND(c, glDrawArrays);
	
	c->mode = mode;
	c->first = first;
	c->count = count;
	send_data((unsigned char) GL_Server_Command::GLSC_glDrawArrays, (void *)c, sizeof(gl_glDrawArrays_t));
	// std::cout << __func__ << std::endl;
}
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {
	GL_SET_COMMAND(c, glScissor);
	
	c->x = x;
	c->y = y;
	c->width = width;
	c->height = height;
	send_data((unsigned char) GL_Server_Command::GLSC_glScissor, (void *)c, sizeof(gl_glScissor_t));
	// std::cout << __func__ << std::endl;
}
void glGetIntegerv(GLenum pname, GLint *data) {
	GL_SET_COMMAND(c, glGetIntegerv);
	
	c->pname = pname;
	zmq::message_t result = send_data((unsigned char) GL_Server_Command::GLSC_glGetIntegerv, (void *)c, sizeof(gl_glGetIntegerv_t), true);
	GLint *ret = (GLint *)result.data();
	memcpy((void *)data, (void *)ret, sizeof(GLint));
	// std::cout << __func__ << std::endl;
}
void glGetFloatv(GLenum pname, GLfloat *data) {
	GL_SET_COMMAND(c, glGetFloatv);
	
	c->pname = pname;
	zmq::message_t result = send_data((unsigned char) GL_Server_Command::GLSC_glGetFloatv, (void *)c, sizeof(gl_glGetFloatv_t), true);
	GLfloat *ret = (GLfloat *)result.data();
	memcpy((void *)data, (void *)ret, sizeof(GLfloat));
	// std::cout << __func__ << std::endl;
}
void glGenTextures(GLsizei n, GLuint *textures) {
	GL_SET_COMMAND(c, glGenTextures);
	
	c->n = n;
	ZMQServer *zmq_server = ZMQServer::get_instance();
	GLuint *indices = new GLuint[n];
	for (int i = 0; i < n; i++) {
		zmq_server->glGenTextures_i++;
		indices[i] = zmq_server->glGenTextures_i;
	}
	c->last_index = indices[n - 1];
	send_data((unsigned char) GL_Server_Command::GLSC_glGenTextures, (void *)c, sizeof(gl_glGenTextures_t));

	memcpy((void *)textures, (void *)indices, sizeof(GLuint) * n);
	// std::cout << __func__ << std::endl;
}
void glActiveTexture(GLenum texture) {
	GL_SET_COMMAND(c, glActiveTexture);
	
	c->texture = texture;
	send_data((unsigned char) GL_Server_Command::GLSC_glActiveTexture, (void *)c, sizeof(gl_glActiveTexture_t));
	// std::cout << "active: " << texture << std::endl;
	// std::cout << __func__ << std::endl;
}
void glBindTexture(GLenum target, GLuint texture) {
	GL_SET_COMMAND(c, glBindTexture);
	
	c->target = target;
	c->texture = texture;
	send_data((unsigned char) GL_Server_Command::GLSC_glBindTexture, (void *)c, sizeof(gl_glBindTexture_t));
	// std::cout << __func__ << std::endl;
}
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) {
	GL_SET_COMMAND(c, glTexImage2D);
	
	c->target = target;
	c->level = level;
	c->internalformat = internalformat;
	c->width = width;
	c->height = height;
	c->border = border;
	c->format = format;
	c->type = type;
	c->pixels = pixels;
	send_data((unsigned char) GL_Server_Command::GLSC_glTexImage2D, (void *)c, sizeof(gl_glTexImage2D_t));
	// std::cout << __func__ << std::endl;
}
void glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) {
	GL_SET_COMMAND(c, glTexStorage2D);
	
	c->target = target;
	c->levels = levels;
	c->internalformat = internalformat;
	c->width = width;
	c->height = height;
	send_data((unsigned char) GL_Server_Command::GLSC_glTexStorage2D, (void *)c, sizeof(gl_glTexStorage2D_t));
	// std::cout << __func__ << std::endl;
}
void glTexParameteri(GLenum target, GLenum pname, GLint param) {
	GL_SET_COMMAND(c, glTexParameteri);
	
	c->target = target;
	c->pname = pname;
	c->param = param;
	send_data((unsigned char) GL_Server_Command::GLSC_glTexParameteri, (void *)c, sizeof(gl_glTexParameteri_t));
	// std::cout << __func__ << std::endl;
}
void glGenFramebuffers(GLsizei n, GLuint *framebuffers) {
	GL_SET_COMMAND(c, glGenFramebuffers);
	
	c->n = n;
	ZMQServer *zmq_server = ZMQServer::get_instance();
	GLuint *indices = new GLuint[n];
	for (int i = 0; i < n; i++) {
		zmq_server->glGenFramebuffers_i++;
		indices[i] = zmq_server->glGenFramebuffers_i;
	}
	c->last_index = indices[n - 1];
	send_data((unsigned char) GL_Server_Command::GLSC_glGenFramebuffers, (void *)c, sizeof(gl_glGenFramebuffers_t));

	memcpy((void *)framebuffers, (void *)indices, sizeof(GLuint) * n);
	// std::cout << __func__ << std::endl;
}
void glBindFramebuffer(GLenum target, GLuint framebuffer) {
	GL_SET_COMMAND(c, glBindFramebuffer);
	
	c->target = target;
	c->framebuffer = framebuffer;
	send_data((unsigned char) GL_Server_Command::GLSC_glBindFramebuffer, (void *)c, sizeof(gl_glBindFramebuffer_t));
	// std::cout << __func__ << std::endl;
}

void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
	GL_SET_COMMAND(c, glFramebufferTexture2D);
	
	c->target = target;
	c->attachment = attachment;
	c->textarget = textarget;
	c->texture = texture;
	c->level = level;
	send_data((unsigned char) GL_Server_Command::GLSC_glFramebufferTexture2D, (void *)c, sizeof(gl_glFramebufferTexture2D_t));
	// std::cout << __func__ << std::endl;
}
GLenum glCheckFramebufferStatus(GLenum target) {
	GL_SET_COMMAND(c, glCheckFramebufferStatus);
	
	c->target = target;
	zmq::message_t result = send_data((unsigned char) GL_Server_Command::GLSC_glCheckFramebufferStatus, (void *)c, sizeof(gl_glCheckFramebufferStatus_t), true);
	GLenum *ret = (GLenum *)result.data();
	return (GLenum)*ret;
	// std::cout << __func__ << std::endl;
}
void glDisable(GLenum cap) {
	GL_SET_COMMAND(c, glDisable);
	
	c->cap = cap;
	send_data((unsigned char) GL_Server_Command::GLSC_glDisable, (void *)c, sizeof(gl_glDisable_t));
	// std::cout << __func__ << std::endl;
}
void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels) {
	GL_SET_COMMAND(c, glTexImage3D);
	
	c->target = target;
	c->level = level;
	c->internalformat = internalformat;
	c->width = width;
	c->height = height;
	c->depth = depth;
	c->border = border;
	c->format = format;
	c->type = type;
	c->pixels = pixels;
	send_data((unsigned char) GL_Server_Command::GLSC_glTexImage3D, (void *)c, sizeof(gl_glTexImage3D_t));
	// std::cout << __func__ << std::endl;
}

// just defined functions
void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name) {
	GL_SET_COMMAND(c, glBindAttribLocation);
	
	c->program = program;
	c->name = name;
	send_data((unsigned char) GL_Server_Command::GLSC_glBindAttribLocation, (void *)c, sizeof(gl_glBindAttribLocation_t));
	// std::cout << __func__ << std::endl;
}
void glBindRenderbuffer(GLenum target, GLuint renderbuffer) {
	GL_SET_COMMAND(c, glBindRenderbuffer);
	
	c->target = target;
	c->renderbuffer = renderbuffer;
	send_data((unsigned char) GL_Server_Command::GLSC_glBindRenderbuffer, (void *)c, sizeof(gl_glBindRenderbuffer_t));
	// std::cout << __func__ << std::endl;
}
void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glBlendEquation(GLenum mode) {
	GL_SET_COMMAND(c, glBlendEquation);
	
	c->mode = mode;
	send_data((unsigned char) GL_Server_Command::GLSC_glBlendEquation, (void *)c, sizeof(gl_glBlendEquation_t));
	// std::cout << __func__ << std::endl;
}
void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glBlendFunc(GLenum sfactor, GLenum dfactor) {
	GL_SET_COMMAND(c, glBlendFunc);
	
	c->sfactor = sfactor;
	c->dfactor = dfactor;
	send_data((unsigned char) GL_Server_Command::GLSC_glBlendFunc, (void *)c, sizeof(gl_glBlendFunc_t));
	// std::cout << __func__ << std::endl;
}
void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {
	GL_SET_COMMAND(c, glBlendFuncSeparate);
	
	c->sfactorRGB = sfactorRGB;
	c->dfactorRGB = dfactorRGB;
	c->sfactorAlpha = sfactorAlpha;
	c->dfactorAlpha = dfactorAlpha;
	send_data((unsigned char) GL_Server_Command::GLSC_glBlendFuncSeparate, (void *)c, sizeof(gl_glBlendFuncSeparate_t));
	// std::cout << __func__ << std::endl;
}
void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data) {
	GL_SET_COMMAND(c, glBufferSubData);
	
	c->target = target;
	c->offset = offset;
	c->size = size;
	c->data = data;
	send_data((unsigned char) GL_Server_Command::GLSC_glBufferSubData, (void *)c, sizeof(gl_glBufferSubData_t));
	// std::cout << __func__ << std::endl;
}

void glClearDepthf(GLfloat d) {
	GL_SET_COMMAND(c, glClearDepthf);
	
	c->d = d;
	send_data((unsigned char) GL_Server_Command::GLSC_glClearDepthf, (void *)c, sizeof(gl_glClearDepthf_t));
	// std::cout << __func__ << std::endl;
}
void glClearStencil(GLint s) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
	GL_SET_COMMAND(c, glColorMask);
	
	c->red = red;
	c->green = green;
	c->blue = blue;
	c->alpha = alpha;
	send_data((unsigned char) GL_Server_Command::GLSC_glColorMask, (void *)c, sizeof(gl_glColorMask_t));
	// std::cout << __func__ << std::endl;
}
void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data) {
	GL_SET_COMMAND(c, glCompressedTexImage2D);
	
	c->target = target;
	c->level = level;
	c->internalformat = internalformat;
	c->width = width;
	c->height = height;
	c->border = border;
	c->imageSize = imageSize;
	c->pixels = data;
	send_data((unsigned char) GL_Server_Command::GLSC_glCompressedTexImage2D, (void *)c, sizeof(gl_glCompressedTexImage2D_t));
	// std::cout << __func__ << std::endl;
}
void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data) {
	// std::cout << __func__ << std::endl;
	std::cout << __func__ << std::endl;
}
void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glCullFace(GLenum mode) {
	GL_SET_COMMAND(c, glCullFace);
	
	c->mode = mode;
	send_data((unsigned char) GL_Server_Command::GLSC_glCullFace, (void *)c, sizeof(gl_glCullFace_t));
	// std::cout << __func__ << std::endl;
}
void glDeleteBuffers(GLsizei n, const GLuint *buffers) {
	GL_SET_COMMAND(c, glDeleteBuffers);
	
	c->n = n;
	c->buffers = buffers;
	send_data((unsigned char) GL_Server_Command::GLSC_glDeleteBuffers, (void *)c, sizeof(gl_glDeleteBuffers_t));
	// std::cout << __func__ << std::endl;
}
void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers) {
	GL_SET_COMMAND(c, glDeleteFramebuffers);
	
	c->n = n;
	c->framebuffers = framebuffers;
	send_data((unsigned char) GL_Server_Command::GLSC_glDeleteFramebuffers, (void *)c, sizeof(gl_glDeleteFramebuffers_t));
	// std::cout << __func__ << std::endl;
}
void glDeleteProgram(GLuint program) {
	// std::cout << __func__ << std::endl;
}
void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers) {
	GL_SET_COMMAND(c, glDeleteRenderbuffers);
	
	c->n = n;
	c->renderbuffers = renderbuffers;
	send_data((unsigned char) GL_Server_Command::GLSC_glDeleteRenderbuffers, (void *)c, sizeof(gl_glDeleteRenderbuffers_t));
	// std::cout << __func__ << std::endl;
}
void glDeleteShader(GLuint shader) {
	// std::cout << __func__ << std::endl;
}
void glDeleteTextures(GLsizei n, const GLuint *textures) {
	GL_SET_COMMAND(c, glDeleteTextures);	
	c->n = n;
	c->textures = textures;
	send_data((unsigned char) GL_Server_Command::GLSC_glDeleteTextures, (void *)c, sizeof(gl_glDeleteTextures_t));
	// std::cout << __func__ << std::endl;
}
void glDepthFunc(GLenum func) {
	GL_SET_COMMAND(c, glDepthFunc);
	
	c->func = func;
	send_data((unsigned char) GL_Server_Command::GLSC_glDepthFunc, (void *)c, sizeof(gl_glDepthFunc_t));
	// std::cout << __func__ << std::endl;
}
void glDepthMask(GLboolean flag) {
	GL_SET_COMMAND(c, glDepthMask);
	
	c->flag = flag;
	send_data((unsigned char) GL_Server_Command::GLSC_glDepthMask, (void *)c, sizeof(gl_glDepthMask_t));
	// std::cout << __func__ << std::endl;
}
void glDepthRangef(GLfloat n, GLfloat f) {
	std::cout << __func__ << std::endl;
}
void glDetachShader(GLuint program, GLuint shader) {
	std::cout << __func__ << std::endl;
}
void glDisableVertexAttribArray(GLuint index) {
	GL_SET_COMMAND(c, glDisableVertexAttribArray);
	
	c->index = index;
	send_data((unsigned char) GL_Server_Command::GLSC_glDisableVertexAttribArray, (void *)c, sizeof(gl_glDisableVertexAttribArray_t));
	// std::cout << __func__ << std::endl;
}
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices) {
	GL_SET_COMMAND(c, glDrawElements);
	
	c->mode = mode;
	c->count = count;
	c->type = type;
	c->indices = (int64_t)indices;
	// std::cout << (int64_t)indices << std::endl;
	send_data((unsigned char) GL_Server_Command::GLSC_glDrawElements, (void *)c, sizeof(gl_glDrawElements_t));
	// std::cout << __func__ << std::endl;
}
void glEnable(GLenum cap) {
	GL_SET_COMMAND(c, glEnable);
	
	c->cap = cap;
	send_data((unsigned char) GL_Server_Command::GLSC_glEnable, (void *)c, sizeof(gl_glEnable_t));
	// std::cout << __func__ << std::endl;
}
void glFinish(void) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
	GL_SET_COMMAND(c, glFramebufferRenderbuffer);
	
	c->target = target;
	c->attachment = attachment;
	c->renderbuffertarget = renderbuffertarget;
	c->renderbuffer = renderbuffer;
	send_data((unsigned char) GL_Server_Command::GLSC_glFramebufferRenderbuffer, (void *)c, sizeof(gl_glFramebufferRenderbuffer_t));
	// std::cout << __func__ << std::endl;
}
void glGenerateMipmap(GLenum target) {
	GL_SET_COMMAND(c, glGenerateMipmap);
	
	c->target = target;
	send_data((unsigned char) GL_Server_Command::GLSC_glGenerateMipmap, (void *)c, sizeof(gl_glGenerateMipmap_t));
	// std::cout << __func__ << std::endl;
}
void glFrontFace(GLenum mode) {
	GL_SET_COMMAND(c, glFrontFace);
	
	c->mode = mode;
	send_data((unsigned char) GL_Server_Command::GLSC_glFrontFace, (void *)c, sizeof(gl_glFrontFace_t));
	// std::cout << __func__ << std::endl;
}
void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers) {
	auto start = std::chrono::steady_clock::now();
	GL_SET_COMMAND(c, glGenRenderbuffers);
	
	c->n = n;
	ZMQServer *zmq_server = ZMQServer::get_instance();
	GLuint *indices = new GLuint[n];
	for (int i = 0; i < n; i++) {
		zmq_server->glGenRenderbuffers_i++;
		indices[i] = zmq_server->glGenRenderbuffers_i;
	}
	c->last_index = indices[n - 1];
	send_data((unsigned char) GL_Server_Command::GLSC_glGenRenderbuffers, (void *)c, sizeof(gl_glGenRenderbuffers_t));

	memcpy((void *)renderbuffers, (void *)indices, sizeof(GLuint) * n);
}
void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glGetBooleanv(GLenum pname, GLboolean *data) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
const GLubyte *glGetString(GLenum name) {
	GL_SET_COMMAND(c, glGetString);
	c->name = name;
	zmq::message_t result = send_data((unsigned char) GL_Server_Command::GLSC_glGetString, (void *)c, sizeof(gl_glGetString_t), true);
	std::string ret = result.to_string();
	return reinterpret_cast<const GLubyte *>(ret.c_str());
}
void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params) {
	// std::cout << __func__ << std::endl;
	std::cout << __func__ << std::endl;
}
void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glGetUniformfv(GLuint program, GLint location, GLfloat *params) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glGetUniformiv(GLuint program, GLint location, GLint *params) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
GLint glGetUniformLocation(GLuint program, const GLchar *name) {
	GL_SET_COMMAND(c, glGetUniformLocation);
	
	c->program = program;
	c->name = name;
	zmq::message_t result = send_data((unsigned char) GL_Server_Command::GLSC_glGetUniformLocation, (void *)c, sizeof(gl_glGetUniformLocation_t), true);
	GLint *ret = (GLint *)result.data();
	return (GLint)*ret;
	// std::cout << __func__ << std::endl;
}
void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glGetVertexAttribPointerv(GLuint index, GLenum pname, void **pointer) {
	// std::cout << __func__ << std::endl;
	std::cout << __func__ << std::endl;
}
void glHint(GLenum target, GLenum mode) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
GLboolean glIsBuffer(GLuint buffer) {
	std::cout << __func__ << std::endl;

	return 0;
	// std::cout << __func__ << std::endl;
}
GLboolean glIsEnabled(GLenum cap) {
	std::cout << __func__ << std::endl;

	return 0;
	// std::cout << __func__ << std::endl;
}
GLboolean glIsFramebuffer(GLuint framebuffer) {
	std::cout << __func__ << std::endl;

	return 0;
	// std::cout << __func__ << std::endl;
}
GLboolean glIsProgram(GLuint program) {
	std::cout << __func__ << std::endl;

	return 0;
	// std::cout << __func__ << std::endl;
}
GLboolean glIsRenderbuffer(GLuint renderbuffer) {
	std::cout << __func__ << std::endl;

	return 0;
	// std::cout << __func__ << std::endl;
}
GLboolean glIsShader(GLuint shader) {
	std::cout << __func__ << std::endl;

	return 0;
	// std::cout << __func__ << std::endl;
}
GLboolean glIsTexture(GLuint texture) {
	std::cout << __func__ << std::endl;

	return 0;
	// std::cout << __func__ << std::endl;
}
void glLineWidth(GLfloat width) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glPixelStorei(GLenum pname, GLint param) {
	switch (pname) {
		case GL_PACK_ALIGNMENT:
			global_pack_alignment = param;
			break;
		case GL_UNPACK_ALIGNMENT:
			global_unpack_alignment = param;
			break;
		default:
			break;
			// std::cout << __func__ << std::endl;
	}
	GL_SET_COMMAND(c, glPixelStorei);
	
	c->pname = pname;
	c->param = param;
	send_data((unsigned char) GL_Server_Command::GLSC_glPixelStorei, (void *)c, sizeof(gl_glPixelStorei_t));
	// std::cout << __func__ << std::endl;
}
void glPolygonOffset(GLfloat factor, GLfloat units) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels) {
	GL_SET_COMMAND(c, glReadPixels);
	
	c->x = x;
	c->y = y;
	c->width = width;
	c->height = height;
	c->format = format;
	c->type = type;
	zmq::message_t result = send_data((unsigned char) GL_Server_Command::GLSC_glReadPixels, (void *)c, sizeof(gl_glReadPixels_t), true);
	void *data = result.data();
	size_t size = 0;
	switch (type) {
		case GL_UNSIGNED_BYTE: {
			if (format == GL_RGBA) {
				size = width * height * 4;
			} else if (format == GL_RGB) {
				size = width * height * 3;
			}
			break;
		}
		default:
			break;
	}
	memcpy(pixels, data, size);
	// std::cout << __func__ << std::endl;
}

void glReleaseShaderCompiler(void) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
	GL_SET_COMMAND(c, glRenderbufferStorage);
	
	c->target = target;
	c->internalformat = internalformat;
	c->width = width;
	c->height = height;
	send_data((unsigned char) GL_Server_Command::GLSC_glRenderbufferStorage, (void *)c, sizeof(gl_glRenderbufferStorage_t));
	// std::cout << __func__ << std::endl;
}
void glSampleCoverage(GLfloat value, GLboolean invert) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}

void glShaderBinary(GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glStencilFunc(GLenum func, GLint ref, GLuint mask) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glStencilMask(GLuint mask) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glStencilMaskSeparate(GLenum face, GLuint mask) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {
	GL_SET_COMMAND(c, glTexParameterf);
	
	c->target = target;
	c->pname = pname;
	c->param = param;
	send_data((unsigned char) GL_Server_Command::GLSC_glTexParameterf, (void *)c, sizeof(gl_glTexParameterf_t));
	// std::cout << __func__ << std::endl;
}
void glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glTexParameteriv(GLenum target, GLenum pname, const GLint *params) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) {
	GL_SET_COMMAND(c, glTexSubImage2D);
	
	c->target = target;
	c->level = level;
	c->xoffset = xoffset;
	c->yoffset = yoffset;
	c->width = width;
	c->height = height;
	c->format = format;
	c->type = type;
	c->pixels = pixels;
	send_data((unsigned char) GL_Server_Command::GLSC_glTexSubImage2D, (void *)c, sizeof(gl_glTexSubImage2D_t));
	// std::cout << __func__ << std::endl;
}
void glUniform1f(GLint location, GLfloat v0) {
	GL_SET_COMMAND(c, glUniform1f);
	
	c->location = location;
	c->v0 = v0;
	send_data((unsigned char) GL_Server_Command::GLSC_glUniform1f, (void *)c, sizeof(gl_glUniform1f_t));
	// std::cout << __func__ << std::endl;
}
void glUniform1fv(GLint location, GLsizei count, const GLfloat *value) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glUniform1i(GLint location, GLint v0) {
	GL_SET_COMMAND(c, glUniform1i);
	
	c->location = location;
	c->v0 = v0;
	// std::cout << location << "\t" << v0 << std::endl;
	send_data((unsigned char) GL_Server_Command::GLSC_glUniform1i, (void *)c, sizeof(gl_glUniform1i_t));
	// std::cout << __func__ << std::endl;
}
void glUniform1iv(GLint location, GLsizei count, const GLint *value) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glUniform2f(GLint location, GLfloat v0, GLfloat v1) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glUniform2fv(GLint location, GLsizei count, const GLfloat *value) {
	GL_SET_COMMAND(c, glUniform2fv);
	
	c->location = location;
	c->count = count;
	c->value = value;
	send_data((unsigned char) GL_Server_Command::GLSC_glUniform2fv, (void *)c, sizeof(gl_glUniform2fv_t));
	// std::cout << __func__ << std::endl;
}
void glUniform2i(GLint location, GLint v0, GLint v1) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glUniform2iv(GLint location, GLsizei count, const GLint *value) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glUniform3fv(GLint location, GLsizei count, const GLfloat *value) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glUniform3iv(GLint location, GLsizei count, const GLint *value) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glUniform4fv(GLint location, GLsizei count, const GLfloat *value) {
	GL_SET_COMMAND(c, glUniform4fv);
	
	c->location = location;
	c->count = count;
	c->value = value;
	send_data((unsigned char) GL_Server_Command::GLSC_glUniform4fv, (void *)c, sizeof(gl_glUniform4fv_t));
	// std::cout << __func__ << std::endl;
}
void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glUniform4iv(GLint location, GLsizei count, const GLint *value) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	GL_SET_COMMAND(c, glUniformMatrix4fv);
	
	c->location = location;
	c->count = count;
	c->transpose = transpose;
	c->value = value;
	send_data((unsigned char) GL_Server_Command::GLSC_glUniformMatrix4fv, (void *)c, sizeof(gl_glUniformMatrix4fv_t));
	// std::cout << __func__ << std::endl;
}
void glValidateProgram(GLuint program) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glVertexAttrib1f(GLuint index, GLfloat x) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glVertexAttrib1fv(GLuint index, const GLfloat *v) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glVertexAttrib2fv(GLuint index, const GLfloat *v) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glVertexAttrib3fv(GLuint index, const GLfloat *v) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	GL_SET_COMMAND(c, glVertexAttrib4f);
	c->index = index;
	c->x = x;
	c->y = y;
	c->z = z;
	c->w = w;
	send_data((unsigned char) GL_Server_Command::GLSC_glVertexAttrib4f, (void *)c, sizeof(gl_glVertexAttrib4f_t));
	// std::cout << __func__ << std::endl;
}
void glVertexAttrib4fv(GLuint index, const GLfloat *v) {
	GL_SET_COMMAND(c, glVertexAttrib4fv);
	
	c->index = index;
	c->v = v;
	send_data((unsigned char) GL_Server_Command::GLSC_glVertexAttrib4fv, (void *)c, sizeof(gl_glVertexAttrib4fv_t));
	// std::cout << __func__ << std::endl;
}
//
void glReadBuffer(GLenum src) {
	GL_SET_COMMAND(c, glReadBuffer);
	
	c->src = src;
	send_data((unsigned char) GL_Server_Command::GLSC_glReadBuffer, (void *)c, sizeof(gl_glReadBuffer_t));
	// std::cout << __func__ << std::endl;
}
void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels) {

	GL_SET_COMMAND(c, glTexSubImage3D);
	
	c->target = target;
	c->level = level;
	c->xoffset = xoffset;
	c->yoffset = yoffset;
	c->zoffset = zoffset;
	c->width = width;
	c->height = height;
	c->depth = depth;
	c->format = format;
	c->type = type;
	c->pixels = pixels;
	send_data((unsigned char) GL_Server_Command::GLSC_glTexSubImage3D, (void *)c, sizeof(gl_glTexSubImage3D_t));
	// std::cout << __func__ << std::endl;
}
void glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
	std::cout << __func__ << std::endl;
}
void glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data) {
	std::cout << __func__ << std::endl;
}
void glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data) {
	std::cout << __func__ << std::endl;
}
void glGenQueries(GLsizei n, GLuint *ids) {
	std::cout << __func__ << std::endl;

	// std::cout << __func__ << std::endl;
}
void glDeleteQueries(GLsizei n, const GLuint *ids) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
GLboolean glIsQuery(GLuint id) {
	std::cout << __func__ << std::endl;
	return 0;
	// std::cout << __func__ << std::endl;
}
void glBeginQuery(GLenum target, GLuint id) {
	std::cout << __func__ << std::endl;
	// std::cout << __func__ << std::endl;
}
void glEndQuery(GLenum target) {
	std::cout << __func__ << std::endl;
}
void glGetQueryiv(GLenum target, GLenum pname, GLint *params) {
	std::cout << __func__ << std::endl;
}
void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params) {
	std::cout << __func__ << std::endl;
}
GLboolean glUnmapBuffer(GLenum target) {
	std::cout << __func__ << std::endl;

	return 0;
	// std::cout << __func__ << std::endl;
}
void glGetBufferPointerv(GLenum target, GLenum pname, void **params) {
	std::cout << __func__ << std::endl;
}
void glDrawBuffers(GLsizei n, const GLenum *bufs) {
	GL_SET_COMMAND(c, glDrawBuffers);
	
	c->n = n;
	c->bufs = bufs;
	send_data((unsigned char) GL_Server_Command::GLSC_glDrawBuffers, (void *)c, sizeof(gl_glDrawBuffers_t));
}
void glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	std::cout << __func__ << std::endl;
}
void glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	std::cout << __func__ << std::endl;
}
void glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	std::cout << __func__ << std::endl;
}
void glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	std::cout << __func__ << std::endl;
}
void glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	std::cout << __func__ << std::endl;
}
void glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	std::cout << __func__ << std::endl;
}
void glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {
	GL_SET_COMMAND(c, glBlitFramebuffer);
	
	c->srcX0 = srcX0;
	c->srcY0 = srcY0;
	c->srcX1 = srcX1;
	c->srcY1 = srcY1;
	c->dstX0 = dstX0;
	c->dstY0 = dstY0;
	c->dstX1 = dstX1;
	c->dstY1 = dstY1;
	c->mask = mask;
	c->filter = filter;
	send_data((unsigned char) GL_Server_Command::GLSC_glBlitFramebuffer, (void *)c, sizeof(gl_glBlitFramebuffer_t));
	// std::cout << __func__ << std::endl;
}
void glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {
	GL_SET_COMMAND(c, glRenderbufferStorageMultisample);
	
	c->target = target;
	c->samples = samples;
	c->internalformat = internalformat;
	c->width = width;
	c->height = height;
	send_data((unsigned char) GL_Server_Command::GLSC_glRenderbufferStorageMultisample, (void *)c, sizeof(gl_glRenderbufferStorageMultisample_t));
	// std::cout << __func__ << std::endl;
}
void glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
	GL_SET_COMMAND(c, glFramebufferTextureLayer);
	
	c->target = target;
	c->attachment = attachment;
	c->texture = texture;
	c->level = level;
	c->layer = layer;
	send_data((unsigned char) GL_Server_Command::GLSC_glFramebufferTextureLayer, (void *)c, sizeof(gl_glFramebufferTextureLayer_t));
	// std::cout << __func__ << std::endl;
}
void *glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) {
	std::cout << __func__ << std::endl;
}
void glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length) {
	std::cout << __func__ << std::endl;
}
void glDeleteVertexArrays(GLsizei n, const GLuint *arrays) {
	GL_SET_COMMAND(c, glDeleteVertexArrays);
	
	c->n = n;
	c->arrays = arrays;
	send_data((unsigned char) GL_Server_Command::GLSC_glDeleteVertexArrays, (void *)c, sizeof(gl_glDeleteVertexArrays_t));
}
GLboolean glIsVertexArray(GLuint array) {
	return 0;
	// std::cout << __func__ << std::endl;
}
void glGetIntegeri_v(GLenum target, GLuint index, GLint *data) {
	std::cout << __func__ << std::endl;
}
void glBeginTransformFeedback(GLenum primitiveMode) {
	GL_SET_COMMAND(c, glBeginTransformFeedback);
	
	c->primitiveMode = primitiveMode;
	send_data((unsigned char) GL_Server_Command::GLSC_glBeginTransformFeedback, (void *)c, sizeof(gl_glBeginTransformFeedback_t));
	// std::cout << __func__ << std::endl;
}
void glEndTransformFeedback(void) {
	GL_SET_COMMAND(c, glEndTransformFeedback);
	
	send_data((unsigned char) GL_Server_Command::GLSC_glEndTransformFeedback, (void *)c, sizeof(gl_glEndTransformFeedback_t));
}
void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
	std::cout << __func__ << std::endl;
}
void glBindBufferBase(GLenum target, GLuint index, GLuint buffer) {
	GL_SET_COMMAND(c, glBindBufferBase);
	
	c->target = target;
	c->index = index;
	c->buffer = buffer;
	send_data((unsigned char) GL_Server_Command::GLSC_glBindBufferBase, (void *)c, sizeof(gl_glBindBufferBase_t));
	// std::cout << __func__ << std::endl;
}
void glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar *const *varyings, GLenum bufferMode) {
	GL_SET_COMMAND(c, glTransformFeedbackVaryings);
	
	c->program = program;
	c->count = count;
	c->varyings = varyings;
	c->bufferMode = bufferMode;
	send_data((unsigned char) GL_Server_Command::GLSC_glTransformFeedbackVaryings, (void *)c, sizeof(gl_glTransformFeedbackVaryings_t));
}
void glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name) {
	std::cout << __func__ << std::endl;
}
void glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer) {
	std::cout << __func__ << std::endl;
}
void glGetVertexAttribIiv(GLuint index, GLenum pname, GLint *params) {
	std::cout << __func__ << std::endl;
}
void glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint *params) {
	std::cout << __func__ << std::endl;
}
void glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w) {
	std::cout << __func__ << std::endl;
}
void glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) {
	std::cout << __func__ << std::endl;
}
void glVertexAttribI4iv(GLuint index, const GLint *v) {
	std::cout << __func__ << std::endl;
}
void glVertexAttribI4uiv(GLuint index, const GLuint *v) {
	std::cout << __func__ << std::endl;
}
void glGetUniformuiv(GLuint program, GLint location, GLuint *params) {
	std::cout << __func__ << std::endl;
}
GLint glGetFragDataLocation(GLuint program, const GLchar *name) {

	return 0;
	// std::cout << __func__ << std::endl;
}
void glUniform1ui(GLint location, GLuint v0) {
	GL_SET_COMMAND(c, glUniform1ui);
	
	c->location = location;
	c->v0 = v0;
	send_data((unsigned char) GL_Server_Command::GLSC_glUniform1ui, (void *)c, sizeof(gl_glUniform1ui_t));
}
void glUniform2ui(GLint location, GLuint v0, GLuint v1) {
	std::cout << __func__ << std::endl;
}
void glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2) {
	std::cout << __func__ << std::endl;
}
void glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {
	std::cout << __func__ << std::endl;
}
void glUniform1uiv(GLint location, GLsizei count, const GLuint *value) {
	std::cout << __func__ << std::endl;
}
void glUniform2uiv(GLint location, GLsizei count, const GLuint *value) {
	std::cout << __func__ << std::endl;
}
void glUniform3uiv(GLint location, GLsizei count, const GLuint *value) {
	std::cout << __func__ << std::endl;
}
void glUniform4uiv(GLint location, GLsizei count, const GLuint *value) {
	std::cout << __func__ << std::endl;
}
void glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value) {
	std::cout << __func__ << std::endl;
}
void glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value) {
	std::cout << __func__ << std::endl;
}
void glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value) {
	GL_SET_COMMAND(c, glClearBufferfv);
	
	c->buffer = buffer;
	c->drawbuffer = drawbuffer;
	c->value = value;
	send_data((unsigned char) GL_Server_Command::GLSC_glClearBufferfv, (void *)c, sizeof(gl_glClearBufferfv_t));
	// std::cout << __func__ << std::endl;
}
void glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
	std::cout << __func__ << std::endl;
}
const GLubyte *glGetStringi(GLenum name, GLuint index) {
	GL_SET_COMMAND(c, glGetStringi);
	
	c->name = name;
	c->index = index;
	zmq::message_t result = send_data((unsigned char) GL_Server_Command::GLSC_glGetStringi, (void *)c, sizeof(gl_glGetStringi_t), true);
	std::string ret = result.to_string();
	// const GLubyte *ret = reinterpret_cast<const GLubyte *>(send_data((unsigned char) GL_Server_Command::GLSC_glGetStringi, (void *)c, sizeof(gl_glGetStringi_t)));
	// std::cout << ret.c_str() << std::endl;
	return reinterpret_cast<const GLubyte *>(ret.c_str());
	// std::cout << __func__ << std::endl;
}
void glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) {
	std::cout << __func__ << std::endl;
}
void glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar *const *uniformNames, GLuint *uniformIndices) {
	std::cout << __func__ << std::endl;
}
void glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params) {
	std::cout << __func__ << std::endl;
}
GLuint glGetUniformBlockIndex(GLuint program, const GLchar *uniformBlockName) {
	GL_SET_COMMAND(c, glGetUniformBlockIndex);
	
	c->program = program;
	c->name = uniformBlockName;
	zmq::message_t result = send_data((unsigned char) GL_Server_Command::GLSC_glGetUniformBlockIndex, (void *)c, sizeof(gl_glGetUniformBlockIndex_t), true);
	GLuint *ret = (GLuint *)result.data();
	return (GLuint)*ret;
	// std::cout << __func__ << std::endl;
}
void glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params) {
	// std::cout << __func__ << std::endl;
}
void glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName) {
	// std::cout << __func__ << std::endl;
}
void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) {
	GL_SET_COMMAND(c, glUniformBlockBinding);
	
	c->program = program;
	c->uniformBlockIndex = uniformBlockIndex;
	c->uniformBlockBinding = uniformBlockBinding;
	send_data((unsigned char) GL_Server_Command::GLSC_glUniformBlockBinding, (void *)c, sizeof(gl_glUniformBlockBinding_t));
	// std::cout << __func__ << std::endl;
}
void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) {
	GL_SET_COMMAND(c, glDrawArraysInstanced);
	
	c->mode = mode;
	c->first = first;
	c->count = count;
	c->instancecount = instancecount;
	send_data((unsigned char) GL_Server_Command::GLSC_glDrawArraysInstanced, (void *)c, sizeof(gl_glDrawArraysInstanced_t));

	// std::cout << __func__ << std::endl;
}
void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount) {
	std::cout << __func__ << std::endl;
}
GLsync glFenceSync(GLenum condition, GLbitfield flags) {
	std::cout << __func__ << std::endl;
	return 0;
	// std::cout << __func__ << std::endl;
}
GLboolean glIsSync(GLsync sync) {
	std::cout << __func__ << std::endl;
	return 0;
}
void glDeleteSync(GLsync sync) {
	std::cout << __func__ << std::endl;
}
GLenum glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) {
	std::cout << __func__ << std::endl;
	return 0;
}
void glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) {
	std::cout << __func__ << std::endl;
}
void glGetInteger64v(GLenum pname, GLint64 *data) {
	std::cout << __func__ << std::endl;
}
void glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values) {
	std::cout << __func__ << std::endl;
}
void glGetInteger64i_v(GLenum target, GLuint index, GLint64 *data) {
	std::cout << __func__ << std::endl;
}
void glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64 *params) {
	std::cout << __func__ << std::endl;
}
void glGenSamplers(GLsizei count, GLuint *samplers) {
	std::cout << __func__ << std::endl;
}
void glDeleteSamplers(GLsizei count, const GLuint *samplers) {
	std::cout << __func__ << std::endl;
}
GLboolean glIsSampler(GLuint sampler) {
	std::cout << __func__ << std::endl;
	return 0;
}
void glBindSampler(GLuint unit, GLuint sampler) {
	std::cout << __func__ << std::endl;
}
void glSamplerParameteri(GLuint sampler, GLenum pname, GLint param) {
	std::cout << __func__ << std::endl;
}
void glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint *param) {
	std::cout << __func__ << std::endl;
}
void glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param) {
	std::cout << __func__ << std::endl;
}
void glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat *param) {
	std::cout << __func__ << std::endl;
}
void glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint *params) {
	std::cout << __func__ << std::endl;
}
void glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat *params) {
	std::cout << __func__ << std::endl;
}
void glVertexAttribDivisor(GLuint index, GLuint divisor) {
	GL_SET_COMMAND(c, glVertexAttribDivisor);
	
	c->index = index;
	c->divisor = divisor;
	send_data((unsigned char) GL_Server_Command::GLSC_glVertexAttribDivisor, (void *)c, sizeof(gl_glVertexAttribDivisor_t));
	// std::cout << __func__ << std::endl;
}
void glBindTransformFeedback(GLenum target, GLuint id) {
	std::cout << __func__ << std::endl;
}
void glDeleteTransformFeedbacks(GLsizei n, const GLuint *ids) {
	std::cout << __func__ << std::endl;
}
void glGenTransformFeedbacks(GLsizei n, GLuint *ids) {
	std::cout << __func__ << std::endl;
}
GLboolean glIsTransformFeedback(GLuint id) {
	std::cout << __func__ << std::endl;
	return 0;
}
void glPauseTransformFeedback(void) {
	std::cout << __func__ << std::endl;
}
void glResumeTransformFeedback(void) {
	std::cout << __func__ << std::endl;
}
void glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary) {
	std::cout << __func__ << std::endl;
}
void glProgramBinary(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length) {
	std::cout << __func__ << std::endl;
}
void glProgramParameteri(GLuint program, GLenum pname, GLint value) {
	std::cout << __func__ << std::endl;
}
void glInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments) {
	std::cout << __func__ << std::endl;
}
void glInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height) {
	std::cout << __func__ << std::endl;
}
void glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) {
	std::cout << __func__ << std::endl;
}
void glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params) {
	std::cout << __func__ << std::endl;
}