#include "main_window.h"

#include <optional>
#include <utility>
#include <vector>

#include "QByteArray"
#include "QFile"
#include "QJsonDocument"

#ifndef _WIN32
#include <sys/stat.h>
#else
#include "fileapi.h"
#include "io.h"
#include "windows.h"
#endif

namespace {
bool referes_to_same_file(const QFile& lhs, const QFile& rhs) {
#ifndef _WIN32
  auto get_inode =
      [](const QFile& file) -> std::optional<std::pair<uint64_t, uint64_t>> {
    auto handle = file.handle();
    struct stat file_stat;
    auto ret = fstat(handle, &file_stat);
    if (ret < 0) {
      return std::nullopt;
    }
    // FIXME: it's this well defined?
    uint64_t high = (uint64_t)file_stat.st_dev;
    uint64_t low = (uint64_t)file_stat.st_ino;
    return std::make_pair(high, low);
  };
#else
  auto get_inode = [](const QFile& file) -> std::vector<TCHAR> {
    auto fd = file.handle();
    HANDLE handle = (HANDLE)_get_osfhandle(fd);
    std::vector<TCHAR> res;
    auto flag = FILE_NAME_NORMALIZED;
    DWORD length = GetFinalPathNameByHandleW(handle, nullptr, 0, flag);
    res.resize(length);
    if (GetFinalPathNameByHandleW(handle, res.data(), length, flag) + 1 ==
        length) {
      return res;
    } else {
      return {};
    }
  };
#endif
  auto lhs_inode = get_inode(lhs);
  auto rhs_inode = get_inode(rhs);
#ifndef _WIN32
  return lhs_inode && rhs_inode && lhs_inode == rhs_inode;
#else
  return lhs_inode.size() && rhs_inode.size() == lhs_inode.size() &&
         lhs_inode == rhs_inode;
#endif
}
}  // namespace

MainWindow::MainWindow(QWidget* parent) : QWidget(parent) {
  reload_config(true);
  load_name_list(false);
  settings = std::make_unique<QWidget>(this);
  settings->setVisible(config.show_setting);
  h_layout = std::make_unique<QHBoxLayout>(settings.get());
  no_dup_box = std::make_unique<QCheckBox>("无重复");
  no_dup_box->setCheckState(config.no_duplicate ? Qt::Checked : Qt::Unchecked);
  no_dup_box->setTristate(false);
  connect(no_dup_box.get(), &QCheckBox::checkStateChanged, this,
          &MainWindow::on_no_dup_box_checked);
  connect(this, &MainWindow::no_duplicate_changed, this,
          &MainWindow::on_config_changed_no_duplicate);
  history_list_button = std::make_unique<QPushButton>("历史记录");
  connect(history_list_button.get(), &QPushButton::clicked, this,
          &MainWindow::on_history_list_button_clicked);
  clear_history_button = std::make_unique<QPushButton>("清空历史");
  connect(clear_history_button.get(), &QPushButton::clicked, this,
          &MainWindow::on_clear_history_button_clicked);
  more_act_button = std::make_unique<QPushButton>("更多");
  connect(more_act_button.get(), &QPushButton::clicked, this,
          &MainWindow::on_more_act_button_clicked);
  h_layout->addStretch();
  h_layout->addWidget(no_dup_box.get());
  h_layout->addWidget(history_list_button.get());
  h_layout->addWidget(clear_history_button.get());
  h_layout->addWidget(more_act_button.get());
  rand_button = std::make_unique<QPushButton>(getTextForButton());
  connect(rand_button.get(), &QPushButton::clicked, this,
          &MainWindow::on_rand_button_clicked);
  text_display = std::make_unique<AutoResizeLabel>(this);
  connect(text_display.get(), &AutoResizeLabel::clicked, this,
          &MainWindow::on_text_display_clicked);
  if (max_name_pos != -1) {
    text_display->setRefText(names[max_name_pos]);
  }
  v_layout = std::make_unique<QVBoxLayout>(this);
  v_layout->addWidget(settings.get(), 1);
  v_layout->addWidget(text_display.get(), 10);
  v_layout->addWidget(rand_button.get(), 1);
  timer = std::make_unique<QTimer>(this);
  connect(timer.get(), &QTimer::timeout, this, &MainWindow::on_timer_timeout);

  connect(this, &MainWindow::show_setting_changed, this,
          &MainWindow::on_config_changed_show_setting);

  config_dialogue = std::make_unique<ConfigDialogue>(*this);
  config_dialogue->hide();

  history_view = std::make_unique<QListView>();
  history_model = std::make_unique<HistoryListModel>(history, names);
  history_view->setModel(history_model.get());
  history_view->hide();

  sc_pick = std::make_unique<QShortcut>(QKeySequence(Qt::Key_P), this,
                                        [this]() { on_rand_button_clicked(); });
  sc_no_dup = std::make_unique<QShortcut>(
      QKeySequence(Qt::Key_N), this,
      [this]() { maybe_change_no_duplicate(!config.no_duplicate); });
  sc_no_animation = std::make_unique<QShortcut>(
      QKeySequence(Qt::Key_M), this,
      [this]() { maybe_change_no_animation(!config.no_animation); });
  sc_show_setting = std::make_unique<QShortcut>(
      QKeySequence(Qt::Key_B), this,
      [this]() { maybe_change_show_setting(!config.show_setting); });
}

