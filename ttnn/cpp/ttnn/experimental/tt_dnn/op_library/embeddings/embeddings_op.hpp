// SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <optional>

#include "ttnn/run_operation.hpp"
#include "ttnn/tensor/tensor.hpp"

using namespace tt::constants;

namespace tt {

namespace tt_metal {

enum class EmbeddingsType { GENERIC, PADDED, BINARY };
enum class EmbeddingsIndexType { UINT32, BFP16};

struct Embeddings {
    const MemoryConfig output_mem_config;
    const bool tilized;
    const EmbeddingsType embeddings_type;
    const std::optional<uint32_t> pad_token;
    const DataType output_dtype;

    void validate(const std::vector<Tensor> &input_tensors) const;
    std::vector<Shape> compute_output_shapes(const std::vector<Tensor> &input_tensors) const;
    std::vector<Tensor> create_output_tensors(const std::vector<Tensor> &input_tensors) const;
    operation::ProgramWithCallbacks create_program(
        const std::vector<Tensor> &input_tensors, std::vector<Tensor> &output_tensors) const;
};

inline Tensor embeddings(
    const Tensor &input_tensor,
    const Tensor &weights,
    bool tilized = true,
    EmbeddingsType embeddings_type = EmbeddingsType::GENERIC,
    std::optional<uint32_t> pad_token = std::nullopt,
    const MemoryConfig &mem_config = operation::DEFAULT_OUTPUT_MEMORY_CONFIG,
    std::optional<const DataType> output_dtype = std::nullopt) {
    std::vector<Tensor> output_tensors = {Tensor(operation::get_workers_for_op_output({input_tensor, weights}))};
    operation::launch_op(
        [tilized, embeddings_type, pad_token, mem_config, output_dtype] (const std::vector<Tensor>& input_tensors, const std::vector<std::optional<const Tensor>>& optional_input_tensors, const std::vector<std::optional<Tensor>>& optional_output_tensors) mutable -> std::vector<Tensor> {
            auto& input_tensor = input_tensors.at(0);
            auto& weights = input_tensors.at(1);
            return operation::run_without_autoformat(
               Embeddings{
                   .output_mem_config = mem_config,
                   .tilized = tilized,
                   .embeddings_type = embeddings_type,
                   .pad_token = pad_token,
                   .output_dtype = output_dtype.value_or(weights.get_dtype())},
               {input_tensor, weights});
        }, {input_tensor, weights}, output_tensors);
    return output_tensors.at(0);
}

}  // namespace tt_metal
}  // namespace tt
