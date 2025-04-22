//
// Created by dave on 29/07/2024.
//

#ifndef RAPIDXML_RAPIDXML_GENERATOR_HPP
#define RAPIDXML_RAPIDXML_GENERATOR_HPP

#include <coroutine>
#include <iterator>

namespace flxml {
    template<typename T>
    class generator {
    public:
        using value_pointer = std::remove_reference<T>::type *;
        struct handle_type;
        struct promise_type {
            value_pointer value;

            std::suspend_always yield_value(T & v) {
                value = &v;
                return {};
            }

            std::suspend_never initial_suspend() {
                return {};
            }

            std::suspend_always final_suspend() noexcept {
                return {}; // Change this to std::suspend_always
            }

            void return_void() {}

            void unhandled_exception() {
                std::terminate();
            }

            generator get_return_object() {
                return generator{handle_type{handle_type::from_promise(*this)}};
            }
        };

        struct handle_type : std::coroutine_handle<promise_type> {
            explicit handle_type(std::coroutine_handle<promise_type> && h) : std::coroutine_handle<promise_type>(std::move(h)) {}

            T &operator*() {
                return *(this->promise().value);
            }

            void operator++() {
                this->resume();
            }

            bool operator!=(std::default_sentinel_t) const {
                return !this->done();
            }
        };

        explicit generator(handle_type h) : m_handle(h) {}

        ~generator() {
            if (m_handle)
                m_handle.destroy();
        }

        handle_type begin() {
            return m_handle;
        }

        std::default_sentinel_t end() {
            return std::default_sentinel;
        }

    private:
        handle_type m_handle{};
    };
}

#endif //RAPIDXML_RAPIDXML_GENERATOR_HPP
