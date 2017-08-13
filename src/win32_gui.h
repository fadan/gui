typedef intptr (__stdcall *Win32Api_WNDPROC)(void *wnd, unsigned int msg, uintptr wparam, intptr lparam);
typedef intptr (__stdcall *Win32Api_PROC)();
typedef int (__stdcall *Win32Api_THREAD_START_ROUTINE)(void *param);

typedef Win32Api_PROC (__stdcall Win32GetProcAddress)(void *module, char *proc);
typedef void * (__stdcall Win32LoadLibrary)(char *lib);
typedef void * (__stdcall Win32Api_FreeLibrary)(void *module);

#if 1
    extern "C" __declspec(dllimport) Win32Api_PROC __stdcall GetProcAddress(void *module, char *proc);
    extern "C" __declspec(dllimport) void * __stdcall LoadLibraryA(char *lib);
    extern "C" __declspec(dllimport) int __stdcall FreeLibrary(void *module); 

    #define win32_get_proc_address GetProcAddress
    #define win32_load_library LoadLibraryA
    #define win32_free_library FreeLibrary

    typedef void *HINSTANCE;
#else
    #include <windows.h>

    static Win32GetProcAddress *win32_get_proc_address = (Win32GetProcAddress *)GetProcAddress;
    static Win32LoadLibrary *win32_load_library = (Win32LoadLibrary *)LoadLibraryA;
    static Win32Api_FreeLibrary *win32_free_library = (Win32Api_FreeLibrary *)FreeLibrary;
#endif

typedef void * (__stdcall *Win32Api_wglCreateContextAttribsARB)(void *dc, void *rc, int *attribs);
typedef int (__stdcall * Win32Api_wglChoosePixelFormatARB)(void *, int *, float *, unsigned int, int *, unsigned int *);
typedef int (__stdcall * Win32Api_wglSwapIntervalEXT)(int);

struct Win32Api_WNDCLASSEXA
{
    unsigned int cbSize;
    unsigned int style;
    Win32Api_WNDPROC lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    void *hInstance;
    void *hIcon;
    void *hCursor;
    void *hBrush;
    char *lpszMenuName;
    char *lpszClassName;
    void *hIconSm;
};

struct Win32Api_PIXELFORMATDESCRIPTOR
{
    unsigned short nSize;
    unsigned short nVersion;
    unsigned int dwFlags;
    unsigned char iPixelType;
    unsigned char cColorBits;
    unsigned char cRedBits;
    unsigned char cRedShift;
    unsigned char cGreenBits;
    unsigned char cGreenShift;
    unsigned char cBlueBits;
    unsigned char cBlueShift;
    unsigned char cAlphaBits;
    unsigned char cAlphaShift;
    unsigned char cAccumBits;
    unsigned char cAccumRedBits;
    unsigned char cAccumGreenBits;
    unsigned char cAccumBlueBits;
    unsigned char cAccumAlphaBits;
    unsigned char cDepthBits;
    unsigned char cStencilBits;
    unsigned char cAuxBuffers;
    unsigned char iLayerType;
    unsigned char bReserved;
    unsigned int dwLayerMask;
    unsigned int dwVisibleMask;
    unsigned int dwDamageMask;
};

struct Win32Api_RECT
{
    int left;
    int top;
    int right;
    int bottom;    
};

struct Win32Api_POINT
{
    int x;
    int y;
};

struct Win32Api_MSG
{
    void *wnd;
    unsigned int message;
    uintptr wParam;
    intptr lParam;
    unsigned int time;
    Win32Api_POINT pt;
};

struct Win32Api_FILETIME
{
    unsigned int dwLowDateTime;
    unsigned int dwHighDateTime;
};

union Win32Api_LARGE_INTEGER
{
    struct
    {
        unsigned int LowPart;
        unsigned int HighPart;
    };
    struct
    {
        unsigned int LowPart;
        int HighPart;
    } u;
    i64 QuadPart;
};

enum Win32Api_GET_FILEEX_INFO_LEVELS
{
    Win32Api_GetFileExInfoStandard,
    Win32Api_GetFileExMaxInfoLevel,
};

struct Win32Api_WIN32_FILE_ATTRIBUTE_DATA
{
    unsigned int dwFileAttributes;
    Win32Api_FILETIME ftCreationTime;
    Win32Api_FILETIME ftLastAccessTime;
    Win32Api_FILETIME ftLastWriteTime;
    unsigned int nFileSizeHigh;
    unsigned int nFileSizeLow;
};

