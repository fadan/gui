
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

    new_dc->min_pos.y = prev_dc->max_pos.y + 1;
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
    Font *font = &ui->default_font;

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

inline Panel *push_panel(UIState *ui)
{
    Panel *panel = 0;
    allocate_freelist(panel, ui->first_free_panel, push_struct(&ui->memory, Panel));

    *panel = {0};
    panel->size = v2(-1.0f, -1.0f);
    panel->status = PanelStatus_Float;
    panel->active = true;
    panel->is_first = true;

    panel->next = ui->first_panel;
    ui->first_panel = panel;
    return panel;
}

inline vec2 get_ui_size(UIState *ui)
{
    assert(ui->num_draw_contexts > 0);

    DrawContext *dc = ui->draw_contexts;
    vec2 ui_size = dc->max_pos;
    return ui_size;
}

static void set_panel_active(Panel *panel)
{
    panel->active = true;

    for (Panel *prev_panel = panel->prev_panel; prev_panel; prev_panel = prev_panel->prev_panel)
    {
        prev_panel->active = false;
    }
    for (Panel *next_panel = panel->next_panel; next_panel; next_panel = next_panel->next_panel)
    {
        next_panel->active = false;
    }
}

static Panel *get_or_create_panel(UIState *ui, char *name, b32 opened)
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
        panel = push_panel(ui);
        panel->id = id;
        panel->pos = v2(0.0f, 0.0f);
        panel->size = get_ui_size(ui);
        panel->opened = opened;
        panel->is_first = true;

        copy_string_and_null_terminate(name, panel->name, array_count(panel->name));
        set_panel_active(panel);
    }
    return panel;
}

static Panel *get_root_panel(UIState *ui)
{
    Panel *root_panel = 0;
    for (Panel *panel = ui->first_panel; panel; panel = panel->next)
    {
        if (!panel->parent && (panel->status == PanelStatus_Docked || panel->children[0]))
        {
            root_panel = panel;
            break;
        }
    }
    return root_panel;
}

static void undock_panel(UIState *ui, Panel *panel)
{
    if (panel->prev_panel)
    {
        set_panel_active(panel->prev_panel);
    }
    else if (panel->next_panel)
    {
        set_panel_active(panel->next_panel);
    }
    else
    {
        panel->active = true;
    }

    Panel *container = panel->parent;
    if (container)
    {
        // TODO(dan): remove from container
    }

    if (panel->prev_panel)
    {
        panel->prev_panel->next_panel = panel->next_panel;
    }
    if (panel->next_panel)
    {
        panel->next_panel->prev_panel = panel->prev_panel;
    }

    panel->parent = 0;
    panel->next_panel = 0;
    panel->prev_panel = 0;
}

inline b32 is_panel_container(Panel *panel)
{
    b32 is_container = (panel->children[0] != 0);
    return is_container;
}

static Panel *get_panel_at_mouse(UIState *ui)
{
    Panel *panel = 0;
    for (Panel *test_panel = ui->first_panel; test_panel; test_panel = test_panel->next)
    {
        if (!is_panel_container(test_panel) && (test_panel->status == PanelStatus_Docked))
        {
            if (mouse_intersect_rect(ui, test_panel->pos, vec2_add(test_panel->pos, test_panel->size)))
            {
                panel = test_panel;
                break;
            }
        }
    }
    return panel;
}

static void set_panel_children_pos_and_size(Panel *panel, vec2 pos, vec2 size);

static void set_panel_pos_and_size(Panel *panel, vec2 pos, vec2 size)
{
    panel->pos = pos;
    panel->size = size;

    for (Panel *prev_panel = panel->prev_panel; prev_panel; prev_panel = prev_panel->prev_panel)
    {
        prev_panel->pos = pos;
        prev_panel->size = size;
    }
    for (Panel *next_panel = panel->next_panel; next_panel; next_panel = next_panel->next_panel)
    {
        next_panel->pos = pos;
        next_panel->size = size;
    }

    if (is_panel_container(panel))
    {
        set_panel_children_pos_and_size(panel, pos, size);
    }
}

