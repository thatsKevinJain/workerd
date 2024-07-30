// Copyright (c) 2017-2022 Cloudflare, Inc.
// Licensed under the Apache 2.0 license found in the LICENSE file or at:
//     https://opensource.org/licenses/Apache-2.0

#include <kj/common.h>
#include <kj/debug.h>
#include <kj/one-of.h>
#include <kj/string.h>

struct UConverter;

namespace workerd::api::node {

enum class Encoding {
  ASCII,
  LATIN1,
  UTF8,
  UTF16LE,
  BASE64,
  BASE64URL,
  HEX,
};

namespace i18n {

// Used by BufferUtil::transcode.
constexpr bool canBeTranscoded(Encoding encoding) noexcept {
  switch (encoding) {
  case Encoding::ASCII:
  case Encoding::LATIN1:
  case Encoding::UTF16LE:
  case Encoding::UTF8:
    return true;
  default:
    return false;
  }
}

class Converter final {
public:
  explicit Converter(Encoding encoding, kj::StringPtr substitude = ""_kj);
  KJ_DISALLOW_COPY_AND_MOVE(Converter);

  UConverter* conv() const;
  size_t maxCharSize() const;
  size_t minCharSize() const;
  void reset();
  void setSubstituteChars(kj::StringPtr sub);

private:
  kj::Own<UConverter> conv_;
};

kj::Array<kj::byte> transcode(kj::ArrayPtr<kj::byte> source, Encoding fromEncoding,
                              Encoding toEncoding);

} // namespace i18n

} // namespace workerd::api::node

KJ_DECLARE_NON_POLYMORPHIC(UConverter)
