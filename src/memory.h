#pragma once

#include "__stl__.h"

namespace pkpy{
    template <typename T>
    class shared_ptr {
        int* counter = nullptr;

#define _t() ((T*)(counter + 1))
#define _inc_counter() if(counter) ++(*counter)
#define _dec_counter() if(counter && --(*counter) == 0){ _t()->~T(); free(counter); }

    public:
        shared_ptr() {}
        shared_ptr(int* block) : counter(block) {}
        shared_ptr(const shared_ptr& other) : counter(other.counter) {
            _inc_counter();
        }
        shared_ptr(shared_ptr&& other) noexcept : counter(other.counter) {
            other.counter = nullptr;
        }
        ~shared_ptr() {
            _dec_counter();
        }

        bool operator==(const shared_ptr& other) const {
            return counter == other.counter;
        }

        bool operator!=(const shared_ptr& other) const {
            return counter != other.counter;
        }

        bool operator==(std::nullptr_t) const {
            return counter == nullptr;
        }

        bool operator!=(std::nullptr_t) const {
            return counter != nullptr;
        }

        shared_ptr& operator=(const shared_ptr& other) {
            if (this != &other) {
                _dec_counter();
                counter = other.counter;
                _inc_counter();
            }
            return *this;
        }

        shared_ptr& operator=(shared_ptr&& other) noexcept {
            if (this != &other) {
                _dec_counter();
                counter = other.counter;
                other.counter = nullptr;
            }
            return *this;
        }

        T& operator*() const {
            return *_t();
        }
        T* operator->() const {
            return _t();
        }
        T* get() const {
            return _t();
        }
        int use_count() const {
            return counter ? *counter : 0;
        }
        void reset(){
            _dec_counter();
            counter = nullptr;
        }
    };

#undef _t
#undef _inc_counter
#undef _dec_counter

    template <typename T, typename U, typename... Args>
    shared_ptr<T> make_shared(Args&&... args) {
        static_assert(std::is_base_of<T, U>::value, "U must be derived from T");
        int* p = (int*)malloc(sizeof(int) + sizeof(U));
        *p = 1;
        new(p+1) U(std::forward<Args>(args)...);
        return shared_ptr<T>(p);
    }

    template <typename T, typename... Args>
    shared_ptr<T> make_shared(Args&&... args) {
        int* p = (int*)malloc(sizeof(int) + sizeof(T));
        *p = 1;
        new(p+1) T(std::forward<Args>(args)...);
        return shared_ptr<T>(p);
    }

    template <typename T>
    class unique_ptr {
        T* ptr;

    public:
        unique_ptr() : ptr(nullptr) {}
        unique_ptr(T* ptr) : ptr(ptr) {}
        unique_ptr(const unique_ptr& other) = delete;
        unique_ptr(unique_ptr&& other) : ptr(other.ptr) {
            other.ptr = nullptr;
        }
        ~unique_ptr() {
            delete ptr;
        }

        bool operator==(const unique_ptr& other) const {
            return ptr == other.ptr;
        }

        bool operator!=(const unique_ptr& other) const {
            return ptr != other.ptr;
        }

        bool operator==(std::nullptr_t) const {
            return ptr == nullptr;
        }

        bool operator!=(std::nullptr_t) const {
            return ptr != nullptr;
        }

        unique_ptr& operator=(const unique_ptr& other) = delete;

        unique_ptr& operator=(unique_ptr&& other) {
            if (this != &other) {
                delete ptr;
                ptr = other.ptr;
                other.ptr = nullptr;
            }
            return *this;
        }

        T& operator*() const {
            return *ptr;
        }
        T* operator->() const {
            return ptr;
        }
        T* get() const {
            return ptr;
        }

        void reset(){
            delete ptr;
            ptr = nullptr;
        }
    };

    template <typename T, typename... Args>
    unique_ptr<T> make_unique(Args&&... args) {
        return unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
};


namespace pkpy{
    class scope_guard {
        std::function<void()> cleanup;
        bool dismissed = false;
    public:
        void dismiss() { dismissed = true; }
        scope_guard(std::function<void()> cleanup) : cleanup(cleanup) {}
        ~scope_guard() { if (!dismissed) cleanup(); }
    };
};