inline b32 is_panel_horizontal(Panel *panel)
{
    b32 is_horizontal = (panel->children[0]->pos.x <panel->children[1]->pos.x);
    return is_horizontal;
}

inline vec2 get_panel_min_size(Panel *panel)
{
    // TODO(dan): get line height + spacing
    vec2 min_size = v2(16.0f, 16.0f + 13.0f);
    if (panel->children[0])
    {
        vec2 child_0_size = get_panel_min_size(panel->children[0]);
        vec2 child_1_size = get_panel_min_size(panel->children[1]);

        if (is_panel_horizontal(panel))
        {
            min_size = v2(child_0_size.x + child_1_size.x, max(child_0_size.y, child_1_size.y));
        }
        else
        {
            min_size = v2(max(child_0_size.x, child_1_size.x), child_0_size.y + child_1_size.y);
        }
    }
    return min_size;
}

static void set_panel_children_pos_and_size(Panel *panel, vec2 pos, vec2 size)
{
    vec2 children_size = panel->children[0]->size;

    // TODO(dan): pull out children min sizes
    if (is_panel_horizontal(panel))
    {
        children_size.x = (f32)((i32)(size.x * panel->children[0]->size.x / (panel->children[0]->size.x + panel->children[1]->size.x)));
        children_size.y = size.y;

        if (children_size.x < get_panel_min_size(panel->children[0]).x)
        {
            children_size.x = get_panel_min_size(panel->children[0]).x;
        }
        else if ((size.x - children_size.x) < get_panel_min_size(panel->children[1]).x)
        {
            children_size.x = size.x - get_panel_min_size(panel->children[1]).x;
        }

        set_panel_pos_and_size(panel->children[0], pos, children_size);

        vec2 children_pos = pos;
        children_size.x = size.x - panel->children[0]->size.x;
        children_pos.x += panel->children[0]->size.x;

        set_panel_pos_and_size(panel->children[1], children_pos, children_size);
    }
    else
    {
        children_size.x = size.x;
        children_size.y = (f32)((i32)(size.y * panel->children[0]->size.y / (panel->children[0]->size.y + panel->children[1]->size.y)));

        if (children_size.y < get_panel_min_size(panel->children[0]).y)
        {
            children_size.y = get_panel_min_size(panel->children[0]).y;
        }
        else if ((size.y - children_size.y) < get_panel_min_size(panel->children[1]).y)
        {
            children_size.y = size.y - get_panel_min_size(panel->children[1]).y;
        }

        set_panel_pos_and_size(panel->children[0], pos, children_size);

        vec2 children_pos = pos;
        children_size.y = size.y - panel->children[0]->size.y;
        children_pos.y += panel->children[0]->size.y;

        set_panel_pos_and_size(panel->children[1], children_pos, children_size);
    }
}

static void dock_panel(UIState *ui, Panel *panel, Panel *container, PanelSlot slot)
{
    assert(!panel->parent);

    if (!container)
    {
        // NOTE(dan): full screen panel
        panel->status = PanelStatus_Docked;

        vec2 pos = v2(0.0f, 0.0f);
        vec2 ui_size = get_ui_size(ui);
        set_panel_pos_and_size(panel, pos, ui_size);
    }

    set_panel_active(panel);
}

