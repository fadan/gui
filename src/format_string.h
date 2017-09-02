
enum FormatStringFlag
{
    FormatStringFlag_LeftJustify        = 1 << 0,
    FormatStringFlag_LeadingPlusSign    = 1 << 1,
    FormatStringFlag_LeadingSpace       = 1 << 2,
    FormatStringFlag_Leading0x          = 1 << 3,
    FormatStringFlag_LeadingZero        = 1 << 4,

    // NOTE(dan): internal
    FormatStringFlag_LeadingOX          = 1 << 5,
    FormatStringFlag_Float              = 1 << 6,
    FormatStringFlag_Negative           = 1 << 7,
    FormatStringFlag_HalfSize           = 1 << 8,
    FormatStringFlag_MaxSize            = 1 << 9,
    FormatStringFlag_LongDouble         = 1 << 10,
};

struct FormatStringBuffer
{
    char *base;
    char *at;
    u32 remaining_size;
};

inline b32 write_char(FormatStringBuffer *buffer, char c)
{
    b32 continue_parsing = false;
    if (buffer->remaining_size)
    {
        *buffer->at++ = c;

        if (--buffer->remaining_size)
        {
            continue_parsing = true;
        }
    }
    return continue_parsing;
}

inline b32 write_string(FormatStringBuffer *buffer, char *string, u32 length)
{
    if (length > buffer->remaining_size)
    {
        length = buffer->remaining_size;
    }
    copy_memory(buffer->at, string, length);

    buffer->at += length;
    buffer->remaining_size -= length;

    b32 continue_parsing = false;
    if (buffer->remaining_size)
    {
        continue_parsing = true;
    }
    return continue_parsing;
}

static char lower_hex_digits[] = "0123456789abcdef";
static char upper_hex_digits[] = "0123456789ABCDEF";

inline void write_u64(FormatStringBuffer *buffer, u64 value, u32 base, char *digits)
{
    char *at = buffer->at;
    do
    {
        u64 digit_index = value % base;
        char digit = digits[digit_index];

        *buffer->at++ = digit;
        --buffer->remaining_size;

        value /= base;
    } while (value);

    char *end = buffer->at;
    while (at < end)
    {
        char temp = *--end;
        *end = *at;
        *at++ = temp;
    }
}

inline void write_f64(FormatStringBuffer *buffer, f64 value, i32 precision, char *digits)
{
    // NOTE(dan): this is very inaccurate, also it truncates the precision instead of rounding
    // TODO(dan): replace if needed: docs @ https://cseweb.ucsd.edu/~lerner/papers/fp-printing-popl16.pdf

    if (value < 0)
    {
        *buffer->at++ = '-';
        --buffer->remaining_size;
        value = -value;
    }

    u64 value_u64 = (u64)value;
    value -= (f64)value_u64;

    write_u64(buffer, value_u64, 10, lower_hex_digits);

    *buffer->at++ = '.';
    --buffer->remaining_size;

    for (i32 precision_index = 0; precision_index < precision; ++precision_index)
    {
        value *= 10.0f;
        u32 value_u32 = (u32)value;
        value -= (f32)value_u32;

        *buffer->at++ = digits[value_u32];
        --buffer->remaining_size;
    }
}

inline b32 write_temp_buffer(FormatStringBuffer *out_buffer, FormatStringBuffer *temp_buffer,
                             u32 flags, u32 width, i32 precision)
{
    char *prefix = "";
    if (flags & FormatStringFlag_Negative)
    {
        prefix = "-";
    }
    else if (flags & FormatStringFlag_LeadingPlusSign)
    {
        prefix = "+";
    }
    else if (flags & FormatStringFlag_LeadingSpace)
    {
        prefix = " ";
    }
    else if (flags & FormatStringFlag_Leading0x)
    {
        prefix = "0x";
    }
    else if (flags & FormatStringFlag_LeadingOX)
    {
        prefix = "0X";
    }
    u32 prefix_length = string_length(prefix);

    u32 use_precision = (u32)precision;
    if (precision < 0 || (flags & FormatStringFlag_Float))
    {
        use_precision = (u32)(temp_buffer->at - temp_buffer->base);
    }

    u32 total_width = use_precision + prefix_length;
    if (total_width > out_buffer->remaining_size)
    {
        total_width = out_buffer->remaining_size;
    }

    u32 use_width = width;
    if (use_width < total_width)
    {
        use_width = total_width;
    }

    if (flags & FormatStringFlag_LeadingZero)
    {
        flags |= ~FormatStringFlag_LeftJustify;
    }

    if (!(flags & FormatStringFlag_LeftJustify))
    {
        while (use_width > (use_precision + prefix_length))
        {
            *out_buffer->at++ = (flags & FormatStringFlag_LeadingZero) ? '0' : ' ';
            --out_buffer->remaining_size;
            --use_width;
        }
    }

    for (char *at = prefix; *at && use_width; ++at)
    {
        *out_buffer->at++ = *at;
        --out_buffer->remaining_size;
        --use_width;
    }

    if (use_precision > use_width)
    {
        use_precision = use_width;
    }

    while (use_precision > (temp_buffer->at - temp_buffer->base))
    {
        *out_buffer->at++ = '0';
        --out_buffer->remaining_size;
        --use_precision;
        --use_width;
    }

    char *temp_at = temp_buffer->base;
    while (use_precision && (temp_buffer->at != temp_at))
    {
        *out_buffer->at++ = *temp_at++;
        --out_buffer->remaining_size;
        --use_precision;
        --use_width;
    }

    if (flags & FormatStringFlag_LeftJustify)
    {
        while (use_width)
        {
            *out_buffer->at++ = ' ';
            --out_buffer->remaining_size;
            --use_width;
        }
    }

    b32 continue_parsing = (out_buffer->remaining_size > 0);
    return continue_parsing;
}

