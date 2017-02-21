#ifndef LSTM_DETAIL_POD_VECTOR_HPP
#define LSTM_DETAIL_POD_VECTOR_HPP

#include <lstm/detail/pod_mallocator.hpp>

#include <cassert>

LSTM_DETAIL_BEGIN
    // this class never calls construct/destroy... there's no need for POD types
    // if an allocator "requires" construct/destroy to be called, it will be in for
    // a surprise
    template<typename T, typename Alloc = pod_mallocator<T>>
    struct pod_vector : private Alloc
    {
        using allocator_type  = Alloc;
        using value_type      = T;
        using reference       = T&;
        using const_reference = const T&;
        using pointer         = T*;
        using const_pointer   = const T*;
        using iterator        = T*;
        using const_iterator  = const T*;

    private:
        static_assert(std::is_pod<value_type>{}, "only works with POD types");
        static_assert(std::is_same<value_type, typename allocator_type::value_type>{}, "");
        static constexpr bool has_noexcept_alloc
            = noexcept(std::declval<Alloc&>().allocate(std::declval<std::size_t>()));

        iterator begin_;
        iterator end_;
        iterator buffer_end_;

        using alloc_traits = std::allocator_traits<allocator_type>;
        static_assert(std::is_pointer<typename alloc_traits::pointer>{},
                      "sorry, lstm currently only supports allocators that return "
                      "raw pointers");

        inline allocator_type& alloc() noexcept { return *this; }

        LSTM_NOINLINE void reserve_more() noexcept(has_noexcept_alloc)
        {
            assert((capacity() << 1) > size()); // zomg big transaction
            const pointer new_begin = alloc().allocate(capacity() << 1);
            assert(new_begin);

            std::memcpy(new_begin, begin_, sizeof(value_type) * size());

            alloc().deallocate(begin_, capacity());

            buffer_end_ = new_begin + (capacity() << 1);
            end_        = new_begin + size();
            begin_      = new_begin;
        }

    public:
        pod_vector(const allocator_type& alloc = {}) noexcept(has_noexcept_alloc)
            : allocator_type(alloc)
            , begin_(this->alloc().allocate(1))
            , end_(begin_)
            , buffer_end_(begin_ + 1)
        {
        }

        pod_vector(const pod_vector&) = delete;
        pod_vector& operator=(const pod_vector&) = delete;

        ~pod_vector() noexcept
        {
            assert(empty());
            alloc().deallocate(begin_, capacity());
        }

        bool  empty() const noexcept { return end_ == begin_; }
        uword size() const noexcept { return end_ - begin_; }
        uword capacity() const noexcept { return buffer_end_ - begin_; }

        template<typename... Us>
        void emplace_back(Us&&... us) noexcept(has_noexcept_alloc)
        {
            ::new (end_++) value_type((Us &&) us...);
            if (LSTM_UNLIKELY(end_ == buffer_end_))
                reserve_more();
        }

        void unordered_erase(const pointer ptr) noexcept
        {
            assert(ptr >= begin_ && ptr < end_);
            std::memmove(ptr, --end_, sizeof(value_type));
        }

        void unordered_erase(const const_pointer ptr) noexcept
        {
            assert(ptr >= begin_ && ptr < end_);
            std::memmove((void*)ptr, --end_, sizeof(value_type));
        }

        void clear() noexcept { end_ = begin_; }

        void shrink_to_fit() noexcept(has_noexcept_alloc)
        {
            const uword new_capacity = size() + 1;
            if (new_capacity != capacity()) {
                const pointer new_begin = alloc().allocate(new_capacity);
                assert(new_begin);

                std::memcpy(new_begin, begin_, sizeof(value_type) * size());

                alloc().deallocate(begin_, capacity());

                buffer_end_ = new_begin + new_capacity;
                end_        = new_begin + size();
                begin_      = new_begin;
            }
        }

        iterator       begin() noexcept { return begin_; }
        iterator       end() noexcept { return end_; }
        const_iterator begin() const noexcept { return begin_; }
        const_iterator end() const noexcept { return end_; }
    };
LSTM_DETAIL_END

#endif /* LSTM_DETAIL_POD_VECTOR_HPP */