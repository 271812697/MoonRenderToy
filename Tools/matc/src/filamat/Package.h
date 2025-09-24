#pragma once

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>  // memcpy

#include <cstddef>
#include <functional>


namespace filamat {

class  Package {
public:
    Package() = default;

    // Regular constructor
    explicit Package(size_t size) : mSize(size) {
        mPayload = new uint8_t[size];
    }

    Package(const void* src, size_t size) : Package(size) {
        memcpy(mPayload, src, size);
    }

    // Move Constructor
    Package(Package&& other) noexcept : mPayload(other.mPayload), mSize(other.mSize),
            mValid(other.mValid) {
        other.mPayload = nullptr;
        other.mSize = 0;
        other.mValid = false;
    }

    // Move assignment
    Package& operator=(Package&& other) noexcept {
        std::swap(mPayload, other.mPayload);
        std::swap(mSize, other.mSize);
        std::swap(mValid, other.mValid);
        return *this;
    }

    // Copy assignment operator disallowed.
    Package& operator=(const Package& other) = delete;

    // Copy constructor disallowed.
    Package(const Package& other) = delete;

    ~Package() {
        delete[] mPayload;
    }

    uint8_t* getData() const noexcept {
        return mPayload;
    }

    size_t getSize() const noexcept {
        return mSize;
    }

    uint8_t* getEnd() const noexcept {
        return mPayload + mSize;
    }

    void setValid(bool valid) noexcept {
        mValid = valid;
    }

    bool isValid() const noexcept {
        return mValid;
    }

    static Package invalidPackage() {
        Package package(0);
        package.setValid(false);
        return package;
    }

private:
    uint8_t* mPayload = nullptr;
    size_t mSize = 0;
    bool mValid = true;
};

}