
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
        vec2 uv = ui->current_font.white_pixel_uv;

        vertex[0].pos.x = current_pos.x + delta_pos.y;
        vertex[0].pos.y = current_pos.y - delta_pos.x;
        vertex[0].uv = uv;
        vertex[0].color = color;

        vertex[1].pos.x = next_pos.x + delta_pos.y;
        vertex[1].pos.y = next_pos.y - delta_pos.x;
        vertex[1].uv = uv;
        vertex[1].color = color;

        vertex[2].pos.x = next_pos.x - delta_pos.y;
        vertex[2].pos.y = next_pos.y + delta_pos.x;
        vertex[2].uv = uv;
        vertex[2].color = color;

        vertex[3].pos.x = current_pos.x - delta_pos.y;
        vertex[3].pos.y = current_pos.y + delta_pos.x;
        vertex[3].uv = uv;
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

static void add_textured_quad(UIState *ui, vec2 top_left_corner, vec2 bottom_right_corner, 
                              vec2 top_left_corner_uv, vec2 bottom_right_corner_uv, u32 color)
{
    u32 vertice_index = ui->num_vertices;

    assert(ui->num_vertices + 4 < MAX_NUM_VERTICES);
    assert(ui->num_elements + 6 < MAX_NUM_ELEMENTS);

    Vertex *vertices = ui->vertices + ui->num_vertices;
    GLuint *elements = ui->elements + ui->num_elements;

    vertices[0].pos = v2(top_left_corner.x, top_left_corner.y);
    vertices[0].uv = v2(top_left_corner_uv.u, top_left_corner_uv.v);
    vertices[0].color = color;

    vertices[1].pos = v2(top_left_corner.x, bottom_right_corner.y);
    vertices[1].uv = v2(top_left_corner_uv.u, bottom_right_corner_uv.v);
    vertices[1].color = color;

    vertices[2].pos = v2(bottom_right_corner.x, bottom_right_corner.y);
    vertices[2].uv = v2(bottom_right_corner_uv.u, bottom_right_corner_uv.v);
    vertices[2].color = color;

    vertices[3].pos = v2(bottom_right_corner.x, top_left_corner.y);
    vertices[3].uv = v2(bottom_right_corner_uv.u, top_left_corner_uv.v);
    vertices[3].color = color;

    elements[0] = vertice_index + 0;
    elements[1] = vertice_index + 1;
    elements[2] = vertice_index + 2;
    elements[3] = vertice_index + 0;
    elements[4] = vertice_index + 2;
    elements[5] = vertice_index + 3;

    ui->num_vertices += 4;
    ui->num_elements += 6;
}

