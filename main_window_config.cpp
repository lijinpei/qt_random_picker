#include "main_window_config.h"

QString MainWindowConfig::get_default_config_path() { return "config.json"; }

QJsonObject MainWindowConfig::to_json(bool is_default_conf) const {
#define IMPL(X, JSON_KIND, KIND) obj.insert(field_##X, X);
  QJsonObject obj;
  MAIN_WINDOW_CONFIG_FIELDS(IMPL)
#undef IMPL
  if (!is_default_conf) {
    auto iter = obj.find(field_config_path);
    if (iter != obj.end()) {
      obj.erase(iter);
    }
  }
  return obj;
}

bool MainWindowConfig::update_from_json(const QJsonObject &obj) {
  bool changed = false;
#define IMPL(X, JSON_KIND, KIND)                            \
  {                                                         \
    auto iter = obj.constFind(field_##X);                   \
    if (iter != obj.end()) {                                \
      auto new_variant = iter->toVariant();                 \
      auto new_value_ptr = get_if<JSON_KIND>(&new_variant); \
      if (new_value_ptr) {                                  \
        decltype(X) new_value = *new_value_ptr;             \
        if (X != new_value) {                               \
          X = new_value;                                    \
          changed = true;                                   \
        }                                                   \
      }                                                     \
    }                                                       \
  }
  MAIN_WINDOW_CONFIG_FIELDS(IMPL)
#undef IMPL
  return changed;
}

bool MainWindowConfig::operator==(const MainWindowConfig &other) const {
#define IMPL(X, JSIN_KIND, KIND) &&X == other.X
  return true MAIN_WINDOW_CONFIG_FIELDS(IMPL);
#undef IMPL
}
