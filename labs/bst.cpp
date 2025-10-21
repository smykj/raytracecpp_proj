#include <cstddef>
#include <fstream>
#include <iostream>
#include <istream>
#include <memory>
#include <print>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
class Node {
  friend class Tree;

public:
  Node(const std::string &value) : value_{value} {}
  Node(std::string &&value) : value_{std::move(value)} {}
  const std::string &value() const { return value_; }
  Node *left() const { return left_.get(); }
  Node *right() const { return right_.get(); }
  void set_left(std::unique_ptr<Node> &&new_left) {
    left_ = std::move(new_left);
  }

  void set_right(std::unique_ptr<Node> &&new_right) {
    right_ = std::move(new_right);
  }

  void print_subtree(size_t depth) {
    if (left_ != nullptr) {
      left_->print_subtree(depth + 1);
    }

    std::println("{}{}", std::string(depth, '\t'), value_);

    if (right_ != nullptr) {
      right_->print_subtree(depth + 1);
    }
  }

private:
  const std::string value_;
  std::unique_ptr<Node> left_, right_;
};

class Tree {
public:
  void insert(std::string &&value) {

    if (root_ == nullptr) {
      root_ = std::make_unique<Node>(Node{std::move(value)});
      return;
    }

    Node *current = root_.get();
    while (true) {
      auto compare = current->value() <=> value;
      if (compare == 0) {
        return; // value was already present
      }
      if (compare < 0) {
        if (current->right() == nullptr) {
          current->set_right(std::make_unique<Node>(Node{std::move(value)}));
          return; // inserted
        }
        current = current->right();
        continue; // traversing deeper to the right
      }
      if (current->left() == nullptr) {
        current->set_left(std::make_unique<Node>(Node{std::move(value)}));
        return; // inserted
      }
      current = current->left();
      // traversing deeeper to the left
    }
  }

  // notice that the value is passed by value, as std::string_view already
  // represents a reference
  void remove(std::string_view value) {
    std::unique_ptr<Node> *to_be_removed;

    auto result = find_and_its_parent(value);
    if (std::get<0>(result) == nullptr || *std::get<0>(result) == nullptr) {
      return; // value not present
    }

    to_be_removed = std::get<0>(result);
    auto *parent = std::get<1>(result);

    if ((*to_be_removed)->left() == nullptr &&
        (*to_be_removed)->right() == nullptr) {
      (*to_be_removed) = nullptr;
      return; // node has no children
    }

    bool is_left_child;
    if (parent != nullptr && (*parent) != nullptr) {
      is_left_child = (*to_be_removed)->value() < (*parent)->value();
    }
    std::unique_ptr<Node> *replacement;
    if ((*to_be_removed)->left() == nullptr &&
        (*to_be_removed)->right() != nullptr) {
      if (parent == nullptr) {
        root_ = std::move((*to_be_removed)->right_);
        return; // found node was the root, and only had right child
      }

      replacement = &(*to_be_removed)->right_; //

      if (is_left_child) {
        (*parent)->set_left(std::move(*replacement));
      } else {
        (*parent)->set_right(std::move(*replacement));
      }

      return; // replaced found node with its only child (right)
    }
    if ((*to_be_removed)->left() != nullptr &&
        (*to_be_removed)->right() == nullptr) {

      if (parent == nullptr) {
        root_ = std::move((*to_be_removed)->left_);
        return; // found node was the root, and only had left child
      }
      replacement = &(*to_be_removed)->left_;

      if (is_left_child) {
        (*parent)->set_left(std::move(*replacement));
      } else {
        (*parent)->set_right(std::move(*replacement));
      }
      return; // replaced found node with its only child (left)
    }

    std::unique_ptr<Node> *replacements_parent =
        min_parent(&((*to_be_removed)->right_));

    if (replacements_parent == nullptr) {
      replacements_parent = to_be_removed;
      replacement = min(&((*to_be_removed)->right_));
    } else {
      replacement = &(*replacements_parent)->left_;
    }

    (*replacement)->set_left(std::move((*to_be_removed)->left_));
    std::unique_ptr<Node> temp = std::move((*replacement));
    if ((*to_be_removed)->right_ != temp) {
      (*replacements_parent)->set_left(std::move(temp->right_));
      temp->set_right(std::move((*to_be_removed)->right_));
    }

    if (parent == nullptr) {
      root_ = std::move(temp);
      return; // general case, except found node was the root
    }
    if (is_left_child) {
      (*parent)->set_left(std::move(temp));
    } else {
      (*parent)->set_right(std::move(temp));
    }
    // general case
  }
  std::tuple<std::unique_ptr<Node> *, std::unique_ptr<Node> *>
  find_and_its_parent(std::string_view value) {
    if (root_ == nullptr) {
      return {nullptr, nullptr};
    }
    std::unique_ptr<Node> *current = &root_;
    std::unique_ptr<Node> *prev = nullptr;
    while (true) {
      auto compare = (*current)->value() <=> value;
      if (compare == 0) {
        return {current, prev};
      }
      if (compare < 0) {
        if ((*current)->right() == nullptr) {
          return {nullptr, nullptr};
        }
        prev = current;
        current = &(*current)->right_;

      } else {
        if ((*current)->left() == nullptr) {
          return {nullptr, nullptr};
        }
        prev = current;
        current = &(*current)->left_;
      }
    }

    return {nullptr, nullptr};
  }

  std::unique_ptr<Node> *find(std::string_view value) {
    return std::get<0>(find_and_its_parent(value));
  }

  void clear() { root_ = nullptr; }

  void print() const {
    if (root_ == nullptr) {
      return;
    }
    root_->print_subtree(0);
  }

private:
  static std::unique_ptr<Node> *min(std::unique_ptr<Node> *node) {
    while ((*node)->left_ != nullptr) {
      node = &(*node)->left_;
    }
    return node;
  }
  static std::unique_ptr<Node> *min_parent(std::unique_ptr<Node> *node) {
    if (node == nullptr || (*node) == nullptr || (*node)->left_ == nullptr) {
      return nullptr;
    }
    while ((*node)->left_->left_ != nullptr) {
      node = &(*node)->left_;
    }
    return node;
  }
  std::unique_ptr<Node> root_;
};

void program_loop(Tree &tree, std::istream &in = std::cin) {
  std::string command;
  std::string name;
  while (in.good()) {
    in >> command >> std::ws;
    if (command == "insert") {
      std::getline(in, name);
      tree.insert(std::move(name));
    } else if (command == "remove") {
      std::getline(in, name);
      tree.remove(name);
    } else if (command == "print") {
      tree.print();
      std::println();
    } else if (command == "clear") {
      tree.clear();
    }
  }
}

int main(int argc, char *argv[]) {
  // create the tree object on the stack
  Tree tree;
  if (argc > 1) {
    std::ifstream is(argv[1]);
    program_loop(tree, is);
    is.close();
  } else {

    program_loop(
        tree); // TODO: if argc > 1, open the file and pass it to program_loop
  }
}
