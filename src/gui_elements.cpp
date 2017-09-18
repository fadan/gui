inline void begin_ui(UIState *ui, u32 window_width, u32 window_height)
{
    PlatformInput *input = ui->input;

    ui->min_pos = v2(0, 0);
    ui->max_pos = v2((f32)window_width, (f32)window_height);

    ui->prev_mouse_pos = ui->mouse_pos;
    ui->mouse_pos = v2((f32)input->mouse_pos[0], (f32)input->mouse_pos[1]);
    ui->delta_mouse_pos = v2((f32)input->delta_mouse_pos[0], (f32)input->delta_mouse_pos[1]);

    if (input->mouse_buttons[mouse_button_left].pressed)
    {
        ui->clicked_at = ui->mouse_pos;
    }
}

inline b32 is_mouse_down(UIState *ui, MouseButtonID button_id)
{
    b32 down = (ui->input->mouse_buttons[button_id].down);
    return down;
}

inline b32 is_mouse_clicked_in_rect(UIState *ui, MouseButtonID button_id, rect2 rect)
{
    b32 clicked_in_rect = (ui->input->mouse_buttons[button_id].down &&
                           vec2_intersect(ui->clicked_at, rect.min_pos, rect.max_pos));
    return clicked_in_rect;
}

inline b32 is_mouse_hovered_rect(UIState *ui, rect2 rect)
{
    b32 hovered_rect = (vec2_intersect(ui->mouse_pos, rect.min_pos, rect.max_pos));
    return hovered_rect;
}

inline b32 was_mouse_hovered_rect(UIState *ui, rect2 rect)
{
    b32 was_hovered_rect = (vec2_intersect(ui->prev_mouse_pos, rect.min_pos, rect.max_pos));
    return was_hovered_rect;
}

inline Panel *find_panel_by_id(UIState *ui, u32 id)
{
    Panel *panel_sentinel = &ui->panel_sentinel;
    Panel *panel = 0;
    for (Panel *test_panel = panel_sentinel->next; test_panel != panel_sentinel; test_panel = test_panel->next)
    {
        if (test_panel->id == id)
        {
            panel = test_panel;
            break;
        }
    }
    return panel;
}

static Panel *get_or_create_panel(UIState *ui, char *name)
{
    u32 id = djb2_hash(name);

    Panel *panel = find_panel_by_id(ui, id);
    if (!panel)
    {
        allocate_freelist(panel, ui->first_free_panel, push_struct(&ui->memory, Panel));
        dllist_insert_last(&ui->panel_sentinel, panel);

        panel->id = id;
        panel->bounds = rect2_min_dim(ui->next_panel_pos, ui->next_panel_size);

        copy_string_and_null_terminate(name, panel->name, array_count(panel->name));
    }
    return panel;
}

static Panel *begin_panel(UIState *ui, char *name, u32 flags = PanelFlag_None)
{
    Panel *panel = get_or_create_panel(ui, name);
    panel->begin_vertex_index = ui->num_vertices;
    panel->begin_element_index = ui->num_elements;
    panel->flags = flags;

    ui->current_panel = panel;

    // NOTE(dan): handle drag
    rect2 header_bb = r2(panel->bounds.min_pos, panel->bounds.max_pos);
    if (panel->flags & PanelFlag_Movable)
    {
        if (panel->flags & PanelFlag_HasHeader)
        {
            header_bb.max_pos.y = header_bb.min_pos.y + ui->current_font.size + 2 * ui->panel_header_padding.y;
        }

        if (is_mouse_clicked_in_rect(ui, mouse_button_left, header_bb))
        {
            panel->bounds = rect2_offset(panel->bounds, ui->delta_mouse_pos);
            ui->clicked_at = vec2_add(ui->clicked_at, ui->delta_mouse_pos);
        }
    }
    vec2 header_dim = rect2_dim(header_bb);

    panel->layout.min_pos = vec2_add(panel->bounds.min_pos, ui->panel_padding);
    panel->layout.max_pos = vec2_sub(panel->bounds.max_pos, ui->panel_padding);
    panel->layout_at = panel->layout.min_pos;
    panel->current_line_height = 0;

    // NOTE(dan): draw header
    if (panel->flags & PanelFlag_HasHeader)
    {
        u32 background_color = ui->colors[UIColor_PanelHeaderBackground];
        u32 text_color       = ui->colors[UIColor_Text];
        vec2 text_pos = vec2_add(panel->bounds.min_pos, ui->panel_header_padding);

        add_rect_filled(ui, header_bb.min_pos, header_bb.max_pos, background_color);
        add_text(ui, name, text_pos, ui->current_font.size, text_color);

        panel->layout_at.y += header_dim.y;
        panel->layout.min_pos.y += header_dim.y;
    }

    // NOTE(dan): draw panel background
    {
        u32 background_color = ui->colors[UIColor_PanelBackground];
        rect2 panel_bb = panel->bounds;

        if (panel->flags & PanelFlag_HasHeader)
        {
            panel_bb.min_pos.y += header_dim.y;
        }

        add_rect_filled(ui, panel_bb.min_pos, panel_bb.max_pos, background_color);
    }

    // NOTE(dan): handle panel overlaps
    if (is_mouse_clicked_in_rect(ui, mouse_button_left, panel->bounds))
    {
        Panel *panel_sentinel = &ui->panel_sentinel;
        Panel *active_panel = 0;
        for (Panel *test_panel = panel->next; test_panel != panel_sentinel; test_panel = test_panel->next)
        {
            if (vec2_intersect(ui->mouse_pos, test_panel->bounds.min_pos, test_panel->bounds.max_pos))
            {
                active_panel = test_panel;
                break;
            }
        }

        if (!active_panel)
        {
            if (panel != panel_sentinel->prev)
            {
                panel->prev->next = panel->next;
                panel->next->prev = panel->prev;

                dllist_insert_last(panel_sentinel, panel);
            }

            ui->active_panel = panel;
        }
    }

    // NOTE(dan): handle resize
    if (panel->flags & PanelFlag_ResizableX || panel->flags & PanelFlag_ResizableY)
    {
        rect2 scaler_bb = r2(vec2_sub(panel->bounds.max_pos, ui->panel_padding), panel->bounds.max_pos);
        u32 color = ui->colors[UIColor_PanelBorder];

        vec2 vertices[] =
        {
            v2(scaler_bb.max_pos.x, scaler_bb.min_pos.y),
            v2(scaler_bb.min_pos.x, scaler_bb.max_pos.y),
            v2(scaler_bb.max_pos.x, scaler_bb.max_pos.y),
        };
        add_poly_filled(ui, vertices, array_count(vertices), color);

        if (is_mouse_clicked_in_rect(ui, mouse_button_left, scaler_bb))
        {
            vec2 delta_mouse = ui->delta_mouse_pos;
            vec2 min_panel_size = v2(40, 30);
            vec2 panel_size = rect2_dim(panel->bounds);

            if (panel->flags & PanelFlag_ResizableX)
            {
                if (panel_size.x + delta_mouse.x >= min_panel_size.x)
                {
                    if ((delta_mouse.x < 0) || (delta_mouse.x > 0 && ui->mouse_pos.x >= scaler_bb.min_pos.x))
                    {
                        panel->bounds.max_pos.x += delta_mouse.x;
                        scaler_bb.min_pos.x += delta_mouse.x;
                        scaler_bb.max_pos.x += delta_mouse.x;
                    }
                }
            }

            if (panel->flags & PanelFlag_ResizableY)
            {
                if (min_panel_size.y < panel_size.y + ui->delta_mouse_pos.y)
                {
                    if ((delta_mouse.y < 0) || (delta_mouse.y > 0 && ui->mouse_pos.y >= scaler_bb.min_pos.y))
                    {
                        panel->bounds.max_pos.y += delta_mouse.y;
                        scaler_bb.min_pos.y += delta_mouse.y;
                        scaler_bb.max_pos.y += delta_mouse.y;
                    }
                }
            }

            vec2 scaler_center = vec2_add(scaler_bb.min_pos, vec2_mul(0.5f, rect2_dim(scaler_bb)));
            ui->clicked_at = scaler_center;
        }
    }

    return panel;
}

