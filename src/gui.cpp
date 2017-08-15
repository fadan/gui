#include "gui.h"

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
}

static void render_ui(UIState *ui, i32 display_width, i32 display_height)
{
    gl.Viewport(0, 0, display_width, display_height);
    gl.UseProgram(ui->program);
    gl.BindVertexArray(ui->vao);

    GLfloat proj_mat[4][4] = 
    {
        { 2.0f / display_width, 0.0f,                  0.0f, 0.0f },
        { 0.0f,                -2.0f / display_height, 0.0f, 0.0f },
        { 0.0f,                 0.0f,                 -1.0f, 0.0f },
        {-1.0f,                 1.0f,                  0.0f, 1.0f },
    };

    gl.UniformMatrix4fv(ui->uniforms[uniform_proj_mat], 1, GL_FALSE, &proj_mat[0][0]);

    if (ui->num_elements)
    {
        gl.BindBuffer(GL_ARRAY_BUFFER, ui->vbo);
        gl.BufferData(GL_ARRAY_BUFFER, ui->num_vertices * sizeof(Vertex), ui->vertices, GL_STREAM_DRAW);

        gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui->ebo);
        gl.BufferData(GL_ELEMENT_ARRAY_BUFFER, ui->num_elements * sizeof(GLuint), ui->elements, GL_STREAM_DRAW);

        gl.DrawElements(GL_TRIANGLES, ui->num_elements, GL_UNSIGNED_INT, 0);
    }
}

inline vec4 v4(f32 x, f32 y, f32 z, f32 w)
{
    vec4 result = {x, y, z, w};
    return result;
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
        vertex->pos[0] = vertices[vertex_index].x;
        vertex->pos[1] = vertices[vertex_index].y;
        *(u32 *)vertex->color = color;
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

            add_rect_filled(ui, min, max, 0xFFFFFFFF);
        }

        {
            vec2 min = v2(200, 200);
            vec2 max = v2(400, 400);

            add_rect_filled(ui, min, max, 0xFFFFFFFF);
        }
    }
    render_ui(ui, window_width, window_height);

    ui->num_elements = 0;
    ui->num_vertices = 0;
}