static u32 format_string_vararg(char *buffer, u32 buffer_size, char *format, param_list params)
{
    FormatStringBuffer out_buffer = {buffer, buffer, buffer_size};
    char *format_at = format;

    b32 parsing = true;
    while (parsing)
    {
        if (*format_at == 0)
        {
            parsing = false;
            break;
        }

        // NOTE(dan): %[flags][width][.precision][length]specifier

        if (*format_at == '%')
        {
            ++format_at;

            // NOTE(dan): flags: [-, +, (space), #, 0]

            u32 flags = 0;
            b32 parsing_flags = true;
            while (parsing_flags)
            {
                switch (*format_at)
                {
                    case '-': 
                    { 
                        flags |= FormatStringFlag_LeftJustify;     
                    } break;

                    case '+': 
                    { 
                        flags |= FormatStringFlag_LeadingPlusSign; 
                    } break;
                    
                    case ' ': 
                    { 
                        flags |= FormatStringFlag_LeadingSpace;    
                    } break;
                    
                    case '#': 
                    { 
                        flags |= FormatStringFlag_Leading0x;
                    } break;
                    
                    case '0': 
                    { 
                        flags |= FormatStringFlag_LeadingZero;
                    } break;
                    
                    default:  
                    { 
                        parsing_flags = false; 
                    } break;
                }

                if (parsing_flags)
                {
                    ++format_at;
                }
            }

            // NOTE(dan): width: [*, (number)]

            u32 width = 0;
            if (*format_at == '*')
            {
                width = get_next_param(params, u32);
                ++format_at;
            }
            else
            {
                while ((*format_at >= '0') && (*format_at <= '9'))
                {
                    width = (width * 10) + (*format_at - '0');
                    ++format_at;
                }
            }

            // NOTE(dan): precision: [.*, .(number)]

            i32 precision = -1;
            if (*format_at == '.')
            {
                ++format_at;

                if (*format_at == '*')
                {
                    precision = get_next_param(params, u32);
                    ++format_at;
                }
                else
                {
                    precision = 0;
                    while ((*format_at >= '0') && (*format_at <= '9'))
                    {
                        precision = (precision * 10) + (*format_at - '0');
                        ++format_at;
                    }
                }
            }

            // NOTE(dan): length: [hh, h, ll, l, j, z, t, L]

            switch (*format_at)
            {
                case 'h':
                {
                    flags |= FormatStringFlag_HalfSize;
                    ++format_at;
                } break;

                case 'l':
                {
                    ++format_at;
                    if (*format_at == 'l')
                    {
                        flags |= FormatStringFlag_MaxSize;
                        ++format_at;
                    }
                } break;

                case 'j':
                {
                    flags |= FormatStringFlag_MaxSize;
                } break;

                case 'z':
                case 't':
                {
                    flags |= (sizeof(char *) == 8) ? FormatStringFlag_MaxSize : 0;
                } break;

                case 'L':
                {
                    flags |= FormatStringFlag_LongDouble;
                    ++format_at;
                } break;
            }

            // NOTE(dan): specifiers: [(d, i), u, o, x, X, f, F, e, E, g, G, a, A, c, s, p, n, %]

            char temp[64];
            FormatStringBuffer temp_buffer = {temp, temp, array_count(temp)};

            switch (*format_at)
            {
                case 'd':
                case 'i':
                case 'u':
                {
                    u64 value;
                    if (flags & FormatStringFlag_MaxSize)
                    {
                        i64 value_i64 = get_next_param(params, i64);
                        if (*format_at != 'u' && value_i64 < 0)
                        {
                            value_i64 = -value_i64;
                            flags |= FormatStringFlag_Negative;
                        }
                        value = (u64)value_i64;
                    }
                    else
                    {
                        i32 value_i32 = get_next_param(params, i32);
                        if (*format_at != 'u' && value_i32 < 0)
                        {
                            value_i32 = -value_i32;
                            flags |= FormatStringFlag_Negative;
                        }
                        value = (u32)value_i32;
                    }

                    write_u64(&temp_buffer, value, 10, lower_hex_digits);
                    parsing = write_temp_buffer(&out_buffer, &temp_buffer, flags, width, precision);
                } break;

                case 'o':
                {
                    u64 value = (flags & FormatStringFlag_MaxSize) ? get_next_param(params, u64) : get_next_param(params, u32);

                    write_u64(&temp_buffer, value, 8, lower_hex_digits);
                    parsing = write_temp_buffer(&out_buffer, &temp_buffer, flags, width, precision);
                } break;

                case 'p':
                {
                    flags |= (sizeof(void *) == 8) ? FormatStringFlag_MaxSize : 0;
                    flags |= ~FormatStringFlag_LeadingZero;
                    
                    u64 value = (flags & FormatStringFlag_MaxSize) ? get_next_param(params, u64) : get_next_param(params, u32);

                    write_u64(&temp_buffer, value, 16, lower_hex_digits);
                    parsing = write_temp_buffer(&out_buffer, &temp_buffer, flags, width, precision);
                } break;

                case 'x':
                {
                    u64 value = (flags & FormatStringFlag_MaxSize) ? get_next_param(params, u64) : get_next_param(params, u32);

                    write_u64(&temp_buffer, value, 16, lower_hex_digits);
                    parsing = write_temp_buffer(&out_buffer, &temp_buffer, flags, width, precision);
                } break;

                case 'X':
                {
                    u64 value = (flags & FormatStringFlag_MaxSize) ? get_next_param(params, u64) : get_next_param(params, u32);
                    flags |= FormatStringFlag_LeadingOX;
                    flags |= ~FormatStringFlag_Leading0x;

                    write_u64(&temp_buffer, value, 16, upper_hex_digits);
                    parsing = write_temp_buffer(&out_buffer, &temp_buffer, flags, width, precision);
                } break;
                
                case 'f':
                case 'F':
                case 'e':
                case 'E':
                case 'g':
                case 'G':
                case 'a':
                case 'A':
                {
                    f64 value = get_next_param(params, f64);
                    flags |= FormatStringFlag_Float;

                    if (precision < 0)
                    {
                        precision = 6;
                    }

                    write_f64(&temp_buffer, value, precision, lower_hex_digits);
                    parsing = write_temp_buffer(&out_buffer, &temp_buffer, flags, width, precision);
                } break;

                case 'c':
                {
                    char c = (char)get_next_param(params, i32);
                    parsing = write_char(&out_buffer, c);
                } break;

                case 's':
                {
                    char *string = get_next_param(params, char *);

                    if (precision < 0)
                    {
                        precision = string_length(string);
                    }


                    // u32 length = string_length(string);
                    // if (length > (u32)precision)
                    // {
                        // length = precision;
                    // }

                    temp_buffer.base = string;
                    temp_buffer.at = string + precision;
                    temp_buffer.remaining_size = 0;

                    parsing = write_temp_buffer(&out_buffer, &temp_buffer, flags, width, precision);
                } break;

                case 'n':
                {
                    i32 *dest = get_next_param(params, i32 *);
                    *dest = (i32)(out_buffer.at - buffer);
                } break;

                case '%':
                {
                    parsing = write_char(&out_buffer, '%');
                } break;

                default:
                {
                    parsing = false;
                    assert_always("Invalid specifier");
                } break;
            }
        }
        else
        {
            parsing = write_char(&out_buffer, *format_at);
        }

        ++format_at;
    }

    u32 buffer_length = (u32)(out_buffer.at - buffer);
    if (buffer_length >= buffer_size)
    {
        buffer_length = buffer_size - 1;
    }
    buffer[buffer_length] = 0;
    return buffer_length;
}

static u32 format_string(char *buffer, u32 buffer_size, char *format, ...)
{
    param_list params = get_params_after(format);
    u32 buffer_length = format_string_vararg(buffer, buffer_size, format, params);
    return buffer_length;
}
