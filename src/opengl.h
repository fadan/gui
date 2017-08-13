#define GL_TRUE     1
#define GL_FALSE    0

#define GL_FLOAT            0x1406
#define GL_UNSIGNED_BYTE    0x1401
#define GL_UNSIGNED_SHORT   0x1403
#define GL_UNSIGNED_INT     0x1405

#define GL_ARRAY_BUFFER                   0x8892
#define GL_BLEND                          0x0BE2
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_COMPILE_STATUS                 0x8B81
#define GL_CULL_FACE                      0x0B44
#define GL_DEPTH_TEST                     0x0B71
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_FUNC_ADD                       0x8006
#define GL_LINEAR                         0x2601
#define GL_LINK_STATUS                    0x8B82
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_RGBA                           0x1908
#define GL_SCISSOR_TEST                   0x0C11
#define GL_SRC_ALPHA                      0x0302
#define GL_STREAM_DRAW                    0x88E0
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE_2D                     0x0DE1
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TRIANGLES                      0x0004
#define GL_UNPACK_ROW_LENGTH              0x0CF2
#define GL_VERTEX_SHADER                  0x8B31
#define GL_WRITE_ONLY                     0x88B9

#define GL_LINE_STRIP                     0x0003
#define GL_QUADS                          0x0007
#define GL_ALPHA                          0x1906
#define GL_VERTEX_ARRAY                   0x8074
#define GL_PROJECTION                     0x1701
#define GL_MODELVIEW                      0x1700

#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
#define GL_DEBUG_SEVERITY_HIGH            0x9146

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
typedef intptr GLsizeiptr;
typedef char GLchar;

