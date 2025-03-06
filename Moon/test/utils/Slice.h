/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_UTILS_SLICE_H
#define TNT_UTILS_SLICE_H



#include <utility>

#include <assert.h>
#include <stddef.h>

namespace utils {

/*
 * A fixed-size slice of a container
 */
template <typename T>
class Slice {
public:
    using iterator = T*;
    using const_iterator = T const*;
    using value_type = T;
    using reference = T&;
    using const_reference = T const&;
    using pointer = T*;
    using const_pointer = T const*;
    using size_type = size_t;

    Slice() noexcept = default;

    Slice(const_iterator begin, const_iterator end) noexcept
            : mBegin(const_cast<iterator>(begin)),
              mEnd(const_cast<iterator>(end)) {
    }

    Slice(const_pointer begin, size_type count) noexcept
            : mBegin(const_cast<iterator>(begin)),
              mEnd(mBegin + count) {
    }

    Slice(Slice const& rhs) noexcept = default;
    Slice(Slice&& rhs) noexcept = default;
    Slice& operator=(Slice const& rhs) noexcept = default;
    Slice& operator=(Slice&& rhs) noexcept = default;

    void set(pointer begin, size_type count)  noexcept {
        mBegin = begin;
        mEnd = begin + count;
    }

    void set(iterator begin, iterator end)  noexcept {
        mBegin = begin;
        mEnd = end;
    }

    void swap(Slice& rhs)  noexcept {
        std::swap(mBegin, rhs.mBegin);
        std::swap(mEnd, rhs.mEnd);
    }

    void clear()  noexcept {
        mBegin = nullptr;
        mEnd = nullptr;
    }

    // size
    size_t size() const  noexcept { return mEnd - mBegin; }
    size_t sizeInBytes() const  noexcept { return size() * sizeof(T); }
    bool empty() const  noexcept { return size() == 0; }

    // iterators
    iterator begin()  noexcept { return mBegin; }
    const_iterator begin() const  noexcept { return mBegin; }
    const_iterator cbegin() const  noexcept { return this->begin(); }
    iterator end()  noexcept { return mEnd; }
    const_iterator end() const  noexcept { return mEnd; }
    const_iterator cend() const  noexcept { return this->end(); }

    // data access
    reference operator[](size_t n)  noexcept {
        assert(n < size());
        return mBegin[n];
    }

    const_reference operator[](size_t n) const  noexcept {
        assert(n < size());
        return mBegin[n];
    }

    reference at(size_t n)  noexcept {
        return operator[](n);
    }

    const_reference at(size_t n) const  noexcept {
        return operator[](n);
    }

    reference front()  noexcept {
        assert(!empty());
        return *mBegin;
    }

    const_reference front() const  noexcept {
        assert(!empty());
        return *mBegin;
    }

    reference back()  noexcept {
        assert(!empty());
        return *(this->end() - 1);
    }

    const_reference back() const  noexcept {
        assert(!empty());
        return *(this->end() - 1);
    }

    pointer data()  noexcept {
        return this->begin();
    }

    const_pointer data() const  noexcept {
        return this->begin();
    }

protected:
    iterator mBegin = nullptr;
    iterator mEnd = nullptr;
};

} // namespace utils

#endif // TNT_UTILS_SLICE_H
