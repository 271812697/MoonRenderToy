#include <glad/glad.h>
#include "Interactive/imgui/imgui.h"
#include "Interactive/Im2DRenderer.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/Context.h"
#include "renderer/SceneView.h"
#include <Rendering/Resources/Loaders/ShaderLoader.h>
#include <Rendering/Data/Material.h>
#include <Rendering/HAL/IndexBuffer.h>
#include <Rendering/HAL/VertexArray.h>
#include <Rendering/HAL/VertexBuffer.h>
#include <Rendering/Settings/VertexAttribute.h>
#include <Rendering/Settings/EOperation.h>
#include <Rendering/Settings/EBlendingEquation.h>
#include <Rendering/Settings/EBlendingFactor.h>
#include <Rendering/Settings/EComparaisonAlgorithm.h>
#include "Qtimgui/imgui/imgui.h"
namespace MOON {
    namespace Render2D {
static std::string v = R"(
#version 450 core
layout(location = 0) in vec2 Position;
layout(location = 1) in vec2 Uv;
layout(location = 2) in vec4 Color;

uniform mat4 ProjMtx;

out vec2 Frag_UV;
out vec4 Frag_Color;

void main()
{
    Frag_UV = Uv;
    Frag_Color = Color;
    gl_Position = ProjMtx * vec4(Position, 0.0, 1.0);
}
)";
static std::string f = R"(
#version 450 core

in vec2 Frag_UV;
in vec4 Frag_Color;
uniform sampler2D Texture;
layout (location = 0) out vec4 Out_Color;
void main()
{
    Out_Color = Frag_Color ;//* texture(Texture, Frag_UV.st);
};

)";
        class Im2DRender::Internal {
        public:
            Internal(Im2DRender*s):self(s) {}
            ~Internal(){}
            void init() {
                if (mIsInit) {
                    return;
                }
                mIsInit = true;
                mShader = ::Rendering::Resources::Loaders::ShaderLoader::CreateFromSource(v, f);
                mMat.SetShader(mShader);

                mVertexArray = new ::Rendering::HAL::VertexArray();
                mIndexBuffer = new ::Rendering::HAL::IndexBuffer();
                mVertexBuffer = new ::Rendering::HAL::VertexBuffer();
                mVertexArray->SetLayout(
                    std::to_array<::Rendering::Settings::VertexAttribute>(
                        {
                            { ::Rendering::Settings::EDataType::FLOAT, 2 }, // position
                            { ::Rendering::Settings::EDataType::FLOAT, 2 }, // texCoords
                            { ::Rendering::Settings::EDataType::UNSIGNED_BYTE, 4,true }//colors
                }), *mVertexBuffer, *mIndexBuffer);
            }
            void uploadVertex() {
                 //drawList
                if (mVertexBuffer->Allocate(drawList.VtxBuffer.size() * sizeof(ImDrawVert), ::Rendering::Settings::EAccessSpecifier::STREAM_DRAW))
                {
                    mVertexBuffer->Upload(drawList.VtxBuffer.Data);
                    if (mIndexBuffer->Allocate(drawList.IdxBuffer.size() * sizeof(unsigned int),::Rendering::Settings::EAccessSpecifier::STREAM_DRAW)) {
                        mIndexBuffer->Upload(drawList.IdxBuffer.Data);
                    }
                }
            }
            void test() {  
                drawList.AddLine({ 0,0, }, { 500,500 }, IM_COL32(255, 255, 255, 255), 1.0);
                drawList.AddNgonFilled({ 250,250 },100, IM_COL32(255, 255, 0, 255),8);
                drawList.AddCircle({ 500,500 }, 100.0, IM_COL32(255, 255, 0, 255),0,6.0);
                drawList.AddCircleFilled({ 500,500 }, 97.0, IM_COL32(255, 0, 0, 100), 0);
              
                drawList.AddRect({ 100,100, }, { 600,600 }, IM_COL32(255, 255, 255, 255), 30.0, 3.0);
            }
        private:
            friend Im2DRender;
            Im2DRender* self = nullptr;
            ImDrawList drawList;
            ::Rendering::Resources::Shader* mShader;
            ::Rendering::Data::Material mMat;
            ::Rendering::HAL::VertexBuffer* mVertexBuffer;
            ::Rendering::HAL::IndexBuffer* mIndexBuffer;
            ::Rendering::HAL::VertexArray* mVertexArray;
            bool mIsInit = false;
        };

        Im2DRender::~Im2DRender()
        {
            delete mInternal;
        }
        Im2DRender& Im2DRender::instance()
        {
            static Im2DRender render2D;
            return render2D;
        }
        ImDrawList* Im2DRender::getDrawList()
        {
            return &mInternal->drawList;
        }
        void Im2DRender::newFrame()
        {
            if (!mInternal->mIsInit) {
                mInternal->init();
            }
            mInternal->drawList._ResetForNewFrame();
        }
        void Im2DRender::endFrame()
        {
            mInternal->test();
            mInternal->uploadVertex();
            auto driver = Core::Global::ServiceLocator::Get<Editor::Core::Context>().driver.get();
            auto p_pso = driver->CreatePipelineState();
            p_pso.stencilTest = true;
            p_pso.stencilWriteMask = 0xFF;
            p_pso.stencilFuncRef = 1;
            p_pso.stencilFuncMask = 0xFF;
            p_pso.stencilOpFail = ::Rendering::Settings::EOperation::REPLACE;
            p_pso.depthOpFail = Rendering::Settings::EOperation::REPLACE;
            p_pso.bothOpFail = Rendering::Settings::EOperation::REPLACE;
            p_pso.colorWriting.mask = 0x00;
            p_pso.depthWriting = true;
            p_pso.colorWriting.mask = 0xFF;
            p_pso.blending = true;
            p_pso.blendingEquation = Rendering::Settings::EBlendingEquation::FUNC_ADD;
            p_pso.blendingSrcFactor = Rendering::Settings::EBlendingFactor::SRC_ALPHA;
            p_pso.depthFunc = Rendering::Settings::EComparaisonAlgorithm::ALWAYS;
            p_pso.culling = 0;
            p_pso.depthTest = false;
            driver->SetPipelineState(p_pso);
            mInternal->mVertexArray->Bind();
            Maths::FMatrix4 projMat;
            auto& view = GetService(Editor::Panels::SceneView);
            std::pair<uint16_t,uint16_t> safeSize=view.GetSafeSize();
            float L = 0;
            float R = safeSize.first;
            float T = 0;
            float B = safeSize.second;

            const float ortho_projection[4][4] =
            {
                { 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
                { 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
                { 0.0f,         0.0f,        -1.0f,   0.0f },
                { (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
            };
            memcpy(projMat.data, ortho_projection,16*sizeof(float));
            projMat=projMat.TransposeMartix();
            mInternal->mMat.SetProperty("ProjMtx",projMat);
            mInternal->mMat.Bind();
            for (int i = 0;i < mInternal->drawList.CmdBuffer.size();i++) {
                auto& cmd = mInternal->drawList.CmdBuffer[i];
                int offsetIndex=cmd.IdxOffset;
                int offsetVertex = cmd.VtxOffset;
                glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)cmd.ElemCount, GL_UNSIGNED_INT, (void*)(intptr_t)(cmd.IdxOffset * sizeof(ImDrawIdx)), (GLint)cmd.VtxOffset);
            }
            mInternal->mMat.Unbind();
            mInternal->mVertexArray->Unbind();
        }
        Im2DRender::Im2DRender():mInternal(new Internal(this))
        {
        }
    }
}