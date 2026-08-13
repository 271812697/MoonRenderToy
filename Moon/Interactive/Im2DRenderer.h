#pragma once 
#include <Eigen/Core>
#include <vector>
#include <unordered_map>
namespace MOON {
	namespace Render2D{
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
	typedef unsigned int        ImU32;
	typedef unsigned int ImDrawIdx;
	typedef int ImDrawFlags;
	typedef int ImDrawListFlags;
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
		ImDrawCmd() { memset(this,0,sizeof(*this)); }
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
	class Im2DRender {
	public:
		~Im2DRender();
		static Im2DRender& instance();
		ImDrawList* getDrawList();
		void newFrame();
		void endFrame();
	private:
		Im2DRender();
		ImDrawList drawList;
	};

}


}