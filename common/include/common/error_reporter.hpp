#pragma once

#include <string>

namespace nlang {

/*
 * Interface for error reporter
 * Will be used for reporting errors in all modules
 */
class IErrorReporter {
public:
    /**
     * Report error, specified as string
     * @param error Error text
     */
    virtual void ReportError(const std::string& error) = 0;
    /**
     * Report error, connected with specified row and column
     * @param row The row, connected with error
     * @param column The column, connected with error
     * @param error Error text
     */
    virtual void ReportError(size_t row, size_t column, const std::string& error) = 0;

    virtual ~IErrorReporter() = default;
};

}
