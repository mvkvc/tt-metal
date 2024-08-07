// SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ttnn/cpp/pybind11/decorators.hpp"
#include "ttnn/operations/eltwise/unary_backward/unary_backward.hpp"
#include "ttnn/operations/eltwise/binary_backward/binary_backward.hpp"
#include "ttnn/operations/eltwise/complex_unary_backward/complex_unary_backward.hpp"
#include "ttnn/operations/eltwise/complex_binary_backward/complex_binary_backward.hpp"
#include "ttnn/types.hpp"

namespace py = pybind11;

namespace ttnn {
namespace operations {
namespace unary_backward {

namespace detail {

//OpHandler_two_float : get_function_type1_w_two_float
template <typename unary_backward_operation_t>
void bind_unary_backward_two_float(py::module& module, const unary_backward_operation_t& operation, const std::string& description) {
    auto doc = fmt::format(
        R"doc({0}(grad_tensor: ttnn.Tensor, input_tensor: ttnn.Tensor, min: float, max: float, *, memory_config: ttnn.MemoryConfig) -> std::vector<Tensor>

        {2}

        Args:
            * :attr:`grad_tensor`
            * :attr:`input_tensor`
            * :attr:`float`
            * :attr:`float`

        Keyword args:
            * :attr:`memory_config` [ttnn.MemoryConfig]: memory config for the output tensor

        Example:

            >>> grad_tensor = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> input = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> output = {1}(grad_tensor, input, float, float)
        )doc",
        operation.base_name(),
        operation.python_fully_qualified_name(),
        description);

    bind_registered_operation(
        module,
        operation,
        doc,
        ttnn::pybind_overload_t{
            [](const unary_backward_operation_t& self,
               const ttnn::Tensor& grad_tensor,
               const ttnn::Tensor& input_tensor,
               float min,
               float max,
               const std::optional<MemoryConfig>& memory_config)  {
                return self(grad_tensor, input_tensor, min, max, memory_config);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor"),
            py::arg("min"),
            py::arg("max"),
            py::kw_only(),
            py::arg("memory_config") = std::nullopt}
    );
}

//OpHandler_two_float_with_default : get_function_type1_w_two_float_with_default
template <typename unary_backward_operation_t>
void bind_unary_backward_two_float_with_default(py::module& module, const unary_backward_operation_t& operation, const std::string& parameter_name_a, const std::string& parameter_a_doc, float parameter_a_value, const std::string& parameter_name_b, const std::string& parameter_b_doc, float parameter_b_value, const std::string& description) {
    auto doc = fmt::format(
        R"doc({0}(grad_tensor: ttnn.Tensor, input_tensor: ttnn.Tensor, {2}: float, {5}: float, *, memory_config: ttnn.MemoryConfig) -> std::vector<Tensor>

        {8}

        Args:
            * :attr:`grad_tensor`
            * :attr:`input_tensor`

        Keyword args:
            * :attr:`{2}` (float): {3} , Default value = {4}
            * :attr:`{5}` (float): {6} , Default value = {7}
            * :attr:`memory_config` [ttnn.MemoryConfig]: memory config for the output tensor

        Example:

            >>> grad_tensor = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> input = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> output = {1}(grad_tensor, input, {2} = {3}, {5} = {6})
        )doc",
        operation.base_name(),
        operation.python_fully_qualified_name(),
        parameter_name_a,
        parameter_a_doc,
        parameter_a_value,
        parameter_name_b,
        parameter_b_doc,
        parameter_b_value,
        description);

    bind_registered_operation(
        module,
        operation,
        doc,
        ttnn::pybind_overload_t{
            [](const unary_backward_operation_t& self,
               const ttnn::Tensor& grad_tensor,
               const ttnn::Tensor& input_tensor,
               float parameter_a,
               float parameter_b,
               const std::optional<MemoryConfig>& memory_config)  {
                return self(grad_tensor, input_tensor, parameter_a, parameter_b, memory_config);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor"),
            py::kw_only(),
            py::arg(parameter_name_a.c_str()) = parameter_a_value,
            py::arg(parameter_name_b.c_str()) = parameter_b_value,
            py::arg("memory_config") = std::nullopt}
    );
}

//OpHandler_optional_float_params_with_default : get_function_optional_float_params_with_default
template <typename unary_backward_operation_t>
void bind_unary_backward_optional_float_params_with_default(py::module& module, const unary_backward_operation_t& operation, const std::string& parameter_name_a, const std::string& parameter_a_doc, std::optional<float> parameter_a_value, const std::string& parameter_name_b, const std::string& parameter_b_doc, std::optional<float> parameter_b_value, const std::string& description) {
    auto doc = fmt::format(
        R"doc({0}(grad_tensor: ttnn.Tensor, input_tensor: ttnn.Tensor, {2}: float, {5}: float, *, memory_config: ttnn.MemoryConfig) -> std::vector<Tensor>

        {8}

        Args:
            * :attr:`grad_tensor`
            * :attr:`input_tensor`

        Keyword args:
            * :attr:`{2}` (Optional[float]): {3} , Default value = {4}
            * :attr:`{5}` (Optional[float]): {6} , Default value = {7}
            * :attr:`memory_config` [ttnn.MemoryConfig]: memory config for the output tensor

        Example:

            >>> grad_tensor = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> input = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> output = {1}(grad_tensor, input, {2} = {3}, {5} = {6})
        )doc",
        operation.base_name(),
        operation.python_fully_qualified_name(),
        parameter_name_a,
        parameter_a_doc,
        parameter_a_value,
        parameter_name_b,
        parameter_b_doc,
        parameter_b_value,
        description);

