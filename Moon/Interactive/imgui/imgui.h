#pragma once
#include <cstring>
#include <utility>   // std::swap
#include <float.h>                  // FLT_MIN, FLT_MAX
#include <stdarg.h>                 // va_list, va_start, va_end
#include <stddef.h>                 // ptrdiff_t, NULL
#include <string.h>                 // memset, memmove, memcpy, strlen, strchr, strcpy, strcmp


// Helper Macros
// (note: compiling with NDEBUG will usually strip out assert() to nothing, which is NOT recommended because we use asserts to notify of programmer mistakes.)
#ifndef IM_ASSERT
#include <assert.h>
#define IM_ASSERT(_EXPR)            assert(_EXPR)                               // You can override the default assert handler by editing imconfig.h
#endif


namespace MOON{
//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------

// Scalar data types
typedef unsigned int        ImGuiID;// A unique ID used by widgets (typically the result of hashing a stack of string)
typedef signed char         ImS8;   // 8-bit signed integer
typedef unsigned char       ImU8;   // 8-bit unsigned integer
typedef signed short        ImS16;  // 16-bit signed integer
typedef unsigned short      ImU16;  // 16-bit unsigned integer
typedef signed int          ImS32;  // 32-bit signed integer == int
typedef unsigned int        ImU32;  // 32-bit unsigned integer (often used to store packed colors)
typedef signed   long long  ImS64;  // 64-bit signed integer
typedef unsigned long long  ImU64;  // 64-bit unsigned integer

// Forward declarations: ImDrawList, ImFontAtlas layer
struct ImDrawChannel;               // Temporary storage to output draw commands out of order, used by ImDrawListSplitter and ImDrawList::ChannelsSplit()
struct ImDrawCmd;                   // A single draw command within a parent ImDrawList (generally maps to 1 GPU draw call, unless it is a callback)
struct ImDrawData;                  // All draw command lists required to render the frame + pos/size coordinates to use for the projection matrix.
struct ImDrawList;                  // A single draw command list (generally one per window, conceptually you may see this as a dynamic "mesh" builder)
struct ImDrawListSharedData;        // Data shared among multiple draw lists (typically owned by parent ImGui context, but you may create one yourself)
struct ImDrawListSplitter;          // Helper to split a draw list into different layers which can be drawn into out of order, then flattened back.
struct ImDrawVert;                  // A single vertex (pos + uv + col = 20 bytes by default. Override layout with IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT)
struct ImFont;                      // Runtime data for a single font within a parent ImFontAtlas
struct ImFontAtlas;                 // Runtime data for multiple fonts, bake multiple fonts into a single texture, TTF/OTF font loader
struct ImFontAtlasBuilder;          // Opaque storage for building a ImFontAtlas
struct ImFontAtlasRect;             // Output of ImFontAtlas::GetCustomRect() when using custom rectangles.
struct ImFontBaked;                 // Baked data for a ImFont at a given size.
struct ImFontConfig;                // Configuration data when adding a font or merging fonts
struct ImFontGlyph;                 // A single font glyph (code point + coordinates within in ImFontAtlas + offset)
struct ImFontGlyphRangesBuilder;    // Helper to build glyph ranges from text/string data
struct ImFontLoader;                // Opaque interface to a font loading backend (stb_truetype, FreeType etc.).
struct ImTextureData;               // Specs and pixel storage for a texture used by Dear ImGui.
struct ImTextureRect;               // Coordinates of a rectangle within a texture.
struct ImColor;                     // Helper functions to create a color that can be converted to either u32 or float4 (*OBSOLETE* please avoid using)


enum ImGuiDir : int;                // -> enum ImGuiDir              // Enum: A cardinal direction (Left, Right, Up, Down)
enum ImGuiKey : int;                // -> enum ImGuiKey              // Enum: A key identifier (ImGuiKey_XXX or ImGuiMod_XXX value)
enum ImGuiMouseSource : int;        // -> enum ImGuiMouseSource      // Enum; A mouse input source identifier (Mouse, TouchScreen, Pen)
enum ImGuiSortDirection : ImU8;     // -> enum ImGuiSortDirection    // Enum: A sorting direction (ascending or descending)
typedef int ImGuiCol;               // -> enum ImGuiCol_             // Enum: A color identifier for styling
typedef int ImGuiCond;              // -> enum ImGuiCond_            // Enum: A condition for many Set*() functions
typedef int ImGuiDataType;          // -> enum ImGuiDataType_        // Enum: A primary data type
typedef int ImGuiMouseButton;       // -> enum ImGuiMouseButton_     // Enum: A mouse button identifier (0=left, 1=right, 2=middle)
typedef int ImGuiMouseCursor;       // -> enum ImGuiMouseCursor_     // Enum: A mouse cursor shape
typedef int ImGuiStyleVar;          // -> enum ImGuiStyleVar_        // Enum: A variable identifier for styling
typedef int ImGuiTableBgTarget;     // -> enum ImGuiTableBgTarget_   // Enum: A color target for TableSetBgColor()

typedef int ImDrawFlags;            // -> enum ImDrawFlags_          // Flags: for ImDrawList functions
typedef int ImDrawListFlags;        // -> enum ImDrawListFlags_      // Flags: for ImDrawList instance
typedef int ImDrawTextFlags;        // -> enum ImDrawTextFlags_      // Internal, do not use!
typedef int ImFontFlags;            // -> enum ImFontFlags_          // Flags: for ImFont
typedef int ImFontAtlasFlags;       // -> enum ImFontAtlasFlags_     // Flags: for ImFontAtlas

// Character types
// (we generally use UTF-8 encoded string in the API. This is storage specifically for a decoded character used for keyboard input and display)
typedef unsigned int ImWchar32;     // A single decoded U32 character/code point. We encode them as multi bytes UTF-8 when used in strings.
typedef unsigned short ImWchar16;   // A single decoded U16 character/code point. We encode them as multi bytes UTF-8 when used in strings.
#ifdef IMGUI_USE_WCHAR32            // ImWchar [configurable type: override in imconfig.h with '#define IMGUI_USE_WCHAR32' to support Unicode planes 1-16]
typedef ImWchar32 ImWchar;
#else
typedef ImWchar16 ImWchar;
#endif


typedef ImS64 ImGuiSelectionUserData;


typedef void*   (*ImGuiMemAllocFunc)(size_t sz, void* user_data);               // Function signature for ImGui::SetAllocatorFunctions()
typedef void    (*ImGuiMemFreeFunc)(void* ptr, void* user_data);                // Function signature for ImGui::SetAllocatorFunctions()

struct ImVec2
{
    float                                   x, y;
    constexpr ImVec2()                      : x(0.0f), y(0.0f) { }
    constexpr ImVec2(float _x, float _y)    : x(_x), y(_y) { }
    float& operator[] (size_t idx)          { IM_ASSERT(idx == 0 || idx == 1); return ((float*)(void*)(char*)this)[idx]; } // We very rarely use this [] operator, so the assert overhead is fine.
    float  operator[] (size_t idx) const    { IM_ASSERT(idx == 0 || idx == 1); return ((const float*)(const void*)(const char*)this)[idx]; }
};
// ImVec4: 4D vector used to store clipping rectangles, colors etc. [Compile-time configurable type]
struct ImVec4
{
    float                                                     x, y, z, w;
    constexpr ImVec4()                                        : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }
    constexpr ImVec4(float _x, float _y, float _z, float _w)  : x(_x), y(_y), z(_z), w(_w) { }
};
#ifndef ImTextureID
typedef ImU64 ImTextureID;      // Default: store up to 64-bits (any pointer or integer). A majority of backends are ok with that.
#endif

#ifndef ImTextureID_Invalid
#define ImTextureID_Invalid     ((ImTextureID)0)
#endif

struct ImTextureRef
{
    ImTextureRef()                          { _TexData = NULL; _TexID = ImTextureID_Invalid; }
    ImTextureRef(ImTextureID tex_id)        { _TexData = NULL; _TexID = tex_id; }
#if !defined(IMGUI_DISABLE_OBSOLETE_FUNCTIONS) && !defined(ImTextureID)
    ImTextureRef(void* tex_id)              { _TexData = NULL; _TexID = (ImTextureID)(size_t)tex_id; }  // For legacy backends casting to ImTextureID
#endif

    inline ImTextureID  GetTexID() const;   // == (_TexData ? _TexData->TexID : _TexID) // Implemented below in the file.

    // Members (either are set, never both!)
    ImTextureData*      _TexData;           //      A texture, generally owned by a ImFontAtlas. Will convert to ImTextureID during render loop, after texture has been uploaded.
    ImTextureID         _TexID;             // _OR_ Low-level backend texture identifier, if already uploaded or created by user/app. Generally provided to e.g. ImGui::Image() calls.
};

// Standard Drag and Drop payload types. You can define you own payload types using short strings. Types starting with '_' are defined by Dear ImGui.
#define IMGUI_PAYLOAD_TYPE_COLOR_3F     "_COL3F"    // float[3]: Standard type for colors, without alpha. User code may use this type.
#define IMGUI_PAYLOAD_TYPE_COLOR_4F     "_COL4F"    // float[4]: Standard type for colors. User code may use this type.

// A primary data type
enum ImGuiDataType_
{
    ImGuiDataType_S8,       // signed char / char (with sensible compilers)
    ImGuiDataType_U8,       // unsigned char
    ImGuiDataType_S16,      // short
    ImGuiDataType_U16,      // unsigned short
    ImGuiDataType_S32,      // int
    ImGuiDataType_U32,      // unsigned int
    ImGuiDataType_S64,      // long long / __int64
    ImGuiDataType_U64,      // unsigned long long / unsigned __int64
    ImGuiDataType_Float,    // float
    ImGuiDataType_Double,   // double
    ImGuiDataType_Bool,     // bool (provided for user convenience, not supported by scalar widgets)
    ImGuiDataType_String,   // char* (provided for user convenience, not supported by scalar widgets)
    ImGuiDataType_COUNT
};

// A cardinal direction
enum ImGuiDir : int
{
    ImGuiDir_None    = -1,
    ImGuiDir_Left    = 0,
    ImGuiDir_Right   = 1,
    ImGuiDir_Up      = 2,
    ImGuiDir_Down    = 3,
    ImGuiDir_COUNT
};

// A sorting direction
enum ImGuiSortDirection : ImU8
{
    ImGuiSortDirection_None         = 0,
    ImGuiSortDirection_Ascending    = 1,    // Ascending = 0->9, A->Z etc.
    ImGuiSortDirection_Descending   = 2     // Descending = 9->0, Z->A etc.
};
enum ImGuiCond_
{
    ImGuiCond_None          = 0,        // No condition (always set the variable), same as _Always
    ImGuiCond_Always        = 1 << 0,   // No condition (always set the variable), same as _None
    ImGuiCond_Once          = 1 << 1,   // Set the variable once per runtime session (only the first call will succeed)
    ImGuiCond_FirstUseEver  = 1 << 2,   // Set the variable if the object/window has no persistently saved data (no entry in .ini file)
    ImGuiCond_Appearing     = 1 << 3,   // Set the variable if the object/window is appearing after being hidden/inactive (or the first time)
};

