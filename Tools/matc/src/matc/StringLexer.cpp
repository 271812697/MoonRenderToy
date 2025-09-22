#include "StringLexer.h"

namespace matc {

bool StringPairLexer::readValue() noexcept
{
    if( isAphaNumericCharacter(*mCursor) )
    {
        readKey();
    }
    else
    {
        size_t braceCount = 0;
        while (hasMore()) {
             skipWhiteSpace();

            // This can occur if the block is malformed.
            if (mCursor >= mEnd) {
                return false;
            }

             if( *mCursor == '{' || *mCursor == '[' )
            {
                 braceCount++;
            }
            else if( *mCursor == '}' || *mCursor == ']' )
            {
                 braceCount--;
             }

             if (braceCount == 0) {
                 consume();
                 return true;
             }

             consume();
         }
    }

     return true;
}

void StringPairLexer::readKey() noexcept
{
    while (hasMore() && isAphaNumericCharacter(*mCursor)) {
        consume();
    }
}

void StringPairLexer::readUnknown() noexcept
{
    consume();
}

bool StringPairLexer::peek(StringPairType* type) const noexcept
{
    return true;
}

StringPairLexer::StringPairLexer()
{
    mType = StringPairType::KEY;
}

void StringPairLexer::switchType()
{
    if( mType == StringPairType::KEY )
    {
        mType = StringPairType::VALUE;
    }
    else if(mType == StringPairType::VALUE)
    {
        mType = StringPairType::KEY;
    }
}

bool StringPairLexer::readLexeme() noexcept
{
    skipWhiteSpace();
    if( !hasMore() )
    {
        return mType == StringPairType::VALUE;
    }
    StringPairType nextMaterialType=mType;
    const char* lexemeStart = mCursor;
    size_t line = getLine();
    size_t cursor = getCursor();
    //to read
    if(nextMaterialType==StringPairType::KEY)
    {
        readKey();
    }
    else if(nextMaterialType==StringPairType::VALUE)
    {
        readValue();
    }
    mLexemes.emplace_back(nextMaterialType, lexemeStart, mCursor - 1, line, cursor);
    switchType();
    return true;
}

} // namespace matc
