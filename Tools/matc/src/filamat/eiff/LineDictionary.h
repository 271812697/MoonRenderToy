#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace filamat {

// Establish a line <-> id mapping. Use for shader compression when each shader is sliced in lines
// and each line encoded into a 16 bit id.
class LineDictionary {
public:
    LineDictionary() = default;

    // Due to the presence of unique_ptr, disallow copy construction but allow move construction.
    LineDictionary(LineDictionary const&) = delete;
    LineDictionary(LineDictionary&&) = default;

    void addText(const std::string& text) noexcept;
    size_t getLineCount() const;

    bool isEmpty() const noexcept {
        return mStrings.empty();
    }

    std::string getString(size_t index) const noexcept;
    size_t getIndex(std::string s) const noexcept;

private:
    void addLine(const std::string&& line) noexcept;

    std::unordered_map<std::string, size_t> mLineIndices;
    std::vector<std::unique_ptr<std::string>> mStrings;
};

} // namespace filamat
