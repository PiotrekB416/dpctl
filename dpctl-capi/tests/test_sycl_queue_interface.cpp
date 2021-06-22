//===------ test_sycl_queue_interface.cpp - Test cases for queue interface ===//
//
//                      Data Parallel Control (dpctl)
//
// Copyright 2020-2021 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file has unit test cases for functions defined in
/// dpctl_sycl_queue_interface.h.
///
//===----------------------------------------------------------------------===//

#include "Support/CBindingWrapping.h"
#include "dpctl_sycl_context_interface.h"
#include "dpctl_sycl_device_interface.h"
#include "dpctl_sycl_device_manager.h"
#include "dpctl_sycl_device_selector_interface.h"
#include "dpctl_sycl_event_interface.h"
#include "dpctl_sycl_queue_interface.h"
#include "dpctl_sycl_queue_manager.h"
#include <CL/sycl.hpp>
#include <gtest/gtest.h>

using namespace cl::sycl;

namespace
{
DEFINE_SIMPLE_CONVERSION_FUNCTIONS(queue, DPCTLSyclQueueRef);

struct TestDPCTLQueueMemberFunctions
    : public ::testing::TestWithParam<const char *>
{
protected:
    DPCTLSyclQueueRef QRef = nullptr;

    TestDPCTLQueueMemberFunctions()
    {
        auto DS = DPCTLFilterSelector_Create(GetParam());
        DPCTLSyclDeviceRef DRef = nullptr;
        if (DS) {
            EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DS));
            EXPECT_NO_FATAL_FAILURE(QRef = DPCTLQueue_CreateForDevice(
                                        DRef, nullptr, DPCTL_DEFAULT_PROPERTY));
        }
        DPCTLDevice_Delete(DRef);
        DPCTLDeviceSelector_Delete(DS);
    }

    void SetUp()
    {
        if (!QRef) {
            auto message = "Skipping as no device of type " +
                           std::string(GetParam()) + ".";
            GTEST_SKIP_(message.c_str());
        }
    }

    ~TestDPCTLQueueMemberFunctions()
    {
        EXPECT_NO_FATAL_FAILURE(DPCTLQueue_Delete(QRef));
    }
};

} /* End of anonymous namespace */

TEST(TestDPCTLSyclQueueInterface, CheckCreateForDevice)
{
    /* We are testing that we do not crash even when input is NULL. */
    DPCTLSyclQueueRef QRef = nullptr;

    EXPECT_NO_FATAL_FAILURE(
        QRef = DPCTLQueue_CreateForDevice(nullptr, nullptr, 0));
    ASSERT_TRUE(QRef == nullptr);
}

TEST(TestDPCTLSyclQueueInterface, CheckCopy)
{
    DPCTLSyclQueueRef Q1 = nullptr;
    DPCTLSyclQueueRef Q2 = nullptr;
    DPCTLSyclDeviceRef DRef = nullptr;

    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_Create());
    EXPECT_NO_FATAL_FAILURE(
        Q1 = DPCTLQueue_CreateForDevice(DRef, nullptr, DPCTL_DEFAULT_PROPERTY));
    ASSERT_TRUE(Q1);
    EXPECT_NO_FATAL_FAILURE(Q2 = DPCTLQueue_Copy(Q1));
    EXPECT_TRUE(bool(Q2));
    EXPECT_NO_FATAL_FAILURE(DPCTLQueue_Delete(Q1));
    EXPECT_NO_FATAL_FAILURE(DPCTLQueue_Delete(Q2));
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
}

TEST(TestDPCTLSyclQueueInterface, CheckCopy_Invalid)
{
    DPCTLSyclQueueRef Q1 = nullptr;
    DPCTLSyclQueueRef Q2 = nullptr;

    EXPECT_NO_FATAL_FAILURE(Q2 = DPCTLQueue_Copy(Q1));
    EXPECT_NO_FATAL_FAILURE(DPCTLQueue_Delete(Q1));
    EXPECT_NO_FATAL_FAILURE(DPCTLQueue_Delete(Q2));
}

