#include "glremote/glremote.h"
#include <stdint.h>
// add all godot gl command
enum GL_Server_Command {
	GLSC_BREAK,
	GLSC_bufferSwap,
	GLSC_glClear,
	GLSC_glBegin,
	GLSC_glColor3f,
	GLSC_glVertex3f,
	GLSC_glEnd,
	GLSC_glFlush,
	GLSC_glCreateShader,
	GLSC_glShaderSource,
	GLSC_glCompileShader,
	GLSC_glGetShaderiv,
	GLSC_glCreateProgram,
	GLSC_glAttachShader,
	GLSC_glLinkProgram,
	GLSC_glGetProgramiv,
	GLSC_glGenBuffers,
	GLSC_glBindBuffer,
	GLSC_glBufferData,
	GLSC_glGenVertexArrays,
	GLSC_glBindVertexArray,
	GLSC_glGetAttribLocation,
	GLSC_glVertexAttribPointer,
	GLSC_glEnableVertexAttribArray,
	GLSC_glUseProgram,
	GLSC_glClearColor,
	GLSC_glDrawArrays,
	GLSC_glViewport,
	GLSC_glScissor,
	GLSC_glGetIntegerv,
	GLSC_glGetFloatv,
	GLSC_glGenTextures,
	GLSC_glActiveTexture,
	GLSC_glBindTexture,
	GLSC_glTexImage2D,
	GLSC_glGetError,
	GLSC_glTexStorage2D,
	GLSC_glTexParameteri,
	GLSC_glGenFramebuffers,
	GLSC_glBindFramebuffer,
	GLSC_glFramebufferTexture2D,
	GLSC_glCheckFramebufferStatus,
	GLSC_glDisable,
	GLSC_glEnable,
	GLSC_glTexImage3D,
	GLSC_glGenerateMipmap,
	GLSC_glFrontFace,
	GLSC_glDepthMask,
	GLSC_glBlendEquation,
	GLSC_glBlendFunc,
	GLSC_glVertexAttrib4f,
	GLSC_glUniform1i,
	GLSC_glUniformBlockBinding,
	GLSC_glPixelStorei,
	GLSC_glTexParameterf,
	GLSC_glUniform4fv,
	GLSC_glUniformMatrix4fv,
	GLSC_glUniform1ui,
	GLSC_glUniform2fv,
	GLSC_glBindBufferBase,
	GLSC_glColorMask,
	GLSC_glDepthFunc,
	GLSC_glGenRenderbuffers,
	GLSC_glBindRenderbuffer,
	GLSC_glRenderbufferStorage,
	GLSC_glFramebufferRenderbuffer,
	GLSC_glTexSubImage3D,
	GLSC_glDeleteTextures,
	GLSC_glDeleteFramebuffers,
	GLSC_glClearDepthf,
	GLSC_glClearBufferfv,
	GLSC_glGetStringi,
	GLSC_glGetString,
	GLSC_glTexSubImage2D,
	GLSC_glReadPixels,
	GLSC_glBlendFuncSeparate,
	GLSC_glDisableVertexAttribArray,
	GLSC_glVertexAttrib4fv,
	GLSC_glDrawElements,
	GLSC_glReadBuffer,
	GLSC_glBlitFramebuffer,
	GLSC_glDeleteRenderbuffers,
	GLSC_glUniform1f,
	GLSC_glFramebufferTextureLayer,
	GLSC_glRenderbufferStorageMultisample,
	GLSC_glGetUniformLocation,
	GLSC_glGetUniformBlockIndex,
	GLSC_glBufferSubData,
	GLSC_glBindAttribLocation,
	GLSC_glCompressedTexImage2D,
	GLSC_glBeginTransformFeedback,
	GLSC_glEndTransformFeedback,
	GLSC_glVertexAttribDivisor,
	GLSC_glDrawArraysInstanced,
	GLSC_glTransformFeedbackVaryings,
	GLSC_glCullFace,
	GLSC_glDrawBuffers,
	GLSC_glDeleteVertexArrays,
	GLSC_glDeleteBuffers,
};

typedef struct {
	unsigned int cmd;
	GLenum cap;
} gl_glDisable_t, gl_glEnable_t;

typedef struct {
	unsigned int cmd;
	GLenum func;
} gl_glDepthFunc_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLint levels;
	GLint internalformat;
	GLsizei width;
	GLsizei height;
} gl_glTexStorage2D_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLenum pname;
	GLint param;
} gl_glTexParameteri_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
} gl_glRenderbufferStorage_t;

typedef struct {
	unsigned int cmd;
	GLint srcX0;
	GLint srcY0;
	GLint srcX1;
	GLint srcY1;
	GLint dstX0;
	GLint dstY0;
	GLint dstX1;
	GLint dstY1;
	GLbitfield mask;
	GLenum filter;
} gl_glBlitFramebuffer_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLenum attachment;
	GLuint texture;
	GLint level;
	GLint layer;
} gl_glFramebufferTextureLayer_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLsizei samples;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
} gl_glRenderbufferStorageMultisample_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLint level;
	GLint internalformat;
	GLsizei width;
	GLsizei height;
	GLint border;
	GLenum format;
	GLenum type;
	const void *pixels;
} gl_glTexImage2D_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLsizei width;
	GLsizei height;
	GLenum format;
	GLenum type;
	const void *pixels;
} gl_glTexSubImage2D_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLint level;
	GLint internalformat;
	GLsizei width;
	GLsizei height;
	GLint border;
	GLsizei imageSize;
	const void *pixels;
} gl_glCompressedTexImage2D_t;

