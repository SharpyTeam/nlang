#pragma once

#include <utility>
#include <string>
#include <string_view>
#include <memory>
#include <fstream>
#include <optional>

namespace nlang {

class CharStream {
public:
    CharStream() = default;
    CharStream(const CharStream&) = delete;
    CharStream& operator=(const CharStream&) = delete;

    virtual bool HasNextChar() = 0;
    virtual char NextChar() = 0;

    inline operator bool() {
        return HasNextChar();
    }

    template<typename T, typename ...Args>
    static std::shared_ptr<T> Create(Args&&... args) {
        return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
    }

    virtual ~CharStream() = default;
};

class StringCharStream : public CharStream {
public:
    explicit StringCharStream(const std::string& source) : source(source) {}
    explicit StringCharStream(std::string&& source) : source(std::move(source)) {}
    StringCharStream(StringCharStream&& other) noexcept : source(std::move(other.source)) {}

    StringCharStream& operator=(StringCharStream&& other) noexcept {
        source = std::move(other.source);
        pos = other.pos;
        return *this;
    }

    bool HasNextChar() override {
        return pos != source.length();
    }

    char NextChar() override {
        return source[pos++];
    }

private:
    std::string source;
    size_t pos = 0;
};

class FileCharStream : public CharStream {
public:
    explicit FileCharStream(const std::string& path) : path(path) {}

    bool HasNextChar() override {
        OpenFile();
        return !file->eof();
    }

    char NextChar() override {
        OpenFile();
        char c;
        *file >> std::noskipws >> c;
        return c;
    }

private:
    void OpenFile() {
        if (!file) {
            file = std::fstream(path);
            if (file->fail()) {
                throw std::runtime_error("failed to open file \"" + path + "\"");
            }
        }
    }

    const std::string path;
    std::optional<std::fstream> file;
};

}
