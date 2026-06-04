#include "AViewControllable.h"
#include "DebugSceneRenderer.h"
#include "GridRenderPass.h"

const Maths::FVector3 kDefaultGridColor{ 1.0f, 1.0f, 1.0f };
const Maths::FVector3 kDefaultClearColor{ 0.0f, 0.0f, 0.0f };
const Maths::FVector3 kDefaultCameraPosition{ -10.0f, 3.0f, 10.0f };
const Maths::FQuaternion kDefaultCameraRotation({ 0.0f, 135.0f, 0.0f });
struct FaceInfo { Maths::FVector3 Normal; Maths::FVector3 Point; float Score; };
FaceInfo ComputeMirrorPlane(const Maths::FVector3& CameraPos, const Maths::FVector3& CameraForward, const Maths::FVector3& BBoxMin, const Maths::FVector3& BBoxMax)
{
    // 包围盒中心和范围
    Maths::FVector3 Center = (BBoxMin + BBoxMax) * 0.5f;
    Maths::FVector3 Extent = (BBoxMax - BBoxMin) * 0.5f;

    // 6个面的法线和分数
    FaceInfo Faces[6];

    // XMin
    Faces[0] = { Maths::FVector3(-1,0,0), Maths::FVector3(BBoxMin.x, Center.y, Center.z), 0 };
    // XMax
    Faces[1] = { Maths::FVector3(1,0,0), Maths::FVector3(BBoxMax.x, Center.y, Center.z), 0 };
    // YMin（地面）
    Faces[2] = { Maths::FVector3(0,-1,0), Maths::FVector3(Center.x, BBoxMin.y, Center.z), 0 };
    // YMax（天花板）
    Faces[3] = { Maths::FVector3(0,1,0), Maths::FVector3(Center.x, BBoxMax.y, Center.z), 0 };
    // ZMin
    Faces[4] = { Maths::FVector3(0,0,-1), Maths::FVector3(Center.x, Center.y, BBoxMin.z), 0 };
    // ZMax
    Faces[5] = { Maths::FVector3(0,0,1), Maths::FVector3(Center.x, Center.y, BBoxMax.z), 0 };

    // 计算每个面的得分
    for (int i = 0; i < 6; i++)
    {
        // 法线朝向与相机方向的点积（正对相机得分高）
        float FacingScore = Maths::FVector3::Dot(Faces[i].Normal, CameraForward);

        // 相机到平面的距离（太远扣分）
        float Dist = Maths::FVector3::Dot(Faces[i].Normal, CameraPos - Faces[i].Point);
        float DistScore = Dist > 0 ? 1.0f / (Dist + 0.5f) : -1.0f;

        // 面积得分（大面优先）
        float AreaScore = 0.1f;
        if (i == 0 || i == 1) AreaScore = Extent.y * Extent.z;
        if (i == 2 || i == 3) AreaScore = Extent.x * Extent.z;
        if (i == 4 || i == 5) AreaScore = Extent.x * Extent.y;
        float tt = AreaScore / 10000.0f;
        if (tt < 0.1f) {
            tt = 0.1f;
        }
        if (tt > 1.0f) {
            tt = 1.0f;
        }
        AreaScore = tt;

        // 综合得分
        Faces[i].Score = FacingScore * 0.6f + DistScore * 0.2f + AreaScore * 0.2f;
    }

    // 选出最佳面
    int BestIdx = 0;
    for (int i = 1; i < 6; i++)
    {
        if (Faces[i].Score > Faces[BestIdx].Score)
            BestIdx = i;
    }

    // 返回平面方程
    float d = -Maths::FVector3::Dot(Faces[BestIdx].Normal, Faces[BestIdx].Point);
    return Faces[BestIdx];//Maths::FVector4(Normal, d);
}
Editor::Panels::AViewControllable::AViewControllable(
	const std::string& p_title) :
	AView(p_title),
	m_cameraController(*this, m_camera)
{
	ResetCameraTransform();
	ResetGridColor();
	ResetClearColor();
}

void Editor::Panels::AViewControllable::Update(float p_deltaTime)
{
	m_cameraController.HandleInputs(p_deltaTime);
	AView::Update(p_deltaTime);
}

void Editor::Panels::AViewControllable::InitFrame()
{
   

	m_camera.SetFrustumGeometryCulling(false);
	m_camera.SetFrustumLightCulling(false);
	AView::InitFrame();

    FaceInfo FaceInfo = {Maths::FVector3(0,1,0),Maths::FVector3(0,0,0),0};
    Maths::FVector3 cameraPos = m_camera.GetPosition();
    auto box= GetScene()->GetSceneBoundingBox();
    if (box.isValid()) {
        Maths::FVector3 cameraForward=m_camera.GetTransform().GetWorldForward();
        FaceInfo =ComputeMirrorPlane(cameraPos, cameraForward, box.pmin, box.pmax);
    }

  
	m_renderer->AddDescriptor<Rendering::GridRenderPass::GridDescriptor>({
		m_gridColor,
        cameraPos,
        FaceInfo.Normal,
        FaceInfo.Point
		});
}

void Editor::Panels::AViewControllable::ResetCameraTransform()
{
	m_camera.transform->SetWorldPosition(kDefaultCameraPosition);
	m_camera.transform->SetWorldRotation(kDefaultCameraRotation);
}

Editor::Core::CameraController& Editor::Panels::AViewControllable::GetCameraController()
{
	return m_cameraController;
}

Rendering::Entities::Camera* Editor::Panels::AViewControllable::GetCamera()
{
	return &m_camera;
}

const Maths::FVector3& Editor::Panels::AViewControllable::GetGridColor() const
{
	return m_gridColor;
}

void Editor::Panels::AViewControllable::SetGridColor(const Maths::FVector3& p_color)
{
	m_gridColor = p_color;
}

void Editor::Panels::AViewControllable::ResetGridColor()
{
	m_gridColor = kDefaultGridColor;
}

void Editor::Panels::AViewControllable::ResetClearColor()
{
	m_camera.SetClearColor(kDefaultClearColor);
}

void Editor::Panels::AViewControllable::setCameraMode(::Rendering::Settings::EProjectionMode mode)
{
	m_camera.SetProjectionMode(mode);
}
