#pragma once

#include <memory>

#include "QCheckBox"
#include "QFileDialog"
#include "QGroupBox"
#include "QHBoxLayout"
#include "QLabel"
#include "QLineEdit"
#include "QPushButton"
#include "QString"
#include "QVBoxLayout"
#include "QWidget"
#include "main_window_config.h"

class MainWindow;

class ConfigDialogue : public QWidget {
  Q_OBJECT

  MainWindow& main_window;

  std::unique_ptr<QVBoxLayout> layout;

  std::unique_ptr<QGroupBox> config_group;
  std::unique_ptr<QLabel> config_path_label;
  std::unique_ptr<QVBoxLayout> config_group_layout;
  std::unique_ptr<QHBoxLayout> config_group_layout_line1,
      config_group_layout_line2;
  std::unique_ptr<QPushButton> reload_config_button, open_config_button,
      save_config_button, save_config_as_button;
  std::unique_ptr<QLineEdit> config_path_edit;

  void on_path_edit_text_changed(const QString& text);
  void on_config_changed_config_path(const QString& new_val) {
    auto pos = config_path_edit->cursorPosition();
    config_path_edit->setText(new_val);
    config_path_edit->setCursorPosition(pos);
  }

  void on_reload_config_button_clicked();
  void on_open_config_button_clicked();
  void on_save_config_button_clicked();
  void on_save_config_as_button_clicked();

  std::unique_ptr<QGroupBox> random_group;
  std::unique_ptr<QVBoxLayout> random_group_layout;
  std::unique_ptr<QHBoxLayout> random_group_layout_line1,
      random_group_layout_line2;
  std::unique_ptr<QCheckBox> no_dup_box;
  std::unique_ptr<QLabel> name_list_label;
  std::unique_ptr<QLineEdit> name_list_edit;
  std::unique_ptr<QPushButton> reload_list_button, open_list_button,
      save_list_as_button;

  void on_reload_list_button_clicked();
  void on_open_list_button_clicked();
  void on_save_list_as_button_clicked();
  void on_config_changed_names_path(const QString& new_val) {
    auto pos = name_list_edit->cursorPosition();
    name_list_edit->setText(new_val);
    name_list_edit->setCursorPosition(pos);
  }

  std::unique_ptr<QGroupBox> history_group;
  std::unique_ptr<QVBoxLayout> history_group_layout;
  std::unique_ptr<QPushButton> show_history_button, clear_history_button,
      save_history_button, load_history_button;
  void on_show_history_button_clicked();
  void on_clear_history_button_clicked();
  void on_save_history_button_clicked();
  void on_load_history_button_clicked();

  std::unique_ptr<QGroupBox> ui_group;
  std::unique_ptr<QVBoxLayout> ui_group_layout;
  std::unique_ptr<QCheckBox> no_animation_box;
  std::unique_ptr<QLineEdit> animation_freq;
  std::unique_ptr<QCheckBox> show_setting_box;

  std::unique_ptr<QFileDialog> config_file_dialog;
  QFileDialog* ensure_config_file_dialog();

  std::unique_ptr<QFileDialog> list_file_dialog;
  QFileDialog* ensure_list_file_dialog();

  std::unique_ptr<QFileDialog> history_file_dialog;
  QFileDialog* ensure_history_file_dialog();

  void on_config_changed_no_duplicate(bool no_dup) {
    no_dup_box->setChecked(no_dup);
  }
  void on_config_changed_no_animation(bool no_animation) {
    no_animation_box->setChecked(no_animation);
  }
  void on_config_changed_time_slice_ms(int new_ms) {
    auto pos = animation_freq->cursorPosition();
    animation_freq->setText(QString::number(new_ms));
    animation_freq->setCursorPosition(pos);
  }
  void on_config_changed_show_setting(bool new_val) {
    show_setting_box->setChecked(new_val);
  }

  void on_no_animation_box_clicked(Qt::CheckState state);
  void on_animation_freq_text_changed(const QString& text);
  void on_no_dup_box_clicked(Qt::CheckState state);
  void on_show_setting_box_clicked(Qt::CheckState state);

 public:
  ConfigDialogue(MainWindow& main_window, QWidget* parent = nullptr);
};
