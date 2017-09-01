
#define PLATFORM_UNDEFINED  0
#define PLATFORM_WINDOWS    1
#define PLATFORM_UNIX       2
#define PLATFORM_OSX        3

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM    PLATFORM_WINDOWS
#elif defined(__unix__)
    #define PLATFORM    PLATFORM_UNIX
#elif defined(__APPLE__) && defined(__MACH__)
    #define PLATFORM    PLATFORM_OSX
#else
    #define PLATFORM    PLATFORM_UNDEFINED
#endif

#define COMPILER_UNDEFINED  0
#define COMPILER_MSVC       1
#define COMPILER_GCC        2
#define COMPILER_CLANG      3

#if defined(_MSC_VER)
    #define COMPILER    COMPILER_MSVC
#elif defined(__GNUC__)
    #define COMPILER    COMPILER_GCC
#elif defined(__clang__)
    #define COMPILER    COMPILER_CLANG
#else
    #define COMPILER    COMPILER_UNDEFINED
#endif

#define ARCH_32_BIT    0
#define ARCH_64_BIT    1

#if defined(_WIN64) || defined(__x86_64__) || defined(__ia64__) || defined(__LP64__)
    #define ARCH    ARCH_64_BIT
#else
    #define ARCH    ARCH_32_BIT
#endif

typedef unsigned char    u8;
typedef   signed char    i8;
typedef unsigned short  u16;
typedef   signed short  i16;
typedef unsigned int    u32;
typedef   signed int    i32;

#if COMPILER == COMPILER_MSVC
    typedef unsigned __int64    u64;
    typedef          __int64    i64;
#else
    typedef unsigned long long  u64;
    typedef          long long  i64;
#endif

typedef u8             uchar;
typedef u16            ushort;
typedef u32            uint;
typedef unsigned long  ulong;

typedef i32    b32;
typedef float  f32;
typedef double f64;

typedef u16 unichar;

#if ARCH == ARCH_64_BIT
    typedef u64     usize;
    typedef i64     isize;
    typedef u64     uintptr;
    typedef i64     intptr;
    typedef u64     ulongptr;
    typedef i64     longptr;
#else
    typedef u32     usize;
    typedef i32     isize;
    typedef u32     uintptr;
    typedef i32     intptr;
    typedef ulong   ulongptr;
    typedef long    longptr;
#endif

typedef char test_size_u8[sizeof(u8)   == 1 ? 1 : -1];
typedef char test_size_u16[sizeof(u16) == 2 ? 1 : -1];
typedef char test_size_u32[sizeof(u32) == 4 ? 1 : -1];
typedef char test_size_u64[sizeof(u64) == 8 ? 1 : -1];
typedef char test_size_usize[sizeof(usize) == sizeof(char *) ? 1 : -1];

#if INTERNAL_BUILD
    #if COMPILER == COMPILER_MSVC
        // TODO(dan): message box?
        #define assert(e) do { if (!(e)) { __debugbreak(); } } while(0)
    #else
        #define assert(e) do { if (!(e)) { *(i32 *)0 = 0; } } while(0)
    #endif
#else
    #define assert(e)
#endif
#define assert_always(...)      assert(!"Invalid codepath! "__VA_ARGS__)
#define invalid_default_case    default: { assert_always(); } break

#define array_count(a)              (sizeof(a) / sizeof((a)[0]))
#define offset_of(type, element)    ((usize)&(((type *)0)->element))
#define swap_values(type, a, b) do { type temp__ = (a); (a) = (b); (b) = temp__; } while (0)

#if COMPILER == COMPILER_MSVC
    #define align_of(t) __alignof(t)
#else
    #define align_of(t) ((char *)(&((struct { char c; t _h; } *)0)->_h) - (char *)0)
#endif

#define max(a, b) ((a) > (b)) ? (a) : (b)
#define min(a, b) ((a) < (b)) ? (a) : (b)

#define KB  (1024LL)
#define MB  (1024LL * KB)
#define GB  (1024LL * MB)
#define TB  (1024LL * GB)

#define F32_MAX 3.402823E+38F