void MainWindow::load_name_list(bool refresh_ui) {
  QFile fin(config.names_path);
  if (!fin.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return;
  }
  std::string line;
  static const char spaces[] = " \t\n\r\f\v";
  int max_line_len = 0;
  int max_line_pos = -1;
  names.clear();
  while (true) {
    auto line = fin.readLine();
    if (line.isEmpty()) {
      break;
    }
    auto name = QString::fromUtf8(line).trimmed();
    if (name.isEmpty()) {
      continue;
    }
    auto name_len = name.size();
    if (name_len > max_line_len) {
      max_line_pos = names.size();
      max_line_len = name_len;
    }
    names.emplace_back(name.data());
  }
  max_name_len = max_line_len;
  max_name_pos = max_line_pos;
  history.reset(names.size());
  if (refresh_ui) {
    maybe_redraw_history_view();
    text_display->setRefText(names[max_name_pos]);
  }
}

bool MainWindow::reload_config(bool no_update_ui) {
  QString conf_path = config.config_path;
  QFile conf_file(conf_path);
  if (!conf_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }
  QByteArray content = conf_file.readAll();
  auto doc = QJsonDocument::fromJson(content);
  if (!doc.isObject()) {
    return false;
  }
  auto obj = doc.object();
  QFile default_conf(MainWindowConfig::get_default_config_path());
  if (!default_conf.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }
  if (referes_to_same_file(conf_file, default_conf)) {
    // Only read secondary config file of default config file.
    auto iter = obj.constFind(MainWindowConfig::field_config_path);
    if (iter != obj.end() && iter->isString()) {
      conf_path = iter->toString();
      QFile second_conf(conf_path);
      if (!second_conf.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
      }
      QByteArray content = second_conf.readAll();
      auto doc = QJsonDocument::fromJson(content);
      if (!doc.isObject()) {
        return false;
      }
      obj = doc.object();
    }
  }
  saved_config = MainWindowConfig();
  saved_config.update_from_json(obj);

  MainWindowConfig new_config = saved_config;
  new_config.config_path = conf_path;
  if (!no_update_ui) {
    refresh_ui_on_reload_config(new_config);
  } else {
    config = new_config;
  }
  return true;
}

void MainWindow::refresh_ui_on_reload_config(
    const MainWindowConfig& new_config) {
#define IMPL(X, JSON_KIND, KIND) maybe_change_##X(new_config.X);
  MAIN_WINDOW_CONFIG_FIELDS(IMPL)
#undef IMPL
}

bool MainWindow::save_config(const QString& path_) {
  bool config_dirty = !(saved_config == config);
  bool is_save_as = !path_.isEmpty();
  if (!config_dirty && !is_save_as) {
    return false;
  }
  const auto& config_path = is_save_as ? path_ : config.config_path;
  QFile conf_file(config_path);
  if (!conf_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return false;
  }
  QFile default_conf_write(MainWindowConfig::get_default_config_path());
  if (!default_conf_write.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return false;
  }
  bool is_default_conf = referes_to_same_file(conf_file, default_conf_write);
  auto json_obj = config.to_json(is_default_conf);
  QJsonDocument doc(json_obj);
  auto content = doc.toJson();
  if (conf_file.write(content) != content.size()) {
    return false;
  }
  if (!is_default_conf) {
    QFile default_conf_read(MainWindowConfig::get_default_config_path());
    if (!default_conf_read.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return false;
    }
    QByteArray content = default_conf_read.readAll();
    auto doc = QJsonDocument::fromJson(content);
    QJsonObject default_conf_obj;
    if (doc.isObject()) {
      default_conf_obj = doc.object();
    }
    default_conf_obj[MainWindowConfig::field_config_path] = config_path;
    QJsonDocument new_doc(default_conf_obj);
    auto new_content = new_doc.toJson();
    if (default_conf_write.write(new_content) != new_content.size()) {
      return false;
    }
  }
  if (!is_save_as) {
    saved_config = config;
  }
  return true;
}

MainWindow::~MainWindow() { save_config(); }
