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
    public:
        // insert() method passed all tests at
        // https://leetcode.com/problems/insert-into-a-binary-search-tree/description/
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
        // https://leetcode.com/problems/binary-tree-inorder-traversal/description/
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
        // https://leetcode.com/problems/search-in-a-binary-search-tree/description/
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


        // delete_node() method passed all tests at
        // https://leetcode.com/problems/delete-node-in-a-bst/description/
        static bool delete_node(TreeNode **root, int data) {
            if (*root == nullptr) {
                return false;
            }
            if (data < (*root)->val) {
                return delete_node(&((*root)->left), data);
            }
            if (data > (*root)->val) {
                return delete_node(&((*root)->right), data);
            }
            if ((*root)->val == data) {
                if ((*root)->left == nullptr && (*root)->right == nullptr) {
                    delete *root;
                    *root = nullptr;
                    return true;
                }
                if ((*root)->left == nullptr || (*root)->right == nullptr) {
                    // No need to look further, just promote the only child as
                    // parent
                    if ((*root)->left != nullptr) {
                        TreeNode *new_root = (*root)->left;
                        delete *root;
                        *root = new_root;
                    } else {
                        TreeNode *new_root = (*root)->right;
                        delete *root;
                        *root = new_root;
                    }
                    return true;
                }
                TreeNode **candidate_node = &((*root)->right);
                while ((*candidate_node)->left != nullptr) {
                    candidate_node = &((*candidate_node)->left);
                }
                // We are not really deleting root, we are just
                // replacing its value and then deleting candidate_node
                (*root)->val = (*candidate_node)->val;
                return delete_node(candidate_node, (*candidate_node)->val);
            }
            return false;
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

        // TODO:
        // https://leetcode.com/problems/kth-smallest-element-in-a-bst/description/
    };
} // namespace PoC::LockFree

#endif // BST_LOCKED_IMPL_H