#if COMPILER == COMPILER_MSVC
    extern "C" long _InterlockedExchangeAdd(long volatile *addend, long value);
    extern "C" __int64 _InterlockedExchangeAdd64(__int64 volatile *addend, __int64 value);
    extern "C" long _InterlockedExchange(long volatile *target, long value);
    extern "C" __int64 _InterlockedExchange64(__int64 volatile *target, __int64 value);
    extern "C" long _InterlockedCompareExchange(long volatile *destination, long exchange, long comparand); 
    extern "C" void _mm_pause();
    extern "C" unsigned __int64 __rdtsc();
    extern "C" unsigned __int64 __readgsqword(unsigned long offset);

    inline u32 atomic_add_u32(u32 volatile *addend, u32 value)
    {
        // NOTE(dan): result = current value of addend
        u32 result = _InterlockedExchangeAdd((long volatile *)addend, value);
        return result;
    }

    inline u64 atomic_add_u64(u64 volatile *addend, u64 value)
    {
        // NOTE(dan): result = current value of addend
        u64 result = _InterlockedExchangeAdd64((__int64 volatile *)addend, value);
        return result;
    }

    inline u32 atomic_exchange_u32(u32 volatile *target, u32 value)
    {
        u32 result = _InterlockedExchange((long volatile *)target, value);
        return result;
    }


    inline u64 atomic_exchange_u64(u64 volatile *target, u64 value)
    {
        u64 result = _InterlockedExchange64((__int64 volatile *)target, value);
        return result;
    }

    inline u32 atomic_cmpxchg_u32(u32 volatile *value, u32 volatile new_value, u32 expected)
    {
        u32 result = _InterlockedCompareExchange((long volatile *)value, new_value, expected);
        return result;
    }

    inline u32 get_thread_id()
    {
        u8 *thread_local_storage = (u8 *)__readgsqword(0x30);
        u32 thread_id = *(u32 *)(thread_local_storage + 0x48);
        return thread_id;
    }
#endif

enum PlatformButtonScanCode
{
    button_escape =             0x01,      button_1 =                 0x02,        button_2 =               0x03,      button_3 =                0x04,
    button_4 =                  0x05,      button_5 =                 0x06,        button_6 =               0x07,      button_7 =                0x08,
    button_8 =                  0x09,      button_9 =                 0x0A,        button_0 =               0x0B,      button_minus =            0x0C,
    button_equals =             0x0D,      button_backspace =         0x0E,        button_tab =             0x0F,      button_q =                0x10,
    button_w =                  0x11,      button_e =                 0x12,        button_r =               0x13,      button_t =                0x14,
    button_y =                  0x15,      button_u =                 0x16,        button_i =               0x17,      button_o =                0x18,
    button_p =                  0x19,      button_bracket_left =      0x1A,        button_bracket_right =   0x1B,      button_enter =            0x1C,
    button_control_left =       0x1D,      button_a =                 0x1E,        button_s =               0x1F,      button_d =                0x20,
    button_f =                  0x21,      button_g =                 0x22,        button_h =               0x23,      button_j =                0x24,
    button_k =                  0x25,      button_l =                 0x26,        button_semicolon =       0x27,      button_apostrophe =       0x28,
    button_grave =              0x29,      button_shift_left =        0x2A,        button_backslash =       0x2B,      button_z =                0x2C,
    button_x =                  0x2D,      button_c =                 0x2E,        button_v =               0x2F,      button_b =                0x30,
    button_n =                  0x31,      button_m =                 0x32,        button_comma =           0x33,      button_preiod =           0x34,
    button_slash =              0x35,      button_shift_right =       0x36,        button_numpad_multiply = 0x37,      button_alt_left =         0x38,
    button_space =              0x39,      button_caps_lock =         0x3A,        button_f1 =              0x3B,      button_f2 =               0x3C,
    button_f3 =                 0x3D,      button_f4 =                0x3E,        button_f5 =              0x3F,      button_f6 =               0x40,
    button_f7 =                 0x41,      button_f8 =                0x42,        button_f9 =              0x43,      button_f10 =              0x44,
    button_num_lock =           0x45,      button_scroll_lock =       0x46,        button_numpad_7 =        0x47,      button_numpad_8 =         0x48,
    button_numpad_9 =           0x49,      button_numpad_minus =      0x4A,        button_numpad_4 =        0x4B,      button_numpad_5 =         0x4C,
    button_numpad_6 =           0x4D,      button_numpad_plus =       0x4E,        button_numpad_1 =        0x4F,      button_numpad_2 =         0x50,
    button_numpad_3 =           0x51,      button_numpad_0 =          0x52,        button_numpad_period =   0x53,      button_alt_print_screen = 0x54,
    button_bracket_angle =      0x56,      button_f11 =               0x57,        button_f12 =             0x58,      button_oem_1 =            0x5a,
    button_oem_2 =              0x5b,      button_oem_3 =             0x5c,        button_erase_EOF =       0x5d,      button_oem_4 =            0x5e,
    button_oem_5 =              0x5f,      button_zoom =              0x62,        button_help =            0x63,      button_f13 =              0x64,
    button_f14 =                0x65,      button_f15 =               0x66,        button_f16 =             0x67,      button_f17 =              0x68,
    button_f18 =                0x69,      button_f19 =               0x6a,        button_f20 =             0x6b,      button_f21 =              0x6c,
    button_f22 =                0x6d,      button_f23 =               0x6e,        button_oem_6 =           0x6f,      button_katakana =         0x70,
    button_oem_7 =              0x71,      button_f24 =               0x76,        button_sbcschar =        0x77,      button_convert =          0x79,
    button_nonconvert =         0x7B,      
    
