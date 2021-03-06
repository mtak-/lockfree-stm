#ifndef LSTM_DETAIL_GP_CALLBACK_HPP
#define LSTM_DETAIL_GP_CALLBACK_HPP

#include <lstm/detail/lstm_fwd.hpp>

LSTM_DETAIL_BEGIN
    struct gp_callback
    {
    private:
        static constexpr auto max_payload_size = sizeof(void*) * 3;

        using cb_payload_t = std::aligned_storage_t<max_payload_size, alignof(std::max_align_t)>;

        template<typename F>
        using sbo_concept = std::
            integral_constant<bool,
                              sizeof(uncvref<F>) <= max_payload_size
                                  && alignof(uncvref<F>) <= alignof(cb_payload_t)
                                  && std::is_trivially_destructible<uncvref<F>>{}
                                  && (std::is_trivially_move_constructible<uncvref<F>>{}
                                      || std::is_trivially_copy_constructible<uncvref<F>>{})>;

        cb_payload_t payload;
        void (*cb)(cb_payload_t&) noexcept;

        template<typename F>
        static void callback(cb_payload_t& payload) noexcept
        {
            static_assert(std::is_same<uncvref<F>, F>{}, "");
            (*reinterpret_cast<F*>(&payload))();
        }

        gp_callback() noexcept = default;

    public:
        template<typename F, LSTM_REQUIRES_(sbo_concept<F>() && callable<uncvref<F>>())>
        gp_callback(F&& f) noexcept(std::is_nothrow_constructible<uncvref<F>, F&&>{})
        {
            static_assert(noexcept(f()), "gp_callbacks must be noexcept");
            ::new (&payload) uncvref<F>((F &&) f);
            cb = &callback<uncvref<F>>;
        }

        void operator()() noexcept { cb(payload); }
    };
LSTM_DETAIL_END

#endif /* LSTM_DETAIL_GP_CALLBACK_HPP */