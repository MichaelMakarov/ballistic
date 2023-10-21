#pragma once

#include <table_data_provider.hpp>

#include <qtableview.h>

class table_view : public QTableView {
  public:
    table_view(std::shared_ptr<table_data_provider> provider, QWidget *parent = nullptr);

    void update_content();

  private:
    void resize_to_contents();
};