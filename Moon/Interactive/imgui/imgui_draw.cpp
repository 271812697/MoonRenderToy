#include "Interactive/imgui/imgui.h"
#include <stdio.h>      // vsnprintf, sscanf, printf
#include <stdint.h>     // intptr_t
#include <math.h>

namespace MOON {
    inline ImVec2  operator*(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x * rhs, lhs.y * rhs); }
    inline ImVec2  operator*(const float lhs, const ImVec2& rhs) { return ImVec2(lhs * rhs.x, lhs * rhs.y); }
    inline ImVec2  operator/(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x / rhs, lhs.y / rhs); }
    inline ImVec2  operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
    inline ImVec2  operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
    inline ImVec2  operator*(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
    inline ImVec2  operator/(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y); }
    inline ImVec2  operator+(const ImVec2& lhs) { return lhs; }
    inline ImVec2  operator-(const ImVec2& lhs) { return ImVec2(-lhs.x, -lhs.y); }
    inline ImVec2& operator*=(ImVec2& lhs, const float rhs) { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
    inline ImVec2& operator/=(ImVec2& lhs, const float rhs) { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
    inline ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
    inline ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
    inline ImVec2& operator*=(ImVec2& lhs, const ImVec2& rhs) { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
    inline ImVec2& operator/=(ImVec2& lhs, const ImVec2& rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }
    template<typename T> T ImMin(T lhs, T rhs) { return lhs < rhs ? lhs : rhs; }
    template<typename T> T ImMax(T lhs, T rhs) { return lhs >= rhs ? lhs : rhs; }
    template<typename T> T ImClamp(T v, T mn, T mx) { return (v < mn) ? mn : (v > mx) ? mx : v; }
    inline float  ImRsqrt(float x) { return 1.0f /sqrtf(x); }
    inline int    ImAbs(int x) { return x < 0 ? -x : x; }
    inline float  ImAbs(float x) { return fabsf(x); }
    inline double ImAbs(double x) { return fabs(x); }
    inline float  ImFloor(float f) { return (float)((f >= 0 || (float)(int)f == f) ? (int)f : (int)f - 1); } // Decent replacement for floorf()
    inline ImVec2 ImFloor(const ImVec2& v) { return ImVec2(ImFloor(v.x), ImFloor(v.y)); }
#define IM_STATIC_ASSERT(_COND)         static_assert(_COND, "")
#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX                     512
    // ImDrawList: Lookup table size for adaptive arc drawing, cover full circle.
#ifndef IM_DRAWLIST_ARCFAST_TABLE_SIZE
#define IM_DRAWLIST_ARCFAST_TABLE_SIZE                          48 // Number of samples in lookup table.
#endif
#define IM_DRAWLIST_ARCFAST_SAMPLE_MAX                          IM_DRAWLIST_ARCFAST_TABLE_SIZE // Sample index _PathArcToFastEx() for 360 angle.
#define ImFabs(X)           fabsf(X)
#define ImSqrt(X)           sqrtf(X)
#define ImFmod(X, Y)        fmodf((X), (Y))
#define ImCos(X)            cosf(X)
#define ImSin(X)            sinf(X)
#define ImAcos(X)           acosf(X)
#define ImAtan2(Y, X)       atan2f((Y), (X))
#define ImAtof(STR)         atof(STR)
#define ImCeil(X)           ceilf(X)
#define IM_PI                           3.14159265358979323846f
    ImDrawList::ImDrawList(ImDrawListSharedData* shared_data)
    {
        memset((void*)this, 0, sizeof(*this));
        _SetDrawListSharedData(shared_data);
        _InvFringeScale = 1.0;
    }
    ImDrawList::ImDrawList()
    {
        memset((void*)this, 0, sizeof(*this));
        _SetDrawListSharedData(nullptr);
        _InvFringeScale = 1.0;
    }
    ImDrawList::~ImDrawList()
    {
        _ClearFreeMemory();
        _SetDrawListSharedData(NULL);
    }
    void ImDrawList::_SetDrawListSharedData(ImDrawListSharedData* data)
    {

    }

    // Initialize before use in a new frame. We always have a command ready in the buffer.
    // In the majority of cases, you would want to call PushClipRect() and PushTexture() after this.
    void ImDrawList::_ResetForNewFrame()
    {
        // Verify that the ImDrawCmd fields we want to memcmp() are contiguous in memory to match ImDrawCmdHeader.
        IM_STATIC_ASSERT(offsetof(ImDrawCmd, ClipRect) == 0);
        IM_STATIC_ASSERT(offsetof(ImDrawCmd, TexRef) == sizeof(ImVec4));
        IM_STATIC_ASSERT(offsetof(ImDrawCmd, VtxOffset) == sizeof(ImVec4) + sizeof(ImTextureRef));
        IM_STATIC_ASSERT(offsetof(ImDrawCmd, ClipRect) == offsetof(ImDrawCmdHeader, ClipRect));
        IM_STATIC_ASSERT(offsetof(ImDrawCmd, TexRef) == offsetof(ImDrawCmdHeader, TexRef));
        IM_STATIC_ASSERT(offsetof(ImDrawCmd, VtxOffset) == offsetof(ImDrawCmdHeader, VtxOffset));
      
        CmdBuffer.resize(0);
        IdxBuffer.resize(0);
        VtxBuffer.resize(0);
       // Flags = _Data->InitialFlags;
        memset(&_CmdHeader, 0, sizeof(_CmdHeader));
        _VtxCurrentIdx = 0;
        _VtxWritePtr = NULL;
        _IdxWritePtr = NULL;
        _ClipRectStack.resize(0);
        _TextureStack.resize(0);
        _CallbacksDataBuf.resize(0);
        _Path.resize(0);
       // _Splitter.Clear();
        CmdBuffer.push_back(ImDrawCmd());
        // Caller needs to bet _SetPixelDensity() as well.
    }

    void ImDrawList::_ClearFreeMemory()
    {
        CmdBuffer.clear();
        IdxBuffer.clear();
        VtxBuffer.clear();
        Flags = ImDrawListFlags_None;
        _VtxCurrentIdx = 0;
        _VtxWritePtr = NULL;
        _IdxWritePtr = NULL;
        _ClipRectStack.clear();
        _TextureStack.clear();
        _CallbacksDataBuf.clear();
        _Path.clear();
    }

    // Note: For multi-threaded rendering, consider using `imgui_threaded_rendering` from https://github.com/ocornut/imgui_club
    ImDrawList* ImDrawList::CloneOutput() const
    {
        ImDrawList* dst = new ImDrawList(NULL);
        dst->CmdBuffer = CmdBuffer;
        dst->IdxBuffer = IdxBuffer;
        dst->VtxBuffer = VtxBuffer;
        dst->Flags = Flags;
        return dst;
    }

    void ImDrawList::AddDrawCmd()
    {
        ImDrawCmd draw_cmd;
        draw_cmd.ClipRect = _CmdHeader.ClipRect;    // Same as calling ImDrawCmd_HeaderCopy()
        draw_cmd.TexRef = _CmdHeader.TexRef;
        draw_cmd.VtxOffset = _CmdHeader.VtxOffset;
        draw_cmd.IdxOffset = IdxBuffer.Size;

        IM_ASSERT(draw_cmd.ClipRect.x <= draw_cmd.ClipRect.z && draw_cmd.ClipRect.y <= draw_cmd.ClipRect.w);
        CmdBuffer.push_back(draw_cmd);
    }
    void ImDrawList::_PopUnusedDrawCmd()
    {
        while (CmdBuffer.Size > 0)
        {
            ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
            if (curr_cmd->ElemCount != 0 )
                return;// break;
            CmdBuffer.pop_back();
        }
    }
    // Compare ClipRect, TexRef and VtxOffset with a single memcmp()
#define ImDrawCmd_HeaderSize                            (offsetof(ImDrawCmd, VtxOffset) + sizeof(unsigned int))
#define ImDrawCmd_HeaderCompare(CMD_LHS, CMD_RHS)       (memcmp(CMD_LHS, CMD_RHS, ImDrawCmd_HeaderSize))    // Compare ClipRect, TexRef, VtxOffset
#define ImDrawCmd_HeaderCopy(CMD_DST, CMD_SRC)          (memcpy(CMD_DST, CMD_SRC, ImDrawCmd_HeaderSize))    // Copy ClipRect, TexRef, VtxOffset
#define ImDrawCmd_AreSequentialIdxOffset(CMD_0, CMD_1)  (CMD_0->IdxOffset + CMD_0->ElemCount == CMD_1->IdxOffset)

// Try to merge two last draw commands
    void ImDrawList::_TryMergeDrawCmds()
    {
       
        ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
        ImDrawCmd* prev_cmd = curr_cmd - 1;
        if (ImDrawCmd_HeaderCompare(curr_cmd, prev_cmd) == 0 && ImDrawCmd_AreSequentialIdxOffset(prev_cmd, curr_cmd) )
        {
            prev_cmd->ElemCount += curr_cmd->ElemCount;
            CmdBuffer.pop_back();
        }
    }
    void ImDrawList::_OnChangedClipRect()
    {
        // If current command is used with different settings we need to add a new command
     
        ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
        if (curr_cmd->ElemCount != 0 && memcmp(&curr_cmd->ClipRect, &_CmdHeader.ClipRect, sizeof(ImVec4)) != 0)
        {
            AddDrawCmd();
            return;
        }
      

        // Try to merge with previous command if it matches, else use current command
        ImDrawCmd* prev_cmd = curr_cmd - 1;
        if (curr_cmd->ElemCount == 0 && CmdBuffer.Size > 1 && ImDrawCmd_HeaderCompare(&_CmdHeader, prev_cmd) == 0 && ImDrawCmd_AreSequentialIdxOffset(prev_cmd, curr_cmd))
        {
            CmdBuffer.pop_back();
            return;
        }
        curr_cmd->ClipRect = _CmdHeader.ClipRect;
    }
    void ImDrawList::_OnChangedTexture()
    {
        // If current command is used with different settings we need to add a new command
        ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
        //if (curr_cmd->ElemCount != 0 && curr_cmd->TexRef != _CmdHeader.TexRef)
        if (curr_cmd->ElemCount != 0 )
        {
            AddDrawCmd();
            return;
        }


        // Try to merge with previous command if it matches, else use current command
        ImDrawCmd* prev_cmd = curr_cmd - 1;
        if (curr_cmd->ElemCount == 0 && CmdBuffer.Size > 1 && ImDrawCmd_HeaderCompare(&_CmdHeader, prev_cmd) == 0 && ImDrawCmd_AreSequentialIdxOffset(prev_cmd, curr_cmd))
        {
            CmdBuffer.pop_back();
            return;
        }
        curr_cmd->TexRef = _CmdHeader.TexRef;
    }

    void ImDrawList::_OnChangedVtxOffset()
    {
        // We don't need to compare curr_cmd->VtxOffset != _CmdHeader.VtxOffset because we know it'll be different at the time we call this.
        _VtxCurrentIdx = 0;
        
        ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
        //IM_ASSERT(curr_cmd->VtxOffset != _CmdHeader.VtxOffset); // See #3349
        if (curr_cmd->ElemCount != 0)
        {
            AddDrawCmd();
            return;
        }
       
        curr_cmd->VtxOffset = _CmdHeader.VtxOffset;
    }

    int ImDrawList::_CalcCircleAutoSegmentCount(float radius) const
    {
        // Automatic segment count
        
        radius *= _InvFringeScale;
        return radius * 4;
        //const int radius_idx = (int)(radius + 0.999f); // ceil to never reduce accuracy
        //if (radius_idx >= 0 && radius_idx < IM_COUNTOF(_Data->CircleSegmentCounts))
        //    return _Data->CircleSegmentCounts[radius_idx]; // Use cached value
        //else
        //    return IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(radius, _Data->CircleTessellationMaxError);
    }

    // Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
    void ImDrawList::PushClipRect(const ImVec2& cr_min, const ImVec2& cr_max, bool intersect_with_current_clip_rect)
    {
        ImVec4 cr(cr_min.x, cr_min.y, cr_max.x, cr_max.y);
        if (intersect_with_current_clip_rect)
        {
            ImVec4 current = _CmdHeader.ClipRect;
            if (cr.x < current.x) cr.x = current.x; // = ClipWith(). Note that passing inverted range wouldn't be fixed here.
            if (cr.y < current.y) cr.y = current.y;
            if (cr.z > current.z) cr.z = current.z;
            if (cr.w > current.w) cr.w = current.w;
        }
        cr.z = ImMax(cr.x, cr.z);
        cr.w = ImMax(cr.y, cr.w);

        _ClipRectStack.push_back(cr);
        _CmdHeader.ClipRect = cr;
        _OnChangedClipRect();
    }

    void ImDrawList::PushClipRectFullScreen()
    {
       // PushClipRect(ImVec2(_Data->ClipRectFullscreen.x, _Data->ClipRectFullscreen.y), ImVec2(_Data->ClipRectFullscreen.z, _Data->ClipRectFullscreen.w));
    }
    void ImDrawList::PopClipRect()
    {
        _ClipRectStack.pop_back();
        //_CmdHeader.ClipRect = (_ClipRectStack.Size == 0) ? _Data->ClipRectFullscreen : _ClipRectStack.Data[_ClipRectStack.Size - 1];
        _OnChangedClipRect();
    }
    void ImDrawList::PushTexture(ImTextureRef tex_ref)
    {
        _TextureStack.push_back(tex_ref);
        _CmdHeader.TexRef = tex_ref;
        if (tex_ref._TexData != NULL)
            IM_ASSERT(tex_ref._TexData->WantDestroyNextFrame == false);
        _OnChangedTexture();
    }

    void ImDrawList::PopTexture()
    {
        _TextureStack.pop_back();
        _CmdHeader.TexRef = (_TextureStack.Size == 0) ? ImTextureRef() : _TextureStack.Data[_TextureStack.Size - 1];
        _OnChangedTexture();
    }

    // This is used by ImGui::PushFont()/PopFont(). It works because we never use _TextureIdStack[] elsewhere than in PushTexture()/PopTexture().
    void ImDrawList::_SetTexture(ImTextureRef tex_ref)
    {
        //if (_CmdHeader.TexRef == tex_ref)
          //  return;
        _CmdHeader.TexRef = tex_ref;
        _TextureStack.back() = tex_ref;
        _OnChangedTexture();
    }

    void ImDrawList::_SetPixelDensity(float pixel_density)
    {
        IM_ASSERT(pixel_density > 0.0f);
        _FringeScale = 1.0f / pixel_density;
        _InvFringeScale = pixel_density;
    }
    void ImDrawList::PrimReserve(int idx_count, int vtx_count)
    {
        // Large mesh support (when enabled)
        
        if (sizeof(ImDrawIdx) == 2 && (_VtxCurrentIdx + vtx_count >= (1 << 16)) && (Flags & ImDrawListFlags_AllowVtxOffset))
        {
            // FIXME: In theory we should be testing that vtx_count <64k here.
            // In practice, RenderText() relies on reserving ahead for a worst case scenario so it is currently useful for us
            // to not make that check until we rework the text functions to handle clipping and large horizontal lines better.
            _CmdHeader.VtxOffset = VtxBuffer.Size;
            _OnChangedVtxOffset();
        }

        ImDrawCmd* draw_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
        draw_cmd->ElemCount += idx_count;

        int vtx_buffer_old_size = VtxBuffer.Size;
        VtxBuffer.resize(vtx_buffer_old_size + vtx_count);
        _VtxWritePtr = VtxBuffer.Data + vtx_buffer_old_size;

        int idx_buffer_old_size = IdxBuffer.Size;
        IdxBuffer.resize(idx_buffer_old_size + idx_count);
        _IdxWritePtr = IdxBuffer.Data + idx_buffer_old_size;
    }

    // Release the number of reserved vertices/indices from the end of the last reservation made with PrimReserve().
    void ImDrawList::PrimUnreserve(int idx_count, int vtx_count)
    {
        ImDrawCmd* draw_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
        draw_cmd->ElemCount -= idx_count;
        VtxBuffer.shrink(VtxBuffer.Size - vtx_count);
        IdxBuffer.shrink(IdxBuffer.Size - idx_count);
    }

    // Fully unrolled with inline call to keep our debug builds decently fast.
    void ImDrawList::PrimRect(const ImVec2& a, const ImVec2& c, ImU32 col)
    {
        ImVec2 b(c.x, a.y), d(a.x, c.y), uv(0,0);
        ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
        _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx + 1); _IdxWritePtr[2] = (ImDrawIdx)(idx + 2);
        _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx + 3);
        _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
        _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
        _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
        _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
        _VtxWritePtr += 4;
        _VtxCurrentIdx += 4;
        _IdxWritePtr += 6;
    }

    void ImDrawList::PrimRectUV(const ImVec2& a, const ImVec2& c, const ImVec2& uv_a, const ImVec2& uv_c, ImU32 col)
    {
        ImVec2 b(c.x, a.y), d(a.x, c.y), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);
        ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
        _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx + 1); _IdxWritePtr[2] = (ImDrawIdx)(idx + 2);
        _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx + 3);
        _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
        _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
        _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
        _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
        _VtxWritePtr += 4;
        _VtxCurrentIdx += 4;
        _IdxWritePtr += 6;
    }

    void ImDrawList::PrimQuadUV(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
    {
        ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
        _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx + 1); _IdxWritePtr[2] = (ImDrawIdx)(idx + 2);
        _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx + 3);
        _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
        _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
        _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
        _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
        _VtxWritePtr += 4;
        _VtxCurrentIdx += 4;
        _IdxWritePtr += 6;
    }
