#include <lstm/containers/list.hpp>

#ifdef NDEBUG
#undef NDEBUG
#include "debug_alloc.hpp"
#define NDEBUG
#else
#include "debug_alloc.hpp"
#endif
#include "simple_test.hpp"
#include "thread_manager.hpp"

static constexpr int iter_count = LSTM_TEST_INIT(5000, 1000);
static constexpr int loop_count = LSTM_TEST_INIT(200, 40);

int main()
{
    for (int loop = 0; loop < loop_count; ++loop) {
        lstm::list<int, debug_alloc<int>> ints;
        thread_manager manager;

        manager.queue_loop_n([&] { lstm::atomic([&] { ints.emplace_front(0); }); }, iter_count);
        manager.queue_loop_n([&] { ints.emplace_front(0); }, iter_count);
        manager.queue_loop_n([&] { ints.clear(); }, iter_count);
        manager.queue_loop_n([&] { lstm::atomic([&] { ints.clear(); }); }, iter_count);

        manager.run();
    }
    CHECK(debug_live_allocations<> == 0);

    return test_result();
}
