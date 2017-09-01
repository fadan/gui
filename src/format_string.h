
enum FormatStringFlag
{
    FormatStringFlag_LeftJustify        = 1 << 0,
    FormatStringFlag_LeadingPlusSign    = 1 << 1,
    FormatStringFlag_LeadingSpace       = 1 << 2,
    FormatStringFlag_Leading0x          = 1 << 3,
    FormatStringFlag_LeadingZero        = 1 << 4,

    FormatStringFlag_Negative           = 1 << 5,
    FormatStringFlag_HalfSize           = 1 << 6,
    FormatStringFlag_MaxSize            = 1 << 7,
    FormatStringFlag_LongDouble         = 1 << 8,
};

struct FormatStringBuffer
{
    char *at;
    u32 remaining_size;
};

#define format_string(buffer, buffer_size, format, ...) \
        format_string_vararg(buffer, buffer_size, format, get_params_after(format))

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

static u32 format_string_vararg(char *buffer, u32 buffer_size, char *format, param_list params)
{
    char temp[512];
    FormatStringBuffer out_buffer = {buffer, buffer_size};
    FormatStringBuffer temp_buffer = {temp, array_count(temp)};

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

            switch (*format_at)
            {
                case 'd':
                case 'i':
                case 'u':
                {
                    i64 dummy = (flags & FormatStringFlag_MaxSize) ? get_next_param(params, i64) : get_next_param(params, i32);
                } break;

                case 'o':
                {
                    u64 dummy = (flags & FormatStringFlag_MaxSize) ? get_next_param(params, u64) : get_next_param(params, u32);
                } break;

                case 'p':
                {
                    flags |= (sizeof(void *) == 8) ? FormatStringFlag_MaxSize : 0;
                    flags |= ~FormatStringFlag_LeadingZero;
                    
                    u64 dummy = (flags & FormatStringFlag_MaxSize) ? get_next_param(params, u64) : get_next_param(params, u32);
                } break;

                case 'x':
                case 'X':
                {
                    u64 dummy = (flags & FormatStringFlag_MaxSize) ? get_next_param(params, u64) : get_next_param(params, u32);
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
                    f64 dummy = get_next_param(params, f64);
                } break;

                case 'c':
                {
                    char c = (char)get_next_param(params, i32);
                    parsing = write_char(&out_buffer, c);
                } break;

                case 's':
                {
                    char *string = get_next_param(params, char *);

                    u32 length = string_length(string);
                    if (length > out_buffer.remaining_size)
                    {
                        length = out_buffer.remaining_size;
                    }
                    copy_memory(out_buffer.at, string, length);
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
