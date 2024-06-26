// SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "tt_eager/tt_dnn/op_library/eltwise_unary/eltwise_unary_op.hpp"
#include "tt_eager/tt_dnn/op_library/run_operation.hpp"
#include "tt_eager/tt_dnn/op_library/composite/composite_ops.hpp"
#include "ttnn/operations/core.hpp"
#include "ttnn/decorators.hpp"
#include "ttnn/validation.hpp"

namespace ttnn {

namespace operations {

namespace unary {

using UnaryOpType = tt::tt_metal::UnaryOpType;

namespace detail {

inline const std::array<ttnn::TensorSchema, 1> input_tensor_schemas() {
    return {ttnn::TensorSchema{
        2,
        4,
        {ttnn::bfloat16, ttnn::bfloat8_b},
        {ttnn::TILE_LAYOUT, ttnn::ROW_MAJOR_LAYOUT},
        true,
        false,
        false,
        false}};
}

template <typename... Args>
inline auto input_tensors_to_validate(const Tensor& input_tensor, Args&&... args) {
    return std::make_tuple(input_tensor);
};

inline Tensor execute(
    const Tensor& input_tensor,
    const std::vector<tt::tt_metal::UnaryWithParam>& op_chain,
    const std::optional<MemoryConfig>& memory_config = std::nullopt) {
    bool fp32_dest_acc_en = input_tensor.get_dtype() == DataType::UINT32 or
                            input_tensor.get_dtype() == DataType::INT32;  // MT: Currently only uint32/int32 is moved to
                                                                          // DST directly, fp32 is converted to fp16b
    return operation::run(
               EltwiseUnary{op_chain, memory_config.value_or(input_tensor.memory_config()), fp32_dest_acc_en},
               {input_tensor})
        .at(0);
}
}  // namespace detail

template <UnaryOpType unary_op_type>
struct Unary : public EltwiseUnary {
    static const std::array<TensorSchema, 1> input_tensor_schemas() { return detail::input_tensor_schemas(); }

    template <typename... Args>
    static auto input_tensors_to_validate(const Tensor& input_tensor, Args&&... args) {
        return detail::input_tensors_to_validate(input_tensor, std::forward<Args>(args)...);
    }

    static Tensor execute(const Tensor& input_tensor, const std::optional<MemoryConfig>& memory_config = std::nullopt) {
        return detail::execute(
            input_tensor, {UnaryWithParam{unary_op_type}}, memory_config);
    }
};

struct Exp : public EltwiseUnary {
    static const std::array<TensorSchema, 1> input_tensor_schemas() { return detail::input_tensor_schemas(); }

    template <typename... Args>
    static auto input_tensors_to_validate(const Tensor& input_tensor, Args&&... args) {
        return detail::input_tensors_to_validate(input_tensor, std::forward<Args>(args)...);
    }

    static Tensor execute(
        const Tensor& input_tensor,
        const bool parameter = false,
        const std::optional<MemoryConfig>& memory_config = std::nullopt) {
        return detail::execute(
            input_tensor,
            {UnaryWithParam{
                ttnn::operations::unary::UnaryOpType::EXP, static_cast<float>(parameter)}},
            memory_config);
    }
};

struct Softplus : public EltwiseUnary {
    static const std::array<TensorSchema, 1> input_tensor_schemas() { return detail::input_tensor_schemas(); }

    template <typename... Args>
    static auto input_tensors_to_validate(const Tensor& input_tensor, Args&&... args) {
        return detail::input_tensors_to_validate(input_tensor, std::forward<Args>(args)...);
    }

    static Tensor execute(const Tensor& input, const float beta, const float threshold, const std::optional<MemoryConfig>& memory_config_arg = std::nullopt) {
        auto original_input_shape = input.get_shape();
        auto input_4D = ttnn::unsqueeze_to_4D(input);

        auto memory_config = memory_config_arg.value_or(input_4D.memory_config());
        auto result = tt::tt_metal::softplus(input_4D, beta, threshold, memory_config);

        result = ttnn::reshape(result, original_input_shape);

        return result;
    }
};
}  // namespace unary
}  // namespace operations

constexpr auto exp = ttnn::register_operation<ttnn::operations::unary::Exp>("ttnn::exp");

constexpr auto softplus = ttnn::register_operation<ttnn::operations::unary::Softplus>("ttnn::softplus");

constexpr auto silu =
    ttnn::register_operation<ttnn::operations::unary::Unary<ttnn::operations::unary::UnaryOpType::SILU>>("ttnn::silu");

} // namespace ttnn