typedef struct {
	unsigned int cmd;
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;
	GLenum format;
	GLenum type;
	void *pixels;
} gl_glReadPixels_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLint level;
	GLint internalformat;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLint border;
	GLenum format;
	GLenum type;
	const void *pixels;
} gl_glTexImage3D_t;

typedef struct {
	unsigned int cmd;
	GLsizei n;
	const GLuint *textures;
} gl_glDeleteTextures_t;

typedef struct {
	unsigned int cmd;
	GLsizei n;
	const GLuint *framebuffers;
} gl_glDeleteFramebuffers_t;

typedef struct {
	unsigned int cmd;
	GLsizei n;
	const GLuint *renderbuffers;
} gl_glDeleteRenderbuffers_t;

typedef struct {
	unsigned int cmd;
	GLenum buffer;
	GLint drawbuffer;
	const GLfloat *value;
} gl_glClearBufferfv_t;

typedef struct {
	unsigned int cmd;
	GLfloat d;
} gl_glClearDepthf_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLenum format;
	GLenum type;
	const void *pixels;
} gl_glTexSubImage3D_t;

typedef struct {
	unsigned int cmd;
	GLenum pname;
	GLint *data;
} gl_glGetIntegerv_t;

typedef struct {
	unsigned int cmd;
	GLenum pname;
	GLfloat *data;
} gl_glGetFloatv_t;

typedef struct {
	unsigned int cmd;
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;
} gl_glScissor_t;

typedef struct {
	unsigned int cmd;
	GLuint program;
	GLuint uniformBlockIndex;
	GLuint uniformBlockBinding;
} gl_glUniformBlockBinding_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLenum attachment;
	GLenum renderbuffertarget;
	GLuint renderbuffer;
} gl_glFramebufferRenderbuffer_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLuint index;
	GLuint buffer;
} gl_glBindBufferBase_t;

typedef struct {
	unsigned int cmd;
	GLboolean red;
	GLboolean green;
	GLboolean blue;
	GLboolean alpha;
} gl_glColorMask_t;

typedef struct {
	unsigned int cmd;
	GLenum pname;
	GLint param;
} gl_glPixelStorei_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLenum pname;
	GLfloat param;
} gl_glTexParameterf_t;

typedef struct {
	unsigned int cmd;
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;
} gl_glViewport_t;

typedef struct {
	unsigned int cmd;
	int size;
} gl_command_t;

typedef struct {
	unsigned int cmd;
	GLenum name;
	GLuint index;
} gl_glGetStringi_t;

typedef struct {
	unsigned int cmd;
	GLenum name;
} gl_glGetString_t;

typedef struct {
	unsigned int cmd;
	GLuint index;
} gl_glDisableVertexAttribArray_t;

typedef struct {
	unsigned int cmd;
} gl_glFlush_t, gl_glCreateProgram_t, gl_glEnd_t, gl_glBreak_t, gl_glGetError_t, gl_glEndTransformFeedback_t;

typedef struct {
	unsigned int cmd;
	GLbitfield mask;
} gl_glClear_t;

typedef struct {
	unsigned int cmd;
	GLint location;
	GLint v0;
} gl_glUniform1i_t;

typedef struct {
	unsigned int cmd;
	GLint location;
	GLfloat v0;
} gl_glUniform1f_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
} gl_glGenerateMipmap_t;

typedef struct {
	unsigned int cmd;
	GLenum mode;
} gl_glBegin_t, gl_glFrontFace_t, gl_glBlendEquation_t;

typedef struct {
	unsigned int cmd;
	GLenum sfactor;
	GLenum dfactor;
} gl_glBlendFunc_t;

typedef struct {
	unsigned int cmd;
	GLuint program;
	const GLchar *name;
} gl_glGetUniformLocation_t, gl_glGetUniformBlockIndex_t;

typedef struct {
	unsigned int cmd;
	GLenum sfactorRGB;
	GLenum dfactorRGB;
	GLenum sfactorAlpha;
	GLenum dfactorAlpha;
} gl_glBlendFuncSeparate_t;

typedef struct {
	unsigned int cmd;
	GLfloat red;
	GLfloat green;
	GLfloat blue;
} gl_glColor3f_t;

typedef struct {
	unsigned int cmd;
	GLfloat x;
	GLfloat y;
	GLfloat z;
} gl_glVertex3f_t;

typedef struct {
	unsigned int cmd;
	GLuint index;
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;
} gl_glVertexAttrib4f_t;

typedef struct {
	unsigned int cmd;
	GLuint index;
	const GLfloat *v;
} gl_glVertexAttrib4fv_t;

