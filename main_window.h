#pragma once

#include <chrono>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "QAction"
#include "QCheckBox"
#include "QFont"
#include "QFontMetrics"
#include "QJsonObject"
#include "QLabel"
#include "QListView"
#include "QMenu"
#include "QPushButton"
#include "QRect"
#include "QResizeEvent"
#include "QShortcut"
#include "QSize"
#include "QString"
#include "QTimer"
#include "QVBoxLayout"
#include "auto_resize_label.h"
#include "config_dialogue.h"
#include "history_list_model.h"
#include "history_recorder.h"
#include "main_window_config.h"

class MainWindow : public QWidget {
  Q_OBJECT

  bool isStoped = true;
  MainWindowConfig config, saved_config;
  QString config_path = "config.json";
  std::unique_ptr<QPushButton> rand_button;
  std::unique_ptr<AutoResizeLabel> text_display;
  std::unique_ptr<QVBoxLayout> v_layout;
  std::unique_ptr<QWidget> settings;
  std::unique_ptr<QHBoxLayout> h_layout;
  std::unique_ptr<QCheckBox> no_dup_box;
  std::unique_ptr<QPushButton> clear_history_button;
  std::unique_ptr<QPushButton> history_list_button;
  std::unique_ptr<QPushButton> more_act_button;
  std::unique_ptr<QTimer> timer;
  std::unique_ptr<ConfigDialogue> config_dialogue;
  std::unique_ptr<QListView> history_view;
  std::unique_ptr<HistoryListModel> history_model;

  std::unique_ptr<QShortcut> sc_pick, sc_no_dup, sc_no_animation,
      sc_show_setting;

  int max_name_len;
  int max_name_pos;
  std::vector<QString> names;

  std::random_device rd;
  HistoryRecorder history;

  QString getTextForButton() const {
    return isStoped ? QString("开始") : QString("结束");
  }

  // Response to config-change events
  void on_config_changed_no_duplicate(bool no_dup) {
    no_dup_box->setChecked(no_dup);
  }
  void on_config_changed_show_setting(bool new_val) {
    settings->setVisible(new_val);
  }

  // Response to UI events
  void on_rand_button_clicked() {
    if (config.no_animation) {
      auto picked = commit_a_rand_pick();
      isStoped = true;
      if (picked >= 0 && picked < names.size()) {
        text_display->setText(names[picked]);
      }
      rand_button->setText(getTextForButton());
    } else {
      isStoped = !isStoped;
      rand_button->setText(getTextForButton());
      if (isStoped) {
        timer->stop();
        auto picked = commit_a_rand_pick();
        if (picked >= 0 && picked < names.size()) {
          text_display->setText(names[picked]);
        }

      } else {
        timer->start(std::chrono::milliseconds(config.time_slice_ms));
      }
    }
  }

  void on_text_display_clicked() {
    auto show_setting = !config.show_setting;
    maybe_change_show_setting(show_setting);
  }

  void on_no_dup_box_checked(Qt::CheckState state) {
    maybe_change_no_duplicate(state == Qt::Checked);
  }

  void on_clear_history_button_clicked() { clear_history(); }
  void on_more_act_button_clicked() { config_dialogue->show(); }
  void on_timer_timeout() {
    auto num_names = names.size();
    if (!num_names) {
      text_display->setText("名单为空,请检查名单设置");
    } else {
      if (!isStoped) {
        int picked = select_a_rand_pick();
        text_display->setText(names[picked]);
        timer->setInterval(std::chrono::milliseconds(config.time_slice_ms));
      } else {
        timer->stop();
      }
    }
  }

  void on_history_list_button_clicked() { show_history_view(); }

  int commit_a_rand_pick() {
    auto res = select_a_rand_pick();
    history.record_pick(res);
    if (history_view->isVisible()) {
      history_model->populate();
    }
    return res;
  }

  int select_a_rand_pick() {
    if (config.no_duplicate) {
      auto distrib =
          std::uniform_int_distribution<>(0, history.get_remaining_count() - 1);
      return history.peek_no_duplicate(distrib(rd));
    } else {
      auto distrib =
          std::uniform_int_distribution<>(0, history.get_total_count() - 1);
      return distrib(rd);
    }
  }

  void refresh_ui_on_reload_config(const MainWindowConfig &);

 public:
// Anything wants to change configs should use these mathods
#define IMPL(X, JSON_KIND, KIND)                 \
  bool maybe_change_##X(const KIND &new_value) { \
    if (config.X == new_value) {                 \
      return false;                              \
    }                                            \
    config.X = new_value;                        \
    emit X##_changed(new_value);                 \
    return true;                                 \
  }
  MAIN_WINDOW_CONFIG_FIELDS(IMPL)
#undef IMPL

 signals:

// Anything sensitive to configs should subscribe to these signals
#define IMPL(X, JSON_KIND, KIND) void X##_changed(const KIND &new_value);
  MAIN_WINDOW_CONFIG_FIELDS(IMPL)
#undef IMPL

 public:
  MainWindow(QWidget *parent = nullptr);
  virtual ~MainWindow();
  const MainWindowConfig &get_config() const { return config; }
  bool reload_config(bool no_update_ui = false);
  bool save_config(const QString &path_ = {});

  const std::vector<QString> &get_names() const { return names; }
  void load_name_list(bool refresh_ui = true);

  void show_history_view() {
    history_model->populate();
    history_view->show();
  }
  void clear_history() {
    history.reset(names.size());
    maybe_redraw_history_view();
  }
  void maybe_redraw_history_view() {
    if (history_view->isVisible()) {
      history_model->populate();
    }
  }
  HistoryRecorder &get_history_recorder() { return history; }
};