struct ImNewWrapper {};
//inline void* operator new(size_t, ImNewWrapper, void* ptr) { return ptr; }
//inline void  operator delete(void*, ImNewWrapper, void*)   {} // This is only required so we can use the symmetrical new()

template<typename T>
struct ImVector
{
    int                 Size;
    int                 Capacity;
    T* Data;

    typedef T                   value_type;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;

    inline ImVector()
    {
        Size = Capacity = 0; Data = nullptr;
    }

    inline ImVector(const ImVector<T>& src)
    {
        Size = Capacity = 0; Data = nullptr;
        *this = src;
    }

    inline ImVector<T>& operator=(const ImVector<T>& src)
    {
        if (&src == this) return *this; // 自赋值保护
        clear();
        resize(src.Size);
        if (Data && src.Data)
            std::memcpy(Data, src.Data, (size_t)Size * sizeof(T));
        return *this;
    }

    inline ~ImVector()
    {
        if (Data)
        {
            ::operator delete(Data); // ✅释放原始内存，不用delete
            Data = nullptr;
        }
        Size = Capacity = 0;
    }

    inline void clear()
    {
        if (Data)
        {
            ::operator delete(Data);
            Data = nullptr;
        }
        Size = Capacity = 0;
    }

    inline void clear_delete()
    {
        for (int n = 0; n < Size; n++)
            delete(Data[n]);
        clear();
    }

    inline void clear_destruct()
    {
        for (int n = 0; n < Size; n++)
            Data[n].~T();
        clear();
    }

    inline bool empty() const { return Size == 0; }
    inline int size() const { return Size; }
    inline int size_in_bytes() const { return Size * (int)sizeof(T); }
    inline int max_size() const { return 0x7FFFFFFF / (int)sizeof(T); }
    inline int capacity() const { return Capacity; }

    inline T& operator[](int i) { IM_ASSERT(i >= 0 && i < Size); return Data[i]; }
    inline const T& operator[](int i) const { IM_ASSERT(i >= 0 && i < Size); return Data[i]; }

    inline T* begin() { return Data; }
    inline const T* begin() const { return Data; }
    inline T* end() { return Data + Size; }
    inline const T* end() const { return Data + Size; }

    inline T& front() { IM_ASSERT(Size > 0); return Data[0]; }
    inline const T& front() const { IM_ASSERT(Size > 0); return Data[0]; }
    inline T& back() { IM_ASSERT(Size > 0); return Data[Size - 1]; }
    inline const T& back() const { IM_ASSERT(Size > 0); return Data[Size - 1]; }

    inline void swap(ImVector<T>& rhs)
    {
        std::swap(Size, rhs.Size);
        std::swap(Capacity, rhs.Capacity);
        std::swap(Data, rhs.Data);
    }

    inline int _grow_capacity(int sz) const
    {
        int new_capacity = Capacity ? (Capacity + Capacity / 2) : 8;
        return new_capacity > sz ? new_capacity : sz;
    }

    inline void resize(int new_size)
    {
        if (new_size > Capacity)
            reserve(_grow_capacity(new_size));
        Size = new_size;
    }

    inline void resize(int new_size, const T& v)
    {
        if (new_size > Capacity)
            reserve(_grow_capacity(new_size));
        if (new_size > Size)
        {
            for (int n = Size; n < new_size; n++)
                std::memcpy(&Data[n], &v, sizeof(v));
        }
        Size = new_size;
    }

    inline void shrink(int new_size)
    {
        IM_ASSERT(new_size <= Size);
        Size = new_size;
    }

    inline void reserve(int new_capacity)
    {
        if (new_capacity <= Capacity) return;
        // ✅分配原始字节内存
        T* new_data = static_cast<T*>(::operator new((size_t)new_capacity * sizeof(T)));
        if (Data)
        {
            std::memcpy(new_data, Data, (size_t)Size * sizeof(T));
            ::operator delete(Data);
        }
        Data = new_data;
        Capacity = new_capacity;
    }

    inline void reserve_discard(int new_capacity)
    {
        if (new_capacity <= Capacity) return;
        if (Data)
            ::operator delete(Data);
        Data = static_cast<T*>(::operator new((size_t)new_capacity * sizeof(T)));
        Capacity = new_capacity;
    }

    inline void push_back(const T& v)
    {
        if (Size == Capacity)
            reserve(_grow_capacity(Size + 1));
        std::memcpy(&Data[Size], &v, sizeof(v));
        Size++;
    }

    inline void pop_back()
    {
        IM_ASSERT(Size > 0);
        Size--;
    }

    inline void push_front(const T& v)
    {
        if (Size == 0)
            push_back(v);
        else
            insert(Data, v);
    }

    inline T* erase(const T* it)
    {
        IM_ASSERT(it >= Data && it < Data + Size);
        const ptrdiff_t off = it - Data;
        std::memmove(Data + off, Data + off + 1, ((size_t)Size - (size_t)off - 1) * sizeof(T));
        Size--;
        return Data + off;
    }

    inline T* erase(const T* it, const T* it_last)
    {
        IM_ASSERT(it >= Data && it < Data + Size && it_last >= it && it_last <= Data + Size);
        const ptrdiff_t count = it_last - it;
        const ptrdiff_t off = it - Data;
        std::memmove(Data + off, Data + off + count, ((size_t)Size - (size_t)off - (size_t)count) * sizeof(T));
        Size -= static_cast<int>(count);
        return Data + off;
    }

    inline T* erase_unsorted(const T* it)
    {
        IM_ASSERT(it >= Data && it < Data + Size);
        const ptrdiff_t off = it - Data;
        if (it < Data + Size - 1)
            std::memcpy(Data + off, Data + Size - 1, sizeof(T));
        Size--;
        return Data + off;
    }

    inline T* insert(const T* it, const T& v)
    {
        IM_ASSERT(it >= Data && it <= Data + Size);
        const ptrdiff_t off = it - Data;
        if (Size == Capacity)
            reserve(_grow_capacity(Size + 1));
        if (off < (int)Size)
            std::memmove(Data + off + 1, Data + off, ((size_t)Size - (size_t)off) * sizeof(T));
        std::memcpy(&Data[off], &v, sizeof(v));
        Size++;
        return Data + off;
    }

    inline bool contains(const T& v) const
    {
        const T* data = Data;
        const T* data_end = Data + Size;
        while (data < data_end)
            if (*data++ == v) return true;
        return false;
    }

    inline T* find(const T& v)
    {
        T* data = Data;
        const T* data_end = Data + Size;
        while (data < data_end)
        {
            if (*data == v) break;
            ++data;
        }
        return data;
    }

    inline const T* find(const T& v) const
    {
        const T* data = Data;
        const T* data_end = Data + Size;
        while (data < data_end)
        {
            if (*data == v) break;
            ++data;
        }
        return data;
    }

    inline int find_index(const T& v) const
    {
        const T* data_end = Data + Size;
        const T* it = find(v);
        if (it == data_end) return -1;
        const ptrdiff_t off = it - Data;
        return static_cast<int>(off);
    }

    inline bool find_erase(const T& v)
    {
        const T* it = find(v);
        if (it < Data + Size)
        {
            erase(it);
            return true;
        }
        return false;
    }

    inline bool find_erase_unsorted(const T& v)
    {
        const T* it = find(v);
        if (it < Data + Size)
        {
            erase_unsorted(it);
            return true;
        }
        return false;
    }

    inline int index_from_ptr(const T* it) const
    {
        IM_ASSERT(it >= Data && it < Data + Size);
        const ptrdiff_t off = it - Data;
        return static_cast<int>(off);
    }
};

struct ImGuiTextBuffer
{
    ImVector<char>      Buf;
     static char EmptyString[1];

    ImGuiTextBuffer()   { }
    inline char         operator[](int i) const { IM_ASSERT(Buf.Data != NULL); return Buf.Data[i]; }
    const char*         begin() const           { return Buf.Data ? &Buf.front() : EmptyString; }
    const char*         end() const             { return Buf.Data ? &Buf.back() : EmptyString; } // Buf is zero-terminated, so end() will point on the zero-terminator
    int                 size() const            { return Buf.Size ? Buf.Size - 1 : 0; }
    bool                empty() const           { return Buf.Size <= 1; }
    void                clear()                 { Buf.clear(); }
    void                resize(int size)        { if (Buf.Size > size) Buf.Data[size] = 0; Buf.resize(size ? size + 1 : 0, 0); } // Similar to resize(0) on ImVector: empty string but don't free buffer.
    void                reserve(int capacity)   { Buf.reserve(capacity); }
    const char*         c_str() const           { return Buf.Data ? Buf.Data : EmptyString; }
     void      append(const char* str, const char* str_end = NULL);
     void      appendf(const char* fmt, ...) ;
     void      appendfv(const char* fmt, va_list args) ;
};


// Helpers: ImVec2/ImVec4 operators
// - It is important that we are keeping those disabled by default so they don't leak in user space.
// - This is in order to allow user enabling implicit cast operators between ImVec2/ImVec4 and their own types (using IM_VEC2_CLASS_EXTRA in imconfig.h)
// - Add '#define IMGUI_DEFINE_MATH_OPERATORS' before including this file (or in imconfig.h) to access courtesy maths operators for ImVec2 and ImVec4.
#ifdef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED

