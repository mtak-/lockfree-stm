#ifndef LSTM_DETAIL_WRITE_SET_LOOKUP_HPP
#define LSTM_DETAIL_WRITE_SET_LOOKUP_HPP

#include <lstm/detail/lstm_fwd.hpp>

LSTM_DETAIL_BEGIN
    struct write_set_lookup
    {
    private:
        hash_t       hash_;
        var_storage* pending_write_;

    public:
        inline constexpr write_set_lookup(var_storage* const in_pending_write) noexcept
            : hash_(0)
            , pending_write_(in_pending_write)
        {
            LSTM_ASSERT(pending_write_ != nullptr);
        }

        inline write_set_lookup(const hash_t in_hash) noexcept
            : hash_(in_hash)
        {
            LSTM_ASSERT(hash_ != 0);
        }

        inline constexpr bool success() const noexcept { return !hash_; }

        inline constexpr var_storage& pending_write() const noexcept
        {
            LSTM_ASSERT(success());
            return *pending_write_;
        }

        inline constexpr hash_t hash() const noexcept
        {
            LSTM_ASSERT(!success());
            return hash_;
        }
    };
LSTM_DETAIL_END

#endif /* LSTM_DETAIL_WRITE_SET_LOOKUP_HPP */