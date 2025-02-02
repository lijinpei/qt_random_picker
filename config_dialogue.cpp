#include "config_dialogue.h"

#include <string>

#include "QIntValidator"
#include "QStringList"
#include "main_window.h"

void ConfigDialogue::on_path_edit_text_changed(const QString& text) {
  main_window.maybe_change_config_path(text);
}

void ConfigDialogue::on_reload_config_button_clicked() {
  main_window.reload_config();
}

void ConfigDialogue::on_open_config_button_clicked() {
  auto* file_dialog = ensure_config_file_dialog();
  file_dialog->setFileMode(QFileDialog::ExistingFile);
  QStringList fileNames;
  if (file_dialog->exec()) {
    fileNames = file_dialog->selectedFiles();
    if (fileNames.size() > 0) {
      main_window.maybe_change_config_path(fileNames[0]);
      main_window.reload_config();
    }
  }
}

void ConfigDialogue::on_save_config_button_clicked() {
  main_window.save_config();
}

void ConfigDialogue::on_save_config_as_button_clicked() {
  auto* file_dialog = ensure_config_file_dialog();
  file_dialog->setFileMode(QFileDialog::AnyFile);
  QStringList fileNames;
  if (file_dialog->exec()) {
    fileNames = file_dialog->selectedFiles();
    if (fileNames.size() > 0) {
      main_window.save_config(fileNames[0]);
    }
  }
}

void ConfigDialogue::on_reload_list_button_clicked() {
  main_window.load_name_list();
}

void ConfigDialogue::on_open_list_button_clicked() {
  auto* list_file_dialog = ensure_list_file_dialog();
  list_file_dialog->setFileMode(QFileDialog::ExistingFile);
  QStringList fileNames;
  if (list_file_dialog->exec()) {
    fileNames = list_file_dialog->selectedFiles();
    if (fileNames.size() > 0) {
      main_window.maybe_change_names_path(fileNames[0]);
      main_window.load_name_list();
    }
  }
}

void ConfigDialogue::on_save_list_as_button_clicked() {
  auto* list_file_dialog = ensure_list_file_dialog();
  list_file_dialog->setFileMode(QFileDialog::AnyFile);
  QStringList fileNames;
  if (!list_file_dialog->exec()) {
    return;
  }
  fileNames = list_file_dialog->selectedFiles();
  if (fileNames.size() == 0) {
    return;
  }
  QFile save_file(fileNames[0]);
  if (!save_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return;
  }
  for (const auto& name : main_window.get_names()) {
    auto bytes = name.toUtf8();
    if (bytes.size() != save_file.write(bytes)) {
      return;
    }
    if (!save_file.putChar('\n')) {
      return;
    }
  }
}

void ConfigDialogue::on_show_history_button_clicked() {
  main_window.show_history_view();
}

void ConfigDialogue::on_clear_history_button_clicked() {
  main_window.clear_history();
}

void ConfigDialogue::on_save_history_button_clicked() {
  auto* hist_file_dialog = ensure_list_file_dialog();
  hist_file_dialog->setFileMode(QFileDialog::AnyFile);
  if (!hist_file_dialog->exec()) {
    return;
  }
  QStringList fileNames = hist_file_dialog->selectedFiles();
  if (fileNames.isEmpty()) {
    return;
  }
  QFile hist_file(fileNames[0]);
  if (!hist_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return;
  }
  auto& history = main_window.get_history_recorder();
  for (int i = 0, e = history.get_history_count(); i < e; ++i) {
    int h = history.get_history_item(i);
    auto h_str = std::to_string(h);
    if (hist_file.write(h_str.data(), h_str.size()) != h_str.size()) {
      return;
    }
    if (!hist_file.putChar('\n')) {
      return;
    }
  }
}

void ConfigDialogue::on_load_history_button_clicked() {
  auto* hist_file_dialog = ensure_list_file_dialog();
  hist_file_dialog->setFileMode(QFileDialog::ExistingFile);
  if (!hist_file_dialog->exec()) {
    return;
  }
  QStringList fileNames = hist_file_dialog->selectedFiles();
  if (fileNames.isEmpty()) {
    return;
  }
  QFile hist_file(fileNames[0]);
  if (!hist_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return;
  }
  auto& history = main_window.get_history_recorder();
  history.reset(main_window.get_names().size());
  while (true) {
    auto line = hist_file.readLine();
    int e = line.size();
    if (e == 0) {
      break;
    }
    int i;
    for (i = 0; i < e; ++i) {
      if (!isspace(line[i])) {
        break;
      }
    }
    if (i == e || !isdigit(line[i])) {
      continue;
    }
    int val = 0;
    for (; i < e; ++i) {
      char c = line[i];
      if (!isdigit(c)) {
        break;
      }
      val = val * 10 + (c - '0');
    }
    history.record_pick(val);
  }
  main_window.maybe_redraw_history_view();
}

