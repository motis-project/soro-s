#pragma once

#include <iterator>

namespace soro::utls {

template <typename Iterable>
concept is_input_iterable = requires(Iterable i) {
                              { std::begin(i) } -> std::input_iterator;
                            };

template <typename Iterable>
concept is_forward_iterable = requires(Iterable i) {
                                { std::begin(i) } -> std::forward_iterator;
                              };

template <typename Iterable>
concept is_bidirectional_iterable = requires(Iterable i) {
                                      {

                                        std::begin(i)

                                        } -> std::bidirectional_iterator;
                                    };

template <typename Iterable>
concept is_random_access_iterable = requires(Iterable i) {
                                      {
                                        std::begin(i)
                                        } -> std::random_access_iterator;
                                    };

template <typename Iterable>
concept is_contiguous_iterable = requires(Iterable i) {
                                   {
                                     std::begin(i)
                                     } -> std::contiguous_iterator;
                                 };

}  // namespace soro::utls
