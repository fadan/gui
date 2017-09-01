#define STBTT_ifloor        floor32
#define STBTT_iceil         ceil32
#define STBTT_sqrt          sqrt32
#define STBTT_pow           pow32
#define STBTT_cos           cos32
#define STBTT_acos          acos32
#define STBTT_fabs          abs32
#define STBTT_fmod          mod32
#define STBTT_malloc(x, u)  platform.virtual_alloc(x)
#define STBTT_free(x, u)    platform.virtual_free(x)
#define STBTT_assert        assert
#define STBTT_strlen        string_length
#define STBTT_memcpy        copy_memory
#define STBTT_memset        set_memory

#define NULL 0
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

union vec2
{
    struct
    {
        f32 x, y;
    };
    struct
    {
        f32 u, v;
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
#define vec2_inner(a, x)    ( (a).x * (b).x + (a).y * (b).y )
#define vec2_hadamard(a, b) v2( (a).x * (b).x, (a).y * (b).y )

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

enum
{
    attrib_pos,
    attrib_uv,
    attrib_color,

    attrib_count,
};

enum
{
    uniform_tex,
    uniform_proj_mat,

    uniform_count,
};

struct Vertex
{
    vec2 pos;
    vec2 uv;
    u32 color;
};

struct Glyph
{
    unichar codepoint;
    f32 advance_x;

    vec2 min_pos;
    vec2 max_pos;
    vec2 min_uv;
    vec2 max_uv;
};

struct Font
{
    vec2 white_pixel_uv;
    f32 size;
    u32 texture_width;
    u32 texture_height;
    u8 *texture_pixels;
    f32 ascent;
    f32 descent;

    u32 num_glyphs;
    Glyph glyphs[256];
    f32 advance_x_lut[256];
    u16 glyph_index_lut[256];
};

struct DrawContext
{
    vec2 min_pos;
    vec2 max_pos;
    vec2 at;
    f32 current_line_height;
};

enum PanelStatus
{
    PanelStatus_Docked,

    PanelStatus_Float,
    PanelStatus_Dragged,
};

struct Panel
{
    Panel *parent;
    Panel *next_panel;
    Panel *prev_panel;

    union
    {
        Panel *next;
        Panel *next_free;
    };

    u32 id;
    PanelStatus status;

    char name[32];
    DrawContext *dc;
};

enum UIColors
{
    UIColor_Text,

    UIColor_Count,
};

struct UIState
{
    PlatformInput *input;

    MemoryStack memory;
    MemoryStack font_memory;
    
    u32 colors[UIColor_Count];

    u32 num_draw_contexts;
    DrawContext draw_contexts[32];
    DrawContext *draw_context;

    DrawContext *panels_dc;
    Panel *root_panel;
    Panel *first_panel;
    Panel *first_free_panel;

    Font current_font;

    GLuint texture;

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
