//
// Created by ilya on 17.01.2020.
//

#include <page.hpp>

#include <catch2/catch.hpp>

TEST_CASE("page allocation, usage & deallocation") {
    using namespace nlang;

    PageRange page_range = Page::AllocateRange(10);
    PageHandle page = page_range.begin();

    auto int_page = page.Typed<int>();

    REQUIRE(page.size() == Page::Size());
    REQUIRE(int_page.size() == page.size() / sizeof(int));

    int_page[1] = 2;
    (++int_page)[3] = 4;
    (int_page += 2)[5] = 6;
    (int_page + 1)[7] = 8;

    REQUIRE(int_page == page + 3);
    REQUIRE((page + 1).Typed<int>()[3] == 4);
    REQUIRE((page + 3).Typed<int>()[5] == 6);
    REQUIRE((page + 4).Typed<int>()[7] == 8);

    auto a = int_page--;
    auto b = int_page--;
    auto c = int_page--;

    REQUIRE(a == page + 3);
    REQUIRE(b == page + 2);
    REQUIRE(c == page + 1);
    REQUIRE(int_page == page);

    int_page += 5123;
    int_page -= 5123;

    REQUIRE(int_page == page);

    REQUIRE(int_page < page + 1);
    REQUIRE_FALSE(int_page > page + 1);
    REQUIRE_FALSE(int_page - 123 != page - 123);
    REQUIRE(int_page - 123 == page - 123);
    REQUIRE(int_page - 123 >= page - 123);
    REQUIRE(int_page - 123 <= page - 123);
    REQUIRE_FALSE(int_page - 123 > page - 123);
    REQUIRE_FALSE(int_page - 123 < page - 123);

    size_t ii = 0;
    for (auto p : page_range) {
        p.Typed<size_t>()[ii] = ii;
        ++ii;
    }

    ii = 0;
    for (auto p : page_range.Typed<size_t>()) {
        REQUIRE(p[ii] == (page.Typed<size_t>() + ii)[ii]);
        ++ii;
    }

    Page::FreeRange(page_range);

    // TODO idk how to test memory allocation & deallocation in cross-platform way...
}