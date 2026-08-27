#include "ImGuiEditor.h"

#include "ImGuiLogOutput.h"

#include <Core/ECS/Actor.h>
#include <Core/ECS/Components/CTransform.h>
#include <Core/SceneSystem/Scene.h>
#include <Rendering/Settings/EProjectionMode.h>

#include "Qtimgui/imgui/imgui.h"
#include "Qtimgui/imgui/imgui_internal.h"
#include "renderer/SceneView.h"

#include <cstdint>
#include <cstdio>

namespace MOON
{

struct ImGuiEditor::Impl
{
    Impl(Editor::Panels::SceneView& view, ImGuiLogOutput& log)
        : sceneView(view)
        , logOutput(log)
    {
    }

    Editor::Panels::SceneView& sceneView;
    ImGuiLogOutput& logOutput;

    std::function<void()> fileOpenCallback;
    std::function<void()> quitCallback;
    bool fileOpenRequested = false;
    bool quitRequested = false;

    int64_t selectedActorId = -1;
    bool showDemoWindow = false;
    bool viewportHovered = false;
    bool viewportFocused = false;
    float viewportWidth = 0.0f;
    float viewportHeight = 0.0f;

    struct Rect
    {
        ImVec2 pos;
        ImVec2 size;
    };
    Rect hierarchyRect;
    Rect viewportRect;
    Rect propertyRect;
    Rect logRect;
};

ImGuiEditor::ImGuiEditor(Editor::Panels::SceneView& sceneView, ImGuiLogOutput& logOutput)
    : mImpl(new Impl(sceneView, logOutput))
{
}

ImGuiEditor::~ImGuiEditor()
{
    delete mImpl;
}

void ImGuiEditor::SetFileOpenCallback(std::function<void()> callback)
{
    mImpl->fileOpenCallback = std::move(callback);
}

void ImGuiEditor::SetQuitCallback(std::function<void()> callback)
{
    mImpl->quitCallback = std::move(callback);
}

bool ImGuiEditor::IsViewportHovered() const
{
    return mImpl->viewportHovered;
}

bool ImGuiEditor::IsViewportFocused() const
{
    return mImpl->viewportFocused;
}

bool ImGuiEditor::IsImGuiInteracting() const
{
    const ImGuiContext* context = ImGui::GetCurrentContext();
    if (context == nullptr) {
        return false;
    }
    if (context->ActiveId != 0) {
        return true;
    }
    return context->OpenPopupStack.Size > 0;
}

bool ImGuiEditor::WantsCaptureKeyboard() const
{
    return ImGui::GetIO().WantCaptureKeyboard;
}

void ImGuiEditor::GetViewportOrigin(float& x, float& y) const
{
    x = mImpl->viewportRect.pos.x;
    y = mImpl->viewportRect.pos.y;
}

void ImGuiEditor::Draw()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 displaySize = viewport->Size;
    const float menuHeight = ImGui::GetFrameHeightWithSpacing();

    if (ImGui::BeginMainMenuBar()) {
        DrawMainMenuBar();
        ImGui::EndMainMenuBar();
    }

    // Fixed manual layout (this ImGui build has no docking support and no
    // ImVec2 math operators).
    const ImVec2 workPos(viewport->Pos.x, viewport->Pos.y + menuHeight);
    const ImVec2 workSize(displaySize.x, displaySize.y - menuHeight);
    const float leftWidth = workSize.x * 0.20f;
    const float rightWidth = workSize.x * 0.24f;
    const float bottomHeight = workSize.y * 0.26f;
    const float sideHeight = workSize.y - bottomHeight;
    const float centerWidth = workSize.x - leftWidth - rightWidth;

    mImpl->hierarchyRect = { ImVec2(workPos.x, workPos.y), ImVec2(leftWidth, sideHeight) };
    mImpl->viewportRect
        = { ImVec2(workPos.x + leftWidth, workPos.y), ImVec2(centerWidth, sideHeight) };
    mImpl->propertyRect
        = { ImVec2(workPos.x + leftWidth + centerWidth, workPos.y), ImVec2(rightWidth, sideHeight) };
    mImpl->logRect = { ImVec2(workPos.x, workPos.y + sideHeight), ImVec2(workSize.x, bottomHeight) };

