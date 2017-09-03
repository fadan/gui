
inline void clear_ui(UIState *ui, f32 ui_width, f32 ui_height)
{
    ui->num_draw_contexts = 1;

    DrawContext *dc = ui->draw_contexts + 0;
    dc->min_pos = v2(0, 0);
    dc->max_pos = v2(ui_width, ui_height);
    dc->at = dc->min_pos;

    ui->draw_context = dc;
}

inline DrawContext *push_draw_context(UIState *ui)
{
    // NOTE(dan): this triggers when there was no clear_ui call prior to this call
    // NOTE(dan): the first draw_context is reserved, it is the whole drawable region
    assert(ui->num_draw_contexts);
    assert(ui->num_draw_contexts < array_count(ui->draw_contexts));

    DrawContext *new_dc = ui->draw_contexts + ui->num_draw_contexts++;
    DrawContext *prev_dc = new_dc - 1;

    // TODO(dan): do we want the previous min_pos or the previous's at as our min_pos?
    new_dc->min_pos = prev_dc->min_pos;
    new_dc->max_pos = prev_dc->max_pos;
    new_dc->at = prev_dc->at;

    ui->draw_context = new_dc;
    return new_dc;
}

inline DrawContext *pop_draw_context(UIState *ui)
{
    // NOTE(dan): this triggers when there were more pop-s than push-es
    // NOTE(dan): the first draw_context is reserved, it is the whole drawable region
    assert(ui->num_draw_contexts > 1);

    DrawContext *prev_dc = ui->draw_contexts + --ui->num_draw_contexts;
    DrawContext *new_dc = prev_dc - 1;

    new_dc->at.y = prev_dc->max_pos.y + 1;
    ui->draw_context = new_dc;
    return new_dc;
}

inline void draw_gradient_background(UIState *ui, u32 top_left_color, u32 bottom_left_color, u32 bottom_right_color, u32 top_right_color)
{
    vec2 top_left_corner     = ui->draw_context->min_pos;
    vec2 bottom_right_corner = ui->draw_context->max_pos;

    add_color_quad(ui, top_left_corner, bottom_right_corner, top_left_color, top_right_color, bottom_left_color, bottom_right_color);
}

inline b32 mouse_intersect_rect(UIState *ui, vec2 top_left_corner, vec2 bottom_right_corner)
{
    f32 x = (f32)ui->input->mouse_pos[0];
    f32 y = (f32)ui->input->mouse_pos[1];
    b32 intersect = ((x >= top_left_corner.x && x < bottom_right_corner.x) &&
                     (y >= top_left_corner.y && y < bottom_right_corner.y));
    return intersect;
}

inline void begin_menu_bar(UIState *ui, f32 bar_height)
{
    DrawContext *prev_dc = ui->draw_context;
    DrawContext *dc = push_draw_context(ui);

    dc->max_pos.y = bar_height;

    add_rect_filled(ui, dc->min_pos, dc->max_pos, 0xAAFFFFFF);
    add_rect_filled(ui, vec2_add(dc->min_pos, v2(0, bar_height - 2.0f)), dc->max_pos, 0xAA7e72c5);
}

inline void end_menu_bar(UIState *ui)
{
    pop_draw_context(ui);
}

static void menu_bar_button(UIState *ui, char *name)
{
    DrawContext *dc = ui->draw_context;
    Font *font = &ui->current_font;

    // TODO(dan): storage for these
    f32 bar_height = dc->max_pos.y - dc->min_pos.y;
    vec2 padding = v2(15.0f, 0.0f);

    vec2 text_size = calc_text_size(ui, name, font->size);
    vec2 button_size = v2(padding.x + text_size.x + padding.x, bar_height);

    vec2 button_top_left = dc->at;
    vec2 button_bottom_right = vec2_add(dc->at, button_size);

    if (mouse_intersect_rect(ui, button_top_left, button_bottom_right))
    {
        vec2 line_top_left = v2(button_top_left.x, button_bottom_right.y - 2.0f);
        vec2 line_bottom_right = button_bottom_right;

        add_rect_filled(ui, line_top_left, line_bottom_right, 0xFFFB9608);
    }

    // TODO(dan): clip
    vec2 text_pos = vec2_add(dc->at, v2(padding.x, (button_size.y - (2.0f * padding.y) - text_size.y) / 2.0f));
    add_text(ui, name, text_pos, font->size, 0xFF201B1C);

    dc->at.x += button_size.x;
}

