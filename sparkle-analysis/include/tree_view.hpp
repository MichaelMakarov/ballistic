#pragma once

#include <tree_data_provider.hpp>

#include <qtreeview.h>

class tree_view : public QTreeView {
  public:
    tree_view(std::shared_ptr<tree_data_provider> provider, QWidget *parent = nullptr);

    void update_content();

  private:
    void resize_to_contents();
};