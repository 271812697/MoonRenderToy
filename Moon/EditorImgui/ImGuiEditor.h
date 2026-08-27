#pragma once

#include <cstdint>
#include <functional>

namespace Core::ECS
{
class Actor;
}
namespace Editor::Panels
{
class SceneView;
}
namespace MOON
{
class ImGuiLogOutput;

/** ImGui based editor shell.
 *
 * Replaces the Qt Widgets editor UI (QMainWindow + docks + menus + toolbars)
 * with a Dear ImGui docking layout. It is intentionally free of Qt widget
 * dependencies: the host only needs to provide a SceneView and call Draw()
 * inside an active ImGui frame.
 */
class ImGuiEditor
{
public:
    ImGuiEditor(Editor::Panels::SceneView& sceneView, ImGuiLogOutput& logOutput);
    ~ImGuiEditor();

    /// Draw the whole editor frame. Must be called between ImGui::NewFrame and ImGui::Render.
    void Draw();

    /// True when the mouse is over the viewport panel (used for input routing).
    bool IsViewportHovered() const;

    /// True when the viewport panel has keyboard focus (used for input routing).
    bool IsViewportFocused() const;

    /// True while ImGui is interacting with a widget or has an open popup/menu.
    bool IsImGuiInteracting() const;

    /// True when ImGui wants to capture keyboard input.
    bool WantsCaptureKeyboard() const;

    /// Returns the top-left corner of the viewport panel in window coordinates.
    void GetViewportOrigin(float& x, float& y) const;

    /// Callback invoked from the File menu (host shows a file dialog and loads the file).
    void SetFileOpenCallback(std::function<void()> callback);

    /// Callback invoked from the File menu (host quits the application).
    void SetQuitCallback(std::function<void()> callback);

    /// Process deferred menu actions (file open / quit). Must be called AFTER
    /// ImGui::Render(), outside of the ImGui frame, because opening a modal
    /// file dialog inside a frame runs a nested event loop that corrupts the
    /// frame state (re-entrant NewFrame()).
    void HandlePendingActions();

private:
    void DrawMainMenuBar();
    void DrawViewportPanel();
    void DrawHierarchyPanel();
    void DrawPropertyPanel();
    void DrawLogPanel();
    void DrawActorNode(Core::ECS::Actor& actor);

    struct Impl;
    Impl* mImpl = nullptr;
};

} // namespace MOON
