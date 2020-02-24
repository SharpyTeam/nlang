//
// Created by ilya on 17.01.2020.
//

#include <utils/page.hpp>

#include <catch2/catch.hpp>

TEST_CASE("page allocation, usage & deallocation") {
    using namespace nlang::utils;

    auto page_range = Page::AllocateRange(10);

    void* data = nullptr;
    for (auto it = page_range.first; it != page_range.second; ++it) {
        REQUIRE((data == nullptr || (size_t)it->data() == (size_t)data + Page::size()));
        data = it->data();
    }

    Page::FreeRange(page_range);
}