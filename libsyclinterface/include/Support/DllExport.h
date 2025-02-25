//===---------  DllExport.h - Decalres dllexport for Windows     -*-C++-*- ===//
//
//                      Data Parallel Control (dpctl)
//
// Copyright 2020-2022 Intel Corporation
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
/// This file defines a "__declspec(dllexport)" wrapper for windows.
///
//===----------------------------------------------------------------------===//

#pragma once

#ifdef _WIN32
#ifdef DPCTLSyclInterface_EXPORTS
#define DPCTL_API __declspec(dllexport)
#else
#define DPCTL_API __declspec(dllimport)
#endif
#else
#define DPCTL_API
#endif