    bind_registered_operation(
        module,
        operation,
        doc,
        ttnn::pybind_overload_t{
            [](const unary_backward_operation_t& self,
               const ttnn::Tensor& grad_tensor,
               const ttnn::Tensor& input_tensor,
               std::optional<float> parameter_a,
               std::optional<float> parameter_b,
               const std::optional<MemoryConfig>& memory_config)  {
                return self(grad_tensor, input_tensor, parameter_a, parameter_b, memory_config);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor"),
            py::kw_only(),
            py::arg(parameter_name_a.c_str()) = parameter_a_value,
            py::arg(parameter_name_b.c_str()) = parameter_b_value,
            py::arg("memory_config") = std::nullopt}
    );
}

//OpHandler_float_string_default : get_function_type1_float_string_default
template <typename unary_backward_operation_t>
void bind_unary_backward_float_string_default(py::module& module, const unary_backward_operation_t& operation, const std::string& parameter_name_a, const std::string& parameter_a_doc, const std::string& parameter_name_b, const std::string& parameter_b_doc, string parameter_b_value, const std::string& description) {
    auto doc = fmt::format(
        R"doc({0}(grad_tensor: ttnn.Tensor, ( input_tensor: ttnn.Tensor, {2}: float ) or  ( input_tensor_a: ttnn.Tensor, input_tensor_b: ttnn.Tensor ), {4}: string, *, memory_config: ttnn.MemoryConfig) -> std::vector<Tensor>

        {7}

        Args:
            * :attr:`grad_tensor`
            * :attr:`input_tensor_a` or :attr:`input_tensor`
            * :attr:`input_tensor_b` or :attr:`{2}` (float): {3}

        Keyword args:
            * :attr:`{4}` (string): {5} , Default value = {6}
            * :attr:`memory_config` [ttnn.MemoryConfig]: memory config for the output tensor

        Example:

            >>> grad_tensor = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> input = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> output = {1}(grad_tensor, input, {2}, {4} = {6})
        )doc",
        operation.base_name(),
        operation.python_fully_qualified_name(),
        parameter_name_a,
        parameter_a_doc,
        parameter_name_b,
        parameter_b_doc,
        parameter_b_value,
        description);

