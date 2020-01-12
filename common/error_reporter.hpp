//
// Created by selya on 21.11.2019.
//

#ifndef NLANG_ERROR_REPORTER_HPP
#define NLANG_ERROR_REPORTER_HPP

#include <string>

namespace nlang {

class IErrorReporter {
public:
    virtual void ReportError(const std::string& error) = 0;
    virtual void ReportError(size_t row, size_t column, const std::string& error) = 0;

    virtual ~ErrorReporter() = default;
};

}

#endif //NLANG_ERROR_REPORTER_HPP
