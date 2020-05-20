// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/third_party/quiche/src/quic/core/http/quic_header_list.h"

#include <limits>
#include <string>

#include "net/third_party/quiche/src/quic/core/quic_packets.h"
#include "net/third_party/quiche/src/quic/platform/api/quic_flags.h"
#include "net/third_party/quiche/src/common/platform/api/quiche_string_piece.h"
#include "net/third_party/quiche/src/spdy/core/spdy_protocol.h"

namespace quic {

QuicHeaderList::QuicHeaderList()
    : max_header_list_size_(std::numeric_limits<size_t>::max()),
      current_header_list_size_(0),
      uncompressed_header_bytes_(0),
      compressed_header_bytes_(0) {}

QuicHeaderList::QuicHeaderList(QuicHeaderList&& other) = default;

QuicHeaderList::QuicHeaderList(const QuicHeaderList& other) = default;

QuicHeaderList& QuicHeaderList::operator=(const QuicHeaderList& other) =
    default;

QuicHeaderList& QuicHeaderList::operator=(QuicHeaderList&& other) = default;

QuicHeaderList::~QuicHeaderList() {}

void QuicHeaderList::OnHeaderBlockStart() {
  QUIC_BUG_IF(current_header_list_size_ != 0)
      << "OnHeaderBlockStart called more than once!";
}

void QuicHeaderList::OnHeader(quiche::QuicheStringPiece name,
                              quiche::QuicheStringPiece value) {
  // Avoid infinite buffering of headers. No longer store headers
  // once the current headers are over the limit.
  if (current_header_list_size_ < max_header_list_size_) {
    current_header_list_size_ += name.size();
    current_header_list_size_ += value.size();
    current_header_list_size_ += spdy::kPerHeaderOverhead;
    header_list_.emplace_back(std::string(name), std::string(value));
  }
}

void QuicHeaderList::OnHeaderBlockEnd(size_t uncompressed_header_bytes,
                                      size_t compressed_header_bytes) {
  uncompressed_header_bytes_ = uncompressed_header_bytes;
  compressed_header_bytes_ = compressed_header_bytes;
  if (current_header_list_size_ > max_header_list_size_) {
    Clear();
  }
}

void QuicHeaderList::Clear() {
  header_list_.clear();
  current_header_list_size_ = 0;
  uncompressed_header_bytes_ = 0;
  compressed_header_bytes_ = 0;
}

std::string QuicHeaderList::DebugString() const {
  std::string s = "{ ";
  for (const auto& p : *this) {
    s.append(p.first + "=" + p.second + ", ");
  }
  s.append("}");
  return s;
}

}  // namespace quic
