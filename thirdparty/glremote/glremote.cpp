// #include <tbb/concurrent_queue.h>
#include <iostream>
#include <string>
#include <type_traits>
#include <zmq.hpp>
// #include <GL/glew.h>
#include "gl_commands.h"
#include "glremote/glremote.h"

#define GL_SET_COMMAND(PTR, FUNCNAME)                                                \
	gl_##FUNCNAME##_t *PTR = (gl_##FUNCNAME##_t *)malloc(sizeof(gl_##FUNCNAME##_t)); \
	PTR->cmd = GLSC_##FUNCNAME

GLint global_pack_alignment = 4;
GLint global_unpack_alignment = 4;

std::string send_data_get_string(unsigned int cmd, void *cmd_data, int size) {
	zmq::context_t ctx;
	zmq::socket_t sock2(ctx, zmq::socket_type::req);

	sock2.connect("tcp://127.0.0.1:12345");

	gl_command_t c = {
		cmd, size
	};
	zmq::message_t msg(sizeof(c));
	switch (cmd) {
		case GLSC_glGetStringi:
		case GLSC_glGetString:
		default: {
			memcpy(msg.data(), (void *)&c, sizeof(c));

			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::none);

			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
	}
	return msg.to_string();
}

void *send_data(unsigned int cmd, void *cmd_data, int size) {
	zmq::context_t ctx;
	zmq::socket_t sock2(ctx, zmq::socket_type::req);

	sock2.connect("tcp://127.0.0.1:12345");

	gl_command_t c = {
		cmd, size
	};
	zmq::message_t msg(sizeof(c));

	switch (cmd) {
		case GLSC_glClearBufferfv: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			// gl_command_t* cc = (gl_command_t *)msg.data();
			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glClearBufferfv_t *more_data = (gl_glClearBufferfv_t *)cmd_data;

			zmq::message_t buffer_data(sizeof(GLfloat) * 4); //
			memcpy(buffer_data.data(), more_data->value, sizeof(GLfloat) * 4);
			sock2.send(buffer_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		case GLSC_glDeleteFramebuffers: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			// gl_command_t* cc = (gl_command_t *)msg.data();
			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glDeleteFramebuffers_t *more_data = (gl_glDeleteFramebuffers_t *)cmd_data;
			zmq::message_t framebuffers_data(sizeof(GLuint) * more_data->n);
			memcpy(framebuffers_data.data(), more_data->framebuffers, sizeof(GLuint) * more_data->n);
			sock2.send(framebuffers_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		case GLSC_glDeleteRenderbuffers: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			// gl_command_t* cc = (gl_command_t *)msg.data();
			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glDeleteRenderbuffers_t *more_data = (gl_glDeleteRenderbuffers_t *)cmd_data;
			zmq::message_t renderbuffers_data(sizeof(GLuint) * more_data->n);
			memcpy(renderbuffers_data.data(), more_data->renderbuffers, sizeof(GLuint) * more_data->n);
			sock2.send(renderbuffers_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		case GLSC_glDeleteTextures: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			// gl_command_t* cc = (gl_command_t *)msg.data();
			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glDeleteTextures_t *more_data = (gl_glDeleteTextures_t *)cmd_data;
			zmq::message_t textures_data(sizeof(GLuint) * more_data->n);
			memcpy(textures_data.data(), more_data->textures, sizeof(GLuint) * more_data->n);
			sock2.send(textures_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		case GLSC_glShaderSource: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			// gl_command_t* cc = (gl_command_t *)msg.data();
			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glShaderSource_t *more_data = (gl_glShaderSource_t *)cmd_data;
			// std::
			for (int i = 0; i < more_data->count; i++) {
				zmq::message_t string_data;
				if (strlen(more_data->string[i]) > 0) {
					string_data.rebuild(strlen(more_data->string[i]));
					memcpy(string_data.data(), more_data->string[i], strlen(more_data->string[i]));
				}
				// std::cout << "data size::\t" << string_data.size() << std::endl;

				if (i == more_data->count - 1) {
					sock2.send(string_data, zmq::send_flags::none);
				} else {
					sock2.send(string_data, zmq::send_flags::sndmore);
				}
			}

			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		case GLSC_glBufferData: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glBufferData_t *more_data = (gl_glBufferData_t *)cmd_data;
			zmq::message_t buffer_data;
			if (more_data->data != NULL) {
				buffer_data.rebuild(more_data->size);
				memcpy(buffer_data.data(), more_data->data, more_data->size);
			}
			sock2.send(buffer_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		case GLSC_glBufferSubData: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glBufferSubData_t *more_data = (gl_glBufferSubData_t *)cmd_data;
			zmq::message_t buffer_data;
			if (more_data->data != NULL) {
				buffer_data.rebuild(more_data->size);
				memcpy(buffer_data.data(), more_data->data, more_data->size);
			}
			sock2.send(buffer_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		case GLSC_glUniformMatrix4fv: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glUniformMatrix4fv_t *more_data = (gl_glUniformMatrix4fv_t *)cmd_data;
			zmq::message_t buffer_data;
			if (more_data->value != NULL) {
				buffer_data.rebuild(more_data->count * 16 * sizeof(GLfloat));
				memcpy(buffer_data.data(), more_data->value, more_data->count * 16 * sizeof(GLfloat));
			}
			sock2.send(buffer_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		case GLSC_glUniform4fv: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glUniform4fv_t *more_data = (gl_glUniform4fv_t *)cmd_data;
			zmq::message_t buffer_data;
			if (more_data->value != NULL) {

				buffer_data.rebuild(more_data->count * 4 * sizeof(GLfloat));
				memcpy(buffer_data.data(), more_data->value, more_data->count * 4 * sizeof(GLfloat));
			}
			sock2.send(buffer_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		case GLSC_glVertexAttrib4fv: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glVertexAttrib4fv_t *more_data = (gl_glVertexAttrib4fv_t *)cmd_data;
			zmq::message_t buffer_data;
			if (more_data->v != NULL) {
				buffer_data.rebuild(sizeof(GLfloat) * 4);
				memcpy(buffer_data.data(), more_data->v, sizeof(GLfloat) * 4);
			}
			sock2.send(buffer_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		case GLSC_glUniform2fv: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glUniform2fv_t *more_data = (gl_glUniform2fv_t *)cmd_data;
			zmq::message_t buffer_data;
			if (more_data->value != NULL) {
				buffer_data.rebuild(more_data->count * 2);
				memcpy(buffer_data.data(), more_data->value, more_data->count * 4);
			}
			sock2.send(buffer_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		case GLSC_glDrawElements: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glDrawElements_t *more_data = (gl_glDrawElements_t *)cmd_data;
			zmq::message_t buffer_data;
			buffer_data.rebuild(sizeof(int64_t));

			memcpy(buffer_data.data(), &more_data->indices, sizeof(int64_t));
			sock2.send(buffer_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		case GLSC_glGetAttribLocation: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glGetAttribLocation_t *more_data = (gl_glGetAttribLocation_t *)cmd_data;
			zmq::message_t buffer_data(strlen(more_data->name));
			memcpy(buffer_data.data(), more_data->name, strlen(more_data->name));
			sock2.send(buffer_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);

			break;
		}
		case GLSC_glBindAttribLocation: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glBindAttribLocation_t *more_data = (gl_glBindAttribLocation_t *)cmd_data;
			zmq::message_t buffer_data(strlen(more_data->name));
			memcpy(buffer_data.data(), more_data->name, strlen(more_data->name));
			sock2.send(buffer_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);

			break;
		}
		case GLSC_glGetUniformLocation: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glGetUniformLocation_t *more_data = (gl_glGetUniformLocation_t *)cmd_data;
			zmq::message_t buffer_data(strlen(more_data->name));
			memcpy(buffer_data.data(), more_data->name, strlen(more_data->name));
			sock2.send(buffer_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);

			break;
		}
		case GLSC_glGetUniformBlockIndex: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glGetUniformBlockIndex_t *more_data = (gl_glGetUniformBlockIndex_t *)cmd_data;
			zmq::message_t buffer_data(strlen(more_data->name));
			memcpy(buffer_data.data(), more_data->name, strlen(more_data->name));
			sock2.send(buffer_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);

			break;
		}
		case GLSC_glTexSubImage2D: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			sock2.send(msg, zmq::send_flags::sndmore);

			// send others except pixel data;
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);
			uint32_t pixelbytes, linebytes, datasize;

			gl_glTexSubImage2D_t *more_data = (gl_glTexSubImage2D_t *)cmd_data;
			switch (more_data->type) {
				case GL_UNSIGNED_BYTE:
					switch (more_data->format) {
						case GL_ALPHA:
							pixelbytes = 1;
							break;
						case GL_RGB:
							pixelbytes = 3;
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
			linebytes = (pixelbytes * more_data->width + global_unpack_alignment - 1) & (~(global_unpack_alignment - 1)); // 4 willbe replaced with pixelstorei param
			datasize = linebytes * more_data->height;

			zmq::message_t pixel_data;

			if (more_data->pixels != NULL) {
				pixel_data.rebuild(datasize);
				// std::cout << "Not NULL" << std::endl;
				memcpy(pixel_data.data(), more_data->pixels, datasize);
			}

			sock2.send(pixel_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		case GLSC_glTexImage2D: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			sock2.send(msg, zmq::send_flags::sndmore);

			// send others except pixel data;
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);
			uint32_t pixelbytes, linebytes, datasize;

			gl_glTexImage2D_t *more_data = (gl_glTexImage2D_t *)cmd_data;
			switch (more_data->type) {
				case GL_UNSIGNED_BYTE:
					switch (more_data->format) {
						case GL_ALPHA:
							pixelbytes = 1;
							break;
						case GL_RGB:
							pixelbytes = 3;
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
			linebytes = (pixelbytes * more_data->width + global_unpack_alignment - 1) & (~(global_unpack_alignment - 1)); // 4 willbe replaced with pixelstorei param
			datasize = linebytes * more_data->height;

			zmq::message_t pixel_data;

			if (more_data->pixels != NULL) {
				pixel_data.rebuild(datasize);
				// std::cout << "Not NULL" << std::endl;
				memcpy(pixel_data.data(), more_data->pixels, datasize);
			}

			sock2.send(pixel_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		case GLSC_glCompressedTexImage2D: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			sock2.send(msg, zmq::send_flags::sndmore);

			// send others except pixel data;
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);

			gl_glCompressedTexImage2D_t *more_data = (gl_glCompressedTexImage2D_t *)cmd_data;

			zmq::message_t pixel_data;

			if (more_data->pixels != NULL) {
				pixel_data.rebuild(more_data->imageSize);
				memcpy(pixel_data.data(), more_data->pixels, more_data->imageSize);
			}

			sock2.send(pixel_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}

		case GLSC_glTexSubImage3D: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			sock2.send(msg, zmq::send_flags::sndmore);

			// send others except pixel data;
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);
			uint32_t pixelbytes, linebytes, datasize;

			gl_glTexSubImage3D_t *more_data = (gl_glTexSubImage3D_t *)cmd_data;
			switch (more_data->type) {
				case GL_UNSIGNED_BYTE:
					switch (more_data->format) {
						case GL_ALPHA:
							pixelbytes = 1;
							break;
						case GL_RGB:
							pixelbytes = 3;
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
			linebytes = (pixelbytes * more_data->width + global_unpack_alignment - 1) & (~(global_unpack_alignment - 1)); // 4 willbe replaced with pixelstorei param
			datasize = linebytes * more_data->height * more_data->depth;

			zmq::message_t pixel_data;

			if (more_data->pixels != NULL) {
				pixel_data.rebuild(datasize);
				// std::cout << "Not NULL" << std::endl;
				memcpy(pixel_data.data(), more_data->pixels, datasize);
			}

			sock2.send(pixel_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		case GLSC_glTexImage3D: {
			memcpy(msg.data(), (void *)&c, sizeof(c));
			sock2.send(msg, zmq::send_flags::sndmore);

			// send others except pixel data;
			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::sndmore);
			uint32_t pixelbytes, linebytes, datasize;

			gl_glTexImage3D_t *more_data = (gl_glTexImage3D_t *)cmd_data;
			switch (more_data->type) {
				case GL_UNSIGNED_BYTE:
					switch (more_data->format) {
						case GL_ALPHA:
							pixelbytes = 1;
							break;
						case GL_RGB:
							pixelbytes = 3;
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
			linebytes = (pixelbytes * more_data->width + global_unpack_alignment - 1) & (~(global_unpack_alignment - 1)); // 4 willbe replaced with pixelstorei param
			datasize = linebytes * more_data->height * more_data->depth;

			zmq::message_t pixel_data;

			if (more_data->pixels != NULL) {
				pixel_data.rebuild(datasize);
				// std::cout << "Not NULL" << std::endl;
				memcpy(pixel_data.data(), more_data->pixels, datasize);
			}

			sock2.send(pixel_data, zmq::send_flags::none);
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		case GLSC_BREAK:
		case GLSC_bufferSwap: {
			// zmq::message_t msg(sizeof(c));
			memcpy(msg.data(), (void *)&c, sizeof(c));

			sock2.send(msg, zmq::send_flags::none); // send cmd
			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
		default: {
			memcpy(msg.data(), (void *)&c, sizeof(c));

			sock2.send(msg, zmq::send_flags::sndmore);

			zmq::message_t data_msg(size);
			memcpy(data_msg.data(), cmd_data, size);
			sock2.send(data_msg, zmq::send_flags::none);

			sock2.recv(msg, zmq::recv_flags::none);
			break;
		}
	}

	return msg.data();
}

void glViewport(int x, int y, int width, int height) {
	GL_SET_COMMAND(c, glViewport);
	c->cmd = GLSC_glViewport;
	c->x = x;
	c->y = y;
	c->width = width;
	c->height = height;
	send_data(GLSC_glViewport, (void *)c, sizeof(gl_glViewport_t));
}

void glClear(GLbitfield mask) {
	GL_SET_COMMAND(c, glClear);
	c->mask = mask;
	c->cmd = GLSC_glClear;
	send_data(GLSC_glClear, (void *)c, sizeof(gl_glClear_t));
}

void glBegin(GLenum mode) {
	GL_SET_COMMAND(c, glBegin);
	c->mode = mode;
	c->cmd = GLSC_glBegin;
	send_data(GLSC_glBegin, (void *)c, sizeof(gl_glBegin_t));
}
void glColor3f(float red, float green, float blue) {
	GL_SET_COMMAND(c, glColor3f);
	c->red = red;
	c->green = green;
	c->blue = blue;
	c->cmd = GLSC_glColor3f;
	// std::cout << c->red << c->green << c->blue << std::endl;

	send_data(GLSC_glColor3f, (void *)c, sizeof(gl_glColor3f_t));
	free(c);
}
void glVertex3f(float x, float y, float z) {
	GL_SET_COMMAND(c, glVertex3f);
	c->x = x;
	c->y = y;
	c->z = z;
	c->cmd = GLSC_glVertex3f;

	send_data(GLSC_glVertex3f, (void *)c, sizeof(gl_glVertex3f_t));
	free(c);
}

void glEnd() {
	GL_SET_COMMAND(c, glEnd);
	c->cmd = GLSC_glEnd;
	send_data(GLSC_glEnd, (void *)c, sizeof(gl_glEnd_t));
}

void glFlush() {
	GL_SET_COMMAND(c, glFlush);
	c->cmd = GLSC_glFlush;
	send_data(GLSC_glFlush, (void *)c, sizeof(gl_glFlush_t));
}

void glBreak() {
	gl_command_t *c = (gl_command_t *)malloc(sizeof(gl_command_t));
	c->cmd = GLSC_BREAK;
	c->size = sizeof(gl_command_t);
	send_data(GLSC_BREAK, (void *)c, sizeof(gl_command_t));
}
void glSwapBuffer() {
	gl_command_t *c = (gl_command_t *)malloc(sizeof(gl_command_t));
	c->cmd = GLSC_bufferSwap;
	c->size = sizeof(gl_command_t);
	send_data(GLSC_bufferSwap, (void *)c, sizeof(gl_command_t));
}
unsigned int glCreateShader(unsigned int type) {
	GL_SET_COMMAND(c, glCreateShader);
	c->cmd = GLSC_glCreateShader;
	c->type = type;
	unsigned int *ret = (unsigned int *)send_data(GLSC_glCreateShader, (void *)c, sizeof(gl_glCreateShader_t));
	return (unsigned int)*ret;
}
GLenum glGetError(void) {
	GL_SET_COMMAND(c, glGetError);
	c->cmd = GLSC_glGetError;
	unsigned int *ret = (unsigned int *)send_data(GLSC_glGetError, (void *)c, sizeof(gl_glGetError_t));
	return static_cast<GLenum>(*ret);
}

void glShaderSource(int shader, int count, const char *const *string, const int *length) {
	GL_SET_COMMAND(c, glShaderSource);
	c->cmd = GLSC_glShaderSource;
	c->shader = shader;
	c->count = count;
	c->string = string;
	c->length = length;
	send_data(GLSC_glShaderSource, (void *)c, sizeof(gl_glShaderSource_t));
	//SEND_MORE
}
void glCompileShader(int shader) {
	GL_SET_COMMAND(c, glCompileShader);
	c->cmd = GLSC_glCompileShader;
	c->shader = shader;
	send_data(GLSC_glCompileShader, (void *)c, sizeof(gl_glCompileShader_t));
}

void glGetShaderiv(int shader, unsigned int pname, int *param) {
	GL_SET_COMMAND(c, glGetShaderiv);
	c->cmd = GLSC_glGetShaderiv;
	c->shader = shader;
	c->pname = pname;
	int *ret = (int *)send_data(GLSC_glGetShaderiv, (void *)c, sizeof(gl_glGetShaderiv_t));
	memcpy((void *)param, (void *)ret, sizeof(unsigned int));
}

unsigned int glCreateProgram() {
	GL_SET_COMMAND(c, glCreateProgram);
	c->cmd = GLSC_glCreateProgram;
	unsigned int *ret = (unsigned int *)send_data(GLSC_glCreateProgram, (void *)c, sizeof(gl_glCreateProgram_t));
	return (unsigned int)*ret;
}

void glAttachShader(unsigned int program, int shader) {
	GL_SET_COMMAND(c, glAttachShader);
	c->cmd = GLSC_glAttachShader;
	c->program = program;
	c->shader = shader;
	send_data(GLSC_glAttachShader, (void *)c, sizeof(gl_glAttachShader_t));
}

void glLinkProgram(unsigned int program) {
	GL_SET_COMMAND(c, glLinkProgram);
	c->cmd = GLSC_glLinkProgram;
	c->program = program;
	send_data(GLSC_glLinkProgram, (void *)c, sizeof(gl_glLinkProgram_t));
}

void glGetProgramiv(int program, unsigned int pname, int *param) {
	GL_SET_COMMAND(c, glGetProgramiv);
	c->cmd = GLSC_glGetProgramiv;
	c->program = program;
	c->pname = pname;
	int *ret = (int *)send_data(GLSC_glGetProgramiv, (void *)c, sizeof(gl_glGetProgramiv_t));
	// param  = (int*) send_data(GLSC_glGetProgramiv, (void*)c, sizeof(gl_glGetProgramiv_t));
	memcpy((void *)param, (void *)ret, sizeof(unsigned int));
}

void glGenBuffers(int n, GLuint *buffers) {
	GL_SET_COMMAND(c, glGenBuffers);
	c->cmd = GLSC_glGenBuffers;
	c->n = n;
	// buffers = (unsigned int*) send_data(GLSC_glGenBuffers, (void*)c, sizeof(gl_glGenBuffers_t));

	unsigned int *ret = (unsigned int *)send_data(GLSC_glGenBuffers, (void *)c, sizeof(gl_glGenBuffers_t));
	memcpy((void *)buffers, (void *)ret, sizeof(GLuint));
}

void glBindBuffer(unsigned int target, unsigned int id) {
	GL_SET_COMMAND(c, glBindBuffer);
	c->cmd = GLSC_glBindBuffer;
	c->target = target;
	c->id = id;
	send_data(GLSC_glBindBuffer, (void *)c, sizeof(gl_glBindBuffer_t));
}

void glBufferData(unsigned int target, long int size, const void *data, unsigned int usage) {
	GL_SET_COMMAND(c, glBufferData);
	c->cmd = GLSC_glBufferData;
	c->target = target;
	c->size = size;
	c->data = data;
	c->usage = usage;
	send_data(GLSC_glBufferData, (void *)c, sizeof(gl_glBufferData_t));
}

void glGenVertexArrays(GLsizei n, GLuint *arrays) {
	GL_SET_COMMAND(c, glGenVertexArrays);
	c->cmd = GLSC_glGenVertexArrays;
	c->n = n;

	unsigned int *ret = (unsigned int *)send_data(GLSC_glGenVertexArrays, (void *)c, sizeof(gl_glGenVertexArrays_t));
	memcpy((void *)arrays, (void *)ret, sizeof(unsigned int));
}

void glBindVertexArray(unsigned int array) {
	GL_SET_COMMAND(c, glBindVertexArray);
	c->cmd = GLSC_glBindVertexArray;
	c->array = array;
	send_data(GLSC_glBindVertexArray, (void *)c, sizeof(gl_glBindVertexArray_t));
}

int glGetAttribLocation(unsigned int programObj, const char *name) {
	GL_SET_COMMAND(c, glGetAttribLocation);
	c->cmd = GLSC_glGetAttribLocation;
	c->programObj = programObj;
	c->name = name;
	int *ret = (int *)send_data(GLSC_glGetAttribLocation, (void *)c, sizeof(gl_glGetAttribLocation_t));
	return (int)*ret;
}
void glVertexAttribPointer(unsigned int index, int size, unsigned int type, bool normalized, int stride, const void *pointer) {
	GL_SET_COMMAND(c, glVertexAttribPointer);
	c->cmd = GLSC_glVertexAttribPointer;
	c->index = index;
	c->size = size;
	c->type = type;
	c->normalized = normalized;
	c->stride = stride;
	c->pointer = pointer;
	send_data(GLSC_glVertexAttribPointer, (void *)c, sizeof(gl_glVertexAttribPointer_t));
}

void glEnableVertexAttribArray(unsigned int index) {
	GL_SET_COMMAND(c, glEnableVertexAttribArray);
	c->cmd = GLSC_glEnableVertexAttribArray;
	c->index = index;
	send_data(GLSC_glEnableVertexAttribArray, (void *)c, sizeof(gl_glEnableVertexAttribArray_t));
}
void glUseProgram(unsigned int program) {
	GL_SET_COMMAND(c, glUseProgram);
	c->cmd = GLSC_glUseProgram;
	c->program = program;
	send_data(GLSC_glUseProgram, (void *)c, sizeof(gl_glUseProgram_t));
}

void glClearColor(float red, float green, float blue, float alpha) {
	GL_SET_COMMAND(c, glClearColor);
	c->cmd = GLSC_glClearColor;
	c->red = red;
	c->green = green;
	c->blue = blue;
	c->alpha = alpha;
	send_data(GLSC_glClearColor, (void *)c, sizeof(gl_glClearColor_t));
}
void glDrawArrays(unsigned int mode, int first, int count) {
	GL_SET_COMMAND(c, glDrawArrays);
	c->cmd = GLSC_glDrawArrays;
	c->mode = mode;
	c->first = first;
	c->count = count;
	send_data(GLSC_glDrawArrays, (void *)c, sizeof(gl_glDrawArrays_t));
}
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {
	GL_SET_COMMAND(c, glScissor);
	c->cmd = GLSC_glScissor;
	c->x = x;
	c->y = y;
	c->width = width;
	c->height = height;
	send_data(GLSC_glScissor, (void *)c, sizeof(gl_glScissor_t));
}
void glGetIntegerv(GLenum pname, GLint *data) {
	GL_SET_COMMAND(c, glGetIntegerv);
	c->cmd = GLSC_glGetIntegerv;
	c->pname = pname;
	c->data = data;
	GLint *ret = (GLint *)send_data(GLSC_glGetIntegerv, (void *)c, sizeof(gl_glGetIntegerv_t));
	memcpy((void *)data, (void *)ret, sizeof(GLint));
}
void glGetFloatv(GLenum pname, GLfloat *data) {
	GL_SET_COMMAND(c, glGetFloatv);
	c->cmd = GLSC_glGetFloatv;
	c->pname = pname;
	c->data = data;
	GLfloat *ret = (GLfloat *)send_data(GLSC_glGetFloatv, (void *)c, sizeof(gl_glGetFloatv_t));
	memcpy((void *)data, (void *)ret, sizeof(GLfloat));
}
void glGenTextures(GLsizei n, GLuint *textures) {
	GL_SET_COMMAND(c, glGenTextures);
	c->cmd = GLSC_glGenTextures;
	c->n = n;
	GLuint *ret = (GLuint *)send_data(GLSC_glGenTextures, (void *)c, sizeof(gl_glGenTextures_t));
	memcpy((void *)textures, (void *)ret, sizeof(GLuint));
	// std::cout << *textures << "\t" << *ret << std::endl;
}
void glActiveTexture(GLenum texture) {
	GL_SET_COMMAND(c, glActiveTexture);
	c->cmd = GLSC_glActiveTexture;
	c->texture = texture;
	send_data(GLSC_glActiveTexture, (void *)c, sizeof(gl_glActiveTexture_t));
}
void glBindTexture(GLenum target, GLuint texture) {
	GL_SET_COMMAND(c, glBindTexture);
	c->cmd = GLSC_glBindTexture;
	c->target = target;
	c->texture = texture;
	send_data(GLSC_glBindTexture, (void *)c, sizeof(gl_glBindTexture_t));
}
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) {
	GL_SET_COMMAND(c, glTexImage2D);
	c->cmd = GLSC_glTexImage2D;
	c->target = target;
	c->level = level;
	c->internalformat = internalformat;
	c->width = width;
	c->height = height;
	c->border = border;
	c->format = format;
	c->type = type;
	c->pixels = pixels;
	send_data(GLSC_glTexImage2D, (void *)c, sizeof(gl_glTexImage2D_t));
}
void glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) {
	GL_SET_COMMAND(c, glTexStorage2D);
	c->cmd = GLSC_glTexStorage2D;
	c->target = target;
	c->levels = levels;
	c->internalformat = internalformat;
	c->width = width;
	c->height = height;
	send_data(GLSC_glTexStorage2D, (void *)c, sizeof(gl_glTexStorage2D_t));
}
void glTexParameteri(GLenum target, GLenum pname, GLint param) {
	GL_SET_COMMAND(c, glTexParameteri);
	c->cmd = GLSC_glTexParameteri;
	c->target = target;
	c->pname = pname;
	c->param = param;
	send_data(GLSC_glTexParameteri, (void *)c, sizeof(gl_glTexParameteri_t));
}
void glGenFramebuffers(GLsizei n, GLuint *framebuffers) {
	GL_SET_COMMAND(c, glGenFramebuffers);
	c->cmd = GLSC_glGenFramebuffers;
	c->n = n;
	GLuint *ret = (GLuint *)send_data(GLSC_glGenFramebuffers, (void *)c, sizeof(gl_glGenFramebuffers_t));
	memcpy((void *)framebuffers, (void *)ret, sizeof(GLuint));
}
void glBindFramebuffer(GLenum target, GLuint framebuffer) {
	GL_SET_COMMAND(c, glBindFramebuffer);
	c->cmd = GLSC_glBindFramebuffer;
	c->target = target;
	c->framebuffer = framebuffer;
	send_data(GLSC_glBindFramebuffer, (void *)c, sizeof(gl_glBindFramebuffer_t));
}

void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
	GL_SET_COMMAND(c, glFramebufferTexture2D);
	c->cmd = GLSC_glFramebufferTexture2D;
	c->target = target;
	c->attachment = attachment;
	c->textarget = textarget;
	c->texture = texture;
	c->level = level;
	send_data(GLSC_glFramebufferTexture2D, (void *)c, sizeof(gl_glFramebufferTexture2D_t));
}
GLenum glCheckFramebufferStatus(GLenum target) {
	GL_SET_COMMAND(c, glCheckFramebufferStatus);
	c->cmd = GLSC_glCheckFramebufferStatus;
	c->target = target;
	unsigned int *ret = (unsigned int *)send_data(GLSC_glCheckFramebufferStatus, (void *)c, sizeof(gl_glCheckFramebufferStatus_t));
	return (GLenum)*ret;
}
void glDisable(GLenum cap) {
	GL_SET_COMMAND(c, glDisable);
	c->cmd = GLSC_glDisable;
	c->cap = cap;
	send_data(GLSC_glDisable, (void *)c, sizeof(gl_glDisable_t));
}
void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels) {
	GL_SET_COMMAND(c, glTexImage3D);
	c->cmd = GLSC_glTexImage3D;
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
	send_data(GLSC_glTexImage3D, (void *)c, sizeof(gl_glTexImage3D_t));
}

// just defined functions
void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name) {
	GL_SET_COMMAND(c, glBindAttribLocation);
	c->cmd = GLSC_glBindAttribLocation;
	c->program = program;
	c->name = name;
	send_data(GLSC_glBindAttribLocation, (void *)c, sizeof(gl_glBindAttribLocation_t));
}
void glBindRenderbuffer(GLenum target, GLuint renderbuffer) {
	GL_SET_COMMAND(c, glBindRenderbuffer);
	c->cmd = GLSC_glBindRenderbuffer;
	c->target = target;
	c->renderbuffer = renderbuffer;
	send_data(GLSC_glBindRenderbuffer, (void *)c, sizeof(gl_glBindRenderbuffer_t));
}
void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
	std::cout << __func__ << std::endl;
}
void glBlendEquation(GLenum mode) {
	GL_SET_COMMAND(c, glBlendEquation);
	c->cmd = GLSC_glBlendEquation;
	c->mode = mode;
	send_data(GLSC_glBlendEquation, (void *)c, sizeof(gl_glBlendEquation_t));
}
void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha) {
	std::cout << __func__ << std::endl;
}
void glBlendFunc(GLenum sfactor, GLenum dfactor) {
	GL_SET_COMMAND(c, glBlendFunc);
	c->cmd = GLSC_glBlendFunc;
	c->sfactor = sfactor;
	c->dfactor = dfactor;
	send_data(GLSC_glBlendFunc, (void *)c, sizeof(gl_glBlendFunc_t));
}
void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {
	GL_SET_COMMAND(c, glBlendFuncSeparate);
	c->cmd = GLSC_glBlendFuncSeparate;
	c->sfactorRGB = sfactorRGB;
	c->dfactorRGB = dfactorRGB;
	c->sfactorAlpha = sfactorAlpha;
	c->dfactorAlpha = dfactorAlpha;
	send_data(GLSC_glBlendFuncSeparate, (void *)c, sizeof(gl_glBlendFuncSeparate_t));
}
void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data) {
	GL_SET_COMMAND(c, glBufferSubData);
	c->cmd = GLSC_glBufferData;
	c->target = target;
	c->offset = offset;
	c->size = size;
	c->data = data;
	send_data(GLSC_glBufferSubData, (void *)c, sizeof(gl_glBufferSubData_t));
}

void glClearDepthf(GLfloat d) {
	GL_SET_COMMAND(c, glClearDepthf);
	c->cmd = GLSC_glClearDepthf;
	c->d = d;
	send_data(GLSC_glClearDepthf, (void *)c, sizeof(gl_glClearDepthf_t));
}
void glClearStencil(GLint s) {
	std::cout << __func__ << std::endl;
}
void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
	GL_SET_COMMAND(c, glColorMask);
	c->cmd = GLSC_glColorMask;
	c->red = red;
	c->green = green;
	c->blue = blue;
	c->alpha = alpha;
	send_data(GLSC_glColorMask, (void *)c, sizeof(gl_glColorMask_t));
}
void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data) {
	GL_SET_COMMAND(c, glCompressedTexImage2D);
	c->cmd = GLSC_glTexImage2D;
	c->target = target;
	c->level = level;
	c->internalformat = internalformat;
	c->width = width;
	c->height = height;
	c->border = border;
	c->imageSize = imageSize;
	c->pixels = data;
	send_data(GLSC_glCompressedTexImage2D, (void *)c, sizeof(gl_glCompressedTexImage2D_t));
}
void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data) {
	std::cout << __func__ << std::endl;
}
void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
	std::cout << __func__ << std::endl;
}
void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
	std::cout << __func__ << std::endl;
}
void glCullFace(GLenum mode) {
	std::cout << __func__ << std::endl;
}
void glDeleteBuffers(GLsizei n, const GLuint *buffers) {
	std::cout << __func__ << std::endl;
}
void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers) {
	GL_SET_COMMAND(c, glDeleteFramebuffers);
	c->cmd = GLSC_glDeleteFramebuffers;
	c->n = n;
	c->framebuffers = framebuffers;
	send_data(GLSC_glDeleteFramebuffers, (void *)c, sizeof(gl_glDeleteFramebuffers_t));
}
void glDeleteProgram(GLuint program) {
	std::cout << __func__ << std::endl;
}
void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers) {
	GL_SET_COMMAND(c, glDeleteRenderbuffers);
	c->cmd = GLSC_glDeleteRenderbuffers;
	c->n = n;
	c->renderbuffers = renderbuffers;
	send_data(GLSC_glDeleteRenderbuffers, (void *)c, sizeof(gl_glDeleteRenderbuffers_t));
}
void glDeleteShader(GLuint shader) {
	std::cout << __func__ << std::endl;
}
void glDeleteTextures(GLsizei n, const GLuint *textures) {
	GL_SET_COMMAND(c, glDeleteTextures);
	c->cmd = GLSC_glDeleteTextures;
	c->n = n;
	c->textures = textures;
	send_data(GLSC_glDeleteTextures, (void *)c, sizeof(gl_glDeleteTextures_t));
}
void glDepthFunc(GLenum func) {
	GL_SET_COMMAND(c, glDepthFunc);
	c->cmd = GLSC_glDepthFunc;
	c->func = func;
	send_data(GLSC_glDepthFunc, (void *)c, sizeof(gl_glDepthFunc_t));
}
void glDepthMask(GLboolean flag) {
	GL_SET_COMMAND(c, glDepthMask);
	c->cmd = GLSC_glDepthMask;
	c->flag = flag;
	send_data(GLSC_glDepthMask, (void *)c, sizeof(gl_glDepthMask_t));
}
void glDepthRangef(GLfloat n, GLfloat f) {
	std::cout << __func__ << std::endl;
}
void glDetachShader(GLuint program, GLuint shader) {
	std::cout << __func__ << std::endl;
}
void glDisableVertexAttribArray(GLuint index) {
	GL_SET_COMMAND(c, glDisableVertexAttribArray);
	c->cmd = GLSC_glDisableVertexAttribArray;
	c->index = index;
	send_data(GLSC_glDisableVertexAttribArray, (void *)c, sizeof(gl_glDisableVertexAttribArray_t));
}
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices) {
	GL_SET_COMMAND(c, glDrawElements);
	c->cmd = GLSC_glDrawElements;
	c->mode = mode;
	c->count = count;
	c->type = type;
	c->indices = indices;
	send_data(GLSC_glDrawElements, (void *)c, sizeof(gl_glDrawElements_t));
}
void glEnable(GLenum cap) {
	GL_SET_COMMAND(c, glEnable);
	c->cmd = GLSC_glEnable;
	c->cap = cap;
	send_data(GLSC_glEnable, (void *)c, sizeof(gl_glEnable_t));
}
void glFinish(void) {
	std::cout << __func__ << std::endl;
}
void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
	GL_SET_COMMAND(c, glFramebufferRenderbuffer);
	c->cmd = GLSC_glFramebufferRenderbuffer;
	c->target = target;
	c->attachment = attachment;
	c->renderbuffertarget = renderbuffertarget;
	c->renderbuffer = renderbuffer;
	send_data(GLSC_glFramebufferRenderbuffer, (void *)c, sizeof(gl_glFramebufferRenderbuffer_t));
}
void glGenerateMipmap(GLenum target) {
	GL_SET_COMMAND(c, glGenerateMipmap);
	c->cmd = GLSC_glGenerateMipmap;
	c->target = target;
	send_data(GLSC_glGenerateMipmap, (void *)c, sizeof(gl_glGenerateMipmap_t));
}
void glFrontFace(GLenum mode) {
	GL_SET_COMMAND(c, glFrontFace);
	c->cmd = GLSC_glFrontFace;
	c->mode = mode;
	send_data(GLSC_glFrontFace, (void *)c, sizeof(gl_glFrontFace_t));
}
void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers) {
	GL_SET_COMMAND(c, glGenRenderbuffers);
	c->cmd = GLSC_glGenRenderbuffers;
	c->n = n;
	unsigned int *ret = (unsigned int *)send_data(GLSC_glGenRenderbuffers, (void *)c, sizeof(gl_glGenRenderbuffers_t));
	memcpy((void *)renderbuffers, (void *)ret, sizeof(unsigned int));
}
void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) {
	std::cout << __func__ << std::endl;
}
void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) {
	std::cout << __func__ << std::endl;
}
void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders) {
	std::cout << __func__ << std::endl;
}
void glGetBooleanv(GLenum pname, GLboolean *data) {
	std::cout << __func__ << std::endl;
}
void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params) {
	std::cout << __func__ << std::endl;
}
void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params) {
	std::cout << __func__ << std::endl;
}
void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
	std::cout << __func__ << std::endl;
}
void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params) {
	std::cout << __func__ << std::endl;
}
void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
	std::cout << __func__ << std::endl;
}
void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision) {
	std::cout << __func__ << std::endl;
}
void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) {
	std::cout << __func__ << std::endl;
}
const GLubyte *glGetString(GLenum name) {
	GL_SET_COMMAND(c, glGetString);
	c->cmd = GLSC_glGetString;
	c->name = name;
	std::string ret = send_data_get_string(GLSC_glGetString, (void *)c, sizeof(gl_glGetString_t));
	// const GLubyte *ret = reinterpret_cast<const GLubyte *>(send_data(GLSC_glGetStringi, (void *)c, sizeof(gl_glGetStringi_t)));
	std::cout << ret << std::endl;
	return reinterpret_cast<const GLubyte *>(ret.c_str());
}
void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params) {
	std::cout << __func__ << std::endl;
}
void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params) {
	std::cout << __func__ << std::endl;
}
void glGetUniformfv(GLuint program, GLint location, GLfloat *params) {
	std::cout << __func__ << std::endl;
}
void glGetUniformiv(GLuint program, GLint location, GLint *params) {
	std::cout << __func__ << std::endl;
}
GLint glGetUniformLocation(GLuint program, const GLchar *name) {
	GL_SET_COMMAND(c, glGetUniformLocation);
	c->cmd = GLSC_glGetUniformLocation;
	c->program = program;
	c->name = name;
	GLint *ret = (GLint *)send_data(GLSC_glGetUniformLocation, (void *)c, sizeof(gl_glGetUniformLocation_t));
	return (GLenum)*ret;
}
void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params) {
	std::cout << __func__ << std::endl;
}
void glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params) {
	std::cout << __func__ << std::endl;
}
void glGetVertexAttribPointerv(GLuint index, GLenum pname, void **pointer) {
	std::cout << __func__ << std::endl;
}
void glHint(GLenum target, GLenum mode) {
	std::cout << __func__ << std::endl;
}
GLboolean glIsBuffer(GLuint buffer) {
	std::cout << __func__ << std::endl;

	return 0;
}
GLboolean glIsEnabled(GLenum cap) {
	std::cout << __func__ << std::endl;

	return 0;
}
GLboolean glIsFramebuffer(GLuint framebuffer) {
	std::cout << __func__ << std::endl;

	return 0;
}
GLboolean glIsProgram(GLuint program) {
	std::cout << __func__ << std::endl;

	return 0;
}
GLboolean glIsRenderbuffer(GLuint renderbuffer) {
	std::cout << __func__ << std::endl;

	return 0;
}
GLboolean glIsShader(GLuint shader) {
	std::cout << __func__ << std::endl;

	return 0;
}
GLboolean glIsTexture(GLuint texture) {
	std::cout << __func__ << std::endl;

	return 0;
}
void glLineWidth(GLfloat width) {
	std::cout << __func__ << std::endl;
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
	}
	GL_SET_COMMAND(c, glPixelStorei);
	c->cmd = GLSC_glPixelStorei;
	c->pname = pname;
	c->param = param;
	send_data(GLSC_glPixelStorei, (void *)c, sizeof(gl_glPixelStorei_t));
}
void glPolygonOffset(GLfloat factor, GLfloat units) {
	std::cout << __func__ << std::endl;
}
void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels) {
	GL_SET_COMMAND(c, glReadPixels);
	c->cmd = GLSC_glReadPixels;
	c->x = x;
	c->y = y;
	c->width = width;
	c->height = height;
	c->format = format;
	c->type = type;
	void *data = send_data(GLSC_glReadPixels, (void *)c, sizeof(gl_glReadPixels_t));
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
}