TEST(TestDPCTLSyclQueueInterface, CheckAreEq_False)
{
    DPCTLSyclDeviceSelectorRef DSRef = nullptr;
    DPCTLSyclDeviceRef DRef = nullptr;
    DPCTLSyclQueueRef Q1 = nullptr;
    DPCTLSyclQueueRef Q2 = nullptr;

    EXPECT_NO_FATAL_FAILURE(DSRef = DPCTLDefaultSelector_Create());
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    EXPECT_NO_FATAL_FAILURE(
        Q1 = DPCTLQueue_CreateForDevice(DRef, nullptr, DPCTL_DEFAULT_PROPERTY));
    EXPECT_NO_FATAL_FAILURE(
        Q2 = DPCTLQueue_CreateForDevice(DRef, nullptr, DPCTL_DEFAULT_PROPERTY));
    EXPECT_FALSE(DPCTLQueue_AreEq(Q1, Q2));
    EXPECT_FALSE(DPCTLQueue_Hash(Q1) == DPCTLQueue_Hash(Q2));
    auto C0 = DPCTLQueue_GetContext(Q1);
    auto C1 = DPCTLQueue_GetContext(Q2);
    // All the queues should share the same context
    EXPECT_TRUE(DPCTLContext_AreEq(C0, C1));
    EXPECT_NO_FATAL_FAILURE(DPCTLContext_Delete(C0));
    EXPECT_NO_FATAL_FAILURE(DPCTLContext_Delete(C1));
    EXPECT_NO_FATAL_FAILURE(DPCTLQueue_Delete(Q1));
    EXPECT_NO_FATAL_FAILURE(DPCTLQueue_Delete(Q2));
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
    EXPECT_NO_FATAL_FAILURE(DPCTLDeviceSelector_Delete(DSRef));
}

TEST(TestDPCTLSyclQueueInterface, CheckAreEq_True)
{
    DPCTLSyclDeviceSelectorRef DSRef = nullptr;
    DPCTLSyclDeviceRef DRef = nullptr;
    DPCTLSyclQueueRef Q1 = nullptr;
    DPCTLSyclQueueRef Q2 = nullptr;

    EXPECT_NO_FATAL_FAILURE(DSRef = DPCTLDefaultSelector_Create());
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    EXPECT_NO_FATAL_FAILURE(
        Q1 = DPCTLQueue_CreateForDevice(DRef, nullptr, DPCTL_DEFAULT_PROPERTY));
    EXPECT_NO_FATAL_FAILURE(Q2 = DPCTLQueue_Copy(Q1));
    EXPECT_TRUE(DPCTLQueue_AreEq(Q1, Q2));
    EXPECT_TRUE(DPCTLQueue_Hash(Q1) == DPCTLQueue_Hash(Q2));
    EXPECT_NO_FATAL_FAILURE(DPCTLQueue_Delete(Q1));
    EXPECT_NO_FATAL_FAILURE(DPCTLQueue_Delete(Q2));
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
    EXPECT_NO_FATAL_FAILURE(DPCTLDeviceSelector_Delete(DSRef));
}

TEST(TestDPCTLSyclQueueInterface, CheckAreEq_Invalid)
{
    DPCTLSyclDeviceSelectorRef DSRef = nullptr;
    DPCTLSyclDeviceRef DRef = nullptr;
    DPCTLSyclQueueRef Q1 = nullptr;
    DPCTLSyclQueueRef Q2 = nullptr;

    EXPECT_FALSE(DPCTLQueue_AreEq(Q1, Q2));
    EXPECT_NO_FATAL_FAILURE(DSRef = DPCTLDefaultSelector_Create());
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    EXPECT_NO_FATAL_FAILURE(
        Q1 = DPCTLQueue_CreateForDevice(DRef, nullptr, DPCTL_DEFAULT_PROPERTY));
    EXPECT_FALSE(DPCTLQueue_AreEq(Q1, Q2));
    EXPECT_FALSE(DPCTLQueue_Hash(Q1) == DPCTLQueue_Hash(Q2));

    EXPECT_NO_FATAL_FAILURE(DPCTLQueue_Delete(Q1));
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
    EXPECT_NO_FATAL_FAILURE(DPCTLDeviceSelector_Delete(DSRef));
}

TEST(TestDPCTLSyclQueueInterface, CheckHash_Invalid)
{
    DPCTLSyclQueueRef Q1 = nullptr;
    DPCTLSyclQueueRef Q2 = nullptr;
    EXPECT_TRUE(DPCTLQueue_Hash(Q1) == 0);
    EXPECT_TRUE(DPCTLQueue_Hash(Q2) == 0);
}

TEST(TestDPCTLSyclQueueInterface, CheckGetBackend_Invalid)
{
    DPCTLSyclQueueRef Q = nullptr;
    DPCTLSyclBackendType Bty = DPCTL_UNKNOWN_BACKEND;
    EXPECT_NO_FATAL_FAILURE(Bty = DPCTLQueue_GetBackend(Q));
    EXPECT_TRUE(Bty == DPCTL_UNKNOWN_BACKEND);
}

