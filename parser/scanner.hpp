//
// Created by selya on 05.11.2019.
//

#ifndef NLANG_SCANNER_HPP
#define NLANG_SCANNER_HPP

#include <string>
#include <string_view>

namespace nlang {

class Scanner {
public:
    explicit Scanner(const std::u32string_view &source) noexcept;
};

}

#endif //NLANG_SCANNER_HPP