    DrawViewportPanel();
    DrawHierarchyPanel();
    DrawPropertyPanel();
    DrawLogPanel();

    if (mImpl->showDemoWindow) {
        ImGui::ShowDemoWindow(&mImpl->showDemoWindow);
    }
}

void ImGuiEditor::DrawMainMenuBar()
{
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open...", "Ctrl+O")) {
            mImpl->fileOpenRequested = true;
        }
        ImGui::MenuItem("Export...", nullptr, false, false);
        ImGui::Separator();
        if (ImGui::MenuItem("Quit", "Alt+F4")) {
            mImpl->quitRequested = true;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Display")) {
        if (ImGui::MenuItem("Perspective")) {
            mImpl->sceneView.setCameraMode(Rendering::Settings::EProjectionMode::PERSPECTIVE);
        }
        if (ImGui::MenuItem("Orthographic")) {
            mImpl->sceneView.setCameraMode(Rendering::Settings::EProjectionMode::ORTHOGRAPHIC);
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Reset Camera")) {
            mImpl->sceneView.ResetCameraTransform();
        }
        ImGui::Separator();
        ImGui::MenuItem("ImGui Demo", nullptr, &mImpl->showDemoWindow);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Sketch")) {
        ImGui::MenuItem("New Sketch", nullptr, false, false);
        ImGui::EndMenu();
    }
}

void ImGuiEditor::HandlePendingActions()
{
    if (mImpl->fileOpenRequested) {
        mImpl->fileOpenRequested = false;
        if (mImpl->fileOpenCallback) {
            mImpl->fileOpenCallback();
        }
    }
    if (mImpl->quitRequested) {
        mImpl->quitRequested = false;
        if (mImpl->quitCallback) {
            mImpl->quitCallback();
        }
    }
}

