#include <catch2/catch.hpp>

#include <utils/nan_boxed_primitive.hpp>
#include <utils/traits.hpp>

#include <type_traits>

TEST_CASE("nan-boxing & fake nan-boxing test") {
    using namespace nlang;

    NanBoxedPrimitive nb_empty;
    NanBoxedPrimitive nb_null;
    nb_null.SetNull();
    NanBoxedPrimitive nb_false(false);
    NanBoxedPrimitive nb_true(true);
    NanBoxedPrimitive nb_number(123.456);
    NanBoxedPrimitive nb_number_neg(-456.124);
    NanBoxedPrimitive nb_int32(78910);
    NanBoxedPrimitive nb_int32_neg(-78913);
    NanBoxedPrimitive nb_pointer(new int(-111213));

    SECTION("create nan boxed primitives") {
        // Default-constructed NBP -> pointer (nullptr)
        REQUIRE(nb_empty.IsPointer());
        REQUIRE(nb_empty.GetPointer() == nullptr);
        REQUIRE(!nb_empty.IsNull());
        REQUIRE(!nb_empty.IsBool());
        REQUIRE(!nb_empty.IsFalse());
        REQUIRE(!nb_empty.IsTrue());
        REQUIRE(!nb_empty.IsNumber());
        REQUIRE(!nb_empty.IsInt32());

        // Null
        REQUIRE(nb_null.IsNull());
        REQUIRE(!nb_null.IsBool());
        REQUIRE(!nb_null.IsFalse());
        REQUIRE(!nb_null.IsTrue());
        REQUIRE(!nb_null.IsNumber());
        REQUIRE(!nb_null.IsInt32());
        REQUIRE(!nb_null.IsPointer());

        // False
        REQUIRE(nb_false.IsBool());
        REQUIRE(nb_false.IsFalse());
        REQUIRE(!nb_false.GetBool());
        REQUIRE(!nb_false.IsTrue());
        REQUIRE(!nb_false.IsNull());
        REQUIRE(!nb_false.IsNumber());
        REQUIRE(!nb_false.IsInt32());
        REQUIRE(!nb_false.IsPointer());

        // True
        REQUIRE(nb_true.IsBool());
        REQUIRE(nb_true.IsTrue());
        REQUIRE(nb_true.GetBool());
        REQUIRE(!nb_true.IsFalse());
        REQUIRE(!nb_true.IsNull());
        REQUIRE(!nb_true.IsNumber());
        REQUIRE(!nb_true.IsInt32());
        REQUIRE(!nb_true.IsPointer());

        // Number (double)
        REQUIRE(nb_number.IsNumber());
        REQUIRE(almost_equal(nb_number.GetNumber(), 123.456, 5));
        REQUIRE(!nb_number.IsNull());
        REQUIRE(!nb_number.IsBool());
        REQUIRE(!nb_number.IsFalse());
        REQUIRE(!nb_number.IsTrue());
        REQUIRE(!nb_number.IsInt32());
        REQUIRE(!nb_number.IsPointer());

        // Number (double, negative)
        REQUIRE(nb_number_neg.IsNumber());
        REQUIRE(almost_equal(nb_number_neg.GetNumber(), -456.124, 5));
        REQUIRE(!nb_number_neg.IsNull());
        REQUIRE(!nb_number_neg.IsBool());
        REQUIRE(!nb_number_neg.IsFalse());
        REQUIRE(!nb_number_neg.IsTrue());
        REQUIRE(!nb_number_neg.IsInt32());
        REQUIRE(!nb_number_neg.IsPointer());

        // Int32
        REQUIRE(nb_int32.IsInt32());
        REQUIRE(nb_int32.GetInt32() == 78910);
        REQUIRE(!nb_int32.IsNull());
        REQUIRE(!nb_int32.IsBool());
        REQUIRE(!nb_int32.IsFalse());
        REQUIRE(!nb_int32.IsTrue());
        REQUIRE(!nb_int32.IsNumber());
        REQUIRE(!nb_int32.IsPointer());

        // Int32 (negative)
        REQUIRE(nb_int32_neg.IsInt32());
        REQUIRE(nb_int32_neg.GetInt32() == -78913);
        REQUIRE(!nb_int32_neg.IsNull());
        REQUIRE(!nb_int32_neg.IsBool());
        REQUIRE(!nb_int32_neg.IsFalse());
        REQUIRE(!nb_int32_neg.IsTrue());
        REQUIRE(!nb_int32_neg.IsNumber());
        REQUIRE(!nb_int32_neg.IsPointer());

        // Pointer (not nullptr)
        REQUIRE(nb_pointer.IsPointer());
        REQUIRE(*((int *) nb_pointer.GetPointer()) == -111213);
        REQUIRE(!nb_pointer.IsNull());
        REQUIRE(!nb_pointer.IsBool());
        REQUIRE(!nb_pointer.IsFalse());
        REQUIRE(!nb_pointer.IsTrue());
        REQUIRE(!nb_pointer.IsNumber());
        REQUIRE(!nb_pointer.IsInt32());
    }

    // TODO: Add more tests for nan-boxed primitives

    SECTION("modify nan-boxed primitives") {

    }

    // TODO: Add tests for fake nan-boxed primitives

    SECTION("create fake nan-boxed primitives") {

    }

    SECTION("modify fake nan-boxed primitives") {

    }
}