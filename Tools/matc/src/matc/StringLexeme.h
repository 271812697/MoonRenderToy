#pragma once
#include "Lexeme.h"

namespace matc {

enum StringPairType {
    KEY,
    VALUE
};

class StringPairLexeme final : public Lexeme<StringPairType>
{
public:
    StringPairLexeme(StringPairType type, const char* start, const char* end, size_t line, size_t pos)
        :
            Lexeme(type, start, end, line, pos) {
    }

    StringPairLexeme trimBlockMarkers() const
    {
        if( *mStart == '{' || *mStart == '[' )
        {
            return { mType, mStart + 1, mEnd - 1, mLineNumber, mPosition };
        }
        return {mType, mStart , mEnd , mLineNumber, mPosition};
    }
};

} // namespace matc