void glReleaseShaderCompiler(void) {
	std::cout << __func__ << std::endl;
}
void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
	GL_SET_COMMAND(c, glRenderbufferStorage);
	c->cmd = GLSC_glRenderbufferStorage;
	c->target = target;
	c->internalformat = internalformat;
	c->width = width;
	c->height = height;
	send_data(GLSC_glRenderbufferStorage, (void *)c, sizeof(gl_glRenderbufferStorage_t));
}
void glSampleCoverage(GLfloat value, GLboolean invert) {
	std::cout << __func__ << std::endl;
}

void glShaderBinary(GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length) {
	std::cout << __func__ << std::endl;
}
void glStencilFunc(GLenum func, GLint ref, GLuint mask) {
	std::cout << __func__ << std::endl;
}
void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) {
	std::cout << __func__ << std::endl;
}
void glStencilMask(GLuint mask) {
	std::cout << __func__ << std::endl;
}
void glStencilMaskSeparate(GLenum face, GLuint mask) {
	std::cout << __func__ << std::endl;
}
void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
	std::cout << __func__ << std::endl;
}
void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
	std::cout << __func__ << std::endl;
}
void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {
	GL_SET_COMMAND(c, glTexParameterf);
	c->cmd = GLSC_glTexParameterf;
	c->target = target;
	c->pname = pname;
	c->param = param;
	send_data(GLSC_glTexParameterf, (void *)c, sizeof(gl_glTexParameterf_t));
}
void glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params) {
	std::cout << __func__ << std::endl;
}
void glTexParameteriv(GLenum target, GLenum pname, const GLint *params) {
	std::cout << __func__ << std::endl;
}
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) {
	GL_SET_COMMAND(c, glTexSubImage2D);
	c->cmd = GLSC_glTexSubImage2D;
	c->target = target;
	c->level = level;
	c->xoffset = xoffset;
	c->yoffset = yoffset;
	c->width = width;
	c->height = height;
	c->format = format;
	c->type = type;
	c->pixels = pixels;
	send_data(GLSC_glTexSubImage2D, (void *)c, sizeof(gl_glTexSubImage2D_t));
}
void glUniform1f(GLint location, GLfloat v0) {
	GL_SET_COMMAND(c, glUniform1f);
	c->cmd = GLSC_glUniform1f;
	c->location = location;
	c->v0 = v0;
	send_data(GLSC_glUniform1f, (void *)c, sizeof(gl_glUniform1f_t));
}
void glUniform1fv(GLint location, GLsizei count, const GLfloat *value) {
	std::cout << __func__ << std::endl;
}
void glUniform1i(GLint location, GLint v0) {
	GL_SET_COMMAND(c, glUniform1i);
	c->cmd = GLSC_glUniform1i;
	c->location = location;
	c->v0 = v0;
	// std::cout << location << "\t" << v0 << std::endl;
	send_data(GLSC_glUniform1i, (void *)c, sizeof(gl_glUniform1i_t));
}
void glUniform1iv(GLint location, GLsizei count, const GLint *value) {
	std::cout << __func__ << std::endl;
}
void glUniform2f(GLint location, GLfloat v0, GLfloat v1) {
	std::cout << __func__ << std::endl;
}
void glUniform2fv(GLint location, GLsizei count, const GLfloat *value) {
	GL_SET_COMMAND(c, glUniform2fv);
	c->cmd = GLSC_glUniform2fv;
	c->location = location;
	c->count = count;
	c->value = value;
	send_data(GLSC_glUniform2fv, (void *)c, sizeof(gl_glUniform2fv_t));
}
void glUniform2i(GLint location, GLint v0, GLint v1) {
	std::cout << __func__ << std::endl;
}
void glUniform2iv(GLint location, GLsizei count, const GLint *value) {
	std::cout << __func__ << std::endl;
}
void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
	std::cout << __func__ << std::endl;
}
void glUniform3fv(GLint location, GLsizei count, const GLfloat *value) {
	std::cout << __func__ << std::endl;
}
void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) {
	std::cout << __func__ << std::endl;
}
void glUniform3iv(GLint location, GLsizei count, const GLint *value) {
	std::cout << __func__ << std::endl;
}
void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
	std::cout << __func__ << std::endl;
}
void glUniform4fv(GLint location, GLsizei count, const GLfloat *value) {
	GL_SET_COMMAND(c, glUniform4fv);
	c->cmd = GLSC_glUniform4fv;
	c->location = location;
	c->count = count;
	c->value = value;
	send_data(GLSC_glUniform4fv, (void *)c, sizeof(gl_glUniform4fv_t));
}
void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
	std::cout << __func__ << std::endl;
}
void glUniform4iv(GLint location, GLsizei count, const GLint *value) {
	std::cout << __func__ << std::endl;
}
void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	std::cout << __func__ << std::endl;
}
void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	std::cout << __func__ << std::endl;
}
void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	GL_SET_COMMAND(c, glUniformMatrix4fv);
	c->cmd = GLSC_glUniformMatrix4fv;
	c->location = location;
	c->count = count;
	c->transpose = transpose;
	c->value = value;
	send_data(GLSC_glUniformMatrix4fv, (void *)c, sizeof(gl_glUniformMatrix4fv_t));
}
void glValidateProgram(GLuint program) {
	std::cout << __func__ << std::endl;
}
void glVertexAttrib1f(GLuint index, GLfloat x) {
	std::cout << __func__ << std::endl;
}
void glVertexAttrib1fv(GLuint index, const GLfloat *v) {
	std::cout << __func__ << std::endl;
}
void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y) {
	std::cout << __func__ << std::endl;
}
void glVertexAttrib2fv(GLuint index, const GLfloat *v) {
	std::cout << __func__ << std::endl;
}
void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z) {
	std::cout << __func__ << std::endl;
}
void glVertexAttrib3fv(GLuint index, const GLfloat *v) {
	std::cout << __func__ << std::endl;
}
void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	GL_SET_COMMAND(c, glVertexAttrib4f);
	c->cmd = GLSC_glVertexAttrib4f;
	c->index = index;
	c->x = x;
	c->y = y;
	c->z = z;
	c->w = w;
	send_data(GLSC_glVertexAttrib4f, (void *)c, sizeof(gl_glVertexAttrib4f_t));
}
void glVertexAttrib4fv(GLuint index, const GLfloat *v) {
	GL_SET_COMMAND(c, glVertexAttrib4fv);
	c->cmd = GLSC_glVertexAttrib4fv;
	c->index = index;
	c->v = v;
	send_data(GLSC_glVertexAttrib4fv, (void *)c, sizeof(gl_glVertexAttrib4fv_t));
}
//
void glReadBuffer(GLenum src) {
	GL_SET_COMMAND(c, glReadBuffer);
	c->cmd = GLSC_glReadBuffer;
	c->src = src;
	send_data(GLSC_glReadBuffer, (void *)c, sizeof(gl_glReadBuffer_t));
}
void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices) {
	std::cout << __func__ << std::endl;
}
void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels) {

	GL_SET_COMMAND(c, glTexSubImage3D);
	c->cmd = GLSC_glTexSubImage3D;
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
	send_data(GLSC_glTexSubImage3D, (void *)c, sizeof(gl_glTexSubImage3D_t));
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
}
void glDeleteQueries(GLsizei n, const GLuint *ids) {
	std::cout << __func__ << std::endl;
}
GLboolean glIsQuery(GLuint id) {
	return 0;
}
void glBeginQuery(GLenum target, GLuint id) {
	std::cout << __func__ << std::endl;
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
	return 0;
}
void glGetBufferPointerv(GLenum target, GLenum pname, void **params) {
	std::cout << __func__ << std::endl;
}
void glDrawBuffers(GLsizei n, const GLenum *bufs) {
	std::cout << __func__ << std::endl;
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
	c->cmd = GLSC_glBlitFramebuffer;
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
	send_data(GLSC_glBlitFramebuffer, (void *)c, sizeof(gl_glBlitFramebuffer_t));
}
void glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {
	GL_SET_COMMAND(c, glRenderbufferStorageMultisample);
	c->cmd = GLSC_glRenderbufferStorageMultisample;
	c->target = target;
	c->samples = samples;
	c->internalformat = internalformat;
	c->width = width;
	c->height = height;
	send_data(GLSC_glRenderbufferStorageMultisample, (void *)c, sizeof(gl_glRenderbufferStorageMultisample_t));
}
void glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
	GL_SET_COMMAND(c, glFramebufferTextureLayer);
	c->cmd = GLSC_glFramebufferTextureLayer;
	c->target = target;
	c->attachment = attachment;
	c->texture = texture;
	c->level = level;
	c->layer = layer;
	send_data(GLSC_glFramebufferTextureLayer, (void *)c, sizeof(gl_glFramebufferTextureLayer_t));
}
void *glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) {
	std::cout << __func__ << std::endl;
}
void glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length) {
	std::cout << __func__ << std::endl;
}
void glDeleteVertexArrays(GLsizei n, const GLuint *arrays) {
	std::cout << __func__ << std::endl;
}
GLboolean glIsVertexArray(GLuint array) {
	return 0;
}
void glGetIntegeri_v(GLenum target, GLuint index, GLint *data) {
	std::cout << __func__ << std::endl;
}
void glBeginTransformFeedback(GLenum primitiveMode) {
	std::cout << __func__ << std::endl;
}
void glEndTransformFeedback(void) {
	std::cout << __func__ << std::endl;
}
void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
	std::cout << __func__ << std::endl;
}
void glBindBufferBase(GLenum target, GLuint index, GLuint buffer) {
	GL_SET_COMMAND(c, glBindBufferBase);
	c->cmd = GLSC_glBindBufferBase;
	c->target = target;
	c->index = index;
	c->buffer = buffer;
	send_data(GLSC_glBindBufferBase, (void *)c, sizeof(gl_glBindBufferBase_t));
}
void glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar *const *varyings, GLenum bufferMode) {
	std::cout << __func__ << std::endl;
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
	std::cout << __func__ << std::endl;

	return 0;
}
void glUniform1ui(GLint location, GLuint v0) {
	std::cout << __func__ << std::endl;
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
	c->cmd = GLSC_glClearBufferfv;
	c->buffer = buffer;
	c->drawbuffer = drawbuffer;
	c->value = value;
	send_data(GLSC_glClearBufferfv, (void *)c, sizeof(gl_glClearBufferfv_t));
}
void glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
	std::cout << __func__ << std::endl;
}
const GLubyte *glGetStringi(GLenum name, GLuint index) {
	GL_SET_COMMAND(c, glGetStringi);
	c->cmd = GLSC_glGetStringi;
	c->name = name;
	c->index = index;
	std::string ret = send_data_get_string(GLSC_glGetStringi, (void *)c, sizeof(gl_glGetStringi_t));
	// const GLubyte *ret = reinterpret_cast<const GLubyte *>(send_data(GLSC_glGetStringi, (void *)c, sizeof(gl_glGetStringi_t)));
	// std::cout << ret.c_str() << std::endl;
	return reinterpret_cast<const GLubyte *>(ret.c_str());
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
	c->cmd = GLSC_glGetUniformBlockIndex;
	c->program = program;
	c->name = uniformBlockName;
	GLint *ret = (GLint *)send_data(GLSC_glGetUniformBlockIndex, (void *)c, sizeof(gl_glGetUniformBlockIndex_t));
	return (GLenum)*ret;
}
void glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params) {
	std::cout << __func__ << std::endl;
}
void glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName) {
	std::cout << __func__ << std::endl;
}
void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) {
	GL_SET_COMMAND(c, glUniformBlockBinding);
	c->cmd = GLSC_glUniformBlockBinding;
	c->program = program;
	c->uniformBlockIndex = uniformBlockIndex;
	c->uniformBlockBinding = uniformBlockBinding;
	send_data(GLSC_glUniformBlockBinding, (void *)c, sizeof(gl_glUniformBlockBinding_t));
}
void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) {
	std::cout << __func__ << std::endl;
}
void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount) {
	std::cout << __func__ << std::endl;
}
GLsync glFenceSync(GLenum condition, GLbitfield flags) {
	return 0;
}
GLboolean glIsSync(GLsync sync) {
	return 0;
}
void glDeleteSync(GLsync sync) {
	std::cout << __func__ << std::endl;
}
GLenum glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) {
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
	std::cout << __func__ << std::endl;
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