#ifndef __COMMON_H__
#define __COMMON_H__

#include "json.h"
#include "folly/FBString.h"
#include "folly/FBVector.h"

using Json = nlohmann::json;

using String = folly::fbstring;

template<typename T>
using Vector = folly::fbvector<T>;

template<typename T>
using UPtr = std::unique_ptr<T>;

#endif
