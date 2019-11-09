/*
 * Tiny self-contained version of the PCG Random Number Generation for C++
 * put together from pieces of the much larger C/C++ codebase.
 * Wenzel Jakob, February 2015
 *
 * The PCG random number generator was developed by Melissa O'Neill
 * <oneill@pcg-random.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For additional information about the PCG random number generation scheme,
 * including its license and other licensing options, visit
 *
 *     http://www.pcg-random.org
 */

#pragma once

#include "mg_common.h"
#include "mg_macros.h"

namespace mg {

// PCG32 Pseudorandom number generator
struct pcg32 {
  static constexpr auto Pcg32DefaultState  = 0x853c49e6748fea9bull;
  static constexpr auto Pcg32DefaultStream = 0xda3e39cb94b95bdbull;
  static constexpr auto Pcg32Mult          = 0x5851f42d4c957f2dull;
  u64 State;  // RNG state.  All values are possible.
  u64 Inc;    // Controls which RNG sequence (stream) is selected. Must *always* be odd.
  // Initialize the pseudorandom number generator with default seed
  pcg32();
  // Initialize the pseudorandom number generator with the Seed() function
  pcg32(u64 Initstate, u64 Initseq = 1);
};

/* Seed the pseudorandom number generator
  *
  * Specified in two parts: a state initializer and a sequence selection
  * constant (a.k.a. stream id) */
void Seed(pcg32* Pcg, u64 InitState, u64 InitSeq = 1);

// Generate a uniformly distributed unsigned 32-bit random number
u32 NextUInt(pcg32* Pcg);

// Generate a uniformly distributed number, r, where 0 <= r < bound
u32 NextUInt(pcg32* Pcg, u32 Bound);

// Generate a single precision floating point value on the interval [0, 1)
f32 NextFloat(pcg32* Pcg);

/* Generate a double precision floating point value on the interval [0, 1)
  *
  * Since the underlying random number generator produces 32 bit output,
  * only the first 32 mantissa bits will be filled (however, the resolution is
  * still finer than in NextFloat(), which only uses 23 mantissa bits) */
f64 NextDouble(pcg32* Pcg);

/* Multi-step advance function (jump-ahead, jump-back)
  *
  * The method used here is based on Brown, "Random Number Generation
  * with Arbitrary Stride", Transactions of the American Nuclear
  * Society (Nov. 1994). The algorithm is very similar to fast exponentiation. */
void Advance(i64 Delta_);

/* Draw uniformly distributed permutation and permute the given STL container
  *
  * From: Knuth, TAoCP Vol. 2 (3rd 3d), Section 3.4.2 */
mg_T(it) void Shuffle(it Begin, it End);

// Compute the distance between two PCG32 pseudorandom number generators
i64 operator-(const pcg32& First, const pcg32& Second);

// Equality operator
bool operator==(const pcg32& First, const pcg32& Second);

// Inequality operator
bool operator!=(const pcg32& First, const pcg32& Second);

} // namespace mg

#include "mg_random.inl"
