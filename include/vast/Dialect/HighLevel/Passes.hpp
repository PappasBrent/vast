// Copyright (c) 2021-present, Trail of Bits, Inc.

#pragma once

#include "vast/Util/Warnings.hpp"

VAST_RELAX_WARNINGS
#include <mlir/IR/Operation.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Pass/PassManager.h>
VAST_UNRELAX_WARNINGS

#include <vast/Dialect/HighLevel/HighLevelDialect.hpp>
#include <vast/Util/Pipeline.hpp>

#include <memory>

namespace vast::hl {
    std::unique_ptr< mlir::Pass > createHLLowerTypesPass();

    std::unique_ptr< mlir::Pass > createExportFnInfoPass();

    std::unique_ptr< mlir::Pass > createDCEPass();

    std::unique_ptr< mlir::Pass > createLowerTypeDefsPass();

    std::unique_ptr< mlir::Pass > createSpliceTrailingScopes();

/// Generate the code for registering passes.
#define GEN_PASS_REGISTRATION
#include "vast/Dialect/HighLevel/Passes.h.inc"

    namespace pipeline {
        pipeline_step_ptr canonicalize();

        pipeline_step_ptr desugar();

        pipeline_step_ptr simplify();

        pipeline_step_ptr stdtypes();
    } // namespace pipeline

} // namespace vast::hl