#define IM_NORMALIZE2F_OVER_ZERO(VX,VY)     { float d2 = VX*VX + VY*VY; if (d2 > 0.0f) { float inv_len = ImRsqrt(d2); VX *= inv_len; VY *= inv_len; } } (void)0
#define IM_FIXNORMAL2F_MAX_INVLEN2          100.0f // 500.0f (see #4053, #3366)
#define IM_FIXNORMAL2F(VX,VY)               { float d2 = VX*VX + VY*VY; if (d2 > 0.000001f) { float inv_len2 = 1.0f / d2; if (inv_len2 > IM_FIXNORMAL2F_MAX_INVLEN2) inv_len2 = IM_FIXNORMAL2F_MAX_INVLEN2; VX *= inv_len2; VY *= inv_len2; } } (void)0
    // TODO: Thickness anti-aliased lines cap are missing their AA fringe.
// We avoid using the ImVec2 math operators here to reduce cost to a minimum for debug/non-inlined builds.
    void ImDrawList::AddPolyline(const ImVec2* points, const int points_count, ImU32 col, float thickness, ImDrawFlags flags)
    {
        if (points_count < 2 || (col & IM_COL32_A_MASK) == 0)
            return;

        const bool closed = (flags & ImDrawFlags_Closed) != 0;
        const ImVec2 opaque_uv = {0,0};
        const int count = closed ? points_count : points_count - 1; // The number of line segments we need to draw
        const bool thick_line = (thickness > _FringeScale);

        // If this assert triggers on legacy code:
        // - 1.92.8 (2025/05): swapped two last parameters order: flags, thickness --> thickness, flags. This should normally be caught by compile-time type-checking.
        // - 1.92.8 (2025/05): changed value of ImDrawList_Closed which was previously guaranteed to be == 1. Hardcoded use of 1 or true should be replaced.
        // Read more details near AddRect() + see "API BREAKING CHANGES" section for 1.82, 1.90 and 1.92.8.
        //IM_ASSERT_USER_ERROR_RET((flags & ImDrawFlags_InvalidMask_) == 0, "Incorrect parameter. Did you swap 'thickness' and 'flags'?");

        if (Flags & ImDrawListFlags_AntiAliasedLines)
        {
            // Anti-aliased stroke
            const float AA_SIZE = _FringeScale;
            const ImU32 col_trans = col & ~IM_COL32_A_MASK;

            // Thicknesses <1.0 should behave like thickness 1.0
            thickness = ImMax(thickness, 1.0f);
            const int integer_thickness = (int)thickness;
            const float fractional_thickness = thickness - integer_thickness;

            // Do we want to draw this line using a texture?
            // - For now, only draw integer-width lines using textures to avoid issues with the way scaling occurs, could be improved.
            // - If AA_SIZE is not 1.0f we cannot use the texture path.
            const bool use_texture = (Flags & ImDrawListFlags_AntiAliasedLinesUseTex) && (integer_thickness < IM_DRAWLIST_TEX_LINES_WIDTH_MAX) && (fractional_thickness <= 0.00001f) && (AA_SIZE == 1.0f);

            // We should never hit this, because NewFrame() doesn't set ImDrawListFlags_AntiAliasedLinesUseTex unless ImFontAtlasFlags_NoBakedLines is off
          

            const int idx_count = use_texture ? (count * 6) : (thick_line ? count * 18 : count * 12);
            const int vtx_count = use_texture ? (points_count * 2) : (thick_line ? points_count * 4 : points_count * 3);
            PrimReserve(idx_count, vtx_count);

            // Temporary buffer
            // The first <points_count> items are normals at each line point, then after that there are either 2 or 4 temp points for each line point
            ImVector<ImVec2>TempBuffer;
            TempBuffer.reserve_discard(points_count * ((use_texture || !thick_line) ? 3 : 5));
            ImVec2* temp_normals = TempBuffer.Data;
            ImVec2* temp_points = temp_normals + points_count;

            // Calculate normals (tangents) for each line segment
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
                float dx = points[i2].x - points[i1].x;
                float dy = points[i2].y - points[i1].y;
                IM_NORMALIZE2F_OVER_ZERO(dx, dy);
                temp_normals[i1].x = dy;
                temp_normals[i1].y = -dx;
            }
            if (!closed)
                temp_normals[points_count - 1] = temp_normals[points_count - 2];

            // If we are drawing a one-pixel-wide line without a texture, or a textured line of any width, we only need 2 or 3 vertices per point
            if (use_texture || !thick_line)
            {
                // [PATH 1] Texture-based lines (thick or non-thick)
                // [PATH 2] Non texture-based lines (non-thick)

                // The width of the geometry we need to draw - this is essentially <thickness> pixels for the line itself, plus "one pixel" for AA.
                // - In the texture-based path, we don't use AA_SIZE here because the +1 is tied to the generated texture
                //   (see ImFontAtlasBuildRenderLinesTexData() function), and so alternate values won't work without changes to that code.
                // - In the non texture-based paths, we would allow AA_SIZE to potentially be != 1.0f with a patch (e.g. fringe_scale patch to
                //   allow scaling geometry while preserving one-screen-pixel AA fringe).
                const float half_draw_size = use_texture ? ((thickness * 0.5f) + 1) : AA_SIZE;

                // If line is not closed, the first and last points need to be generated differently as there are no normals to blend
                if (!closed)
                {
                    temp_points[0] = points[0] + temp_normals[0] * half_draw_size;
                    temp_points[1] = points[0] - temp_normals[0] * half_draw_size;
                    temp_points[(points_count - 1) * 2 + 0] = points[points_count - 1] + temp_normals[points_count - 1] * half_draw_size;
                    temp_points[(points_count - 1) * 2 + 1] = points[points_count - 1] - temp_normals[points_count - 1] * half_draw_size;
                }

                // Generate the indices to form a number of triangles for each line segment, and the vertices for the line edges
                // This takes points n and n+1 and writes into n+1, with the first point in a closed line being generated from the final one (as n+1 wraps)
                // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
                unsigned int idx1 = _VtxCurrentIdx; // Vertex index for start of line segment
                for (int i1 = 0; i1 < count; i1++) // i1 is the first point of the line segment
                {
                    const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1; // i2 is the second point of the line segment
                    const unsigned int idx2 = ((i1 + 1) == points_count) ? _VtxCurrentIdx : (idx1 + (use_texture ? 2 : 3)); // Vertex index for end of segment

                    // Average normals
                    float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
                    float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
                    IM_FIXNORMAL2F(dm_x, dm_y);
                    dm_x *= half_draw_size; // dm_x, dm_y are offset to the outer edge of the AA area
                    dm_y *= half_draw_size;

                    // Add temporary vertices for the outer edges
                    ImVec2* out_vtx = &temp_points[i2 * 2];
                    out_vtx[0].x = points[i2].x + dm_x;
                    out_vtx[0].y = points[i2].y + dm_y;
                    out_vtx[1].x = points[i2].x - dm_x;
                    out_vtx[1].y = points[i2].y - dm_y;

                    if (use_texture)
                    {
                        // Add indices for two triangles
                        _IdxWritePtr[0] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[1] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[2] = (ImDrawIdx)(idx1 + 1); // Right tri
                        _IdxWritePtr[3] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[4] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[5] = (ImDrawIdx)(idx2 + 0); // Left tri
                        _IdxWritePtr += 6;
                    }
                    else
                    {
                        // Add indexes for four triangles
                        _IdxWritePtr[0] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[1] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[2] = (ImDrawIdx)(idx1 + 2); // Right tri 1
                        _IdxWritePtr[3] = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[4] = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx2 + 0); // Right tri 2
                        _IdxWritePtr[6] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[7] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[8] = (ImDrawIdx)(idx1 + 0); // Left tri 1
                        _IdxWritePtr[9] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[10] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[11] = (ImDrawIdx)(idx2 + 1); // Left tri 2
                        _IdxWritePtr += 12;
                    }

                    idx1 = idx2;
                }

                // Add vertices for each point on the line
                if (use_texture)
                {
                    // If we're using textures we only need to emit the left/right edge vertices
                    ImVec4 tex_uvs = { 0,0,0,0 };// _Data->TexUvLines[integer_thickness];
                    /*if (fractional_thickness != 0.0f) // Currently always zero when use_texture==false!
                    {
                        const ImVec4 tex_uvs_1 = _Data->TexUvLines[integer_thickness + 1];
                        tex_uvs.x = tex_uvs.x + (tex_uvs_1.x - tex_uvs.x) * fractional_thickness; // inlined ImLerp()
                        tex_uvs.y = tex_uvs.y + (tex_uvs_1.y - tex_uvs.y) * fractional_thickness;
                        tex_uvs.z = tex_uvs.z + (tex_uvs_1.z - tex_uvs.z) * fractional_thickness;
                        tex_uvs.w = tex_uvs.w + (tex_uvs_1.w - tex_uvs.w) * fractional_thickness;
                    }*/
                    ImVec2 tex_uv0(tex_uvs.x, tex_uvs.y);
                    ImVec2 tex_uv1(tex_uvs.z, tex_uvs.w);
                    for (int i = 0; i < points_count; i++)
                    {
                        _VtxWritePtr[0].pos = temp_points[i * 2 + 0]; _VtxWritePtr[0].uv = tex_uv0; _VtxWritePtr[0].col = col; // Left-side outer edge
                        _VtxWritePtr[1].pos = temp_points[i * 2 + 1]; _VtxWritePtr[1].uv = tex_uv1; _VtxWritePtr[1].col = col; // Right-side outer edge
                        _VtxWritePtr += 2;
                    }
                }
                else
                {
                    // If we're not using a texture, we need the center vertex as well
                    for (int i = 0; i < points_count; i++)
                    {
                        _VtxWritePtr[0].pos = points[i];              _VtxWritePtr[0].uv = opaque_uv; _VtxWritePtr[0].col = col;       // Center of line
                        _VtxWritePtr[1].pos = temp_points[i * 2 + 0]; _VtxWritePtr[1].uv = opaque_uv; _VtxWritePtr[1].col = col_trans; // Left-side outer edge
                        _VtxWritePtr[2].pos = temp_points[i * 2 + 1]; _VtxWritePtr[2].uv = opaque_uv; _VtxWritePtr[2].col = col_trans; // Right-side outer edge
                        _VtxWritePtr += 3;
                    }
                }
            }
            else
            {
                // [PATH 2] Non texture-based lines (thick): we need to draw the solid line core and thus require four vertices per point
                const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;

                // If line is not closed, the first and last points need to be generated differently as there are no normals to blend
                if (!closed)
                {
                    const int points_last = points_count - 1;
                    temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
                    temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
                    temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
                    temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
                    temp_points[points_last * 4 + 0] = points[points_last] + temp_normals[points_last] * (half_inner_thickness + AA_SIZE);
                    temp_points[points_last * 4 + 1] = points[points_last] + temp_normals[points_last] * (half_inner_thickness);
                    temp_points[points_last * 4 + 2] = points[points_last] - temp_normals[points_last] * (half_inner_thickness);
                    temp_points[points_last * 4 + 3] = points[points_last] - temp_normals[points_last] * (half_inner_thickness + AA_SIZE);
                }

                // Generate the indices to form a number of triangles for each line segment, and the vertices for the line edges
                // This takes points n and n+1 and writes into n+1, with the first point in a closed line being generated from the final one (as n+1 wraps)
                // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
                unsigned int idx1 = _VtxCurrentIdx; // Vertex index for start of line segment
                for (int i1 = 0; i1 < count; i1++) // i1 is the first point of the line segment
                {
                    const int i2 = (i1 + 1) == points_count ? 0 : (i1 + 1); // i2 is the second point of the line segment
                    const unsigned int idx2 = (i1 + 1) == points_count ? _VtxCurrentIdx : (idx1 + 4); // Vertex index for end of segment

                    // Average normals
                    float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
                    float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
                    IM_FIXNORMAL2F(dm_x, dm_y);
                    float dm_out_x = dm_x * (half_inner_thickness + AA_SIZE);
                    float dm_out_y = dm_y * (half_inner_thickness + AA_SIZE);
                    float dm_in_x = dm_x * half_inner_thickness;
                    float dm_in_y = dm_y * half_inner_thickness;

                    // Add temporary vertices
                    ImVec2* out_vtx = &temp_points[i2 * 4];
                    out_vtx[0].x = points[i2].x + dm_out_x;
                    out_vtx[0].y = points[i2].y + dm_out_y;
                    out_vtx[1].x = points[i2].x + dm_in_x;
                    out_vtx[1].y = points[i2].y + dm_in_y;
                    out_vtx[2].x = points[i2].x - dm_in_x;
                    out_vtx[2].y = points[i2].y - dm_in_y;
                    out_vtx[3].x = points[i2].x - dm_out_x;
                    out_vtx[3].y = points[i2].y - dm_out_y;

                    // Add indexes
                    _IdxWritePtr[0] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[1] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[2] = (ImDrawIdx)(idx1 + 2);
                    _IdxWritePtr[3] = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[4] = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx2 + 1);
                    _IdxWritePtr[6] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[7] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[8] = (ImDrawIdx)(idx1 + 0);
                    _IdxWritePtr[9] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[10] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[11] = (ImDrawIdx)(idx2 + 1);
                    _IdxWritePtr[12] = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[13] = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[14] = (ImDrawIdx)(idx1 + 3);
                    _IdxWritePtr[15] = (ImDrawIdx)(idx1 + 3); _IdxWritePtr[16] = (ImDrawIdx)(idx2 + 3); _IdxWritePtr[17] = (ImDrawIdx)(idx2 + 2);
                    _IdxWritePtr += 18;

                    idx1 = idx2;
                }

                // Add vertices
                for (int i = 0; i < points_count; i++)
                {
                    _VtxWritePtr[0].pos = temp_points[i * 4 + 0]; _VtxWritePtr[0].uv = opaque_uv; _VtxWritePtr[0].col = col_trans;
                    _VtxWritePtr[1].pos = temp_points[i * 4 + 1]; _VtxWritePtr[1].uv = opaque_uv; _VtxWritePtr[1].col = col;
                    _VtxWritePtr[2].pos = temp_points[i * 4 + 2]; _VtxWritePtr[2].uv = opaque_uv; _VtxWritePtr[2].col = col;
                    _VtxWritePtr[3].pos = temp_points[i * 4 + 3]; _VtxWritePtr[3].uv = opaque_uv; _VtxWritePtr[3].col = col_trans;
                    _VtxWritePtr += 4;
                }
            }
            _VtxCurrentIdx += (ImDrawIdx)vtx_count;
        }
        else
        {
            // [PATH 4] Non texture-based, Non anti-aliased lines
            const int idx_count = count * 6;
            const int vtx_count = count * 4;    // FIXME-OPT: Not sharing edges
            PrimReserve(idx_count, vtx_count);

            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
                const ImVec2& p1 = points[i1];
                const ImVec2& p2 = points[i2];

                float dx = p2.x - p1.x;
                float dy = p2.y - p1.y;
                IM_NORMALIZE2F_OVER_ZERO(dx, dy);
                dx *= (thickness * 0.5f);
                dy *= (thickness * 0.5f);

                _VtxWritePtr[0].pos.x = p1.x + dy; _VtxWritePtr[0].pos.y = p1.y - dx; _VtxWritePtr[0].uv = opaque_uv; _VtxWritePtr[0].col = col;
                _VtxWritePtr[1].pos.x = p2.x + dy; _VtxWritePtr[1].pos.y = p2.y - dx; _VtxWritePtr[1].uv = opaque_uv; _VtxWritePtr[1].col = col;
                _VtxWritePtr[2].pos.x = p2.x - dy; _VtxWritePtr[2].pos.y = p2.y + dx; _VtxWritePtr[2].uv = opaque_uv; _VtxWritePtr[2].col = col;
                _VtxWritePtr[3].pos.x = p1.x - dy; _VtxWritePtr[3].pos.y = p1.y + dx; _VtxWritePtr[3].uv = opaque_uv; _VtxWritePtr[3].col = col;
                _VtxWritePtr += 4;

                _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx + 1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx + 2);
                _IdxWritePtr[3] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[4] = (ImDrawIdx)(_VtxCurrentIdx + 2); _IdxWritePtr[5] = (ImDrawIdx)(_VtxCurrentIdx + 3);
                _IdxWritePtr += 6;
                _VtxCurrentIdx += 4;
            }
        }
    }
    // - We intentionally avoid using ImVec2 and its math operators here to reduce cost to a minimum for debug/non-inlined builds.
