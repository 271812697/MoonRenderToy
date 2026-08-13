#include "Interactive/Im2DRenderer.h"
#include "Qtimgui/imgui/imgui.h"
namespace MOON {
    namespace Render2D {
#define COL32_A_MASK     0xFF000000
#define   DRAWLIST_ARCFAST_SAMPLE_MAX 48
#define NORMALIZE2F_OVER_ZERO(VX,VY)     { float d2 = VX*VX + VY*VY; if (d2 > 0.0f) { float inv_len = 1.0/sqrt(d2); VX *= inv_len; VY *= inv_len; } } (void)0
#define FIXNORMAL2F_MAX_INVLEN2          100.0f // 500.0f (see #4053, #3366)
#define FIXNORMAL2F(VX,VY)               { float d2 = VX*VX + VY*VY; if (d2 > 0.000001f) { float inv_len2 = 1.0f / d2; if (inv_len2 > FIXNORMAL2F_MAX_INVLEN2) inv_len2 = FIXNORMAL2F_MAX_INVLEN2; VX *= inv_len2; VY *= inv_len2; } } (void)0
        ImVec2  operator*(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x * rhs, lhs.y * rhs); }
        ImVec2  operator*(const float lhs, const ImVec2& rhs) { return ImVec2(lhs * rhs.x, lhs * rhs.y); }
        ImVec2  operator/(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x / rhs, lhs.y / rhs); }
        ImVec2  operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
        ImVec2  operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
        ImVec2  operator*(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
        ImVec2  operator/(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y); }
        ImVec2  operator+(const ImVec2& lhs) { return lhs; }
        ImVec2  operator-(const ImVec2& lhs) { return ImVec2(-lhs.x, -lhs.y); }
        ImVec2& operator*=(ImVec2& lhs, const float rhs) { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
        ImVec2& operator/=(ImVec2& lhs, const float rhs) { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
        ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
        ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
        ImVec2& operator*=(ImVec2& lhs, const ImVec2& rhs) { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
        ImVec2& operator/=(ImVec2& lhs, const ImVec2& rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }
        bool          ImTriangleIsClockwise(const ImVec2& a, const ImVec2& b, const ImVec2& c) { return ((b.x - a.x) * (c.y - b.y)) - ((c.x - b.x) * (b.y - a.y)) > 0.0f; }
        bool ImTriangleContainsPoint(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p)
        {
            bool b1 = ((p.x - b.x) * (a.y - b.y) - (p.y - b.y) * (a.x - b.x)) < 0.0f;
            bool b2 = ((p.x - c.x) * (b.y - c.y) - (p.y - c.y) * (b.x - c.x)) < 0.0f;
            bool b3 = ((p.x - a.x) * (c.y - a.y) - (p.y - a.y) * (c.x - a.x)) < 0.0f;
            return (b1 == b2) && (b2 == b3);
        }

        enum ImTriangulatorNodeType
        {
            ImTriangulatorNodeType_Convex,
            ImTriangulatorNodeType_Ear,
            ImTriangulatorNodeType_Reflex
        };

        struct ImTriangulatorNode
        {
            ImTriangulatorNodeType  Type;
            int                     Index;
            ImVec2                  Pos;
            ImTriangulatorNode* Next;
            ImTriangulatorNode* Prev;

            void    Unlink() { Next->Prev = Prev; Prev->Next = Next; }
        };

        struct ImTriangulatorNodeSpan
        {
            ImTriangulatorNode** Data = NULL;
            int                     Size = 0;

            void    push_back(ImTriangulatorNode* node) { Data[Size++] = node; }
            void    find_erase_unsorted(int idx) { for (int i = Size - 1; i >= 0; i--) if (Data[i]->Index == idx) { Data[i] = Data[Size - 1]; Size--; return; } }
        };

        struct ImTriangulator
        {
            static int EstimateTriangleCount(int points_count) { return (points_count < 3) ? 0 : points_count - 2; }
            static int EstimateScratchBufferSize(int points_count) { return sizeof(ImTriangulatorNode) * points_count + sizeof(ImTriangulatorNode*) * points_count * 2; }

            void    Init(const ImVec2* points, int points_count, void* scratch_buffer);
            void    GetNextTriangle(unsigned int out_triangle[3]);     // Return relative indexes for next triangle

            // Internal functions
            void    BuildNodes(const ImVec2* points, int points_count);
            void    BuildReflexes();
            void    BuildEars();
            void    FlipNodeList();
            bool    IsEar(int i0, int i1, int i2, const ImVec2& v0, const ImVec2& v1, const ImVec2& v2) const;
            void    ReclassifyNode(ImTriangulatorNode* node);

            // Internal members
            int                     _TrianglesLeft = 0;
            ImTriangulatorNode* _Nodes = NULL;
            ImTriangulatorNodeSpan  _Ears;
            ImTriangulatorNodeSpan  _Reflexes;
        };

        // Distribute storage for nodes, ears and reflexes.
        // FIXME-OPT: if everything is convex, we could report it to caller and let it switch to an convex renderer
        // (this would require first building reflexes to bail to convex if empty, without even building nodes)
        void ImTriangulator::Init(const ImVec2* points, int points_count, void* scratch_buffer)
        {
            assert(scratch_buffer != NULL && points_count >= 3);
            _TrianglesLeft = EstimateTriangleCount(points_count);
            _Nodes = (ImTriangulatorNode*)scratch_buffer;                          // points_count x Node
            _Ears.Data = (ImTriangulatorNode**)(_Nodes + points_count);                // points_count x Node*
            _Reflexes.Data = (ImTriangulatorNode**)(_Nodes + points_count) + points_count; // points_count x Node*
            BuildNodes(points, points_count);
            BuildReflexes();
            BuildEars();
        }

        void ImTriangulator::BuildNodes(const ImVec2* points, int points_count)
        {
            for (int i = 0; i < points_count; i++)
            {
                _Nodes[i].Type = ImTriangulatorNodeType_Convex;
                _Nodes[i].Index = i;
                _Nodes[i].Pos = points[i];
                _Nodes[i].Next = _Nodes + i + 1;
                _Nodes[i].Prev = _Nodes + i - 1;
            }
            _Nodes[0].Prev = _Nodes + points_count - 1;
            _Nodes[points_count - 1].Next = _Nodes;
        }

        void ImTriangulator::BuildReflexes()
        {
            ImTriangulatorNode* n1 = _Nodes;
            for (int i = _TrianglesLeft; i >= 0; i--, n1 = n1->Next)
            {
                if (ImTriangleIsClockwise(n1->Prev->Pos, n1->Pos, n1->Next->Pos))
                    continue;
                n1->Type = ImTriangulatorNodeType_Reflex;
                _Reflexes.push_back(n1);
            }
        }

        void ImTriangulator::BuildEars()
        {
            ImTriangulatorNode* n1 = _Nodes;
            for (int i = _TrianglesLeft; i >= 0; i--, n1 = n1->Next)
            {
                if (n1->Type != ImTriangulatorNodeType_Convex)
                    continue;
                if (!IsEar(n1->Prev->Index, n1->Index, n1->Next->Index, n1->Prev->Pos, n1->Pos, n1->Next->Pos))
                    continue;
                n1->Type = ImTriangulatorNodeType_Ear;
                _Ears.push_back(n1);
            }
        }

        void ImTriangulator::GetNextTriangle(unsigned int out_triangle[3])
        {
            if (_Ears.Size == 0)
            {
                FlipNodeList();

                ImTriangulatorNode* node = _Nodes;
                for (int i = _TrianglesLeft; i >= 0; i--, node = node->Next)
                    node->Type = ImTriangulatorNodeType_Convex;
                _Reflexes.Size = 0;
                BuildReflexes();
                BuildEars();

                // If we still don't have ears, it means geometry is degenerated.
                if (_Ears.Size == 0)
                {
                    // Return first triangle available, mimicking the behavior of convex fill.
                    assert(_TrianglesLeft > 0); // Geometry is degenerated
                    _Ears.Data[0] = _Nodes;
                    _Ears.Size = 1;
                }
            }

            ImTriangulatorNode* ear = _Ears.Data[--_Ears.Size];
            out_triangle[0] = ear->Prev->Index;
            out_triangle[1] = ear->Index;
            out_triangle[2] = ear->Next->Index;

            ear->Unlink();
            if (ear == _Nodes)
                _Nodes = ear->Next;

            ReclassifyNode(ear->Prev);
            ReclassifyNode(ear->Next);
            _TrianglesLeft--;
        }

        void ImTriangulator::FlipNodeList()
        {
            ImTriangulatorNode* prev = _Nodes;
            ImTriangulatorNode* temp = _Nodes;
            ImTriangulatorNode* current = _Nodes->Next;
            prev->Next = prev;
            prev->Prev = prev;
            while (current != _Nodes)
            {
                temp = current->Next;

                current->Next = prev;
                prev->Prev = current;
                _Nodes->Next = current;
                current->Prev = _Nodes;

                prev = current;
                current = temp;
            }
            _Nodes = prev;
        }

        // A triangle is an ear is no other vertex is inside it. We can test reflexes vertices only (see reference algorithm)
        bool ImTriangulator::IsEar(int i0, int i1, int i2, const ImVec2& v0, const ImVec2& v1, const ImVec2& v2) const
        {
            ImTriangulatorNode** p_end = _Reflexes.Data + _Reflexes.Size;
            for (ImTriangulatorNode** p = _Reflexes.Data; p < p_end; p++)
            {
                ImTriangulatorNode* reflex = *p;
                if (reflex->Index != i0 && reflex->Index != i1 && reflex->Index != i2)
                    if (ImTriangleContainsPoint(v0, v1, v2, reflex->Pos))
                        return false;
            }
            return true;
        }

        void ImTriangulator::ReclassifyNode(ImTriangulatorNode* n1)
        {
            // Classify node
            ImTriangulatorNodeType type;
            const ImTriangulatorNode* n0 = n1->Prev;
            const ImTriangulatorNode* n2 = n1->Next;
            if (!ImTriangleIsClockwise(n0->Pos, n1->Pos, n2->Pos))
                type = ImTriangulatorNodeType_Reflex;
            else if (IsEar(n0->Index, n1->Index, n2->Index, n0->Pos, n1->Pos, n2->Pos))
                type = ImTriangulatorNodeType_Ear;
            else
                type = ImTriangulatorNodeType_Convex;

            // Update lists when a type changes
            if (type == n1->Type)
                return;
            if (n1->Type == ImTriangulatorNodeType_Reflex)
                _Reflexes.find_erase_unsorted(n1->Index);
            else if (n1->Type == ImTriangulatorNodeType_Ear)
                _Ears.find_erase_unsorted(n1->Index);
            if (type == ImTriangulatorNodeType_Reflex)
                _Reflexes.push_back(n1);
            else if (type == ImTriangulatorNodeType_Ear)
                _Ears.push_back(n1);
            n1->Type = type;
        }
        void  ImDrawList::AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness) {
            if ((col & COL32_A_MASK) == 0)
                return;
            const ImVec2 points[2] = { ImVec2(p1.x + 0.5f, p1.y + 0.5f), ImVec2(p2.x + 0.5f, p2.y + 0.5f) };
            AddPolyline(points, 2, col, thickness);
        }
        void ImDrawList::PrimReserve(int idx_count, int vtx_count) {
            ImDrawCmd* draw_cmd = &CmdBuffer.back();
            draw_cmd->ElemCount += idx_count;

            int vtx_buffer_old_size = VtxBuffer.size();
            VtxBuffer.resize(vtx_buffer_old_size + vtx_count);
            _VtxWritePtr = VtxBuffer.data() + vtx_buffer_old_size;

            int idx_buffer_old_size = IdxBuffer.size();
            IdxBuffer.resize(idx_buffer_old_size + idx_count);
            _IdxWritePtr = IdxBuffer.data() + idx_buffer_old_size;

        }
        void ImDrawList::AddConvexPolyFilled(const ImVec2* points, const int points_count, ImU32 col) {
            if (points_count < 3 || (col & COL32_A_MASK) == 0)
                return;

            const ImVec2 uv = { 0,0 };//_Data->TexUvWhitePixel;

            if (Flags & ImDrawListFlags_AntiAliasedFill)
            {
                // Anti-aliased Fill
                const float AA_SIZE = 1.0;
                const ImU32 col_trans = col & ~COL32_A_MASK;
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
                std::vector<ImVec2>TempBuffer(points_count);
                //_Data->TempBuffer.reserve_discard(points_count);
                ImVec2* temp_normals = TempBuffer.data();
                for (int i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++)
                {
                    const ImVec2& p0 = points[i0];
                    const ImVec2& p1 = points[i1];
                    float dx = p1.x - p0.x;
                    float dy = p1.y - p0.y;
                    NORMALIZE2F_OVER_ZERO(dx, dy);
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
                    FIXNORMAL2F(dm_x, dm_y);
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
        void ImDrawList::AddConcavePolyFilled(const ImVec2* points, const int points_count, ImU32 col) {
            if (points_count < 3 || (col & COL32_A_MASK) == 0)
                return;

            const ImVec2 uv = { 0,0 };//_Data->TexUvWhitePixel;
            ImTriangulator triangulator;
            unsigned int triangle[3];
            if (Flags & ImDrawListFlags_AntiAliasedFill)
            {
                // Anti-aliased Fill
                const float AA_SIZE = 1.0;//_FringeScale;
                const ImU32 col_trans = col & ~COL32_A_MASK;
                const int idx_count = (points_count - 2) * 3 + points_count * 6;
                const int vtx_count = (points_count * 2);
                PrimReserve(idx_count, vtx_count);

                // Add indexes for fill
                unsigned int vtx_inner_idx = _VtxCurrentIdx;
                unsigned int vtx_outer_idx = _VtxCurrentIdx + 1;
                std::vector<ImVec2>TempBuffer((ImTriangulator::EstimateScratchBufferSize(points_count) + sizeof(ImVec2)) / sizeof(ImVec2));

                triangulator.Init(points, points_count, TempBuffer.data());
                while (triangulator._TrianglesLeft > 0)
                {
                    triangulator.GetNextTriangle(triangle);
                    _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx + (triangle[0] << 1)); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + (triangle[1] << 1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx + (triangle[2] << 1));
                    _IdxWritePtr += 3;
                }

                // Compute normals
                TempBuffer.clear();
                TempBuffer.resize(points_count);
                ImVec2* temp_normals = TempBuffer.data();
                for (int i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++)
                {
                    const ImVec2& p0 = points[i0];
                    const ImVec2& p1 = points[i1];
                    float dx = p1.x - p0.x;
                    float dy = p1.y - p0.y;
                    NORMALIZE2F_OVER_ZERO(dx, dy);
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
                    FIXNORMAL2F(dm_x, dm_y);
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
                std::vector<ImVec2>TempBuffer((ImTriangulator::EstimateScratchBufferSize(points_count) + sizeof(ImVec2)) / sizeof(ImVec2));

                triangulator.Init(points, points_count, TempBuffer.data());
                while (triangulator._TrianglesLeft > 0)
                {
                    triangulator.GetNextTriangle(triangle);
                    _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx + triangle[0]); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx + triangle[1]); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx + triangle[2]);
                    _IdxWritePtr += 3;
                }
                _VtxCurrentIdx += (ImDrawIdx)vtx_count;
            }
        }

        void ImDrawList::AddPolyline(const ImVec2* points, int num_points, ImU32 col, float thickness, ImDrawFlags flags)
        {
            const bool closed = (flags & ImDrawFlags_Closed) != 0;
            const ImVec2 opaque_uv = { 0,0 };
            const int count = closed ? num_points : num_points - 1;
            const int idx_count = count * 6;
            const int vtx_count = count * 4;    // FIXME-OPT: Not sharing edges
            PrimReserve(idx_count, vtx_count);

            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1 + 1) == num_points ? 0 : i1 + 1;
                const ImVec2& p1 = points[i1];
                const ImVec2& p2 = points[i2];

                float dx = p2.x - p1.x;
                float dy = p2.y - p1.y;
                NORMALIZE2F_OVER_ZERO(dx, dy);
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
        void ImDrawList::AddLineH(float min_x, float max_x, float y, ImU32 col, float thickness)
        {
            if ((col & COL32_A_MASK) == 0)
                return;
            const ImVec2 points[2] = { ImVec2(min_x + 0.5f, y + 0.5f), ImVec2(max_x + 0.5f, y + 0.5f) }; // Same as AddLine() above.
            AddPolyline(points, 2, col, thickness);
        }
        void ImDrawList::AddLineV(float x, float min_y, float max_y, ImU32 col, float thickness)
        {
            if ((col & COL32_A_MASK) == 0)
                return;
            const ImVec2 points[2] = { ImVec2(x + 0.5f, min_y + 0.5f), ImVec2(x + 0.5f, max_y + 0.5f) }; // Same as AddLine() above.
            AddPolyline(points, 2, col, thickness);
        }
        void ImDrawList::AddRect(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, float thickness, ImDrawFlags flags)
        {

            if ((col & COL32_A_MASK) == 0)
                return;
            if (Flags & ImDrawListFlags_AntiAliasedLines)
                PathRect(p_min + ImVec2(0.50f, 0.50f), p_max - ImVec2(0.50f, 0.50f), rounding, flags);
            else
                PathRect(p_min + ImVec2(0.50f, 0.50f), p_max - ImVec2(0.49f, 0.49f), rounding, flags); // Better looking lower-right corner and rounded non-AA shapes.
            PathStroke(col, thickness, ImDrawFlags_Closed);
        }
        void ImDrawList::PrimRect(const ImVec2& a, const ImVec2& c, ImU32 col)
        {
            ImVec2 b(c.x, a.y), d(a.x, c.y), uv(0, 0);;//_Data->TexUvWhitePixel
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
        void ImDrawList::AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, ImDrawFlags flags) {
            if ((col & COL32_A_MASK) == 0)
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
        void ImDrawList::AddDrawCmd() {
            ImDrawCmd draw_cmd;
            //draw_cmd.ClipRect = _CmdHeader.ClipRect;    // Same as calling ImDrawCmd_HeaderCopy()
            //draw_cmd.TexRef = _CmdHeader.TexRef;
            draw_cmd.VtxOffset = VtxBuffer.size();//_CmdHeader.VtxOffset;
            draw_cmd.IdxOffset = IdxBuffer.size();

            //IM_ASSERT(draw_cmd.ClipRect.x <= draw_cmd.ClipRect.z && draw_cmd.ClipRect.y <= draw_cmd.ClipRect.w);
            CmdBuffer.push_back(draw_cmd);
        }
        void  ImDrawList::PathArcToFast(const ImVec2& center, float radius, int a_min_of_12, int a_max_of_12) {
            if (radius < 0.5f)
            {
                _Path.push_back(center);
                return;
            }
            _PathArcToFastEx(center, radius, a_min_of_12 * DRAWLIST_ARCFAST_SAMPLE_MAX / 12, a_max_of_12 * DRAWLIST_ARCFAST_SAMPLE_MAX / 12, 0);
        }
        void  ImDrawList::_PathArcToFastEx(const ImVec2& center, float radius, int a_min_sample, int a_max_sample, int a_step) {
            static ImVec2 arcTable[DRAWLIST_ARCFAST_SAMPLE_MAX];
            static bool initArc = false;
            if (!initArc) {
                initArc = true;
                for (int i = 0;i < DRAWLIST_ARCFAST_SAMPLE_MAX;i++) {
                    float angle = -i * 1.0f / DRAWLIST_ARCFAST_SAMPLE_MAX * 2 * 3.141592653589793;
                    arcTable[i] = { cos(angle) ,sin(angle) };
                }
            }

            if (radius < 0.5f)
            {
                _Path.push_back(center);
                return;
            }

            // Calculate arc auto segment step size
            if (a_step <= 0)
                a_step = DRAWLIST_ARCFAST_SAMPLE_MAX / radius;

            // Make sure we never do steps larger than one quarter of the circle
            if (a_step < 1) {
                a_step = 1;
            }
            if (a_step > 12) {
                a_step = 12;
            }

            const int sample_range = abs(a_max_sample - a_min_sample);
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

            _Path.resize(_Path.size() + samples);
            ImVec2* out_ptr = _Path.data() + (_Path.size() - samples);

            int sample_index = a_min_sample;
            if (sample_index < 0 || sample_index >= DRAWLIST_ARCFAST_SAMPLE_MAX)
            {
                sample_index = sample_index % DRAWLIST_ARCFAST_SAMPLE_MAX;
                if (sample_index < 0)
                    sample_index += DRAWLIST_ARCFAST_SAMPLE_MAX;
            }

            if (a_max_sample >= a_min_sample)
            {
                for (int a = a_min_sample; a <= a_max_sample; a += a_step, sample_index += a_step, a_step = a_next_step)
                {
                    // a_step is clamped to IM_DRAWLIST_ARCFAST_SAMPLE_MAX, so we have guaranteed that it will not wrap over range twice or more
                    if (sample_index >= DRAWLIST_ARCFAST_SAMPLE_MAX)
                        sample_index -= DRAWLIST_ARCFAST_SAMPLE_MAX;

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
                        sample_index += DRAWLIST_ARCFAST_SAMPLE_MAX;

                    const ImVec2 s = arcTable[sample_index];
                    out_ptr->x = center.x + s.x * radius;
                    out_ptr->y = center.y + s.y * radius;
                    out_ptr++;
                }
            }

            if (extra_max_sample)
            {
                int normalized_max_sample = a_max_sample % DRAWLIST_ARCFAST_SAMPLE_MAX;
                if (normalized_max_sample < 0)
                    normalized_max_sample += DRAWLIST_ARCFAST_SAMPLE_MAX;

                const ImVec2 s = arcTable[normalized_max_sample];
                out_ptr->x = center.x + s.x * radius;
                out_ptr->y = center.y + s.y * radius;
                out_ptr++;
            }
        }
        void  ImDrawList::PathRect(const ImVec2& a, const ImVec2& b, float rounding, ImDrawFlags flags) {
            if (rounding >= 0.5f)
            {
                if ((flags & ImDrawFlags_RoundCornersMask_) == 0)
                    flags |= ImDrawFlags_RoundCornersAll;

                rounding = std::min(rounding, abs(b.x - a.x) * (((flags & ImDrawFlags_RoundCornersTop) == ImDrawFlags_RoundCornersTop) || ((flags & ImDrawFlags_RoundCornersBottom) == ImDrawFlags_RoundCornersBottom) ? 0.5f : 1.0f) - 1.0f);
                rounding = std::min(rounding, abs(b.y - a.y) * (((flags & ImDrawFlags_RoundCornersLeft) == ImDrawFlags_RoundCornersLeft) || ((flags & ImDrawFlags_RoundCornersRight) == ImDrawFlags_RoundCornersRight) ? 0.5f : 1.0f) - 1.0f);
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
        void  ImDrawList::_ResetForNewFrame() {
            CmdBuffer.resize(0);
            IdxBuffer.resize(0);
            VtxBuffer.resize(0);
            Flags = 0;

            _VtxCurrentIdx = 0;
            _VtxWritePtr = NULL;
            _IdxWritePtr = NULL;
            _Path.resize(0);
            CmdBuffer.push_back(ImDrawCmd());
        }
        Im2DRender::~Im2DRender()
        {
        }
        Im2DRender& Im2DRender::instance()
        {
            static Im2DRender render2D;
            return render2D;
        }
        ImDrawList* Im2DRender::getDrawList()
        {
            return &drawList;
        }
        void Im2DRender::newFrame()
        {
            drawList._ResetForNewFrame();
        }
        void Im2DRender::endFrame()
        {
            for (int i = 0;i < drawList.CmdBuffer.size();i++) {
                auto& cmd = drawList.CmdBuffer[i];
                int offsetIndex=cmd.IdxOffset;
                int offsetVertex = cmd.VtxOffset;
                for (int i = 0;i < cmd.ElemCount;i += 3) {
                    
                    ImDrawVert v0 = drawList.VtxBuffer[drawList.IdxBuffer[offsetIndex + i]];
                    ImDrawVert v1 = drawList.VtxBuffer[drawList.IdxBuffer[offsetIndex + i+1]];
                    ImDrawVert v2 = drawList.VtxBuffer[drawList.IdxBuffer[offsetIndex + i+2]];
                    auto list = ImGui::GetForegroundDrawList();
                    list->AddTriangle({v0.pos.x,v0.pos.y}, { v1.pos.x,v1.pos.y }, { v2.pos.x,v2.pos.y },IM_COL32(255,255,255,255));
                }
            }
           
        }
        Im2DRender::Im2DRender()
        {
        }
    }
}