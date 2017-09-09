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
    // TODO(dan): store only the kern table from font_info
    stbtt_fontinfo font_info;

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

enum PanelStatus
{
    PanelStatus_Float,

    PanelStatus_Docked,
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

    vec2 min_pos;
    vec2 max_pos;
    vec2 at;

    // 
    
    f32 current_line_height;
};

enum UIColors
{
    UIColor_MenuBarBackground,

    UIColor_ButtonBackground,
    UIColor_ButtonBackgroundActive,
    UIColor_ButtonBackgroundHover,

    UIColor_ButtonText,
    UIColor_ButtonTextHover,
    UIColor_ButtonTextActive,

    UIColor_ButtonBorderHover,

    UIColor_Text,

    UIColor_Count,
};

struct UIState
{
    // NOTE(dan): storage

    PlatformInput *input;

    vec2 mouse_pos;
    vec2 prev_mouse_pos;
    vec2 delta_mouse_pos;
    vec2 clicked_at;

    MemoryStack memory;
    MemoryStack font_memory;

    Font current_font;

    // TODO(dan): how do we want to store styles?

    u32 colors[UIColor_Count];
    vec2 menu_bar_padding;

    // NOTE(dan): elements

    vec2 min_pos;
    vec2 max_pos;

    u32 num_panels;
    Panel panels[32];
    Panel *current_panel;

    // NOTE(dan): draw

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
