#ifndef BST_LOCKED_IMPL_H
#define BST_LOCKED_IMPL_H

#include <print>
#include <vector>

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
        // insert() method passed all tests at
        // https://leetcode.com/problems/insert-into-a-binary-search-tree/submissions/1618140923/
        static TreeNode *insert(TreeNode *root, int val) {
            if (root == nullptr) {
                root = new TreeNode(val);
                return root;
            }
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
            return root;
        }

        // inorder_traversal() method passed all tests at
        // https://leetcode.com/problems/binary-tree-inorder-traversal/submissions/1618146714/
        // Note that this problem is inorder traversal only, not limited to BST
        static std::vector<int> inorder_traversal(TreeNode *root) {
            if (root == nullptr) {
                return {};
            }
            std::vector<int> result;
            if (root->left != nullptr) {
                auto left_result = inorder_traversal(root->left);
                result.insert(result.end(), left_result.begin(),
                              left_result.end());
            }
            result.push_back(root->val);
            std::print("{:>6}, ", root->val);
            if (root->right != nullptr) {
                auto right_result = inorder_traversal(root->right);
                result.insert(result.end(), right_result.begin(),
                              right_result.end());
            }
            return result;
        }

        // search() method passed all tests at
        // https://leetcode.com/problems/search-in-a-binary-search-tree/submissions/1618143004/
        static TreeNode *search(TreeNode *root, int val) {
            if (root == nullptr) {
                return nullptr;
            }
            if (root->val == val) {
                return root;
            }
            if (root->val > val) {
                return search(root->left, val);
            }
            return search(root->right, val);
        }


        static void delete_node(TreeNode **root, int data) {
            if (*root == nullptr) {
                return;
            }
            if (data < (*root)->val) {
                delete_node(&((*root)->left), data);
            } else if (data > (*root)->val) {
                delete_node(&((*root)->right), data);
            } else if ((*root)->val == data) {
                if ((*root)->left == nullptr && (*root)->right == nullptr) {
                    delete *root;
                    *root = nullptr;
                } else if ((*root)->left == nullptr ||
                           (*root)->right == nullptr) {
                    std::print("Deleteing {} with one child, ", (*root)->val);
                    // No need to look further, just promote the only child as
                    // parent
                    if ((*root)->left != nullptr) {
                        std::println("left child");
                        TreeNode *new_root = (*root)->left;
                        delete *root;
                        *root = new_root;
                    } else {
                        std::println("right child");
                        TreeNode *new_root = (*root)->right;
                        delete *root;
                        *root = new_root;
                    }
                } else {
                    std::println("Deleteing {} with both children",
                                 (*root)->val);
                    TreeNode **new_root = &((*root)->right);
                    while ((*new_root)->left != nullptr) {
                        new_root = &((*new_root)->left);
                    }
                    (*new_root)->left = (*root)->left;
                    // delete *root;
                    //(*root)->val = (*new_root)->val;
                    *root = *new_root;
                    // *root
                }
            } else {
                std::println("not found");
            }
        }

        static void visualize_tree(TreeNode *root, int depth = 0,
                                   char branch = ' ') {
            if (root == nullptr) {
                return;
            }
            // Print right subtree
            visualize_tree(root->right, depth + 1, '/');

            // Print current node value with appropriate indentation
            std::print("{:>{}}{}{}\n", "", 6 * depth, branch, root->val);

            // Print left subtree
            visualize_tree(root->left, depth + 1, '\\');
        }
    };
} // namespace PoC::LockFree

#endif // BST_LOCKED_IMPL_H
