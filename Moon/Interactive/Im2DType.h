#pragma once
#include <vector>
namespace MOON {
	namespace Render2D {
#define COL32_R_SHIFT    0
#define COL32_G_SHIFT    8
#define COL32_B_SHIFT    16
#define COL32_A_SHIFT    24
#define COL32_A_MASK     0xFF000000	

#define COL32(R,G,B,A)    (((ImU32)(A)<<COL32_A_SHIFT) | ((ImU32)(B)<<COL32_B_SHIFT) | ((ImU32)(G)<<COL32_G_SHIFT) | ((ImU32)(R)<<COL32_R_SHIFT))
		
        enum ImDrawListFlags_
		{
			ImDrawListFlags_None = 0,
			ImDrawListFlags_AntiAliasedLines = 1 << 0,  // Enable anti-aliased lines/borders (*2 the number of triangles for 1.0f wide line or lines thin enough to be drawn using textures, otherwise *3 the number of triangles)
			ImDrawListFlags_AntiAliasedLinesUseTex = 1 << 1,  // Enable anti-aliased lines/borders using textures when possible. Require backend to render with bilinear filtering (NOT point/nearest filtering).
			ImDrawListFlags_AntiAliasedFill = 1 << 2,  // Enable anti-aliased edge around filled shapes (rounded rectangles, circles).
			ImDrawListFlags_AllowVtxOffset = 1 << 3,  // Can emit 'VtxOffset > 0' to allow large meshes. Set when 'ImGuiBackendFlags_RendererHasVtxOffset' is enabled.
			ImDrawListFlags_TextNoPixelSnap = 1 << 4,  // Disable automatically snapping AddText() calls to pixel boundaries.
		};
		enum ImDrawFlags_
		{
			ImDrawFlags_None = 0,

			// Rounding for AddRect(), AddRectFilled(), PathRect()
			// - When not specified, we defaults to ImDrawFlags_RoundCornersAll! So you only need to use those flags if you want another configuration.
			ImDrawFlags_RoundCornersTopLeft = 1 << 4, // Round top-left corner only (when rounding > 0.0f, we default to all corners).
			ImDrawFlags_RoundCornersTopRight = 1 << 5, // Round top-right corner only (when rounding > 0.0f, we default to all corners).
			ImDrawFlags_RoundCornersBottomLeft = 1 << 6, // Round bottom-left corner only (when rounding > 0.0f, we default to all corners).
			ImDrawFlags_RoundCornersBottomRight = 1 << 7, // Round bottom-right corner only (when rounding > 0.0f, we default to all corners).
			ImDrawFlags_RoundCornersNone = 1 << 8, // Disable rounding even if `float rounding > 0.0f`. This is NOT zero, NOT an implicit flag!
			ImDrawFlags_RoundCornersAll = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight, // (Default!!)
			ImDrawFlags_RoundCornersDefault_ = ImDrawFlags_RoundCornersAll, // Default to ALL corners if none of the _RoundCornersXX flags are specified!
			ImDrawFlags_RoundCornersTop = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight,
			ImDrawFlags_RoundCornersBottom = ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight,
			ImDrawFlags_RoundCornersLeft = ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersTopLeft,
			ImDrawFlags_RoundCornersRight = ImDrawFlags_RoundCornersBottomRight | ImDrawFlags_RoundCornersTopRight,
			ImDrawFlags_RoundCornersMask_ = ImDrawFlags_RoundCornersAll | ImDrawFlags_RoundCornersNone,

			// Stroke options
			ImDrawFlags_Closed = 1 << 9, // PathStroke(), AddPolyline(): specify that shape should be closed.
			//ImDrawFlags_Closed                    = 1,      // Prior to 1.92.8 (May 2026), ImDrawFlags_Closed was guaranteed to be == 1<<0 == 1 for legacy compatibility reason. Hardcoded use of 1 or true should be replaced.

			ImDrawFlags_InvalidMask_ = ~0x7FFFFFF0, // == 0x8000000F,
		};
        // Font flags
// (in future versions as we redesign font loading API, this will become more important and better documented. for now please consider this as internal/advanced use)
        enum ImFontFlags_
        {
            ImFontFlags_None = 0,
            ImFontFlags_NoLoadError = 1 << 1,   // Disable throwing an error/assert when calling AddFontXXX() with missing file/data. Calling code is expected to check AddFontXXX() return value.
            ImFontFlags_NoLoadGlyphs = 1 << 2,   // [Internal] Disable loading new glyphs.
            ImFontFlags_LockBakedSizes = 1 << 3,   // [Internal] Disable loading new baked sizes, disable garbage collecting current ones. e.g. if you want to lock a font to a single size. Important: if you use this to preload given sizes, consider the possibility of multiple font density used on Retina display.
            ImFontFlags_ImplicitRefSize = 1 << 4,   // [Internal] Reference size was not set explicitly.
        };
        typedef unsigned long long  ImU64;
        typedef ImU64 ImTextureID;
		typedef unsigned int        ImU32;
		typedef unsigned int ImDrawIdx;
		typedef int ImDrawFlags;
		typedef int ImDrawListFlags;
        typedef int ImFontFlags;
		struct ImVec2
		{
			float                                   x, y;
			constexpr ImVec2() : x(0.0f), y(0.0f) {}
			constexpr ImVec2(float _x, float _y) : x(_x), y(_y) {}
			float& operator[] (size_t idx) { return ((float*)(void*)(char*)this)[idx]; }
			float  operator[] (size_t idx) const { return ((const float*)(const void*)(const char*)this)[idx]; }

		};
		struct ImDrawVert
		{
			ImVec2  pos;
			ImVec2  uv;
			ImU32   col;
		};
		struct ImDrawCmd
		{
			unsigned long long TextureId;
			unsigned int VtxOffset;
			unsigned int IdxOffset;
			unsigned int ElemCount;
			ImDrawCmd() { memset(this, 0, sizeof(*this)); }
		};
		struct ImDrawList
		{
			ImDrawListFlags         Flags;
			std::vector<ImDrawCmd>CmdBuffer;
			std::vector<ImDrawVert> VtxBuffer;
			std::vector<unsigned int>IdxBuffer;
			unsigned int            _VtxCurrentIdx;
			ImDrawVert* _VtxWritePtr;
			ImDrawIdx* _IdxWritePtr;
			std::vector<ImVec2>        _Path;
			void AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness = 1.0f);
			void PrimReserve(int idx_count, int vtx_count);
			void PrimRect(const ImVec2& a, const ImVec2& b, ImU32 col);
			void AddPolyline(const ImVec2* points, int num_points, ImU32 col, float thickness, ImDrawFlags flags = 0);
			void AddConvexPolyFilled(const ImVec2* points, int num_points, ImU32 col);
			void AddConcavePolyFilled(const ImVec2* points, int num_points, ImU32 col);
			void AddLineH(float min_x, float max_x, float y, ImU32 col, float thickness = 1.0f);
			void AddLineV(float x, float min_y, float max_y, ImU32 col, float thickness = 1.0f);
			void AddRect(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding = 0.0f, float thickness = 1.0f, ImDrawFlags flags = 0);
			void AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding = 0.0f, ImDrawFlags flags = 0);
			void AddDrawCmd();

