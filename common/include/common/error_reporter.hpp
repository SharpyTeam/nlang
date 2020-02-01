#pragma once

#include <string>

namespace nlang {

class IErrorReporter {
public:
    virtual void ReportError(const std::string& error) = 0;
    virtual void ReportError(size_t row, size_t column, const std::string& error) = 0;

    virtual ~IErrorReporter() = default;
};

}
