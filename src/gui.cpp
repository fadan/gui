#include "gui.h"

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

static char *ui_vertex_shader = R"GLSL(
    #version 330

    uniform mat4 proj_mat;

    in vec2 pos;
    in vec4 color;

    out vec4 frag_color;

    void main()
    {
        frag_color = color;
        gl_Position = proj_mat * vec4(pos, 0.0f, 1.0f);
    }
)GLSL";

static char *ui_fragment_shader = R"GLSL(
    #version 330

    in vec4 frag_color;
    
    out vec4 out_color;

    void main()
    {
        out_color = frag_color;
    }
)GLSL";

#define MAX_NUM_VERTICES 1024
#define MAX_NUM_ELEMENTS 1024

#include "ui_data.h"
#include "font.h"

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
    
    ui->attribs[attrib_pos]   = gl.GetAttribLocation(ui->program, "pos");
    ui->attribs[attrib_color] = gl.GetAttribLocation(ui->program, "color");

    gl.EnableVertexAttribArray(ui->attribs[attrib_pos]);
    gl.EnableVertexAttribArray(ui->attribs[attrib_color]);

    gl.VertexAttribPointer(ui->attribs[attrib_pos],   2, GL_FLOAT,         GL_FALSE, sizeof(Vertex), (void *)offset_of(Vertex, pos));
    gl.VertexAttribPointer(ui->attribs[attrib_color], 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(Vertex), (void *)offset_of(Vertex, color));

    init_memory_stack(&ui->font_memory, 1*MB);
    init_default_ui_texture(ui);

    Font *font = &ui->default_font;

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
    gl.UniformMatrix4fv(ui->uniforms[uniform_proj_mat], 1, GL_FALSE, &proj_mat[0][0]);

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

static void add_poly_outline(UIState *ui, vec2 *points, u32 point_count, u32 color, f32 thickness, b32 connect_last_with_first)
{
    u32 vertice_index = ui->num_vertices;
    u32 element_index = ui->num_elements;

    u32 num_points = point_count;
    if (!connect_last_with_first)
    {
        --num_points;
    }

    u32 num_vertices = num_points * 4;
    u32 num_elements = num_points * 6;

    assert(ui->num_vertices + num_vertices < MAX_NUM_VERTICES);
    assert(ui->num_elements + num_elements < MAX_NUM_ELEMENTS);

    ui->num_vertices += num_vertices;
    ui->num_elements += num_elements;

    for (u32 point_index = 0; point_index < num_points; ++point_index)
    {
        u32 next_point_index = ((point_index + 1) == point_count) ? 0 : point_index + 1;

        vec2 current_pos = points[point_index];
        vec2 next_pos = points[next_point_index];

        vec2 delta_pos = vec2_sub(next_pos, current_pos);
        f32 delta_pos_length2 = vec2_length2(delta_pos);
        f32 inv_delta_pos_length = inv_sqrt32(delta_pos_length2);

        delta_pos = vec2_mul(0.5f * thickness * inv_delta_pos_length, delta_pos);

        Vertex *vertex = ui->vertices + vertice_index;
        GLuint *element = ui->elements + element_index;

        vertex[0].pos.x = current_pos.x + delta_pos.y;
        vertex[0].pos.y = current_pos.y - delta_pos.x;
        vertex[0].color = color;

        vertex[1].pos.x = next_pos.x + delta_pos.y;
        vertex[1].pos.y = next_pos.y - delta_pos.x;
        vertex[1].color = color;

        vertex[2].pos.x = next_pos.x - delta_pos.y;
        vertex[2].pos.y = next_pos.y + delta_pos.x;
        vertex[2].color = color;

        vertex[3].pos.x = current_pos.x - delta_pos.y;
        vertex[3].pos.y = current_pos.y + delta_pos.x;
        vertex[3].color = color;

        element[0] = vertice_index + 0;
        element[1] = vertice_index + 1;
        element[2] = vertice_index + 2;
        element[3] = vertice_index + 0;
        element[4] = vertice_index + 2;
        element[5] = vertice_index + 3;

        vertice_index += 4;
        element_index += 6;
    }
}

