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
    LockedBinarySearchTree::insert(root, -23);
    LockedBinarySearchTree::inorder_traversal(root);
    std::println();

    LockedBinarySearchTree::delete_node(&root, 2);
    LockedBinarySearchTree::inorder_traversal(root);
    std::println();

    /*
    LockedBinarySearchTree::delete_node(root, -10086);
    LockedBinarySearchTree::inorder_traversal(root);
    std::println();
    */
    LockedBinarySearchTree::delete_node(&root, 3);
    LockedBinarySearchTree::inorder_traversal(root);
    std::println();
    LockedBinarySearchTree::delete_node(&root, -23);
    LockedBinarySearchTree::inorder_traversal(root);
    std::println();
    LockedBinarySearchTree::delete_node(&root, -10086);
    LockedBinarySearchTree::inorder_traversal(root);
    std::println();
    return 0;
}
