#include <lstm/lstm.hpp>

#include "simple_test.hpp"
#include "thread_manager.hpp"

#include <random>

static constexpr auto loop_count0 = LSTM_TEST_INIT(5000000, 50000);

using lstm::uword;

int main() {
    thread_manager manager;
    
    for (int j = 0; j < 5; ++j) {
        manager.queue_thread([] {
            auto& tls_td = lstm::detail::tls_thread_data();
            for (uword i = 0; i < loop_count0; ++i) {
                tls_td.access_lock();
                tls_td.access_unlock();
                lstm::detail::synchronize(tls_td.mut);
            }
        });
    }
    
    manager.run();
    
    return test_result();
}
