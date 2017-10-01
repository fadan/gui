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

#define MAX_NUM_VERTICES 4096
#define MAX_NUM_ELEMENTS 8192

#include "format_string.h"

#include "gui.h"
#include "gui_data.h"
#include "gui_draw.cpp"
#include "gui_elements.cpp"

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

    dllist_init(&ui->panel_sentinel);
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

    gl.BindBuffer(GL_ARRAY_BUFFER, ui->vbo);
    gl.BufferData(GL_ARRAY_BUFFER, ui->num_vertices * sizeof(Vertex), ui->vertices, GL_STREAM_DRAW);

    gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui->ebo);
    gl.BufferData(GL_ELEMENT_ARRAY_BUFFER, ui->num_elements * sizeof(GLuint), ui->elements, GL_STREAM_DRAW);

    gl.Enable(GL_SCISSOR_TEST);

    Panel *panel_sentinel = &ui->panel_sentinel;
    for (Panel *panel = panel_sentinel->next; panel != panel_sentinel; panel = panel->next)
    {
        if (!(panel->flags & PanelFlag_Hidden))
        {
            vec2 min_pos = v2(panel->bounds.min_pos.x, display_height - panel->bounds.max_pos.y);
            vec2 dim = rect2_dim(panel->bounds);
            u32 element_index = panel->begin_element_index * sizeof(u32);

            gl.Scissor((GLint)min_pos.x, (GLint)min_pos.y, (GLsizei)dim.x, (GLsizei)dim.y);
            gl.DrawElements(GL_TRIANGLES, panel->num_elements, GL_UNSIGNED_INT, (void *)element_index);
        }
    }

    gl.Disable(GL_SCISSOR_TEST);

    ui->num_elements = 0;
    ui->num_vertices = 0;
}

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

    // gl.ClearColor(27.0f / 255.0f, 27.0f / 255.0f, 29.0f / 255.0f, 1.0f);
    gl.ClearColor(14.0f / 255.0f, 28.0f / 255.0f, 42.0f / 255.0f, 1.0f);
    gl.Clear(GL_COLOR_BUFFER_BIT);

    UIState *ui = &app_state->ui_state;
    begin_ui(ui, window_width, window_height);
    {
        static u32 last_frame_num_vertices = 0;
        static u32 last_frame_num_elements = 0;

        // NOTE(dan): menu bar
        begin_menu_bar(ui);
        {
            if (begin_menu(ui, "File"))
            {
                if (menu_button(ui, "Exit"))
                {
                    input->quit_requested = true;
                }
            }
            end_menu(ui);

            if (begin_menu(ui, "View"))
            {
                if (menu_button(ui, "Test 1"))
                {
                }
                if (menu_button(ui, "Test 2"))
                {
                }
                if (menu_button(ui, "Test 3"))
                {
                }
            }
            end_menu(ui);
        }
        end_menu_bar(ui);

        // NOTE(dan): info panel
        {
            ui->next_panel_pos  = v2(660.0f, 30.0f);
            ui->next_panel_size = v2(300.0f, 200.0f);

            Panel *panel = begin_panel(ui, "Info", PanelFlag_Default);
            {
                PlatformMemoryStats memory_stats = platform.get_memory_stats();

                char *total_size_unit = "B";
                usize total_size = memory_stats.total_size;
                change_unit_and_size(&total_size_unit, &total_size);

                char *total_used_unit = "B";
                usize total_used = memory_stats.total_used;
                change_unit_and_size(&total_used_unit, &total_used);

                textf_out(ui, "Frame time: %.3fs", input->dt);
                newline(ui);
                textf_out(ui, "Vertices: %d Elements: %d", last_frame_num_vertices, last_frame_num_elements);
                newline(ui);
                textf_out(ui, "Memory blocks: %d Total: %d%s Used: %d%s", 
                          memory_stats.num_memblocks, total_size, total_size_unit, total_used, total_used_unit);
                newline(ui);
                newline(ui);
                textf_out(ui, "%15s = vec2(%.1f, %.1f)", "mouse_pos", ui->mouse_pos.x, ui->mouse_pos.y);
                newline(ui);
                textf_out(ui, "%15s = vec2(%.1f, %.1f)", "delta_moues_pos", ui->delta_mouse_pos.x, ui->delta_mouse_pos.y);
                newline(ui);
                textf_out(ui, "%15s = vec2(%.1f, %.1f)", "clicked_at", ui->clicked_at.x, ui->clicked_at.y);
                newline(ui);
                newline(ui);

                textf_out(ui, "%24s = %s", "is_mouse_down", is_mouse_down(ui, mouse_button_left) ? "true" : "false");
                newline(ui);
                textf_out(ui, "%24s = %s", "is_mouse_down_in_rect", is_mouse_down_in_rect(ui, mouse_button_left, panel->bounds) ? "true" : "false");
                newline(ui);
                textf_out(ui, "%24s = %s", "is_mouse_hovered_rect", is_mouse_hovered_rect(ui, panel->bounds) ? "true" : "false");
            }
            end_panel(ui);
        }

        // NOTE(dan): test panel
        {
            ui->next_panel_pos = v2(350.0f, 30.0f);
            ui->next_panel_size = v2(300.0f, 500.0f);

            Panel *panel = begin_panel(ui, "Test", PanelFlag_Default);
            {
                text_out(ui, "Panels: ");
                newline(ui);

                Panel *sentinel = &ui->panel_sentinel;
                for (Panel *p = sentinel->next; p != sentinel; p = p->next)
                {
                    textf_out(ui, "%s:", p->name);
                    newline(ui);
                    textf_out(ui, "bounds: min(%.1f, %.1f) max(%.1f, %.1f)", p->bounds.min_pos.x, p->bounds.min_pos.y, p->bounds.max_pos.x, p->bounds.max_pos.y);
                    newline(ui);
                    textf_out(ui, "parent: %s", p->parent ? p->parent->name : "(null)");
                    newline(ui);
                    textf_out(ui, "child: %s", p->child ? p->child->name : "(null)");
                    newline(ui);
                    textf_out(ui, "hidden: %s", (p->flags & PanelFlag_Hidden) ? "true" : "false");
                    newline(ui);
                    textf_out(ui, "popup: %s", (p->flags & PanelFlag_Popup) ? "true" : "false");
                    newline(ui);
                    newline(ui);
                }
                
                static b32 toggle_value = true;
                if (button(ui, toggle_value ? "true" : "false"))
                {
                    toggle_value = !toggle_value;
                }
            }
            end_panel(ui);
        }

        last_frame_num_vertices = ui->num_vertices;
        last_frame_num_elements = ui->num_elements;
    }
    render_ui(ui, window_width, window_height);
}
