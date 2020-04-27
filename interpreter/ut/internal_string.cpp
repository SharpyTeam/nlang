#include <catch2/catch.hpp>
#include <interpreter/value.hpp>
#include <interpreter/objects/string.hpp>
#include <interpreter/heap.hpp>

TEST_CASE("string creation & manipulation") {
    using namespace nlang;

    Heap heap;
    std::string a("w");
    std::string r("ъ");
    std::u32string b(U"й");
    std::u32string w(U"q");
    std::u16string c(u"m");
    const char* d = "d";
    const char16_t* e = u"ф";
    const char32_t* f = U"ъ";

    /*heap.RegisterDeleterForType(Value::Type::STRING, [](HeapValue* value) {
        delete static_cast<String*>(value);
    });*/

    SECTION("strings can be created from STL strings") {
        auto s = String::New(&heap, a, b, c);
        REQUIRE(s->GetLength() == a.length() + b.length() + c.length());
//        REQUIRE(s->GetCharCodeAt(0)->Value() == int32_t(U'w'));
    }

    SECTION("strings can be created from combination of STL strings, literals and internal strings") {
        auto s = String::New(&heap, a, d, e, b, c, f);
        REQUIRE(s->GetLength() ==
                a.length() +
                b.length() +
                c.length() +
                std::string(d).length() +
                std::u16string(e).length() +
                std::u32string(f).length()
        );
//        REQUIRE(s->GetCharCodeAt(2)->Value() == int32_t(U'ф'));
    }

    SECTION("hashes of equal strings are the same") {
        auto s1 = String::New(&heap, a, b, c, d, e, f);
        auto s2 = String::New(&heap, a, b, c, d, e, f);
        REQUIRE(s1->GetHash() == s2->GetHash());
    }

    SECTION("strings can be compared for equality") {
        auto s1 = String::New(&heap, a, b, c, d, e, f);
        auto s2 = String::New(&heap, a, b, c, d, e, f);
        auto s3 = String::New(&heap, a, b, c, b, e, f);
        REQUIRE(*s1 == *s2);
        REQUIRE(*s1 != *s3);
        REQUIRE(*s2 != *s3);
    }

    SECTION("strings can be converted to std::string and to raw string") {
        auto s1 = String::New(&heap, a, r);
//        auto s2 = String::New(&heap, b, w);

        std::string raw_s1;
//        std::u32string raw_s2;
        s1->GetRawString().toUTF8String(raw_s1);
//        s2->GetRawString().toUTF8String(raw_s2);
        REQUIRE(raw_s1 == (a + r));
//        REQUIRE(raw_s2 == (b + w));
    }

    SECTION("std::hash and std::equal_to support strings") {
        auto s1 = String::New(&heap, "lol");
        auto s2 = String::New(&heap, "kek");
        auto s3 = String::New(&heap, "krokus");

        std::unordered_map<Handle<String>, int> map;
        map[s1] = 1234;
        map[s2] = 2345;
        map[s3] = 3456;
        REQUIRE(map[s1] == 1234);
        REQUIRE(map[s2] == 2345);
        REQUIRE(map[s3] == 3456);
    }

}