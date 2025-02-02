#pragma once

#include <vector>

#include "QAbstractListModel"
#include "QString"
#include "QVariant"
#include "QWidget"
#include "history_recorder.h"

class HistoryListModel : public QAbstractListModel {
  Q_OBJECT
  HistoryRecorder& history;
  const std::vector<QString>& names;

 public:
  HistoryListModel(HistoryRecorder& history, const std::vector<QString>& names,
                   QWidget* parent = nullptr)
      : history(history), names(names), QAbstractListModel(parent) {}
  int rowCount(const QModelIndex& parent) const override {
    return history.get_history_count();
  }
  QVariant data(const QModelIndex& index, int role) const override {
    if (!index.isValid()) {
      return {};
    }
    if (role == Qt::DisplayRole) {
      return names[history.get_history_item(index.row())];
    }
    return {};
  }
  virtual ~HistoryListModel();
  void populate() {
    beginResetModel();
    endResetModel();
  }
};
