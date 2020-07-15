//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcpp-no-rtti

// <typeindex>

// class type_index

// size_t hash_code() const;

// UNSUPPORTED: -fno-rtti

#include <typeindex>
#include <cassert>

#include "test_macros.h"

int main(int, char**)
{
    const std::type_info& ti = typeid(int);
    std::type_index t1 = typeid(int);
    assert(t1.hash_code() == ti.hash_code());

  return 0;
}
