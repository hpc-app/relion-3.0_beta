// MIT License
//
// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>
#include <vector>
#include <algorithm>

// Google Test
#include <gtest/gtest.h>

// HIP API
#include <hip/hip_runtime.h>
#include <hip/hip_hcc.h>
// rocPRIM API
#include <rocprim/rocprim.hpp>

#include "test_utils.hpp"

#define HIP_CHECK(error)         \
    ASSERT_EQ(static_cast<hipError_t>(error),hipSuccess)

namespace rp = rocprim;

// Params for tests
template<
    class KeyType,
    class ValueType
>
struct DeviceMergeParams
{
    using key_type = KeyType;
    using value_type = ValueType;
};

template<class Params>
class RocprimDeviceMergeTests : public ::testing::Test
{
public:
    using key_type = typename Params::key_type;
    using value_type = typename Params::value_type;
    const bool debug_synchronous = false;
};

using custom_int2 = test_utils::custom_test_type<int>;
using custom_double2 = test_utils::custom_test_type<double>;

typedef ::testing::Types<
    DeviceMergeParams<int, double>,
    DeviceMergeParams<unsigned long, unsigned int>,
    DeviceMergeParams<float, custom_double2>,
    DeviceMergeParams<int, float>,
    DeviceMergeParams<custom_double2, custom_int2>,
    DeviceMergeParams<custom_int2, char>
> RocprimDeviceMergeTestsParams;

// size1, size2
std::vector<std::tuple<size_t, size_t>> get_sizes()
{
    std::vector<std::tuple<size_t, size_t>> sizes = {
        std::make_tuple(2, 1),
        std::make_tuple(10, 10),
        std::make_tuple(111, 111),
        std::make_tuple(128, 1289),
        std::make_tuple(12, 1000),
        std::make_tuple(123, 3000),
        std::make_tuple(1024, 512),
        std::make_tuple(2345, 49),
        std::make_tuple(17867, 41),
        std::make_tuple(17867, 34567),
        std::make_tuple(34567, (1 << 17) - 1220),
        std::make_tuple(924353, 1723454),
    };
    return sizes;
}

TYPED_TEST_CASE(RocprimDeviceMergeTests, RocprimDeviceMergeTestsParams);

TYPED_TEST(RocprimDeviceMergeTests, MergeKey)
{
    using key_type = typename TestFixture::key_type;
    const bool debug_synchronous = TestFixture::debug_synchronous;

    hipStream_t stream = 0; // default

    for(auto sizes : get_sizes())
    {
        SCOPED_TRACE(
            testing::Message() << "with sizes = {" <<
            std::get<0>(sizes) << ", " << std::get<1>(sizes) << "}"
        );

        const size_t size1 = std::get<0>(sizes);
        const size_t size2 = std::get<1>(sizes);

        // Generate data
        std::vector<key_type> keys_input1 = test_utils::get_random_data<key_type>(size1, 0, size1);
        std::vector<key_type> keys_input2 = test_utils::get_random_data<key_type>(size2, 0, size2);
        std::sort(keys_input1.begin(), keys_input1.end());
        std::sort(keys_input2.begin(), keys_input2.end());
        std::vector<key_type> keys_output(size1 + size2, 0);

        test_utils::out_of_bounds_flag out_of_bounds;

        key_type * d_keys_input1;
        key_type * d_keys_input2;
        key_type * d_keys_output;
        HIP_CHECK(hipMalloc(&d_keys_input1, keys_input1.size() * sizeof(key_type)));
        HIP_CHECK(hipMalloc(&d_keys_input2, keys_input2.size() * sizeof(key_type)));
        HIP_CHECK(hipMalloc(&d_keys_output, keys_output.size() * sizeof(key_type)));
        HIP_CHECK(
            hipMemcpy(
                d_keys_input1, keys_input1.data(),
                keys_input1.size() * sizeof(key_type),
                hipMemcpyHostToDevice
            )
        );
        HIP_CHECK(
            hipMemcpy(
                d_keys_input2, keys_input2.data(),
                keys_input2.size() * sizeof(key_type),
                hipMemcpyHostToDevice
            )
        );
        HIP_CHECK(hipDeviceSynchronize());

        // Calculate expected results on host
        std::vector<key_type> expected(keys_output.size());
        std::merge(
            keys_input1.begin(),
            keys_input1.end(),
            keys_input2.begin(),
            keys_input2.end(),
            expected.begin()
        );

        test_utils::bounds_checking_iterator<key_type> d_keys_checking_output(
            d_keys_output,
            out_of_bounds.device_pointer(),
            size1 + size2
        );

        // compare function
        ::rocprim::less<key_type> lesser_op;
        // temp storage
        size_t temp_storage_size_bytes;
        void * d_temp_storage = nullptr;
        // Get size of d_temp_storage
        HIP_CHECK(
            rocprim::merge(
                d_temp_storage, temp_storage_size_bytes,
                d_keys_input1, d_keys_input2,
                d_keys_checking_output,
                keys_input1.size(), keys_input2.size(),
                lesser_op, stream, debug_synchronous
            )
        );

        // temp_storage_size_bytes must be >0
        ASSERT_GT(temp_storage_size_bytes, 0);

        // allocate temporary storage
        HIP_CHECK(hipMalloc(&d_temp_storage, temp_storage_size_bytes));
        HIP_CHECK(hipDeviceSynchronize());

        // Run
        HIP_CHECK(
            rocprim::merge(
                d_temp_storage, temp_storage_size_bytes,
                d_keys_input1, d_keys_input2,
                d_keys_checking_output,
                keys_input1.size(), keys_input2.size(),
                lesser_op, stream, debug_synchronous
            )
        );
        HIP_CHECK(hipPeekAtLastError());
        HIP_CHECK(hipDeviceSynchronize());

        ASSERT_FALSE(out_of_bounds.get());

        // Copy keys_output to host
        HIP_CHECK(
            hipMemcpy(
                keys_output.data(), d_keys_output,
                keys_output.size() * sizeof(key_type),
                hipMemcpyDeviceToHost
            )
        );
        HIP_CHECK(hipDeviceSynchronize());

        // Check if keys_output values are as expected
        for(size_t i = 0; i < keys_output.size(); i++)
        {
            ASSERT_EQ(keys_output[i], expected[i]);
        }

        hipFree(d_keys_input1);
        hipFree(d_keys_input2);
        hipFree(d_keys_output);
        hipFree(d_temp_storage);
    }
}