    button_media_previous =     0xE010,    button_media_next =        0xE019,      button_numpad_enter =    0xE01C,    button_control_right =    0xE01D,    
    button_volume_mute =        0xE020,    button_launch_app2 =       0xE021,      button_media_play =      0xE022,    button_media_stop =       0xE024,    
    button_volume_down =        0xE02E,    button_volume_up =         0xE030,      button_browser_home =    0xE032,    button_numpad_divide =    0xE035,
    button_print_screen =       0xE037,    button_alt_right =         0xE038,      button_cancel =          0xE046,    button_home =             0xE047,    
    button_arrow_up =           0xE048,    button_page_up =           0xE049,      button_arrow_left =      0xE04B,    button_arrow_right =      0xE04D,    
    button_end =                0xE04F,    button_arrow_down =        0xE050,      button_page_down =       0xE051,    button_insert =           0xE052,    
    button_delete =             0xE053,    button_meta_left =         0xE05B,      button_meta_right =      0xE05C,    button_application =      0xE05D,
    button_power =              0xE05E,    button_sleep =             0xE05F,      button_wake =            0xE063,    button_browser_search =   0xE065,
    button_browser_favorites =  0xE066,    button_browser_refresh =   0xE067,      button_browser_stop =    0xE068,    button_browser_forward =  0xE069,
    button_browser_back =       0xE06A,    button_launch_app1 =      0xE06B,       button_launch_email =    0xE06C,    button_launch_media =     0xE06D,    

    button_pause =              0xE11D45,
};

inline u32 get_scan_code_offset(u32 scan_code)
{
    u32 offset = scan_code;
    if (scan_code >= button_pause)
    {
        offset = button_nonconvert + 1 + (button_launch_media - button_media_previous) + 1 + (scan_code - button_pause);
    }
    else if (scan_code >= button_media_previous)
    {
        offset = button_nonconvert + 1 + (scan_code - button_media_previous);
    }
    assert(offset <= 0xFF);
    return offset;
}

enum
{
    mouse_button_left,
    mouse_button_middle,
    mouse_button_right,
    mouse_button_extended0,
    mouse_button_extended1,

    mouse_button_count,
};

struct PlatformButton
{
    b32 down;
    b32 pressed;
    b32 released;
};

struct PlatformInput
{
    f32 dt;
    b32 quit_requested;

    i32 wheel;
    i32 delta_wheel;
    i32 mouse_pos[2];
    i32 delta_mouse_pos[2];

    u32 text_input_length;
    u16 text_input[16];

    PlatformButton buttons[256];
    PlatformButton mouse_buttons[mouse_button_count];
};

inline b32 is_down(PlatformInput *input, u32 scan_code)
{
    u32 offset = get_scan_code_offset(scan_code);
    b32 is_down = input->buttons[offset].down;
    return is_down;
}

