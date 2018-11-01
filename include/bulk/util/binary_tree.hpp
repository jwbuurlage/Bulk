#pragma once

namespace bulk::util {

struct split {
    // dimension of the split
    int d;

    // voxel index at which the *second* subvolume begins
    int a;
};

template <typename T>
struct binary_tree {
    binary_tree() = default;
    binary_tree(T t) { root = std::make_unique<node>(t); }
    binary_tree(binary_tree&& other) : root(std::move(other.root)) {}

    enum class dir { left, right };

    struct node {
        node(T value_) : value(value_) {}

        int size() {
            int result = 1;
            if (left) {
                result += left->size();
            }
            if (right) {
                result += right->size();
            }
            return result;
        }

        std::unique_ptr<node> left = nullptr;
        std::unique_ptr<node> right = nullptr;
        T value;
    };

    node* add(node* parent, dir direction, T value) {
        if (parent != nullptr) {
            if (direction == dir::left) {
                parent->left = std::make_unique<node>(value);
                return parent->left.get();
            } else {
                parent->right = std::make_unique<node>(value);
                return parent->right.get();
            }
        } else {
            root = std::make_unique<node>(value);
            return root.get();
        }
    }

    int size() {
        if (root) {
            return root->size();
        }
        return 0;
    }

    std::unique_ptr<node> root;
};

} // namespace bulk::util
