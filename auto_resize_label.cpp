#include "auto_resize_label.h"

void AutoResizeLabel::calc_bounding_boxes() {
  bbs.clear();
  if (ref_str) {
    for (const auto font_metric : font_metrics) {
      bbs.push_back(font_metric.boundingRect(*ref_str));
    }
  }
}

AutoResizeLabel::AutoResizeLabel(QWidget *parent)
    : QScrollArea(parent), label(std::make_unique<QLabel>(this)) {
  setAlignment(Qt::AlignCenter);
  setWidgetResizable(true);
  label->setAlignment(Qt::AlignCenter);
  label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setWidget(label.get());
  for (int i = 0; i < num_fonts; ++i) {
    fonts.emplace_back("Helvetica", 2 * i + 1, QFont::Bold);
    font_metrics.emplace_back(fonts.back());
  }
  calc_bounding_boxes();
}

void AutoResizeLabel::resizeEvent(QResizeEvent *event) {
  if (bbs.empty()) {
    return;
  }
  auto size = event->size();
  auto compare = [](const QRect &rect, const QSize &size) {
    return rect.height() < size.height() && rect.width() < size.width();
  };
  if (compare(bbs.back(), size)) {
    label->setFont(fonts.back());
    return;
  }
  if (!compare(bbs[0], size)) {
    label->setFont(fonts[0]);
    return;
  }
  auto iter = std::lower_bound(bbs.begin(), bbs.end(), size, compare);
  int idx = std::distance(bbs.begin(), iter) - 1;
  idx = std::max<int>(idx, 0);
  idx = std::min<int>(idx, fonts.size() - 1);
  label->setFont(fonts[idx]);
}