inline b32 is_pressed(PlatformInput *input, u32 scan_code)
{
    u32 offset = get_scan_code_offset(scan_code);
    b32 is_pressed = input->buttons[offset].pressed;
    return is_pressed;
}

inline b32 is_released(PlatformInput *input, u32 scan_code)
{
    u32 offset = get_scan_code_offset(scan_code);
    b32 is_released = input->buttons[offset].released;
    return is_released;
}

struct PlatformMemoryBlock
{
    usize size;
    usize used;
    u8 *base;
    u64 flags;
    PlatformMemoryBlock *prev;
};

struct PlatformMemoryStats
{
    usize num_memblocks;
    usize total_size;
    usize total_used;
};

#define PLATFORM_ALLOCATE(name)    PlatformMemoryBlock *name(usize size)
#define PLATFORM_DEALLOCATE(name)  void name(PlatformMemoryBlock *memblock)
#define PLATFORM_GET_MEMORY_STATS(name) PlatformMemoryStats name()
#define PLATFORM_INIT_OPENGL(name) void name(struct OpenGL *open_gl)

typedef PLATFORM_ALLOCATE(PlatformAllocate);
typedef PLATFORM_DEALLOCATE(PlatformDeallocate);
typedef PLATFORM_GET_MEMORY_STATS(PlatformGetMemoryStats);
typedef PLATFORM_INIT_OPENGL(PlatformInitOpenGL);

#define PLATFORM_VIRTUAL_ALLOC(name)    void *name(usize size)
#define PLATFORM_VIRTUAL_FREE(name)     void name(void *memory)

typedef PLATFORM_VIRTUAL_ALLOC(PlatformVirtualAlloc);
typedef PLATFORM_VIRTUAL_FREE(PlatformVirtualFree);

struct Platform
{
    PlatformAllocate *allocate;
    PlatformDeallocate *deallocate;
    PlatformGetMemoryStats *get_memory_stats;
    PlatformInitOpenGL *init_opengl;

    PlatformVirtualAlloc *virtual_alloc;
    PlatformVirtualFree *virtual_free;
};

extern Platform platform;

struct AppMemory
{
    struct AppState *app_state;
    Platform platform;
};

struct Mutex
{
    u64 volatile ticket;
    u64 volatile serving;
};

inline void begin_mutex(Mutex *mutex)
{
    u64 ticket = atomic_add_u64(&mutex->ticket, 1);
    while (ticket != mutex->serving)
    {
        _mm_pause();
    }
}

inline void end_mutex(Mutex *mutex)
{
    atomic_add_u64(&mutex->serving, 1);
}

#define allocate_freelist(result, first_free, allocation) \
    (result) = (first_free); \
    if (result) \
    { \
        first_free = (result)->next_free; \
    } \
    else \
    { \
        result = allocation; \
    }

#define deallocate_freelist(free, free_list) \
    if (free) \
    { \
        (free)->next_free = (free_list); \
        (free_list) = (free); \
    }

#define dllist_init(sentinel) \
    (sentinel)->next = (sentinel); \
    (sentinel)->prev = (sentinel);

#define dllist_insert_last(sentinel, element) \
    (element)->next = (sentinel); \
    (element)->prev = (sentinel)->prev; \
    (element)->next->prev = (element); \
    (element)->prev->next = (element);

#define zero_struct(dest) zero_size(&(dest), sizeof(dest))

inline void zero_size(void *dest, usize size)
{
    u8 *byte = (u8 *)dest;
    while (size--)
    {
        *byte++ = 0;
    }
}

struct MemoryStack
{
    PlatformMemoryBlock *memblock;
    usize min_stack_size;
    u64 flags;
    u64 temp_stacks;
};

struct TempMemoryStack
{
    PlatformMemoryBlock *memblock;
    MemoryStack *memstack;
    usize used;
};

struct MemoryStackParams
{
    b32 clear_to_zero;
    u32 alignment;
};

#define DEFAULT_MEMORY_STACK_ALINGMENT  4
#define DEFAULT_MEMORY_STACK_SIZE       1*MB

inline MemoryStackParams default_params()               { return {true,  DEFAULT_MEMORY_STACK_ALINGMENT}; }
inline MemoryStackParams no_clear()                     { return {false, DEFAULT_MEMORY_STACK_ALINGMENT}; }
inline MemoryStackParams align_no_clear(u32 alignment)  { return {false, alignment}; }
inline MemoryStackParams align_clear(u32 alignment)     { return {true,  alignment}; }

