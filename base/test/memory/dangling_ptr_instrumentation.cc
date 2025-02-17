// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "base/test/memory/dangling_ptr_instrumentation.h"

#include <cstdint>

#include "base/allocator/partition_alloc_features.h"
#include "base/allocator/partition_allocator/dangling_raw_ptr_checks.h"
#include "base/check_op.h"
#include "base/feature_list.h"
#include "base/memory/raw_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace base::test {

// static
base::expected<DanglingPtrInstrumentation, base::StringPiece>
DanglingPtrInstrumentation::Create() {
#if BUILDFLAG(USE_PARTITION_ALLOC)
  if (!FeatureList::IsEnabled(features::kPartitionAllocBackupRefPtr)) {
    return base::unexpected(
        "DanglingPtrInstrumentation requires the feature flag "
        "'PartitionAllocBackupRefPtr' to be on.");
  }
#endif
  // Note: We don't need to enable the `PartitionAllocDanglingPtr` feature,
  // because this does provide an alternative "implementation", by incrementing
  // the two counters.

#if !BUILDFLAG(USE_PARTITION_ALLOC_AS_MALLOC)
  return base::unexpected(
      "DanglingPtrInstrumentation requires the binary flag "
      "'use_partition_alloc_as_malloc' to be on.");
#elif !BUILDFLAG(ENABLE_DANGLING_RAW_PTR_CHECKS)
  return base::unexpected(
      "DanglingPtrInstrumentation requires the binary flag "
      "'enable_dangling_raw_ptr_checks' to be on.");
#elif BUILDFLAG(ENABLE_DANGLING_RAW_PTR_PERF_EXPERIMENT)
  return base::unexpected(
      "DanglingPtrInstrumentation requires the binary flag "
      "'enable_dangling_raw_ptr_perf_experiment' to be off.");
#else
  return DanglingPtrInstrumentation();
#endif
}

DanglingPtrInstrumentation::DanglingPtrInstrumentation() {
  Register();
}

DanglingPtrInstrumentation::~DanglingPtrInstrumentation() {
  Unregister();
}

DanglingPtrInstrumentation::DanglingPtrInstrumentation(
    DanglingPtrInstrumentation&& old) {
  operator=(std::move(old));
}

DanglingPtrInstrumentation& DanglingPtrInstrumentation::operator=(
    DanglingPtrInstrumentation&& old) {
  old.Unregister();
  Register();
  return *this;
}

void DanglingPtrInstrumentation::Register() {
  CHECK_EQ(g_observer, nullptr);
  g_observer = this;
#if BUILDFLAG(USE_PARTITION_ALLOC)
  old_detected_fn_ = partition_alloc::GetDanglingRawPtrDetectedFn();
  old_dereferenced_fn_ = partition_alloc::GetDanglingRawPtrReleasedFn();
  partition_alloc::SetDanglingRawPtrDetectedFn(IncreaseCountDetected);
  partition_alloc::SetDanglingRawPtrReleasedFn(IncreaseCountReleased);
#endif
}

void DanglingPtrInstrumentation::Unregister() {
  if (g_observer != this) {
    return;
  }
  g_observer = nullptr;
#if BUILDFLAG(USE_PARTITION_ALLOC)
  partition_alloc::SetDanglingRawPtrDetectedFn(old_detected_fn_);
  partition_alloc::SetDanglingRawPtrReleasedFn(old_dereferenced_fn_);
#endif
}

raw_ptr<DanglingPtrInstrumentation> DanglingPtrInstrumentation::g_observer =
    nullptr;

// static
void DanglingPtrInstrumentation::IncreaseCountDetected(std::uintptr_t) {
  g_observer->dangling_ptr_detected_++;
}

// static
void DanglingPtrInstrumentation::IncreaseCountReleased(std::uintptr_t) {
  g_observer->dangling_ptr_released_++;
}

}  // namespace base::test
