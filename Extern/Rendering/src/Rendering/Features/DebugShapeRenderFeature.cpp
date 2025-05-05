/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include "Rendering/Features/DebugShapeRenderFeature.h"
#include "Rendering/Core/CompositeRenderer.h"
#include "Rendering/Resources/Loaders/ShaderLoader.h"
#include "Rendering/Utils/Conversions.h"

Rendering::Features::DebugShapeRenderFeature::DebugShapeRenderFeature(Core::CompositeRenderer& p_renderer)
	: ARenderFeature(p_renderer)
{
	constexpr auto kVertices = std::to_array<Geometry::Vertex>({
		{
			0, 0, 0,
			0, 0,
			0, 0, 0,
			0, 0, 0,
			0, 0, 0
		},
		{
			0, 0, 0,
			0, 0,
			0, 0, 0,
			0, 0, 0,
			0, 0, 0
		}
	});

	constexpr auto kIndices = std::to_array<uint32_t>({ 0, 1 });

	m_lineMesh = std::make_unique<Resources::Mesh>(
		kVertices,
		kIndices
	);

	// TODO: Move these out of here, maybe we could have proper source files for these.
	std::string vertexShader = R"(
#version 450 core

uniform vec3 start;
uniform vec3 end;
uniform mat4 viewProjection;

void main()
{
	vec3 position = gl_VertexID == 0 ? start : end;
	gl_Position = viewProjection * vec4(position, 1.0);
}

)";

	std::string fragmentShader = R"(
#version 450 core

uniform vec3 color;

out vec4 FRAGMENT_COLOR;

void main()
{
	FRAGMENT_COLOR = vec4(color, 1.0);
}
)";

	m_lineShader = Rendering::Resources::Loaders::ShaderLoader::CreateFromSource(vertexShader, fragmentShader);
	m_lineMaterial = std::make_unique<Rendering::Data::Material>(m_lineShader);
}

Rendering::Features::DebugShapeRenderFeature::~DebugShapeRenderFeature()
{
	Rendering::Resources::Loaders::ShaderLoader::Destroy(m_lineShader);
}

void Rendering::Features::DebugShapeRenderFeature::OnBeginFrame(const Data::FrameDescriptor& p_frameDescriptor)
{
	const Maths::FMatrix4 viewProjection =
		p_frameDescriptor.camera->GetProjectionMatrix() *
		p_frameDescriptor.camera->GetViewMatrix();

	m_lineMaterial->SetProperty("viewProjection", viewProjection);
}

void Rendering::Features::DebugShapeRenderFeature::DrawLine(
	Rendering::Data::PipelineState p_pso,
	const Maths::FVector3& p_start,
	const Maths::FVector3& p_end,
	const Maths::FVector3& p_color,
	float p_lineWidth
)
{
	m_lineMaterial->SetBackfaceCulling(false);
	m_lineMaterial->SetFrontfaceCulling(false);
	m_lineMaterial->SetProperty("start", p_start);
	m_lineMaterial->SetProperty("end", p_end);
	m_lineMaterial->SetProperty("color", p_color);


	p_pso.rasterizationMode = Settings::ERasterizationMode::LINE;
	p_pso.lineWidthPow2 = Utils::Conversions::FloatToPow2(p_lineWidth);

	Rendering::Entities::Drawable drawable;
	drawable.material = *m_lineMaterial;
	drawable.mesh = m_lineMesh.get();
	drawable.stateMask = m_lineMaterial->GenerateStateMask();
	drawable.primitiveMode = Settings::EPrimitiveMode::LINES;

	m_renderer.DrawEntity(p_pso, drawable);

	m_lineShader->GetProgram().Unbind();
}

