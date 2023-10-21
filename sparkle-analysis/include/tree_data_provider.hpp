#pragma once

#include <memory>
#include <string>
#include <vector>

class tree_data_content {
  public:
    virtual ~tree_data_content() = default;

    void set_parent(tree_data_content *provider, std::size_t index) {
        _parent = provider;
        _index = index;
    }

    void append_child(std::unique_ptr<tree_data_content> provider) {
        provider->set_parent(this, _children.size());
        _children.push_back(std::move(provider));
    }

    void clear_children() {
        _children.clear();
    }

    std::size_t get_parent_index() const {
        return _index;
    }

    tree_data_content *get_child(std::size_t index) const {
        return _children[index].get();
    }

    tree_data_content *get_parent() const {
        return _parent;
    }

    std::size_t get_children_count() const {
        return _children.size();
    }

    virtual std::string get_content(std::size_t column) const = 0;

    virtual int get_alignment(std::size_t column) const = 0;

  private:
    tree_data_content *_parent;
    std::size_t _index;
    std::vector<std::unique_ptr<tree_data_content>> _children;
};

class tree_data_provider {
  public:
    virtual ~tree_data_provider() = default;

    virtual tree_data_content *get_root_content() const = 0;

    virtual std::string get_header(std::size_t column) const = 0;

    virtual std::size_t get_columns_count() const = 0;
};
