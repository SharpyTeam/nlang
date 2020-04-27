#include <catch2/catch.hpp>

#include <interpreter/heap.hpp>
#include <interpreter/object.hpp>
#include <interpreter/value.hpp>

TEST_CASE("class and object creation") {
    using namespace nlang;

    Heap heap;

    auto class_name = String::New(&heap, "test");

    auto class_handle = Class::New(heap, class_name);
    REQUIRE(class_handle.Is<HeapValue>());
    REQUIRE(class_handle.Is<Class>());

    REQUIRE(class_handle->GetFieldCount() == 0);
    REQUIRE(class_handle->GetMethodCount() == 0);


    class_handle->AddField(String::New(&heap, "the_field"));
    REQUIRE(class_handle->GetFieldCount() == 1);

    auto object_handle = Object::New(heap, class_handle);
    REQUIRE(object_handle.Is<HeapValue>());
    REQUIRE(object_handle.Is<Object>());

    REQUIRE(object_handle->GetFieldCount() == 1);
    //REQUIRE(object_handle->GetFieldByName(String::New(heap, "the_field")).As<Number>()->IsNull());

    //object_handle->GetFieldByName(String::New(heap, "the_field")) = Number::New(1234);
    //REQUIRE(object_handle->GetFieldByName(String::New(heap, "the_field")).As<Number>()->Value() == 1234);
    //REQUIRE(object_handle->GetField(field).As<Number>()->Value() == 1234);
}

