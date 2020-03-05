#pragma once

#include <utils/defs.hpp>

#include <utility>
#include <string>
#include <string_view>
#include <memory>
#include <fstream>
#include <optional>
#include <cstddef>
#include <iterator>
#include <vector>


namespace nlang {


class ICharStream {
public:
    ICharStream(const ICharStream&) = delete;
    ICharStream(ICharStream&&) = delete;
    ICharStream& operator=(const ICharStream&) = delete;
    ICharStream& operator=(ICharStream&&) = delete;

    virtual bool HasNext() = 0;
    virtual char Next() = 0;

    virtual ~ICharStream() = default;

protected:
    ICharStream() = default;
};


class StringCharStream : public ICharStream {
public:
    StringCharStream() = delete;
    StringCharStream(const StringCharStream&) = delete;
    StringCharStream(StringCharStream&&) = delete;
    StringCharStream& operator=(const StringCharStream&) = delete;
    StringCharStream& operator=(StringCharStream&&) = delete;

    bool HasNext() override {
        return pos != source.length();
    }

    char Next() override {
        return source[pos++];
    }

    static std::unique_ptr<StringCharStream> New(const std::string& source) {
        return std::unique_ptr<StringCharStream>(new StringCharStream(source));
    }

protected:
    explicit StringCharStream(const std::string& source) : source(source) {}
    explicit StringCharStream(std::string&& source) : source(std::move(source)) {}

    std::string source;
    size_t pos = 0;
};


class FileCharStream : public ICharStream {
public:
    FileCharStream() = delete;
    FileCharStream(const FileCharStream&) = delete;
    FileCharStream(FileCharStream&&) = delete;
    FileCharStream& operator=(const FileCharStream&) = delete;
    FileCharStream& operator=(FileCharStream&&) = delete;

    bool HasNext() override {
        OpenFile();
        return !file->eof();
    }

    char Next() override {
        OpenFile();
        char c;
        *file >> c;
        return c;
    }

    static std::unique_ptr<FileCharStream> New(const std::string& path) {
        return std::unique_ptr<FileCharStream>(new FileCharStream(path));
    }

private:
    explicit FileCharStream(const std::string& path) : path(path) {}

    void OpenFile() {
        if (file) {
            return;
        }
        file = std::fstream(path);
        if (file->fail()) {
            throw std::runtime_error("failed to open file \"" + path + "\"");
        }
        *file >> std::noskipws;
    }

    const std::string path;
    std::optional<std::fstream> file;
};


}
