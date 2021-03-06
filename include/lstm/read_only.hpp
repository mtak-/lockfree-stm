#ifndef LSTM_READ_ONLY_HPP
#define LSTM_READ_ONLY_HPP

#include <lstm/detail/atomic_base.hpp>

#include <lstm/read_transaction.hpp>

LSTM_DETAIL_BEGIN
    struct read_only_fn : private detail::atomic_base_fn
    {
    private:
        template<typename Func,
                 typename... Args,
                 LSTM_REQUIRES_(!is_void_transact_function<Func&, read_transaction, Args&&...>()),
                 typename Result = transact_result<Func, read_transaction, Args&&...>>
        static Result slow_path(thread_data& tls_td, Func func, Args&&... args)
        {
            while (true) {
                const epoch_t          version = default_domain().get_clock();
                const read_transaction tx{version};
                tls_td.access_lock(version);
                try {
                    LSTM_ASSERT(valid_start_state(tls_td));

                    Result result = atomic_base_fn::call(func, tx, (Args &&) args...);

                    tx_success<tx_kind::read_only>(tls_td, 0);
                    LSTM_ASSERT(valid_start_state(tls_td));
                    LSTM_ASSERT(!tls_td.in_critical_section());

                    if (std::is_reference<Result>{})
                        return static_cast<Result>(result);
                    else
                        return result;
                } catch (const tx_retry&) {
                    // nothing
                } catch (...) {
                    unhandled_exception<tx_kind::read_only>(tls_td);
                }
                tx_failure<tx_kind::read_only>(tls_td);
            }
        }

        template<typename Func,
                 typename... Args,
                 LSTM_REQUIRES_(is_void_transact_function<Func&, read_transaction, Args&&...>())>
        static void slow_path(thread_data& tls_td, Func func, Args&&... args)
        {
            while (true) {
                const epoch_t          version = default_domain().get_clock();
                const read_transaction tx{version};
                tls_td.access_lock(version);
                try {
                    LSTM_ASSERT(valid_start_state(tls_td));

                    atomic_base_fn::call(func, tx, (Args &&) args...);

                    tx_success<tx_kind::read_only>(tls_td, 0);
                    LSTM_ASSERT(valid_start_state(tls_td));
                    LSTM_ASSERT(!tls_td.in_critical_section());

                    return;
                } catch (const tx_retry&) {
                    // nothing
                } catch (...) {
                    unhandled_exception<tx_kind::read_only>(tls_td);
                }
                tx_failure<tx_kind::read_only>(tls_td);
            }
        }

    public:
        template<typename Func,
                 typename... Args,
                 LSTM_REQUIRES_(is_transact_function<Func&&, read_transaction, Args&&...>())>
        transact_result<Func, read_transaction, Args&&...>
        operator()(thread_data& tls_td, Func&& func, Args&&... args) const
        {
            switch (tls_td.tx_kind()) {
            case tx_kind::read_only:
                return atomic_base_fn::call((Func &&) func,
                                            read_transaction{tls_td.epoch()},
                                            (Args &&) args...);
            case tx_kind::read_write:
                return atomic_base_fn::call((Func &&) func,
                                            read_transaction{tls_td, tls_td.epoch()},
                                            (Args &&) args...);
            case tx_kind::none:
                set_read(tls_td);
                return read_only_fn::slow_path(tls_td, (Func &&) func, (Args &&) args...);
            }
        }

        template<typename Func,
                 typename... Args,
                 LSTM_REQUIRES_(is_transact_function<Func&&, read_transaction, Args&&...>())>
        transact_result<Func, read_transaction, Args&&...>
        operator()(Func&& func, Args&&... args) const
        {
            return (*this)(tls_thread_data(), (Func &&) func, (Args &&) args...);
        }

#ifndef LSTM_MAKE_SFINAE_FRIENDLY
        template<typename Func,
                 typename... Args,
                 LSTM_REQUIRES_(!is_transact_function<Func&&, read_transaction, Args&&...>())>
        transact_result<Func, read_transaction, Args&&...>
        operator()(thread_data&, Func&&, Args&&...) const
        {
            static_assert(is_transact_function_<Func&&, read_transaction, Args&&...>()
                              && is_transact_function_<uncvref<Func>&,
                                                       read_transaction,
                                                       Args&&...>(),
                          "functions passed to lstm::read_only must either take no parameters, "
                          "or take a `lstm::read_transaction` either by value or `const&`");
            static_assert(!is_nothrow_transact_function<Func&&, read_transaction, Args&&...>()
                              && !is_nothrow_transact_function<uncvref<Func>&,
                                                               read_transaction,
                                                               Args&&...>(),
                          "functions passed to lstm::read_only must not be marked noexcept");
        }

        template<typename Func,
                 typename... Args,
                 LSTM_REQUIRES_(!is_transact_function<Func&&, read_transaction, Args&&...>())>
        transact_result<Func, read_transaction, Args&&...> operator()(Func&&, Args&&...) const
        {
            static_assert(is_transact_function_<Func&&, read_transaction, Args&&...>()
                              && is_transact_function_<uncvref<Func>&,
                                                       read_transaction,
                                                       Args&&...>(),
                          "functions passed to lstm::read_only must either take no parameters, "
                          "or take a `lstm::read_transaction` either by value or `const&`");
            static_assert(!is_nothrow_transact_function<Func&&, read_transaction, Args&&...>()
                              && !is_nothrow_transact_function<uncvref<Func>&,
                                                               read_transaction,
                                                               Args&&...>(),
                          "functions passed to lstm::read_only must not be marked noexcept");
        }
#endif /* LSTM_MAKE_SFINAE_FRIENDLY */
    };
LSTM_DETAIL_END

LSTM_BEGIN
    namespace
    {
        constexpr auto& read_only = detail::static_const<detail::read_only_fn>;
    }
LSTM_END

#endif /* LSTM_READ_ONLY_HPP */