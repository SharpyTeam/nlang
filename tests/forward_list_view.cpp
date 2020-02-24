#include <catch2/catch.hpp>

#include <utils/forward_list_view.hpp>

namespace {

struct Node : public nlang::utils::ForwardListView<Node>::NodeHeader {
    int value;

    Node(int value) : value(value) {}

    bool operator<(const Node& other) const {
        return value < other.value;
    }

    bool operator==(const Node& other) const {
        return value == other.value;
    }
};

std::pair<nlang::utils::ForwardListView<Node>, std::vector<Node>> Construct(const std::vector<int>& values) {
    nlang::utils::ForwardListView<Node> list;
    std::vector<Node> holder;
    holder.reserve(values.size());
    for (auto it = values.rbegin(); it != values.rend(); ++it) {
        holder.emplace_back(*it);
        list.push_front(holder.back());
    }
    return std::pair(list, std::move(holder));
}

bool Compare(const nlang::utils::ForwardListView<Node>& list, const std::vector<int>& values) {
    if (list.size() != values.size()) return false;
    auto it = values.begin();
    for (Node& node : list) {
        if (node.value != *it) {
            return false;
        }
        ++it;
    }
    return true;
}

}

TEST_CASE("forward list view functionality") {
    using namespace nlang::utils;

    auto [list, holder] = Construct({ 1, 9, 6, 2, 6, 4, 8 });
    auto [list2, holder2] = Construct({ 1, 5, 123 });

    REQUIRE(Compare(list, { 1, 9, 6, 2, 6, 4, 8 }));

    SECTION("sort") {
        list.sort();
        REQUIRE(Compare(list, { 1, 2, 4, 6, 6, 8, 9 }));
    }

    SECTION("reverse") {
        list.reverse();
        REQUIRE(Compare(list, { 8, 4, 6, 2, 6, 9, 1 }));
    }

    SECTION("unique") {
        list.sort();
        list.unique();
        REQUIRE(Compare(list, { 1, 2, 4, 6, 8, 9 }));
    }

    SECTION("remove") {
        list.remove(*new Node(2));
        REQUIRE(Compare(list, { 1, 9, 6, 6, 4, 8 }));
    }

    SECTION("splice_after") {
        list.splice_after(std::next(list.cbegin()), list2, list2.cbegin(), list2.cend());
        REQUIRE(Compare(list, { 1, 9, 5, 123, 6, 2, 6, 4, 8 }));
        REQUIRE(Compare(list2, { 1 }));
    }

    SECTION("merge") {
        list.sort();
        list2.sort();
        list.merge(list2);
        REQUIRE(Compare(list, { 1, 1, 2, 4, 5, 6, 6, 8, 9, 123 }));
        REQUIRE(Compare(list2, { }));
    }

    SECTION("resize") {
        list.resize(0);
        REQUIRE(list.size() == 0);
        REQUIRE(list.empty());
    }
}