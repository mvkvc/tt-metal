
// SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "device/complex_binary_backward_op.hpp"
#include "ttnn/device_operation.hpp"
#include "ttnn/operations/data_movement.hpp"

namespace ttnn {

namespace operations::complex_binary_backward {

template <ComplexBinaryBackwardOpType complex_binary_backward_op_type>
struct ExecuteComplexBinaryBackwardWFloat {

    //Type 1: 2 inputs, 1 grad tensor 1 float

    static std::vector<ComplexTensor> execute_on_main_thread(
        const ComplexTensor &grad_tensor_arg,
        const ComplexTensor &input_tensor_a_arg,
        const ComplexTensor &input_tensor_b_arg,
        float alpha,
        const MemoryConfig &memory_config) {

        auto op_type = get_function_w_float<complex_binary_backward_op_type>();
        return op_type(grad_tensor_arg, input_tensor_a_arg, input_tensor_b_arg, alpha, memory_config);
        }

};

template <ComplexBinaryBackwardOpType complex_binary_backward_op_type>
struct ExecuteComplexBinaryBackwardWoFloat {

    //Type 1: 2 inputs, 1 grad tensor

    static std::vector<ComplexTensor> execute_on_main_thread(
        const ComplexTensor &grad_tensor_arg,
        const ComplexTensor &input_tensor_a_arg,
        const ComplexTensor &input_tensor_b_arg,
        const MemoryConfig& memory_config) {

        auto op_type = get_function_wo_float<complex_binary_backward_op_type>();
        return op_type(grad_tensor_arg, input_tensor_a_arg, input_tensor_b_arg, memory_config);
        }

};

}

}
