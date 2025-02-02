#pragma once

#include "QJsonObject"
#include "QString"

struct MainWindowConfig {
  QString config_path = get_default_config_path();
  static constexpr char field_config_path[] = "config path";

  QString names_path = "name.txt";
  static constexpr char field_names_path[] = "names path";

  bool no_animation = false;
  static constexpr char field_no_animation[] = "no animation";

  int time_slice_ms = 150;
  static constexpr char field_time_slice_ms[] = "refresh time";

  bool show_setting = false;
  static constexpr char field_show_setting[] = "show setting";

  bool no_duplicate = false;
  static constexpr char field_no_duplicate[] = "no duplicate";

  QString history_path = "history.txt";
  static constexpr char field_history_path[] = "history path";

  bool auto_load_history = false;
  static constexpr char field_auto_load_history[] = "auto load history";

  bool auto_save_history = false;
  static constexpr char field_auto_save_history[] = "auto save history";

  QJsonObject to_json(bool is_default_conf) const;
  bool update_from_json(const QJsonObject &obj);
  bool operator==(const MainWindowConfig &other) const;

  static QString get_default_config_path();
};

#define MAIN_WINDOW_CONFIG_FIELDS(impl)                                  \
  impl(config_path, QString, QString) impl(names_path, QString, QString) \
      impl(no_animation, bool, bool) impl(time_slice_ms, qlonglong, int) \
          impl(show_setting, bool, bool) impl(no_duplicate, bool, bool)  \
              impl(history_path, QString, QString)