typedef struct {
	unsigned int cmd;
	GLint location;
	GLuint v0;
} gl_glUniform1ui_t;

typedef struct {
	unsigned int cmd;
	GLenum mode;
	GLsizei count;
	GLenum type;
	int64_t indices;
} gl_glDrawElements_t;

typedef struct {
	unsigned int cmd;
	GLenum type;
} gl_glCreateShader_t;

typedef struct {
	unsigned int cmd;
	GLenum src;
} gl_glReadBuffer_t;

typedef struct {
	unsigned int cmd;
	GLuint shader;
	GLuint count;
	const GLchar *const *string;
	const GLint *length; // usally null
} gl_glShaderSource_t;

typedef struct {
	unsigned int cmd;
	GLuint shader;
} gl_glCompileShader_t;

typedef struct {
	unsigned int cmd;
	GLboolean flag;
} gl_glDepthMask_t;

typedef struct {
	unsigned int cmd;
	GLuint shader;
	GLenum pname;
	GLint *result;
} gl_glGetShaderiv_t;

typedef struct {
	unsigned int cmd;
	GLuint program;
	GLuint shader;
} gl_glAttachShader_t;

typedef struct {
	unsigned int cmd;
	GLuint program;
} gl_glLinkProgram_t, gl_glUseProgram_t;

typedef struct {
	unsigned int cmd;
	GLuint program;
	GLenum pname;
	GLint *result;
} gl_glGetProgramiv_t;

typedef struct {
	unsigned int cmd;
	GLsizei n;
	unsigned int last_index;
} gl_glGenTextures_t, gl_glGenBuffers_t, gl_glGenFramebuffers_t, gl_glGenRenderbuffers_t, gl_glGenVertexArrays_t, gl_glGenQueries_t, gl_glGenSamplers_t, gl_glGenTransformFeedbacks_t;

typedef struct {
	unsigned int cmd;
	GLenum texture;
} gl_glActiveTexture_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLuint texture;
} gl_glBindTexture_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLuint framebuffer;
} gl_glBindFramebuffer_t;

typedef struct {
	unsigned int cmd;
	GLint location;
	GLsizei count;
	const GLfloat *value;

} gl_glUniform4fv_t, gl_glUniform2fv_t;
typedef struct {
	unsigned int cmd;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

} gl_glUniformMatrix4fv_t;
typedef struct {
	unsigned int cmd;
	GLenum target;
	GLenum attachment;
	GLenum textarget;
	GLuint texture;
	GLint level;
} gl_glFramebufferTexture2D_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
} gl_glCheckFramebufferStatus_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLuint id;
} gl_glBindBuffer_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLuint renderbuffer;
} gl_glBindRenderbuffer_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLsizeiptr size;
	const void *data;
	GLenum usage;
} gl_glBufferData_t;

typedef struct {
	unsigned int cmd;
	GLenum target;
	GLintptr offset;
	GLsizeiptr size;
	const void *data;
} gl_glBufferSubData_t;

typedef struct {
	unsigned int cmd;
	GLuint array;
} gl_glBindVertexArray_t;

typedef struct {
	unsigned int cmd;
	GLuint programObj;
	const GLchar *name;
} gl_glGetAttribLocation_t;

typedef struct {
	unsigned int cmd;
	GLuint index;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLsizei stride;
	int64_t pointer;
} gl_glVertexAttribPointer_t;

typedef struct {
	unsigned int cmd;
	GLuint index;
} gl_glEnableVertexAttribArray_t;

typedef struct {
	unsigned int cmd;
	GLfloat red;
	GLfloat green;
	GLfloat blue;
	GLfloat alpha;
} gl_glClearColor_t;

typedef struct {
	unsigned int cmd;
	GLenum mode;
	GLint first;
	GLsizei count;
} gl_glDrawArrays_t;

typedef struct {
	unsigned int cmd;
	GLuint program;
	GLuint index;
	const GLchar *name;
} gl_glBindAttribLocation_t;

typedef struct {
	unsigned int cmd;
	GLenum primitiveMode;
} gl_glBeginTransformFeedback_t;

typedef struct {
	unsigned int cmd;
	GLuint index;
	GLuint divisor;
} gl_glVertexAttribDivisor_t;

typedef struct {
	unsigned int cmd;
	GLenum mode;
	GLint first;
	GLsizei count;
	GLsizei instancecount;
} gl_glDrawArraysInstanced_t;

typedef struct {
	unsigned int cmd;
	GLuint program;
	GLsizei count;
	const GLchar *const *varyings;
	GLenum bufferMode;
} gl_glTransformFeedbackVaryings_t;

typedef struct {
	unsigned int cmd;
	GLenum mode;
} gl_glCullFace_t;

typedef struct {
	unsigned int cmd;
	GLsizei n;
	const GLenum *bufs;
} gl_glDrawBuffers_t;

typedef struct {
	unsigned int cmd;
	GLsizei n;
	const GLuint *arrays;
} gl_glDeleteVertexArrays_t;

typedef struct {
	unsigned int cmd;
	GLsizei n;
	const GLuint *buffers;
} gl_glDeleteBuffers_t;