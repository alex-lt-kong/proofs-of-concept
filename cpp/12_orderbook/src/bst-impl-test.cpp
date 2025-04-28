#include "bst-impl.h"
#include "order.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <random>
#include <thread>

using namespace PoC::OrderBook;
using BST = BinarySearchTree<int>;

TEST(Bst, InsertShouldWork) {
    std::vector<int> numbers;
    numbers.push_back(0);
    TreeNode<int> *root = nullptr;
    BST::insert(&root, numbers.back());
    EXPECT_EQ(BST::inorder_traversal(root), numbers);
    EXPECT_TRUE(BST::is_valid_bst(root));

    numbers.push_back(1);
    BST::insert(&root, numbers.back());
    numbers.push_back(2);
    BST::insert(&root, numbers.back());
    numbers.push_back(-1);
    BST::insert(&root, numbers.back());
    std::ranges::sort(numbers);
    EXPECT_EQ(BST::inorder_traversal(root), numbers);
    EXPECT_TRUE(BST::is_valid_bst(root));

    EXPECT_EQ(BST::insert(&root, -0), false);
    EXPECT_EQ(BST::insert(&root, 0), false);
    EXPECT_EQ(BST::insert(&root, -1), false);
    EXPECT_EQ(BST::insert(&root, -1), false);
    EXPECT_EQ(BST::insert(&root, -1), false);
    EXPECT_EQ(BST::inorder_traversal(root), numbers);
    EXPECT_TRUE(BST::is_valid_bst(root));

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution dist(INT_MIN, INT_MAX);

    constexpr size_t N = 1'000'000;
    for (size_t i = 0; i < N; ++i) {
        numbers.push_back(dist(generator));
        BST::insert(&root, numbers.back());
    }

    std::ranges::sort(numbers);
    EXPECT_NE(BST::inorder_traversal(root), numbers);
    // shall NOT equal before deduplication!
    EXPECT_TRUE(BST::is_valid_bst(root));

    // Deduplication
    const auto last = std::ranges::unique(numbers).begin();
    numbers.erase(last, numbers.end());
    EXPECT_EQ(BST::inorder_traversal(root), numbers);
    EXPECT_TRUE(BST::is_valid_bst(root));

    while (root != nullptr) {
        BST::delete_node(&root, root->val);
    }
}

TEST(Bst, InsertThenDeleteRootShouldWork) {
    TreeNode<int> *root = nullptr;
    BST::insert(&root, 0);
    EXPECT_EQ(BST::inorder_traversal(root), std::vector<int>{0});
    EXPECT_TRUE(BST::is_valid_bst(root));
    BST::delete_node(&root, 0);
    EXPECT_TRUE(BST::is_valid_bst(root));
    EXPECT_TRUE(root == nullptr);
}

TEST(Bst, InsertThenDeleteEntireBSTShouldWork) {
    std::vector<int> numbers;
    numbers.push_back(9527);
    TreeNode<int> *root = nullptr;
    BST::insert(&root, numbers.back());
    EXPECT_EQ(BST::inorder_traversal(root), numbers);
    EXPECT_TRUE(BST::is_valid_bst(root));

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution dist(INT_MIN, INT_MAX);

    constexpr size_t N = 2'000;
    for (size_t i = 0; i < N; ++i) {
        numbers.push_back(dist(generator));
        BST::insert(&root, numbers.back());
    }

    EXPECT_NE(BST::inorder_traversal(root), numbers);
    EXPECT_TRUE(BST::is_valid_bst(root));
    std::ranges::sort(numbers);
    const auto last = std::ranges::unique(numbers).begin();
    numbers.erase(last, numbers.end());
    EXPECT_EQ(BST::inorder_traversal(root), numbers);
    EXPECT_TRUE(BST::is_valid_bst(root));

    while (numbers.size() > 0) {
        std::uniform_int_distribution<> dis(0, numbers.size() - 1);
        const int random_index = dis(generator);
        const auto ele = numbers[random_index];
        numbers.erase(numbers.begin() + random_index);
        BST::delete_node(&root, ele);
        EXPECT_EQ(BST::inorder_traversal(root), numbers);
        EXPECT_TRUE(BST::is_valid_bst(root));
    }
    EXPECT_EQ(root, nullptr);
}

TEST(Bst, CanDeallocateByKeepDeletingRoot) {
    using BST = BinarySearchTree<LimitOrder>;
    TreeNode<LimitOrder> *root = nullptr;
    BST::insert(&root, -7);

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution dist(INT_MIN, INT_MAX);
    std::unordered_map<int, int> prices;

    for (int i = 0; i < 1'000'000; ++i) {
        auto ele = dist(generator);
        const auto res = BST::insert(&root, ele);
        EXPECT_EQ(prices.contains(ele), !res);
        prices[ele] = 1;
    }

    while (root != nullptr) {
        EXPECT_TRUE(prices.contains(root->val.price) || root->val == -7)
                << "Trying to delete root->val:" << root->val << "\n";
        prices.erase(root->val.price);
        BST::delete_node(&root, root->val);
    }
    EXPECT_EQ(root, nullptr);
    EXPECT_EQ(prices.size(), 0);
}

TEST(Bst, MultipleRoundsOfInsertAndDeleteShouldWork) {
    TreeNode<int> *root = nullptr;
    std::vector<int> numbers{-2};
    BST::insert(&root, numbers.back());

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution dist(INT_MIN, INT_MAX);

    constexpr size_t N = 100;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 2'000; ++j) {
            auto ele = dist(generator);
            const auto res = BST::insert(&root, ele);
            auto it = std::ranges::find(numbers, ele);
            if (res) {
                EXPECT_TRUE(it == numbers.end());
                numbers.push_back(ele);
            } else {
                EXPECT_TRUE(it != numbers.end());
            }
        }

        // EXPECT_NE(BST::inorder_traversal(root), numbers);
        std::ranges::sort(numbers);
        EXPECT_EQ(BST::inorder_traversal(root), numbers) << "i: " << i << "\n";

        while (numbers.size() > i) {
            std::uniform_int_distribution<> dis(0, numbers.size() - 1);
            const int random_index = dis(generator);
            const auto ele = numbers[random_index];
            numbers.erase(numbers.begin() + random_index);
            EXPECT_TRUE(BST::delete_node(&root, ele));
        }
    }
    EXPECT_EQ(numbers.size(), N - 1);
    // EXPECT_NE(BST::inorder_traversal(root), numbers);
    std::ranges::sort(numbers);
    EXPECT_EQ(BST::inorder_traversal(root), numbers);
    while (root != nullptr) {
        BST::delete_node(&root, root->val);
    }
    EXPECT_EQ(root, nullptr);
}
