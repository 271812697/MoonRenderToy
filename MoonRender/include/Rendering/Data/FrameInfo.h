#pragma once
#include <cstdint>

namespace Rendering::Data
{
	/**
	* Holds information about a given frame
	*/
	struct FrameInfo
	{
		uint64_t batchPolyCount = 0;
		uint64_t batchLineCount = 0;
		uint64_t instancePolyCount = 0;
		uint64_t instancelineCount = 0;
		uint64_t polyCount = 0;
		uint64_t lineCount = 0;
		uint64_t vertexPolyCount = 0;
		uint64_t vertexLineCount = 0;
		uint64_t vertexCount = 0;
		void reset() {
			batchPolyCount = 0;
			batchLineCount = 0;
			instancePolyCount = 0;
			instancelineCount = 0;
			polyCount = 0;
			lineCount = 0;
			vertexPolyCount = 0;
			vertexLineCount = 0;
			vertexCount = 0;
		}
	};
}