// ImVec2 operators
inline ImVec2  operator*(const ImVec2& lhs, const float rhs)    { return ImVec2(lhs.x * rhs, lhs.y * rhs); }
inline ImVec2  operator*(const float lhs, const ImVec2& rhs)    { return ImVec2(lhs * rhs.x, lhs * rhs.y); }
inline ImVec2  operator/(const ImVec2& lhs, const float rhs)    { return ImVec2(lhs.x / rhs, lhs.y / rhs); }
inline ImVec2  operator+(const ImVec2& lhs, const ImVec2& rhs)  { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
inline ImVec2  operator-(const ImVec2& lhs, const ImVec2& rhs)  { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
inline ImVec2  operator*(const ImVec2& lhs, const ImVec2& rhs)  { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
inline ImVec2  operator/(const ImVec2& lhs, const ImVec2& rhs)  { return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y); }
inline ImVec2  operator+(const ImVec2& lhs)                     { return lhs; }
inline ImVec2  operator-(const ImVec2& lhs)                     { return ImVec2(-lhs.x, -lhs.y); }
inline ImVec2& operator*=(ImVec2& lhs, const float rhs)         { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
inline ImVec2& operator/=(ImVec2& lhs, const float rhs)         { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
inline ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs)       { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
inline ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs)       { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
inline ImVec2& operator*=(ImVec2& lhs, const ImVec2& rhs)       { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
inline ImVec2& operator/=(ImVec2& lhs, const ImVec2& rhs)       { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }
inline bool    operator==(const ImVec2& lhs, const ImVec2& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
inline bool    operator!=(const ImVec2& lhs, const ImVec2& rhs) { return lhs.x != rhs.x || lhs.y != rhs.y; }
// ImVec4 operators
inline ImVec4  operator*(const ImVec4& lhs, const float rhs)    { return ImVec4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs); }
inline ImVec4  operator*(const float lhs, const ImVec4& rhs)    { return ImVec4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w); }
inline ImVec4  operator/(const ImVec4& lhs, const float rhs)    { return ImVec4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs); }
inline ImVec4  operator+(const ImVec4& lhs, const ImVec4& rhs)  { return ImVec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
inline ImVec4  operator-(const ImVec4& lhs, const ImVec4& rhs)  { return ImVec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); }
inline ImVec4  operator*(const ImVec4& lhs, const ImVec4& rhs)  { return ImVec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }
inline ImVec4  operator/(const ImVec4& lhs, const ImVec4& rhs)  { return ImVec4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w); }
inline ImVec4  operator+(const ImVec4& lhs)                     { return lhs; }
inline ImVec4  operator-(const ImVec4& lhs)                     { return ImVec4(-lhs.x, -lhs.y, -lhs.z, -lhs.w); }
inline ImVec4& operator*=(ImVec4& lhs, const float rhs)         { lhs.x *= rhs; lhs.y *= rhs; lhs.z *= rhs; lhs.w *= rhs; return lhs; }
inline ImVec4& operator/=(ImVec4& lhs, const float rhs)         { lhs.x /= rhs; lhs.y /= rhs; lhs.z /= rhs; lhs.w /= rhs; return lhs; }
inline ImVec4& operator+=(ImVec4& lhs, const ImVec4& rhs)       { lhs.x += rhs.x; lhs.y += rhs.y; lhs.z += rhs.z; lhs.w += rhs.w; return lhs; }
inline ImVec4& operator-=(ImVec4& lhs, const ImVec4& rhs)       { lhs.x -= rhs.x; lhs.y -= rhs.y; lhs.z -= rhs.z; lhs.w -= rhs.w; return lhs; }
inline ImVec4& operator*=(ImVec4& lhs, const ImVec4& rhs)       { lhs.x *= rhs.x; lhs.y *= rhs.y; lhs.z *= rhs.z; lhs.w *= rhs.w; return lhs; }
inline ImVec4& operator/=(ImVec4& lhs, const ImVec4& rhs)       { lhs.x /= rhs.x; lhs.y /= rhs.y; lhs.z /= rhs.z; lhs.w /= rhs.w; return lhs; }
inline bool    operator==(const ImVec4& lhs, const ImVec4& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w; }
inline bool    operator!=(const ImVec4& lhs, const ImVec4& rhs) { return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z || lhs.w != rhs.w; }

#endif

// Helpers macros to generate 32-bit encoded colors
// - User can declare their own format by #defining the 5 _SHIFT/_MASK macros in their imconfig file.
// - Any setting other than the default will need custom backend support. The only standard backend that supports anything else than the default is DirectX9.
#ifndef IM_COL32_R_SHIFT
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
#define IM_COL32_R_SHIFT    16
#define IM_COL32_G_SHIFT    8
#define IM_COL32_B_SHIFT    0
#define IM_COL32_A_SHIFT    24
#define IM_COL32_A_MASK     0xFF000000
#else
#define IM_COL32_R_SHIFT    0
#define IM_COL32_G_SHIFT    8
#define IM_COL32_B_SHIFT    16
#define IM_COL32_A_SHIFT    24
#define IM_COL32_A_MASK     0xFF000000
#endif
#endif
#define IM_COL32(R,G,B,A)    (((ImU32)(A)<<IM_COL32_A_SHIFT) | ((ImU32)(B)<<IM_COL32_B_SHIFT) | ((ImU32)(G)<<IM_COL32_G_SHIFT) | ((ImU32)(R)<<IM_COL32_R_SHIFT))
#define IM_COL32_WHITE       IM_COL32(255,255,255,255)  // Opaque white = 0xFFFFFFFF
#define IM_COL32_BLACK       IM_COL32(0,0,0,255)        // Opaque black
#define IM_COL32_BLACK_TRANS IM_COL32(0,0,0,0)          // Transparent black = 0x00000000


struct ImColor
{
    ImVec4          Value;

    constexpr ImColor()                                             { }
    constexpr ImColor(float r, float g, float b, float a = 1.0f)    : Value(r, g, b, a) { }
    constexpr ImColor(const ImVec4& col)                            : Value(col) {}
    constexpr ImColor(int r, int g, int b, int a = 255)             : Value((float)r * (1.0f / 255.0f), (float)g * (1.0f / 255.0f), (float)b * (1.0f / 255.0f), (float)a* (1.0f / 255.0f)) {}
    constexpr ImColor(ImU32 rgba)                                   : Value((float)((rgba >> IM_COL32_R_SHIFT) & 0xFF) * (1.0f / 255.0f), (float)((rgba >> IM_COL32_G_SHIFT) & 0xFF) * (1.0f / 255.0f), (float)((rgba >> IM_COL32_B_SHIFT) & 0xFF) * (1.0f / 255.0f), (float)((rgba >> IM_COL32_A_SHIFT) & 0xFF) * (1.0f / 255.0f)) {}
    inline operator ImU32() const { return 0;/*ImGui::ColorConvertFloat4ToU32(Value);*/ }
    inline operator ImVec4() const                                  { return Value; }

    // FIXME-OBSOLETE: May need to obsolete/cleanup those helpers.
    inline void    SetHSV(float h, float s, float v, float a = 1.0f){ /*ImGui::ColorConvertHSVtoRGB(h, s, v, Value.x, Value.y, Value.z)*/; Value.w = a; }
    static ImColor HSV(float h, float s, float v, float a = 1.0f)   { float r, g, b; /*ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b);*/ return ImColor(r, g, b, a); }
};


//-----------------------------------------------------------------------------
// [SECTION] Drawing API (ImDrawCmd, ImDrawIdx, ImDrawVert, ImDrawChannel, ImDrawListSplitter, ImDrawListFlags, ImDrawList, ImDrawData)
// Hold a series of drawing commands. The user provides a renderer for ImDrawData which essentially contains an array of ImDrawList.
//-----------------------------------------------------------------------------

// The maximum line width to bake anti-aliased textures for. Build atlas with ImFontAtlasFlags_NoBakedLines to disable baking.
#ifndef IM_DRAWLIST_TEX_LINES_WIDTH_MAX
#define IM_DRAWLIST_TEX_LINES_WIDTH_MAX     (32)
#endif

// ImDrawIdx: vertex index. [Compile-time configurable type]
// - To use 16-bit indices + allow large meshes: backend need to set 'io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset' and handle ImDrawCmd::VtxOffset (recommended).
// - To use 32-bit indices: override with '#define ImDrawIdx unsigned int' in your imconfig.h file.
#ifndef ImDrawIdx
typedef unsigned int ImDrawIdx;   // Default: 16-bit (for maximum compatibility with renderer backends)
#endif



// Typically, 1 command = 1 GPU draw call (unless command is a callback)
// - VtxOffset: When 'io.BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset' is enabled,
//   this fields allow us to render meshes larger than 64K vertices while keeping 16-bit indices.
//   Backends made for <1.71. will typically ignore the VtxOffset fields.
// - The ClipRect/TexRef/VtxOffset fields must be contiguous as we memcmp() them together (this is asserted for).
struct ImDrawCmd
{
    ImVec4          ClipRect;           // 4*4  // Clipping rectangle (x1, y1, x2, y2). Subtract ImDrawData->DisplayPos to get clipping rectangle in "viewport" coordinates
    ImTextureRef    TexRef;             // 16   // Reference to a font/texture atlas (where backend called ImTextureData::SetTexID()) or to a user-provided texture ID (via e.g. ImGui::Image() calls). Both will lead to a ImTextureID value.
    unsigned int    VtxOffset;          // 4    // Start offset in vertex buffer. ImGuiBackendFlags_RendererHasVtxOffset: always 0, otherwise may be >0 to support meshes larger than 64K vertices with 16-bit indices.
    unsigned int    IdxOffset;          // 4    // Start offset in index buffer.
    unsigned int    ElemCount;          // 4    // Number of indices (multiple of 3) to be rendered as triangles. Vertices are stored in the callee ImDrawList's vtx_buffer[] array, indices in idx_buffer[].
    //ImDrawCallback  UserCallback;       // 4-8  // If != NULL, call the function instead of rendering the vertices. clip_rect and texture_id will be set normally.
    void*           UserCallbackData;   // 4-8  // Callback user data (when UserCallback != NULL). If called AddCallback() with size == 0, this is a copy of the AddCallback() argument. If called AddCallback() with size > 0, this is pointing to a buffer where data is stored.
    int             UserCallbackDataSize;  // 4 // Size of callback user data when using storage, otherwise 0.
    int             UserCallbackDataOffset;// 4 // [Internal] Offset of callback user data when using storage, otherwise -1.

    ImDrawCmd()     { memset((void*)this, 0, sizeof(*this)); } // Also ensure our padding fields are zeroed

    // Since 1.83: returns ImTextureID associated with this draw call. Warning: DO NOT assume this is always same as 'TextureId' (we will change this function for an upcoming feature)
    // Since 1.92: removed ImDrawCmd::TextureId field, the getter function must be used!
    inline ImTextureID GetTexID() const;    // == (TexRef._TexData ? TexRef._TexData->TexID : TexRef._TexID)
};


struct ImDrawVert
{
    ImVec2  pos;
    ImVec2  uv;
    ImU32   col;
};


// [Internal] For use by ImDrawList
struct ImDrawCmdHeader
{
    ImVec4          ClipRect;
    ImTextureRef    TexRef;
    unsigned int    VtxOffset;
};

// [Internal] For use by ImDrawListSplitter
struct ImDrawChannel
{
    ImVector<ImDrawCmd>         _CmdBuffer;
    ImVector<ImDrawIdx>         _IdxBuffer;
};



// Flags for ImDrawList functions
enum ImDrawFlags_
{
    ImDrawFlags_None                        = 0,

    // Rounding for AddRect(), AddRectFilled(), PathRect()
    // - When not specified, we defaults to ImDrawFlags_RoundCornersAll! So you only need to use those flags if you want another configuration.
    ImDrawFlags_RoundCornersTopLeft         = 1 << 4, // Round top-left corner only (when rounding > 0.0f, we default to all corners).
    ImDrawFlags_RoundCornersTopRight        = 1 << 5, // Round top-right corner only (when rounding > 0.0f, we default to all corners).
    ImDrawFlags_RoundCornersBottomLeft      = 1 << 6, // Round bottom-left corner only (when rounding > 0.0f, we default to all corners).
    ImDrawFlags_RoundCornersBottomRight     = 1 << 7, // Round bottom-right corner only (when rounding > 0.0f, we default to all corners).
    ImDrawFlags_RoundCornersNone            = 1 << 8, // Disable rounding even if `float rounding > 0.0f`. This is NOT zero, NOT an implicit flag!
    ImDrawFlags_RoundCornersAll             = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight, // (Default!!)
    ImDrawFlags_RoundCornersDefault_        = ImDrawFlags_RoundCornersAll, // Default to ALL corners if none of the _RoundCornersXX flags are specified!
    ImDrawFlags_RoundCornersTop             = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight,
    ImDrawFlags_RoundCornersBottom          = ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight,
    ImDrawFlags_RoundCornersLeft            = ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersTopLeft,
    ImDrawFlags_RoundCornersRight           = ImDrawFlags_RoundCornersBottomRight | ImDrawFlags_RoundCornersTopRight,
    ImDrawFlags_RoundCornersMask_           = ImDrawFlags_RoundCornersAll | ImDrawFlags_RoundCornersNone,

    // Stroke options
    ImDrawFlags_Closed                      = 1 << 9, // PathStroke(), AddPolyline(): specify that shape should be closed.
    //ImDrawFlags_Closed                    = 1,      // Prior to 1.92.8 (May 2026), ImDrawFlags_Closed was guaranteed to be == 1<<0 == 1 for legacy compatibility reason. Hardcoded use of 1 or true should be replaced.

    ImDrawFlags_InvalidMask_                = ~0x7FFFFFF0, // == 0x8000000F,
};

// Flags for ImDrawList instance. Those are set automatically by ImGui:: functions from ImGuiIO settings, and generally not manipulated directly.
// It is however possible to temporarily alter flags between calls to ImDrawList:: functions.
enum ImDrawListFlags_
{
    ImDrawListFlags_None                    = 0,
    ImDrawListFlags_AntiAliasedLines        = 1 << 0,  // Enable anti-aliased lines/borders (*2 the number of triangles for 1.0f wide line or lines thin enough to be drawn using textures, otherwise *3 the number of triangles)
    ImDrawListFlags_AntiAliasedLinesUseTex  = 1 << 1,  // Enable anti-aliased lines/borders using textures when possible. Require backend to render with bilinear filtering (NOT point/nearest filtering).
    ImDrawListFlags_AntiAliasedFill         = 1 << 2,  // Enable anti-aliased edge around filled shapes (rounded rectangles, circles).
    ImDrawListFlags_AllowVtxOffset          = 1 << 3,  // Can emit 'VtxOffset > 0' to allow large meshes. Set when 'ImGuiBackendFlags_RendererHasVtxOffset' is enabled.
    ImDrawListFlags_TextNoPixelSnap         = 1 << 4,  // Disable automatically snapping AddText() calls to pixel boundaries.
};
struct ImDrawList
{
    // This is what you have to render
    ImVector<ImDrawCmd>     CmdBuffer;          // Draw commands. Typically 1 command = 1 GPU draw call, unless the command is a callback.
    ImVector<ImDrawIdx>     IdxBuffer;          // Index buffer. Each command consume ImDrawCmd::ElemCount of those
    ImVector<ImDrawVert>    VtxBuffer;          // Vertex buffer.
    ImDrawListFlags         Flags;              // Flags, you may poke into these to adjust anti-aliasing settings per-primitive.

    // [Internal, used while building lists]
    unsigned int            _VtxCurrentIdx;     // [Internal] generally == VtxBuffer.Size unless we are past 64K vertices, in which case this gets reset to 0.
    ImDrawListSharedData*   _Data;              // Pointer to shared draw data (you can use ImGui::GetDrawListSharedData() to get the one from current ImGui context)
    ImDrawVert*             _VtxWritePtr;       // [Internal] point within VtxBuffer.Data after each add command (to avoid using the ImVector<> operators too much)
    ImDrawIdx*              _IdxWritePtr;       // [Internal] point within IdxBuffer.Data after each add command (to avoid using the ImVector<> operators too much)
    ImVector<ImVec2>        _Path;              // [Internal] current path building
    ImDrawCmdHeader         _CmdHeader;         // [Internal] template of active commands. Fields should match those of CmdBuffer.back().
    //ImDrawListSplitter      _Splitter;          // [Internal] for channels api (note: prefer using your own persistent instance of ImDrawListSplitter!)
    ImVector<ImVec4>        _ClipRectStack;     // [Internal]
    ImVector<ImTextureRef>  _TextureStack;      // [Internal]
    ImVector<ImU8>          _CallbacksDataBuf;  // [Internal]
    float                   _FringeScale;       // [Internal] anti-alias fringe is scaled by this value, this helps to keep things sharp while zooming at vertex buffer content.
    float                   _InvFringeScale;    // [internal] 1.0 / _FringeScale // FIXME: Consider renaming to _PixelDensity.
    const char*             _OwnerName;         // Pointer to owner window's name for debugging

     ImDrawList(ImDrawListSharedData* shared_data);
     ImDrawList();
     ~ImDrawList();

     void  PushClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect = false);  // Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
     void  PushClipRectFullScreen();
     void  PopClipRect();
     void  PushTexture(ImTextureRef tex_ref);
     void  PopTexture();
    inline ImVec2   GetClipRectMin() const { const ImVec4& cr = _ClipRectStack.back(); return ImVec2(cr.x, cr.y); }
    inline ImVec2   GetClipRectMax() const { const ImVec4& cr = _ClipRectStack.back(); return ImVec2(cr.z, cr.w); }


     void  AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness = 1.0f);
     void  AddLineH(float min_x, float max_x, float y, ImU32 col, float thickness = 1.0f);
     void  AddLineV(float x, float min_y, float max_y, ImU32 col, float thickness = 1.0f);
     void  AddRect(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding = 0.0f, float thickness = 1.0f, ImDrawFlags flags = 0);   // a: upper-left, b: lower-right (== upper-left + size)
     void  AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding = 0.0f, ImDrawFlags flags = 0);                     // a: upper-left, b: lower-right (== upper-left + size)
     void  AddRectFilledMultiColor(const ImVec2& p_min, const ImVec2& p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left);
     void  AddQuad(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness = 1.0f);
     void  AddQuadFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col);
     void  AddTriangle(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness = 1.0f);
     void  AddTriangleFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col);
     void  AddCircle(const ImVec2& center, float radius, ImU32 col, int num_segments = 0, float thickness = 1.0f);
     void  AddCircleFilled(const ImVec2& center, float radius, ImU32 col, int num_segments = 0);
     void  AddNgon(const ImVec2& center, float radius, ImU32 col, int num_segments, float thickness = 1.0f);
     void  AddNgonFilled(const ImVec2& center, float radius, ImU32 col, int num_segments);
     void  AddEllipse(const ImVec2& center, const ImVec2& radius, ImU32 col, float rot = 0.0f, int num_segments = 0, float thickness = 1.0f);
     void  AddEllipseFilled(const ImVec2& center, const ImVec2& radius, ImU32 col, float rot = 0.0f, int num_segments = 0);
     void  AddText(const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end = NULL);
     void  AddText(ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const ImVec4* cpu_fine_clip_rect = NULL);
     void  AddBezierCubic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness, int num_segments = 0); // Cubic Bezier (4 control points)
     void  AddBezierQuadratic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness, int num_segments = 0);               // Quadratic Bezier (3 control points)

     void  AddPolyline(const ImVec2* points, int num_points, ImU32 col, float thickness, ImDrawFlags flags = 0);
     void  AddConvexPolyFilled(const ImVec2* points, int num_points, ImU32 col);
     void  AddConcavePolyFilled(const ImVec2* points, int num_points, ImU32 col);
     void  AddImage(ImTextureRef tex_ref, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min = ImVec2(0, 0), const ImVec2& uv_max = ImVec2(1, 1), ImU32 col = IM_COL32_WHITE);
     void  AddImageQuad(ImTextureRef tex_ref, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, const ImVec2& uv1 = ImVec2(0, 0), const ImVec2& uv2 = ImVec2(1, 0), const ImVec2& uv3 = ImVec2(1, 1), const ImVec2& uv4 = ImVec2(0, 1), ImU32 col = IM_COL32_WHITE);
     void  AddImageRounded(ImTextureRef tex_ref, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min, const ImVec2& uv_max, ImU32 col, float rounding, ImDrawFlags flags = 0);

    inline    void  PathClear()                                                 { _Path.Size = 0; }
    inline    void  PathLineTo(const ImVec2& pos)                               { _Path.push_back(pos); }
    inline    void  PathLineToMergeDuplicate(const ImVec2& pos)                 { if (_Path.Size == 0 || memcmp(&_Path.Data[_Path.Size - 1], &pos, 8) != 0) _Path.push_back(pos); }
    inline    void  PathFillConvex(ImU32 col)                                   { AddConvexPolyFilled(_Path.Data, _Path.Size, col); _Path.Size = 0; }
    inline    void  PathFillConcave(ImU32 col)                                  { AddConcavePolyFilled(_Path.Data, _Path.Size, col); _Path.Size = 0; }
    inline    void  PathStroke(ImU32 col, float thickness = 1.0f, ImDrawFlags flags = 0) { AddPolyline(_Path.Data, _Path.Size, col, thickness, flags); _Path.Size = 0; }
     void  PathArcTo(const ImVec2& center, float radius, float a_min, float a_max, int num_segments = 0);
     void  PathArcToFast(const ImVec2& center, float radius, int a_min_of_12, int a_max_of_12);                // Use precomputed angles for a 12 steps circle
     void  PathEllipticalArcTo(const ImVec2& center, const ImVec2& radius, float rot, float a_min, float a_max, int num_segments = 0); // Ellipse
     void  PathBezierCubicCurveTo(const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, int num_segments = 0); // Cubic Bezier (4 control points)
     void  PathBezierQuadraticCurveTo(const ImVec2& p2, const ImVec2& p3, int num_segments = 0);               // Quadratic Bezier (3 control points)
     void  PathRect(const ImVec2& rect_min, const ImVec2& rect_max, float rounding = 0.0f, ImDrawFlags flags = 0);
    // Advanced: Miscellaneous
     void  AddDrawCmd();                                               // This is useful if you need to forcefully create a new draw call (to allow for dependent rendering / blending). Otherwise primitives are merged into the same draw-call as much as possible
     ImDrawList* CloneOutput() const;                                  // Create a clone of the CmdBuffer/IdxBuffer/VtxBuffer. For multi-threaded rendering, consider using `imgui_threaded_rendering` from https://github.com/ocornut/imgui_club instead.

    inline void     ChannelsSplit(int count)    { /*_Splitter.Split(this, count);*/ }
    inline void     ChannelsMerge()             { /*_Splitter.Merge(this);*/ }
    inline void     ChannelsSetCurrent(int n)   { /*_Splitter.SetCurrentChannel(this, n);*/ }

    // Advanced: Primitives allocations
    // - We render triangles (three vertices)
    // - All primitives needs to be reserved via PrimReserve() beforehand.
     void  PrimReserve(int idx_count, int vtx_count);
     void  PrimUnreserve(int idx_count, int vtx_count);
     void  PrimRect(const ImVec2& a, const ImVec2& b, ImU32 col);      // Axis aligned rectangle (composed of two triangles)
     void  PrimRectUV(const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, ImU32 col);
     void  PrimQuadUV(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col);
    inline    void  PrimWriteVtx(const ImVec2& pos, const ImVec2& uv, ImU32 col)    { _VtxWritePtr->pos = pos; _VtxWritePtr->uv = uv; _VtxWritePtr->col = col; _VtxWritePtr++; _VtxCurrentIdx++; }
    inline    void  PrimWriteIdx(ImDrawIdx idx)                                     { *_IdxWritePtr = idx; _IdxWritePtr++; }
    inline    void  PrimVtx(const ImVec2& pos, const ImVec2& uv, ImU32 col)         { PrimWriteIdx((ImDrawIdx)_VtxCurrentIdx); PrimWriteVtx(pos, uv, col); } // Write vertex with unique index

    // Obsolete names
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    inline    void  AddRect(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, ImDrawFlags flags, float thickness) { AddRect(p_min, p_max, col, rounding, thickness, flags); } // OBSOLETED in 1.92.8: NEW FUNCTION SIGNATURE HAS 'thickness' AND 'flags' SWAPPED.
    inline    void  AddPolyline(const ImVec2* points, int num_points, ImU32 col, ImDrawFlags flags, float thickness)                 { AddPolyline(points, num_points, col, thickness, flags); } // OBSOLETED in 1.92.8: NEW FUNCTION SIGNATURE HAS 'thickness' AND 'flags' SWAPPED.
    inline    void  PathStroke(ImU32 col, ImDrawFlags flags, float thickness)                                                        { PathStroke(col, thickness, flags); }                      // OBSOLETED in 1.92.8: NEW FUNCTION SIGNATURE HAS 'thickness' AND 'flags' SWAPPED.
    inline    void  PushTextureID(ImTextureRef tex_ref) { PushTexture(tex_ref); }   // RENAMED in 1.92.0
    inline    void  PopTextureID()                      { PopTexture(); }           // RENAMED in 1.92.0