typedef void (__stdcall * PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (__stdcall * PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (__stdcall * PFNGLBINDTEXTUREPROC) (GLenum target, GLuint texture);
typedef void (__stdcall * PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (__stdcall * PFNGLBLENDEQUATIONPROC) (GLenum mode);
typedef void (__stdcall * PFNGLBLENDFUNCPROC) (GLenum sfactor, GLenum dfactor);
typedef void (__stdcall * PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (__stdcall * PFNGLCLEARCOLORPROC) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (__stdcall * PFNGLCLEARPROC) (GLbitfield mask);
typedef void (__stdcall * PFNGLDISABLEPROC) (GLenum cap);
typedef void (__stdcall * PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (__stdcall * PFNGLDRAWELEMENTSPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices);
typedef void (__stdcall * PFNGLENABLEPROC) (GLenum cap);
typedef void (__stdcall * PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (__stdcall * PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (__stdcall * PFNGLGENTEXTURESPROC) (GLsizei n, GLuint *textures);
typedef void (__stdcall * PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
typedef void *(__stdcall * PFNGLMAPBUFFERPROC) (GLenum target, GLenum access);
typedef void (__stdcall * PFNGLSCISSORPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (__stdcall * PFNGLTEXIMAGE2DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void (__stdcall * PFNGLTEXPARAMETERIPROC) (GLenum target, GLenum pname, GLint param);
typedef GLboolean (__stdcall * PFNGLUNMAPBUFFERPROC) (GLenum target);
typedef void (__stdcall * PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void (__stdcall * PFNGLVIEWPORTPROC) (GLint x, GLint y, GLsizei width, GLsizei height);

typedef void (__stdcall * PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (__stdcall * PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar *name);
typedef void (__stdcall * PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef GLuint (__stdcall * PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint (__stdcall * PFNGLCREATESHADERPROC) (GLenum type);
typedef void (__stdcall * PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (__stdcall * PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef GLint (__stdcall * PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar *name);
typedef void (__stdcall * PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (__stdcall * PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void (__stdcall * PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void (__stdcall * PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef GLint (__stdcall * PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
typedef void (__stdcall * PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (__stdcall * PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (__stdcall * PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (__stdcall * PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void (__stdcall * PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
typedef void (__stdcall * PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (__stdcall * PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);

// 

typedef void (__stdcall * PFNGLVERTEX2FPROC)(GLfloat x, GLfloat y);
typedef void (__stdcall * PFNGLCOLOR4FPROC)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (__stdcall * PFNGLCOLOR3UBPROC)(GLubyte red, GLubyte green, GLubyte blue);
typedef void (__stdcall * PFNGLBEGINPROC)(GLenum mode);
typedef void (__stdcall * PFNGLENDPROC)();
typedef GLboolean (__stdcall * PFNGLISENABLEDPROC)(GLenum cap);
typedef void (__stdcall * PFNGLDISABLEPROC)(GLenum cap);
typedef void (__stdcall * PFNGLBEGINPROC)(GLenum mode);
typedef void (__stdcall * PFNGLCOLOR3FVPROC)(GLfloat *v);
typedef void (__stdcall * PFNGLTEXCOORD2FPROC)(GLfloat s, GLfloat t);
typedef void (__stdcall * PFNGLCOLOR3FPROC)(GLfloat red, GLfloat green, GLfloat blue);
typedef void (__stdcall * PFNGLENABLECLIENTSTATEPROC)(GLenum array);
typedef void (__stdcall * PFNGLDISABLECLIENTSTATEPROC)(GLenum array);
typedef void (__stdcall * PFNGLVERTEXPOINTERPROC)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (__stdcall * PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);
typedef void (__stdcall * PFNGLMATRIXMODEPROC)(GLenum mode);
typedef void (__stdcall * PFNGLLOADIDENTITYPROC)();
typedef void (__stdcall * PFNGLLOADMATRIXFPROC)(GLfloat *m);

typedef void (__stdcall * GLDEBUGPROCARB)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar *message, GLvoid *user_param);
typedef void (__stdcall * PFNGLDEBUGMESSAGECALLBACKPROC)(GLDEBUGPROCARB callback, GLvoid *user_param);

// NOTE(dan): wglGetProcAddress will not work with functions that are directly exported by OpenGL32.dll
#define GL_FUNCTION_LIST_1_1 \
    GLCORE(BINDTEXTURE,                 BindTexture) \
    GLCORE(BLENDFUNC,                   BlendFunc) \
    GLCORE(CLEAR,                       Clear) \
    GLCORE(CLEARCOLOR,                  ClearColor) \
    GLCORE(DISABLE,                     Disable) \
    GLCORE(DRAWELEMENTS,                DrawElements) \
    GLCORE(ENABLE,                      Enable) \
    GLCORE(GENTEXTURES,                 GenTextures) \
    GLCORE(SCISSOR,                     Scissor) \
    GLCORE(TEXIMAGE2D,                  TexImage2D) \
    GLCORE(TEXPARAMETERI,               TexParameteri) \
    GLCORE(VIEWPORT,                    Viewport) \
    \
    GLCORE(VERTEX2F, Vertex2f) \
    GLCORE(COLOR4F, Color4f) \
    GLCORE(COLOR3UB, Color3ub) \
    GLCORE(BEGIN, Begin) \
    GLCORE(END, End) \
    GLCORE(ISENABLED, IsEnabled) \
    GLCORE(COLOR3FV, Color3fv) \
    GLCORE(TEXCOORD2F, TexCoord2f) \
    GLCORE(COLOR3F, Color3f) \
    GLCORE(ENABLECLIENTSTATE, EnableClientState) \
    GLCORE(DISABLECLIENTSTATE, DisableClientState) \
    GLCORE(VERTEXPOINTER, VertexPointer) \
    GLCORE(DRAWARRAYS, DrawArrays) \
    GLCORE(MATRIXMODE, MatrixMode) \
    GLCORE(LOADIDENTITY, LoadIdentity) \
    GLCORE(LOADMATRIXF, LoadMatrixf)

#define GL_FUNCITON_LIST \
    GLCORE(ACTIVETEXTURE,               ActiveTexture) \
    GLCORE(ATTACHSHADER,                AttachShader) \
    GLCORE(BINDBUFFER,                  BindBuffer) \
    GLCORE(BINDATTRIBLOCATION,          BindAttribLocation) \
    GLCORE(BINDVERTEXARRAY,             BindVertexArray) \
    GLCORE(BLENDEQUATION,               BlendEquation) \
    GLCORE(BUFFERDATA,                  BufferData) \
    GLCORE(COMPILESHADER,               CompileShader) \
    GLCORE(CREATESHADER,                CreateShader) \
    GLCORE(DEBUGMESSAGECALLBACK,        DebugMessageCallback) \
    GLCORE(DELETESHADER,                DeleteShader) \
    GLCORE(DETACHSHADER,                DetachShader) \
    GLCORE(DISABLEVERTEXATTRIBARRAY,    DisableVertexAttribArray) \
    GLCORE(CREATEPROGRAM,               CreateProgram) \
    GLCORE(ENABLEVERTEXATTRIBARRAY,     EnableVertexAttribArray) \
    GLCORE(GENBUFFERS,                  GenBuffers) \
    GLCORE(GENVERTEXARRAYS,             GenVertexArrays) \
    GLCORE(GETATTRIBLOCATION,           GetAttribLocation) \
    GLCORE(GETPROGRAMINFOLOG,           GetProgramInfoLog) \
    GLCORE(GETPROGRAMIV,                GetProgramiv) \
    GLCORE(GETSHADERINFOLOG,            GetShaderInfoLog) \
    GLCORE(GETSHADERIV,                 GetShaderiv) \
    GLCORE(GETUNIFORMLOCATION,          GetUniformLocation) \
    GLCORE(LINKPROGRAM,                 LinkProgram) \
    GLCORE(MAPBUFFER,                   MapBuffer) \
    GLCORE(SHADERSOURCE,                ShaderSource) \
    GLCORE(UNIFORM1I,                   Uniform1i) \
    GLCORE(UNIFORM1F,                   Uniform1f) \
    GLCORE(UNIFORM4FV,                  Uniform4fv) \
    GLCORE(UNIFORMMATRIX4FV,            UniformMatrix4fv) \
    GLCORE(UNMAPBUFFER,                 UnmapBuffer) \
    GLCORE(USEPROGRAM,                  UseProgram) \
    GLCORE(VERTEXATTRIBPOINTER,         VertexAttribPointer) \

#define GLARB(a, b) GLCORE(a##ARB, b##ARB)
#define GLEXT(a, b) GLCORE(a##EXT, b##EXT)

struct OpenGL
{
    #define GLCORE(a, b) PFNGL##a##PROC b;
    GL_FUNCTION_LIST_1_1
    GL_FUNCITON_LIST
    #undef GLCORE
};

extern OpenGL gl;

static GLuint opengl_compile_shader(char const *source, GLenum type, char *error, u32 error_length)
{
    GLuint handle = gl.CreateShader(type);
    gl.ShaderSource(handle, 1, &source, 0);
    gl.CompileShader(handle);

    GLint status;
    gl.GetShaderiv(handle, GL_COMPILE_STATUS, &status);

    if (!status)
    {
        gl.GetShaderInfoLog(handle, error_length, 0, error);
        assert_always();
    }

    return handle;
}

static GLuint opengl_create_program(char *vertex_source, char *fragment_source, char *error, u32 error_length)
{
    GLuint handle = gl.CreateProgram();

    if (vertex_source)
    {
        GLuint vertex_shader = opengl_compile_shader(vertex_source, GL_VERTEX_SHADER, error, error_length);
        gl.AttachShader(handle, vertex_shader);
        gl.DeleteShader(vertex_shader);
    }
    if (fragment_source)
    {
        GLuint fragment_shader = opengl_compile_shader(fragment_source, GL_FRAGMENT_SHADER, error, error_length);
        gl.AttachShader(handle, fragment_shader);
        gl.DeleteShader(fragment_shader);
    }

    gl.LinkProgram(handle);

    GLint status;
    gl.GetProgramiv(handle, GL_LINK_STATUS, &status);

    if (!status)
    {
        gl.GetProgramInfoLog(handle, error_length, 0, error);
        assert_always();
    }

    return handle;
}
