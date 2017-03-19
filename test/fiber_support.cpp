#define LSTM_USE_BOOST_FIBERS
#include <lstm/lstm.hpp>

#include "simple_test.hpp"
#include "thread_manager.hpp"

using lstm::atomic;
using lstm::var;

static constexpr std::size_t food_size    = LSTM_TEST_INIT(300 * 10000, 10000);
static constexpr std::size_t repeat_count = LSTM_TEST_INIT(1, 30);

struct LSTM_CACHE_ALIGNED philosopher
{
    LSTM_CACHE_ALIGNED std::size_t food{food_size};
};

struct fork
{
    LSTM_CACHE_ALIGNED var<bool> in_use{false};
};

static auto get_loop(philosopher& p, struct fork& f0, struct fork& f1)
{
    return [&] {
        while (p.food != 0) {
            atomic([&](const auto tx) {
                if (!f0.in_use.untracked_get(tx) && !f1.in_use.untracked_get(tx)) {
                    f0.in_use.set(tx, true);
                    f1.in_use.set(tx, true);
                } else
                    lstm::retry();
            });

            --p.food;

            std::atomic_thread_fence(std::memory_order_release);

            f0.in_use.unsafe_set(false);
            f1.in_use.unsafe_set(false);
        }
    };
}

int main()
{
    for (std::size_t i = 0; i < repeat_count; ++i) {
        thread_manager manager;

        philosopher phils[5];
        struct fork forks[5];

        for (int i = 0; i < 5; ++i)
            manager.queue_thread(get_loop(phils[i], forks[i], forks[(i + 1) % 5]));

        manager.run();

        for (auto& fork : forks)
            CHECK(fork.in_use.unsafe_get() == false);

        for (auto& phil : phils)
            CHECK(phil.food == 0u);
    }
    return test_result();
}