static void add_color_quad(UIState *ui, vec2 top_left_corner, vec2 bottom_right_corner, 
                           u32 top_left_color, u32 top_right_color,
                           u32 bottom_left_color, u32 bottom_right_color)
{
    u32 vertice_index = ui->num_vertices;
    vec2 uv = ui->current_font.white_pixel_uv;

    assert(ui->num_vertices + 4 < MAX_NUM_VERTICES);
    assert(ui->num_elements + 6 < MAX_NUM_ELEMENTS);

    Vertex *vertices = ui->vertices + ui->num_vertices;
    GLuint *elements = ui->elements + ui->num_elements;

    vertices[0].pos = v2(top_left_corner.x, top_left_corner.y);
    vertices[0].uv = uv;
    vertices[0].color = top_left_color;

    vertices[1].pos = v2(top_left_corner.x, bottom_right_corner.y);
    vertices[1].uv = uv;
    vertices[1].color = bottom_left_color;

    vertices[2].pos = v2(bottom_right_corner.x, bottom_right_corner.y);
    vertices[2].uv = uv;
    vertices[2].color = bottom_right_color;

    vertices[3].pos = v2(bottom_right_corner.x, top_left_corner.y);
    vertices[3].uv = uv;
    vertices[3].color = top_right_color;

    elements[0] = vertice_index + 0;
    elements[1] = vertice_index + 1;
    elements[2] = vertice_index + 2;
    elements[3] = vertice_index + 0;
    elements[4] = vertice_index + 2;
    elements[5] = vertice_index + 3;

    ui->num_vertices += 4;
    ui->num_elements += 6;
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
        vertex->uv = ui->current_font.white_pixel_uv;
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

static u32 read_unicode(u8 **utf8_bytes_start, u8 *utf8_bytes_end, u32 default_unicode = 0)
{
    // NOTE(dan): to get the unicode: extract x-es and glue them together
    // 0x00000 - 0x00007F:  0xxx xxxx                                      // ascii
    // 0x00080 - 0x0007FF:  110x xxxx  10xx xxxx                           // 1100 0000 == 0xC0
    // 0x00800 - 0x07FFFF:  1110 xxxx  10xx xxxx  10xx xxxx                // 1110 0000 == 0xE0
    // 0x10000 - 0x1FFFFF:  1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx     // 1111 0000 == 0xF0
    u32 unicode = default_unicode;
    u32 num_bytes_read = 0;

    u8 *c = *utf8_bytes_start;
    if (*c < 0x80) // NOTE(dan): 1 byte - ascii
    {
        unicode = *c;
        num_bytes_read = 1;
    }
    else if ((*c & 0xE0) == 0xC0) // NOTE(dan): 2 bytes
    {
        if (c + 1 < utf8_bytes_end)
        {
            u8 byte_0_bits = c[0] & 0x1F;
            u8 byte_1_bits = c[1] & 0x3F;

            unicode = (byte_0_bits << 6) | byte_1_bits;
        }
        num_bytes_read = 2;
    }
    else if ((*c & 0xF0) == 0xE0) // NOTE(dan): 3 bytes
    {
        if (c + 2 < utf8_bytes_end)
        {
            u8 byte_0_bits = c[0] & 0x0F;
            u8 byte_1_bits = c[1] & 0x3F;
            u8 byte_2_bits = c[2] & 0x3F;

            unicode = (byte_0_bits << 12) | (byte_1_bits << 6) | byte_2_bits;
        }
        num_bytes_read = 3;
    }
    else // NOTE(dan): 4 bytes
    {
        assert((*c & 0xF8) == 0xF0);
        if (c + 3 < utf8_bytes_end)
        {
            u8 byte_0_bits = c[0] & 0x07;
            u8 byte_1_bits = c[1] & 0x3F;
            u8 byte_2_bits = c[2] & 0x3F;
            u8 byte_3_bits = c[3] & 0x3F;

            unicode = (byte_0_bits << 18) | (byte_1_bits << 12) | (byte_2_bits << 6) | byte_3_bits;
        }
        num_bytes_read = 4;
    }

    *utf8_bytes_start += num_bytes_read;
    return unicode;
}

static vec2 calc_text_size(UIState *ui, char *text, f32 size)
{
    Font *font = &ui->current_font;
    char *at = text;
    char *end = text + string_length(text);
    vec2 at_pos = v2(0, 0);
    f32 scale = size / font->size;

    while (at < end)
    {
        u32 unicode = read_unicode((u8 **)&at, (u8 *)end);
        if (!unicode)
        {
            break;
        }

        // TODO(dan): 0xFF -> max unicode codepoint
        unichar c = (unichar)unicode;
        if (c < 0xFF)
        {
            u16 glyph_index = font->glyph_index_lut[c];
            Glyph *glyph = font->glyphs + glyph_index;

            // TODO(dan): wordwrap, clip
            at_pos.x += scale * glyph->advance_x;
        }
    }

    // TODO(dan): calc when we have wordwrap
    at_pos.y = size;

    return at_pos;
}

static vec2 add_text(UIState *ui, char *text, vec2 pos, f32 size, u32 color)
{
    Font *font = &ui->current_font;
    char *at = text;
    char *end = text + string_length(text);
    vec2 at_pos = pos;
    f32 scale = size / font->size;

    while (at < end)
    {
        u32 unicode = read_unicode((u8 **)&at, (u8 *)end);
        if (!unicode)
        {
            break;
        }

        // TODO(dan): 0xFF -> max unicode codepoint
        unichar c = (unichar)unicode;
        if (c < 0xFF)
        {
            u16 glyph_index = font->glyph_index_lut[c];
            Glyph *glyph = font->glyphs + glyph_index;

            vec2 top_left_corner     = vec2_add(at_pos, vec2_mul(scale, glyph->min_pos));
            vec2 bottom_right_corner = vec2_add(at_pos, vec2_mul(scale, glyph->max_pos));

            add_textured_quad(ui, top_left_corner, bottom_right_corner, glyph->min_uv, glyph->max_uv, color);

            // TODO(dan): wordwrap, clip
            at_pos.x += scale * glyph->advance_x;
        }
    }

    // TODO(dan): calc when we have wordwrap
    at_pos.y += size;

    vec2 text_size = vec2_sub(at_pos, pos);
    return text_size;
}

inline u32 stb_decompress_length(u8 *input);
static u32 stb_decompress(u8 *output, u8 *i, u32 length);

static void decode_base85(u8 *src, u8 *dest)
{
    while (*src)
    {
        #define decode(c)  (((c) >= '\\') ? (c) - 36 : (c) - 35)
        u32 temp = decode(src[0]) + 85 * (decode(src[1]) + 85 * (decode(src[2]) + 85 * (decode(src[3]) + 85 * decode(src[4]))));
        #undef decode

        dest[0] = (temp >>  0) & 0xFF; 
        dest[1] = (temp >>  8) & 0xFF; 
        dest[2] = (temp >> 16) & 0xFF; 
        dest[3] = (temp >> 24) & 0xFF;

        src += 5;
        dest += 4;
    }
}

inline u32 upper_pow2(u32 value)
{
    --value;
    value |= value >>  1; 
    value |= value >>  2; 
    value |= value >>  4; 
    value |= value >>  8; 
    value |= value >> 16; 
    ++value; 
    return value; 
}

static void init_default_ui_texture(UIState *ui)
{
    // TODO(dan): replace this font, it does not have extended latin chars
    unichar glyph_ranges[] =
    {
        0x0020, 0x00FF,
        0
    };
    char *default_font = get_default_font();
    char *default_texture = get_default_texture();

    Font *font = &ui->current_font;
    font->size = 13.0f;
    font->texture_width = 256;
    font->texture_height = 0;
    font->white_pixel_uv = v2(0, 0);

    // TODO(dan): do we want to free this
    // TempMemoryStack temp_memory = begin_temp_memory(&font_state->font_memory);
    {
        u32 decoded_font_size = ((string_length(default_font) + 4) / 5) * 4;
        u8 *decoded_font = (u8 *)push_size(&ui->font_memory, decoded_font_size);
        decode_base85((u8 *)default_font, decoded_font);

        u32 decompressed_font_size = stb_decompress_length((u8 *)decoded_font);
        u8 *decompressed_font = (u8 *)push_size(&ui->font_memory, decompressed_font_size);
        stb_decompress(decompressed_font, decoded_font, decoded_font_size);

        u32 num_glyphs = 0;
        u32 num_glyph_ranges = 0;
        u32 font_offset = stbtt_GetFontOffsetForIndex(decompressed_font, 0);

        stbtt_fontinfo font_info;
        stbtt_InitFont(&font_info, decompressed_font, font_offset);

        for (unichar *glyph_range = glyph_ranges; glyph_range[0] && glyph_range[1]; glyph_range += 2)
        {
            num_glyphs += (glyph_range[1] - glyph_range[0]) + 1;
            ++num_glyph_ranges;
        }

        stbtt_packedchar *packed_chars  = push_array(&ui->font_memory, num_glyphs, stbtt_packedchar);
        stbtt_pack_range *packed_ranges = push_array(&ui->font_memory, num_glyph_ranges, stbtt_pack_range);
        stbrp_rect *packed_rects        = push_array(&ui->font_memory, num_glyphs, stbrp_rect);
        
        u32 num_packed_chars = 0;
        for (u32 glyph_range_index = 0; glyph_range_index < num_glyph_ranges; ++glyph_range_index)
        {
            unichar *glyph_range = glyph_ranges + (2 * glyph_range_index);
            stbtt_pack_range *packed_range = packed_ranges + glyph_range_index;

            packed_range->font_size = font->size;
            packed_range->first_unicode_codepoint_in_range = glyph_range[0];
            packed_range->num_chars = (glyph_range[1] - glyph_range[0]) + 1;
            packed_range->chardata_for_range = packed_chars + num_packed_chars;

            num_packed_chars += packed_range->num_chars;
        }

        stbtt_pack_context pack_context;
        stbtt_PackBegin(&pack_context, 0, font->texture_width, 1024 * 32, 0, 1, 0);

        stbrp_rect default_texture_rect = {0};
        default_texture_rect.w = (DEFAULT_TEXTURE_WIDTH * 2) + 1;
        default_texture_rect.h = DEFAULT_TEXTURE_HEIGHT + 1;

        font->texture_height = (u32)(default_texture_rect.y + default_texture_rect.h);

        stbtt_PackSetOversampling(&pack_context, 1, 1);
        stbrp_pack_rects((stbrp_context *)pack_context.pack_info, &default_texture_rect, 1);

        u32 num_packed_rects = stbtt_PackFontRangesGatherRects(&pack_context, &font_info, packed_ranges, num_glyph_ranges, packed_rects);

        stbtt_PackSetOversampling(&pack_context, 3, 1);
        stbrp_pack_rects((stbrp_context *)pack_context.pack_info, packed_rects, num_packed_rects);

        for (u32 packed_rect_index = 0; packed_rect_index < num_packed_rects; ++packed_rect_index)
        {
            stbrp_rect *packed_rect = packed_rects + packed_rect_index;
            if (packed_rect->was_packed)
            {
                font->texture_height = max(font->texture_height, (u32)(packed_rect->y + packed_rect->h));
            }
        }

        // TODO(dan): permanent storage for this?
        font->texture_height = upper_pow2(font->texture_height);
        font->texture_pixels = (u8 *)push_size(&ui->font_memory, font->texture_height * font->texture_width * 4);

        pack_context.pixels = font->texture_pixels;
        pack_context.height = font->texture_height;

        for (u32 y = 0, pixel_index = 0; y < DEFAULT_TEXTURE_HEIGHT; ++y)
        {
            for (u32 x = 0; x < DEFAULT_TEXTURE_WIDTH; ++x, ++pixel_index)
            {
                u32 offset_0 = (u32)(default_texture_rect.x + x) + (u32)(default_texture_rect.y + y) * font->texture_width;
                u32 offset_1 = offset_0 + 1 + DEFAULT_TEXTURE_WIDTH;
                char texture_data = default_texture[pixel_index];

                font->texture_pixels[offset_0] = (texture_data == '.') ? 0xFF : 0x00;
                font->texture_pixels[offset_1] = (texture_data == 'X') ? 0xFF : 0x00;
            }
        }

        stbtt_PackFontRangesRenderIntoRects(&pack_context, &font_info, packed_ranges, num_glyph_ranges, packed_rects);
        stbtt_PackEnd(&pack_context);

        f32 font_scale = stbtt_ScaleForPixelHeight(&font_info, font->size);

        i32 unscaled_ascent;
        i32 unscaled_descent;
        i32 unscaled_line_gap;
        stbtt_GetFontVMetrics(&font_info, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

        font->ascent = font_scale * unscaled_ascent;
        font->descent = font_scale * unscaled_descent;
        font->num_glyphs = 0;

        for (u32 glyph_range_index = 0; glyph_range_index < num_glyph_ranges; ++glyph_range_index)
        {
            stbtt_pack_range *packed_range = packed_ranges + glyph_range_index;

            for (i32 char_index = 0; char_index < packed_range->num_chars; ++char_index)
            {
                stbtt_packedchar *packed_char = packed_range->chardata_for_range + char_index;

                if (packed_char->x0 || packed_char->x1 || packed_char->y0 || packed_char->y1)
                {
                    f32 x = 0.0f;
                    f32 y = 0.0f;
                    stbtt_aligned_quad quad;
                    stbtt_GetPackedQuad(packed_range->chardata_for_range, font->texture_width, font->texture_height, char_index, &x, &y, &quad, 0);

                    assert(font->num_glyphs < array_count(font->glyphs));
                    Glyph *glyph = font->glyphs + font->num_glyphs++;
                    
                    glyph->codepoint = (unichar)(packed_range->first_unicode_codepoint_in_range + char_index);
                    glyph->advance_x = packed_char->xadvance;
                    
                    glyph->min_pos = v2(quad.x0, quad.y0);
                    glyph->max_pos = v2(quad.x1, quad.y1);
                    glyph->min_uv = v2(quad.s0, quad.t0);
                    glyph->max_uv = v2(quad.s1, quad.t1);

                    glyph->min_pos.y += font->ascent;
                    glyph->max_pos.y += font->ascent;
                }
            }
        }

        for (u32 glyph_index = 0; glyph_index < font->num_glyphs; ++glyph_index)
        {
            unichar codepoint = font->glyphs[glyph_index].codepoint;

            font->advance_x_lut[codepoint] = font->glyphs[glyph_index].advance_x;
            font->glyph_index_lut[codepoint] = (u16)glyph_index;
        }

        for (u32 pixel_index = font->texture_width * font->texture_height - 1; pixel_index; --pixel_index)
        {
            u32 *dest_pixel = (u32 *)font->texture_pixels + pixel_index;
            u8 *src_pixel = (u8 *)font->texture_pixels + pixel_index;

            *dest_pixel = 0xFFFFFF | ((*src_pixel) << 24);
        }

        font->texture_pixels[0] = 0xFF;
        font->texture_pixels[1] = 0xFF;
        font->texture_pixels[2] = 0xFF;
        font->texture_pixels[3] = 0xFF;

        vec2 white_pixel_uv_scale = v2(1.0f / font->texture_width, 1.0f / font->texture_height);
        font->white_pixel_uv = vec2_hadamard(white_pixel_uv_scale, vec2_add(default_texture_rect, v2(0.5f, 0.5f)));

    }
    // end_temp_memory(temp_memory);
}

static u8 *stb__barrier;
static u8 *stb__barrier2;
static u8 *stb__barrier3;
static u8 *stb__barrier4;
static u8 *stb__dout;

inline u32 stb_decompress_length(u8 *input)
{
    u32 result = (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
    return result;
}

static void stb__match(u8 *data, u32 length)
{
   assert(stb__dout + length <= stb__barrier);
   if (stb__dout + length > stb__barrier) { stb__dout += length; return; }
   if (data < stb__barrier4) { stb__dout = stb__barrier+1; return; }
   while (length--) *stb__dout++ = *data++;
}

static void stb__lit(u8 *data, u32 length)
{
   assert(stb__dout + length <= stb__barrier);
   if (stb__dout + length > stb__barrier) { stb__dout += length; return; }
   if (data < stb__barrier2) { stb__dout = stb__barrier+1; return; }
   copy_memory(stb__dout, data, length);
   stb__dout += length;
}

#define stb__in2(x)   ((i[x] << 8) + i[(x)+1])
#define stb__in3(x)   ((i[x] << 16) + stb__in2((x)+1))
#define stb__in4(x)   ((i[x] << 24) + stb__in3((x)+1))

static u8 *stb_decompress_token(u8 *i)
{
   if (*i >= 0x20) { // use fewer if's for cases that expand small
      if (*i >= 0x80)       stb__match(stb__dout-i[1]-1, i[0] - 0x80 + 1), i += 2;
      else if (*i >= 0x40)  stb__match(stb__dout-(stb__in2(0) - 0x4000 + 1), i[2]+1), i += 3;
      else /* *i >= 0x20 */ stb__lit(i+1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
   } else { // more ifs for cases that expand large, since overhead is amortized
      if (*i >= 0x18)       stb__match(stb__dout-(stb__in3(0) - 0x180000 + 1), i[3]+1), i += 4;
      else if (*i >= 0x10)  stb__match(stb__dout-(stb__in3(0) - 0x100000 + 1), stb__in2(3)+1), i += 5;
      else if (*i >= 0x08)  stb__lit(i+2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
      else if (*i == 0x07)  stb__lit(i+3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
      else if (*i == 0x06)  stb__match(stb__dout-(stb__in3(1)+1), i[4]+1), i += 5;
      else if (*i == 0x04)  stb__match(stb__dout-(stb__in3(1)+1), stb__in2(4)+1), i += 6;
   }
   return i;
}

static u32 stb_adler32(u32 adler32, u8 *buffer, u32 buflen)
{
   #define ADLER_MOD 65521
   u32 s1 = adler32 & 0xffff, s2 = adler32 >> 16;
   u32 blocklen, i;

   blocklen = buflen % 5552;
   while (buflen) {
      for (i=0; i + 7 < blocklen; i += 8) {
         s1 += buffer[0], s2 += s1;
         s1 += buffer[1], s2 += s1;
         s1 += buffer[2], s2 += s1;
         s1 += buffer[3], s2 += s1;
         s1 += buffer[4], s2 += s1;
         s1 += buffer[5], s2 += s1;
         s1 += buffer[6], s2 += s1;
         s1 += buffer[7], s2 += s1;

         buffer += 8;
      }

      for (; i < blocklen; ++i)
         s1 += *buffer++, s2 += s1;

      s1 %= ADLER_MOD, s2 %= ADLER_MOD;
      buflen -= blocklen;
      blocklen = 5552;
   }
   return (s2 << 16) + s1;
}

static u32 stb_decompress(u8 *output, u8 *i, u32 length)
{
   u32 olen;
   if (stb__in4(0) != 0x57bC0000) return 0;
   if (stb__in4(4) != 0)          return 0; // error! stream is > 4GB
   olen = stb_decompress_length(i);
   stb__barrier2 = i;
   stb__barrier3 = i+length;
   stb__barrier = output + olen;
   stb__barrier4 = output;
   i += 16;

   stb__dout = output;
   while (1) {
      u8 *old_i = i;
      i = stb_decompress_token(i);
      if (i == old_i) {
         if (*i == 0x05 && i[1] == 0xfa) {
            assert(stb__dout == output + olen);
            if (stb__dout != output + olen) return 0;
            if (stb_adler32(1, output, olen) != (u32) stb__in4(2))
               return 0;
            return olen;
         } else {
            assert(0); /* NOTREACHED */
            return 0;
         }
      }
      assert(stb__dout <= output + olen); 
      if (stb__dout > output + olen)
         return 0;
   }
}
