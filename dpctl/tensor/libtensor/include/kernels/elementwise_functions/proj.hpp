//=== proj.hpp -   Unary function CONJ                     ------
//*-C++-*--/===//
//
//                      Data Parallel Control (dpctl)
//
// Copyright 2020-2023 Intel Corporation
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
//===---------------------------------------------------------------------===//
///
/// \file
/// This file defines kernels for elementwise evaluation of PROJ(x) function.
//===---------------------------------------------------------------------===//

#pragma once
#include <CL/sycl.hpp>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "kernels/elementwise_functions/common.hpp"

#include "utils/offset_utils.hpp"
#include "utils/type_dispatch.hpp"
#include "utils/type_utils.hpp"
#include <pybind11/pybind11.h>

namespace dpctl
{
namespace tensor
{
namespace kernels
{
namespace proj
{

namespace py = pybind11;
namespace td_ns = dpctl::tensor::type_dispatch;

using dpctl::tensor::type_utils::is_complex;

template <typename argT, typename resT> struct ProjFunctor
{

    // is function constant for given argT
    using is_constant = typename std::false_type;
    // constant value, if constant
    // constexpr resT constant_value = resT{};
    // is function defined for sycl::vec
    using supports_vec = typename std::false_type;
    // do both argTy and resTy support sugroup store/load operation
    using supports_sg_loadstore = typename std::false_type;

    resT operator()(const argT &in)
    {
        using realT = typename argT::value_type;
        const realT x = std::real(in);
        const realT y = std::imag(in);

        if (std::isinf(x) || std::isinf(y)) {
            const realT res_im = std::copysign(realT(0), y);
            return resT{std::numeric_limits<realT>::infinity(), res_im};
        }
        return in;
    }
};

template <typename argTy,
          typename resTy = argTy,
          unsigned int vec_sz = 4,
          unsigned int n_vecs = 2>
using ProjContigFunctor = elementwise_common::
    UnaryContigFunctor<argTy, resTy, ProjFunctor<argTy, resTy>, vec_sz, n_vecs>;

template <typename argTy, typename resTy, typename IndexerT>
using ProjStridedFunctor = elementwise_common::
    UnaryStridedFunctor<argTy, resTy, IndexerT, ProjFunctor<argTy, resTy>>;

template <typename T> struct ProjOutputType
{
    using value_type = typename std::disjunction< // disjunction is C++17
                                                  // feature, supported by DPC++
        td_ns::TypeMapResultEntry<T, std::complex<float>>,
        td_ns::TypeMapResultEntry<T, std::complex<double>>,
        td_ns::DefaultResultEntry<void>>::result_type;
};

template <typename T1, typename T2, unsigned int vec_sz, unsigned int n_vecs>
class proj_contig_kernel;

template <typename argTy>
sycl::event proj_contig_impl(sycl::queue exec_q,
                             size_t nelems,
                             const char *arg_p,
                             char *res_p,
                             const std::vector<sycl::event> &depends = {})
{
    return elementwise_common::unary_contig_impl<
        argTy, ProjOutputType, ProjContigFunctor, proj_contig_kernel>(
        exec_q, nelems, arg_p, res_p, depends);
}

template <typename fnT, typename T> struct ProjContigFactory
{
    fnT get()
    {
        if constexpr (std::is_same_v<typename ProjOutputType<T>::value_type,
                                     void>) {
            fnT fn = nullptr;
            return fn;
        }
        else {
            fnT fn = proj_contig_impl<T>;
            return fn;
        }
    }
};

template <typename fnT, typename T> struct ProjTypeMapFactory
{
    /*! @brief get typeid for output type of std::proj(T x) */
    std::enable_if_t<std::is_same<fnT, int>::value, int> get()
    {
        using rT = typename ProjOutputType<T>::value_type;
        return td_ns::GetTypeid<rT>{}.get();
    }
};

template <typename T1, typename T2, typename T3> class proj_strided_kernel;

template <typename argTy>
sycl::event
proj_strided_impl(sycl::queue exec_q,
                  size_t nelems,
                  int nd,
                  const py::ssize_t *shape_and_strides,
                  const char *arg_p,
                  py::ssize_t arg_offset,
                  char *res_p,
                  py::ssize_t res_offset,
                  const std::vector<sycl::event> &depends,
                  const std::vector<sycl::event> &additional_depends)
{
    return elementwise_common::unary_strided_impl<
        argTy, ProjOutputType, ProjStridedFunctor, proj_strided_kernel>(
        exec_q, nelems, nd, shape_and_strides, arg_p, arg_offset, res_p,
        res_offset, depends, additional_depends);
}

template <typename fnT, typename T> struct ProjStridedFactory
{
    fnT get()
    {
        if constexpr (std::is_same_v<typename ProjOutputType<T>::value_type,
                                     void>) {
            fnT fn = nullptr;
            return fn;
        }
        else {
            fnT fn = proj_strided_impl<T>;
            return fn;
        }
    }
};

} // namespace proj
} // namespace kernels
} // namespace tensor
} // namespace dpctl
