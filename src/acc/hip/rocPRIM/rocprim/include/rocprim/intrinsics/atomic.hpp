// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef ROCPRIM_INTRINSICS_ATOMIC_HPP_
#define ROCPRIM_INTRINSICS_ATOMIC_HPP_

#include "src/acc/hip/rocPRIM/rocprim/include/rocprim/device/../config.hpp"
#include "src/acc/hip/rocPRIM/rocprim/include/rocprim/rocprim.hpp"
BEGIN_ROCPRIM_NAMESPACE

namespace detail
{
    ROCPRIM_DEVICE inline
    unsigned int atomic_add(unsigned int * address, unsigned int value)
    {
        #ifdef ROCPRIM_HC_API
            return hc::atomic_fetch_add(address, value);
        #else
            return ::atomicAdd(address, value);
        #endif
    }

    ROCPRIM_DEVICE inline
    int atomic_add(int * address, int value)
    {
        #ifdef ROCPRIM_HC_API
            return hc::atomic_fetch_add(address, value);
        #else
            return ::atomicAdd(address, value);
        #endif
    }

    ROCPRIM_DEVICE inline
    float atomic_add(float * address, float value)
    {
        #ifdef ROCPRIM_HC_API
            return hc::atomic_fetch_add(address, value);
        #else
            return ::atomicAdd(address, value);
        #endif
    }

    ROCPRIM_DEVICE inline
    unsigned long long atomic_add(unsigned long long * address, unsigned long long value)
    {
        #ifdef ROCPRIM_HC_API
            return hc::atomic_fetch_add(reinterpret_cast<uint64_t*>(address), static_cast<uint64_t>(value));
        #else
            return ::atomicAdd(address, value);
        #endif
    }

    ROCPRIM_DEVICE inline
    unsigned int atomic_wrapinc(unsigned int * address, unsigned int value)
    {
        #ifdef ROCPRIM_HC_API
            return hc::__atomic_wrapinc(address, value);
        #else
            return ::atomicInc(address, value);
        #endif
    }

    ROCPRIM_DEVICE inline
    unsigned int atomic_exch(unsigned int * address, unsigned int value)
    {
        #ifdef ROCPRIM_HC_API
            return hc::atomic_exchange(address, value);
        #else
            return ::atomicExch(address, value);
        #endif
    }

    ROCPRIM_DEVICE inline
    unsigned long long atomic_exch(unsigned long long * address, unsigned long long value)
    {
        #ifdef ROCPRIM_HC_API
            return hc::atomic_exchange(reinterpret_cast<uint64_t*>(address), static_cast<uint64_t>(value));
        #else
            return ::atomicExch(address, value);
        #endif
    }
}

END_ROCPRIM_NAMESPACE

#endif // ROCPRIM_INTRINSICS_ATOMIC_HPP_
