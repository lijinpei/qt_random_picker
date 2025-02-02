#pragma once

#include <iterator>
#include <vector>

class HistoryRecorder {
  std::vector<int> visit_order;
  std::vector<int> reorder_map;
  std::vector<int> reverse_map;
  void remove_item_impl(int n, int pos) {
    visit_order.push_back(n);
    auto last_val = std::exchange(reorder_map.back(), n);
    reorder_map.pop_back();
    if (last_val != n) {
      reorder_map[pos] = last_val;
      reverse_map[last_val] = pos;
    }
    reverse_map[n] = -1;
  }

 public:
  void reset() {
    visit_order.clear();
    reorder_map.clear();
    reverse_map.clear();
  }
  void reset(int capacity) {
    reset();
    reorder_map.reserve(capacity);
    reverse_map.reserve(capacity);
    for (int i = 0; i < capacity; ++i) {
      reorder_map.push_back(i);
      reverse_map.push_back(i);
    }
  }
  int get_history_count() const { return visit_order.size(); }
  int get_history_item(int index) const {
    if (index >= 0 && index < visit_order.size()) {
      return visit_order[visit_order.size() - 1 - index];
    }
    return visit_order.back();
  }
  int get_remaining_count() const { return reorder_map.size(); }
  int get_total_count() const { return reverse_map.size(); }
  void record_pick(int val) {
    if (val < 0 || val >= reverse_map.size()) {
      return;
    }
    auto pos = reverse_map[val];
    if (pos >= 0) {
      remove_item_impl(val, pos);
    }
  }
  int peek_no_duplicate(int pos) const {
    if (pos >= 0 && pos < reorder_map.size()) {
      return reorder_map[pos];
    }
    return 0;
  }
};