#else
              void  AddRect(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding /*= 0.0f*/, ImDrawFlags flags /*= 0*/, float thickness /*= 1.0f*/) = delete;
              void  AddPolyline(const ImVec2* points, int num_points, ImU32 col, ImDrawFlags flags, float thickness) = delete;
    inline    void  PathStroke(ImU32 col, ImDrawFlags flags /*= 0*/, float thickness /*= 1.0f*/) = delete;
#endif
    // [Internal helpers]
     void  _SetDrawListSharedData(ImDrawListSharedData* data);
     void  _ResetForNewFrame();
     void  _SetPixelDensity(float pixel_density);
     void  _ClearFreeMemory();
     void  _PopUnusedDrawCmd();
     void  _TryMergeDrawCmds();
     void  _OnChangedClipRect();
     void  _OnChangedTexture();
     void  _OnChangedVtxOffset();
     void  _SetTexture(ImTextureRef tex_ref);
     int   _CalcCircleAutoSegmentCount(float radius) const;
     void  _PathArcToFastEx(const ImVec2& center, float radius, int a_min_sample, int a_max_sample, int a_step);
     void  _PathArcToN(const ImVec2& center, float radius, float a_min, float a_max, int num_segments);
};

// All draw data to render a Dear ImGui frame
// (NB: the style and the naming convention here is a little inconsistent, we currently preserve them for backward compatibility purpose,
// as this is one of the oldest structure exposed by the library! Basically, ImDrawList == CmdList)
struct ImDrawData
{
    bool                Valid;              // Only valid after Render() is called and before the next NewFrame() is called.
    int                 FrameCount;         // Frame counter of the emitter context. Mostly for debugging purpose.
    int                 TotalIdxCount;      // For convenience, sum of all ImDrawList's IdxBuffer.Size
    int                 TotalVtxCount;      // For convenience, sum of all ImDrawList's VtxBuffer.Size
    ImVector<ImDrawList*> CmdLists;         // Array of ImDrawList* to render. The ImDrawLists are owned by ImGuiContext and only pointed to from here.
    ImVec2              DisplayPos;         // Top-left position of the viewport to render (== top-left of the orthogonal projection matrix to use) (== GetMainViewport()->Pos for the main viewport, == (0.0) in most single-viewport applications)
    ImVec2              DisplaySize;        // Size of the viewport to render (== GetMainViewport()->Size for the main viewport, == io.DisplaySize in most single-viewport applications)
    ImVec2              FramebufferScale;   // Amount of pixels for each unit of DisplaySize. Copied from viewport->FramebufferScale (== io.DisplayFramebufferScale for main viewport). Generally (1,1) on normal display, (2,2) on OSX with Retina display.
   