void Rendering::Features::DebugShapeRenderFeature::DrawBox(
	Rendering::Data::PipelineState p_pso,
	const Maths::FVector3& p_position,
	const Maths::FQuaternion& p_rotation,
	const Maths::FVector3& p_size,
	const Maths::FVector3& p_color,
	float p_lineWidth
)
{
	DrawLine(p_pso, p_position + p_rotation * Maths::FVector3{ -p_size.x, -p_size.y, -p_size.z }, p_position + p_rotation * Maths::FVector3{ -p_size.x, -p_size.y, +p_size.z }, p_color, p_lineWidth);
	DrawLine(p_pso, p_position + p_rotation * Maths::FVector3{ -p_size.x, p_size.y, -p_size.z }, p_position + p_rotation * Maths::FVector3{ -p_size.x, +p_size.y, +p_size.z }, p_color, p_lineWidth);
	DrawLine(p_pso, p_position + p_rotation * Maths::FVector3{ -p_size.x, -p_size.y, -p_size.z }, p_position + p_rotation * Maths::FVector3{ -p_size.x, +p_size.y, -p_size.z }, p_color, p_lineWidth);
	DrawLine(p_pso, p_position + p_rotation * Maths::FVector3{ -p_size.x, -p_size.y, +p_size.z }, p_position + p_rotation * Maths::FVector3{ -p_size.x, +p_size.y, +p_size.z }, p_color, p_lineWidth);
	DrawLine(p_pso, p_position + p_rotation * Maths::FVector3{ +p_size.x, -p_size.y, -p_size.z }, p_position + p_rotation * Maths::FVector3{ +p_size.x, -p_size.y, +p_size.z }, p_color, p_lineWidth);
	DrawLine(p_pso, p_position + p_rotation * Maths::FVector3{ +p_size.x, p_size.y, -p_size.z }, p_position + p_rotation * Maths::FVector3{ +p_size.x, +p_size.y, +p_size.z }, p_color, p_lineWidth);
	DrawLine(p_pso, p_position + p_rotation * Maths::FVector3{ +p_size.x, -p_size.y, -p_size.z }, p_position + p_rotation * Maths::FVector3{ +p_size.x, +p_size.y, -p_size.z }, p_color, p_lineWidth);
	DrawLine(p_pso, p_position + p_rotation * Maths::FVector3{ +p_size.x, -p_size.y, +p_size.z }, p_position + p_rotation * Maths::FVector3{ +p_size.x, +p_size.y, +p_size.z }, p_color, p_lineWidth);
	DrawLine(p_pso, p_position + p_rotation * Maths::FVector3{ -p_size.x, -p_size.y, -p_size.z }, p_position + p_rotation * Maths::FVector3{ +p_size.x, -p_size.y, -p_size.z }, p_color, p_lineWidth);
	DrawLine(p_pso, p_position + p_rotation * Maths::FVector3{ -p_size.x, +p_size.y, -p_size.z }, p_position + p_rotation * Maths::FVector3{ +p_size.x, +p_size.y, -p_size.z }, p_color, p_lineWidth);
	DrawLine(p_pso, p_position + p_rotation * Maths::FVector3{ -p_size.x, -p_size.y, +p_size.z }, p_position + p_rotation * Maths::FVector3{ +p_size.x, -p_size.y, +p_size.z }, p_color, p_lineWidth);
	DrawLine(p_pso, p_position + p_rotation * Maths::FVector3{ -p_size.x, +p_size.y, +p_size.z }, p_position + p_rotation * Maths::FVector3{ +p_size.x, +p_size.y, +p_size.z }, p_color, p_lineWidth);
}

void Rendering::Features::DebugShapeRenderFeature::DrawSphere(Rendering::Data::PipelineState p_pso, const Maths::FVector3& p_position, const Maths::FQuaternion& p_rotation, float p_radius, const Maths::FVector3& p_color, float p_lineWidth)
{
	if (!std::isinf(p_radius))
	{
		for (float i = 0; i <= 360.0f; i += 10.0f)
		{
			DrawLine(p_pso, p_position + p_rotation * (Maths::FVector3{ cos(i * (3.14f / 180.0f)), sin(i * (3.14f / 180.0f)), 0.f } *p_radius), p_position + p_rotation * (Maths::FVector3{ cos((i + 10.0f) * (3.14f / 180.0f)), sin((i + 10.0f) * (3.14f / 180.0f)), 0.f } *p_radius), p_color, p_lineWidth);
			DrawLine(p_pso, p_position + p_rotation * (Maths::FVector3{ 0.f, sin(i * (3.14f / 180.0f)), cos(i * (3.14f / 180.0f)) } *p_radius), p_position + p_rotation * (Maths::FVector3{ 0.f, sin((i + 10.0f) * (3.14f / 180.0f)), cos((i + 10.0f) * (3.14f / 180.0f)) } *p_radius), p_color, p_lineWidth);
			DrawLine(p_pso, p_position + p_rotation * (Maths::FVector3{ cos(i * (3.14f / 180.0f)), 0.f, sin(i * (3.14f / 180.0f)) } *p_radius), p_position + p_rotation * (Maths::FVector3{ cos((i + 10.0f) * (3.14f / 180.0f)), 0.f, sin((i + 10.0f) * (3.14f / 180.0f)) } *p_radius), p_color, p_lineWidth);
		}
	}
}

