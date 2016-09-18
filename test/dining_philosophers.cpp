#include <lstm/lstm.hpp>

#include "simple_test.hpp"
#include "thread_manager.hpp"

using lstm::atomic;
using lstm::var;

static constexpr std::size_t food_size = 1000;
static constexpr std::size_t repeat_count = 300;

struct philosopher {
    std::size_t food{food_size};
};

struct fork {
    var<bool> in_use{false};
};

auto get_loop(philosopher& p, fork& f0, fork& f1) {
    return [&] {
        while (p.food != 0) {
            atomic([&](auto& tx) {
                if (!tx.load(f0.in_use) && !tx.load(f1.in_use)) {
                    tx.store(f0.in_use, true);
                    tx.store(f1.in_use, true);
                } else
                    lstm::retry();
            });
            
            --p.food;
            
            f0.in_use.unsafe() = false;
            f1.in_use.unsafe() = false;
        }
    };
}

int main() {
    thread_manager manager;
    for (std::size_t i = 0; i < repeat_count; ++i) {
        philosopher phil, sami, eric, aimy, joey;
        fork forks[5];
        
        manager.queue_thread(get_loop(phil, forks[0], forks[1]));
        manager.queue_thread(get_loop(sami, forks[1], forks[2]));
        manager.queue_thread(get_loop(eric, forks[2], forks[3]));
        manager.queue_thread(get_loop(aimy, forks[3], forks[4]));
        manager.queue_thread(get_loop(joey, forks[4], forks[0]));
            
        manager.run();
            
        for(auto& fork : forks)
            CHECK(fork.in_use.unsafe() == false);
        
        CHECK(phil.food == 0u);
        CHECK(sami.food == 0u);
        CHECK(eric.food == 0u);
        CHECK(aimy.food == 0u);
        CHECK(joey.food == 0u);
        
        // auto& log = lstm::detail::transaction_log::get();
        //
        // TODO: get failure count low on debug builds
        // CHECK(log.each_threads_successes_equals(food_size));
        // CHECK(log.total_successes() == food_size * log.thread_count());
        // CHECK(log.total_internal_failures() <= food_size * (log.thread_count() - 1u));
        //
        LSTM_LOG_DUMP();
        // assert(log.total_internal_failures() <= food_size * (log.thread_count() - 1));
        
        LSTM_LOG_CLEAR();
    }
    return test_result();
}