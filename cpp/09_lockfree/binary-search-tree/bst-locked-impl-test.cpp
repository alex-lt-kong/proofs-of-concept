#include "bst-locked-impl.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <random>
#include <thread>

// using namespace PoC::LockFree;

using BST = PoC::LockFree::LockedBinarySearchTree;

TEST(BstLocked, InsertShouldWork) {
    std::vector<int> numbers;
    numbers.push_back(0);
    auto root = BST::insert(nullptr, numbers.back());
    EXPECT_EQ(BST::inorder_traversal(root), numbers);
    EXPECT_TRUE(BST::is_valid_bst(root));

    numbers.push_back(1);
    BST::insert(root, numbers.back());
    numbers.push_back(2);
    BST::insert(root, numbers.back());
    numbers.push_back(-1);
    BST::insert(root, numbers.back());
    std::ranges::sort(numbers);
    EXPECT_EQ(BST::inorder_traversal(root), numbers);
    EXPECT_TRUE(BST::is_valid_bst(root));

    BST::insert(root, -0);
    BST::insert(root, 0);
    BST::insert(root, -1);
    BST::insert(root, -1);
    BST::insert(root, -1);
    EXPECT_EQ(BST::inorder_traversal(root), numbers);
    EXPECT_TRUE(BST::is_valid_bst(root));

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution dist(INT_MIN, INT_MAX);

    constexpr size_t N = 1'000'000;
    for (size_t i = 0; i < N; ++i) {
        numbers.push_back(dist(generator));
        BST::insert(root, numbers.back());
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

TEST(BstLocked, InsertThenDeleteRootShouldWork) {
    auto root = BST::insert(nullptr, 0);
    EXPECT_EQ(BST::inorder_traversal(root), std::vector<int>{0});
    EXPECT_TRUE(BST::is_valid_bst(root));
    BST::delete_node(&root, 0);
    EXPECT_TRUE(BST::is_valid_bst(root));
    EXPECT_TRUE(root == nullptr);
}

TEST(BstLocked, InsertThenDeleteEntireBSTShouldWork) {
    std::vector<int> numbers;
    numbers.push_back(9527);
    auto root = BST::insert(nullptr, numbers.back());
    EXPECT_EQ(BST::inorder_traversal(root), numbers);
    EXPECT_TRUE(BST::is_valid_bst(root));

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution dist(INT_MIN, INT_MAX);

    constexpr size_t N = 1'000;
    for (size_t i = 0; i < N; ++i) {
        numbers.push_back(dist(generator));
        BST::insert(root, numbers.back());
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

TEST(BstLocked, CanDeallocateByKeepDeletingRoot) {
    auto root = BST::insert(nullptr, -1);

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution dist(INT_MIN, INT_MAX);
    std::unordered_map<int, int> numbers;

    for (int i = 0; i < 1'000; ++i) {
        auto ele = dist(generator);
        BST::insert(root, ele);
        numbers[ele] = 1;
    }

    while (root != nullptr) {
        EXPECT_TRUE(numbers.contains(root->val) || root->val == -1)
                << "Trying to delete root->val:" << root->val << "\n";
        numbers.erase(root->val);
        BST::delete_node(&root, root->val);
    }
    EXPECT_EQ(root, nullptr);
    EXPECT_EQ(numbers.size(), 0);
}