struct Win32Api_RAWINPUTDEVICE
{
    unsigned short usUsagePage;
    unsigned short usUsage;
    unsigned int dwFlags;
    void *hwndTarget;
};

struct Win32Api_RAWMOUSE
{
    unsigned short usFlags;
    union
    {
        unsigned int ulButtons;
        struct
        {
            unsigned short usButtonFlags;
            unsigned short usButtonData;
        };
    };
    unsigned int ulRawButtons;
    int lLastX;
    int lLastY;
    unsigned int ulExtraInformation;
};

struct Win32Api_RAWKEYBOARD
{
    unsigned short MakeCode;
    unsigned short Flags;
    unsigned short Reserved;
    unsigned short VKey;
    unsigned int Message;
    unsigned int ExtraInformation;
};

struct Win32Api_RAWHID
{
    unsigned int dwSizeHid;
    unsigned int dwCount;
    char bRawData[1];
};

struct Win32Api_RAWINPUTHEADER
{
    unsigned int dwType;
    unsigned int dwSize;
    void *hDevice;
    uintptr wParam;
};

struct Win32Api_RAWINPUT
{
    Win32Api_RAWINPUTHEADER header;
    union
    {
        Win32Api_RAWMOUSE mouse;
        Win32Api_RAWKEYBOARD keyboard;
        Win32Api_RAWHID hid;
    } data;
};

struct Win32Api_WAVEFORMATEX
{
    short wFormatTag;
    short nChannels;
    int nSamplesPerSec;
    int nAvgBytesPerSec;
    short nBlockAlign;
    short wBitsPerSample;
    short cbSize;
};

struct Win32Api_GUID
{
    unsigned int Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
};

struct Win32Api_IMMDeviceEnumeratorVtbl
{
    void (*f1)(); // QueryInterface
    void (*f2)(); // AddRef
    unsigned int (__stdcall *Release)(void *device_enumerator);
    void (*f4)(); // EnumAudioEndpoints
    int (__stdcall *GetDefaultAudioEndpoint)(void *device_enum, int data_flow, int role, void **endpoint);
    void (*f6)(); // GetDevice
    void (*f7)(); // RegisterEndpointNotificationCallback
    void (*f8)(); // UnregisterEndpointNotificationCallback
};

struct Win32Api_IMMDeviceEnumerator
{
    Win32Api_IMMDeviceEnumeratorVtbl *vtbl;
};

struct Win32Api_IMMDeviceVtbl
{
    void (*f1)(); // QueryInterface
    void (*f2)(); // AddRef
    unsigned int (__stdcall *Release)(void *device);
    int (__stdcall *Activate)(void *device, void *iid, int cls_ctx, struct tagPROPVARIANT *activation_params, void **interface);
    void (*f5)(); // OpenPropertyStore
    void (*f6)(); // GetId
    void (*f7)(); // GetState
};

struct Win32Api_IMMDevice
{
    Win32Api_IMMDeviceVtbl *vtbl;
};

struct Win32Api_IAudioClientVtbl
{
    void (*f1)(); // QueryInterface
    void (*f2)(); // AddRef
    void (*f3)(); // Release
    int (__stdcall *Initialize)(void *client, int share_mode, int stream_flags, u64 buffer_duration, u64 periodicity, Win32Api_WAVEFORMATEX *format, void *audio_session_guid);
    int (__stdcall *GetBufferSize)(void *client, unsigned int *num_buffer_frames);
    void (*f6)(); // GetStreamLatency
    int (__stdcall *GetCurrentPadding)(void *client, unsigned int *num_padding_frames);
    void (*f8)(); // IsFormatSupported
    void (*f9)(); // GetMixFormat
    void (*f10)(); // GetDevicePeriod
    int (__stdcall *Start)(void *client);
    int (__stdcall *Stop)(void *client);
    void (*f13)(); // Reset
    int (__stdcall *SetEventHandle)(void *client, void *event_handle);
    int (__stdcall *GetService)(void *client, Win32Api_GUID *riid, void **ppv);
};

struct Win32Api_IAudioClient
{
    Win32Api_IAudioClientVtbl *vtbl;
};

struct Win32Api_IAudioRenderClientVtbl
{
    void (*f1)(); // QueryInterface
    void (*f2)(); // AddRef
    void (*f3)(); // Release
    int (__stdcall *GetBuffer)(void *client, unsigned int num_frames_requested, unsigned char **data);
    int (__stdcall *ReleaseBuffer)(void *client, unsigned int num_frames_written, int flags);
};

