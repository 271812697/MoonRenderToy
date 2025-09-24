#include "Includes.h"
#include <string>

namespace filamat {

static bool isWhitespace(char c) {
    return (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v');
}

struct CommentRange {
    size_t begin;
    size_t end;

    CommentRange(size_t begin, size_t end) :
        begin(begin), end(end) {}
};

template<size_t BTSize, size_t ETSize>
static void findCommentRanges(const char (&beginToken)[BTSize], const char (&endToken)[ETSize],
        std::string& source, std::vector<CommentRange>& comments) {
    // BTSize and ETSize include the null terminator.
    constexpr size_t beginTokenSize = BTSize - 1;
    constexpr size_t endTokenSize = ETSize - 1;

    // Find the first occurance of the begin token.
    size_t r = source.find(beginToken);
    while (r != std::string::npos) {
        // This is the start of a new comment.
        const size_t commentStart = r;

        // Advance the pointer past the token.
        r += beginTokenSize;

        // Find the terminating token for this comment.
        r = source.find(endToken, r);
        if (r != std::string::npos) {
            // Advance the pointer past the token.
            r += endTokenSize;

            // The end of the comment is the last character of the end token.
            size_t commentEnd = r - 1;

            comments.emplace_back(commentStart, commentEnd);

            // Find the next comment.
            r = source.find(beginToken, r);
        } else {
            // This comment was never terminated- consider the rest of the source to a comment.
            // This also solves the case of a single-line source with a // comment but no
            // terminating newline.
            comments.emplace_back(commentStart, source.size() - 1);
        }
    }
}


bool resolveIncludes(IncludeResult& root, IncludeCallback callback,
        const ResolveOptions& options, size_t depth) {
    if (depth > 30) {
        return false;
    }
    const size_t lineNumberOffset = root.lineNumberOffset;
    std::string& text = root.text;

    std::vector<FoundInclude> includes = parseForIncludes(text);

    while (!includes.empty()) {
        const auto include = includes[0];
        // Ask the includer to resolve this include.
        if( !callback )
        {
            return false;
        }
        IncludeResult resolved;
        resolved.includeName = include.name;
        if( !callback(resolved) )
        {
            return false;
        }
        // Recursively resolve all of its includes.
        if (!resolveIncludes(resolved, callback, options, depth + 1)) {
            return false;
        }
        text.replace(include.startPosition, include.length, resolved.text);
        includes = parseForIncludes(text);
    }
    return true;
}

std::vector<FoundInclude> parseForIncludes(const std::string& source) {
    std::vector<FoundInclude> results;

    if (source.empty()) {
        return results;
    }
    std::string sourceString = source.c_str();

    std::vector<CommentRange> commentRanges;
    findCommentRanges("/*", "*/", sourceString, commentRanges);
    findCommentRanges("//", "\n", sourceString, commentRanges);

    auto isInsideComment = [&commentRanges] (size_t pos) {
        for (auto r : commentRanges) {
            if (pos >= r.begin && pos <= r.end) {
                return true;
            }
        }
        return false;
    };

    size_t result = sourceString.find("#include");

    while(result != std::string::npos) {
        const size_t includeStart = result;

        // Move to the character immediately after #include
        result += 8;

        // If this include is within a comment, ignore it.
        if (isInsideComment(includeStart)) {
            continue;
        }

        // Eat up any whitespace after "#include"
        while (result < sourceString.length() && isWhitespace(sourceString[result])) {
            result++;
        }

        // The next character must be a "
        if (result >= sourceString.length() || sourceString[result] != '"') {
            result = sourceString.find("#include", result);
            continue;
        }
        result++;

        const size_t nameStart = result;

        // Increment until we reach the next "
        while (result < sourceString.length() && sourceString[result] != '"') {
            result++;
        }

        // check we're not at the end of the line -- this would be a malformed include directive.
        if (result >= sourceString.length()) {
            result = sourceString.find("#include", result);
            continue;
        }


        const size_t nameEnd = result - 1;

        const size_t includeEnd = result;

        // Move on to the next character
        result++;
        // Grab the include name.
        const auto includeName = sourceString.substr(nameStart, nameEnd - nameStart + 1);
        // Calculate the line number of the include.
        size_t lineNumber = 1;
        for (size_t i = 0; i < includeStart; i++) {
            if (source[i] == '\n') {
                lineNumber++;
            }
        }
        results.push_back({std::string(includeName.c_str()), includeStart,
                includeEnd - includeStart + 1, lineNumber});
        // Find next occurrence.
        result = sourceString.find("#include", result);
    }
    return results;
}
} 
