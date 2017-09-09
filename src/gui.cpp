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

#include "gui.h"

#define MAX_NUM_VERTICES 1024
#define MAX_NUM_ELEMENTS 1024

#include "ui_data.h"

#include "gui_draw.cpp"
#include "gui_elements.cpp"

#include "format_string.h"

struct AppState
{
    MemoryStack app_memory;

    UIState ui_state;
};

static void opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar *message, GLvoid *user_param)
{
    if (severity == GL_DEBUG_SEVERITY_HIGH)
    {
        char *error = (char *)message;
        assert(!error);
    }
}

static void init_ui(UIState *ui, PlatformInput *input)
{
    ui->input = input;

    ui->num_vertices = 0;
    ui->num_elements = 0;
    ui->vertices = push_array(&ui->memory, MAX_NUM_VERTICES, Vertex);
    ui->elements = push_array(&ui->memory, MAX_NUM_ELEMENTS, GLuint);

    char error[1024];
    ui->program = opengl_create_program(ui_vertex_shader, ui_fragment_shader, error, sizeof(error));

    gl.GenVertexArrays(1, &ui->vao);
    gl.BindVertexArray(ui->vao);

    gl.GenBuffers(1, &ui->vbo);
    gl.BindBuffer(GL_ARRAY_BUFFER, ui->vbo);
    
    gl.GenBuffers(1, &ui->ebo);
    gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui->ebo);

    ui->uniforms[uniform_proj_mat] = gl.GetUniformLocation(ui->program, "proj_mat");
    ui->uniforms[uniform_tex]      = gl.GetUniformLocation(ui->program, "tex");
    
    ui->attribs[attrib_pos]   = gl.GetAttribLocation(ui->program, "pos");
    ui->attribs[attrib_uv]    = gl.GetAttribLocation(ui->program, "uv");
    ui->attribs[attrib_color] = gl.GetAttribLocation(ui->program, "color");

    gl.EnableVertexAttribArray(ui->attribs[attrib_pos]);
    gl.EnableVertexAttribArray(ui->attribs[attrib_uv]);
    gl.EnableVertexAttribArray(ui->attribs[attrib_color]);

    gl.VertexAttribPointer(ui->attribs[attrib_pos],   2, GL_FLOAT,         GL_FALSE, sizeof(Vertex), (void *)offset_of(Vertex, pos));
    gl.VertexAttribPointer(ui->attribs[attrib_uv],    2, GL_FLOAT,         GL_FALSE, sizeof(Vertex), (void *)offset_of(Vertex, uv));
    gl.VertexAttribPointer(ui->attribs[attrib_color], 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(Vertex), (void *)offset_of(Vertex, color));

    init_memory_stack(&ui->font_memory, 1*MB);
    init_default_ui_texture(ui);
    set_default_colors(ui);

    Font *font = &ui->current_font;

    gl.GenTextures(1, &ui->texture);
    gl.BindTexture(GL_TEXTURE_2D, ui->texture);
    gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, font->texture_width, font->texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, font->texture_pixels);
}

static void render_ui(UIState *ui, i32 display_width, i32 display_height)
{
    GLfloat proj_mat[4][4] = 
    {
        { 2.0f / display_width, 0.0f,                  0.0f, 0.0f },
        { 0.0f,                -2.0f / display_height, 0.0f, 0.0f },
        { 0.0f,                 0.0f,                 -1.0f, 0.0f },
        {-1.0f,                 1.0f,                  0.0f, 1.0f },
    };

    gl.Viewport(0, 0, display_width, display_height);
    gl.UseProgram(ui->program);
    gl.BindVertexArray(ui->vao);
    gl.Uniform1i(ui->uniforms[uniform_tex], 0);
    gl.UniformMatrix4fv(ui->uniforms[uniform_proj_mat], 1, GL_FALSE, &proj_mat[0][0]);
    gl.Enable(GL_BLEND);
    gl.BlendEquation(GL_FUNC_ADD);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.Disable(GL_CULL_FACE);
    gl.Disable(GL_DEPTH_TEST);

    gl.ActiveTexture(GL_TEXTURE0);
    gl.BindTexture(GL_TEXTURE_2D, ui->texture);

    if (ui->num_elements)
    {
        gl.BindBuffer(GL_ARRAY_BUFFER, ui->vbo);
        gl.BufferData(GL_ARRAY_BUFFER, ui->num_vertices * sizeof(Vertex), ui->vertices, GL_STREAM_DRAW);

        gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui->ebo);
        gl.BufferData(GL_ELEMENT_ARRAY_BUFFER, ui->num_elements * sizeof(GLuint), ui->elements, GL_STREAM_DRAW);

        gl.DrawElements(GL_TRIANGLES, ui->num_elements, GL_UNSIGNED_INT, 0);
    }

    ui->num_elements = 0;
    ui->num_vertices = 0;
}

