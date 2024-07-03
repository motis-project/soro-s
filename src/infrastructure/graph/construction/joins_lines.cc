#include "soro/infrastructure/graph/construction/joins_lines.h"

#include <cstdint>

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/graph/section.h"

namespace soro::infra {

bool joins_lines(end_element const&) { return false; }

bool joins_lines(track_element const&) { return false; }

bool joins_lines(simple_element const&) { return false; }

bool joins_lines(simple_switch const& s) {
  using enum simple_switch::direction;

  return !(s.get_line(start) == s.get_line(stem) &&
           s.get_line(start) == s.get_line(branch));
}

bool joins_lines(cross const& c) {
  using enum cross::direction;

  return !(c.get_line(start_left) == c.get_line(end_left) &&
           c.get_line(start_left) == c.get_line(start_right) &&
           c.get_line(start_right) == c.get_line(end_right));
}

bool misaligned_join(end_element const&) { return false; }

bool misaligned_join(track_element const&) { return false; }

bool misaligned_join(simple_element const&) { return false; }

bool misaligned_join(simple_switch const& s, graph const& g) {
  using enum simple_switch::direction;

  // if we don't join any lines we cant have a misaligned join
  if (!joins_lines(s)) {
    return false;
  }

  // check if the different directions start a section
  auto const start_pos = g.sections_.get_section_position(s.id_, start);
  auto const stem_pos = g.sections_.get_section_position(s.id_, stem);
  auto const branch_pos = g.sections_.get_section_position(s.id_, branch);

  auto const start_count = static_cast<uint32_t>(is_start(start_pos)) +
                           static_cast<uint32_t>(is_start(stem_pos)) +
                           static_cast<uint32_t>(is_start(branch_pos));

  // when start is at the start of a section:
  //    we only have an aligned join if we have just a single start direction
  // when start is at the end of a section
  //    we only have an aligned join if we have exactly two start directions

  return is_start(start_pos) ? start_count != 1 : start_count != 2;

  //  auto const join_count =
  //      (static_cast<int>(s.dir_) + static_cast<int>(s.stem_dir_) +
  //       static_cast<int>(s.branch_dir_));

  //  return is_rising(s.dir_) ? join_count != 1 : join_count != 2;
}

bool misaligned_join(cross const& c, graph const& g) {
  using enum cross::direction;

  // if we don't join any lines we cant have a misaligned join
  if (!joins_lines(c)) {
    return false;
  }

  auto const sl_pos = g.sections_.get_section_position(c.id_, start_left);
  auto const el_pos = g.sections_.get_section_position(c.id_, end_left);
  auto const sr_pos = g.sections_.get_section_position(c.id_, start_right);
  auto const er_pos = g.sections_.get_section_position(c.id_, end_right);

  auto const start_count = static_cast<uint32_t>(is_start(sl_pos)) +
                           static_cast<uint32_t>(is_start(el_pos)) +
                           static_cast<uint32_t>(is_start(sr_pos)) +
                           static_cast<uint32_t>(is_start(er_pos));

  // we have a misaligned join if we have not exactly two starts in the cross
  auto const not_two = start_count != 2;

  // or if the start right section does not align with the end right section
  auto const right_misaligned = sr_pos == er_pos;

  return not_two || right_misaligned;

  //  return joins_lines(c) &&
  //         ((static_cast<int>(c.dir_) + static_cast<int>(c.end_left_dir_) +
  //           static_cast<int>(c.start_right_dir_) +
  //           static_cast<int>(c.end_right_dir_)) != 2 ||
  //          c.start_right_dir_ == c.end_right_dir_);
}

}  // namespace soro::infra