			inline    void  PathClear() { _Path.clear(); }
			inline    void  PathLineTo(const ImVec2& pos) { _Path.push_back(pos); }
			inline    void  PathFillConvex(ImU32 col) { AddConvexPolyFilled(_Path.data(), _Path.size(), col); _Path.clear(); }
			inline    void  PathFillConcave(ImU32 col) { AddConcavePolyFilled(_Path.data(), _Path.size(), col); _Path.clear(); }
			inline    void  PathStroke(ImU32 col, float thickness = 1.0f, ImDrawFlags flags = 0) { AddPolyline(_Path.data(), _Path.size(), col, thickness, flags); _Path.clear(); }
			void  PathRect(const ImVec2& rect_min, const ImVec2& rect_max, float rounding = 0.0f, ImDrawFlags flags = 0);
			void  PathArcToFast(const ImVec2& center, float radius, int a_min_of_12, int a_max_of_12);
			void  _PathArcToFastEx(const ImVec2& center, float radius, int a_min_sample, int a_max_sample, int a_step);
			void  _ResetForNewFrame();
		};
//#define ImTextureID_Invalid     ((ImTextureID)0)
//
//        // We intentionally support a limited amount of texture formats to limit burden on CPU-side code and extension.
//        // Most standard backends only support RGBA32 but we provide a single channel option for low-resource/embedded systems.
//        enum ImTextureFormat
//        {
//            ImTextureFormat_RGBA32,         // 4 components per pixel, each is unsigned 8-bit. Total size = TexWidth * TexHeight * 4
//            ImTextureFormat_Alpha8,         // 1 component per pixel, each is unsigned 8-bit. Total size = TexWidth * TexHeight
//        };
//
//        // Status of a texture to communicate with Renderer Backend.
//        enum ImTextureStatus
//        {
//            ImTextureStatus_OK,
//            ImTextureStatus_Destroyed,      // Backend destroyed the texture.
//            ImTextureStatus_WantCreate,     // Requesting backend to create the texture. Set status OK when done.
//            ImTextureStatus_WantUpdates,    // Requesting backend to update specific blocks of pixels (write to texture portions which have never been used before). Set status OK when done.
//            ImTextureStatus_WantDestroy,    // Requesting backend to destroy the texture. Set status to Destroyed when done.
//        };
//
//        // Coordinates of a rectangle within a texture.
//        // When a texture is in ImTextureStatus_WantUpdates state, we provide a list of individual rectangles to copy to the graphics system.
//        // You may use ImTextureData::Updates[] for the list, or ImTextureData::UpdateBox for a single bounding box.
//        struct ImTextureRect
//        {
//            unsigned short      x, y;       // Upper-left coordinates of rectangle to update, within the parent Pixels[] array.
//            unsigned short      w, h;       // Size of rectangle to update (in pixels)
//        };
//
//        // Specs and pixel storage for a texture used by Dear ImGui.
//        // This is only useful for (1) core library and (2) backends. End-user/applications do not need to care about this.
//        // Renderer Backends will create a GPU-side version of this.
//        // Why does we store two identifiers: TexID and BackendUserData?
//        // - ImTextureID    TexID           = lower-level identifier stored in ImDrawCmd. ImDrawCmd can refer to textures not created by the backend, and for which there's no ImTextureData.
//        // - void*          BackendUserData = higher-level opaque storage for backend own book-keeping. Some backends may have enough with TexID and not need both.
//        // In columns below: who reads/writes each fields? 'r'=read, 'w'=write, 'core'=main library, 'backend'=renderer backend
//        struct ImTextureData
//        {
//            //------------------------------------------ core / backend ---------------------------------------
//            int                 UniqueID;               // w    -   // [DEBUG] Sequential index to facilitate identifying a texture when debugging/printing. Unique per atlas.
//            ImTextureStatus     Status;                 // rw   rw  // ImTextureStatus_OK/_WantCreate/_WantUpdates/_WantDestroy. Always use SetStatus() to modify!
//            void* BackendUserData;        // -    rw  // Convenience storage for backend. Some backends may have enough with TexID.
//            void* QueueUserData;          // r    -   // Convenience storage for a staged/multi-threaded rendering texture queue (e.g. imgui_threaded_rendering.h. See #8597). When != NULL, core assumes the texture is referenced by the queue.
//            ImTextureID         TexID;                  // r    w   // Backend-specific texture identifier. Always use SetTexID() to modify! The identifier will stored in ImDrawCmd::GetTexID() and passed to backend's RenderDrawData function.
//            ImTextureFormat     Format;                 // w    r   // ImTextureFormat_RGBA32 (default) or ImTextureFormat_Alpha8
//            int                 Width;                  // w    r   // Texture width
//            int                 Height;                 // w    r   // Texture height
//            int                 BytesPerPixel;          // w    r   // 4 or 1
//            unsigned char* Pixels;                 // w    r   // Pointer to whole texture buffer holding 'Width*Height' pixels and 'Width*Height*BytesPerPixels' bytes.
//            ImTextureRect       UsedRect;               // w    r   // Bounding box encompassing all past and queued Updates[].
//            ImTextureRect       UpdateRect;             // w    r   // Bounding box encompassing all queued Updates[].
//            ImVector<ImTextureRect> Updates;            // w    r   // Array of individual updates.
//            int                 UnusedFrames;           // w    r   // In order to facilitate handling Status==WantDestroy in some backend: this is a count successive frames where the texture was not used. Always >0 when Status==WantDestroy.
//            unsigned short      RefCount;               // w    r   // Number of contexts using this texture. Used during backend shutdown.
//            bool                UseColors;              // w    r   // Tell whether our texture data is known to use colors (rather than just white + alpha).
//            bool                WantDestroyNextFrame;   // rw   -   // [Internal] Queued to set ImTextureStatus_WantDestroy next frame. May still be used in the current frame.
//
//            // Functions
//            // - If GetPixels() functions asserts while being called by your render loop, it could be caused by calling ImFontAtlas::Clear()/ClearFonts()?
//            ImTextureData() { memset((void*)this, 0, sizeof(*this)); Status = ImTextureStatus_Destroyed; TexID = ImTextureID_Invalid; }
//            ~ImTextureData() { DestroyPixels(); }
//             void      Create(ImTextureFormat format, int w, int h);
//             void      DestroyPixels();
//            void* GetPixels() { assert(Pixels != NULL); return Pixels; }
//            void* GetPixelsAt(int x, int y) { IM_ASSERT(Pixels != NULL); return Pixels + (x + y * Width) * BytesPerPixel; }
//            int                 GetSizeInBytes() const { return Width * Height * BytesPerPixel; }
//            int                 GetPitch() const { return Width * BytesPerPixel; }
//            ImTextureRef        GetTexRef() { ImTextureRef tex_ref; tex_ref._TexData = this; tex_ref._TexID = ImTextureID_Invalid; return tex_ref; }
//            ImTextureID         GetTexID() const { return TexID; }
//
//            // Called by Renderer backend
//            // - Call SetTexID() and SetStatus() after honoring texture requests. Never modify TexID and Status directly!
//            // - A backend may decide to destroy a texture that we did not request to destroy, which is fine (e.g. freeing resources), but we immediately set the texture back in _WantCreate mode.
//            void    SetTexID(ImTextureID tex_id) { TexID = tex_id; }
//            void    SetStatus(ImTextureStatus status) { Status = status; if (status == ImTextureStatus_Destroyed && !WantDestroyNextFrame && Pixels != nullptr) Status = ImTextureStatus_WantCreate; }
//        };
//
//        struct ImTextureRef
//        {
//            ImTextureRef() { _TexData = NULL; _TexID = ImTextureID_Invalid; }
//            ImTextureRef(ImTextureID tex_id) { _TexData = NULL; _TexID = tex_id; }
//#if !defined(IMGUI_DISABLE_OBSOLETE_FUNCTIONS) && !defined(ImTextureID)
//            ImTextureRef(void* tex_id) { _TexData = NULL; _TexID = (ImTextureID)(size_t)tex_id; }  // For legacy backends casting to ImTextureID
//#endif
//
//            inline ImTextureID  GetTexID() const;   // == (_TexData ? _TexData->TexID : _TexID) // Implemented below in the file.
//
//            // Members (either are set, never both!)
//            ImTextureData* _TexData;           //      A texture, generally owned by a ImFontAtlas. Will convert to ImTextureID during render loop, after texture has been uploaded.
//            ImTextureID         _TexID;             // _OR_ Low-level backend texture identifier, if already uploaded or created by user/app. Generally provided to e.g. ImGui::Image() calls.
//        };
////-----------------------------------------------------------------------------
//// [SECTION] Font API (ImFontConfig, ImFontGlyph, ImFontAtlasFlags, ImFontAtlas, ImFontGlyphRangesBuilder, ImFont)
////-----------------------------------------------------------------------------
//
//// A font input/source (we may rename this to ImFontSource in the future)
//        struct ImFontConfig
//        {
//            // Data Source
//            char            Name[40];               // <auto>   // Name (strictly to ease debugging, hence limited size buffer)
//            void* FontData;               //          // TTF/OTF data
//            int             FontDataSize;           //          // TTF/OTF data size
//            bool            FontDataOwnedByAtlas;   // true     // TTF/OTF data ownership taken by the owner ImFontAtlas (will delete memory itself). SINCE 1.92, THE DATA NEEDS TO PERSIST FOR WHOLE DURATION OF ATLAS.
//
//            // Options
//            bool            MergeMode;              // false    // Merge into previous ImFont, so you can combine multiple inputs font into one ImFont (e.g. ASCII font + icons + Japanese glyphs). You may want to use GlyphOffset.y when merge font of different heights.
//            bool            PixelSnapH;             // false    // Align every glyph AdvanceX to pixel boundaries. Prevents fractional font size from working correctly! Useful e.g. if you are merging a non-pixel aligned font with the default font. If enabled, OversampleH/V will default to 1.
//            ImS8            OversampleH;            // 0 (2)    // Rasterize at higher quality for sub-pixel positioning. 0 == auto == 1 or 2 depending on size. Note the difference between 2 and 3 is minimal. You can reduce this to 1 for large glyphs save memory. Read https://github.com/nothings/stb/blob/master/tests/oversample/README.md for details.
//            ImS8            OversampleV;            // 0 (1)    // Rasterize at higher quality for sub-pixel positioning. 0 == auto == 1. This is not really useful as we don't use sub-pixel positions on the Y axis.
//            ImWchar         EllipsisChar;           // 0        // Explicitly specify Unicode codepoint of ellipsis character. When fonts are being merged first specified ellipsis will be used.
//            float           SizePixels;             //          // Output size in pixels for rasterizer (more or less maps to the resulting font height).
//            const ImWchar* GlyphRanges;            // NULL     // *LEGACY* THE ARRAY DATA NEEDS TO PERSIST AS LONG AS THE FONT IS ALIVE. Pointer to a user-provided list of Unicode range (2 value per range, values are inclusive, zero-terminated list).
//            const ImWchar* GlyphExcludeRanges;     // NULL     // Pointer to a small user-provided list of Unicode ranges (2 value per range, values are inclusive, zero-terminated list). This is very close to GlyphRanges[] but designed to exclude ranges from a font source, when merging fonts with overlapping glyphs. Use "Input Glyphs Overlap Detection Tool" to find about your overlapping ranges.
//            //ImVec2        GlyphExtraSpacing;      // 0, 0     // (REMOVED AT IT SEEMS LARGELY OBSOLETE. PLEASE REPORT IF YOU WERE USING THIS). Extra spacing (in pixels) between glyphs when rendered: essentially add to glyph->AdvanceX. Only X axis is supported for now.
//            ImVec2          GlyphOffset;            // 0, 0     // Offset (in pixels) all glyphs from this font input. Absolute value for default size, other sizes will scale this value.
//            float           GlyphMinAdvanceX;       // 0        // Minimum AdvanceX for glyphs, set Min to align font icons, set both Min/Max to enforce mono-space font. Absolute value for default size, other sizes will scale this value.
//            float           GlyphMaxAdvanceX;       // FLT_MAX  // Maximum AdvanceX for glyphs
//            float           GlyphExtraAdvanceX;     // 0        // Extra spacing (in pixels) between glyphs. Please contact us if you are using this. // FIXME-NEWATLAS: Intentionally unscaled
//            ImU32           FontNo;                 // 0        // Index of font within TTF/OTF file
//            unsigned int    FontLoaderFlags;        // 0        // Settings for custom font builder. THIS IS BUILDER IMPLEMENTATION DEPENDENT. Leave as zero if unsure.
//            //unsigned int  FontBuilderFlags;       // --       // [Renamed in 1.92] Use FontLoaderFlags.
//            float           RasterizerMultiply;     // 1.0f     // Linearly brighten (>1.0f) or darken (<1.0f) font output. Brightening small fonts may be a good workaround to make them more readable. This is a silly thing we may remove in the future.
//            float           RasterizerDensity;      // 1.0f     // [LEGACY: this only makes sense when ImGuiBackendFlags_RendererHasTextures is not supported] DPI scale multiplier for rasterization. Not altering other font metrics: makes it easy to swap between e.g. a 100% and a 400% fonts for a zooming display, or handle Retina screen. IMPORTANT: If you change this it is expected that you increase/decrease font scale roughly to the inverse of this, otherwise quality may look lowered.
//            float           ExtraSizeScale;         // 1.0f     // Extra rasterizer scale over SizePixels.
//
//            // [Internal]
//            ImFontFlags     Flags;                  // Font flags (don't use just yet, will be exposed in upcoming 1.92.X updates)
//            ImFont* DstFont;                // Target font (as we merging fonts, multiple ImFontConfig may target the same font)
//            //const ImFontLoader* FontLoader;         // Custom font backend for this source (default source is the one stored in ImFontAtlas)
//            void* FontLoaderData;         // Font loader opaque storage (per font config)
//
//#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
//            bool            PixelSnapV;             // true    // [Obsoleted in 1.91.6] Align Scaled GlyphOffset.y to pixel boundaries.
//#endif
//             ImFontConfig();
//        };
//
//        // Hold rendering data for one glyph.
//        // (Note: some language parsers may fail to convert the bitfield members, in this case maybe drop store a single u32 or we can rework this)
//        struct ImFontGlyph
//        {
//            unsigned int    Colored : 1;        // Flag to indicate glyph is colored and should generally ignore tinting (make it usable with no shift on little-endian as this is used in loops)
//            unsigned int    Visible : 1;        // Flag to indicate glyph has no visible pixels (e.g. space). Allow early out when rendering.
//            unsigned int    SourceIdx : 4;      // Index of source in parent font
//            unsigned int    Codepoint : 26;     // 0x0000..0x10FFFF
//            float           AdvanceX;           // Horizontal distance to advance cursor/layout position.
//            float           X0, Y0, X1, Y1;     // Glyph corners. Offsets from current cursor/layout position.
//            float           U0, V0, U1, V1;     // Texture coordinates for the current value of ImFontAtlas->TexRef. Cached equivalent of calling GetCustomRect() with PackId.
//            int             PackId;             // [Internal] ImFontAtlasRectId value (FIXME: Cold data, could be moved elsewhere?)
//
//            ImFontGlyph() { memset((void*)this, 0, sizeof(*this)); PackId = -1; }
//        };
//
//        // Helper to build glyph ranges from text/string data. Feed your application strings/characters to it then call BuildRanges().
//        // This is essentially a tightly packed of vector of 64k booleans = 8KB storage.
//        struct ImFontGlyphRangesBuilder
//        {
//            ImVector<ImU32> UsedChars;            // Store 1-bit per Unicode code point (0=unused, 1=used)
//
//            ImFontGlyphRangesBuilder() { Clear(); }
//            inline void     Clear() { int size_in_bytes = (0xFFFF + 1) / 8; UsedChars.resize(size_in_bytes / (int)sizeof(ImU32)); memset(UsedChars.Data, 0, (size_t)size_in_bytes); }
//            inline bool     GetBit(size_t n) const { int off = (int)(n >> 5); ImU32 mask = 1u << (n & 31); return (UsedChars[off] & mask) != 0; }  // Get bit n in the array
//            inline void     SetBit(size_t n) { int off = (int)(n >> 5); ImU32 mask = 1u << (n & 31); UsedChars[off] |= mask; }               // Set bit n in the array
//            inline void     AddChar(ImWchar c) { SetBit(c); }                      // Add character
//             void  AddText(const char* text, const char* text_end = NULL);     // Add string (each character of the UTF-8 string are added)
//             void  AddRanges(const ImWchar* ranges);                           // Add ranges, e.g. builder.AddRanges(ImFontAtlas::GetGlyphRangesDefault()) to force add all of ASCII/Latin+Ext
//             void  BuildRanges(ImVector<ImWchar>* out_ranges);                 // Output new ranges
//        };
//
//        // An opaque identifier to a rectangle in the atlas. -1 when invalid.
//        // The rectangle may move and UV may be invalidated, use GetCustomRect() to retrieve it.
//        typedef int ImFontAtlasRectId;
//#define ImFontAtlasRectId_Invalid -1
//
//        // Output of ImFontAtlas::GetCustomRect() when using custom rectangles.
//        // Those values may not be cached/stored as they are only valid for the current value of atlas->TexRef
//        // (this is in theory derived from ImTextureRect but we use separate structures for reasons)
//        struct ImFontAtlasRect
//        {
//            unsigned short  x, y;               // Position (in current texture)
//            unsigned short  w, h;               // Size
//            ImVec2          uv0, uv1;           // UV coordinates (in current texture)
//
//            ImFontAtlasRect() { memset((void*)this, 0, sizeof(*this)); }
//        };
//
//        // Flags for ImFontAtlas build
//        enum ImFontAtlasFlags_
//        {
//            ImFontAtlasFlags_None = 0,
//            ImFontAtlasFlags_NoPowerOfTwoHeight = 1 << 0,   // Don't round the height to next power of two
//            ImFontAtlasFlags_NoMouseCursors = 1 << 1,   // Don't build software mouse cursors into the atlas (save a little texture memory)
//            ImFontAtlasFlags_NoBakedLines = 1 << 2,   // Don't build thick line textures into the atlas (save a little texture memory, allow support for point/nearest filtering). The AntiAliasedLinesUseTex features uses them, otherwise they will be rendered using polygons (more expensive for CPU/GPU).
//        };
//
//        // Load and rasterize multiple TTF/OTF fonts into a same texture. The font atlas will build a single texture holding:
//        //  - One or more fonts.
//        //  - Custom graphics data needed to render the shapes needed by Dear ImGui.
//        //  - Mouse cursor shapes for software cursor rendering (unless setting 'Flags |= ImFontAtlasFlags_NoMouseCursors' in the font atlas).
//        //  - If you don't call any AddFont*** functions, the default font embedded in the code will be loaded for you.
//        // It is the rendering backend responsibility to upload texture into your graphics API:
//        //  - ImGui_ImplXXXX_RenderDrawData() functions generally iterate platform_io->Textures[] to create/update/destroy each ImTextureData instance.
//        //  - Backend then set ImTextureData's TexID and BackendUserData.
//        //  - Texture id are passed back to you during rendering to identify the texture. Read FAQ entry about ImTextureID/ImTextureRef for more details.
//        // Legacy path:
//        //  - Call Build() + GetTexDataAsAlpha8() or GetTexDataAsRGBA32() to build and retrieve pixels data.
//        //  - Call SetTexID(my_tex_id); and pass the pointer/identifier to your texture in a format natural to your graphics API.
//        // Common pitfalls:
//        // - If you pass a 'glyph_ranges' array to AddFont*** functions, you need to make sure that your array persists up until the
//        //   atlas is build (when calling GetTexData*** or Build()). We only copy the pointer, not the data.
//        // - Important: By default, AddFontFromMemoryTTF() takes ownership of the data. Even though we are not writing to it, we will free the pointer on destruction.
//        //   You can set font_cfg->FontDataOwnedByAtlas=false to keep ownership of your data and it won't be freed,
//        // - Even though many functions are suffixed with "TTF", OTF data is supported just as well.
//        // - This is an old API and it is currently awkward for those and various other reasons! We will address them in the future!
//        struct ImFontAtlas
//        {
//             ImFontAtlas();
//             ~ImFontAtlas();
//             ImFont* AddFont(const ImFontConfig* font_cfg);
//             ImFont* AddFontDefault(const ImFontConfig* font_cfg = NULL);        // Selects between AddFontDefaultVector() and AddFontDefaultBitmap().
//             ImFont* AddFontDefaultVector(const ImFontConfig* font_cfg = NULL);  // Embedded scalable font. Recommended at any higher size.
//             ImFont* AddFontDefaultBitmap(const ImFontConfig* font_cfg = NULL);  // Embedded classic pixel-clean font. Recommended at Size 13px with no scaling.
//             ImFont* AddFontFromFileTTF(const char* filename, float size_pixels = 0.0f, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL);
//             ImFont* AddFontFromMemoryTTF(void* font_data, int font_data_size, float size_pixels = 0.0f, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL); // Note: Transfer ownership of 'ttf_data' to ImFontAtlas! Will be deleted after destruction of the atlas. Set font_cfg->FontDataOwnedByAtlas=false to keep ownership of your data and it won't be freed.
//             ImFont* AddFontFromMemoryCompressedTTF(const void* compressed_font_data, int compressed_font_data_size, float size_pixels = 0.0f, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL); // 'compressed_font_data' still owned by caller. Compress with binary_to_compressed_c.cpp.
//             ImFont* AddFontFromMemoryCompressedBase85TTF(const char* compressed_font_data_base85, float size_pixels = 0.0f, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL);              // 'compressed_font_data_base85' still owned by caller. Compress with binary_to_compressed_c.cpp with -base85 parameter.
//             void              RemoveFont(ImFont* font);                       // Remove a font
//             void              CompactCache();                                 // Compact cached glyphs and texture.
//             //void              SetFontLoader(const ImFontLoader* font_loader); // Change font loader at runtime.
//
//            // Clearing the atlas/fonts has little use nowadays, unless you want to batch remove all fonts.
//            // - Since 1.92, you can call ClearFonts() mid-frame, if you load new fonts afterwards.
//            // - As we are transitioning toward our new font system the semantic for those functions gets increasingly misleading and are often a source of issues.
//            //   TL;DR; most likely, don't use any of those functions. We expect to obsolete/rework them.
//             void              Clear();                    // Clear everything (fonts + textures). Don't call mid-frame!
//             void              ClearFonts();               // Clear input+output font data/glyphs. New fonts and textures will be recreated afterwards.
//             void              ClearInputData();           // [OBSOLETE] Clear input data (all ImFontConfig structures including sizes, TTF data, glyph ranges, etc.) = all the data used to build the texture and fonts.
//             void              ClearTexData();             // [OBSOLETE] Clear CPU-side copy of the texture data. Saves RAM once the texture has been copied to graphics memory.
//
//#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
//            // Legacy path for build atlas + retrieving pixel data.
//            // - User is in charge of copying the pixels into graphics memory (e.g. create a texture with your engine). Then store your texture handle with SetTexID().
//            // - The pitch is always = Width * BytesPerPixels (1 or 4)
//            // - Building in RGBA32 format is provided for convenience and compatibility, but note that unless you manually manipulate or copy color data into
//            //   the texture (e.g. when using the AddCustomRect*** api), then the RGB pixels emitted will always be white (~75% of memory/bandwidth waste).
//            // - From 1.92 with backends supporting ImGuiBackendFlags_RendererHasTextures:
//            //   - Calling Build(), GetTexDataAsAlpha8(), GetTexDataAsRGBA32() is not needed.
//            //   - In backend: replace calls to ImFontAtlas::SetTexID() with calls to ImTextureData::SetTexID() after honoring texture creation.
//             bool  Build();                    // Build pixels data. This is called automatically for you by the GetTexData*** functions.
//             void  GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel = NULL); // 1 byte per-pixel
//             void  GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel = NULL); // 4 bytes-per-pixel
//            void            SetTexID(ImTextureID id) { (TexRef._TexID == ImTextureID_Invalid); TexRef._TexData->TexID = id; }                               // Called by legacy backends. May be called before texture creation.
//            void            SetTexID(ImTextureRef id) { IM_ASSERT(TexRef._TexID == ImTextureID_Invalid && id._TexData == NULL); TexRef._TexData->TexID = id._TexID; } // Called by legacy backends.
//            bool            IsBuilt() const { return Fonts.Size > 0 && TexIsBuilt; } // Bit ambiguous: used to detect when user didn't build texture but effectively we should check TexID != 0 except that would be backend dependent..
//#endif
//
//            //-------------------------------------------
//            // Glyph Ranges
//            //-------------------------------------------
//
//            // Since 1.92: specifying glyph ranges is only useful/necessary if your backend doesn't support ImGuiBackendFlags_RendererHasTextures!
//             const ImWchar* GetGlyphRangesDefault();                // Basic Latin, Extended Latin
//#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
//            // Helpers to retrieve list of common Unicode ranges (2 value per range, values are inclusive, zero-terminated list)
//            // NB: Make sure that your string are UTF-8 and NOT in your local code page.
//            // Read https://github.com/ocornut/imgui/blob/master/docs/FONTS.md/#about-utf-8-encoding for details.
//            // NB: Consider using ImFontGlyphRangesBuilder to build glyph ranges from textual data.
//             const ImWchar* GetGlyphRangesGreek();                  // Default + Greek and Coptic
//             const ImWchar* GetGlyphRangesKorean();                 // Default + Korean characters
//             const ImWchar* GetGlyphRangesJapanese();               // Default + Hiragana, Katakana, Half-Width, Selection of 2999 Ideographs
//             const ImWchar* GetGlyphRangesChineseFull();            // Default + Half-Width + Japanese Hiragana/Katakana + full set of about 21000 CJK Unified Ideographs
//             const ImWchar* GetGlyphRangesChineseSimplifiedCommon();// Default + Half-Width + Japanese Hiragana/Katakana + set of 2500 CJK Unified Ideographs for common simplified Chinese
//             const ImWchar* GetGlyphRangesCyrillic();               // Default + about 400 Cyrillic characters
//             const ImWchar* GetGlyphRangesThai();                   // Default + Thai characters
//             const ImWchar* GetGlyphRangesVietnamese();             // Default + Vietnamese characters
//#endif
//
//            //-------------------------------------------
//            // [ALPHA] Custom Rectangles/Glyphs API
//            //-------------------------------------------
//
//            // Register and retrieve custom rectangles
//            // - You can request arbitrary rectangles to be packed into the atlas, for your own purpose.
//            // - Since 1.92.0, packing is done immediately in the function call (previously packing was done during the Build call)
//            // - You can render your pixels into the texture right after calling the AddCustomRect() functions.
//            // - VERY IMPORTANT:
//            //   - Texture may be created/resized at any time when calling ImGui or ImFontAtlas functions.
//            //   - IT WILL INVALIDATE RECTANGLE DATA SUCH AS UV COORDINATES. Always use latest values from GetCustomRect().
//            //   - UV coordinates are associated to the current texture identifier aka 'atlas->TexRef'. Both TexRef and UV coordinates are typically changed at the same time.
//            // - If you render colored output into your custom rectangles: set 'atlas->TexPixelsUseColors = true' as this may help some backends decide of preferred texture format.
//            // - Read docs/FONTS.md for more details about using colorful icons.
//            // - Note: this API may be reworked further in order to facilitate supporting e.g. multi-monitor, varying DPI settings.
//            // - (Pre-1.92 names) ------------> (1.92 names)
//            //   - GetCustomRectByIndex()   --> Use GetCustomRect()
//            //   - CalcCustomRectUV()       --> Use GetCustomRect() and read uv0, uv1 fields.
//            //   - AddCustomRectRegular()   --> Renamed to AddCustomRect()
//            //   - AddCustomRectFontGlyph() --> Prefer using custom ImFontLoader inside ImFontConfig
//            //   - ImFontAtlasCustomRect    --> Renamed to ImFontAtlasRect
//             ImFontAtlasRectId AddCustomRect(int width, int height, ImFontAtlasRect* out_r = NULL);// Register a rectangle. Return -1 (ImFontAtlasRectId_Invalid) on error.
//             void              RemoveCustomRect(ImFontAtlasRectId id);                             // Unregister a rectangle. Existing pixels will stay in texture until resized / garbage collected.
//             bool              GetCustomRect(ImFontAtlasRectId id, ImFontAtlasRect* out_r) const;  // Get rectangle coordinates for current texture. Valid immediately, never store this (read above)!
//
//            //-------------------------------------------
//            // Members
//            //-------------------------------------------
//
//            // Input
//            ImFontAtlasFlags            Flags;              // Build flags (see ImFontAtlasFlags_)
//            ImTextureFormat             TexDesiredFormat;   // Desired texture format (default to ImTextureFormat_RGBA32 but may be changed to ImTextureFormat_Alpha8).
//            int                         TexGlyphPadding;    // FIXME: Should be called "TexPackPadding". Padding between glyphs within texture in pixels. Defaults to 1. If your rendering method doesn't rely on bilinear filtering you may set this to 0 (will also need to set AntiAliasedLinesUseTex = false).
//            int                         TexMinWidth;        // Minimum desired texture width. Must be a power of two. Default to 512.
//            int                         TexMinHeight;       // Minimum desired texture height. Must be a power of two. Default to 128.
//            int                         TexMaxWidth;        // Maximum desired texture width. Must be a power of two. Default to 8192.
//            int                         TexMaxHeight;       // Maximum desired texture height. Must be a power of two. Default to 8192.
//            void* UserData;           // Store your own atlas related user-data (if e.g. you have multiple font atlas).
//
//            // Output
//            // - Because textures are dynamically created/resized, the current texture identifier may changed at *ANY TIME* during the frame.
//            // - This should not affect you as you can always use the latest value. But note that any precomputed UV coordinates are only valid for the current TexRef.
//#ifdef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
//            ImTextureRef                TexRef;             // Latest texture identifier == TexData->GetTexRef().
//#else
//            union { ImTextureRef TexRef; ImTextureRef TexID; }; // Latest texture identifier == TexData->GetTexRef(). // RENAMED TexID to TexRef in 1.92.0.
//#endif
//            ImTextureData* TexData;            // Latest texture.
//
//            // [Internal]
//            ImVector<ImTextureData*>    TexList;            // Texture list (most often TexList.Size == 1). TexData is always == TexList.back(). DO NOT USE DIRECTLY, USE GetDrawData().Textures[]/GetPlatformIO().Textures[] instead!
//            bool                        Locked;             // Marked as locked during ImGui::NewFrame()..EndFrame() scope if TexUpdates are not supported. Any attempt to modify the atlas will assert.
//            bool                        RendererHasTextures;// Copy of (BackendFlags & ImGuiBackendFlags_RendererHasTextures) from supporting context.
//            bool                        TexIsBuilt;         // Set when texture was built matching current font input. Mostly useful for legacy IsBuilt() call.
//            bool                        TexPixelsUseColors; // Tell whether our texture data is known to use colors (rather than just alpha channel), in order to help backend select a format or conversion process.
//            ImVec2                      TexUvScale;         // = (1.0f/TexData->TexWidth, 1.0f/TexData->TexHeight). May change as new texture gets created.
//            ImVec2                      TexUvWhitePixel;    // Texture coordinates to a white pixel. May change as new texture gets created.
//            ImVector<ImFont*>           Fonts;              // Hold all the fonts returned by AddFont*. Fonts[0] is the default font upon calling ImGui::NewFrame(), use ImGui::PushFont()/PopFont() to change the current font.
//            ImVector<ImFontConfig>      Sources;            // Source/configuration data
//            ImVec4                      TexUvLines[IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 1];  // UVs for baked anti-aliased lines
//            int                         TexNextUniqueID;    // Next value to be stored in TexData->UniqueID
//            int                         FontNextUniqueID;   // Next value to be stored in ImFont->FontID
//            ImVector<ImDrawListSharedData*> DrawListSharedDatas; // List of users for this atlas. Typically one per Dear ImGui context.
//            ImFontAtlasBuilder* Builder;            // Opaque interface to our data that doesn't need to be public and may be discarded when rebuilding.
//            const ImFontLoader* FontLoader;         // Font loader opaque interface (default to use FreeType when IMGUI_ENABLE_FREETYPE is defined, otherwise default to use stb_truetype). Use SetFontLoader() to change this at runtime.
//            const char* FontLoaderName;     // Font loader name (for display e.g. in About box) == FontLoader->Name
//            void* FontLoaderData;     // Font backend opaque storage
//            unsigned int                FontLoaderFlags;    // Shared flags (for all fonts) for font loader. THIS IS BUILD IMPLEMENTATION DEPENDENT (e.g. Per-font override is also available in ImFontConfig).
//            int                         RefCount;           // Number of contexts using this atlas
//            ImGuiContext* OwnerContext;       // Context which own the atlas will be in charge of updating and destroying it.
//
//            // [Obsolete]
//#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
//    // Legacy: You can request your rectangles to be mapped as font glyph (given a font + Unicode point), so you can render e.g. custom colorful icons and use them as regular glyphs. --> Prefer using a custom ImFontLoader.
//            ImFontAtlasRect             TempRect;           // For old GetCustomRectByIndex() API
//            inline ImFontAtlasRectId    AddCustomRectRegular(int w, int h) { return AddCustomRect(w, h); }                             // RENAMED in 1.92.0
//            inline const ImFontAtlasRect* GetCustomRectByIndex(ImFontAtlasRectId id) { return GetCustomRect(id, &TempRect) ? &TempRect : NULL; } // OBSOLETED in 1.92.0
//            inline void                 CalcCustomRectUV(const ImFontAtlasRect* r, ImVec2* out_uv_min, ImVec2* out_uv_max) const { *out_uv_min = r->uv0; *out_uv_max = r->uv1; }             // OBSOLETED in 1.92.0
//             ImFontAtlasRectId AddCustomRectFontGlyph(ImFont* font, ImWchar codepoint, int w, int h, float advance_x, const ImVec2& offset = ImVec2(0, 0));                            // OBSOLETED in 1.92.0: Use custom ImFontLoader in ImFontConfig
//             ImFontAtlasRectId AddCustomRectFontGlyphForSize(ImFont* font, float font_size, ImWchar codepoint, int w, int h, float advance_x, const ImVec2& offset = ImVec2(0, 0));    // ADDED AND OBSOLETED in 1.92.0
//#endif
//            //unsigned int                      FontBuilderFlags;        // OBSOLETED in 1.92.0: Renamed to FontLoaderFlags.
//            //int                               TexDesiredWidth;         // OBSOLETED in 1.92.0: Force texture width before calling Build(). Must be a power-of-two. If have many glyphs your graphics API have texture size restrictions you may want to increase texture width to decrease height.
//            //typedef ImFontAtlasRect           ImFontAtlasCustomRect;   // OBSOLETED in 1.92.0
//            //typedef ImFontAtlasCustomRect     CustomRect;              // OBSOLETED in 1.72+
//            //typedef ImFontGlyphRangesBuilder  GlyphRangesBuilder;      // OBSOLETED in 1.67+
//        };
//
//        // Font runtime data for a given size
//        // Important: pointers to ImFontBaked are only valid for the current frame.
//        struct ImFontBaked
//        {
//            // [Internal] Members: Hot ~20/24 bytes (for CalcTextSize)
//            ImVector<float>             IndexAdvanceX;      // 12-16 // out // Sparse. Glyphs->AdvanceX in a directly indexable way (cache-friendly for CalcTextSize functions which only this info, and are often bottleneck in large UI).
//            float                       FallbackAdvanceX;   // 4     // out // FindGlyph(FallbackChar)->AdvanceX
//            float                       Size;               // 4     // in  // Height of characters/line, set during loading (doesn't change after loading)
//            float                       RasterizerDensity;  // 4     // in  // Density this is baked at
//
//            // [Internal] Members: Hot ~28/36 bytes (for RenderText loop)
//            ImVector<ImU16>             IndexLookup;        // 12-16 // out // Sparse. Index glyphs by Unicode code-point.
//            ImVector<ImFontGlyph>       Glyphs;             // 12-16 // out // All glyphs.
//            int                         FallbackGlyphIndex; // 4     // out // Index of FontFallbackChar
//
//            // [Internal] Members: Cold
//            float                       Ascent, Descent;    // 4+4   // out // Ascent: distance from top to bottom of e.g. 'A' [0..FontSize] (unscaled)
//            unsigned int                MetricsTotalSurface : 26;// 3  // out // Total surface in pixels to get an idea of the font rasterization/texture cost (not exact, we approximate the cost of padding between glyphs)
//            unsigned int                WantDestroy : 1;         // 0  //     // Queued for destroy
//            unsigned int                LoadNoFallback : 1;      // 0  //     // Disable loading fallback in lower-level calls.
//            unsigned int                LoadNoRenderOnLayout : 1;// 0  //     // Enable a two-steps mode where CalcTextSize() calls will load AdvanceX *without* rendering/packing glyphs. Only advantageous if you know that the glyph is unlikely to actually be rendered, otherwise it is slower because we'd do one query on the first CalcTextSize and one query on the first Draw.
//            int                         LastUsedFrame;         // 4  //     // Record of that time this was bounds
//            ImGuiID                     BakedId;            // 4     //     // Unique ID for this baked storage
//            ImFont* OwnerFont;          // 4-8   // in  // Parent font
//            void* FontLoaderDatas;    // 4-8   //     // Font loader opaque storage (per baked font * sources): single contiguous buffer allocated by imgui, passed to loader.
//
//            // Functions
//             ImFontBaked();
//             void              ClearOutputData();
//             ImFontGlyph* FindGlyph(ImWchar c);               // Return U+FFFD glyph if requested glyph doesn't exists.
//             ImFontGlyph* FindGlyphNoFallback(ImWchar c);     // Return NULL if glyph doesn't exist
//             float             GetCharAdvance(ImWchar c);
//             bool              IsGlyphLoaded(ImWchar c);
//        };
//
//        // Font flags
//        // (in future versions as we redesign font loading API, this will become more important and better documented. for now please consider this as internal/advanced use)
//        enum ImFontFlags_
//        {
//            ImFontFlags_None = 0,
//            ImFontFlags_NoLoadError = 1 << 1,   // Disable throwing an error/assert when calling AddFontXXX() with missing file/data. Calling code is expected to check AddFontXXX() return value.
//            ImFontFlags_NoLoadGlyphs = 1 << 2,   // [Internal] Disable loading new glyphs.
//            ImFontFlags_LockBakedSizes = 1 << 3,   // [Internal] Disable loading new baked sizes, disable garbage collecting current ones. e.g. if you want to lock a font to a single size. Important: if you use this to preload given sizes, consider the possibility of multiple font density used on Retina display.
//            ImFontFlags_ImplicitRefSize = 1 << 4,   // [Internal] Reference size was not set explicitly.
//        };
//
//        // Font runtime data and rendering
//        // - ImFontAtlas automatically loads a default embedded font for you if you didn't load one manually.
//        // - Since 1.92.0 a font may be rendered as any size! Therefore a font doesn't have one specific size.
//        // - Use 'font->GetFontBaked(size)' to retrieve the ImFontBaked* corresponding to a given size.
//        // - If you used g.Font + g.FontSize (which is frequent from the ImGui layer), you can use g.FontBaked as a shortcut, as g.FontBaked == g.Font->GetFontBaked(g.FontSize).
//        struct ImFont
//        {
//            // [Internal] Members: Hot ~12-20 bytes
//            ImFontBaked* LastBaked;          // 4-8   // Cache last bound baked. NEVER USE DIRECTLY. Use GetFontBaked().
//            ImFontAtlas* OwnerAtlas;         // 4-8   // What we have been loaded into.
//            ImFontFlags                 Flags;              // 4     // Font flags.
//            float                       CurrentRasterizerDensity;    // Current rasterizer density. This is a varying state of the font.
//
//            // [Internal] Members: Cold ~24-52 bytes
//            // Conceptually Sources[] is the list of font sources merged to create this font.
//            ImGuiID                     FontId;             // Unique identifier for the font
//            float                       LegacySize;         // 4     // in  // Font size passed to AddFont(). Use for old code calling PushFont() expecting to use that size. (use ImGui::GetFontBaked() to get font baked at current bound size).
//            ImVector<ImFontConfig*>     Sources;            // 16    // in  // List of sources. Pointers within OwnerAtlas->Sources[]
//            ImWchar                     EllipsisChar;       // 2-4   // out // Character used for ellipsis rendering ('...'). If you ever want to temporarily swap this for an alternative/dummy char, make sure to clear EllipsisAutoBake.
//            ImWchar                     FallbackChar;       // 2-4   // out // Character used if a glyph isn't found (U+FFFD, '?')
//            ImU8                        Used8kPagesMap[(IM_UNICODE_CODEPOINT_MAX + 1) / 8192 / 8]; // 1 bytes if ImWchar=ImWchar16, 17 bytes if ImWchar==ImWchar32. Store 1-bit for each block of 8K codepoints that has one active glyph. This is mainly used to facilitate iterations across all used codepoints.
//            bool                        EllipsisAutoBake;   // 1     //     // Mark when the "..." glyph (== EllipsisChar) needs to be generated by combining multiple '.'.
//            ImGuiStorage                RemapPairs;         // 16    //     // Remapping pairs when using AddRemapChar(), otherwise empty.
//#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
//            float                       Scale;              // 4     // in  // Legacy base font scale (~1.0f), multiplied by the per-window font scale which you can adjust with SetWindowFontScale()
//#endif
//
//            // Methods
//             ImFont();
//             ~ImFont();
//             bool              IsGlyphInFont(ImWchar c);
//            bool                        IsLoaded() const { return OwnerAtlas != NULL; }
//            const char* GetDebugName() const { return Sources.Size ? Sources[0]->Name : "<unknown>"; } // Fill ImFontConfig::Name.
//
//            // [Internal] Don't use!
//            // 'max_width' stops rendering after a certain width (could be turned into a 2d size). FLT_MAX to disable.
//            // 'wrap_width' enable automatic word-wrapping across multiple lines to fit into given width. 0.0f to disable.
//             ImFontBaked* GetFontBaked(float font_size, float density = -1.0f);  // Get or create baked data for given size
//             ImVec2            CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end = NULL, const char** out_remaining = NULL);
//             const char* CalcWordWrapPosition(float size, const char* text, const char* text_end, float wrap_width);
//             void              RenderChar(ImDrawList* draw_list, float size, const ImVec2& pos, ImU32 col, ImWchar c, const ImVec4* cpu_fine_clip = NULL);
//             void              RenderText(ImDrawList* draw_list, float size, const ImVec2& pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width = 0.0f, ImDrawTextFlags flags = 0);
//#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
//            inline const char* CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) { return CalcWordWrapPosition(LegacySize * scale, text, text_end, wrap_width); } // Obsoleted old name in 1.92.0. Note how `scale` was to `size`.
//#endif
//
//            // [Internal] Don't use!
//             void              ClearOutputData();
//             void              AddRemapChar(ImWchar from_codepoint, ImWchar to_codepoint); // Makes 'from_codepoint' character points to 'to_codepoint' glyph.
//             bool              IsGlyphRangeUnused(unsigned int c_begin, unsigned int c_last);
//        };


	}
}