    ImVector<ImTextureData*>* Textures;     // List of textures to update. Most of the times the list is shared by all ImDrawData, has only 1 texture and it doesn't need any update. This almost always points to ImGui::GetPlatformIO().Textures[]. May be overridden or set to NULL if you want to manually update textures.

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    int                 CmdListsCount;      // Use CmdLists.Size instead. Number of ImDrawList* to render.
#endif

    // Functions
    ImDrawData()    { Clear(); }
     void  Clear();
     void  AddDrawList(ImDrawList* draw_list);     // Helper to add an external draw list into an existing ImDrawData.
     void  DeIndexAllBuffers();                    // Helper to convert all buffers from indexed to non-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
     void  ScaleClipRects(const ImVec2& fb_scale); // Helper to scale the ClipRect field of each ImDrawCmd. Use if your final output buffer is at a different scale than Dear ImGui expects, or if there is a difference between your window resolution and framebuffer resolution.
};
#undef Status // X11 headers are leaking this.


enum ImTextureFormat
{
    ImTextureFormat_RGBA32,         // 4 components per pixel, each is unsigned 8-bit. Total size = TexWidth * TexHeight * 4
    ImTextureFormat_Alpha8,         // 1 component per pixel, each is unsigned 8-bit. Total size = TexWidth * TexHeight
};

// Status of a texture to communicate with Renderer Backend.
enum ImTextureStatus
{
    ImTextureStatus_OK,
    ImTextureStatus_Destroyed,      // Backend destroyed the texture.
    ImTextureStatus_WantCreate,     // Requesting backend to create the texture. Set status OK when done.
    ImTextureStatus_WantUpdates,    // Requesting backend to update specific blocks of pixels (write to texture portions which have never been used before). Set status OK when done.
    ImTextureStatus_WantDestroy,    // Requesting backend to destroy the texture. Set status to Destroyed when done.
};

// Coordinates of a rectangle within a texture.
// When a texture is in ImTextureStatus_WantUpdates state, we provide a list of individual rectangles to copy to the graphics system.
// You may use ImTextureData::Updates[] for the list, or ImTextureData::UpdateBox for a single bounding box.
struct ImTextureRect
{
    unsigned short      x, y;       // Upper-left coordinates of rectangle to update, within the parent Pixels[] array.
    unsigned short      w, h;       // Size of rectangle to update (in pixels)
};
struct ImTextureData
{
    //------------------------------------------ core / backend ---------------------------------------
    int                 UniqueID;               // w    -   // [DEBUG] Sequential index to facilitate identifying a texture when debugging/printing. Unique per atlas.
    ImTextureStatus     Status;                 // rw   rw  // ImTextureStatus_OK/_WantCreate/_WantUpdates/_WantDestroy. Always use SetStatus() to modify!
    void*               BackendUserData;        // -    rw  // Convenience storage for backend. Some backends may have enough with TexID.
    void*               QueueUserData;          // r    -   // Convenience storage for a staged/multi-threaded rendering texture queue (e.g. imgui_threaded_rendering.h. See #8597). When != NULL, core assumes the texture is referenced by the queue.
    ImTextureID         TexID;                  // r    w   // Backend-specific texture identifier. Always use SetTexID() to modify! The identifier will stored in ImDrawCmd::GetTexID() and passed to backend's RenderDrawData function.
    ImTextureFormat     Format;                 // w    r   // ImTextureFormat_RGBA32 (default) or ImTextureFormat_Alpha8
    int                 Width;                  // w    r   // Texture width
    int                 Height;                 // w    r   // Texture height
    int                 BytesPerPixel;          // w    r   // 4 or 1
    unsigned char*      Pixels;                 // w    r   // Pointer to whole texture buffer holding 'Width*Height' pixels and 'Width*Height*BytesPerPixels' bytes.
    ImTextureRect       UsedRect;               // w    r   // Bounding box encompassing all past and queued Updates[].
    ImTextureRect       UpdateRect;             // w    r   // Bounding box encompassing all queued Updates[].
    ImVector<ImTextureRect> Updates;            // w    r   // Array of individual updates.
    int                 UnusedFrames;           // w    r   // In order to facilitate handling Status==WantDestroy in some backend: this is a count successive frames where the texture was not used. Always >0 when Status==WantDestroy.
    unsigned short      RefCount;               // w    r   // Number of contexts using this texture. Used during backend shutdown.
    bool                UseColors;              // w    r   // Tell whether our texture data is known to use colors (rather than just white + alpha).
    bool                WantDestroyNextFrame;   // rw   -   // [Internal] Queued to set ImTextureStatus_WantDestroy next frame. May still be used in the current frame.

    // Functions
    // - If GetPixels() functions asserts while being called by your render loop, it could be caused by calling ImFontAtlas::Clear()/ClearFonts()?
    ImTextureData()     { memset((void*)this, 0, sizeof(*this)); Status = ImTextureStatus_Destroyed; TexID = ImTextureID_Invalid; }
    ~ImTextureData()    { DestroyPixels(); }
     void      Create(ImTextureFormat format, int w, int h);
     void      DestroyPixels();
    void*               GetPixels()                 { IM_ASSERT(Pixels != NULL); return Pixels; }
    void*               GetPixelsAt(int x, int y)   { IM_ASSERT(Pixels != NULL); return Pixels + (x + y * Width) * BytesPerPixel; }
    int                 GetSizeInBytes() const      { return Width * Height * BytesPerPixel; }
    int                 GetPitch() const            { return Width * BytesPerPixel; }
    ImTextureRef        GetTexRef()                 { ImTextureRef tex_ref; tex_ref._TexData = this; tex_ref._TexID = ImTextureID_Invalid; return tex_ref; }
    ImTextureID         GetTexID() const            { return TexID; }

    // Called by Renderer backend
    // - Call SetTexID() and SetStatus() after honoring texture requests. Never modify TexID and Status directly!
    // - A backend may decide to destroy a texture that we did not request to destroy, which is fine (e.g. freeing resources), but we immediately set the texture back in _WantCreate mode.
    void    SetTexID(ImTextureID tex_id)            { TexID = tex_id; }
    void    SetStatus(ImTextureStatus status)       { Status = status; if (status == ImTextureStatus_Destroyed && !WantDestroyNextFrame && Pixels != nullptr) Status = ImTextureStatus_WantCreate; }
};

//-----------------------------------------------------------------------------
// [SECTION] Font API (ImFontConfig, ImFontGlyph, ImFontAtlasFlags, ImFontAtlas, ImFontGlyphRangesBuilder, ImFont)
//-----------------------------------------------------------------------------

