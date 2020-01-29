//
// Created by ilya on 17.01.2020.
//

#include <page.hpp>

#include <catch2/catch.hpp>

TEST_CASE("page allocation, usage & deallocation") {
    using namespace nlang;

    PageRange page_range = Page::AllocateRange(10);
    PageHandle page = page_range.begin();

    REQUIRE(page.size() == Page::Size());

    void* data = nullptr;
    for (auto& p : page_range) {
        REQUIRE((data == nullptr || (size_t)p.data() == (size_t)data + Page::Size()));
        data = p.data();
    }

    Page::FreeRange(page_range);

    // TODO idk how to test memory allocation & deallocation in cross-platform way...
}