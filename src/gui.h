
union vec2
{
    struct
    {
        f32 x, y;
    };
    f32 e[2];
};

inline vec2 v2(f32 x, f32 y)
{
    vec2 result = {x, y};
    return result;
}

union vec4
{
    struct
    {
        f32 x, y, z, w;
    };
    struct
    {
        f32 r, g, b, a;
    };
    f32 e[4];
};

enum
{
    attrib_pos,
    attrib_color,

    attrib_count,
};

enum
{
    uniform_proj_mat,

    uniform_count,
};

struct Vertex
{
    f32 pos[2];
    u8 color[4];
};

struct UIState
{
    PlatformInput *input;

    MemoryStack memory;

    GLuint vbo;
    GLuint vao;
    GLuint ebo;

    GLuint program;

    GLuint attribs[attrib_count];
    GLuint uniforms[uniform_count];

    Vertex *vertices;
    GLuint *elements;

    u32 num_vertices;
    u32 num_elements;
};