    bind_registered_operation(
        module,
        operation,
        doc,
        ttnn::pybind_overload_t{
            [](const unary_backward_operation_t& self,
               const ttnn::Tensor& grad_tensor,
               const ttnn::Tensor& input_tensor,
               float parameter_a,
               string parameter_b,
               const std::optional<MemoryConfig>& memory_config)  {
                return self(grad_tensor, input_tensor, parameter_a, parameter_b, memory_config);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor"),
            py::arg(parameter_name_a.c_str()),
            py::kw_only(),
            py::arg(parameter_name_b.c_str()) = parameter_b_value,
            py::arg("memory_config") = std::nullopt},

        ttnn::pybind_overload_t{
            [operation](const unary_backward_operation_t& self,
               const ttnn::Tensor& grad_tensor,
               const ttnn::Tensor& input_tensor_a,
               const ttnn::Tensor& input_tensor_b,
               string parameter_b,
               const std::optional<ttnn::MemoryConfig>& memory_config) -> std::vector<ttnn::Tensor> {
                auto output_memory_config = memory_config.value_or(input_tensor_a.memory_config());
                using BinaryBackwardOp = ttnn::operations::binary_backward::ExecuteBinaryBackward<binary_backward::BinaryBackwardOpType::BIAS_GELU_BW>;
                if(operation.base_name()=="div_bw"){
                    using BinaryBackwardOp = ttnn::operations::binary_backward::ExecuteBinaryBackward<binary_backward::BinaryBackwardOpType::DIV_BW>;
                    return BinaryBackwardOp::execute_on_worker_thread(grad_tensor, input_tensor_a, parameter_b, input_tensor_b, output_memory_config);
                }
                return BinaryBackwardOp::execute_on_worker_thread(grad_tensor, input_tensor_a, parameter_b, input_tensor_b, output_memory_config);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor_a"),
            py::arg("input_tensor_b"),
            py::kw_only(),
            py::arg(parameter_name_b.c_str()) = parameter_b_value,
            py::arg("memory_config") = std::nullopt},

        ttnn::pybind_overload_t{
            [operation](const unary_backward_operation_t& self,
               const ComplexTensor& grad_tensor,
               const ComplexTensor& input_tensor_a,
               const ComplexTensor& input_tensor_b,
               const MemoryConfig& memory_config) -> std::vector<ComplexTensor> {
                using ComplexBinaryBackwardOp = ttnn::operations::complex_binary_backward::ExecuteComplexBinaryBackwardWoFloat<complex_binary_backward::ComplexBinaryBackwardOpType::COMPLEX_DIV_BW>;
                return ComplexBinaryBackwardOp::execute_on_main_thread(grad_tensor, input_tensor_a, input_tensor_b, memory_config);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor_a"),
            py::arg("input_tensor_b"),
            py::kw_only(),
            py::arg("memory_config")}
    );
}

//OpHandler_float_string_default : get_function_type1_float_string_default
template <typename unary_backward_operation_t>
void bind_unary_backward_string_default(py::module& module, const unary_backward_operation_t& operation, const std::string& parameter_name_a, const std::string& parameter_a_doc, string parameter_a_value, const std::string& description) {
    auto doc = fmt::format(
        R"doc({0}(grad_tensor: ttnn.Tensor, input_tensor: ttnn.Tensor, {2}: string, *, memory_config: ttnn.MemoryConfig) -> std::vector<Tensor>

        {5}

        Args:
            * :attr:`grad_tensor`
            * :attr:`input_tensor`

        Keyword args:
            * :attr:`{2}` (string): {3} , Default value = {4}
            * :attr:`memory_config` [ttnn.MemoryConfig]: memory config for the output tensor

        Example:

            >>> grad_tensor = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> input = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> output = {1}(grad_tensor, input, {2} = {4})
        )doc",
        operation.base_name(),
        operation.python_fully_qualified_name(),
        parameter_name_a,
        parameter_a_doc,
        parameter_a_value,
        description);

    bind_registered_operation(
        module,
        operation,
        doc,
        ttnn::pybind_overload_t{
            [](const unary_backward_operation_t& self,
               const ttnn::Tensor& grad_tensor,
               const ttnn::Tensor& input_tensor,
               string parameter_a,
               const std::optional<MemoryConfig>& memory_config)  {
                return self(grad_tensor, input_tensor, parameter_a, memory_config);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor"),
            py::kw_only(),
            py::arg(parameter_name_a.c_str()) = parameter_a_value,
            py::arg("memory_config") = std::nullopt});
}

//OpHandler_unary_optional_float : get_function_unary_optional_float
template <typename unary_backward_operation_t>
void bind_unary_backward_unary_optional_float(py::module& module, const unary_backward_operation_t& operation, const std::string& parameter_name, const std::string& parameter_doc, const std::string& description) {
    auto doc = fmt::format(
        R"doc({0}(grad_tensor: ttnn.Tensor, input_tensor: ttnn.Tensor, exponent: float, *, memory_config: ttnn.MemoryConfig) -> std::vector<std::optional<Tensor>>

        {4}

        Args:
            * :attr:`grad_tensor`
            * :attr:`input_tensor`
            * :attr:`{2}` (float): {3}

        Keyword args:
            * :attr:`memory_config` (Optional[ttnn.MemoryConfig]): memory config for the output tensor
            * :attr:`are_required_outputs` (Optional[std::vector<bool>]): List of bool, Default value is [True]
            * :attr:`input_grad` (Optional[ttnn.Tensor]): preallocated output tensor,
            * :attr:`queue_id` (Optional[uint8]): command queue id

        Example:

            >>> grad_tensor = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> tensor1 = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> output = {1}(grad_tensor, tensor, `{2}`)
        )doc",
        operation.base_name(),
        operation.python_fully_qualified_name(),
        parameter_name,
        parameter_doc,
        description);

    bind_registered_operation(
        module,
        operation,
        doc,
        ttnn::pybind_overload_t{
            [](const unary_backward_operation_t& self,
               const ttnn::Tensor& grad_tensor,
               const ttnn::Tensor& input_tensor,
               float parameter,
               const std::optional<ttnn::MemoryConfig>& memory_config,
               const std::vector<bool>& are_required_outputs,
               const std::optional<ttnn::Tensor>& input_grad,
               const uint8_t& queue_id) -> std::vector<optional<ttnn::Tensor>> {
                return self(queue_id, grad_tensor, input_tensor, parameter, memory_config, are_required_outputs, input_grad);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor"),
            py::arg(parameter_name.c_str()),
            py::kw_only(),
            py::arg("memory_config") = std::nullopt,
            py::arg("are_required_outputs") = std::vector<bool>{true},
            py::arg("input_grad") = std::nullopt,
            py::arg("queue_id") = 0}
    );
}

//OpHandler_float_shape : get_function_type1_float_shape
template <typename unary_backward_operation_t>
void bind_unary_backward_shape(py::module& module, const unary_backward_operation_t& operation, const std::string& parameter_name_a, const std::string& parameter_a_doc, const std::string& description) {
    auto doc = fmt::format(
        R"doc({0}(grad_tensor: ttnn.Tensor, input_tensor: ttnn.Tensor, {2}: shape, *, memory_config: ttnn.MemoryConfig) -> std::vector<Tensor>

        {4}

        Args:
            * :attr:`grad_tensor`
            * :attr:`input_tensor`

        Keyword args:
            * :attr:`{2}` (string): {3}
            * :attr:`memory_config` [ttnn.MemoryConfig]: memory config for the output tensor

        Example:

            >>> grad_tensor = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> input = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> output = {1}(grad_tensor, input, {2})
        )doc",
        operation.base_name(),
        operation.python_fully_qualified_name(),
        parameter_name_a,
        parameter_a_doc,
        description);

    bind_registered_operation(
        module,
        operation,
        doc,
        ttnn::pybind_overload_t{
            [](const unary_backward_operation_t& self,
               const ttnn::Tensor& grad_tensor,
               const ttnn::Tensor& input_tensor,
               const tt::tt_metal::Shape& parameter_a,
               const std::optional<MemoryConfig>& memory_config)  {
                return self(grad_tensor, input_tensor, parameter_a, memory_config);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor"),
            py::arg(parameter_name_a.c_str()),
            py::kw_only(),
            py::arg("memory_config") = std::nullopt});
}

//OpHandler_unary_optional : get_function_unary_optional
template <typename unary_backward_operation_t>
void bind_unary_backward_unary_optional(py::module& module, const unary_backward_operation_t& operation, const std::string& description) {
    auto doc = fmt::format(
        R"doc({0}(grad_tensor: ttnn.Tensor, input_tensor: ttnn.Tensor, *, memory_config: ttnn.MemoryConfig) -> std::vector<std::optional<Tensor>>

        {2}

        Args:
            * :attr:`grad_tensor`
            * :attr:`input_tensor`

        Keyword args:
            * :attr:`memory_config` (Optional[ttnn.MemoryConfig]): memory config for the output tensor
            * :attr:`are_required_outputs` (Optional[std::vector<bool>]): List of bool, Default value is [True]
            * :attr:`input_grad` (Optional[ttnn.Tensor]): preallocated output tensor,
            * :attr:`queue_id` (Optional[uint8]): command queue id

        Example:

            >>> grad_tensor = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> tensor1 = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> output = {1}(grad_tensor, tensor)
        )doc",
        operation.base_name(),
        operation.python_fully_qualified_name(),
        description);

    bind_registered_operation(
        module,
        operation,
        doc,
        ttnn::pybind_overload_t{
            [](const unary_backward_operation_t& self,
               const ttnn::Tensor& grad_tensor,
               const ttnn::Tensor& input_tensor,
               const std::optional<ttnn::MemoryConfig>& memory_config,
               const std::vector<bool>& are_required_outputs,
               const std::optional<ttnn::Tensor>& input_grad,
               const uint8_t& queue_id) -> std::vector<optional<ttnn::Tensor>> {
                return self(queue_id, grad_tensor, input_tensor, memory_config, are_required_outputs, input_grad);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor"),
            py::kw_only(),
            py::arg("memory_config") = std::nullopt,
            py::arg("are_required_outputs") = std::vector<bool>{true},
            py::arg("input_grad") = std::nullopt,
            py::arg("queue_id") = 0}
    );
}

//OpHandler_prod_bw : get_function_prod_bw
template <typename unary_backward_operation_t>
void bind_unary_backward_prod_bw(py::module& module, const unary_backward_operation_t& operation) {
    auto doc = fmt::format(
        R"doc({0}(grad_tensor: ttnn.Tensor, input_tensor: ttnn.Tensor, all_dimensions: bool, dim: int64_t, *, memory_config: ttnn.MemoryConfig) -> std::vector<Tensor>

        Performs backward operations for prod on input along `all_dimensions` or a particular `dim`.

        Args:
            * :attr:`grad_tensor`
            * :attr:`input_tensor`

        Keyword args:
            * :attr:`all_dimensions` :Can be true (perform prod backward along all dimensions ,ignores dim param) or false (Default value = true)
            * :attr:`dim` : Dimension to perform prod backward, if :attr:`all_dimensions` = false (Default dimension = 0)
            * :attr:`memory_config` [ttnn.MemoryConfig]: memory config for the output tensor

        Example:

            >>> grad_tensor = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> input = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
            >>> output = {1}(grad_tensor, input, all_dimensions, dim)
        )doc",
        operation.base_name(),
        operation.python_fully_qualified_name());

    bind_registered_operation(
        module,
        operation,
        doc,
        ttnn::pybind_overload_t{
            [](const unary_backward_operation_t& self,
               const ttnn::Tensor& grad_tensor,
               const ttnn::Tensor& input_tensor,
               bool all_dimensions,
               int64_t dim,
               const std::optional<MemoryConfig>& memory_config)  {
                return self(grad_tensor, input_tensor, all_dimensions, dim, memory_config);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor"),
            py::kw_only(),
            py::arg("all_dimensions") = true,
            py::arg("dim") = 0,
            py::arg("memory_config") = std::nullopt}
    );
}

template <typename unary_backward_operation_t>
void bind_unary_backward(py::module& module, const unary_backward_operation_t& operation, const std::string& description) {
    auto doc = fmt::format(
R"doc({0}(grad_tensor: ttnn.Tensor, input_tensor: ttnn.Tensor *, memory_config: ttnn.MemoryConfig) -> std::vector<Tensor>

{2}

Args:
    * :attr:`grad_tensor`
    * :attr:`input_tensor`

Keyword args:
    * :attr:`memory_config` (Optional[ttnn.MemoryConfig]): memory config for the output tensor

Example:

    >>> grad_tensor = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
    >>> input = ttnn.to_device(ttnn.from_torch(torch.tensor((1, 2), dtype=torch.bfloat16)), device)
    >>> output = {1}(grad_tensor, input)
)doc",
        operation.base_name(),
        operation.python_fully_qualified_name(),
        description);

    bind_registered_operation(
        module,
        operation,
        doc,
        ttnn::pybind_overload_t{
            [operation](const unary_backward_operation_t& self,
               const ttnn::Tensor& grad_tensor,
               const ttnn::Tensor& input_tensor_a,
               const ttnn::Tensor& input_tensor_b,
               const std::optional<ttnn::MemoryConfig>& memory_config) -> std::vector<ttnn::Tensor> {
                auto output_memory_config = memory_config.value_or(input_tensor_a.memory_config());

                using BinaryBackwardOp = ttnn::operations::binary_backward::ExecuteBinaryBackward<binary_backward::BinaryBackwardOpType::MUL_BW>;
                if(operation.base_name()=="assign_bw"){
                    using BinaryBackwardOp = ttnn::operations::binary_backward::ExecuteBinaryBackward<binary_backward::BinaryBackwardOpType::ASSIGN_BW>;
                    return BinaryBackwardOp::execute_on_worker_thread(grad_tensor, input_tensor_a, output_memory_config, input_tensor_b);
                }else if(operation.base_name()=="add_bw"){
                    using BinaryBackwardOp = ttnn::operations::binary_backward::ExecuteBinaryBackward<binary_backward::BinaryBackwardOpType::ADD_BW>;
                    return BinaryBackwardOp::execute_on_worker_thread(grad_tensor, input_tensor_a, output_memory_config, input_tensor_b);
                }else if(operation.base_name()=="eq_bw"){
                    using BinaryBackwardOp = ttnn::operations::binary_backward::ExecuteBinaryBackward<binary_backward::BinaryBackwardOpType::EQ_BW>;
                    return BinaryBackwardOp::execute_on_worker_thread(grad_tensor, input_tensor_a, output_memory_config, input_tensor_b);
                }else if(operation.base_name()=="sub_bw"){
                    using BinaryBackwardOp = ttnn::operations::binary_backward::ExecuteBinaryBackward<binary_backward::BinaryBackwardOpType::SUB_BW>;
                    return BinaryBackwardOp::execute_on_worker_thread(grad_tensor, input_tensor_a, output_memory_config, input_tensor_b);
                }else if(operation.base_name()=="gt_bw"){
                    using BinaryBackwardOp = ttnn::operations::binary_backward::ExecuteBinaryBackward<binary_backward::BinaryBackwardOpType::GT_BW>;
                    return BinaryBackwardOp::execute_on_worker_thread(grad_tensor, input_tensor_a, output_memory_config, input_tensor_b);
                }else if(operation.base_name()=="lt_bw"){
                    using BinaryBackwardOp = ttnn::operations::binary_backward::ExecuteBinaryBackward<binary_backward::BinaryBackwardOpType::LT_BW>;
                    return BinaryBackwardOp::execute_on_worker_thread(grad_tensor, input_tensor_a, output_memory_config, input_tensor_b);
                }else if(operation.base_name()=="le_bw"){
                    using BinaryBackwardOp = ttnn::operations::binary_backward::ExecuteBinaryBackward<binary_backward::BinaryBackwardOpType::LE_BW>;
                    return BinaryBackwardOp::execute_on_worker_thread(grad_tensor, input_tensor_a, output_memory_config, input_tensor_b);
                }else if(operation.base_name()=="ge_bw"){
                    using BinaryBackwardOp = ttnn::operations::binary_backward::ExecuteBinaryBackward<binary_backward::BinaryBackwardOpType::GE_BW>;
                    return BinaryBackwardOp::execute_on_worker_thread(grad_tensor, input_tensor_a, output_memory_config, input_tensor_b);
                }else if(operation.base_name()=="ne_bw"){
                    using BinaryBackwardOp = ttnn::operations::binary_backward::ExecuteBinaryBackward<binary_backward::BinaryBackwardOpType::NE_BW>;
                    return BinaryBackwardOp::execute_on_worker_thread(grad_tensor, input_tensor_a, output_memory_config, input_tensor_b);
                }
                return BinaryBackwardOp::execute_on_worker_thread(grad_tensor, input_tensor_a, output_memory_config, input_tensor_b);

            },
            py::arg("grad_tensor"),
            py::arg("input_tensor_a"),
            py::arg("input_tensor_b"),
            py::kw_only(),
            py::arg("memory_config") = std::nullopt},

        ttnn::pybind_overload_t{
            [operation](const unary_backward_operation_t& self,
               const ComplexTensor& grad_tensor,
               const ComplexTensor& input_tensor_a,
               const ComplexTensor& input_tensor_b,
               float alpha,
               const MemoryConfig& memory_config) -> std::vector<ComplexTensor> {
                using ComplexBinaryBackwardOp = ttnn::operations::complex_binary_backward::ExecuteComplexBinaryBackwardWFloat<complex_binary_backward::ComplexBinaryBackwardOpType::COMPLEX_SUB_BW>;
                if(operation.base_name()=="add_bw"){
                    using ComplexBinaryBackwardOp = ttnn::operations::complex_binary_backward::ExecuteComplexBinaryBackwardWFloat<complex_binary_backward::ComplexBinaryBackwardOpType::COMPLEX_ADD_BW>;
                    return ComplexBinaryBackwardOp::execute_on_main_thread(grad_tensor, input_tensor_a, input_tensor_b, alpha, memory_config);
                }
                return ComplexBinaryBackwardOp::execute_on_main_thread(grad_tensor, input_tensor_a, input_tensor_b, alpha, memory_config);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor_a"),
            py::arg("input_tensor_b"),
            py::arg("alpha"),
            py::kw_only(),
            py::arg("memory_config")},

        ttnn::pybind_overload_t{
            [operation](const unary_backward_operation_t& self,
               const ComplexTensor& grad_tensor,
               const ComplexTensor& input_tensor_a,
               const ComplexTensor& input_tensor_b,
               const MemoryConfig& memory_config) -> std::vector<ComplexTensor> {
                using ComplexBinaryBackwardOp = ttnn::operations::complex_binary_backward::ExecuteComplexBinaryBackwardWoFloat<complex_binary_backward::ComplexBinaryBackwardOpType::COMPLEX_MUL_BW>;
                return ComplexBinaryBackwardOp::execute_on_main_thread(grad_tensor, input_tensor_a, input_tensor_b, memory_config);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor_a"),
            py::arg("input_tensor_b"),
            py::kw_only(),
            py::arg("memory_config")},

        ttnn::pybind_overload_t{
            [operation](const unary_backward_operation_t& self,
               const ttnn::Tensor& grad_tensor,
               const ttnn::Tensor& input_tensor_a,
               const ttnn::Tensor& input_tensor_b,
               const std::optional<ttnn::MemoryConfig>& memory_config,
               const std::vector<bool>& are_required_outputs,
               const std::optional<ttnn::Tensor>& input_a_grad,
               const std::optional<ttnn::Tensor>& input_b_grad,
               const uint8_t& queue_id) -> std::vector<optional<ttnn::Tensor>> {
                using BinaryBackwardOp = ttnn::operations::binary_backward::ExecuteBinaryBackward<binary_backward::BinaryBackwardOpType::MUL_BW>;
                if(operation.base_name()=="add_bw"){
                    using BinaryBackwardOp = ttnn::operations::binary_backward::ExecuteBinaryBackward<binary_backward::BinaryBackwardOpType::ADD_BW>;
                    return BinaryBackwardOp::execute_on_main_thread(queue_id, grad_tensor, input_tensor_a, input_tensor_b, memory_config, are_required_outputs, input_a_grad, input_b_grad);
                }else if(operation.base_name()=="eq_bw"){
                    using BinaryBackwardOp = ttnn::operations::binary_backward::ExecuteBinaryBackward<binary_backward::BinaryBackwardOpType::EQ_BW>;
                    return BinaryBackwardOp::execute_on_main_thread(queue_id, grad_tensor, input_tensor_a, input_tensor_b, memory_config, are_required_outputs, input_a_grad, input_b_grad);
                }
                return BinaryBackwardOp::execute_on_main_thread(queue_id, grad_tensor, input_tensor_a, input_tensor_b, memory_config, are_required_outputs, input_a_grad, input_b_grad);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor_a"),
            py::arg("input_tensor_b"),
            py::kw_only(),
            py::arg("memory_config") = std::nullopt,
            py::arg("are_required_outputs") = std::vector<bool>{true, true},
            py::arg("input_a_grad") = std::nullopt,
            py::arg("input_b_grad") = std::nullopt,
            py::arg("queue_id") = 0},

        ttnn::pybind_overload_t{
            [](const unary_backward_operation_t& self,
               const ttnn::Tensor& grad_tensor,
               const ttnn::Tensor& input_tensor,
               const std::optional<ttnn::MemoryConfig>& memory_config) -> std::vector<ttnn::Tensor> {
                auto output_memory_config = memory_config.value_or(input_tensor.memory_config());
                return self(grad_tensor, input_tensor, output_memory_config);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor"),
            py::kw_only(),
            py::arg("memory_config") = std::nullopt},

        ttnn::pybind_overload_t{
            [operation](const unary_backward_operation_t& self,
               const ComplexTensor& grad_tensor,
               const ComplexTensor& input_tensor_a,
               const MemoryConfig& memory_config) -> std::vector<ComplexTensor> {
                using ComplexUnaryBackwardOp = ttnn::operations::complex_unary_backward::ExecuteComplexUnaryBackward<complex_unary_backward::ComplexUnaryBackwardOpType::COMPLEX_RECIP_BW>;
                return ComplexUnaryBackwardOp::execute_on_main_thread(grad_tensor, input_tensor_a, memory_config);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor_a"),
            py::kw_only(),
            py::arg("memory_config")},

        ttnn::pybind_overload_t{
            [operation](const unary_backward_operation_t& self,
               const Tensor& grad_tensor,
               const ComplexTensor& input_tensor_a,
               const MemoryConfig& memory_config) -> std::vector<ComplexTensor> {
                using ComplexUnaryBackwardOp = ttnn::operations::complex_unary_backward::ExecuteComplexUnaryBackwardTensor<complex_unary_backward::ComplexUnaryBackwardOpType::COMPLEX_ABS_BW>;
                return ComplexUnaryBackwardOp::execute_on_main_thread(grad_tensor, input_tensor_a, memory_config);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor_a"),
            py::kw_only(),
            py::arg("memory_config")},

        ttnn::pybind_overload_t{
            [](const unary_backward_operation_t& self,
               const ttnn::Tensor& grad_tensor,
               const ttnn::Tensor& input_tensor,
               const float alpha,
               const std::optional<ttnn::MemoryConfig>& memory_config) -> std::vector<ttnn::Tensor> {
                return self(grad_tensor, input_tensor, alpha, memory_config);
            },
            py::arg("grad_tensor"),
            py::arg("input_tensor"),
            py::arg("alpha"),
            py::kw_only(),
            py::arg("memory_config") = std::nullopt});

}


}  // namespace detail


void py_module(py::module& module) {
    detail::bind_unary_backward(
        module,
        ttnn::mul_bw,
        R"doc(Performs backward operations for multiply on :attr:`input_tensor`, :attr:`alpha` or attr:`input_tensor_a`, attr:`input_tensor_b`, with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward_optional_float_params_with_default(
        module,
        ttnn::clamp_bw,
        "min", "Minimum value", std::nullopt,
        "max", "Maximum value", std::nullopt,
        R"doc(Performs backward operations for clamp value on :attr:`input_tensor`, :attr:`min`, :attr:`max` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward_two_float_with_default(
        module,
        ttnn::hardtanh_bw,
        "min", "Minimum value", -1.0,
        "max", "Maximum value", 1.0,
        R"doc(Performs backward operations for hardtanh activation function on :attr:`input_tensor`, :attr:`min`, :attr:`max` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward_two_float(
        module,
        ttnn::threshold_bw,
        R"doc(Performs backward operations for threshold on :attr:`input_tensor`, :attr:`threshold`, :attr:`value` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward_two_float_with_default(
        module,
        ttnn::softplus_bw,
        "beta", "Beta value for the Softplus formula ", 1.0,
        "threshold", "Threshold value", 20.0,
        R"doc(Performs backward operations for softplus on :attr:`input_tensor`, :attr:`beta`, :attr:`threshold` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::assign_bw,
        R"doc(Performs backward operations for assign on :attr:`input_tensor` or attr:`input_tensor_a`, attr:`input_tensor_b`, with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward_float_string_default(
        module,
        ttnn::div_bw,
        "scalar", "Denominator value",
        "round_mode", "Mode of Rounding", "None",
        R"doc(Performs backward operations for division on :attr:`input_tensor` and :attr:`scalar` or :attr:`input_tensor_a` and :attr:`input_tensor_b`, with given :attr:`grad_tensor` using given :attr:`round_mode`.
        :attr:`round_mode` can be 'None', 'trunc', or 'floor'.)doc");

    detail::bind_unary_backward_float_string_default(
        module,
        ttnn::rdiv_bw,
        "scalar", "divisor",
        "round_mode", "Mode of Rounding", "None",
        R"doc(Performs backward operations for Unary rdiv on :attr:`input_tensor`, :attr:`scalar` with given :attr:`grad_tensor` using given :attr:`round_mode`.
        :attr:`round_mode` can be 'None', 'trunc', or 'floor'.)doc");

    detail::bind_unary_backward_float_string_default(
        module,
        ttnn::bias_gelu_bw,
        "bias", "Bias value",
        "approximate", "Approximation type", "none",
        R"doc(Performs backward operations for bias_gelu on :attr:`input_tensor_a` and :attr:`input_tensor_b` or :attr:`input_tensor` and :attr:`bias`, with given :attr:`grad_tensor` using given :attr:`approximate` mode.
        :attr:`approximate` mode can be 'none', 'tanh'.)doc");

    detail::bind_unary_backward_shape(
        module,
        ttnn::repeat_bw,
        "shape", "Shape",
        R"doc(Performs backward operations for repeat on :attr:`input_tensor_a` or :attr:`input_tensor`, with given :attr:`grad_tensor` using given :attr:`shape`.)doc");

    detail::bind_unary_backward_string_default(
        module,
        ttnn::gelu_bw,
        "approximate", "Approximation type", "none",
        R"doc(Performs backward operations for gelu on :attr:`input_tensor_a` or :attr:`input_tensor`, with given :attr:`grad_tensor` using given :attr:`approximate` mode.
        :attr:`approximate` mode can be 'none', 'tanh'.)doc");

    detail::bind_unary_backward_unary_optional_float(
        module,
        ttnn::pow_bw,
        "exponent", "Exponent value",
        R"doc(Performs backward operations for power on :attr:`input_tensor` , :attr:`exponent` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward_unary_optional(
        module,
        ttnn::exp_bw,
        R"doc(Performs backward operations for exponential function on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward_unary_optional(
        module,
        ttnn::tanh_bw,
        R"doc(Performs backward operations for Hyperbolic Tangent (Tanh) function on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward_unary_optional(
        module,
        ttnn::sqrt_bw,
        R"doc(Performs backward operations for square-root on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::multigammaln_bw,
        R"doc(Performs backward operations for multigammaln on :attr:`input_tensor` with given :attr:`grad_tensor` and value of P is taken as 4.
        mvlgamma is refered as multigammaln.
        Input value must be greater than 2.5f)doc");

    detail::bind_unary_backward(
        module,
        ttnn::add_bw,
        R"doc(Performs backward operations for addition on :attr:`input_tensor`, :attr:`alpha` or attr:`input_tensor_a`, attr:`input_tensor_b`, with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward_prod_bw(module,ttnn::prod_bw);

    detail::bind_unary_backward(
        module,
        ttnn::eq_bw,
        R"doc(Performs backward operations for equal to comparison on :attr:`input_tensor`, :attr:`alpha` or attr:`input_tensor_a`, attr:`input_tensor_b`, with given :attr:`grad_tensor`.
        Returns an tensor of zeros like input tensors.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::gt_bw,
        R"doc(Performs backward operations for greater than comparison on :attr:`input_tensor`, :attr:`alpha` or attr:`input_tensor_a`, attr:`input_tensor_b`. with given :attr:`grad_tensor`.
        Returns an tensor of zeros like input tensors.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::lt_bw,
        R"doc(Performs backward operations for less than comparison on :attr:`input_tensor`, :attr:`alpha` or attr:`input_tensor_a`, attr:`input_tensor_b`, with given :attr:`grad_tensor`.
        Returns an tensor of zeros like input tensors.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::le_bw,
        R"doc(Performs backward operations for less than or equal comparison on :attr:`input_tensor`, :attr:`alpha` or attr:`input_tensor_a`, attr:`input_tensor_b`, with given :attr:`grad_tensor`.
        Returns an tensor of zeros like input tensors.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::ge_bw,
        R"doc(Performs backward operations for greater than or equal comparison on :attr:`input_tensor`, :attr:`alpha` or attr:`input_tensor_a`, attr:`input_tensor_b`, with given :attr:`grad_tensor`.
        Returns an tensor of zeros like input tensors.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::ne_bw,
        R"doc(Performs backward operations for not equal comparison on :attr:`input_tensor`, :attr:`alpha` or attr:`input_tensor_a`, attr:`input_tensor_b`, with given :attr:`grad_tensor`.
        Returns an tensor of zeros like input tensors.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::lgamma_bw,
        R"doc(Performs backward operations for lgamma on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::fill_bw,
        R"doc(Performs backward operations for fill on :attr:`input_tensor` with given :attr:`grad_tensor`.
        Returns an tensor like :attr:`grad_tensor` with sum of tensor values.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::hardsigmoid_bw,
        R"doc(Performs backward operations for hardsigmoid on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::cos_bw,
        R"doc(Performs backward operations for cos on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::acosh_bw,
        R"doc(Performs backward operations for inverse cosine (acos) on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::acos_bw,
        R"doc(Performs backward operations for inverse hyperbolic cosine (acosh) on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::atan_bw,
        R"doc(Performs backward operations for atan on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::rad2deg_bw,
        R"doc(Performs backward operations for radian to degree conversion (rad2deg) on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::sub_bw,
        R"doc(Performs backward operations for subtraction on :attr:`input_tensor`, :attr:`alpha` or attr:`input_tensor_a`, attr:`input_tensor_b`, with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::frac_bw,
        R"doc(Performs backward operations for frac on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::trunc_bw,
        R"doc(Performs backward operations for truncation on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::log_sigmoid_bw,
        R"doc(Performs backward operations for log sigmoid on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::fill_zero_bw,
        R"doc(Performs backward operations of fill zero on :attr:`input_tensor` with given :attr:`grad_tensor`. Returns an tensor of zeros like :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::i0_bw,
        R"doc(Performs backward operations for i0 on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::tan_bw,
        R"doc(Performs backward operations for tan on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::sigmoid_bw,
        R"doc(Performs backward operations for sigmoid on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::rsqrt_bw,
        R"doc(Performs backward operations for rsqrt on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::neg_bw,
        R"doc(Performs backward operations for neg on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::relu_bw,
        R"doc(Performs backward operations for relu on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::logit_bw,
        R"doc(Performs backward operations for logit on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::floor_bw,
        R"doc(Performs backward operations for floor on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::hardshrink_bw,
        R"doc(Performs backward operations for hardshrink on :attr:`input_tensor`, :attr:`lambd` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::softshrink_bw,
        R"doc(Performs backward operations for softshrink on :attr:`input_tensor`, :attr:`lambd` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::leaky_relu_bw,
        R"doc(Performs backward operations for leaky relu on :attr:`input_tensor`, :attr:`negative_slope` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::elu_bw,
        R"doc(Performs backward operations for elu on :attr:`input_tensor`, :attr:`alpha` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::celu_bw,
        R"doc(Performs backward operations for celu on :attr:`input_tensor`, :attr:`alpha` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::rpow_bw,
        R"doc(Performs backward operations for rpow on :attr:`input_tensor`, :attr:`exponent` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::round_bw,
        R"doc(Performs backward operations for round on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::log_bw,
        R"doc(Performs backward operations for logarithm on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::relu6_bw,
        R"doc(Performs backward operations for relu6 on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::abs_bw,
        R"doc(Performs backward operations for abs on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::silu_bw,
        R"doc(Performs backward operations for silu on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::selu_bw,
        R"doc(Performs backward operations for selu on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::square_bw,
        R"doc(Performs backward operations for square on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::hardswish_bw,
        R"doc(Performs backward operations for  hardswish on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::tanhshrink_bw,
        R"doc(Performs backward operations for  tanhshrink on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::atanh_bw,
        R"doc(Performs backward operations for  atanh on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::asin_bw,
        R"doc(Performs backward operations for  asin on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::asinh_bw,
        R"doc(Performs backward operations for  asinh on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::sin_bw,
        R"doc(Performs backward operations for sin on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::sinh_bw,
        R"doc(Performs backward operations for sinh on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::log10_bw,
        R"doc(Performs backward operations for log10 on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::log1p_bw,
        R"doc(Performs backward operations for log1p on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::erfc_bw,
        R"doc(Performs backward operations for erfc on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::ceil_bw,
        R"doc(Performs backward operations for ceil on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::softsign_bw,
        R"doc(Performs backward operations for softsign on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");


    detail::bind_unary_backward(
        module,
        ttnn::cosh_bw,
        R"doc(Performs backward operations for cosh on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::logiteps_bw,
        R"doc(Performs backward operations for logiteps on :attr:`input_tensor`, :attr:`eps` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::log2_bw,
        R"doc(Performs backward operations for log2 on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::sign_bw,
        R"doc(Performs backward operations for sign on :attr:`input_tensor` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::fmod_bw,
        R"doc(Performs backward operations for fmod on :attr:`input_tensor`, :attr:`eps` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::remainder_bw,
        R"doc(Performs backward operations for remainder on :attr:`input_tensor`, :attr:`scalar` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::div_no_nan_bw,
        R"doc(Performs backward operations for div_no_nan on :attr:`input_tensor`, :attr:`scalar` with given :attr:`grad_tensor`.)doc");

    detail::bind_unary_backward(
        module,
        ttnn::exp2_bw,
        R"doc(Performs backward operations for exp2 on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::expm1_bw,
        R"doc(Performs backward operations for exp2 on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::reciprocal_bw,
        R"doc(Performs backward operations for exp2 on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::digamma_bw,
        R"doc(Performs backward operations for digamma on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::erfinv_bw,
        R"doc(Performs backward operations for erfinv on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::erf_bw,
        R"doc(Performs backward operations for erf on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::deg2rad_bw,
        R"doc(Performs backward operations for deg2rad on :attr:`input_tensor` with given :attr:`grad_tensor`)doc");

    detail::bind_unary_backward(
        module,
        ttnn::polygamma_bw,
        R"doc(Performs backward operations for polygamma on :attr:`input_tensor` or attr:`input_tensor_a`, attr:`scalar` with given :attr:`grad_tensor`.)doc");

}

}  // namespace binary_backward
}  // namespace operations
}  // namespace ttnn
