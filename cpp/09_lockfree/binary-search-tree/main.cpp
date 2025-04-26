#include "bst-locked-impl.h"

using namespace PoC::LockFree;

int main() {
    auto root = new TreeNode(0);
    LockedBinarySearchTree::insert(root, 1);
    LockedBinarySearchTree::insert(root, 2);
    LockedBinarySearchTree::insert(root, -1);
    LockedBinarySearchTree::insert(root, -10086);
    LockedBinarySearchTree::insert(root, 1314);
    LockedBinarySearchTree::insert(root, 65536);
    LockedBinarySearchTree::insert(root, 3);
    LockedBinarySearchTree::insert(root, 4);
    LockedBinarySearchTree::insert(root, 5);
    LockedBinarySearchTree::insert(root, -213);
    LockedBinarySearchTree::insert(root, -3);
    LockedBinarySearchTree::insert(root, -23);
    LockedBinarySearchTree::inorder_traversal(root);
    std::println();

    LockedBinarySearchTree::delete_node(&root, 2);
    LockedBinarySearchTree::inorder_traversal(root);
    std::println();

    LockedBinarySearchTree::delete_node(&root, 3);
    LockedBinarySearchTree::inorder_traversal(root);
    std::println();

    LockedBinarySearchTree::delete_node(&root, -23);
    LockedBinarySearchTree::inorder_traversal(root);
    std::println();

    LockedBinarySearchTree::insert(root, 51);
    LockedBinarySearchTree::insert(root, -23);
    LockedBinarySearchTree::insert(root, -32345);
    LockedBinarySearchTree::insert(root, -32346);
    LockedBinarySearchTree::insert(root, -32344);
    LockedBinarySearchTree::insert(root, -76);
    LockedBinarySearchTree::inorder_traversal(root);
    std::println();
    LockedBinarySearchTree::visualize_tree(root);
    std::println();
    LockedBinarySearchTree::delete_node(&root, -10086);
    LockedBinarySearchTree::inorder_traversal(root);
    std::println();
    LockedBinarySearchTree::visualize_tree(root);
    std::println();
    return 0;
}