struct Win32Api_IAudioRenderClient
{
    Win32Api_IAudioRenderClientVtbl *vtbl;
};

struct Win32Api_IAudioCaptureClientVtbl
{
    void (*f1)(); // QueryInterface
    void (*f2)(); // AddRef
    void (*f3)(); // Release
    int (__stdcall *GetBuffer)(void *capture_client, unsigned char **data, unsigned int *num_frames_to_read, unsigned int *flags, u64 *device_position, u64 *qpc_position);
    int (__stdcall *ReleaseBuffer)(void *capture_client, unsigned int num_frames_read);
    int (__stdcall *GetNextPacketSize)(void *capture_client, unsigned int num_frames_in_next_packet);
};

struct Win32Api_IAudioCaptureClient
{
    Win32Api_IAudioCaptureClientVtbl *vtbl;
};

static Win32Api_GUID Win32Api_uuid_IMMDeviceEnumerator = { 0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6 };
static Win32Api_GUID Win32Api_uuid_MMDeviceEnumerator  = { 0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E };
static Win32Api_GUID Win32Api_uuid_IAudioClient        = { 0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2 };
static Win32Api_GUID Win32Api_uuid_IAudioRenderClient  = { 0xF294ACFC, 0x3146, 0x4483, 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2 };
static Win32Api_GUID Win32Api_uuid_IAudioCaptureClient = { 0xC8ADBD64, 0xE71E, 0x48a0, 0xA4, 0xDE, 0x18, 0x5C, 0x39, 0x5C, 0xD3, 0x17 };

