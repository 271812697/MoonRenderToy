#pragma once

#include "Lexeme.h"

namespace TEST {

	enum MaterialType {
		BLOCK,
		IDENTIFIER,
		UNKNOWN
	};

	class MaterialLexeme final : public Lexeme<MaterialType> {
	public:
		MaterialLexeme(MaterialType type, const char* start, const char* end, size_t line, size_t pos) :
			Lexeme(type, start, end, line, pos) {
		}

		MaterialLexeme trimBlockMarkers() const {
			return { mType, mStart + 1, mEnd - 1, mLineNumber, mPosition };
		}
	};

}