void ImGuiEditor::DrawViewportPanel()
{
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::SetNextWindowPos(mImpl->viewportRect.pos);
    ImGui::SetNextWindowSize(mImpl->viewportRect.size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Viewport", nullptr, flags);

    mImpl->viewportHovered = ImGui::IsWindowHovered(
        ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup
    );
    mImpl->viewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

    const ImVec2 contentSize = ImGui::GetContentRegionAvail();
    if (contentSize.x != mImpl->viewportWidth || contentSize.y != mImpl->viewportHeight) {
        mImpl->viewportWidth = contentSize.x;
        mImpl->viewportHeight = contentSize.y;
        if (contentSize.x > 1.0f && contentSize.y > 1.0f) {
            mImpl->sceneView.Resize(
                static_cast<int>(contentSize.x),
                static_cast<int>(contentSize.y)
            );
        }
    }
    const uint32_t textureId = mImpl->sceneView.GetRenderedTextureID();
    if (textureId != 0 && contentSize.x > 1.0f && contentSize.y > 1.0f) {
        // Renderer textures have their origin at the bottom-left, flip vertically.
        ImGui::Image(
            static_cast<ImTextureID>(textureId),
            contentSize,
            ImVec2(0.0f, 1.0f),
            ImVec2(1.0f, 0.0f)
        );
        if (ImGui::IsItemHovered()) {
            mImpl->viewportHovered = true;
        }
    }
    else {
        ImGui::Text("No render target");
    }

    const ImGuiIO& io = ImGui::GetIO();
    Core::SceneSystem::Scene* scene = mImpl->sceneView.GetScene();
    ImGui::TextColored(
        ImVec4(0.65f, 0.65f, 0.65f, 1.0f),
        "FPS %.1f | Actors %zu",
        io.Framerate,
        scene ? scene->GetActors().size() : 0
    );

    ImGui::End();
    ImGui::PopStyleVar();
}

void ImGuiEditor::DrawHierarchyPanel()
{
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::SetNextWindowPos(mImpl->hierarchyRect.pos);
    ImGui::SetNextWindowSize(mImpl->hierarchyRect.size);
    ImGui::Begin("Hierarchy", nullptr, flags);

    Core::SceneSystem::Scene* scene = mImpl->sceneView.GetScene();
    if (scene) {
        for (Core::ECS::Actor* actor : scene->GetActors()) {
            if (actor && !actor->HasParent()) {
                DrawActorNode(*actor);
            }
        }
    }

    ImGui::End();
}

void ImGuiEditor::DrawActorNode(Core::ECS::Actor& actor)
{
    const bool isLeaf = actor.GetChildren().empty();
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
        | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (mImpl->selectedActorId == actor.GetID()) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (isLeaf) {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    const bool opened = ImGui::TreeNodeEx(actor.GetName().c_str(), flags);
    if (ImGui::IsItemClicked()) {
        mImpl->selectedActorId = actor.GetID();
        mImpl->sceneView.SelectActor(actor);
    }

    // Leaf nodes use NoTreePushOnOpen and never push an ID scope, so they
    // must not be paired with TreePop().
    if (opened && !isLeaf) {
        for (Core::ECS::Actor* child : actor.GetChildren()) {
            if (child) {
                DrawActorNode(*child);
            }
        }
        ImGui::TreePop();
    }
}

void ImGuiEditor::DrawPropertyPanel()
{
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::SetNextWindowPos(mImpl->propertyRect.pos);
    ImGui::SetNextWindowSize(mImpl->propertyRect.size);
    ImGui::Begin("Property", nullptr, flags);

    Core::SceneSystem::Scene* scene = mImpl->sceneView.GetScene();
    Core::ECS::Actor* actor = scene ? scene->FindActorByID(mImpl->selectedActorId) : nullptr;
    if (!actor) {
        mImpl->selectedActorId = -1;
        ImGui::TextDisabled("No actor selected");
        ImGui::End();
        return;
    }

    ImGui::Text("Name: %s", actor->GetName().c_str());
    ImGui::Text("Tag:  %s", actor->GetTag().c_str());
    ImGui::Text("ID:   %lld", static_cast<long long>(actor->GetID()));
    ImGui::Separator();

    if (auto* transform = actor->GetComponent<Core::ECS::Components::CTransform>()) {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            Maths::FVector3 position = transform->GetLocalPosition();
            if (ImGui::DragFloat3("Position", &position.x, 0.05f)) {
                transform->SetLocalPosition(position);
            }
            Maths::FVector3 scale = transform->GetLocalScale();
            if (ImGui::DragFloat3("Scale", &scale.x, 0.01f)) {
                transform->SetLocalScale(scale);
            }
        }
    }

    if (ImGui::CollapsingHeader("Components")) {
        for (const auto& component : actor->GetComponents()) {
            if (component) {
                ImGui::BulletText("%s", component->GetName().c_str());
            }
        }
    }

    ImGui::End();
}

void ImGuiEditor::DrawLogPanel()
{
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::SetNextWindowPos(mImpl->logRect.pos);
    ImGui::SetNextWindowSize(mImpl->logRect.size);
    ImGui::Begin("Log", nullptr, flags);

    const std::vector<ImGuiLogOutput::Entry> entries = mImpl->logOutput.GetEntries();
    for (const auto& entry : entries) {
        ImVec4 color(0.8f, 0.8f, 0.8f, 1.0f);
        switch (entry.level) {
            case MOON::LogOutput::LL_ERROR:
            case MOON::LogOutput::LL_FATAL:
                color = ImVec4(1.0f, 0.35f, 0.35f, 1.0f);
                break;
            case MOON::LogOutput::LL_WARNING:
                color = ImVec4(1.0f, 0.85f, 0.3f, 1.0f);
                break;
            case MOON::LogOutput::LL_DEBUG:
                color = ImVec4(0.55f, 0.55f, 0.55f, 1.0f);
                break;
            default:
                color = ImVec4(0.8f, 0.9f, 0.8f, 1.0f);
                break;
        }
        ImGui::TextColored(color, "%s", entry.message.c_str());
    }

    ImGui::End();
}

} // namespace MOON