static b32 dock_panel_to_container(UIState *ui, Panel *panel, Panel *container, vec2 min_pos, vec2 max_pos, b32 on_border)
{
    vec2 size = vec2_sub(max_pos, min_pos);
    vec2 center = vec2_add(min_pos, vec2_mul(0.5f, size));

    b32 docked = false;
    for (i32 slot_index = 0; slot_index < (on_border ? 4 : 5); ++slot_index)
    {
        vec2 rect_min_pos = v2(0.0f, 0.0f);
        vec2 rect_max_pos = v2(0.0f, 0.0f);

        if (on_border)
        {
            switch ((PanelSlot)slot_index)
            {
                case PanelSlot_Top:
                {
                    rect_min_pos = v2(center.x - 20.0f, min_pos.y + 10.0f);
                    rect_max_pos = v2(center.x + 20.0f, min_pos.y + 30.0f);
                } break;
                case PanelSlot_Left:
                {
                    rect_min_pos = v2(min_pos.x + 10.0f, center.y - 20.0f);
                    rect_max_pos = v2(min_pos.x + 30.0f, center.y + 20.0f);
                } break;
                case PanelSlot_Bottom:
                {
                    rect_min_pos = v2(center.x - 20.0f, max_pos.y - 30.0f);
                    rect_max_pos = v2(center.x + 20.0f, max_pos.y - 10.0f);
                } break;
                case PanelSlot_Right:
                {
                    rect_min_pos = v2(max_pos.x - 30.0f, center.y - 20.0f);
                    rect_max_pos = v2(max_pos.x - 10.0f, center.y + 20.0f);
                } break;
                invalid_default_case;
            }
        }
        else
        {
            switch ((PanelSlot)slot_index)
            {
                case PanelSlot_Top:
                {
                    rect_min_pos = vec2_add(center, v2(-20.0f, -50.0f));
                    rect_max_pos = vec2_add(center, v2( 20.0f, -30.0f));
                } break;
                case PanelSlot_Left:
                {
                    rect_min_pos = vec2_add(center, v2(-50.0f, -20.0f));
                    rect_max_pos = vec2_add(center, v2(-30.0f,  20.0f));
                } break;
                case PanelSlot_Bottom:
                {
                    rect_min_pos = vec2_add(center, v2(-20.0f, 30.0f));
                    rect_max_pos = vec2_add(center, v2( 20.0f, 50.0f));
                } break;
                case PanelSlot_Right:
                {
                    rect_min_pos = vec2_add(center, v2(30.0f, -20.0f));
                    rect_max_pos = vec2_add(center, v2(50.0f,  20.0f));
                } break;
                default:
                {
                    rect_min_pos = vec2_sub(center, v2(20.0f, 20.0f));
                    rect_max_pos = vec2_add(center, v2(20.0f, 20.0f));
                }
            }
        }

        b32 hovered = mouse_intersect_rect(ui, rect_min_pos, rect_max_pos);
        u32 color = (hovered) ? 0xFFfc8d45 : 0xFFfc7825;
        add_rect_filled(ui, rect_min_pos, rect_max_pos, color);

        if (hovered)
        {
            if (!ui->input->mouse_buttons[mouse_button_left].down)
            {
                Panel *to = container ? container : get_root_panel(ui);
                dock_panel(ui, panel, to, (PanelSlot)slot_index);

                docked = true;
                break;
            }

            vec2 dock_min_pos = v2(0.0f, 0.0f);
            vec2 dock_max_pos = v2(0.0f, 0.0f);
            vec2 half_size = vec2_mul(0.5f, vec2_sub(max_pos, min_pos));

            switch ((PanelSlot)slot_index)
            {
                case PanelSlot_Top:
                {
                    dock_min_pos = min_pos;
                    dock_max_pos = vec2_add(min_pos, v2(max_pos.x, half_size.y));
                } break;
                case PanelSlot_Left:
                {
                    dock_min_pos = min_pos;
                    dock_max_pos = vec2_add(min_pos, v2(half_size.x, max_pos.y));
                } break;
                case PanelSlot_Bottom:
                {
                    dock_min_pos = vec2_add(min_pos, v2(0, half_size.y));
                    dock_max_pos = max_pos;
                } break;
                case PanelSlot_Right:
                {
                    dock_min_pos = vec2_add(min_pos, v2(half_size.x, 0));
                    dock_max_pos = max_pos;
                } break;
            }

            add_rect_filled(ui, dock_min_pos, dock_max_pos, 0xFFfb7926);
        }

    }
    return docked;
}

