#ifndef LSTM_DETAIL_FAST_RW_MUTEX_HPP
#define LSTM_DETAIL_FAST_RW_MUTEX_HPP

#include <lstm/detail/exponential_delay.hpp>

LSTM_DETAIL_BEGIN
    // this class is probly best for use in wait-free write side algorithms
    struct fast_rw_mutex {
    private:
        static constexpr uword write_bit = uword(1) << (sizeof(uword) * 8 - 1);
        std::atomic<uword> read_count{0};
        
    public:
        // TODO: compare_exchange_weak is overkill?
        void lock_shared() noexcept {
            uword read_state = read_count.fetch_add(1, LSTM_ACQUIRE);
            if (read_state & write_bit) {
                exponential_delay<100000, 10000000> exp_delay;
                
                read_count.fetch_sub(1, LSTM_RELAXED);
                do {
                    exp_delay();
                    read_state = 0;
                    while (!(read_state & write_bit)
                        && !read_count.compare_exchange_weak(read_state,
                                                             read_state + 1,
                                                             LSTM_RELAXED,
                                                             LSTM_RELAXED));
                    
                } while (read_state & write_bit);
                std::atomic_thread_fence(LSTM_ACQUIRE);
            }
        }
        
        void unlock_shared() noexcept { read_count.fetch_sub(1, LSTM_RELEASE); }
        
        void lock() noexcept {
            uword prev_read_count;
            exponential_delay<100000, 10000000> exp_delay;
            
            while ((prev_read_count = read_count.fetch_or(write_bit, LSTM_RELAXED)) & write_bit)
                exp_delay();
            
            exp_delay.reset();
            
            while (prev_read_count & ~write_bit) {
                exp_delay();
                prev_read_count = read_count.load(LSTM_RELAXED);
            }
            
            std::atomic_thread_fence(LSTM_ACQUIRE);
        }
        void unlock() noexcept { read_count.store(0, LSTM_RELEASE); }
    };
LSTM_DETAIL_END

#endif /* LSTM_DETAIL_FAST_RW_MUTEX_HPP */
