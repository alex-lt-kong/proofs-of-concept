#ifndef ORDER_H
#define ORDER_H

#include <iostream>

enum struct Side : char { Bid = 0, Ask = 1 };

struct LimitOrder {
    int id = 0;
    int64_t timestamp = 0;
    Side side = Side::Bid;
    int price = 0;
    int size = 0;

    [[nodiscard]] int volume() const { return price * size; }

    LimitOrder(const int price) : price(price) {}

    // Custom three-way comparison
    auto operator<=>(const LimitOrder &other) const {
        return price <=> other.price;
    }

    bool operator==(const LimitOrder &other) const {
        return price == other.price;
    }
};

// Overload operator<< to output the price
std::ostream &operator<<(std::ostream &os, const LimitOrder &lo) {
    os << lo.price;
    return os;
}
#endif // ORDER_H