static b32 begin_panel(UIState *ui, char *name, b32 *opened)
{
    Panel *panel = get_or_create_panel(ui, name, !opened || *opened);

    // TODO(dan): store position when not opened

    panel->last_frame = ui->frame_index;
    ui->panel_end_action = PanelEndAction_None;

    if (panel->is_first && opened)
    {
        *opened = panel->opened;
    }
    panel->is_first = false;

    // TODO(dan): undock when not opened

    b32 result = false;
    {
        panel->opened = true;

        // NOTE(dan): begin panel
        ui->panel_end_action = PanelEndAction_Panel;
        {
            vec2 ui_size = get_ui_size(ui);
            Panel *root_panel = get_root_panel(ui);
            DrawContext *dc = push_draw_context(ui);

            if (!root_panel)
            {
                dc->min_pos = v2(0.0f, 30.0f);
                dc->max_pos = ui_size;
            }
            else
            {
                vec2 scale = v2(ui_size.x / root_panel->size.x, ui_size.y / root_panel->size.y);
                vec2 scaled_pos = vec2_hadamard(scale, root_panel->pos);
                vec2 scaled_size = vec2_hadamard(scale, root_panel->size);

                dc->min_pos = scaled_pos;
                dc->max_pos = vec2_add(dc->min_pos, scaled_size);
            }

            // TODO(dan): handle splits
            // TODO(dan): undock when we dont have a container and we are docked
        }

        ui->current_panel = panel;

        if (panel->status == PanelStatus_Dragged)
        {
            DrawContext *dc = push_draw_context(ui);
            {
                panel->pos = v2(ui->input->mouse_pos[0] - ui->panel_drag_offset.x, ui->input->mouse_pos[1] - ui->panel_drag_offset.y);

                Panel *container = get_panel_at_mouse(ui);
                if (container)
                {
                    // TODO(dan): dock into container
                    vec2 min_pos = container->pos;
                    vec2 max_pos = vec2_add(container->pos, container->size);

                    dock_panel_to_container(ui, panel, container, min_pos, max_pos, false);
                }
                else
                {
                    // TODO(dan): fullscreen panel without the menubar, storage for this?
                    vec2 min_pos = v2(0.0f, 25.0f);
                    vec2 max_pos = get_ui_size(ui);

                    if (!dock_panel_to_container(ui, panel, container, min_pos, max_pos, true))
                    {
                        add_rect_filled(ui, panel->pos, vec2_add(panel->pos, panel->size), 0x80FFFFFF);

                        if (!ui->input->mouse_buttons[mouse_button_left].down)
                        {
                            panel->status = PanelStatus_Float;
                            panel->location[0] = 0;

                            set_panel_active(panel);
                        }
                    }
                }
            }
            pop_draw_context(ui);
        }

        if (panel->status == PanelStatus_Float)
        {
            DrawContext *dc = push_draw_context(ui);
            dc->min_pos = panel->pos;
            dc->max_pos = vec2_add(dc->min_pos, panel->size);

            ui->panel_end_action = PanelEndAction_End;

            if (mouse_intersect_rect(ui, dc->min_pos, dc->max_pos) && ui->input->mouse_buttons[mouse_button_left].down)
            {
                ui->panel_drag_offset = v2(ui->input->mouse_pos[0] - panel->pos.x, ui->input->mouse_pos[1] - panel->pos.y);

                undock_panel(ui, panel);

                panel->status = PanelStatus_Dragged;
            }
            result = true;
        }
    }
    return result;
}

static void end_panel(UIState *ui)
{
    if (ui->panel_end_action == PanelEndAction_End)
    {
        pop_draw_context(ui);
    }

    ui->current_panel = 0;

    if (ui->panel_end_action > PanelEndAction_None)
    {
        pop_draw_context(ui);
    }
}
