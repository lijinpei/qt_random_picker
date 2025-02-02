#pragma once

#include <algorithm>
#include <iterator>
#include <memory>
#include <vector>

#include "QFont"
#include "QFontMetrics"
#include "QLabel"
#include "QMouseEvent"
#include "QRect"
#include "QResizeEvent"
#include "QScrollArea"
#include "QString"
#include "QWidget"

class AutoResizeLabel : public QScrollArea {
  Q_OBJECT
  std::unique_ptr<QLabel> label;
  const QString *ref_str = nullptr;
  std::vector<QFont> fonts;
  std::vector<QFontMetrics> font_metrics;
  std::vector<QRect> bbs;
  static constexpr int num_fonts = 100;
  void calc_bounding_boxes();

 signals:
  void clicked();

 protected:
  void mousePressEvent(QMouseEvent *event) { emit clicked(); }

 public:
  AutoResizeLabel(QWidget *parent);
  void setRefText(const QString &new_ref) {
    this->ref_str = &new_ref;
    calc_bounding_boxes();
  }
  void resizeEvent(QResizeEvent *event) override;
  auto setText(const QString &text) { return label->setText(text); }
};
