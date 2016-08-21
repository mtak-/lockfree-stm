#include <lstm/var.hpp>

#include "simple_test.hpp"

#include <scoped_allocator>
#include <vector>

using lstm::var;

int main() {
    // value tests
    {
        var<int> x;
        x.unsafe() = 42;

        NOEXCEPT_CHECK(x.unsafe() == 42);
    }
    {
        var<int> x{42};
        NOEXCEPT_CHECK(x.unsafe() == 42);

        x.unsafe() = 43;
        static_assert(Same<int&, decltype(x.unsafe())>, "");
        NOEXCEPT_CHECK(x.unsafe() == 43);

        static_assert(Same<int&&, decltype(std::move(x).unsafe())>, "");
    }
    {
        const var<int> x{42};
        NOEXCEPT_CHECK(x.unsafe() == 42);

        static_assert(Same<const int&, decltype(x.unsafe())>, "");
        static_assert(Same<const int&&, decltype(std::move(x).unsafe())>, "");
    }
    {
        var<const int> x{42};
        NOEXCEPT_CHECK(x.unsafe() == 42);

        static_assert(Same<const int&, decltype(x.unsafe())>, "");
        static_assert(Same<const int&&, decltype(std::move(x).unsafe())>, "");
    }
    {
        const var<const int> x{42};
        NOEXCEPT_CHECK(x.unsafe() == 42);

        static_assert(Same<const int&, decltype(x.unsafe())>, "");
        static_assert(Same<const int&&, decltype(std::move(x).unsafe())>, "");
    }

    // reference tests
    // {
    //     struct foo {
    //         char bar[30];
    //     };
    //     foo y; y.bar[0] = 42;
    //     var<foo&> x{y};
    //     NOEXCEPT_CHECK(x.unsafe().bar[0] == 42);
    //
    //     NOEXCEPT_CHECK(++x.unsafe().bar[0] == 43);
    //     NOEXCEPT_CHECK(y.bar[0] == 43);
    //
    //     static_assert(Same<foo&, decltype(x.unsafe())>, "");
    //     static_assert(Same<foo&, decltype(std::move(x).unsafe())>, "");
    // }
    // {
    //     struct foo {
    //         char bar[30];
    //     };
    //     foo y; y.bar[0] = 42;
    //     const var<foo&> x{y};
    //     NOEXCEPT_CHECK(x.unsafe().bar[0] == 42);
    //
    //     NOEXCEPT_CHECK(++x.unsafe().bar[0] == 43);
    //     NOEXCEPT_CHECK(y.bar[0] == 43);
    //
    //     static_assert(Same<foo&, decltype(x.unsafe())>, "");
    //     static_assert(Same<foo&, decltype(std::move(x).unsafe())>, "");
    // }
    // {
    //     struct foo {
    //         char bar[30];
    //     };
    //     foo y; y.bar[0] = 42;
    //     var<const foo&> x{y};
    //     NOEXCEPT_CHECK(x.unsafe().bar[0] == 42);
    //
    //     NOEXCEPT_CHECK(++y.bar[0] == 43);
    //     NOEXCEPT_CHECK(x.unsafe().bar[0] == 43);
    //
    //     static_assert(Same<const foo&, decltype(x.unsafe())>, "");
    //     static_assert(Same<const foo&, decltype(std::move(x).unsafe())>, "");
    // }
    // {
    //     struct foo {
    //         char bar[30];
    //     };
    //     foo y; y.bar[0] = 42;
    //     const var<const foo&> x{y};
    //     NOEXCEPT_CHECK(x.unsafe().bar[0] == 42);
    //
    //     NOEXCEPT_CHECK(++y.bar[0] == 43);
    //     NOEXCEPT_CHECK(x.unsafe().bar[0] == 43);
    //
    //     static_assert(Same<const foo&, decltype(x.unsafe())>, "");
    //     static_assert(Same<const foo&, decltype(std::move(x).unsafe())>, "");
    // }

    // allocators
    {
        var<std::vector<int>, std::scoped_allocator_adaptor<std::allocator<std::vector<int>>>> x;
        x.unsafe().push_back(42);

        CHECK(x.unsafe().size() == 1u);
        CHECK(x.unsafe()[0] == 42);
    }
    {
        using alloc = std::scoped_allocator_adaptor<std::allocator<std::vector<int>>>;
        var<std::vector<int>, alloc> x{alloc{}};
        x.unsafe().push_back(42);

        CHECK(x.unsafe().size() == 1u);
        CHECK(x.unsafe()[0] == 42);
    }
    {
        using alloc = std::scoped_allocator_adaptor<std::allocator<std::vector<int>>>;
        var<std::vector<int>, alloc> x{alloc{}, std::vector<int>{0,1,2,3}};
        x.unsafe().push_back(42);

        CHECK(x.unsafe().size() == 5u);
        CHECK(x.unsafe()[0] == 0);
        CHECK(x.unsafe()[1] == 1);
        CHECK(x.unsafe()[2] == 2);
        CHECK(x.unsafe()[3] == 3);
        CHECK(x.unsafe()[4] == 42);
    }

    // non const
    static_assert(std::is_default_constructible<lstm::var<int>>{}, "");
    static_assert(std::is_constructible<lstm::var<int>>{}, "");
    static_assert(std::is_constructible<lstm::var<int>, int&>{}, "");
    static_assert(std::is_constructible<lstm::var<int>, int&&>{}, "");
    static_assert(std::is_constructible<lstm::var<int>, const int&>{}, "");
    static_assert(std::is_constructible<lstm::var<int>, const int&&>{}, "");
    static_assert(std::is_nothrow_default_constructible<lstm::var<int>>{}, "");
    static_assert(std::is_nothrow_constructible<lstm::var<int>>{}, "");
    static_assert(std::is_nothrow_constructible<lstm::var<int>, int&>{}, "");
    static_assert(std::is_nothrow_constructible<lstm::var<int>, int&&>{}, "");
    static_assert(std::is_nothrow_constructible<lstm::var<int>, const int&>{}, "");
    static_assert(std::is_nothrow_constructible<lstm::var<int>, const int&&>{}, "");
    static_assert(!std::is_copy_constructible<lstm::var<int>>{}, "");
    static_assert(!std::is_move_constructible<lstm::var<int>>{}, "");
    static_assert(std::is_destructible<lstm::var<int>>{}, "");

    // const
    // static_assert(!std::is_default_constructible<lstm::var<const int>>{}, "");
    // static_assert(!std::is_constructible<lstm::var<const int>>{}, "");
    // static_assert(!std::is_nothrow_constructible<lstm::var<const int>>{}, "");
    static_assert(std::is_constructible<lstm::var<const int>, int&>{}, "");
    static_assert(std::is_constructible<lstm::var<const int>, int&&>{}, "");
    static_assert(std::is_constructible<lstm::var<const int>, const int&>{}, "");
    static_assert(std::is_constructible<lstm::var<const int>, const int&&>{}, "");
    static_assert(std::is_nothrow_constructible<lstm::var<const int>, int&>{}, "");
    static_assert(std::is_nothrow_constructible<lstm::var<const int>, int&&>{}, "");
    static_assert(std::is_nothrow_constructible<lstm::var<const int>, const int&>{}, "");
    static_assert(std::is_nothrow_constructible<lstm::var<const int>, const int&&>{}, "");
    static_assert(!std::is_copy_constructible<lstm::var<const int>>{}, "");
    static_assert(!std::is_move_constructible<lstm::var<const int>>{}, "");
    static_assert(std::is_destructible<lstm::var<const int>>{}, "");

    // ref
    // static_assert(std::is_constructible<lstm::var<int&>, int&>{}, "");
    // static_assert(!std::is_constructible<lstm::var<int&>, const int&>{}, "");
    // static_assert(std::is_constructible<lstm::var<const int&>, int&>{}, "");
    // static_assert(std::is_constructible<lstm::var<const int&>, const int&>{}, "");
    // static_assert(!std::is_constructible<lstm::var<int&&>, int&&>{}, "");

    return test_result();
}