inline usize get_next_memory_stack_offset(MemoryStack *memstack, usize alignment)
{
    usize result = 0;
    usize mask = alignment - 1;
    usize next_base = (usize)memstack->memblock->base + memstack->memblock->used;
    if (next_base & mask)
    {
        result = alignment - (next_base & mask);
    }
    return result;
}

#define push_struct(memstack, type, ...)        (type *)push_size(memstack, sizeof(type), ## __VA_ARGS__)
#define push_array(memstack, count, type, ...)  (type *)push_size(memstack, (count) * sizeof(type), ## __VA_ARGS__)

inline void *push_size(MemoryStack *memstack, usize size, MemoryStackParams params = default_params())
{
    usize effective_size = 0;
    if (memstack->memblock)
    {
        usize offset = get_next_memory_stack_offset(memstack, params.alignment);
        effective_size = size + offset;
    }

    if (!memstack->memblock || ((memstack->memblock->used + effective_size) > memstack->memblock->size))
    {
        // TODO(dan): debug this, sometimes contains garbage!!!
        // if (!memstack->min_stack_size)
        // {
            memstack->min_stack_size = DEFAULT_MEMORY_STACK_SIZE;
        // }

        effective_size = size;
        usize stack_size = max(effective_size, memstack->min_stack_size);

        PlatformMemoryBlock *memblock = platform.allocate(stack_size);
        memblock->prev = memstack->memblock;
        memstack->memblock = memblock;
    }

    assert((memstack->memblock->used + effective_size) <= memstack->memblock->size);

    usize offset = get_next_memory_stack_offset(memstack, params.alignment);
    void *result = memstack->memblock->base + memstack->memblock->used + offset;
    memstack->memblock->used += effective_size;

    if (params.clear_to_zero)
    {
        zero_size(result, size);
    }

    return result;
}

inline char *push_string(MemoryStack *memstack, char *string)
{
    u32 size = 1;
    for (char *at = string; *at; ++at)
    {
        ++size;
    }

    char *dest = (char *)push_size(memstack, size, no_clear());
    for (u32 char_index = 0; char_index < size; ++char_index)
    {
        dest[char_index] = string[char_index];
    }
    return dest;
}

#define bootstrap_push_struct(type, member, ...) (type *)bootstrap_push_size(sizeof(type), offset_of(type, member), ## __VA_ARGS__)

inline void *bootstrap_push_size(usize size, usize offset, MemoryStackParams params = default_params())
{
    MemoryStack bootstrap = {};
    void *result = push_size(&bootstrap, size, params);

    *(MemoryStack *)((u8 *)result + offset) = bootstrap;
    
    return result;
}

inline void init_memory_stack(MemoryStack *memstack, usize size, MemoryStackParams params = default_params())
{
    if (!memstack->memblock)
    {
        void *dummy = push_size(memstack, size, params);
        memstack->memblock->used = 0;
    }
}

inline TempMemoryStack begin_temp_memory(MemoryStack *memstack)
{
    TempMemoryStack result = {0};
    result.memstack = memstack;
    result.memblock = memstack->memblock;
    result.used = memstack->memblock ? memstack->memblock->used : 0;

    ++memstack->temp_stacks;
    return result;
}

inline void free_last_memory_block(MemoryStack *memstack)
{
    PlatformMemoryBlock *memblock = memstack->memblock;
    memstack->memblock = memblock->prev;
    platform.deallocate(memblock);
}

inline void end_temp_memory(TempMemoryStack tempmem)
{
    MemoryStack *memstack = tempmem.memstack;
    while (memstack->memblock != tempmem.memblock)
    {
        free_last_memory_block(memstack);
    }

    if (memstack->memblock)
    {
        assert(memstack->memblock->used >= tempmem.used);
        
        memstack->memblock->used = tempmem.used;
    }

    assert(memstack->temp_stacks > 0);
    --memstack->temp_stacks;
}

inline void check_memory_stack(MemoryStack *memstack)
{
    assert(memstack->temp_stacks == 0);
}

