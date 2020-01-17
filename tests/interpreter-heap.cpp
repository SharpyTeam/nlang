//
// Created by ilya on 17.01.2020.
//

#include <heap.hpp>


#include <catch2/catch.hpp>

TEST_CASE( "heap allocates and deallocates stuff", "[vector]" ) {

    nlang::Heap heap;

    std::vector<nlang::ObjectSlot*> slots;


    REQUIRE( heap.GetOccupiedSlotsCount() == 0 );
    REQUIRE( heap.GetPagesCount() == 0 );

    SECTION( "storing object allocates a page and occupies a slot" ) {
        heap.Store((nlang::Address) 1234);

        REQUIRE( heap.GetOccupiedSlotsCount() == 1);
        REQUIRE( heap.GetPagesCount() > 0 );
    }
    SECTION( "storing more allocates more slots" ) {
        for (int i = 1; i < 151346; ++i) {
            slots.emplace_back(heap.Store(nlang::Address(i)));
        }

        REQUIRE( heap.GetOccupiedSlotsCount() == 151345);
        REQUIRE( heap.GetPagesCount() > 0 );
    }
    SECTION( "unused slots gets collected by GC" ) {
        for (int i = 1; i < 151346; ++i) {
            slots.emplace_back(heap.Store(nlang::Address(i)));
        }

        heap.PreGC();
        heap.PostGC();

        REQUIRE( heap.GetOccupiedSlotsCount() == 0 );
    }
}