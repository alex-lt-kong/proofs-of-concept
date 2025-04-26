#include "bst-locked-impl.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <print>
#include <thread>

using namespace PoC::LockFree;

TEST(BstLocked, InsertShouldWork) {
    auto root = LockedBinarySearchTree::insert(nullptr, 0);
    EXPECT_EQ(LockedBinarySearchTree::inorder_traversal(root),
              (std::vector{0}));

    LockedBinarySearchTree::insert(root, 1);
    LockedBinarySearchTree::insert(root, 2);
    LockedBinarySearchTree::insert(root, -1);
    EXPECT_EQ(LockedBinarySearchTree::inorder_traversal(root),
              (std::vector{-1, 0, 1, 2}));

    LockedBinarySearchTree::insert(root, -10086);
    LockedBinarySearchTree::insert(root, 1314);
    LockedBinarySearchTree::insert(root, 65536);
    LockedBinarySearchTree::insert(root, 3);
    LockedBinarySearchTree::insert(root, 4);
    LockedBinarySearchTree::insert(root, 5);
    LockedBinarySearchTree::insert(root, -213);
    LockedBinarySearchTree::insert(root, -3);
    LockedBinarySearchTree::insert(root, -23);
    EXPECT_EQ(LockedBinarySearchTree::inorder_traversal(root),
              (std::vector{-10086, -213, -23, -3, -1, 0, 1, 2, 3, 4, 5, 1314,
                           65536}));

    LockedBinarySearchTree::insert(root, -0);
    LockedBinarySearchTree::insert(root, 0);
    LockedBinarySearchTree::insert(root, -23);
    LockedBinarySearchTree::insert(root, -23);
    LockedBinarySearchTree::insert(root, -23);
    EXPECT_EQ(LockedBinarySearchTree::inorder_traversal(root),
              (std::vector{-10086, -213, -23, -3, -1, 0, 1, 2, 3, 4, 5, 1314,
                           65536}));
}