QFileDialog* ConfigDialogue::ensure_config_file_dialog() {
  if (!config_file_dialog) {
    config_file_dialog = std::make_unique<QFileDialog>(this);
    config_file_dialog->setViewMode(QFileDialog::List);
    config_file_dialog->setNameFilter("JSON file (*.json)");
  }
  return config_file_dialog.get();
}

QFileDialog* ConfigDialogue::ensure_list_file_dialog() {
  if (!list_file_dialog) {
    list_file_dialog = std::make_unique<QFileDialog>(this);
    list_file_dialog->setViewMode(QFileDialog::List);
  }
  return list_file_dialog.get();
}

QFileDialog* ConfigDialogue::ensure_history_file_dialog() {
  if (!history_file_dialog) {
    history_file_dialog = std::make_unique<QFileDialog>(this);
    history_file_dialog->setViewMode(QFileDialog::List);
  }
  return history_file_dialog.get();
}

void ConfigDialogue::on_no_dup_box_clicked(Qt::CheckState state) {
  main_window.maybe_change_no_duplicate(state == Qt::Checked);
}

void ConfigDialogue::on_no_animation_box_clicked(Qt::CheckState state) {
  main_window.maybe_change_no_animation(state == Qt::Checked);
}

void ConfigDialogue::on_animation_freq_text_changed(const QString& text) {
  bool ok;
  int value = text.toInt(&ok);
  if (!ok) {
    auto pos = animation_freq->cursorPosition();
    animation_freq->setText(
        QString::number(main_window.get_config().time_slice_ms));
    animation_freq->setCursorPosition(pos);
  } else {
    main_window.maybe_change_time_slice_ms(value);
  }
}

void ConfigDialogue::on_show_setting_box_clicked(Qt::CheckState state) {
  main_window.maybe_change_show_setting(state == Qt::Checked);
}