// inline void ui_newline(UIState *ui)
// {
//     DrawContext *dc = ui->draw_context;
//     dc->at.x = dc->min_pos.x;
//     dc->at.y += dc->current_line_height;
// }

// inline void ui_text(UIState *ui, char *text)
// {
//     DrawContext *dc = ui->draw_context;
//     vec2 pos = dc->at;
//     f32 font_size = ui->current_font.size;
//     u32 color = ui->colors[UIColor_Text];

//     vec2 text_size = add_text(ui, text, pos, font_size, color);
//     dc->at.x += text_size.x;
//     dc->current_line_height = max(dc->current_line_height, text_size.y);
// }

// inline void ui_textf(UIState *ui, char *format, ...)
// {
//     char text_buffer[256];
//     param_list params = get_params_after(format);
//     format_string_vararg(text_buffer, sizeof(text_buffer), format, params);
//     ui_text(ui, text_buffer);
// }

inline void change_unit_and_size(char **unit, usize *size)
{
    *unit = "B";
    if (*size > 1024*MB)
    {
        *unit = "GB";
        *size /= GB;
    }
    else if (*size > 1024*KB)
    {
        *unit = "MB";
        *size /= MB;
    }
    else if (*size > 1024)
    {
        *unit = "KB";
        *size /= KB;
    }
}

static UPDATE_AND_RENDER(update_and_render)
{
    AppState *app_state = memory->app_state;
    if (!app_state)
    {
        app_state = memory->app_state = bootstrap_push_struct(AppState, app_memory);

        platform.init_opengl(&gl);

        if (gl.DebugMessageCallback)
        {
            gl.Enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            gl.DebugMessageCallback(opengl_debug_callback, 0);
        }

        init_ui(&app_state->ui_state, input);
    }

    gl.ClearColor(0xED / 255.0f, 0xEF / 255.0f, 0xF5 / 255.0f, 1.0f);
    gl.Clear(GL_COLOR_BUFFER_BIT);

    UIState *ui = &app_state->ui_state;
    begin_ui(ui, window_width, window_height);
    {
        // NOTE(dan): background
        #if 0
        {
            u32 top_left_color     = 0xFFAE323F;
            u32 bottom_left_color  = 0xFFb2364E;
            u32 top_right_color    = 0xFFC4558E;
            u32 bottom_right_color = 0xFFC95d9F;
            
            add_color_quad(ui, ui->min_pos, ui->max_pos, 
                           top_left_color, top_right_color, bottom_left_color, bottom_right_color);
        }
        #endif
        
        // NOTE(dan): menu
        {
            f32 menu_bar_height = 25.0f;
            begin_menu_bar(ui, menu_bar_height);
            {
                if (menu_bar_button(ui, "File"))
                {
                }
                if (menu_bar_button(ui, "View"))
                {
                }
            }
            end_menu_bar(ui);
        }
    }
    render_ui(ui, window_width, window_height);

    #if 0
    clear_ui(ui, (f32)window_width, (f32)window_height);
    {
        // NOTE(dan): background
        {
            u32 top_left_color     = 0xFFae323f;
            u32 bottom_left_color  = 0xFFb2364e;
            u32 top_right_color    = 0xFFc4558e;
            u32 bottom_right_color = 0xFFc95d9f;
            draw_gradient_background(ui, top_left_color, bottom_left_color, bottom_right_color, top_right_color);
        }

        // NOTE(dan): menu bar
        {
            begin_menu_bar(ui, 25.0f);
            {
                menu_bar_button(ui, "File");
                menu_bar_button(ui, "View");
            }
            end_menu_bar(ui);
        }

        begin_panels(ui);
        {
            begin_panel(ui, "Info", v2(300, 200));
            {
                PlatformMemoryStats memory_stats = platform.get_memory_stats();

                char *total_size_unit = "B";
                usize total_size = memory_stats.total_size;
                change_unit_and_size(&total_size_unit, &total_size);

                char *total_used_unit = "B";
                usize total_used = memory_stats.total_used;
                change_unit_and_size(&total_used_unit, &total_used);

                ui_textf(ui, "Mouse: (%d, %d)", input->mouse_pos[0], input->mouse_pos[1]);
                ui_newline(ui);
                ui_textf(ui, "Vertices: %d Elements: %d", ui->num_vertices, ui->num_elements);
                ui_newline(ui);
                ui_textf(ui, "Memory blocks: %d Total: %d%s Used: %d%s", 
                         memory_stats.num_memblocks, total_size, total_size_unit, total_used, total_used_unit);
                ui_newline(ui);
                ui_textf(ui, "Frame time: %.3fs", input->dt);
            }
            end_panel(ui);

            begin_panel(ui, "Volume Mixer", v2(300, 200));
            {
                ui_text(ui, "asdf");
            }
            end_panel(ui);
        }
        end_panels(ui);
    }
    #endif
}