TEST(TestDPCTLSyclQueueInterface, CheckGetContext_Invalid)
{
    DPCTLSyclQueueRef Q = nullptr;
    DPCTLSyclContextRef CRef = nullptr;
    EXPECT_NO_FATAL_FAILURE(CRef = DPCTLQueue_GetContext(Q));
    EXPECT_TRUE(CRef == nullptr);
}

TEST(TestDPCTLSyclQueueInterface, CheckGetDevice_Invalid)
{
    DPCTLSyclQueueRef Q = nullptr;
    DPCTLSyclDeviceRef DRef = nullptr;
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLQueue_GetDevice(Q));
    EXPECT_TRUE(DRef == nullptr);
}

TEST(TestDPCTLSyclQueueInterface, CheckIsInOrder)
{
    bool ioq = true;
    DPCTLSyclDeviceSelectorRef DSRef = nullptr;
    DPCTLSyclDeviceRef DRef = nullptr;
    DPCTLSyclQueueRef Q1 = nullptr;
    DPCTLSyclQueueRef Q2 = nullptr;

    EXPECT_NO_FATAL_FAILURE(DSRef = DPCTLDefaultSelector_Create());
    EXPECT_NO_FATAL_FAILURE(DRef = DPCTLDevice_CreateFromSelector(DSRef));
    EXPECT_NO_FATAL_FAILURE(
        Q1 = DPCTLQueue_CreateForDevice(DRef, nullptr, DPCTL_DEFAULT_PROPERTY));
    EXPECT_NO_FATAL_FAILURE(ioq = DPCTLQueue_IsInOrder(Q1));
    EXPECT_FALSE(ioq);

    EXPECT_NO_FATAL_FAILURE(
        Q2 = DPCTLQueue_CreateForDevice(DRef, nullptr, DPCTL_IN_ORDER));
    EXPECT_NO_FATAL_FAILURE(ioq = DPCTLQueue_IsInOrder(Q2));
    EXPECT_TRUE(ioq);

    EXPECT_NO_FATAL_FAILURE(DPCTLQueue_Delete(Q1));
    EXPECT_NO_FATAL_FAILURE(DPCTLQueue_Delete(Q2));
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(DRef));
    EXPECT_NO_FATAL_FAILURE(DPCTLDeviceSelector_Delete(DSRef));
}

TEST(TestDPCTLSyclQueueInterface, CheckIsInOrder_Invalid)
{
    bool ioq = true;
    DPCTLSyclQueueRef Q1 = nullptr;
    EXPECT_NO_FATAL_FAILURE(ioq = DPCTLQueue_IsInOrder(Q1));
    EXPECT_FALSE(ioq);
}

TEST_P(TestDPCTLQueueMemberFunctions, CheckGetBackend)
{
    auto q = unwrap(QRef);
    auto Backend = q->get_device().get_platform().get_backend();
    auto Bty = DPCTLQueue_GetBackend(QRef);
    switch (Bty) {
    case DPCTL_CUDA:
        EXPECT_TRUE(Backend == backend::cuda);
        break;
    case DPCTL_HOST:
        EXPECT_TRUE(Backend == backend::host);
        break;
    case DPCTL_LEVEL_ZERO:
        EXPECT_TRUE(Backend == backend::level_zero);
        break;
    case DPCTL_OPENCL:
        EXPECT_TRUE(Backend == backend::opencl);
        break;
    default:
        FAIL();
    }
}

TEST_P(TestDPCTLQueueMemberFunctions, CheckGetContext)
{
    auto Ctx = DPCTLQueue_GetContext(QRef);
    ASSERT_TRUE(Ctx != nullptr);
    EXPECT_NO_FATAL_FAILURE(DPCTLContext_Delete(Ctx));
}

TEST_P(TestDPCTLQueueMemberFunctions, CheckGetDevice)
{
    auto D = DPCTLQueue_GetDevice(QRef);
    ASSERT_TRUE(D != nullptr);
    EXPECT_NO_FATAL_FAILURE(DPCTLDevice_Delete(D));
}

INSTANTIATE_TEST_SUITE_P(DPCTLQueueMemberFuncTests,
                         TestDPCTLQueueMemberFunctions,
                         ::testing::Values("opencl:gpu:0",
                                           "opencl:cpu:0",
                                           "level_zero:gpu:0"));
