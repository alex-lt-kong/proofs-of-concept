#include "singleton-impl.h"

#include <gtest/gtest.h>
#include <random>


TEST(SingletonTest, SimpleMeyers) {
    MySingletonMeyers &singleton0 = MySingletonMeyers::instance();
    MySingletonMeyers &singleton1 = MySingletonMeyers::instance();
    std::vector<int> values = {2, 1, 4, 7, 4, 8, 3, 6, 4, 7};
    for (const auto &value: values) {
        singleton0.set_value(value);
        EXPECT_EQ(singleton1.get_value(), value);
    }
}

TEST(SingletonTest, SimpleMutex) {
    MySingletonMutex &singleton0 = MySingletonMutex::instance();
    MySingletonMutex &singleton1 = MySingletonMutex::instance();
    std::vector<int> values = {2, 1, 4, 7, 4, 8, 3, 6, 4, 7};
    for (const auto &value: values) {
        singleton0.set_value(value);
        EXPECT_EQ(singleton1.get_value(), value);
    }
}

TEST(SingletonTest, SimpleCallOnce) {
    MySingletonCallOnce &singleton0 = MySingletonCallOnce::instance();
    MySingletonCallOnce &singleton1 = MySingletonCallOnce::instance();
    std::vector<int> values = {2, 1, 4, 7, 4, 8, 3, 6, 4, 7};
    for (const auto &value: values) {
        singleton0.set_value(value);
        EXPECT_EQ(singleton1.get_value(), value);
    }
}


TEST(SingletonTest, ManyMeyers) {
    std::vector<MySingletonMeyers *> mySingletons;
    for (int i = 0; i < 10; ++i) {
        mySingletons.push_back(&MySingletonMeyers::instance());
    }
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(INT_MIN, INT_MAX);
    std::vector<int> values;
    for (int i = 0; i < 1'000'000; ++i) {
        values.push_back(dist(gen));
    }
    for (const auto &value: values) {
        mySingletons[dist(gen) % mySingletons.size()]->set_value(value);
        EXPECT_EQ(mySingletons[dist(gen) % mySingletons.size()]->get_value(),
                  value);
    }
}

TEST(SingletonTest, ManyMutex) {
    std::vector<MySingletonMutex *> mySingletons;
    for (int i = 0; i < 10; ++i) {
        mySingletons.push_back(&MySingletonMutex::instance());
    }
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(INT_MIN, INT_MAX);
    std::vector<int> values;
    for (int i = 0; i < 1'000'000; ++i) {
        values.push_back(dist(gen));
    }
    for (const auto &value: values) {
        mySingletons[dist(gen) % mySingletons.size()]->set_value(value);
        EXPECT_EQ(mySingletons[dist(gen) % mySingletons.size()]->get_value(),
                  value);
    }
}

TEST(SingletonTest, ManyCallOnce) {
    std::vector<MySingletonCallOnce *> mySingletons;
    for (int i = 0; i < 10; ++i) {
        mySingletons.push_back(&MySingletonCallOnce::instance());
    }
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(INT_MIN, INT_MAX);
    std::vector<int> values;
    for (int i = 0; i < 1'000'000; ++i) {
        values.push_back(dist(gen));
    }
    for (const auto &value: values) {
        mySingletons[dist(gen) % mySingletons.size()]->set_value(value);
        EXPECT_EQ(mySingletons[dist(gen) % mySingletons.size()]->get_value(),
                  value);
    }
}
