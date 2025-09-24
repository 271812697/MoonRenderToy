#pragma once

#include "Lexer.h"
#include "JsonishLexeme.h"

namespace matc {

// Breaks down a stream of chars into JsonLexemes.
class JsonishLexer final: public Lexer<JsonLexeme> {
public:
    virtual bool readLexeme() noexcept;
private:
    JsonType readIdentifier() noexcept;
    void readNumber() noexcept;
    void readString() noexcept;
    JsonType readPunctuation() noexcept;
};

} // namespace matc