static Panel *get_or_create_panel(UIState *ui, char *name)
{
    u32 id = djb2_hash(name);
    Panel *panel = 0;

    for (Panel *test_panel = ui->first_panel; test_panel; test_panel = test_panel->next)
    {
        if (test_panel->id == id)
        {
            panel = test_panel;
            break;
        }
    }

    if (!panel)
    {
        allocate_freelist(panel, ui->first_free_panel, push_struct(&ui->memory, Panel));

        *panel = {0};
        panel->next = ui->first_panel;
        ui->first_panel = panel;

        panel->id = id;
        panel->status = PanelStatus_Float;

    }
    return panel;
}

static void begin_panels(UIState *ui)
{
    ui->panels_dc = push_draw_context(ui);
}

static void end_panels(UIState *ui)
{
    pop_draw_context(ui);
}

static void begin_panel(UIState *ui, char *name, vec2 default_size)
{
    Panel *panel = get_or_create_panel(ui, name);
    DrawContext *dc = push_draw_context(ui);

    if (panel->min_pos.x == panel->max_pos.x && panel->min_pos.y == panel->max_pos.y)
    {
        panel->min_pos = ui->panels_dc->min_pos;
        panel->max_pos = vec2_add(panel->min_pos, default_size);

        ui->active_panel = panel;
    }

    u32 background_color = ui->colors[UIColor_PanelBackground];

    if (panel == ui->active_panel)
    {
        if (panel->status == PanelStatus_Dragged)
        {
            vec2 size = vec2_sub(panel->max_pos, panel->min_pos);

            panel->min_pos = v2(ui->input->mouse_pos[0] - ui->drag_offset.x, ui->input->mouse_pos[1] - ui->drag_offset.y);
            panel->max_pos = vec2_add(panel->min_pos, size);

            if (!ui->input->mouse_buttons[mouse_button_left].down)
            {
                panel->status = PanelStatus_Float;
            }
        }
    }

    if (mouse_intersect_rect(ui, panel->min_pos, panel->max_pos))
    {
        if (ui->input->mouse_buttons[mouse_button_left].down)
        {
            ui->active_panel = panel;
            background_color = (background_color & 0xFFFFFF) | (0x80 << 24);
            panel->status = PanelStatus_Dragged;
            ui->drag_offset = v2(ui->input->mouse_pos[0] - panel->min_pos.x, (f32)ui->input->mouse_pos[1] - panel->min_pos.y);
        }
    }

    dc->min_pos = panel->min_pos;
    dc->max_pos = panel->max_pos;
    dc->at = dc->min_pos;

    vec2 text_size = calc_text_size(ui, name, ui->current_font.size);
    vec2 padding = v2(10, 5);
    vec2 text_bb = vec2_add(text_size, vec2_mul(2.0f, padding));
    vec2 text_pos = vec2_add(panel->min_pos, padding);

    add_rect_filled(ui, panel->min_pos, vec2_add(panel->min_pos, text_bb), 0xFFCCCCCC);
    add_text(ui, name, text_pos, ui->current_font.size, ui->colors[UIColor_Text]);

    dc->at.y += text_size.y + padding.y * 2;
    add_rect_filled(ui, dc->at, panel->max_pos, background_color);
}

static void end_panel(UIState *ui)
{
    pop_draw_context(ui);
}
