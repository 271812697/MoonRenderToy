#pragma once
#include <functional>

#include <stddef.h>
#include <stdint.h>
#include "DriverBase.h"
#include "GLBufferObject.h"
#include "OpenGLContext.h"
#include "DriverEnums.h"
#include "HandleAllocator.h"
#include "ShaderCompilerService.h"
// Command debugging off. debugging virtuals are not called.
// This is automatically enabled in DEBUG builds.
#define FILAMENT_DEBUG_COMMANDS_NONE         0x0
// Command debugging enabled. No logging by default.
#define FILAMENT_DEBUG_COMMANDS_ENABLE       0x1
// Command debugging enabled. Every command logged to slog.d
#define FILAMENT_DEBUG_COMMANDS_LOG          0x2
// Command debugging enabled. Every command logged to systrace
#define FILAMENT_DEBUG_COMMANDS_SYSTRACE     0x4

#define FILAMENT_DEBUG_COMMANDS              FILAMENT_DEBUG_COMMANDS_NONE

namespace TEST {

	class Dispatcher;
	class CommandStream;
	class OpenGLProgram;
	class Driver {
	public:
		static size_t getElementTypeSize(ElementType type) noexcept;
		Driver();

		virtual ~Driver();

		OpenGLContext& getContext() noexcept { return *mContext; }
		struct GLVertexBufferInfo : public HwVertexBufferInfo {
			GLVertexBufferInfo() noexcept = default;
			GLVertexBufferInfo(uint8_t bufferCount, uint8_t attributeCount,
				AttributeArray const& attributes)
				: HwVertexBufferInfo(bufferCount, attributeCount),
				attributes(attributes) {
			}
			AttributeArray attributes;
		};

		struct GLVertexBuffer : public HwVertexBuffer {
			GLVertexBuffer() noexcept = default;
			GLVertexBuffer(uint32_t vertexCount, Handle<HwVertexBufferInfo> vbih)
				: HwVertexBuffer(vertexCount), vbih(vbih) {
			}
			Handle<HwVertexBufferInfo> vbih;
			struct {
				// 4 * MAX_VERTEX_ATTRIBUTE_COUNT bytes
				std::array<unsigned int, MAX_VERTEX_ATTRIBUTE_COUNT> buffers{};
			} gl;
		};

		struct GLIndexBuffer : public HwIndexBuffer {
			using HwIndexBuffer::HwIndexBuffer;
			struct {
				unsigned int buffer{};
			} gl;
		};
		bool useProgram(OpenGLProgram* p);
		virtual void execute(std::function<void(void)> const& fn);
	public:

		void test(int val);
		Handle<HwProgram>createProgramS();
		void createProgramR(Handle<HwProgram> ph, Program&& program);
		Handle<HwVertexBuffer> createVertexBufferS();
		void createVertexBufferR(Handle<HwVertexBuffer> vbh, uint32_t vertexCount, Handle<HwVertexBufferInfo> vbih);
		Handle<HwIndexBuffer> createIndexBufferS();
		void createIndexBufferR(Handle<HwIndexBuffer> ibh, ElementType elementType, uint32_t indexCount, BufferUsage usage);
		void createBufferObjectR(Handle<HwBufferObject> boh, uint32_t byteCount, BufferObjectBinding bindingType, BufferUsage usage);
	private:
		OpenGLProgram* mBoundProgram = nullptr;
		utils::bitset8 mInvalidDescriptorSetBindings;
		friend class OpenGLProgram;
		friend class ShaderCompilerService;

		std::shared_ptr<OpenGLContext> mContext;

		HandleAllocator<32, 96, 136> mHandleAllocator;
		ShaderCompilerService mShaderCompilerService;
		ShaderCompilerService& getShaderCompilerService() noexcept {
			return mShaderCompilerService;
		}
		template<typename D, typename ... ARGS>
		Handle<D> initHandle(ARGS&& ... args) {
			return mHandleAllocator.allocateAndConstruct<D>(std::forward<ARGS>(args) ...);
		}

		template<typename D, typename B, typename ... ARGS>
		typename std::enable_if<std::is_base_of<B, D>::value, D>::type*
			construct(Handle<B> const& handle, ARGS&& ... args) {
			return mHandleAllocator.destroyAndConstruct<D, B>(handle, std::forward<ARGS>(args) ...);
		}

		template<typename B, typename D,
			typename = typename std::enable_if<std::is_base_of<B, D>::value, D>::type>
		void destruct(Handle<B>& handle, D const* p) noexcept {
			return mHandleAllocator.deallocate(handle, p);
		}

		template<typename Dp, typename B>
		typename std::enable_if_t<
			std::is_pointer_v<Dp>&&
			std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
			handle_cast(Handle<B>& handle) {
			return mHandleAllocator.handle_cast<Dp, B>(handle);
		}

		template<typename B>
		bool is_valid(Handle<B>& handle) {
			return mHandleAllocator.is_valid(handle);
		}

		template<typename Dp, typename B>
		inline typename std::enable_if_t<
			std::is_pointer_v<Dp>&&
			std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
			handle_cast(Handle<B> const& handle) {
			return mHandleAllocator.handle_cast<Dp, B>(handle);
		}


	};

} // namespace filament::backend