ConfigDialogue::ConfigDialogue(MainWindow& main_window, QWidget* parent)
    : QWidget(parent), main_window(main_window) {
  layout = std::make_unique<QVBoxLayout>();

  config_group = std::make_unique<QGroupBox>("配置");
  config_group_layout = std::make_unique<QVBoxLayout>();
  reload_config_button = std::make_unique<QPushButton>("加载");
  open_config_button = std::make_unique<QPushButton>("打开");
  config_path_edit =
      std::make_unique<QLineEdit>(main_window.get_config().config_path);
  config_path_label = std::make_unique<QLabel>("路径:");
  config_group_layout_line1 = std::make_unique<QHBoxLayout>();
  config_group_layout_line1->addWidget(config_path_label.get());
  config_group_layout_line1->addWidget(config_path_edit.get(), 10);
  config_group_layout->addLayout(config_group_layout_line1.get());
  save_config_button = std::make_unique<QPushButton>("保存");
  save_config_as_button = std::make_unique<QPushButton>("另存为");
  config_group_layout_line2 = std::make_unique<QHBoxLayout>();
  config_group_layout_line2->addWidget(reload_config_button.get());
  config_group_layout_line2->addWidget(open_config_button.get());
  config_group_layout_line2->addWidget(save_config_button.get());
  config_group_layout_line2->addWidget(save_config_as_button.get());
  config_group_layout->addLayout(config_group_layout_line2.get());
  config_group->setLayout(config_group_layout.get());
  layout->addWidget(config_group.get());
  connect(config_path_edit.get(), &QLineEdit::textChanged, this,
          &ConfigDialogue::on_path_edit_text_changed);
  connect(&main_window, &MainWindow::config_path_changed, this,
          &ConfigDialogue::on_config_changed_config_path);
  connect(reload_config_button.get(), &QPushButton::clicked, this,
          &ConfigDialogue::on_reload_config_button_clicked);
  connect(open_config_button.get(), &QPushButton::clicked, this,
          &ConfigDialogue::on_open_config_button_clicked);
  connect(save_config_button.get(), &QPushButton::clicked, this,
          &ConfigDialogue::on_save_config_button_clicked);
  connect(save_config_as_button.get(), &QPushButton::clicked, this,
          &ConfigDialogue::on_save_config_as_button_clicked);

  random_group = std::make_unique<QGroupBox>("抽签");
  random_group_layout = std::make_unique<QVBoxLayout>();

  name_list_label = std::make_unique<QLabel>("路径:");
  name_list_edit =
      std::make_unique<QLineEdit>(main_window.get_config().names_path);
  random_group_layout_line1 = std::make_unique<QHBoxLayout>();
  random_group_layout_line1->addWidget(name_list_label.get());
  random_group_layout_line1->addWidget(name_list_edit.get(), 10);
  random_group_layout->addLayout(random_group_layout_line1.get());

  reload_list_button = std::make_unique<QPushButton>("加载");
  open_list_button = std::make_unique<QPushButton>("打开");
  save_list_as_button = std::make_unique<QPushButton>("另存为");
  random_group_layout_line2 = std::make_unique<QHBoxLayout>();
  random_group_layout_line2->addWidget(reload_list_button.get());
  random_group_layout_line2->addWidget(open_list_button.get());
  random_group_layout_line2->addWidget(save_list_as_button.get());
  random_group_layout->addLayout(random_group_layout_line2.get());
  connect(reload_list_button.get(), &QPushButton::clicked, this,
          &ConfigDialogue::on_reload_list_button_clicked);
  connect(open_list_button.get(), &QPushButton::clicked, this,
          &ConfigDialogue::on_open_list_button_clicked);
  connect(save_list_as_button.get(), &QPushButton::clicked, this,
          &ConfigDialogue::on_save_list_as_button_clicked);
  connect(&main_window, &MainWindow::names_path_changed, this,
          &ConfigDialogue::on_config_changed_names_path);

  no_dup_box = std::make_unique<QCheckBox>("无重复");
  no_dup_box->setCheckState(
      main_window.get_config().no_duplicate ? Qt::Checked : Qt::Unchecked);
  no_dup_box->setTristate(false);
  random_group_layout->addWidget(no_dup_box.get());
  random_group->setLayout(random_group_layout.get());
  layout->addWidget(random_group.get());

  connect(no_dup_box.get(), &QCheckBox::checkStateChanged, this,
          &ConfigDialogue::on_no_dup_box_clicked);
  connect(&main_window, &MainWindow::no_duplicate_changed, this,
          &ConfigDialogue::on_config_changed_no_duplicate);

  history_group = std::make_unique<QGroupBox>("历史记录");
  history_group_layout = std::make_unique<QVBoxLayout>();
  show_history_button = std::make_unique<QPushButton>("显示");
  clear_history_button = std::make_unique<QPushButton>("清空");
  save_history_button = std::make_unique<QPushButton>("保存");
  load_history_button = std::make_unique<QPushButton>("加载");
  history_group_layout->addWidget(show_history_button.get());
  history_group_layout->addWidget(clear_history_button.get());
  history_group_layout->addWidget(save_history_button.get());
  history_group_layout->addWidget(load_history_button.get());
  history_group->setLayout(history_group_layout.get());
  layout->addWidget(history_group.get());

  connect(show_history_button.get(), &QPushButton::clicked, this,
          &ConfigDialogue::on_show_history_button_clicked);
  connect(clear_history_button.get(), &QPushButton::clicked, this,
          &ConfigDialogue::on_clear_history_button_clicked);
  connect(save_history_button.get(), &QPushButton::clicked, this,
          &ConfigDialogue::on_save_history_button_clicked);
  connect(load_history_button.get(), &QPushButton::clicked, this,
          &ConfigDialogue::on_load_history_button_clicked);

  ui_group = std::make_unique<QGroupBox>("界面");
  ui_group_layout = std::make_unique<QVBoxLayout>();
  no_animation_box = std::make_unique<QCheckBox>("无滚动效果");
  no_animation_box->setCheckState(
      main_window.get_config().no_animation ? Qt::Checked : Qt::Unchecked);
  no_animation_box->setTristate(false);
  ui_group_layout->addWidget(no_animation_box.get());
  connect(no_animation_box.get(), &QCheckBox::checkStateChanged, this,
          &ConfigDialogue::on_no_animation_box_clicked);
  connect(&main_window, &MainWindow::no_animation_changed, this,
          &ConfigDialogue::on_config_changed_no_animation);

  animation_freq = std::make_unique<QLineEdit>(
      QString::number(main_window.get_config().time_slice_ms));
  auto* freq_validator = new QIntValidator();
  freq_validator->setBottom(1);
  animation_freq->setValidator(freq_validator);
  auto* freq_edit_label = new QLabel("滚动间隔(毫秒)");
  auto* freq_edit_layout = new QHBoxLayout();
  freq_edit_layout->addWidget(freq_edit_label);
  freq_edit_layout->addWidget(animation_freq.get(), 10);
  ui_group_layout->addLayout(freq_edit_layout);
  connect(animation_freq.get(), &QLineEdit::textChanged, this,
          &ConfigDialogue::on_animation_freq_text_changed);
  connect(&main_window, &MainWindow::time_slice_ms_changed, this,
          &ConfigDialogue::on_config_changed_time_slice_ms);

  show_setting_box = std::make_unique<QCheckBox>("显示更多");
  show_setting_box->setCheckState(
      main_window.get_config().show_setting ? Qt::Checked : Qt::Unchecked);
  show_setting_box->setTristate(false);
  ui_group_layout->addWidget(show_setting_box.get());
  connect(show_setting_box.get(), &QCheckBox::checkStateChanged, this,
          &ConfigDialogue::on_show_setting_box_clicked);
  connect(&main_window, &MainWindow::show_setting_changed, this,
          &ConfigDialogue::on_config_changed_show_setting);

  ui_group->setLayout(ui_group_layout.get());
  layout->addWidget(ui_group.get());
  layout->addStretch(10);
  setLayout(layout.get());
}