#define WIN32_API_FUNCTION_LIST \
    WIN32_API(gdi32, ChoosePixelFormat, int __stdcall, (void *dc, Win32Api_PIXELFORMATDESCRIPTOR *pfd)) \
    WIN32_API(gdi32, DescribePixelFormat, int __stdcall, (void *dc, int pixel_format, unsigned int bytes, Win32Api_PIXELFORMATDESCRIPTOR *pfd)) \
    WIN32_API(gdi32, SetPixelFormat, int __stdcall, (void *dc, int format, Win32Api_PIXELFORMATDESCRIPTOR *pfd)) \
    WIN32_API(gdi32, SwapBuffers, int __stdcall, (void *dc)) \
    \
    WIN32_API(kernel32, CompareFileTime, int __stdcall, (Win32Api_FILETIME *filetime1, Win32Api_FILETIME *filetime2)) \
    WIN32_API(kernel32, CopyFileA, int __stdcall, (char *filename, char *new_filename, int fail_if_exists)) \
    WIN32_API(kernel32, CreateEventA, void * __stdcall, (void *event_attributes, int manual_reset, int initial_state, char *name)) \
    WIN32_API(kernel32, CreateThread, void * __stdcall, (void *thread_attributes, unsigned int stack_size, Win32Api_THREAD_START_ROUTINE proc, void *param, unsigned int creation_flags, unsigned int *thread_id)) \
    WIN32_API(kernel32, GetFileAttributesExA, int __stdcall, (char *filename, Win32Api_GET_FILEEX_INFO_LEVELS info_level_id, void *file_info)) \
    WIN32_API(kernel32, GetModuleFileNameA, unsigned int __stdcall, (void *module, char *filename, unsigned int size)) \
    WIN32_API(kernel32, GetModuleHandleA, void * __stdcall, (char *module)) \
    WIN32_API(kernel32, QueryPerformanceCounter, int __stdcall, (Win32Api_LARGE_INTEGER *perf_count)) \
    WIN32_API(kernel32, QueryPerformanceFrequency, int __stdcall, (Win32Api_LARGE_INTEGER *freq)) \
    WIN32_API(kernel32, SetThreadPriority, int __stdcall, (void *thread, int priority)) \
    WIN32_API(kernel32, VirtualAlloc, void * __stdcall, (void *addr, usize size, unsigned int alloc_type, unsigned int protect)) \
    WIN32_API(kernel32, VirtualFree, int __stdcall, (void *addr, usize size, usize free_type)) \
    WIN32_API(kernel32, WaitForSingleObject, unsigned int __stdcall, (void *handle, unsigned int milliseconds)) \
    WIN32_API(kernel32, WaitForMultipleObjects, unsigned int __stdcall, (unsigned int count, void **handles, int wait_all, unsigned int milliseconds)) \
    \
    WIN32_API(ole32, CoCreateInstance, int __stdcall, (Win32Api_GUID *rclsid, void *unk_outer, int cls_context, Win32Api_GUID *riid, void **ppv)) \
    WIN32_API(ole32, CoInitializeEx, int __stdcall, (void *reserved, int co_init)) \
    \
    WIN32_API(opengl32, wglCreateContext, void * __stdcall, (void *dc)) \
    WIN32_API(opengl32, wglDeleteContext, int __stdcall, (void *rc)) \
    WIN32_API(opengl32, wglGetProcAddress, Win32Api_PROC, (char *proc)) \
    WIN32_API(opengl32, wglMakeCurrent, int __stdcall, (void *dc, void *rc)) \
    \
    WIN32_API(user32, CreateWindowExA, void * __stdcall, (unsigned int ex_style, char *class_name, char *window_name, unsigned int style, int x, int y, int width, int height, void *parent, void *menu, void *instance, void *param)) \
    WIN32_API(user32, DefWindowProcA, intptr __stdcall, (void *wnd, unsigned int msg, uintptr wparam, intptr lparam)) \
    WIN32_API(user32, DestroyWindow, int __stdcall, (void *wnd)) \
    WIN32_API(user32, DispatchMessageA, intptr __stdcall, (Win32Api_MSG *msg)) \
    WIN32_API(user32, GetClientRect, int __stdcall, (void *wnd, Win32Api_RECT *rect)) \
    WIN32_API(user32, GetCursorPos, int __stdcall, (Win32Api_POINT *point)) \
    WIN32_API(user32, GetDC, void * __stdcall, (void *wnd)) \
    WIN32_API(user32, GetKeyState, short __stdcall, (int virtual_key)) \
    WIN32_API(user32, GetRawInputData, unsigned int __stdcall, (void *rawinput, unsigned int commad, void *data, unsigned int *cbSize, unsigned int cbSizeHeader)) \
    WIN32_API(user32, LoadCursorA, void * __stdcall, (void *instance, char *cursor_name)) \
    WIN32_API(user32, PeekMessageA, int __stdcall, (Win32Api_MSG *msg, void *wnd, unsigned int msg_filter_min, unsigned int msg_filter_max, unsigned int remove_msg)) \
    WIN32_API(user32, RegisterClassExA, unsigned short __stdcall, (Win32Api_WNDCLASSEXA *)) \
    WIN32_API(user32, RegisterRawInputDevices, int __stdcall, (Win32Api_RAWINPUTDEVICE *devices, unsigned int num_devices, unsigned int cbSize)) \
    WIN32_API(user32, ReleaseDC, int __stdcall, (void *wnd, void *dc)) \
    WIN32_API(user32, ScreenToClient, int __stdcall, (void *wnd, Win32Api_POINT *point)) \
    WIN32_API(user32, TranslateMessage, int __stdcall, (Win32Api_MSG *msg))

#define WIN32_API(dll, name, return_type, params) typedef return_type Win32Api_##name params;
WIN32_API_FUNCTION_LIST
#undef WIN32_API

struct Win32Api
{
    #define WIN32_API(dll, name, return_type, params) Win32Api_##name *name;
    WIN32_API_FUNCTION_LIST
    #undef WIN32_API
};

static void win32_init_win32_api(Win32Api *win32_api)
{
    void *user32 = win32_load_library("user32.dll");
    void *gdi32 = win32_load_library("gdi32.dll");
    void *opengl32 = win32_load_library("opengl32.dll");
    void *kernel32 = win32_load_library("kernel32.dll");
    void *ole32 = win32_load_library("ole32.dll");
    
    #define WIN32_API(dll, name, return_type, params) win32_api->name = (Win32Api_##name *)win32_get_proc_address(dll, #name);
    WIN32_API_FUNCTION_LIST
    #undef WIN32_API
}

struct Win32Window
{
    void *wnd;
    void *dc;
    void *rc;
    i32 width;
    i32 height;
};

struct Win32MemoryBlock
{
    PlatformMemoryBlock memblock;
    Win32MemoryBlock *prev;
    Win32MemoryBlock *next;
    u64 flags;
};

struct Win32State
{
    b32 running;
    Win32Window window;

    AppMemory app_memory;
    PlatformInput input;

    Mutex memory_mutex;
    Win32MemoryBlock memory_sentinel;

    b32 pause_scan_code_read;
};