// A font input/source (we may rename this to ImFontSource in the future)
struct ImFontConfig
{
    // Data Source
    char            Name[40];               // <auto>   // Name (strictly to ease debugging, hence limited size buffer)
    void*           FontData;               //          // TTF/OTF data
    int             FontDataSize;           //          // TTF/OTF data size
    bool            FontDataOwnedByAtlas;   // true     // TTF/OTF data ownership taken by the owner ImFontAtlas (will delete memory itself). SINCE 1.92, THE DATA NEEDS TO PERSIST FOR WHOLE DURATION OF ATLAS.

    // Options
    bool            MergeMode;              // false    // Merge into previous ImFont, so you can combine multiple inputs font into one ImFont (e.g. ASCII font + icons + Japanese glyphs). You may want to use GlyphOffset.y when merge font of different heights.
    bool            PixelSnapH;             // false    // Align every glyph AdvanceX to pixel boundaries. Prevents fractional font size from working correctly! Useful e.g. if you are merging a non-pixel aligned font with the default font. If enabled, OversampleH/V will default to 1.
    ImS8            OversampleH;            // 0 (2)    // Rasterize at higher quality for sub-pixel positioning. 0 == auto == 1 or 2 depending on size. Note the difference between 2 and 3 is minimal. You can reduce this to 1 for large glyphs save memory. Read https://github.com/nothings/stb/blob/master/tests/oversample/README.md for details.
    ImS8            OversampleV;            // 0 (1)    // Rasterize at higher quality for sub-pixel positioning. 0 == auto == 1. This is not really useful as we don't use sub-pixel positions on the Y axis.
    ImWchar         EllipsisChar;           // 0        // Explicitly specify Unicode codepoint of ellipsis character. When fonts are being merged first specified ellipsis will be used.
    float           SizePixels;             //          // Output size in pixels for rasterizer (more or less maps to the resulting font height).
    const ImWchar*  GlyphRanges;            // NULL     // *LEGACY* THE ARRAY DATA NEEDS TO PERSIST AS LONG AS THE FONT IS ALIVE. Pointer to a user-provided list of Unicode range (2 value per range, values are inclusive, zero-terminated list).
    const ImWchar*  GlyphExcludeRanges;     // NULL     // Pointer to a small user-provided list of Unicode ranges (2 value per range, values are inclusive, zero-terminated list). This is very close to GlyphRanges[] but designed to exclude ranges from a font source, when merging fonts with overlapping glyphs. Use "Input Glyphs Overlap Detection Tool" to find about your overlapping ranges.
    //ImVec2        GlyphExtraSpacing;      // 0, 0     // (REMOVED AT IT SEEMS LARGELY OBSOLETE. PLEASE REPORT IF YOU WERE USING THIS). Extra spacing (in pixels) between glyphs when rendered: essentially add to glyph->AdvanceX. Only X axis is supported for now.
    ImVec2          GlyphOffset;            // 0, 0     // Offset (in pixels) all glyphs from this font input. Absolute value for default size, other sizes will scale this value.
    float           GlyphMinAdvanceX;       // 0        // Minimum AdvanceX for glyphs, set Min to align font icons, set both Min/Max to enforce mono-space font. Absolute value for default size, other sizes will scale this value.
    float           GlyphMaxAdvanceX;       // FLT_MAX  // Maximum AdvanceX for glyphs
    float           GlyphExtraAdvanceX;     // 0        // Extra spacing (in pixels) between glyphs. Please contact us if you are using this. // FIXME-NEWATLAS: Intentionally unscaled
    ImU32           FontNo;                 // 0        // Index of font within TTF/OTF file
    unsigned int    FontLoaderFlags;        // 0        // Settings for custom font builder. THIS IS BUILDER IMPLEMENTATION DEPENDENT. Leave as zero if unsure.
    //unsigned int  FontBuilderFlags;       // --       // [Renamed in 1.92] Use FontLoaderFlags.
    float           RasterizerMultiply;     // 1.0f     // Linearly brighten (>1.0f) or darken (<1.0f) font output. Brightening small fonts may be a good workaround to make them more readable. This is a silly thing we may remove in the future.
    float           RasterizerDensity;      // 1.0f     // [LEGACY: this only makes sense when ImGuiBackendFlags_RendererHasTextures is not supported] DPI scale multiplier for rasterization. Not altering other font metrics: makes it easy to swap between e.g. a 100% and a 400% fonts for a zooming display, or handle Retina screen. IMPORTANT: If you change this it is expected that you increase/decrease font scale roughly to the inverse of this, otherwise quality may look lowered.
    float           ExtraSizeScale;         // 1.0f     // Extra rasterizer scale over SizePixels.

    // [Internal]
    ImFontFlags     Flags;                  // Font flags (don't use just yet, will be exposed in upcoming 1.92.X updates)
    ImFont*         DstFont;                // Target font (as we merging fonts, multiple ImFontConfig may target the same font)
    const ImFontLoader* FontLoader;         // Custom font backend for this source (default source is the one stored in ImFontAtlas)
    void*           FontLoaderData;         // Font loader opaque storage (per font config)

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    bool            PixelSnapV;             // true    // [Obsoleted in 1.91.6] Align Scaled GlyphOffset.y to pixel boundaries.
#endif
     ImFontConfig();
};

// Hold rendering data for one glyph.
// (Note: some language parsers may fail to convert the bitfield members, in this case maybe drop store a single u32 or we can rework this)
struct ImFontGlyph
{
    unsigned int    Colored : 1;        // Flag to indicate glyph is colored and should generally ignore tinting (make it usable with no shift on little-endian as this is used in loops)
    unsigned int    Visible : 1;        // Flag to indicate glyph has no visible pixels (e.g. space). Allow early out when rendering.
    unsigned int    SourceIdx : 4;      // Index of source in parent font
    unsigned int    Codepoint : 26;     // 0x0000..0x10FFFF
    float           AdvanceX;           // Horizontal distance to advance cursor/layout position.
    float           X0, Y0, X1, Y1;     // Glyph corners. Offsets from current cursor/layout position.
    float           U0, V0, U1, V1;     // Texture coordinates for the current value of ImFontAtlas->TexRef. Cached equivalent of calling GetCustomRect() with PackId.
    int             PackId;             // [Internal] ImFontAtlasRectId value (FIXME: Cold data, could be moved elsewhere?)

    ImFontGlyph()   { memset((void*)this, 0, sizeof(*this)); PackId = -1; }
};

// Helper to build glyph ranges from text/string data. Feed your application strings/characters to it then call BuildRanges().
// This is essentially a tightly packed of vector of 64k booleans = 8KB storage.
struct ImFontGlyphRangesBuilder
{
    ImVector<ImU32> UsedChars;            // Store 1-bit per Unicode code point (0=unused, 1=used)

    ImFontGlyphRangesBuilder()              { Clear(); }
    inline void     Clear()                 { int size_in_bytes = (0xFFFF + 1) / 8; UsedChars.resize(size_in_bytes / (int)sizeof(ImU32)); memset(UsedChars.Data, 0, (size_t)size_in_bytes); }
    inline bool     GetBit(size_t n) const  { int off = (int)(n >> 5); ImU32 mask = 1u << (n & 31); return (UsedChars[off] & mask) != 0; }  // Get bit n in the array
    inline void     SetBit(size_t n)        { int off = (int)(n >> 5); ImU32 mask = 1u << (n & 31); UsedChars[off] |= mask; }               // Set bit n in the array
    inline void     AddChar(ImWchar c)      { SetBit(c); }                      // Add character
     void  AddText(const char* text, const char* text_end = NULL);     // Add string (each character of the UTF-8 string are added)
     void  AddRanges(const ImWchar* ranges);                           // Add ranges, e.g. builder.AddRanges(ImFontAtlas::GetGlyphRangesDefault()) to force add all of ASCII/Latin+Ext
     void  BuildRanges(ImVector<ImWchar>* out_ranges);                 // Output new ranges
};

// An opaque identifier to a rectangle in the atlas. -1 when invalid.
// The rectangle may move and UV may be invalidated, use GetCustomRect() to retrieve it.
typedef int ImFontAtlasRectId;
#define ImFontAtlasRectId_Invalid -1

// Output of ImFontAtlas::GetCustomRect() when using custom rectangles.
// Those values may not be cached/stored as they are only valid for the current value of atlas->TexRef
// (this is in theory derived from ImTextureRect but we use separate structures for reasons)
struct ImFontAtlasRect
{
    unsigned short  x, y;               // Position (in current texture)
    unsigned short  w, h;               // Size
    ImVec2          uv0, uv1;           // UV coordinates (in current texture)

    ImFontAtlasRect() { memset((void*)this, 0, sizeof(*this)); }
};

// Flags for ImFontAtlas build
enum ImFontAtlasFlags_
{
    ImFontAtlasFlags_None               = 0,
    ImFontAtlasFlags_NoPowerOfTwoHeight = 1 << 0,   // Don't round the height to next power of two
    ImFontAtlasFlags_NoMouseCursors     = 1 << 1,   // Don't build software mouse cursors into the atlas (save a little texture memory)
    ImFontAtlasFlags_NoBakedLines       = 1 << 2,   // Don't build thick line textures into the atlas (save a little texture memory, allow support for point/nearest filtering). The AntiAliasedLinesUseTex features uses them, otherwise they will be rendered using polygons (more expensive for CPU/GPU).
};

