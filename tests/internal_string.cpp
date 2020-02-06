#include <catch2/catch.hpp>
#include <interpreter/value.hpp>
#include <interpreter/string.hpp>
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

    heap.RegisterDeleterForType(Value::Type::STRING, [](HeapValue* value) {
        delete static_cast<String*>(value);
    });

    SECTION("strings can be created from STL strings") {
        Handle<String> s = String::New(heap, a, b, c);
        REQUIRE(s->GetLength() == a.length() + b.length() + c.length());
//        REQUIRE(s->GetCharCodeAt(0)->Value() == int32_t(U'w'));
    }

    SECTION("strings can be created from combination of STL strings, literals and internal strings") {
        Handle<String> s = String::New(heap, a, d, e, b, c, f);
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
        Handle<String> s1 = String::New(heap, a, b, c, d, e, f);
        Handle<String> s2 = String::New(heap, a, b, c, d, e, f);
        REQUIRE(s1->GetHash() == s2->GetHash());
    }

    SECTION("strings can be compared for equality") {
        Handle<String> s1 = String::New(heap, a, b, c, d, e, f);
        Handle<String> s2 = String::New(heap, a, b, c, d, e, f);
        Handle<String> s3 = String::New(heap, a, b, c, b, e, f);
        REQUIRE(*s1 == *s2);
        REQUIRE(*s1 != *s3);
        REQUIRE(*s2 != *s3);
    }

    SECTION("strings can be converted to std::string and to raw string") {
        Handle<String> s1 = String::New(heap, a, r);
        Handle<String> s2 = String::New(heap, b, w);

        REQUIRE(s1->AsStdString() == (a + r));
        REQUIRE(s2->GetRawString() == (b + w));
    }


}