#define UPDATE_AND_RENDER(name) void name(AppMemory *memory, PlatformInput *input, i32 window_width, i32 window_height)
typedef UPDATE_AND_RENDER(UpdateAndRender);
static UPDATE_AND_RENDER(update_and_render_stub)
{
}

#if 0
// dst and src must be 16-byte aligned
// size must be multiple of 16*2 = 32 bytes
static void copy_memory_sse2_aligned(u8 *dst, u8 *src, u64 size)
{
    u64 stride = 2 * sizeof(__m128);
    while (size)
    {
        __m128 a = _mm_load_ps((float *)(src + 0 * sizeof(__m128)));
        __m128 b = _mm_load_ps((float *)(src + 1 * sizeof(__m128)));

        _mm_stream_ps((float *)(dst + 0 * sizeof(__m128)), a);
        _mm_stream_ps((float *)(dst + 1 * sizeof(__m128)), b);

        size -= stride;
        src += stride;
        dst += stride;
    }
}
#endif

static void set_memory(void *memory, char set_byte, u64 num_bytes)
{
    u16 set_word  = ((u16)set_byte  <<  8) | set_byte;
    u32 set_dword = ((u32)set_word  << 16) | set_word;
    u64 set_qword = ((u64)set_dword << 32) | set_dword;

    // NOTE(dan): set with bytes while unaligned
    u8 *byte_ptr    = (u8 *)memory;
    u8 *aligned_ptr = (u8 *)((u64)byte_ptr & (u64)-8);
    u32 num_misaligned_bytes = (u32)(byte_ptr - aligned_ptr);

    if (num_misaligned_bytes > (u32)num_bytes)
    {
        num_misaligned_bytes = (u32)num_bytes;
    }

    for (u32 byte_index = 0; byte_index < num_misaligned_bytes; ++byte_index)
    {
        *byte_ptr++ = set_byte;
    }
    num_bytes -= num_misaligned_bytes;

    // NOTE(dan): set with qwords while possible
    u64 *qword_ptr = (u64 *)byte_ptr;
    u64 num_qwords = (num_bytes >> 3);

    for (u64 qword_index = 0; qword_index < num_qwords; ++qword_index)
    {
        *qword_ptr++ = set_qword;
    }

    // NOTE(dan): set the remaining bytes
    byte_ptr  = (u8 *)qword_ptr;
    num_bytes = num_bytes & 7;

    for (u32 byte_index = 0; byte_index < num_bytes; ++byte_index)
    {
        *byte_ptr++ = set_byte;
    }
}

static void copy_memory(void *dest, void *src, u64 num_bytes)
{
    // NOTE(dan): copy bytes while unaligned
    u8 *src_byte_ptr         = (u8 *)src;
    u8 *dest_byte_ptr        = (u8 *)dest;
    u8 *aligned_src_ptr      = (u8 *)((u64)src_byte_ptr & ~7);
    u32 num_misaligned_bytes = (u32)(src_byte_ptr - aligned_src_ptr);

    if (num_misaligned_bytes > (u32)num_bytes)
    {
        num_misaligned_bytes = (u32)num_bytes;
    }

    for (u32 byte_index = 0; byte_index < num_misaligned_bytes; ++byte_index)
    {
        *dest_byte_ptr++ = *src_byte_ptr++;
    }
    num_bytes -= num_misaligned_bytes;

    // NOTE(dan): copy qwords while possible
    u64 *src_qword_ptr  = (u64 *)src_byte_ptr;
    u64 *dest_qword_ptr = (u64 *)dest_byte_ptr;
    u64 num_qwords      = (num_bytes >> 3);

    for (u64 qword_index = 0; qword_index < num_qwords; ++qword_index)
    {
        *dest_qword_ptr++ = *src_qword_ptr++;
    }

    // NOTE(dan): copy the remaining bytes
    src_byte_ptr  = (u8 *)src_qword_ptr;
    dest_byte_ptr = (u8 *)dest_qword_ptr;
    num_bytes     = num_bytes & 7;

    for (u32 byte_index = 0; byte_index < num_bytes; ++byte_index)
    {
        *dest_byte_ptr++ = *src_byte_ptr++;
    }
}

#define PI32  3.14159265359f
#define TAU32 6.28318530717958647692f

