#ifndef INC_12_SINGLETON_SINGLETON_IMPL_H
#define INC_12_SINGLETON_SINGLETON_IMPL_H

#include <mutex>

class MySingletonMeyers {
private:
    int m_value;

    // Private constructor to prevent external instantiation
    MySingletonMeyers() = default;

public:
    // Meyers' Singleton, which leverages the static
    // local variable initialization guarantee in modern C++
    static MySingletonMeyers &instance() {
        static MySingletonMeyers instance;
        return instance;
    }

    [[nodiscard]]  int get_value() const {
        return m_value;
    }

    void set_value(int value) {
        m_value = value;
    }

    // Delete copy constructor & assignment operator
    MySingletonMeyers(const MySingletonMeyers &) = delete;

    MySingletonMeyers &operator=(const MySingletonMeyers &) = delete;
};

class MySingletonMutex {
private:
    int m_value;
    inline static std::mutex m_mutex;
    inline static MySingletonMutex *m_instance = nullptr;

    // Private constructor to prevent external instantiation
    MySingletonMutex() = default;

public:
    static MySingletonMutex &instance() {
        std::lock_guard<std::mutex> lg(m_mutex);
        if (!m_instance) {
            m_instance = new MySingletonMutex();
        }
        return *m_instance;
    }

    [[nodiscard]]  int get_value() const {
        return m_value;
    }

    void set_value(int value) {
        m_value = value;
    }

    // Delete copy constructor & assignment operator
    MySingletonMutex(const MySingletonMutex &) = delete;

    MySingletonMutex &operator=(const MySingletonMutex &) = delete;
};

class MySingletonCallOnce {
private:
    int m_value;
    inline static MySingletonCallOnce *m_instance;
    inline static std::once_flag m_flg;

    // Private constructor to prevent external instantiation
    MySingletonCallOnce() = default;

public:
    static MySingletonCallOnce &instance() {
        std::call_once(m_flg, []() {
            m_instance = new MySingletonCallOnce();
        });
        return *m_instance;
    }

    [[nodiscard]]  int get_value() const {
        return m_value;
    }

    void set_value(int value) {
        m_value = value;
    }

    // Delete copy constructor & assignment operator
    MySingletonCallOnce(const MySingletonCallOnce &) = delete;

    MySingletonCallOnce &operator=(const MySingletonCallOnce &) = delete;
};

// Since C++17 we can use inline keyword to define static members, well, inline.
//MySingletonMutex *MySingletonMutex::m_instance = nullptr;
//std::mutex MySingletonMutex::m_mutex;

//std::once_flag MySingletonCallOnce::m_flg;
//MySingletonCallOnce *MySingletonCallOnce::m_instance = nullptr;
#endif //INC_12_SINGLETON_SINGLETON_IMPL_H