void Rendering::Features::DebugShapeRenderFeature::DrawCapsule(Rendering::Data::PipelineState p_pso, const Maths::FVector3& p_position, const Maths::FQuaternion& p_rotation, float p_radius, float p_height, const Maths::FVector3& p_color, float p_lineWidth)
{
	if (!std::isinf(p_radius))
	{
		float halfHeight = p_height / 2;

		Maths::FVector3 hVec = { 0.0f, halfHeight, 0.0f };

		for (float i = 0; i < 360.0f; i += 10.0f)
		{
			DrawLine(p_pso, p_position + p_rotation * (hVec + Maths::FVector3{ cos(i * (3.14f / 180.0f)), 0.f, sin(i * (3.14f / 180.0f)) } *p_radius), p_position + p_rotation * (hVec + Maths::FVector3{ cos((i + 10.0f) * (3.14f / 180.0f)), 0.f, sin((i + 10.0f) * (3.14f / 180.0f)) } *p_radius), p_color, p_lineWidth);
			DrawLine(p_pso, p_position + p_rotation * (-hVec + Maths::FVector3{ cos(i * (3.14f / 180.0f)), 0.f, sin(i * (3.14f / 180.0f)) } *p_radius), p_position + p_rotation * (-hVec + Maths::FVector3{ cos((i + 10.0f) * (3.14f / 180.0f)), 0.f, sin((i + 10.0f) * (3.14f / 180.0f)) } *p_radius), p_color, p_lineWidth);

			if (i < 180.f)
			{
				DrawLine(p_pso, p_position + p_rotation * (hVec + Maths::FVector3{ cos(i * (3.14f / 180.0f)), sin(i * (3.14f / 180.0f)), 0.f } *p_radius), p_position + p_rotation * (hVec + Maths::FVector3{ cos((i + 10.0f) * (3.14f / 180.0f)), sin((i + 10.0f) * (3.14f / 180.0f)), 0.f } *p_radius), p_color, p_lineWidth);
				DrawLine(p_pso, p_position + p_rotation * (hVec + Maths::FVector3{ 0.f, sin(i * (3.14f / 180.0f)), cos(i * (3.14f / 180.0f)) } *p_radius), p_position + p_rotation * (hVec + Maths::FVector3{ 0.f, sin((i + 10.0f) * (3.14f / 180.0f)), cos((i + 10.0f) * (3.14f / 180.0f)) } *p_radius), p_color, p_lineWidth);
			}
			else
			{
				DrawLine(p_pso, p_position + p_rotation * (-hVec + Maths::FVector3{ cos(i * (3.14f / 180.0f)), sin(i * (3.14f / 180.0f)), 0.f } *p_radius), p_position + p_rotation * (-hVec + Maths::FVector3{ cos((i + 10.0f) * (3.14f / 180.0f)), sin((i + 10.0f) * (3.14f / 180.0f)), 0.f } *p_radius), p_color, p_lineWidth);
				DrawLine(p_pso, p_position + p_rotation * (-hVec + Maths::FVector3{ 0.f, sin(i * (3.14f / 180.0f)), cos(i * (3.14f / 180.0f)) } *p_radius), p_position + p_rotation * (-hVec + Maths::FVector3{ 0.f, sin((i + 10.0f) * (3.14f / 180.0f)), cos((i + 10.0f) * (3.14f / 180.0f)) } *p_radius), p_color, p_lineWidth);
			}
		}

		DrawLine(p_pso, p_position + p_rotation * (Maths::FVector3{ -p_radius, -halfHeight, 0.f }), p_position + p_rotation * (Maths::FVector3{ -p_radius, +halfHeight, 0.f }), p_color, p_lineWidth);
		DrawLine(p_pso, p_position + p_rotation * (Maths::FVector3{ p_radius, -halfHeight, 0.f }), p_position + p_rotation * (Maths::FVector3{ p_radius, +halfHeight, 0.f }), p_color, p_lineWidth);
		DrawLine(p_pso, p_position + p_rotation * (Maths::FVector3{ 0.f, -halfHeight, -p_radius }), p_position + p_rotation * (Maths::FVector3{ 0.f, +halfHeight, -p_radius }), p_color, p_lineWidth);
		DrawLine(p_pso, p_position + p_rotation * (Maths::FVector3{ 0.f, -halfHeight, p_radius }), p_position + p_rotation * (Maths::FVector3{ 0.f, +halfHeight, p_radius }), p_color, p_lineWidth);
	}
}