inline f32 abs32(f32 x)
{
    f32 result = (x < 0.0f) ? -x : x;
    return result;
}

inline i32 floor32(f32 x)
{
    i32 result = ((i32)x - ((x < 0.0f) ? 1 : 0));
    return result;
}

inline f32 mod32(f32 a, f32 b)
{
    f32 result = (a - b * floor32(a / b));
    return result;
}

inline i32 ceil32(f32 x)
{
    i32 result = (i32)x;
    if (x < 0.0f)
    {
        f32 frac = x - result;
        if (frac > 0.0f)
        {
            ++result;
        }
    }
    return result;
}

inline f32 pow32(f32 a, f32 b)
{
    union { f32 f; i32 i; } memory = {a};
    memory.i = (i32)(b * (memory.i - 1064866805) + 1064866805);
    return memory.f;
}

inline f32 acos32(f32 x)
{
    static f32 c0 = -0.939115566365855f;
    static f32 c1 = +0.9217841528914573f;
    static f32 c2 = -1.2845906244690837f;
    static f32 c3 = +0.295624144969963174f;

    f32 x2 = x * x;
    f32 result = (PI32 / 2.0f) + (c0 * x + c1 * x2 * x) / (1 + c2 * x2 + c3 * x2 * x2);
    return result;
}

inline f32 inv_sqrt32(f32 a)
{
    union { f32 f; u32 u; } memory = {a};
    memory.u = 0x5F375A86 - (memory.u >> 1);
    memory.f *= 1.5f - (0.5f * a * memory.f * memory.f);
    return memory.f;
}

#define sqrt32(a) ( (a) * inv_sqrt32(a) )

inline f32 sin32(f32 x)
{
    static f32 c0 = +1.91059300966915117e-31f;
    static f32 c1 = +1.00086760103908896f;
    static f32 c2 = -1.21276126894734565e-2f;
    static f32 c3 = -1.38078780785773762e-1f;
    static f32 c4 = -2.67353392911981221e-2f;
    static f32 c5 = +2.08026600266304389e-2f;
    static f32 c6 = -3.03996055049204407e-3f;
    static f32 c7 = +1.38235642404333740e-4f;

    f32 sine = c0 + x * (c1 + x * (c2 + x * (c3 + x * (c4 + x * (c5 + x * (c6 + x * c7))))));
    return sine;
}

inline f32 cos32(f32 x)
{
    static f32 c0 = +1.00238601909309722f;
    static f32 c1 = -3.81919947353040024e-2f;
    static f32 c2 = -3.94382342128062756e-1f;
    static f32 c3 = -1.18134036025221444e-1f;
    static f32 c4 = +1.07123798512170878e-1f;
    static f32 c5 = -1.86637164165180873e-2f;
    static f32 c6 = +9.90140908664079833e-4f;
    static f32 c7 = -5.23022132118824778e-14f;

    f32 cosine = c0 + x * (c1 + x * (c2 + x * (c3 + x * (c4 + x * (c5 + x * (c6 + x * c7))))));
    return cosine;
}

#define intsizeof(type) ((sizeof(type) + sizeof(intptr) - 1) & ~(sizeof(intptr) - 1))

typedef char *param_list;
#define get_params_after(param)     ((param_list)&(param) + intsizeof(param))
#define get_next_param(list, type)  (*(type *)((list += intsizeof(type)) - intsizeof(type)))

inline b32 strings_are_equal(char *a, char *b)
{
    b32 equals = (a == b);
    if (a && b)
    {
        while (*a && *b && (*a == *b))
        {
            ++a;
            ++b;
        }
        equals = ((*a == 0) && (*b == 0));
    }
    return equals;
}

inline u32 string_length(char *string)
{
    u32 length = 0;
    while (*string++)
    {
        ++length;
    }
    return length;
}

inline void copy_string_and_null_terminate(char *src, char *dest, u32 dest_size)
{
    u32 dest_length = 0;
    while (*src && dest_length++ < dest_size)
    {
        *dest++ = *src++;
    }
    *dest++ = 0;
}

static u32 djb2_hash(char *string)
{
    u32 hash = 5381;
    for (char *at = string; *at; ++at)
    {
        hash = ((hash << 5) + hash) + *at;
    }
    return hash;
}
