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

enum PanelFlags
{
    PanelFlag_None,

    PanelFlag_Movable    = 1 << 0,
    PanelFlag_HasHeader  = 1 << 1,
    PanelFlag_ResizableX = 1 << 2,
    PanelFlag_ResizableY = 1 << 3,
    PanelFlag_Bordered   = 1 << 4,

    PanelFlag_Resizable = (PanelFlag_ResizableX | PanelFlag_ResizableY),
    PanelFlag_Default   = (PanelFlag_Movable | PanelFlag_Resizable | PanelFlag_HasHeader | PanelFlag_Bordered),
};

enum Behavior
{
    Behavior_Inactive,

    Behavior_Hover,
    Behavior_Active,
    Behavior_LeftClick,
};

struct Panel
{
    union
    {
        Panel *next;
        Panel *next_free;
    };
    Panel *prev;

    u32 id;
    u32 flags;

    char name[32];

    rect2 bounds;
    rect2 layout;
    vec2 layout_at;

    f32 current_line_height;

    // NOTE(dan): for drawing

    u32 begin_vertex_index;
    u32 begin_element_index;
    u32 num_vertices;
    u32 num_elements;
};

enum UIColors
{
    UIColor_PanelHeaderBackground,
    UIColor_PanelBackground,
    UIColor_PanelBorder,

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

enum StyleType
{
    StyleType_u32,
    StyleType_f32,
    StyleType_vec2,
};

struct Style
{
    void *dest;
    StyleType type;
    union
    {
        u32 value_u32;
        f32 value_f32;
        vec2 value_vec2;
    };
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
    f32 menu_bar_height;
    vec2 menu_bar_padding;
    vec2 menu_bar_button_padding;
    vec2 panel_header_padding;
    vec2 panel_padding;
    vec2 button_padding;

    u32 num_custom_styles;
    Style custom_styles[32];

    // NOTE(dan): elements

    vec2 next_panel_pos;
    vec2 next_panel_size;

    vec2 min_pos;
    vec2 max_pos;

    Panel panel_sentinel;
    Panel *first_free_panel;
    Panel *current_panel;
    Panel *active_panel;

    // NOTE(dan): draw

    GLuint program;
    GLuint texture;

    GLuint vbo;
    GLuint vao;
    GLuint ebo;

    GLuint attribs[attrib_count];
    GLuint uniforms[uniform_count];

    Vertex *vertices;
    GLuint *elements;

    u32 num_vertices;
    u32 num_elements;
};

#define push_style(ui, dest_init, value, t) \
    { \
        assert(ui->num_custom_styles < array_count(ui->custom_styles)); \
        Style *style = ui->custom_styles + ui->num_custom_styles++; \
        style->dest = (void *)&ui->dest_init; \
        style->type = StyleType_##t; \
        style->value_##t = ui->dest_init; \
        ui->dest_init = value; \
    }

#define push_style_u32( ui, dest, value) push_style(ui, dest, value,  u32)
#define push_style_f32( ui, dest, value) push_style(ui, dest, value,  f32)
#define push_style_vec2(ui, dest, value) push_style(ui, dest, value, vec2)

#define push_style_color(ui, color_id, color) push_style_u32(ui, colors[color_id], color)

inline void pop_style(struct UIState *ui)
{
    assert(ui->num_custom_styles);
    if (ui->num_custom_styles)
    {
        Style *style = ui->custom_styles + ui->num_custom_styles - 1;
        switch (style->type)
        {
            case StyleType_u32:
            {
                *(u32 *)style->dest = style->value_u32;
            } break;
            case StyleType_f32:
            {
                *(f32 *)style->dest = style->value_f32;
            } break;
            case StyleType_vec2:
            {
                *(vec2 *)style->dest = style->value_vec2;
            } break;
            invalid_default_case;
        }
    }
    --ui->num_custom_styles;
}
