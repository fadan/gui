
inline f32 inv_sqrt32(f32 a)
{
    union { f32 f; u32 u; } memory = {a};
    memory.u = 0x5F375A86 - (memory.u >> 1);
    memory.f *= 1.5f - (0.5f * a * memory.f * memory.f);
    return memory.f;
}

#define sqrt32(a) ( (a) * inv_sqrt32(a) )

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

#define vec2_add(a, b)      v2( (a).x + (b).x,   (a).y + (b).y )
#define vec2_sub(a, b)      v2( (a).x - (b).x,   (a).y - (b).y )

#define vec2_mul(s, a)      v2( (s) * (a).x,  (s) * (a).y )

#define vec2_length2(a)     ( (a).x * (a).x + (a).y * (a).y )
#define vec2_length(a)      sqrt32(vec2_length2(a))

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

inline vec4 v4(f32 x, f32 y, f32 z, f32 w)
{
    vec4 result = {x, y, z, w};
    return result;
}

#define TAU32 6.28318530717958647692f

inline f32 sin32(f32 x)
{
    // NOTE(dan): constants generated with Remez's minimax approxs on [0,TAU] range.
    static f32 c0 = +1.91059300966915117e-31f;
    static f32 c1 = +1.00086760103908896f;
    static f32 c2 = -1.21276126894734565e-2f;
    static f32 c3 = -1.38078780785773762e-1f;
    static f32 c4 = -2.67353392911981221e-2f;
    static f32 c5 = +2.08026600266304389e-2f;
    static f32 c6 = -3.03996055049204407e-3f;
    static f32 c7 = +1.38235642404333740e-4f;

    f32 sine = c0 + x * (c1 + x * (c2 + x * (c3 + x * (c4 + x * (c5 + x * (c6 + x * c7))))));
    return sine;
}

inline f32 cos32(f32 x)
{
    // NOTE(dan): constants generated with Remez's minimax approxs on [0,TAU] range.
    static f32 c0 = +1.00238601909309722f;
    static f32 c1 = -3.81919947353040024e-2f;
    static f32 c2 = -3.94382342128062756e-1f;
    static f32 c3 = -1.18134036025221444e-1f;
    static f32 c4 = +1.07123798512170878e-1f;
    static f32 c5 = -1.86637164165180873e-2f;
    static f32 c6 = +9.90140908664079833e-4f;
    static f32 c7 = -5.23022132118824778e-14f;

    f32 cosine = c0 + x * (c1 + x * (c2 + x * (c3 + x * (c4 + x * (c5 + x * (c6 + x * c7))))));
    return cosine;
}

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
    vec2 pos;
    u32 color;
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
