#pragma once

namespace MOON
{

inline bool g_imGuiEditorMode = false;

/// Global flag telling renderer code whether the ImGui editor is active.
/// Kept in a Qt-free header so renderer files (which include glad) can check
/// it without pulling in QOpenGLWidget headers.
inline bool IsImGuiEditorMode()
{
    return g_imGuiEditorMode;
}

inline void SetImGuiEditorMode(bool enable)
{
    g_imGuiEditorMode = enable;
}

} // namespace MOON
