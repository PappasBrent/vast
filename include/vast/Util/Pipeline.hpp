// Copyright (c) 2023-present, Trail of Bits, Inc.

#pragma once

#include "vast/Util/Warnings.hpp"

VAST_RELAX_WARNINGS
#include <llvm/ADT/DenseSet.h>
#include <mlir/IR/DialectRegistry.h>
#include <mlir/Pass/PassManager.h>
#include <mlir/Pass/PassRegistry.h>
#include <mlir/Pass/Pass.h>
VAST_UNRELAX_WARNINGS

#include "vast/Util/Common.hpp"

#include "gap/core/generator.hpp"

namespace vast {

    //
    // pipeline_step is a single step in the pipeline that is a pass or a list
    // of pipelines
    //
    // Each step defiens a list of dependencies, which are scheduled before the step
    //
    struct pipeline_step;

    using pipeline_step_ptr = std::unique_ptr< pipeline_step >;


    //
    // pipeline is a pass manager, which keeps track of duplicit passes and does
    // not schedule them twice
    //
    struct pipeline_t : mlir::PassManager
    {
        using base      = mlir::PassManager;
        using pass_id_t = mlir::TypeID;

        using base::base;

        void addPass(std::unique_ptr< mlir::Pass > pass);

        template< typename parent_t >
        void addNestedPass(std::unique_ptr< mlir::Pass > pass) {
            auto id = pass->getTypeID();
            if (seen.count(id)) {
                return;
            }

            seen.insert(id);
            base::addNestedPass< parent_t >(std::move(pass));
        }

        friend pipeline_t &operator<<(pipeline_t &ppl, pipeline_step_ptr pass);

        llvm::DenseSet< pass_id_t > seen;
    };


    using pipeline_step_builder = std::function< pipeline_step_ptr(void) >;

    //
    // initilizer wrapper to setup dependencies after make is called
    //
    template< typename step_t >
    struct pipeline_step_init : pipeline_step_ptr {
        using pipeline_step_ptr::pipeline_step_ptr;

        template< typename ...args_t >
        pipeline_step_init(args_t &&...args)
            : pipeline_step_ptr(std::make_unique< step_t >(
                std::forward< args_t >(args)...
            ))
        {}

        template< typename ...deps_t >
        pipeline_step_init depends_on(deps_t &&... deps) && {
            get()->depends_on(std::forward< deps_t >(deps)...);
            return std::move(*this);
        }
    };

    struct pipeline_step
    {
        explicit pipeline_step(
            std::vector< pipeline_step_builder > dependencies
        )
            : dependencies(std::move(dependencies))
        {}

        explicit pipeline_step() = default;

        virtual ~pipeline_step() = default;

        virtual void schedule_on(pipeline_t &ppl) const;

        virtual string_ref name() const = 0;

        void schedule_dependencies(pipeline_t &ppl) const;

        template< typename ...deps_t >
        void depends_on(deps_t &&... deps) {
            (dependencies.emplace_back(std::forward< deps_t >(deps)), ...);
        }

        std::vector< pipeline_step_builder > dependencies;
    };

    using pass_builder_t = llvm::function_ref< std::unique_ptr< mlir::Pass >(void) >;

    struct pass_pipeline_step : pipeline_step
    {
        explicit pass_pipeline_step(pass_builder_t builder)
            : pass_builder(builder)
        {}

        void schedule_on(pipeline_t &ppl) const override;

        string_ref name() const override;

    protected:
        pass_builder_t pass_builder;
    };

    template< typename parent_t >
    struct nested_pass_pipeline_step : pass_pipeline_step
    {
        explicit nested_pass_pipeline_step(pass_builder_t builder)
            : pass_pipeline_step(builder)
        {}

        void schedule_on(pipeline_t &ppl) const override {
            schedule_dependencies(ppl);
            ppl.addNestedPass< parent_t >(pass_builder());
        }
    };

    // compound step represents subpipeline to be run
    struct compound_pipeline_step : pipeline_step
    {
        template< typename... steps_t >
        explicit compound_pipeline_step(string_ref name, steps_t &&...steps)
            : pipeline_name(name), steps{ std::forward< steps_t >(steps)... }
        {}

        void schedule_on(pipeline_t &ppl) const override;

        string_ref name() const override;

    protected:
        std::string pipeline_name;
        std::vector< pipeline_step_builder > steps;
    };

    template< typename... args_t >
    decltype(auto) pass(args_t &&... args) {
        return pipeline_step_init< pass_pipeline_step >(
            std::forward< args_t >(args)...
        );
    }

    template< typename parent, typename... args_t >
    decltype(auto) nested(args_t &&... args) {
        return pipeline_step_init< nested_pass_pipeline_step< parent > >(
            std::forward< args_t >(args)...
        );
    }

    template< typename... steps_t >
    decltype(auto) compose(string_ref name, steps_t &&...steps) {
        return pipeline_step_init< compound_pipeline_step >(
            name, std::forward< steps_t >(steps)...
        );
    }

} // namespace vast
