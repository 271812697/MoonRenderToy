#pragma once

#include "Lexer.h"
#include "StringLexeme.h"

namespace matc {

class StringPairLexer final : public Lexer<StringPairLexeme>
{
public:
    StringPairLexer();
private:
    void switchType();

    virtual bool readLexeme() noexcept override;

    bool peek(StringPairType* type) const noexcept;

    bool readValue() noexcept;
    void readKey() noexcept;
    void readUnknown() noexcept;
private:
    StringPairType mType;
};

}
