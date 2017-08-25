#if 0
// dst and src must be 16-byte aligned
// size must be multiple of 16*2 = 32 bytes
static void copy_memory_sse2_aligned(u8 *dst, u8 *src, u64 size)
{
    u64 stride = 2 * sizeof(__m128);
    while (size)
    {
        __m128 a = _mm_load_ps((float *)(src + 0 * sizeof(__m128)));
        __m128 b = _mm_load_ps((float *)(src + 1 * sizeof(__m128)));

        _mm_stream_ps((float *)(dst + 0 * sizeof(__m128)), a);
        _mm_stream_ps((float *)(dst + 1 * sizeof(__m128)), b);

        size -= stride;
        src += stride;
        dst += stride;
    }
}
#endif

static void set_memory(void *memory, char set_byte, u64 num_bytes)
{
    u16 set_word  = ((u16)set_byte  <<  8) | set_byte;
    u32 set_dword = ((u32)set_word  << 16) | set_word;
    u64 set_qword = ((u64)set_dword << 32) | set_dword;

    // NOTE(dan): set with bytes while unaligned
    u8 *byte_ptr    = (u8 *)memory;
    u8 *aligned_ptr = (u8 *)((u64)byte_ptr & (u64)-8);
    u32 num_misaligned_bytes = (u32)(byte_ptr - aligned_ptr);

    for (u32 byte_index = 0; byte_index < num_misaligned_bytes; ++byte_index)
    {
        *byte_ptr++ = set_byte;
    }
    num_bytes -= num_misaligned_bytes;

    // NOTE(dan): set with qwords while possible
    u64 *qword_ptr = (u64 *)byte_ptr;
    u64 num_qwords = (num_bytes >> 3);

    for (u64 qword_index = 0; qword_index < num_qwords; ++qword_index)
    {
        *qword_ptr++ = set_qword;
    }

    // NOTE(dan): set the remaining bytes
    byte_ptr  = (u8 *)qword_ptr;
    num_bytes = num_bytes & 7;

    for (u32 byte_index = 0; byte_index < num_bytes; ++byte_index)
    {
        *byte_ptr++ = set_byte;
    }
}

static void copy_memory(void *dest, void *src, u64 num_bytes)
{
    // NOTE(dan): copy bytes while unaligned
    u8 *src_byte_ptr         = (u8 *)src;
    u8 *dest_byte_ptr        = (u8 *)dest;
    u8 *aligned_src_ptr      = (u8 *)((u64)src_byte_ptr & ~7);
    u32 num_misaligned_bytes = (u32)(src_byte_ptr - aligned_src_ptr);

    if (num_misaligned_bytes > (u32)num_bytes)
    {
        num_misaligned_bytes = (u32)num_bytes;
    }

    for (u32 byte_index = 0; byte_index < num_misaligned_bytes; ++byte_index)
    {
        *dest_byte_ptr++ = *src_byte_ptr++;
    }
    num_bytes -= num_misaligned_bytes;

    // NOTE(dan): copy qwords while possible
    u64 *src_qword_ptr  = (u64 *)src_byte_ptr;
    u64 *dest_qword_ptr = (u64 *)dest_byte_ptr;
    u64 num_qwords      = (num_bytes >> 3);

    for (u64 qword_index = 0; qword_index < num_qwords; ++qword_index)
    {
        *dest_qword_ptr++ = *src_qword_ptr++;
    }

    // NOTE(dan): copy the remaining bytes
    src_byte_ptr  = (u8 *)src_qword_ptr;
    dest_byte_ptr = (u8 *)dest_qword_ptr;
    num_bytes     = num_bytes & 7;

    for (u32 byte_index = 0; byte_index < num_bytes; ++byte_index)
    {
        *dest_byte_ptr++ = *src_byte_ptr++;
    }
}

#define PI32  3.14159265359f
#define TAU32 6.28318530717958647692f

inline f32 abs32(f32 x)
{
    f32 result = (x < 0.0f) ? -x : x;
    return result;
}

inline i32 floor32(f32 x)
{
    i32 result = ((i32)x - ((x < 0.0f) ? 1 : 0));
    return result;
}

inline f32 mod32(f32 a, f32 b)
{
    f32 result = (a - b * floor32(a / b));
    return result;
}

inline i32 ceil32(f32 x)
{
    i32 result = (i32)x;
    if (x < 0.0f)
    {
        f32 frac = x - result;
        if (frac > 0.0f)
        {
            ++result;
        }
    }
    return result;
}

inline f32 pow32(f32 a, f32 b)
{
    union { f32 f; i32 i; } memory = {a};
    memory.i = (i32)(b * (memory.i - 1064866805) + 1064866805);
    return memory.f;
}

inline f32 acos32(f32 x)
{
    static f32 c0 = -0.939115566365855f;
    static f32 c1 = +0.9217841528914573f;
    static f32 c2 = -1.2845906244690837f;
    static f32 c3 = +0.295624144969963174f;

    f32 x2 = x * x;
    f32 result = (PI32 / 2.0f) + (c0 * x + c1 * x2 * x) / (1 + c2 * x2 + c3 * x2 * x2);
    return result;
}

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

inline f32 sin32(f32 x)
{
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

inline u32 string_length(char *string)
{
    u32 length = 0;
    while (*string++)
    {
        ++length;
    }
    return length;
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
};

struct UIState
{
    PlatformInput *input;

    MemoryStack memory;
    MemoryStack font_memory;

    u32 num_draw_contexts;
    DrawContext draw_contexts[32];
    DrawContext *draw_context;

    Font default_font;

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
