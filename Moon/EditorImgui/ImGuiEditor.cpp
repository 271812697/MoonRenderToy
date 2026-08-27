#include "ImGuiEditor.h"

#include "ImGuiLogOutput.h"

#include <Core/ECS/Actor.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/ECS/Components/CTransform.h>
#include <Core/Rendering/EngineBufferRenderFeature.h>
#include <Core/Rendering/SceneRenderer.h>
#include <Core/SceneSystem/BvhService.h>
#include <Core/SceneSystem/Scene.h>
#include <Rendering/Settings/EProjectionMode.h>
#include <Settings/DebugSetting.h>
#include <core/log.h>

#include "Qtimgui/imgui/imgui.h"
#include "Qtimgui/imgui/imgui_internal.h"
#include "renderer/GizmoRenderPass.h"
#include "renderer/PointRenderPass.h"
#include "renderer/SceneView.h"
#include "core/SelectionManager.h"

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
    ImVec2 viewportImageMin { 0.0f, 0.0f };

    bool wireframe = false;
    bool points = false;
    bool measure = false;
    bool clipPlane = false;
    bool primitiveBox = false;
    bool primitiveCone = false;
    bool primitiveCylinder = false;
    bool primitiveSphere = false;

    struct Rect
    {
        ImVec2 pos;
        ImVec2 size;
    };
    Rect hierarchyRect;
    Rect viewportRect;
    Rect propertyRect;
    Rect settingsRect;
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
    x = mImpl->viewportImageMin.x;
    y = mImpl->viewportImageMin.y;
}

void ImGuiEditor::Draw()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 displaySize = viewport->Size;
    const float menuHeight = ImGui::GetFrameHeightWithSpacing();
    const float toolbarHeight = ImGui::GetFrameHeightWithSpacing() * 2.0f;

    if (ImGui::BeginMainMenuBar()) {
        DrawMainMenuBar();
        ImGui::EndMainMenuBar();
    }

    DrawToolbar();

    // Fixed manual layout (this ImGui build has no docking support and no
    // ImVec2 math operators).
    const ImVec2 workPos(viewport->Pos.x, viewport->Pos.y + menuHeight + toolbarHeight);
    const ImVec2 workSize(displaySize.x, displaySize.y - menuHeight - toolbarHeight);
    const float leftWidth = workSize.x * 0.20f;
    const float rightWidth = workSize.x * 0.24f;
    const float bottomHeight = workSize.y * 0.26f;
    const float sideHeight = workSize.y - bottomHeight;
    const float centerWidth = workSize.x - leftWidth - rightWidth;

    mImpl->hierarchyRect = { ImVec2(workPos.x, workPos.y), ImVec2(leftWidth, sideHeight) };
    mImpl->viewportRect
        = { ImVec2(workPos.x + leftWidth, workPos.y), ImVec2(centerWidth, sideHeight) };
    mImpl->propertyRect = {
        ImVec2(workPos.x + leftWidth + centerWidth, workPos.y),
        ImVec2(rightWidth, sideHeight * 0.55f)
    };
    mImpl->settingsRect = {
        ImVec2(workPos.x + leftWidth + centerWidth, workPos.y + sideHeight * 0.55f),
        ImVec2(rightWidth, sideHeight * 0.45f)
    };
    mImpl->logRect = { ImVec2(workPos.x, workPos.y + sideHeight), ImVec2(workSize.x, bottomHeight) };

    // Hover: preselect highlight through SelectionManager (CTopoShape
    // domain-color face/edge highlight).
    SyncHoverSelection();

    // Left click in the viewport: commit the picked actor to the
    // SelectionManager, replicating the old RotateCenter flow.
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && mImpl->viewportHovered) {
        SyncClickSelection();
    }

    // Sync the hierarchy selection with the view (viewport picking / gizmo).
    if (mImpl->sceneView.IsSelectActor()) {
        if (Core::ECS::Actor* actor = mImpl->sceneView.GetSelectedActor()) {
            mImpl->selectedActorId = actor->GetID();
        }
    }
    else {
        mImpl->selectedActorId = -1;
    }

    DrawViewportPanel();
    DrawHierarchyPanel();
    DrawPropertyPanel();
    DrawSettingsPanel();
    DrawLogPanel();

    if (mImpl->showDemoWindow) {
        ImGui::ShowDemoWindow(&mImpl->showDemoWindow);
    }
}

void ImGuiEditor::SyncHoverSelection()
{
    const auto pick = mImpl->sceneView.GetPickResult();
    if (pick.has_value()) {
        if (const auto* pval
            = std::get_if<Tools::Utils::OptRef<::Core::ECS::Actor>>(&pick.value())) {
            const auto actor = *pval;
            if (actor) {
                MOON::SelectionManager::instance().setPreselect({ actor.value().GetID() });
                return;
            }
        }
    }
    MOON::SelectionManager::instance().clearPreselect();
}

