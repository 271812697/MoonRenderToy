

#include "MaterialLexer.h"

namespace TEST {

	bool MaterialLexer::readBlock() noexcept {
		size_t braceCount = 0;
		while (hasMore()) {
			skipWhiteSpace();

			// This can occur if the block is malformed.
			if (mCursor >= mEnd) {
				return false;
			}

			if (*mCursor == '{') {
				braceCount++;
			}
			else if (*mCursor == '}') {
				braceCount--;
			}

			if (braceCount == 0) {
				consume();
				return true;
			}

			consume();
		}
		return true;
	}

	void MaterialLexer::readIdentifier() noexcept {
		while (hasMore() && isAphaNumericCharacter(*mCursor)) {
			consume();
		}
	}

	void MaterialLexer::readUnknown() noexcept {
		consume();
	}

	bool MaterialLexer::peek(MaterialType* type) const noexcept {
		if (!hasMore()) return false;

		char c = *mCursor;
		if (isAlphaCharacter(c)) {
			*type = MaterialType::IDENTIFIER;
		}
		else if (c == '{') {
			*type = MaterialType::BLOCK;
		}
		else {
			*type = MaterialType::UNKNOWN;
		}

		return true;
	}

	bool MaterialLexer::readLexeme() noexcept {
		skipWhiteSpace();

		MaterialType nextMaterialType;
		bool peeked = peek(&nextMaterialType);
		if (!peeked) {
			return true;
		}

		const char* lexemeStart = mCursor;
		size_t line = getLine();
		size_t cursor = getCursor();
		switch (nextMaterialType) {
		case MaterialType::BLOCK:
			if (!readBlock()) {
				nextMaterialType = MaterialType::UNKNOWN;
				break;
			}
		case MaterialType::IDENTIFIER: readIdentifier(); break;
		case MaterialType::UNKNOWN:    readUnknown();    break;
		default:
			break;
		}
		mLexemes.emplace_back(nextMaterialType, lexemeStart, mCursor - 1, line, cursor);

		return nextMaterialType != UNKNOWN;
	}

} // namespace matc
