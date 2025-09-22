#pragma once
#include <fstream>
#include <iostream>
#include <string>

#include "Config.h"

namespace matc {

class FilesystemOutput : public Config::Output {
public:
    explicit FilesystemOutput(const char* path) : mPath(path) {
    }

    ~FilesystemOutput() override = default;

    bool open() noexcept override {
        mFile.open(mPath.c_str(), std::ofstream::out | std::ofstream::binary);
        return !mFile.fail();
    }

    bool write(const uint8_t* data, size_t size) noexcept override {
        mFile.write((char*)(data), size);
        return mFile.fail();
    }

    std::ostream& getOutputStream() noexcept override {
        return mFile;
    }

    bool close() noexcept override {
        mFile.close();
        return mFile.fail();
    };
private:
    const std::string mPath;
    std::ofstream mFile;
};

class FilesystemInput : public Config::Input {
public:
    explicit FilesystemInput(const char* path) : mPath(path) {
    }

    ~FilesystemInput() override = default;

    size_t open() noexcept override {
        mFile.open(mPath.c_str(), std::ifstream::binary | std::ios::ate);
        if (!mFile) {
            std::cerr << "Unable to open material source file '" << mPath << "'" << std::endl;
            return 0;
        }
        mFilesize = mFile.tellg();
        mFile.seekg(0, std::ios::beg);
        return mFilesize;
    }

    std::unique_ptr<const char[]> read() noexcept override {
        auto buffer = std::make_unique<char[]>(mFilesize);
        if (!mFile.read(buffer.get(), mFilesize)) {
            std::cerr << "Unable to read material source file '" << mPath << "'" << std::endl;
            return nullptr;
        }
        return std::unique_ptr<const char[]>(buffer.release());
    }

    bool close() noexcept override {
        mFile.close();
        return mFile.fail();
    }

    const char* getName() const noexcept override {
        return mPath.c_str();
    }

private:
    const std::string mPath;
    std::ifstream mFile;
    size_t mFilesize = 0;
};

class CommandlineConfig : public Config {
public:

    CommandlineConfig(int argc, char** argv);
    ~CommandlineConfig() override {
        delete mInput;
        delete mOutput;
    };

    Output* getOutput()  const noexcept override  {
        return mOutput;
    }

    Input* getInput() const noexcept override {
        return mInput;
    }

    std::string toString() const noexcept override {
        std::string parameters;
        for (size_t i = 0 ; i < mArgc; i++) {
            parameters += mArgv[i];
            parameters += " ";
        }
        return parameters;
    }

private:
    bool parse();

    int mArgc = 0;
    char** mArgv = nullptr;

    FilesystemInput* mInput = nullptr;
    FilesystemOutput* mOutput = nullptr;
};

} // namespace matc

