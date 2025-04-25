#ifndef BST_LOCKED_IMPL_H
#define BST_LOCKED_IMPL_H

#include <print>

namespace PoC::LockFree {
    struct TreeNode {
        int val;
        TreeNode *left;
        TreeNode *right;
        TreeNode() : val(0), left(nullptr), right(nullptr) {}
        explicit TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
        TreeNode(int x, TreeNode *left, TreeNode *right) :
            val(x), left(left), right(right) {}
    };
    class LockedBinarySearchTree {
        static int find_successor(TreeNode *root, TreeNode *node) {}

    public:
        static void insert(TreeNode *root, int val) {
            if (root->val > val) {
                if (root->left == nullptr) {
                    root->left = new TreeNode(val);
                } else {
                    insert(root->left, val);
                }
            } else if (root->val < val) {
                if (root->right == nullptr) {
                    root->right = new TreeNode(val);
                } else {
                    insert(root->right, val);
                }
            } else {
                std::println("Node exists");
            }
        }

        static void inorder_traversal(TreeNode *root) {
            if (root->left != nullptr) {
                inorder_traversal(root->left);
            }
            std::print("{}, ", root->val);
            if (root->right != nullptr) {
                inorder_traversal(root->right);
            }
        }

        static void delete_node(TreeNode **root, int data) {
            if (root == nullptr) {
                return;
            }
            if (data < (*root)->val) {
                delete_node(&((*root)->left), data);
            } else if (data > (*root)->val) {
                delete_node(&((*root)->right), data);
            } else if ((*root)->val == data) {
                // Node with two children: Get the inorder successor (smallest
                // in the right subtree)
                if ((*root)->right != nullptr) {
                    TreeNode **successorNode = &((*root)->right);
                    while ((*successorNode)->left) {
                        //  successorParent = successorNode;
                        successorNode = &((*successorNode)->left);
                    }
                    (*root)->val = (*successorNode)->val;
                    *successorNode = nullptr;
                } else if ((*root)->left != nullptr) {
                    std::println("if (root->left != nullptr)");
                    TreeNode **successorNode = &((*root)->left);
                    while ((*successorNode)->right) {
                        successorNode = &((*successorNode)->right);
                    }
                    (*root)->val = (*successorNode)->val;
                    *successorNode = nullptr;
                }

            } else {
                std::println("not found");
            }
        }
    };
} // namespace PoC::LockFree

#endif // BST_LOCKED_IMPL_H
