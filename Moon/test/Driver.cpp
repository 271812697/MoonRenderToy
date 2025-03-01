#include "DriverBase.h"
#include "Driver.h"
#include "CommandStream.h"
#include <functional>
#include <mutex>
#include <utility>

#include <stddef.h>
#include <stdint.h>
#include <iostream>

#define UTILS_HAS_THREADING 0
namespace TEST {

	Driver::Driver() :mHandleAllocator("handles", 4194304, false)
	{
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
	}

} // namespace filament::backend