inline void end_panel(UIState *ui)
{
    Panel *panel = ui->current_panel;

    panel->num_vertices = ui->num_vertices - panel->begin_vertex_index;
    panel->num_elements = ui->num_elements - panel->begin_element_index;
    
    ui->current_panel = 0;
}

enum ButtonState
{
    ButtonState_Inactive,

    ButtonState_Hover,
    ButtonState_Active,
    ButtonState_LeftClick,
};

static ButtonState button_behavior(UIState *ui, rect2 bounds)
{
    ButtonState state = ButtonState_Inactive;

    b32 hovered = is_mouse_hovered_rect(ui, bounds);
    if (hovered)
    {
        state = ButtonState_Hover;
        if (is_mouse_down(ui, mouse_button_left))
        {
            state = ButtonState_Active;
            if (is_mouse_clicked_in_rect(ui, mouse_button_left, bounds))
            {
                state = ButtonState_LeftClick;
            }
        }
    }
    return state;
}

static b32 button(UIState *ui, char *name)
{
    Panel *panel = ui->current_panel;
    vec2 padding = v2(ui->menu_bar_button_padding_x, 0.0f);

    vec2 menu_bar_size = vec2_sub(panel->bounds.max_pos, panel->bounds.min_pos);
    vec2 text_size = calc_text_size(ui, name, ui->current_font.size);
    vec2 button_size = vec2_add(text_size, vec2_mul(2.0f, padding));

    vec2 min_pos = panel->layout_at;
    vec2 max_pos = vec2_add(min_pos, v2(button_size.x, menu_bar_size.y));

    vec2 text_pos = vec2_add(min_pos, padding);
    text_pos.y += (menu_bar_size.y - ui->current_font.size) / 2;
    
    ButtonState state = button_behavior(ui, r2(min_pos, max_pos));

    u32 text_color = ui->colors[UIColor_Text];
    u32 background_color = ui->colors[UIColor_ButtonBackground];
    if (state == ButtonState_Active || state == ButtonState_LeftClick)
    {
        background_color = ui->colors[UIColor_ButtonBackgroundActive];
    }
    else if (state == ButtonState_Hover)
    {
        background_color = ui->colors[UIColor_ButtonBackgroundHover];
    }

    add_rect_filled(ui, min_pos, max_pos, background_color);
    add_text(ui, name, text_pos, ui->current_font.size, text_color);

    panel->layout_at.x += button_size.x;

    b32 pressed = (state == ButtonState_LeftClick);
    return pressed;
}

inline void newline(UIState *ui)
{
    Panel *panel = ui->current_panel;
    panel->layout_at.x = panel->layout.min_pos.x;
    panel->layout_at.y += panel->current_line_height;
}

inline void text_out(UIState *ui, char *text)
{
    Panel *panel = ui->current_panel;
    u32 color = ui->colors[UIColor_Text];

    vec2 text_size = add_text(ui, text, panel->layout_at, ui->current_font.size, color);
    panel->layout_at.x += text_size.x;
    panel->current_line_height = max(panel->current_line_height, text_size.y);
}

inline void textf_out(UIState *ui, char *format, ...)
{
    char text_buffer[256];
    param_list params = get_params_after(format);
    format_string_vararg(text_buffer, sizeof(text_buffer), format, params);
    text_out(ui, text_buffer);
}
