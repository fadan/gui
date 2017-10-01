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

inline b32 is_mouse_down_in_rect(UIState *ui, MouseButtonID button_id, rect2 rect)
{
    b32 clicked_in_rect = (ui->input->mouse_buttons[button_id].down &&
                           vec2_intersect(ui->clicked_at, rect.min_pos, rect.max_pos));
    return clicked_in_rect;
}

inline b32 is_mouse_clicked_in_rect(UIState *ui, MouseButtonID button_id, rect2 rect)
{
    b32 clicked_in_rect = (ui->input->mouse_buttons[button_id].pressed &&
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
        *panel = {0};

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
    panel->flags = flags;

    panel->begin_element_index = ui->num_elements;

    // NOTE(dan): panel overlaps
    if (is_mouse_clicked_in_rect(ui, mouse_button_left, panel->bounds))
    {
        Panel *panel_sentinel = &ui->panel_sentinel;
        Panel *active_panel = 0;
        for (Panel *test_panel = panel->next; test_panel != panel_sentinel; test_panel = test_panel->next)
        {
            if (!(panel->flags & PanelFlag_Hidden) && vec2_intersect(ui->mouse_pos, test_panel->bounds.min_pos, test_panel->bounds.max_pos))
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
    ui->current_panel = panel;

    // NOTE(dan): dragable area
    rect2 header_bb = r2(panel->bounds.min_pos, panel->bounds.max_pos);
    if (panel->flags & PanelFlag_Movable)
    {
        if (panel->flags & PanelFlag_HasHeader)
        {
            header_bb.max_pos.y = header_bb.min_pos.y + ui->current_font.size + 2 * ui->panel_header_padding.y;
        }

        if (is_mouse_down_in_rect(ui, mouse_button_left, header_bb) && ui->active_panel == panel)
        {
            panel->bounds = rect2_offset(panel->bounds, ui->delta_mouse_pos);
            ui->clicked_at = vec2_add(ui->clicked_at, ui->delta_mouse_pos);
        }
    }
    vec2 header_dim = rect2_dim(header_bb);

    panel->layout.min_pos = vec2_add(panel->bounds.min_pos, ui->panel_padding);
    panel->layout.max_pos = vec2_sub(panel->bounds.max_pos, ui->panel_padding);
    panel->layout_at = panel->layout.min_pos;
    panel->layout_max = panel->layout.min_pos;
    panel->current_line_height = 0;

    // NOTE(dan): resizing
    rect2 scaler_bb = r2(vec2_sub(panel->bounds.max_pos, ui->panel_padding), panel->bounds.max_pos);
    if (panel->flags & PanelFlag_ResizableX || panel->flags & PanelFlag_ResizableY)
    {
        if (is_mouse_down_in_rect(ui, mouse_button_left, scaler_bb))
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

    // NOTE(dan): draw header
    if (panel->flags & PanelFlag_HasHeader)
    {
        rect2 bb = r2(panel->bounds.min_pos, panel->bounds.max_pos);
        if (panel->flags & PanelFlag_Movable)
        {
            bb.max_pos.y = bb.min_pos.y + ui->current_font.size + 2 * ui->panel_header_padding.y;
        }

        u32 background_color = ui->colors[UIColor_PanelHeaderBackground];
        u32 text_color       = ui->colors[UIColor_Text];
        vec2 text_pos = vec2_add(panel->bounds.min_pos, ui->panel_header_padding);

        add_rect_filled(ui, bb.min_pos, bb.max_pos, background_color);
        add_text(ui, name, text_pos, ui->current_font.size, text_color);

        panel->layout_at.y += header_dim.y;
        panel->layout.min_pos.y += header_dim.y;
        panel->layout_max.y = max(panel->layout_max.y, panel->layout_at.y);
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

    // NOTE(dan): drwa panel border
    if (panel->flags & PanelFlag_Bordered)
    {
        rect2 bounds = panel->bounds;
        u32 color = ui->colors[UIColor_PanelBorder];

        add_rect_outline(ui, bounds.min_pos, bounds.max_pos, color);
    }

    // NOTE(dan): draw scaler
    if (panel->flags & PanelFlag_ResizableX || panel->flags & PanelFlag_ResizableY)
    {
        u32 color = ui->colors[UIColor_PanelBorder];
        vec2 vertices[] =
        {
            v2(scaler_bb.max_pos.x, scaler_bb.min_pos.y),
            v2(scaler_bb.min_pos.x, scaler_bb.max_pos.y),
            v2(scaler_bb.max_pos.x, scaler_bb.max_pos.y),
        };
        add_poly_filled(ui, vertices, array_count(vertices), color);
    }
    return panel;
}

inline void end_panel(UIState *ui)
{
    Panel *panel = ui->current_panel;

    panel->num_elements = ui->num_elements - panel->begin_element_index;
    
    ui->current_panel = panel->parent;
}

static Behavior button_behavior(UIState *ui, rect2 bounds)
{
    Behavior state = Behavior_Inactive;

    if (is_mouse_hovered_rect(ui, bounds))
    {
        state = Behavior_Hover;
        if (ui->active_panel == ui->current_panel)
        {
            if (is_mouse_down(ui, mouse_button_left))
            {
                state = Behavior_Active;
            }

            if (is_mouse_clicked_in_rect(ui, mouse_button_left, bounds))
            {
                state = Behavior_LeftClick;
            }
        }
    }
    return state;
}

static b32 button(UIState *ui, char *name)
{
    Panel *panel = ui->current_panel;
    vec2 padding = ui->button_padding;
    vec2 text_size = calc_text_size(ui, name, ui->current_font.size);
    vec2 button_size = vec2_add(text_size, vec2_mul(2.0f, padding));

    rect2 bounds = r2(panel->layout_at, vec2_add(panel->layout_at, button_size));
    Behavior behavior = button_behavior(ui, bounds);
    
    u32 background_color = ui->colors[UIColor_ButtonBackground];
    if (behavior == Behavior_Active || behavior == Behavior_LeftClick)
    {
        background_color = ui->colors[UIColor_ButtonBackgroundActive];
    }
    else if (behavior == Behavior_Hover)
    {
        background_color = ui->colors[UIColor_ButtonBackgroundHover];
    }
    add_rect_filled(ui, bounds.min_pos, bounds.max_pos, background_color);
    
    u32 text_color = ui->colors[UIColor_Text];
    vec2 text_pos = vec2_add(bounds.min_pos, padding);
    add_text(ui, name, text_pos, ui->current_font.size, text_color);

    panel->current_line_height = max(panel->current_line_height, button_size.y);
    panel->layout_at.x += button_size.x;
    panel->layout_max.x = max(panel->layout_max.x, bounds.max_pos.x);
    panel->layout_max.y = max(panel->layout_max.y, bounds.max_pos.y);

    b32 pressed = (behavior == Behavior_LeftClick);
    return pressed;
}

inline Panel *begin_menu_bar(UIState *ui)
{
    push_style_vec2(ui, panel_padding, ui->menu_bar_padding);
    push_style_vec2(ui, button_padding, ui->menu_bar_button_padding);
    push_style_color(ui, UIColor_PanelBackground,  ui->colors[UIColor_PanelHeaderBackground]);
    push_style_color(ui, UIColor_ButtonBackground, ui->colors[UIColor_PanelHeaderBackground]);

    Panel *panel = begin_panel(ui, "Menu Bar");
    panel->bounds.min_pos = v2(0.0f, 0.0f);
    panel->bounds.max_pos = v2(ui->max_pos.x - ui->min_pos.x, ui->menu_bar_height);
    return panel;
}

inline void end_menu_bar(UIState *ui)
{
    pop_style(ui);
    pop_style(ui);
    pop_style(ui);
    pop_style(ui);
    
    end_panel(ui);
}

inline void set_panel_topmost(UIState *ui, Panel *panel)
{
    Panel *panel_sentinel = &ui->panel_sentinel;
    if (panel != panel_sentinel->prev)
    {
        panel->prev->next = panel->next;
        panel->next->prev = panel->prev;

        dllist_insert_last(panel_sentinel, panel);
    }
}

static void remove_panel(UIState *ui, Panel *panel)
{
    panel->prev->next = panel->next;
    panel->next->prev = panel->prev;

    deallocate_freelist(panel, ui->first_free_panel);
}

static b32 begin_menu(UIState *ui, char *name)
{
    Panel *panel = ui->current_panel;

    vec2 padding = ui->button_padding;
    vec2 text_size = calc_text_size(ui, name, ui->current_font.size);
    vec2 button_size = vec2_add(text_size, vec2_mul(2.0f, padding));

    rect2 bounds = r2(panel->layout_at, vec2_add(panel->layout_at, button_size));
    Behavior behavior = button_behavior(ui, bounds);
    
    u32 background_color = ui->colors[UIColor_ButtonBackground];
    if (behavior == Behavior_Active || behavior == Behavior_LeftClick)
    {
        background_color = ui->colors[UIColor_ButtonBackgroundActive];
    }
    else if (behavior == Behavior_Hover)
    {
        background_color = ui->colors[UIColor_ButtonBackgroundHover];
    }
    add_rect_filled(ui, bounds.min_pos, bounds.max_pos, background_color);
    
    u32 text_color = ui->colors[UIColor_Text];
    vec2 text_pos = vec2_add(bounds.min_pos, padding);
    add_text(ui, name, text_pos, ui->current_font.size, text_color);

    panel->layout_at.x += button_size.x;
    panel->layout_max.x = max(panel->layout_max.x, bounds.max_pos.x);
    panel->layout_max.y = max(panel->layout_max.y, bounds.max_pos.y);

    ui->next_panel_pos = vec2_add(bounds.min_pos, v2(0.0f, button_size.y));
    ui->next_panel_size = v2(button_size.x, button_size.y);

    char child_name[32];
    format_string(child_name, array_count(child_name), "%s/child", name);

    Panel *child = get_or_create_panel(ui, child_name);
    if (behavior == Behavior_LeftClick)
    {
        if (panel->child == child)
        {
            if (child->flags & PanelFlag_Hidden)
            {
                child->flags &= ~PanelFlag_Hidden;
            }
            else
            {
                child->flags |= PanelFlag_Hidden;
            }
        }
        else
        {
            panel->child = child;
            panel->child->flags &= ~PanelFlag_Hidden;
        }
    }
    else if (behavior == Behavior_Hover && panel->child != 0 && !(panel->child->flags & PanelFlag_Hidden))
    {
        if (panel->child != child)
        {
            panel->child = child;
            panel->child->flags &= ~PanelFlag_Hidden;
        }
    }

    if (panel->child == child && !(child->flags & PanelFlag_Hidden))
    {
        begin_panel(ui, child_name, panel->child->flags);
        set_panel_topmost(ui, child);
        child->parent = panel;
    }

    b32 result = (panel->child == child && !(child->flags & PanelFlag_Hidden));
    return result;
}

inline void end_menu(UIState *ui)
{
    Panel *panel = ui->current_panel;
    if (panel->parent && panel->parent->child == panel && !(panel->flags & PanelFlag_Hidden))
    {
        if (ui->input->mouse_buttons[mouse_button_left].pressed && 
            !vec2_intersect(ui->mouse_pos, vec2_sub(panel->bounds.min_pos, v2(0.0f, panel->current_line_height)), panel->bounds.max_pos))
        {
            panel->flags |= PanelFlag_Hidden;
        }

        panel->bounds.max_pos = panel->layout_max;
        end_panel(ui);
    }
}

inline void newline(UIState *ui)
{
    Panel *panel = ui->current_panel;
    panel->layout_at.x = panel->layout.min_pos.x;
    panel->layout_at.y += panel->current_line_height;

    panel->layout_max.y = max(panel->layout_max.y, panel->layout_at.y);
}

static b32 menu_button(UIState *ui, char *name)
{
    b32 result = button(ui, name);
    Panel *panel = ui->current_panel;
    if (result)
    {
        panel->flags |= PanelFlag_Hidden;
    }
    newline(ui);
    return result;
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