static void add_poly_filled(UIState *ui, vec2 *vertices, u32 num_vertices, u32 color)
{
    u32 start_vertice_index = ui->num_vertices;
    u32 num_elements = (num_vertices - 2) * 3;

    assert(ui->num_vertices + num_vertices < MAX_NUM_VERTICES);
    assert(ui->num_elements + num_elements < MAX_NUM_ELEMENTS);

    for (u32 vertex_index = 0; vertex_index < num_vertices; ++vertex_index)
    {
        Vertex *vertex = ui->vertices + ui->num_vertices++;
        vertex->pos.x = vertices[vertex_index].x;
        vertex->pos.y = vertices[vertex_index].y;
        vertex->color = color;
    }

    for (u32 element_index = 2; element_index < num_vertices; ++element_index)
    {
        GLuint *element = ui->elements + ui->num_elements;
        element[0] = start_vertice_index;
        element[1] = start_vertice_index + element_index - 1;
        element[2] = start_vertice_index + element_index;

        ui->num_elements += 3;
    }
}

static void add_rect_outline(UIState *ui, vec2 top_left_corner, vec2 bottom_right_corner, u32 color)
{
    vec2 vertices[] =
    {
        v2(top_left_corner.x, top_left_corner.y),
        v2(top_left_corner.x, bottom_right_corner.y),
        v2(bottom_right_corner.x, bottom_right_corner.y),
        v2(bottom_right_corner.x, top_left_corner.y),
    };
    add_poly_outline(ui, vertices, array_count(vertices), color, 1.0f, true);
}

static void add_rect_filled(UIState *ui, vec2 top_left_corner, vec2 bottom_right_corner, u32 color)
{
    vec2 vertices[] =
    {
        v2(top_left_corner.x,     top_left_corner.y),
        v2(top_left_corner.x,     bottom_right_corner.y),
        v2(bottom_right_corner.x, bottom_right_corner.y),
        v2(bottom_right_corner.x, top_left_corner.y),
    };
    add_poly_filled(ui, vertices, array_count(vertices), color);
}

static void add_arc_filled(UIState *ui, vec2 center, f32 radius, u32 color, f32 start_angle, f32 end_angle, u32 num_segments)
{
    TempMemoryStack temp_memory = begin_temp_memory(&ui->memory);
    {
        vec2 *vertices = push_array(&ui->memory, num_segments + 1, vec2);

        f32 angle_step = (end_angle - start_angle) / num_segments;
        for (u32 vertex_index = 0; vertex_index <= num_segments; ++vertex_index)
        {
            f32 angle = start_angle + (f32)vertex_index * angle_step;

            vertices[vertex_index].x = center.x + cos32(angle) * radius;
            vertices[vertex_index].y = center.y - sin32(angle) * radius;
        }

        add_poly_filled(ui, vertices, num_segments + 1, color);
    }
    end_temp_memory(temp_memory);
}

inline void add_circle_filled(UIState *ui, vec2 center, f32 radius, u32 color)
{
    add_arc_filled(ui, center, radius, color, 0.0f, TAU32, 24);
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

    gl.ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gl.Clear(GL_COLOR_BUFFER_BIT);

    UIState *ui = &app_state->ui_state;
    {
        {
            vec2 min = v2(20, 20);
            vec2 max = v2(200, 200);

            add_rect_filled(ui, min, max, 0xFF00FFFF);
        }

        {
            vec2 min = v2(200, 200);
            vec2 max = v2(400, 400);

            add_rect_filled(ui, min, max, 0x00FF00FF);
        }

        {
            vec2 center = v2(500, 500);

            add_circle_filled(ui, center, 100.0f, 0xFFFFFFFF);
        }

        {
            vec2 points[] =
            {
                v2(500, 200),
                v2(550, 300),
                v2(600, 200),
            };

            add_poly_outline(ui, points, array_count(points), 0xFFFFFFFF, 1.0f, true);
        }

        {
            vec2 min = v2(600, 100);
            vec2 max = v2(700, 200);

            add_rect_outline(ui, min, max, 0xFFFFFFFF);
        }

        {
            vec2 min = v2(200, 200);
            vec2 max = v2(400, 400);

            add_rect_outline(ui, min, max, 0xFFFFFFFF);
        }
    }
    render_ui(ui, window_width, window_height);
}