// - Filled shapes must always use clockwise winding order. The anti-aliasing fringe depends on it. Counter-clockwise shapes will have "inward" anti-aliasing.
    void ImDrawList::AddConvexPolyFilled(const ImVec2* points, const int points_count, ImU32 col)
    {
        if (points_count < 3 || (col & IM_COL32_A_MASK) == 0)
            return;

        const ImVec2 uv = { 0,0 };// _Data->TexUvWhitePixel;

        if (Flags & ImDrawListFlags_AntiAliasedFill)
        {
            // Anti-aliased Fill
            const float AA_SIZE = _FringeScale;
            const ImU32 col_trans = col & ~IM_COL32_A_MASK;
            const int idx_count = (points_count - 2) * 3 + points_count * 6;
            const int vtx_count = (points_count * 2);
            PrimReserve(idx_count, vtx_count);

            // Add indexes for fill
            unsigned int vtx_inner_idx = _VtxCurrentIdx;
            unsigned int vtx_outer_idx = _VtxCurrentIdx + 1;
            for (int i = 2; i < points_count; i++)
            {
                _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + ((i - 1) << 1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx + (i << 1));
                _IdxWritePtr += 3;
            }

            // Compute normals
            ImVector<ImVec2>TempBuffer;
            TempBuffer.reserve_discard(points_count);
            ImVec2* temp_normals =TempBuffer.Data;
            for (int i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++)
            {
                const ImVec2& p0 = points[i0];
                const ImVec2& p1 = points[i1];
                float dx = p1.x - p0.x;
                float dy = p1.y - p0.y;
                IM_NORMALIZE2F_OVER_ZERO(dx, dy);
                temp_normals[i0].x = dy;
                temp_normals[i0].y = -dx;
            }

            for (int i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++)
            {
                // Average normals
                const ImVec2& n0 = temp_normals[i0];
                const ImVec2& n1 = temp_normals[i1];
                float dm_x = (n0.x + n1.x) * 0.5f;
                float dm_y = (n0.y + n1.y) * 0.5f;
                IM_FIXNORMAL2F(dm_x, dm_y);
                dm_x *= AA_SIZE * 0.5f;
                dm_y *= AA_SIZE * 0.5f;

                // Add vertices
                _VtxWritePtr[0].pos.x = (points[i1].x - dm_x); _VtxWritePtr[0].pos.y = (points[i1].y - dm_y); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
                _VtxWritePtr[1].pos.x = (points[i1].x + dm_x); _VtxWritePtr[1].pos.y = (points[i1].y + dm_y); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
                _VtxWritePtr += 2;

                // Add indexes for fringes
                _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx + (i1 << 1)); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + (i0 << 1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx + (i0 << 1));
                _IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx + (i0 << 1)); _IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx + (i1 << 1)); _IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx + (i1 << 1));
                _IdxWritePtr += 6;
            }
            _VtxCurrentIdx += (ImDrawIdx)vtx_count;
        }
        else
        {
            // Non Anti-aliased Fill
            const int idx_count = (points_count - 2) * 3;
            const int vtx_count = points_count;
            PrimReserve(idx_count, vtx_count);
            for (int i = 0; i < vtx_count; i++)
            {
                _VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
                _VtxWritePtr++;
            }
            for (int i = 2; i < points_count; i++)
            {
                _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx + i - 1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx + i);
                _IdxWritePtr += 3;
            }
            _VtxCurrentIdx += (ImDrawIdx)vtx_count;
        }
    }

    void ImDrawList::_PathArcToFastEx(const ImVec2& center, float radius, int a_min_sample, int a_max_sample, int a_step)
    {
        static ImVec2 arcTable[IM_DRAWLIST_ARCFAST_SAMPLE_MAX];
        static bool initArc = false;
        if (!initArc) {
            initArc = true;
            for (int i = 0; i < IM_DRAWLIST_ARCFAST_SAMPLE_MAX; i++) {
                double angle = i * 1.0 / (IM_DRAWLIST_ARCFAST_SAMPLE_MAX - 1) * 2 * IM_PI;
                arcTable[i] = { static_cast<float>(cos(angle)) ,static_cast<float>(sin(angle)) };
            }
        }
        if (radius < 0.5f)
        {
            _Path.push_back(center);
            return;
        }

        // Calculate arc auto segment step size
        if (a_step <= 0)
            a_step = IM_DRAWLIST_ARCFAST_SAMPLE_MAX / _CalcCircleAutoSegmentCount(radius);

        // Make sure we never do steps larger than one quarter of the circle
        a_step = ImClamp(a_step, 1, IM_DRAWLIST_ARCFAST_TABLE_SIZE / 4);

        const int sample_range = ImAbs(a_max_sample - a_min_sample);
        const int a_next_step = a_step;

        int samples = sample_range + 1;
        bool extra_max_sample = false;
        if (a_step > 1)
        {
            samples = sample_range / a_step + 1;
            const int overstep = sample_range % a_step;

            if (overstep > 0)
            {
                extra_max_sample = true;
                samples++;

                // When we have overstep to avoid awkwardly looking one long line and one tiny one at the end,
                // distribute first step range evenly between them by reducing first step size.
                if (sample_range > 0)
                    a_step -= (a_step - overstep) / 2;
            }
        }

        _Path.resize(_Path.Size + samples);
        ImVec2* out_ptr = _Path.Data + (_Path.Size - samples);

        int sample_index = a_min_sample;
        if (sample_index < 0 || sample_index >= IM_DRAWLIST_ARCFAST_SAMPLE_MAX)
        {
            sample_index = sample_index % IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
            if (sample_index < 0)
                sample_index += IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
        }

        if (a_max_sample >= a_min_sample)
        {
            for (int a = a_min_sample; a <= a_max_sample; a += a_step, sample_index += a_step, a_step = a_next_step)
            {
                // a_step is clamped to IM_DRAWLIST_ARCFAST_SAMPLE_MAX, so we have guaranteed that it will not wrap over range twice or more
                if (sample_index >= IM_DRAWLIST_ARCFAST_SAMPLE_MAX)
                    sample_index -= IM_DRAWLIST_ARCFAST_SAMPLE_MAX;

                const ImVec2 s = arcTable[sample_index];
                out_ptr->x = center.x + s.x * radius;
                out_ptr->y = center.y + s.y * radius;
                out_ptr++;
            }
        }
        else
        {
            for (int a = a_min_sample; a >= a_max_sample; a -= a_step, sample_index -= a_step, a_step = a_next_step)
            {
                // a_step is clamped to IM_DRAWLIST_ARCFAST_SAMPLE_MAX, so we have guaranteed that it will not wrap over range twice or more
                if (sample_index < 0)
                    sample_index += IM_DRAWLIST_ARCFAST_SAMPLE_MAX;

                const ImVec2 s = arcTable[sample_index];
                out_ptr->x = center.x + s.x * radius;
                out_ptr->y = center.y + s.y * radius;
                out_ptr++;
            }
        }

        if (extra_max_sample)
        {
            int normalized_max_sample = a_max_sample % IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
            if (normalized_max_sample < 0)
                normalized_max_sample += IM_DRAWLIST_ARCFAST_SAMPLE_MAX;

            const ImVec2 s = arcTable[normalized_max_sample];
            out_ptr->x = center.x + s.x * radius;
            out_ptr->y = center.y + s.y * radius;
            out_ptr++;
        }

       
    }

    void ImDrawList::_PathArcToN(const ImVec2& center, float radius, float a_min, float a_max, int num_segments)
    {
        if (radius < 0.5f)
        {
            _Path.push_back(center);
            return;
        }

        // Note that we are adding a point at both a_min and a_max.
        // If you are trying to draw a full closed circle you don't want the overlapping points!
        _Path.reserve(_Path.Size + (num_segments + 1));
        for (int i = 0; i <= num_segments; i++)
        {
            const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
            _Path.push_back(ImVec2(center.x + cosf(a) * radius, center.y + sinf(a) * radius));
        }
    }

    // 0: East, 3: South, 6: West, 9: North, 12: East
    void ImDrawList::PathArcToFast(const ImVec2& center, float radius, int a_min_of_12, int a_max_of_12)
    {
        if (radius < 0.5f)
        {
            _Path.push_back(center);
            return;
        }
        _PathArcToFastEx(center, radius, a_min_of_12 * IM_DRAWLIST_ARCFAST_SAMPLE_MAX / 12, a_max_of_12 * IM_DRAWLIST_ARCFAST_SAMPLE_MAX / 12, 0);
    }

    void ImDrawList::PathArcTo(const ImVec2& center, float radius, float a_min, float a_max, int num_segments)
    {
        if (radius < 0.5f)
        {
            _Path.push_back(center);
            return;
        }

        if (num_segments > 0)
        {
            _PathArcToN(center, radius, a_min, a_max, num_segments);
            return;
        }

        // Automatic segment count
        if (radius <= 140.11)
        {
            const bool a_is_reverse = a_max < a_min;

            // We are going to use precomputed values for mid samples.
            // Determine first and last sample in lookup table that belong to the arc.
            const float a_min_sample_f = IM_DRAWLIST_ARCFAST_SAMPLE_MAX * a_min / (IM_PI * 2.0f);
            const float a_max_sample_f = IM_DRAWLIST_ARCFAST_SAMPLE_MAX * a_max / (IM_PI * 2.0f);

            const int a_min_sample = a_is_reverse ? (int)ImFloor(a_min_sample_f) : (int)ImCeil(a_min_sample_f);
            const int a_max_sample = a_is_reverse ? (int)ImCeil(a_max_sample_f) : (int)ImFloor(a_max_sample_f);
            const int a_mid_samples = a_is_reverse ? ImMax(a_min_sample - a_max_sample, 0) : ImMax(a_max_sample - a_min_sample, 0);

            const float a_min_segment_angle = a_min_sample * IM_PI * 2.0f / IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
            const float a_max_segment_angle = a_max_sample * IM_PI * 2.0f / IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
            const bool a_emit_start = ImAbs(a_min_segment_angle - a_min) >= 1e-5f;
            const bool a_emit_end = ImAbs(a_max - a_max_segment_angle) >= 1e-5f;

            _Path.reserve(_Path.Size + (a_mid_samples + 1 + (a_emit_start ? 1 : 0) + (a_emit_end ? 1 : 0)));
            if (a_emit_start)
                _Path.push_back(ImVec2(center.x + ImCos(a_min) * radius, center.y + ImSin(a_min) * radius));
            if (a_mid_samples > 0)
                _PathArcToFastEx(center, radius, a_min_sample, a_max_sample, 0);
            if (a_emit_end)
                _Path.push_back(ImVec2(center.x + ImCos(a_max) * radius, center.y + ImSin(a_max) * radius));
        }
        else
        {
            const float arc_length = ImAbs(a_max - a_min);
            const int circle_segment_count = _CalcCircleAutoSegmentCount(radius);
            const int arc_segment_count = ImMax((int)ImCeil(circle_segment_count * arc_length / (IM_PI * 2.0f)), 1);
            _PathArcToN(center, radius, a_min, a_max, arc_segment_count);
        }
    }
    void ImDrawList::PathEllipticalArcTo(const ImVec2& center, const ImVec2& radius, float rot, float a_min, float a_max, int num_segments)
    {
        if (num_segments <= 0)
            num_segments = _CalcCircleAutoSegmentCount(ImMax(radius.x, radius.y)); // A bit pessimistic, maybe there's a better computation to do here.

        _Path.reserve(_Path.Size + (num_segments + 1));

        const float cos_rot = ImCos(rot);
        const float sin_rot = ImSin(rot);
        for (int i = 0; i <= num_segments; i++)
        {
            const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
            ImVec2 point(ImCos(a) * radius.x, ImSin(a) * radius.y);
            const ImVec2 rel((point.x * cos_rot) - (point.y * sin_rot), (point.x * sin_rot) + (point.y * cos_rot));
            point.x = rel.x + center.x;
            point.y = rel.y + center.y;
            _Path.push_back(point);
        }
    }

    ImVec2 ImBezierCubicCalc(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float t)
    {
        float u = 1.0f - t;
        float w1 = u * u * u;
        float w2 = 3 * u * u * t;
        float w3 = 3 * u * t * t;
        float w4 = t * t * t;
        return ImVec2(w1 * p1.x + w2 * p2.x + w3 * p3.x + w4 * p4.x, w1 * p1.y + w2 * p2.y + w3 * p3.y + w4 * p4.y);
    }

    ImVec2 ImBezierQuadraticCalc(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float t)
    {
        float u = 1.0f - t;
        float w1 = u * u;
        float w2 = 2 * u * t;
        float w3 = t * t;
        return ImVec2(w1 * p1.x + w2 * p2.x + w3 * p3.x, w1 * p1.y + w2 * p2.y + w3 * p3.y);
    }

    // Closely mimics ImBezierCubicClosestPointCasteljau() in imgui.cpp
    static void PathBezierCubicCurveToCasteljau(ImVector<ImVec2>* path, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float max_error_sqr, int level)
    {
        float dx = x4 - x1;
        float dy = y4 - y1;
        float d2 = (x2 - x4) * dy - (y2 - y4) * dx;
        float d3 = (x3 - x4) * dy - (y3 - y4) * dx;
        d2 = (d2 >= 0) ? d2 : -d2;
        d3 = (d3 >= 0) ? d3 : -d3;
        if ((d2 + d3) * (d2 + d3) < max_error_sqr * (dx * dx + dy * dy))
        {
            path->push_back(ImVec2(x4, y4));
        }
        else if (level < 10)
        {
            float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
            float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
            float x34 = (x3 + x4) * 0.5f, y34 = (y3 + y4) * 0.5f;
            float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
            float x234 = (x23 + x34) * 0.5f, y234 = (y23 + y34) * 0.5f;
            float x1234 = (x123 + x234) * 0.5f, y1234 = (y123 + y234) * 0.5f;
            PathBezierCubicCurveToCasteljau(path, x1, y1, x12, y12, x123, y123, x1234, y1234, max_error_sqr, level + 1);
            PathBezierCubicCurveToCasteljau(path, x1234, y1234, x234, y234, x34, y34, x4, y4, max_error_sqr, level + 1);
        }
    }

    static void PathBezierQuadraticCurveToCasteljau(ImVector<ImVec2>* path, float x1, float y1, float x2, float y2, float x3, float y3, float max_error_sqr, int level)
    {
        float dx = x3 - x1, dy = y3 - y1;
        float det = (x2 - x3) * dy - (y2 - y3) * dx;
        if (det * det * 4.0f < max_error_sqr * (dx * dx + dy * dy))
        {
            path->push_back(ImVec2(x3, y3));
        }
        else if (level < 10)
        {
            float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
            float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
            float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
            PathBezierQuadraticCurveToCasteljau(path, x1, y1, x12, y12, x123, y123, max_error_sqr, level + 1);
            PathBezierQuadraticCurveToCasteljau(path, x123, y123, x23, y23, x3, y3, max_error_sqr, level + 1);
        }
    }

    void ImDrawList::PathBezierCubicCurveTo(const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, int num_segments)
    {
        ImVec2 p1 = _Path.back();
        if (num_segments == 0)
        {
          
            float max_error_sqr = (1.12000000 * _FringeScale) * (1.12000000 * _FringeScale);
            PathBezierCubicCurveToCasteljau(&_Path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, max_error_sqr, 0); // Auto-tessellated
        }
        else
        {
            float t_step = 1.0f / (float)num_segments;
            for (int i_step = 1; i_step <= num_segments; i_step++)
                _Path.push_back(ImBezierCubicCalc(p1, p2, p3, p4, t_step * i_step));
        }
    }

    void ImDrawList::PathBezierQuadraticCurveTo(const ImVec2& p2, const ImVec2& p3, int num_segments)
    {
        ImVec2 p1 = _Path.back();
        if (num_segments == 0)
        {
           
            float max_error_sqr = (1.12000000 * _FringeScale) * (1.12000000 * _FringeScale);
            PathBezierQuadraticCurveToCasteljau(&_Path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, max_error_sqr, 0);// Auto-tessellated
        }
        else
        {
            float t_step = 1.0f / (float)num_segments;
            for (int i_step = 1; i_step <= num_segments; i_step++)
                _Path.push_back(ImBezierQuadraticCalc(p1, p2, p3, t_step * i_step));
        }
    }

    void ImDrawList::PathRect(const ImVec2& a, const ImVec2& b, float rounding, ImDrawFlags flags)
    {
        if (rounding >= 0.5f)
        {
            if ((flags & ImDrawFlags_RoundCornersMask_) == 0)
                flags |= ImDrawFlags_RoundCornersAll;

            rounding = ImMin(rounding, ImFabs(b.x - a.x) * (((flags & ImDrawFlags_RoundCornersTop) == ImDrawFlags_RoundCornersTop) || ((flags & ImDrawFlags_RoundCornersBottom) == ImDrawFlags_RoundCornersBottom) ? 0.5f : 1.0f) - 1.0f);
            rounding = ImMin(rounding, ImFabs(b.y - a.y) * (((flags & ImDrawFlags_RoundCornersLeft) == ImDrawFlags_RoundCornersLeft) || ((flags & ImDrawFlags_RoundCornersRight) == ImDrawFlags_RoundCornersRight) ? 0.5f : 1.0f) - 1.0f);
        }
        if (rounding < 0.5f || (flags & ImDrawFlags_RoundCornersMask_) == ImDrawFlags_RoundCornersNone)
        {
            PathLineTo(a);
            PathLineTo(ImVec2(b.x, a.y));
            PathLineTo(b);
            PathLineTo(ImVec2(a.x, b.y));
        }
        else
        {
            const float rounding_tl = (flags & ImDrawFlags_RoundCornersTopLeft) ? rounding : 0.0f;
            const float rounding_tr = (flags & ImDrawFlags_RoundCornersTopRight) ? rounding : 0.0f;
            const float rounding_br = (flags & ImDrawFlags_RoundCornersBottomRight) ? rounding : 0.0f;
            const float rounding_bl = (flags & ImDrawFlags_RoundCornersBottomLeft) ? rounding : 0.0f;
            PathArcToFast(ImVec2(a.x + rounding_tl, a.y + rounding_tl), rounding_tl, 6, 9);
            PathArcToFast(ImVec2(b.x - rounding_tr, a.y + rounding_tr), rounding_tr, 9, 12);
            PathArcToFast(ImVec2(b.x - rounding_br, b.y - rounding_br), rounding_br, 0, 3);
            PathArcToFast(ImVec2(a.x + rounding_bl, b.y - rounding_bl), rounding_bl, 3, 6);
        }
    }

    void ImDrawList::AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness)
    {
        if ((col & IM_COL32_A_MASK) == 0)
            return;
        const ImVec2 points[2] = { ImVec2(p1.x + 0.5f, p1.y + 0.5f), ImVec2(p2.x + 0.5f, p2.y + 0.5f) };
        AddPolyline(points, 2, col, thickness);
    }

    void ImDrawList::AddLineH(float min_x, float max_x, float y, ImU32 col, float thickness)
    {
        if ((col & IM_COL32_A_MASK) == 0)
            return;
        const ImVec2 points[2] = { ImVec2(min_x + 0.5f, y + 0.5f), ImVec2(max_x + 0.5f, y + 0.5f) }; // Same as AddLine() above.
        AddPolyline(points, 2, col, thickness);
    }

    void ImDrawList::AddLineV(float x, float min_y, float max_y, ImU32 col, float thickness)
    {
        if ((col & IM_COL32_A_MASK) == 0)
            return;
        const ImVec2 points[2] = { ImVec2(x + 0.5f, min_y + 0.5f), ImVec2(x + 0.5f, max_y + 0.5f) }; // Same as AddLine() above.
        AddPolyline(points, 2, col, thickness);
    }

    // p_min = upper-left, p_max = lower-right
    // Note we don't render 1 pixels sized rectangles properly.
    void ImDrawList::AddRect(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, float thickness, ImDrawFlags flags)
    {
        // If this assert triggers on legacy code:
        // - 1.92.8 (2025/05): swapped two last parameters order: flags, thickness --> thickness, flags. This should normally be caught by compile-time type-checking.
        // - 1.92.8 (2025/05): changed value of ImDrawList_Closed which was previously guaranteed to be == 1. Hardcoded use of 1 or true should be replaced.
        // - 1.82.0 (2021/03): changed ImDrawCornerFlags to ImDrawFlags_RoundCornersXXX values.
        //   If you used hard-coded 1 to 15 or ~0 in flags to configure corner rounding use the new flags!
        //   - Hard coded support for ~0 == ImDrawFlags_RoundCornersAll.
        //   - Hard coded support for values 0x01 to 0x0F (matching 15 out of 16 old flags combinations) --> see FixRectCornerFlags() in <1.90 code.
        //   - Hard coded 0x00 with 'float rounding > 0.0f' --> replace with ImDrawFlags_RoundCornersNone or use 'float rounding = 0.0f'.
        //   See "API BREAKING CHANGES" section for 1.82, 1.90 and 1.92.8.
        //IM_ASSERT_USER_ERROR_RET((flags & ImDrawFlags_InvalidMask_) == 0, "Incorrect parameter. Did you swap 'thickness' and 'flags'?"); // Or misuse of legacy hard-coded ImDrawCornerFlags values

        if ((col & IM_COL32_A_MASK) == 0)
            return;
        if (Flags & ImDrawListFlags_AntiAliasedLines)
            PathRect(p_min + ImVec2(0.50f, 0.50f), p_max - ImVec2(0.50f, 0.50f), rounding, flags);
        else
            PathRect(p_min + ImVec2(0.50f, 0.50f), p_max - ImVec2(0.49f, 0.49f), rounding, flags); // Better looking lower-right corner and rounded non-AA shapes.
        PathStroke(col, thickness, ImDrawFlags_Closed);
    }

    void ImDrawList::AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, ImDrawFlags flags)
    {
        if ((col & IM_COL32_A_MASK) == 0)
            return;
        if (rounding < 0.5f || (flags & ImDrawFlags_RoundCornersMask_) == ImDrawFlags_RoundCornersNone)
        {
            PrimReserve(6, 4);
            PrimRect(p_min, p_max, col);
        }
        else
        {
            PathRect(p_min, p_max, rounding, flags);
            PathFillConvex(col);
        }
    }

    // p_min = upper-left, p_max = lower-right
    void ImDrawList::AddRectFilledMultiColor(const ImVec2& p_min, const ImVec2& p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left)
    {
        if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
            return;

        const ImVec2 uv = {0,0}; //_Data->TexUvWhitePixel;
        PrimReserve(6, 4);
        PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 1)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 2));
        PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 2)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 3));
        PrimWriteVtx(p_min, uv, col_upr_left);
        PrimWriteVtx(ImVec2(p_max.x, p_min.y), uv, col_upr_right);
        PrimWriteVtx(p_max, uv, col_bot_right);
        PrimWriteVtx(ImVec2(p_min.x, p_max.y), uv, col_bot_left);
    }
    void ImDrawList::AddQuadFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col)
    {
        if ((col & IM_COL32_A_MASK) == 0)
            return;

        PathLineTo(p1);
        PathLineTo(p2);
        PathLineTo(p3);
        PathLineTo(p4);
        PathFillConvex(col);
    }

    void ImDrawList::AddTriangle(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness)
    {
        if ((col & IM_COL32_A_MASK) == 0)
            return;

        PathLineTo(p1);
        PathLineTo(p2);
        PathLineTo(p3);
        PathStroke(col, thickness, ImDrawFlags_Closed);
    }

    void ImDrawList::AddTriangleFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col)
    {
        if ((col & IM_COL32_A_MASK) == 0)
            return;

        PathLineTo(p1);
        PathLineTo(p2);
        PathLineTo(p3);
        PathFillConvex(col);
    }

    void ImDrawList::AddCircle(const ImVec2& center, float radius, ImU32 col, int num_segments, float thickness)
    {
        if ((col & IM_COL32_A_MASK) == 0 || radius < 0.5f)
            return;

        if (num_segments <= 0)
        {
            // Use arc with automatic segment count
            _PathArcToFastEx(center, radius - 0.5f, 0, IM_DRAWLIST_ARCFAST_SAMPLE_MAX, 0);
            _Path.Size--;
        }
        else
        {
            // Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
            num_segments = ImClamp(num_segments, 3, IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX);

            // Because we are filling a closed shape we remove 1 from the count of segments/points
            const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
            PathArcTo(center, radius - 0.5f, 0.0f, a_max, num_segments - 1);
        }

        PathStroke(col, thickness, ImDrawFlags_Closed);
    }

    void ImDrawList::AddCircleFilled(const ImVec2& center, float radius, ImU32 col, int num_segments)
    {
        if ((col & IM_COL32_A_MASK) == 0 || radius < 0.5f)
            return;

        if (num_segments <= 0)
        {
            // Use arc with automatic segment count
            _PathArcToFastEx(center, radius, 0, IM_DRAWLIST_ARCFAST_SAMPLE_MAX, 0);
            _Path.Size--;
        }
        else
        {
            // Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
            num_segments = ImClamp(num_segments, 3, IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX);

            // Because we are filling a closed shape we remove 1 from the count of segments/points
            const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
            PathArcTo(center, radius, 0.0f, a_max, num_segments - 1);
        }

        PathFillConvex(col);
    }
    // Guaranteed to honor 'num_segments'
    void ImDrawList::AddNgon(const ImVec2& center, float radius, ImU32 col, int num_segments, float thickness)
    {
        if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
            return;

        // Because we are filling a closed shape we remove 1 from the count of segments/points
        const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
        PathArcTo(center, radius - 0.5f, 0.0f, a_max, num_segments - 1);
        PathStroke(col, thickness, ImDrawFlags_Closed);
    }

    // Guaranteed to honor 'num_segments'
    void ImDrawList::AddNgonFilled(const ImVec2& center, float radius, ImU32 col, int num_segments)
    {
        if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
            return;

        // Because we are filling a closed shape we remove 1 from the count of segments/points
        const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
        PathArcTo(center, radius, 0.0f, a_max, num_segments - 1);
        PathFillConvex(col);
    }

    // Ellipse
    void ImDrawList::AddEllipse(const ImVec2& center, const ImVec2& radius, ImU32 col, float rot, int num_segments, float thickness)
    {
        if ((col & IM_COL32_A_MASK) == 0)
            return;

        if (num_segments <= 0)
            num_segments = _CalcCircleAutoSegmentCount(ImMax(radius.x, radius.y)); // A bit pessimistic, maybe there's a better computation to do here.

        // Because we are filling a closed shape we remove 1 from the count of segments/points
        const float a_max = IM_PI * 2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
        PathEllipticalArcTo(center, radius, rot, 0.0f, a_max, num_segments - 1);
        PathStroke(col, thickness, ImDrawFlags_Closed);
    }

    void ImDrawList::AddEllipseFilled(const ImVec2& center, const ImVec2& radius, ImU32 col, float rot, int num_segments)
    {
        if ((col & IM_COL32_A_MASK) == 0)
            return;

        if (num_segments <= 0)
            num_segments = _CalcCircleAutoSegmentCount(ImMax(radius.x, radius.y)); // A bit pessimistic, maybe there's a better computation to do here.

        // Because we are filling a closed shape we remove 1 from the count of segments/points
        const float a_max = IM_PI * 2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
        PathEllipticalArcTo(center, radius, rot, 0.0f, a_max, num_segments - 1);
        PathFillConvex(col);
    }

    // Cubic Bezier takes 4 controls points
    void ImDrawList::AddBezierCubic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness, int num_segments)
    {
        if ((col & IM_COL32_A_MASK) == 0)
            return;

        PathLineTo(p1);
        PathBezierCubicCurveTo(p2, p3, p4, num_segments);
        PathStroke(col, thickness);
    }

    // Quadratic Bezier takes 3 controls points
    void ImDrawList::AddBezierQuadratic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness, int num_segments)
    {
        if ((col & IM_COL32_A_MASK) == 0)
            return;

        PathLineTo(p1);
        PathBezierQuadraticCurveTo(p2, p3, num_segments);
        PathStroke(col, thickness);
    }

}