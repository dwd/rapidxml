//
// Created by dave on 10/07/2024.
//

#ifndef RAPIDXML_RAPIDXML_WRAPPERS_HPP
#define RAPIDXML_RAPIDXML_WRAPPERS_HPP

#include <type_traits>
#include <numeric>
#include <stdexcept>

namespace flxml {
    // Most of rapidxml was written to use a NUL-terminated Ch * for parsing.
    // This utility struct wraps a buffer to provide something that
    // looks mostly like a pointer, deferencing to NUL when it hits the end.
    // It's also a forward_iterator, so it'll work with the rage type constructors for string{_view} etc.
    template<typename T>
    struct buffer_ptr {
        // Iterator magic typedefs
        using iterator_category = std::contiguous_iterator_tag;
        using difference_type = T::difference_type;
        using value_type = T::value_type;
        using pointer = T::const_pointer;
        using reference = T::const_reference;

        using real_it = T::const_iterator;
        real_it it;
        real_it end_it;
        static constexpr value_type end_char = value_type(0);

        explicit buffer_ptr(T const & buf) : it(buf.cbegin()), end_it(buf.cend()) {}
        buffer_ptr(buffer_ptr const & other) : it(other.it), end_it(other.end_it) {}
        buffer_ptr() = default;
        buffer_ptr & operator = (buffer_ptr const & other) {
            it = other.it;
            return *this;
        }
        reference validated_it(typename T::const_iterator const &it) const {
            if (it == end_it) return end_char;
            return *it;
        }
        reference operator[](int i) const {
            real_it it2 = it + i;
            if (it2 >= end_it) return end_char;
            return *it2;
        }
        pointer operator -> () const {
            if (it >= end_it) return &end_char;
            return &*it;
        }

        auto operator <=> (buffer_ptr other) const {
            return it <=> other.it;
        }
        auto operator < (buffer_ptr other) const {
            return it < other.it;
        }
        auto operator > (buffer_ptr other) const {
            return it > other.it;
        }
        auto operator >= (buffer_ptr other) const {
            return it >= other.it;
        }
        auto operator <= (buffer_ptr other) const {
            return it <= other.it;
        }

        buffer_ptr & operator ++() {
            ++it;
            return *this;
        }
        buffer_ptr operator ++(int) {
            auto old = *this;
            ++it;
            return old;
        }

        buffer_ptr & operator --() {
            --it;
            return *this;
        }
        buffer_ptr operator --(int) {
            auto old = *this;
            --it;
            return old;
        }

        reference operator *() const {
            return validated_it(it);
        }

        bool operator == (buffer_ptr const & other) const {
            return it == other.it;
        }

        auto operator + (difference_type n) const {
            buffer_ptr other(*this);
            other.it += n;
            return other;
        }
        buffer_ptr & operator += (difference_type i) {
            it += i;
            return *this;
        }

        auto operator - (difference_type n) const {
            buffer_ptr other(*this);
            other.it -= n;
            return other;
        }
        buffer_ptr & operator -= (difference_type i) {
            it -= i;
            return *this;
        }

        difference_type operator - (buffer_ptr const & other) const {
            return it - other.it;
        }

        pointer ptr() {
            return &*it;
        }
    };

    template<typename T>
    static auto operator + (int n, buffer_ptr<T> it) {
        it.it += n;
        return it;
    }

    class no_such_node : std::runtime_error {
    public:
        no_such_node() : std::runtime_error("No such node") {}
    };

    template<typename T>
    class optional_ptr {
        T * m_ptr;

        void assert_value() const {
            if (m_ptr == nullptr) {
                throw no_such_node();
            }
        }
    public:
        optional_ptr(std::nullptr_t) : m_ptr(nullptr) {}
        optional_ptr() : m_ptr(nullptr) {}
        optional_ptr(T * ptr) : m_ptr(ptr) {}

        bool has_value() const {
            return m_ptr != nullptr;
        }

        T & value() {
            assert_value();
            return *m_ptr;
        }
        T * get() {
            assert_value();
            return m_ptr;
        }
        T * operator -> () {
            return get();
        }
        T & operator * () {
            return value();
        }
        T * ptr_unsafe() {
            return m_ptr;
        }

        T const & value() const {
            assert_value();
            return *m_ptr;
        }
        T const * get() const {
            assert_value();
            return m_ptr;
        }
        T const * operator -> () const {
            return get();
        }
        T const & operator * () const {
            return value();
        }
        T const * ptr_unsafe() const {
            return m_ptr;
        }

        bool operator ! () const {
            return m_ptr == nullptr;
        }
        operator bool() const {
            return m_ptr != nullptr;
        }

        bool operator == (T * t) const {
            return m_ptr == t;
        }
        bool operator == (optional_ptr const & t) const {
            return m_ptr == t.m_ptr;
        }
    };
}

#endif //RAPIDXML_RAPIDXML_WRAPPERS_HPP
