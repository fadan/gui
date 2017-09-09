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

inline b32 is_mouse_clicked(UIState *ui, MouseButtonID button_id)
{
    b32 clicked = (ui->input->mouse_buttons[button_id].released);
    return clicked;
}

inline b32 is_mouse_clicked_in_rect(UIState *ui, MouseButtonID button_id, vec2 min_pos, vec2 max_pos)
{
    b32 clicked_in_rect = (is_mouse_clicked(ui, button_id) && vec2_intersect(ui->clicked_at, min_pos, max_pos));
    return clicked_in_rect;
}

inline b32 is_mouse_hovered_rect(UIState *ui, vec2 min_pos, vec2 max_pos)
{
    b32 hovered_rect = (vec2_intersect(ui->mouse_pos, min_pos, max_pos));
    return hovered_rect;
}

inline b32 was_mouse_hovered_rect(UIState *ui, vec2 min_pos, vec2 max_pos)
{
    b32 was_hovered_rect = (vec2_intersect(ui->prev_mouse_pos, min_pos, max_pos));
    return was_hovered_rect;
}

static Panel *get_or_create_panel(UIState *ui, char *name)
{
    u32 id = djb2_hash(name);

    Panel *panel = 0;
    for (u32 panel_index = 0; panel_index < ui->num_panels; ++panel_index)
    {
        Panel *test_panel = ui->panels + panel_index;
        if (test_panel->id == id)
        {
            panel = test_panel;
            break;
        }        
    }

    if (!panel)
    {
        assert(ui->num_panels < array_count(ui->panels));
        panel = ui->panels + ui->num_panels++;

        *panel = {0};
        panel->id = id;
        panel->status = PanelStatus_Float;
    }

    return panel;
}

static Panel *begin_panel(UIState *ui, char *name, vec2 default_pos = v2(30.0f, 30.0f), vec2 default_size = v2(300.0f, 200.0f))
{
    Panel *panel = get_or_create_panel(ui, name);

    if ((panel->min_pos.x == panel->max_pos.x) && (panel->max_pos.y == panel->max_pos.y))
    {
        panel->min_pos = default_pos;
        panel->max_pos = vec2_add(panel->min_pos, default_size);
    }
    
    panel->at = panel->min_pos;

    ui->current_panel = panel;

    return panel;
}

inline void end_panel(UIState *ui)
{
}

static void begin_menu_bar(UIState *ui, f32 menu_bar_height)
{
    Panel *panel = begin_panel(ui, "menu bar");
    f32 bottom_border_height = 2.0f;

    panel->min_pos = ui->min_pos;
    panel->max_pos = v2(ui->max_pos.x, ui->min_pos.y + menu_bar_height);

    u32 background_color = ui->colors[UIColor_MenuBarBackground];
    // u32 menu_bar_border_color = ui->colors[UIColor_MenuBarBorder];

    add_rect_filled(ui, panel->min_pos, panel->max_pos, background_color);
    // add_rect_filled(ui, v2(panel->min_pos.x, panel->max_pos.y - bottom_border_height), panel->max_pos, menu_bar_border_color);
}

enum ButtonState
{
    ButtonState_Inactive,

    ButtonState_Hover,
    ButtonState_Active,
    ButtonState_LeftClick,
};

static ButtonState button_behavior(UIState *ui, vec2 min_pos, vec2 max_pos)
{
    ButtonState state = ButtonState_Inactive;

    b32 hovered = is_mouse_hovered_rect(ui, min_pos, max_pos);
    if (hovered)
    {
        state = ButtonState_Hover;
        if (is_mouse_down(ui, mouse_button_left))
        {
            state = ButtonState_Active;
            if (is_mouse_clicked_in_rect(ui, mouse_button_left, min_pos, max_pos))
            {
                state = ButtonState_LeftClick;
            }
        }
    }
    return state;
}

static b32 menu_bar_button(UIState *ui, char *name)
{
    Panel *panel = ui->current_panel;
    vec2 padding = ui->menu_bar_padding;

    vec2 menu_bar_size = vec2_sub(panel->max_pos, panel->min_pos);
    vec2 text_size = calc_text_size(ui, name, ui->current_font.size);
    vec2 button_size = vec2_add(text_size, vec2_mul(2.0f, padding));

    vec2 min_pos = panel->at;
    vec2 max_pos = vec2_add(min_pos, v2(button_size.x, menu_bar_size.y));

    vec2 text_pos = vec2_add(min_pos, padding);
    text_pos.y += (menu_bar_size.y - ui->current_font.size) / 2;
    
    ButtonState state = button_behavior(ui, min_pos, max_pos);

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

    panel->at.x += button_size.x;

    b32 pressed = (state == ButtonState_LeftClick);
    return pressed;
}

inline void end_menu_bar(UIState *ui)
{
    end_panel(ui);
}