void ImGuiEditor::SyncClickSelection()
{
    const auto pick = mImpl->sceneView.GetPickResult();
    if (pick.has_value()) {
        if (const auto* pval
            = std::get_if<Tools::Utils::OptRef<::Core::ECS::Actor>>(&pick.value())) {
            const auto actor = *pval;
            if (actor) {
                MOON::SelectionManager::instance().select({ actor.value().GetID() });
                return;
            }
        }
    }
    MOON::SelectionManager::instance().select({});
}

void ImGuiEditor::DrawToolbar()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float menuHeight = ImGui::GetFrameHeightWithSpacing();
    const float toolbarHeight = ImGui::GetFrameHeightWithSpacing() * 2.0f;

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, toolbarHeight));
    ImGui::Begin(
        "##Toolbar",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoBringToFrontOnFocus
    );

    Editor::Panels::SceneView& view = mImpl->sceneView;
    auto fit = [&view](const Maths::FVector3& dir) {
        view.FitToSelectedActor(dir);
    };

    // Camera alignment (mirrors ViewerWindowTitleBar / CameraFitCommand).
    if (ImGui::Button("X-")) { fit({ -1, 0, 0 }); }
    ImGui::SameLine();
    if (ImGui::Button("X+")) { fit({ 1, 0, 0 }); }
    ImGui::SameLine();
    if (ImGui::Button("Y-")) { fit({ 0, -1, 0 }); }
    ImGui::SameLine();
    if (ImGui::Button("Y+")) { fit({ 0, 1, 0 }); }
    ImGui::SameLine();
    if (ImGui::Button("Z-")) { fit({ 0, 0, -1 }); }
    ImGui::SameLine();
    if (ImGui::Button("Z+")) { fit({ 0, 0, 1 }); }
    ImGui::SameLine();
    if (ImGui::Button("Iso")) {
        fit(Maths::FVector3::Normalize(Maths::FVector3(1, 1, 1)));
    }
    ImGui::SameLine();
    if (ImGui::Button("Fit")) {
        fit(view.GetCamera()->GetTransform().GetWorldForward());
    }
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(12.0f, 0.0f));
    ImGui::SameLine();

    // View toggles (Wire/Points/Measure/Clip from ViewerWindowTitleBar).
    if (ImGui::Checkbox("Wire", &mImpl->wireframe)) {
        if (view.IsSelectActor()) {
            if (auto* matList
                = view.GetSelectedActor()->GetComponent<Core::ECS::Components::CMaterialRenderer>()) {
                auto material = matList->GetMaterialAtIndex(0);
                if (material && material->SupportsFeature("WITH_EDGE")) {
                    material->EnableFeature("WITH_EDGE", mImpl->wireframe);
                }
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Points", &mImpl->points)) {
        view.GetRenderer().GetPass<Editor::Rendering::PointRenderPass>("PointDraw")
            .SetEnabled(mImpl->points);
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Measure", &mImpl->measure)) {
        view.GetRenderer().GetPass<Editor::Rendering::GizmoRenderPass>("ImRenderer")
            .enableGizmoWidget("Measure", mImpl->measure);
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Clip", &mImpl->clipPlane)) {
        view.GetRenderer().GetPass<Editor::Rendering::GizmoRenderPass>("ImRenderer")
            .enableGizmoWidget("ClipPlane", mImpl->clipPlane);
        view.GetRenderer().GetFeature<::Core::Rendering::EngineBufferRenderFeature>()
            .EnableClip(mImpl->clipPlane);
    }
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(12.0f, 0.0f));
    ImGui::SameLine();

    // Primitive creation tools (mirrors PrimitiveToolbar).
    auto primitiveToggle = [&view](const char* label, bool& value, const std::string& widgetName) {
        if (ImGui::Checkbox(label, &value)) {
            view.GetRenderer().GetPass<Editor::Rendering::GizmoRenderPass>("ImRenderer")
                .enableGizmoWidget(widgetName, value);
        }
        ImGui::SameLine();
    };
    primitiveToggle("Box", mImpl->primitiveBox, "PrimitiveBox");
    primitiveToggle("Cone", mImpl->primitiveCone, "PrimitiveCone");
    primitiveToggle("Cylinder", mImpl->primitiveCylinder, "PrimitiveCylinder");
    primitiveToggle("Sphere", mImpl->primitiveSphere, "PrimitiveSphere");
    ImGui::Dummy(ImVec2(12.0f, 0.0f));
    ImGui::SameLine();

    // Design modeling tasks (mirrors DesignModelingToolbar). The Qt task
    // dialogs are not available in the ImGui editor yet.
    const char* designTasks[] = { "Pad", "Revolve", "Thickness", "Fillet", "Pocket", "Groove" };
    for (const char* task : designTasks) {
        if (ImGui::Button(task)) {
            LOG_INFO("Design modeling task '%s' is not available in the ImGui editor yet.", task);
        }
        ImGui::SameLine();
    }

    ImGui::End();
}