// Load and rasterize multiple TTF/OTF fonts into a same texture. The font atlas will build a single texture holding:
//  - One or more fonts.
//  - Custom graphics data needed to render the shapes needed by Dear ImGui.
//  - Mouse cursor shapes for software cursor rendering (unless setting 'Flags |= ImFontAtlasFlags_NoMouseCursors' in the font atlas).
//  - If you don't call any AddFont*** functions, the default font embedded in the code will be loaded for you.
// It is the rendering backend responsibility to upload texture into your graphics API:
//  - ImGui_ImplXXXX_RenderDrawData() functions generally iterate platform_io->Textures[] to create/update/destroy each ImTextureData instance.
//  - Backend then set ImTextureData's TexID and BackendUserData.
//  - Texture id are passed back to you during rendering to identify the texture. Read FAQ entry about ImTextureID/ImTextureRef for more details.
// Legacy path:
//  - Call Build() + GetTexDataAsAlpha8() or GetTexDataAsRGBA32() to build and retrieve pixels data.
//  - Call SetTexID(my_tex_id); and pass the pointer/identifier to your texture in a format natural to your graphics API.
// Common pitfalls:
// - If you pass a 'glyph_ranges' array to AddFont*** functions, you need to make sure that your array persists up until the
//   atlas is build (when calling GetTexData*** or Build()). We only copy the pointer, not the data.
// - Important: By default, AddFontFromMemoryTTF() takes ownership of the data. Even though we are not writing to it, we will free the pointer on destruction.
//   You can set font_cfg->FontDataOwnedByAtlas=false to keep ownership of your data and it won't be freed,
// - Even though many functions are suffixed with "TTF", OTF data is supported just as well.
// - This is an old API and it is currently awkward for those and various other reasons! We will address them in the future!
struct ImFontAtlas
{
     ImFontAtlas();
     ~ImFontAtlas();
     ImFont*           AddFont(const ImFontConfig* font_cfg);
     ImFont*           AddFontDefault(const ImFontConfig* font_cfg = NULL);        // Selects between AddFontDefaultVector() and AddFontDefaultBitmap().
     ImFont*           AddFontDefaultVector(const ImFontConfig* font_cfg = NULL);  // Embedded scalable font. Recommended at any higher size.
     ImFont*           AddFontDefaultBitmap(const ImFontConfig* font_cfg = NULL);  // Embedded classic pixel-clean font. Recommended at Size 13px with no scaling.
     ImFont*           AddFontFromFileTTF(const char* filename, float size_pixels = 0.0f, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL);
     ImFont*           AddFontFromMemoryTTF(void* font_data, int font_data_size, float size_pixels = 0.0f, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL); // Note: Transfer ownership of 'ttf_data' to ImFontAtlas! Will be deleted after destruction of the atlas. Set font_cfg->FontDataOwnedByAtlas=false to keep ownership of your data and it won't be freed.
     ImFont*           AddFontFromMemoryCompressedTTF(const void* compressed_font_data, int compressed_font_data_size, float size_pixels = 0.0f, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL); // 'compressed_font_data' still owned by caller. Compress with binary_to_compressed_c.cpp.
     ImFont*           AddFontFromMemoryCompressedBase85TTF(const char* compressed_font_data_base85, float size_pixels = 0.0f, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL);              // 'compressed_font_data_base85' still owned by caller. Compress with binary_to_compressed_c.cpp with -base85 parameter.
     void              RemoveFont(ImFont* font);                       // Remove a font
     void              CompactCache();                                 // Compact cached glyphs and texture.
     void              SetFontLoader(const ImFontLoader* font_loader); // Change font loader at runtime.

    // Clearing the atlas/fonts has little use nowadays, unless you want to batch remove all fonts.
    // - Since 1.92, you can call ClearFonts() mid-frame, if you load new fonts afterwards.
    // - As we are transitioning toward our new font system the semantic for those functions gets increasingly misleading and are often a source of issues.
    //   TL;DR; most likely, don't use any of those functions. We expect to obsolete/rework them.
     void              Clear();                    // Clear everything (fonts + textures). Don't call mid-frame!
     void              ClearFonts();               // Clear input+output font data/glyphs. New fonts and textures will be recreated afterwards.
     void              ClearInputData();           // [OBSOLETE] Clear input data (all ImFontConfig structures including sizes, TTF data, glyph ranges, etc.) = all the data used to build the texture and fonts.
     void              ClearTexData();             // [OBSOLETE] Clear CPU-side copy of the texture data. Saves RAM once the texture has been copied to graphics memory.

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
  
     bool  Build();                    // Build pixels data. This is called automatically for you by the GetTexData*** functions.
     void  GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel = NULL); // 1 byte per-pixel
     void  GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel = NULL); // 4 bytes-per-pixel
    void            SetTexID(ImTextureID id)    { IM_ASSERT(TexRef._TexID == ImTextureID_Invalid); TexRef._TexData->TexID = id; }                               // Called by legacy backends. May be called before texture creation.
    void            SetTexID(ImTextureRef id)   { IM_ASSERT(TexRef._TexID == ImTextureID_Invalid && id._TexData == NULL); TexRef._TexData->TexID = id._TexID; } // Called by legacy backends.
    bool            IsBuilt() const { return Fonts.Size > 0 && TexIsBuilt; } // Bit ambiguous: used to detect when user didn't build texture but effectively we should check TexID != 0 except that would be backend dependent..
#endif

    //-------------------------------------------
    // Glyph Ranges
    //-------------------------------------------

    // Since 1.92: specifying glyph ranges is only useful/necessary if your backend doesn't support ImGuiBackendFlags_RendererHasTextures!
     const ImWchar*    GetGlyphRangesDefault();                // Basic Latin, Extended Latin
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    // Helpers to retrieve list of common Unicode ranges (2 value per range, values are inclusive, zero-terminated list)
    // NB: Make sure that your string are UTF-8 and NOT in your local code page.
    // Read https://github.com/ocornut/imgui/blob/master/docs/FONTS.md/#about-utf-8-encoding for details.
    // NB: Consider using ImFontGlyphRangesBuilder to build glyph ranges from textual data.
     const ImWchar*    GetGlyphRangesGreek();                  // Default + Greek and Coptic
     const ImWchar*    GetGlyphRangesKorean();                 // Default + Korean characters
     const ImWchar*    GetGlyphRangesJapanese();               // Default + Hiragana, Katakana, Half-Width, Selection of 2999 Ideographs
     const ImWchar*    GetGlyphRangesChineseFull();            // Default + Half-Width + Japanese Hiragana/Katakana + full set of about 21000 CJK Unified Ideographs
     const ImWchar*    GetGlyphRangesChineseSimplifiedCommon();// Default + Half-Width + Japanese Hiragana/Katakana + set of 2500 CJK Unified Ideographs for common simplified Chinese
     const ImWchar*    GetGlyphRangesCyrillic();               // Default + about 400 Cyrillic characters
     const ImWchar*    GetGlyphRangesThai();                   // Default + Thai characters
     const ImWchar*    GetGlyphRangesVietnamese();             // Default + Vietnamese characters
#endif
     ImFontAtlasRectId AddCustomRect(int width, int height, ImFontAtlasRect* out_r = NULL);// Register a rectangle. Return -1 (ImFontAtlasRectId_Invalid) on error.
     void              RemoveCustomRect(ImFontAtlasRectId id);                             // Unregister a rectangle. Existing pixels will stay in texture until resized / garbage collected.
     bool              GetCustomRect(ImFontAtlasRectId id, ImFontAtlasRect* out_r) const;  // Get rectangle coordinates for current texture. Valid immediately, never store this (read above)!

    //-------------------------------------------
    // Members
    //-------------------------------------------

    // Input
    ImFontAtlasFlags            Flags;              // Build flags (see ImFontAtlasFlags_)
    ImTextureFormat             TexDesiredFormat;   // Desired texture format (default to ImTextureFormat_RGBA32 but may be changed to ImTextureFormat_Alpha8).
    int                         TexGlyphPadding;    // FIXME: Should be called "TexPackPadding". Padding between glyphs within texture in pixels. Defaults to 1. If your rendering method doesn't rely on bilinear filtering you may set this to 0 (will also need to set AntiAliasedLinesUseTex = false).
    int                         TexMinWidth;        // Minimum desired texture width. Must be a power of two. Default to 512.
    int                         TexMinHeight;       // Minimum desired texture height. Must be a power of two. Default to 128.
    int                         TexMaxWidth;        // Maximum desired texture width. Must be a power of two. Default to 8192.
    int                         TexMaxHeight;       // Maximum desired texture height. Must be a power of two. Default to 8192.
    void*                       UserData;           // Store your own atlas related user-data (if e.g. you have multiple font atlas).

    // Output
    // - Because textures are dynamically created/resized, the current texture identifier may changed at *ANY TIME* during the frame.
    // - This should not affect you as you can always use the latest value. But note that any precomputed UV coordinates are only valid for the current TexRef.
#ifdef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    ImTextureRef                TexRef;             // Latest texture identifier == TexData->GetTexRef().
#else
    union { ImTextureRef TexRef; ImTextureRef TexID; }; // Latest texture identifier == TexData->GetTexRef(). // RENAMED TexID to TexRef in 1.92.0.
#endif
    ImTextureData*              TexData;            // Latest texture.

    // [Internal]
    ImVector<ImTextureData*>    TexList;            // Texture list (most often TexList.Size == 1). TexData is always == TexList.back(). DO NOT USE DIRECTLY, USE GetDrawData().Textures[]/GetPlatformIO().Textures[] instead!
    bool                        Locked;             // Marked as locked during ImGui::NewFrame()..EndFrame() scope if TexUpdates are not supported. Any attempt to modify the atlas will assert.
    bool                        RendererHasTextures;// Copy of (BackendFlags & ImGuiBackendFlags_RendererHasTextures) from supporting context.
    bool                        TexIsBuilt;         // Set when texture was built matching current font input. Mostly useful for legacy IsBuilt() call.
    bool                        TexPixelsUseColors; // Tell whether our texture data is known to use colors (rather than just alpha channel), in order to help backend select a format or conversion process.
    ImVec2                      TexUvScale;         // = (1.0f/TexData->TexWidth, 1.0f/TexData->TexHeight). May change as new texture gets created.
    ImVec2                      TexUvWhitePixel;    // Texture coordinates to a white pixel. May change as new texture gets created.
    ImVector<ImFont*>           Fonts;              // Hold all the fonts returned by AddFont*. Fonts[0] is the default font upon calling ImGui::NewFrame(), use ImGui::PushFont()/PopFont() to change the current font.
    ImVector<ImFontConfig>      Sources;            // Source/configuration data
    ImVec4                      TexUvLines[IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 1];  // UVs for baked anti-aliased lines
    int                         TexNextUniqueID;    // Next value to be stored in TexData->UniqueID
    int                         FontNextUniqueID;   // Next value to be stored in ImFont->FontID
    ImVector<ImDrawListSharedData*> DrawListSharedDatas; // List of users for this atlas. Typically one per Dear ImGui context.
    ImFontAtlasBuilder*         Builder;            // Opaque interface to our data that doesn't need to be public and may be discarded when rebuilding.
    const ImFontLoader*         FontLoader;         // Font loader opaque interface (default to use FreeType when IMGUI_ENABLE_FREETYPE is defined, otherwise default to use stb_truetype). Use SetFontLoader() to change this at runtime.
    const char*                 FontLoaderName;     // Font loader name (for display e.g. in About box) == FontLoader->Name
    void*                       FontLoaderData;     // Font backend opaque storage
    unsigned int                FontLoaderFlags;    // Shared flags (for all fonts) for font loader. THIS IS BUILD IMPLEMENTATION DEPENDENT (e.g. Per-font override is also available in ImFontConfig).
    int                         RefCount;           // Number of contexts using this atlas
    //ImGuiContext*               OwnerContext;       // Context which own the atlas will be in charge of updating and destroying it.

    // [Obsolete]
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    // Legacy: You can request your rectangles to be mapped as font glyph (given a font + Unicode point), so you can render e.g. custom colorful icons and use them as regular glyphs. --> Prefer using a custom ImFontLoader.
    ImFontAtlasRect             TempRect;           // For old GetCustomRectByIndex() API
    inline ImFontAtlasRectId    AddCustomRectRegular(int w, int h)                                                          { return AddCustomRect(w, h); }                             // RENAMED in 1.92.0
    inline const ImFontAtlasRect* GetCustomRectByIndex(ImFontAtlasRectId id)                                                { return GetCustomRect(id, &TempRect) ? &TempRect : NULL; } // OBSOLETED in 1.92.0
    inline void                 CalcCustomRectUV(const ImFontAtlasRect* r, ImVec2* out_uv_min, ImVec2* out_uv_max) const    { *out_uv_min = r->uv0; *out_uv_max = r->uv1; }             // OBSOLETED in 1.92.0
     ImFontAtlasRectId AddCustomRectFontGlyph(ImFont* font, ImWchar codepoint, int w, int h, float advance_x, const ImVec2& offset = ImVec2(0, 0));                            // OBSOLETED in 1.92.0: Use custom ImFontLoader in ImFontConfig
     ImFontAtlasRectId AddCustomRectFontGlyphForSize(ImFont* font, float font_size, ImWchar codepoint, int w, int h, float advance_x, const ImVec2& offset = ImVec2(0, 0));    // ADDED AND OBSOLETED in 1.92.0
