///////////////////////////////////////////////////////////////////////
// File:        simddetect.h
// Description: Architecture detector.
// Author:      Stefan Weil (based on code from Ray Smith)
//
// (C) Copyright 2014, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////
#ifndef TESSERACT_ARCH_SIMDDETECT_H_
#define TESSERACT_ARCH_SIMDDETECT_H_

#include <tesseract/export.h>
#include "tesstypes.h"

namespace tesseract {

// Function pointer for best calculation of dot product.
using DotProductFunction = TFloat (*)(const TFloat *, const TFloat *, int);
extern DotProductFunction DotProduct;

// Architecture detector. Add code here to detect any other architectures for
// SIMD-based faster dot product functions. Intended to be a single static
// object, but it does no real harm to have more than one.
class SIMDDetect {
public:
  // Returns true if AVX is available on this system.
  static inline bool IsAVXAvailable() {
    return detector.avx_available_;
  }
  // Returns true if AVX2 (integer support) is available on this system.
  static inline bool IsAVX2Available() {
    return detector.avx2_available_;
  }
  // Returns true if AVX512 Foundation (float) is available on this system.
  static inline bool IsAVX512FAvailable() {
    return detector.avx512F_available_;
  }
  // Returns true if AVX512 integer is available on this system.
  static inline bool IsAVX512BWAvailable() {
    return detector.avx512BW_available_;
  }
  // Returns true if AVX512 Vector Neural Network Instructions are available.
  static inline bool IsAVX512VNNIAvailable() {
    return detector.avx512VNNI_available_;
  }
  // Returns true if FMA is available on this system.
  static inline bool IsFMAAvailable() {
    return detector.fma_available_;
  }
  // Returns true if SSE4.1 is available on this system.
  static inline bool IsSSEAvailable() {
    return detector.sse_available_;
  }
  // Returns true if NEON is available on this system.
  static inline bool IsNEONAvailable() {
    return detector.neon_available_;
  }

  // Update settings after config variable was set.
  static TESS_API void Update();
  /* willus mod */
  static char *simd_method() { return detector.method; }
  static TESS_API char method[16];

private:
  // Constructor, must set all static member variables.
  SIMDDetect();

private:
  // Singleton.
  static SIMDDetect detector;
  // If true, then AVX has been detected.
  static TESS_API bool avx_available_;
  static TESS_API bool avx2_available_;
  static TESS_API bool avx512F_available_;
  static TESS_API bool avx512BW_available_;
  static TESS_API bool avx512VNNI_available_;
  // If true, then FMA has been detected.
  static TESS_API bool fma_available_;
  // If true, then SSe4.1 has been detected.
  static TESS_API bool sse_available_;
  // If true, then NEON has been detected.
  static TESS_API bool neon_available_;
};

} // namespace tesseract

#endif // TESSERACT_ARCH_SIMDDETECT_H_