void ImGuiEditor::DrawSettingsPanel()
{
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::SetNextWindowPos(mImpl->settingsRect.pos);
    ImGui::SetNextWindowSize(mImpl->settingsRect.size);
    ImGui::Begin("Settings", nullptr, flags);

    // ---- Render pass enable/disable toggles (mirrors PassSettingWidget) ----
    if (ImGui::CollapsingHeader("Passes", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& passes = mImpl->sceneView.GetRenderer().GetPasses();
        for (auto& [order, entry] : passes) {
            auto& name = entry.first;
            auto& pass = entry.second;
            bool enabled = pass->IsEnabled();
            if (ImGui::Checkbox(name.c_str(), &enabled)) {
                pass->SetEnabled(enabled);
            }
        }
    }

    // ---- Debug settings tree (mirrors DebugSettingWidget) ----
    if (ImGui::CollapsingHeader("Debug Settings")) {
        auto& groups = MOON::DebugSettings::instance().getGroup();
        auto& registry = MOON::DebugSettings::instance().getRegistry();
        for (auto& [group, indices] : groups) {
            if (ImGui::CollapsingHeader(group.c_str())) {
                for (int index : indices) {
                    MOON::NodeBase* node = registry[index];
                    if (!node) {
                        continue;
                    }
                    const std::string type = node->getType();
                    const std::string& label = node->getName();
                    if (type == "bool") {
                        bool value = node->getData<bool>();
                        if (ImGui::Checkbox(label.c_str(), &value)) {
                            node->setData<bool>(value);
                        }
                    }
                    else if (type == "float") {
                        float value = node->getData<float>();
                        if (ImGui::DragFloat(label.c_str(), &value, 0.01f)) {
                            node->setData<float>(value);
                        }
                    }
                    else if (type == "int") {
                        int value = node->getData<int>();
                        if (ImGui::DragInt(label.c_str(), &value, 0.1f)) {
                            node->setData<int>(value);
                        }
                    }
                }
            }
        }
    }

    // ---- PathTrace material editor (mirrors RenderSettingWidget) ----
    if (Core::SceneSystem::Scene* scene = mImpl->sceneView.GetScene()) {
        if (auto* bvhService = scene->GetBvhService()) {
            if (ImGui::CollapsingHeader("PathTrace Materials")) {
                for (size_t i = 0; i < bvhService->materials.size(); ++i) {
                    auto& material = bvhService->materials[i];
                    const std::string label = "Material " + std::to_string(i);
                    if (ImGui::CollapsingHeader(label.c_str())) {
                        ImGui::ColorEdit3("BaseColor", &material.baseColor.x);
                        ImGui::ColorEdit3("Emission", &material.emission.x);
                        ImGui::SliderFloat("Opacity", &material.opacity, 0.0f, 1.0f);
                        ImGui::SliderFloat("Roughness", &material.roughness, 0.0f, 1.0f);
                        ImGui::SliderFloat("Metallic", &material.metallic, 0.0f, 1.0f);
                        ImGui::SliderFloat("Anisotropic", &material.anisotropic, -1.0f, 1.0f);
                        ImGui::SliderFloat("Clearcoat", &material.clearcoat, 0.0f, 1.0f);
                        ImGui::SliderFloat("ClearcoatGloss", &material.clearcoatGloss, 0.0f, 1.0f);
                        ImGui::DragFloat("Ior", &material.ior, 0.01f, 1.0f, 3.0f);
                        ImGui::SliderFloat("MediumDensity", &material.mediumDensity, 0.0f, 10.0f);
                        ImGui::SliderFloat("MediumAnisotropy", &material.mediumAnisotropy, -1.0f, 1.0f);
                        ImGui::ColorEdit3("MediumColor", &material.mediumColor.x);
                        ImGui::SliderFloat("AlphaCutOff", &material.alphaCutoff, 0.0f, 1.0f);
                    }
                }
            }
        }
    }

    ImGui::End();
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
        // Exact screen-space origin of the rendered image. Used to translate
        // window mouse events into viewport-local coordinates so picking and
        // camera deltas match the render target size.
        mImpl->viewportImageMin = ImGui::GetItemRectMin();
        if (ImGui::IsItemHovered()) {
            mImpl->viewportHovered = true;
        }
    }
    else {
        ImGui::Text("No render target");
        const ImVec2 windowPos = ImGui::GetWindowPos();
        const ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
        mImpl->viewportImageMin = ImVec2(windowPos.x + contentMin.x, windowPos.y + contentMin.y);
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

    // Read-only multiline: makes the log text selectable/copyable.
    std::string buffer;
    const std::vector<ImGuiLogOutput::Entry> entries = mImpl->logOutput.GetEntries();
    for (const auto& entry : entries) {
        buffer += entry.message;
        buffer += '\n';
    }
    ImGui::InputTextMultiline(
        "##LogText",
        buffer.data(),
        buffer.size() + 1,
        ImVec2(-FLT_MIN, -FLT_MIN),
        ImGuiInputTextFlags_ReadOnly
    );

    ImGui::End();
}

} // namespace MOON