#endif
    //unsigned int                      FontBuilderFlags;        // OBSOLETED in 1.92.0: Renamed to FontLoaderFlags.
    //int                               TexDesiredWidth;         // OBSOLETED in 1.92.0: Force texture width before calling Build(). Must be a power-of-two. If have many glyphs your graphics API have texture size restrictions you may want to increase texture width to decrease height.
    //typedef ImFontAtlasRect           ImFontAtlasCustomRect;   // OBSOLETED in 1.92.0
    //typedef ImFontAtlasCustomRect     CustomRect;              // OBSOLETED in 1.72+
    //typedef ImFontGlyphRangesBuilder  GlyphRangesBuilder;      // OBSOLETED in 1.67+
};

// Font runtime data for a given size
// Important: pointers to ImFontBaked are only valid for the current frame.
struct ImFontBaked
{
    // [Internal] Members: Hot ~20/24 bytes (for CalcTextSize)
    ImVector<float>             IndexAdvanceX;      // 12-16 // out // Sparse. Glyphs->AdvanceX in a directly indexable way (cache-friendly for CalcTextSize functions which only this info, and are often bottleneck in large UI).
    float                       FallbackAdvanceX;   // 4     // out // FindGlyph(FallbackChar)->AdvanceX
    float                       Size;               // 4     // in  // Height of characters/line, set during loading (doesn't change after loading)
    float                       RasterizerDensity;  // 4     // in  // Density this is baked at

    // [Internal] Members: Hot ~28/36 bytes (for RenderText loop)
    ImVector<ImU16>             IndexLookup;        // 12-16 // out // Sparse. Index glyphs by Unicode code-point.
    ImVector<ImFontGlyph>       Glyphs;             // 12-16 // out // All glyphs.
    int                         FallbackGlyphIndex; // 4     // out // Index of FontFallbackChar

    // [Internal] Members: Cold
    float                       Ascent, Descent;    // 4+4   // out // Ascent: distance from top to bottom of e.g. 'A' [0..FontSize] (unscaled)
    unsigned int                MetricsTotalSurface:26;// 3  // out // Total surface in pixels to get an idea of the font rasterization/texture cost (not exact, we approximate the cost of padding between glyphs)
    unsigned int                WantDestroy:1;         // 0  //     // Queued for destroy
    unsigned int                LoadNoFallback:1;      // 0  //     // Disable loading fallback in lower-level calls.
    unsigned int                LoadNoRenderOnLayout:1;// 0  //     // Enable a two-steps mode where CalcTextSize() calls will load AdvanceX *without* rendering/packing glyphs. Only advantageous if you know that the glyph is unlikely to actually be rendered, otherwise it is slower because we'd do one query on the first CalcTextSize and one query on the first Draw.
    int                         LastUsedFrame;         // 4  //     // Record of that time this was bounds
    ImGuiID                     BakedId;            // 4     //     // Unique ID for this baked storage
    ImFont*                     OwnerFont;          // 4-8   // in  // Parent font
    void*                       FontLoaderDatas;    // 4-8   //     // Font loader opaque storage (per baked font * sources): single contiguous buffer allocated by imgui, passed to loader.

    // Functions
     ImFontBaked();
     void              ClearOutputData();
     ImFontGlyph*      FindGlyph(ImWchar c);               // Return U+FFFD glyph if requested glyph doesn't exists.
     ImFontGlyph*      FindGlyphNoFallback(ImWchar c);     // Return NULL if glyph doesn't exist
     float             GetCharAdvance(ImWchar c);
     bool              IsGlyphLoaded(ImWchar c);
};

// Font flags
// (in future versions as we redesign font loading API, this will become more important and better documented. for now please consider this as internal/advanced use)
enum ImFontFlags_
{
    ImFontFlags_None                    = 0,
    ImFontFlags_NoLoadError             = 1 << 1,   // Disable throwing an error/assert when calling AddFontXXX() with missing file/data. Calling code is expected to check AddFontXXX() return value.
    ImFontFlags_NoLoadGlyphs            = 1 << 2,   // [Internal] Disable loading new glyphs.
    ImFontFlags_LockBakedSizes          = 1 << 3,   // [Internal] Disable loading new baked sizes, disable garbage collecting current ones. e.g. if you want to lock a font to a single size. Important: if you use this to preload given sizes, consider the possibility of multiple font density used on Retina display.
    ImFontFlags_ImplicitRefSize         = 1 << 4,   // [Internal] Reference size was not set explicitly.
};

// Font runtime data and rendering
// - ImFontAtlas automatically loads a default embedded font for you if you didn't load one manually.
// - Since 1.92.0 a font may be rendered as any size! Therefore a font doesn't have one specific size.
// - Use 'font->GetFontBaked(size)' to retrieve the ImFontBaked* corresponding to a given size.
// - If you used g.Font + g.FontSize (which is frequent from the ImGui layer), you can use g.FontBaked as a shortcut, as g.FontBaked == g.Font->GetFontBaked(g.FontSize).
struct ImFont
{
    // [Internal] Members: Hot ~12-20 bytes
    ImFontBaked*                LastBaked;          // 4-8   // Cache last bound baked. NEVER USE DIRECTLY. Use GetFontBaked().
    ImFontAtlas*                OwnerAtlas;         // 4-8   // What we have been loaded into.
    ImFontFlags                 Flags;              // 4     // Font flags.
    float                       CurrentRasterizerDensity;    // Current rasterizer density. This is a varying state of the font.

    // [Internal] Members: Cold ~24-52 bytes
    // Conceptually Sources[] is the list of font sources merged to create this font.
    ImGuiID                     FontId;             // Unique identifier for the font
    float                       LegacySize;         // 4     // in  // Font size passed to AddFont(). Use for old code calling PushFont() expecting to use that size. (use ImGui::GetFontBaked() to get font baked at current bound size).
    ImVector<ImFontConfig*>     Sources;            // 16    // in  // List of sources. Pointers within OwnerAtlas->Sources[]
    ImWchar                     EllipsisChar;       // 2-4   // out // Character used for ellipsis rendering ('...'). If you ever want to temporarily swap this for an alternative/dummy char, make sure to clear EllipsisAutoBake.
    ImWchar                     FallbackChar;       // 2-4   // out // Character used if a glyph isn't found (U+FFFD, '?')
    ImU8                        Used8kPagesMap[(0xFFFF +1)/8192/8]; // 1 bytes if ImWchar=ImWchar16, 17 bytes if ImWchar==ImWchar32. Store 1-bit for each block of 8K codepoints that has one active glyph. This is mainly used to facilitate iterations across all used codepoints.
    bool                        EllipsisAutoBake;   // 1     //     // Mark when the "..." glyph (== EllipsisChar) needs to be generated by combining multiple '.'.
    //ImGuiStorage                RemapPairs;         // 16    //     // Remapping pairs when using AddRemapChar(), otherwise empty.
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    float                       Scale;              // 4     // in  // Legacy base font scale (~1.0f), multiplied by the per-window font scale which you can adjust with SetWindowFontScale()
#endif

    // Methods
     ImFont();
     ~ImFont();
     bool              IsGlyphInFont(ImWchar c);
    bool                        IsLoaded() const                { return OwnerAtlas != NULL; }
    const char*                 GetDebugName() const            { return Sources.Size ? Sources[0]->Name : "<unknown>"; } // Fill ImFontConfig::Name.

    // [Internal] Don't use!
    // 'max_width' stops rendering after a certain width (could be turned into a 2d size). FLT_MAX to disable.
    // 'wrap_width' enable automatic word-wrapping across multiple lines to fit into given width. 0.0f to disable.
     ImFontBaked*      GetFontBaked(float font_size, float density = -1.0f);  // Get or create baked data for given size
     ImVec2            CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end = NULL, const char** out_remaining = NULL);
     const char*       CalcWordWrapPosition(float size, const char* text, const char* text_end, float wrap_width);
     void              RenderChar(ImDrawList* draw_list, float size, const ImVec2& pos, ImU32 col, ImWchar c, const ImVec4* cpu_fine_clip = NULL);
     void              RenderText(ImDrawList* draw_list, float size, const ImVec2& pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width = 0.0f, ImDrawTextFlags flags = 0);
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    inline const char*          CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) { return CalcWordWrapPosition(LegacySize * scale, text, text_end, wrap_width); } // Obsoleted old name in 1.92.0. Note how `scale` was to `size`.
#endif

    // [Internal] Don't use!
     void              ClearOutputData();
     void              AddRemapChar(ImWchar from_codepoint, ImWchar to_codepoint); // Makes 'from_codepoint' character points to 'to_codepoint' glyph.
     bool              IsGlyphRangeUnused(unsigned int c_begin, unsigned int c_last);
};

// This is provided for consistency (but we don't actually use this)
inline ImTextureID ImTextureRef::GetTexID() const
{
    IM_ASSERT(!(_TexData != NULL && _TexID != ImTextureID_Invalid));
    return _TexData ? _TexData->TexID : _TexID;
}

// Using an indirection to avoid patching ImDrawCmd after a SetTexID() call (but this could be an alternative solution too)
inline ImTextureID ImDrawCmd::GetTexID() const
{
    // If you are getting this assert with ImTextureID_Invalid == 0 and your ImTextureID is used to store an index or an offset:
    // - You can add '#define ImTextureID_Invalid ((ImTextureID)-1)' in your imconfig.h file.
    // If you are getting this assert with a renderer backend with support for ImGuiBackendFlags_RendererHasTextures (1.92+):
    // - You must correctly iterate and handle ImTextureData requests stored in ImDrawData::Textures[]. See docs/BACKENDS.md.
    ImTextureID tex_id = TexRef._TexData ? TexRef._TexData->TexID : TexRef._TexID; // == TexRef.GetTexID() above.
    if (TexRef._TexData != NULL)
        IM_ASSERT(tex_id != ImTextureID_Invalid && "ImDrawCmd is referring to ImTextureData that wasn't uploaded to graphics system. Backend must call ImTextureData::SetTexID() after handling ImTextureStatus_WantCreate request!");
    return tex_id;
}
typedef ImFontAtlasRect ImFontAtlasCustomRect;

}