#include "DriverBase.h"
#include "Driver.h"
#include "CommandStream.h"
#include "OpenGLContext.h"
#include "OpenGLProgram.h"
#include "test/utils/GLUtils.h"
#include <functional>
#include <mutex>
#include <utility>
#include <glad/glad.h>
#include <stddef.h>
#include <stdint.h>
#include <iostream>

#define UTILS_HAS_THREADING 0
namespace TEST {
	size_t Driver::getElementTypeSize(ElementType type) noexcept
	{
		switch (type) {
		case ElementType::BYTE:     return sizeof(int8_t);
		case ElementType::BYTE2:    return 2 * sizeof(int8_t);
		case ElementType::BYTE3:    return 3 * sizeof(int8_t);
		case ElementType::BYTE4:    return 4 * sizeof(int8_t);
		case ElementType::UBYTE:    return sizeof(uint8_t);
		case ElementType::UBYTE2:   return 2 * sizeof(int8_t);
		case ElementType::UBYTE3:   return 3 * sizeof(int8_t);
		case ElementType::UBYTE4:   return 4 * sizeof(int8_t);
		case ElementType::SHORT:    return sizeof(int16_t);
		case ElementType::SHORT2:   return 2 * sizeof(int16_t);
		case ElementType::SHORT3:   return 3 * sizeof(int16_t);
		case ElementType::SHORT4:   return 4 * sizeof(int16_t);
		case ElementType::USHORT:   return sizeof(uint16_t);
		case ElementType::USHORT2:  return 2 * sizeof(int16_t);
		case ElementType::USHORT3:  return 3 * sizeof(int16_t);
		case ElementType::USHORT4:  return 4 * sizeof(int16_t);
		case ElementType::INT:      return sizeof(int32_t);
		case ElementType::UINT:     return sizeof(uint32_t);
		case ElementType::FLOAT:    return sizeof(float);
		case ElementType::FLOAT2:   return 2 * sizeof(float);
		case ElementType::FLOAT3:   return 3 * sizeof(float);
		case ElementType::FLOAT4:   return 4 * sizeof(float);
		case ElementType::HALF:     return 2;// sizeof(half);
		case ElementType::HALF2:    return 4;// sizeof(half2);
		case ElementType::HALF3:    return 6;// sizeof(half3);
		case ElementType::HALF4:    return 8;// sizeof(half4);
		}
	}
	Driver::Driver() :mHandleAllocator("handles", 4194304, false), mShaderCompilerService(*this)
	{
		// here we check we're on a supported version of GL before initializing the driver
		GLint major = 0, minor = 0;
		bool const success = OpenGLContext::queryOpenGLVersion(&major, &minor);
		if ((!success)) {
			std::cout << "Can't get OpenGL version" << std::endl;;


		}
		mContext = std::make_shared<OpenGLContext>();
	}

	Driver::~Driver()
	{
	}

	void Driver::execute(std::function<void(void)> const& fn) {
		fn();
	}

	void Driver::test(int val)
	{
		std::cout << val << std::endl;
	}

	Handle<HwVertexBuffer> Driver::createVertexBufferS()
	{
		return initHandle<GLVertexBuffer>();
	}

	void Driver::createVertexBufferR(Handle<HwVertexBuffer> vbh, uint32_t vertexCount, Handle<HwVertexBufferInfo> vbih)
	{
		construct<GLVertexBuffer>(vbh, vertexCount, vbih);
	}

	Handle<HwIndexBuffer> Driver::createIndexBufferS()
	{
		return initHandle<GLIndexBuffer>();
	}

	void Driver::createIndexBufferR(Handle<HwIndexBuffer> ibh, ElementType elementType, uint32_t indexCount, BufferUsage usage)
	{
		//to do
		auto& gl = mContext;
		uint8_t const elementSize = static_cast<uint8_t>(getElementTypeSize(elementType));
		GLIndexBuffer* ib = construct<GLIndexBuffer>(ibh, elementSize, indexCount);
		glGenBuffers(1, &ib->gl.buffer);
		GLsizeiptr const size = elementSize * indexCount;
		gl->bindVertexArray(nullptr);
		gl->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->gl.buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, GLUtils::getBufferUsage(usage));
		CHECK_GL_ERROR;
	}
	void Driver::createBufferObjectR(Handle<HwBufferObject> boh,
		uint32_t byteCount, BufferObjectBinding bindingType, BufferUsage usage) {

		assert(byteCount > 0);

		auto& gl = mContext;
		if (bindingType == BufferObjectBinding::VERTEX) {
			gl->bindVertexArray(nullptr);
		}

		GLBufferObject* bo = construct<GLBufferObject>(boh, byteCount, bindingType, usage);
		bo->gl.binding = GLUtils::getBufferBindingType(bindingType);
		glGenBuffers(1, &bo->gl.id);
		gl->bindBuffer(bo->gl.binding, bo->gl.id);
		glBufferData(bo->gl.binding, byteCount, nullptr, GLUtils::getBufferUsage(usage));
		CHECK_GL_ERROR
	}

	Handle<HwProgram> Driver::createProgramS()
	{

		return initHandle<OpenGLProgram>();
	}

	void Driver::createProgramR(Handle<HwProgram> ph, Program&& program)
	{
		std::cout << "Create Program" << std::endl;
		construct<OpenGLProgram>(ph, *this, std::move(program));
	}





} // namespace filament::backend
