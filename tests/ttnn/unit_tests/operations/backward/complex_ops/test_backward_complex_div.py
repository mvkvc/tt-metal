# SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.

# SPDX-License-Identifier: Apache-2.0

import math
from pathlib import Path
import sys

import torch

import tt_lib as ttl
from models.utility_functions import print_diff_argmax
import pytest
import ttnn
from loguru import logger

from tests.tt_eager.python_api_testing.sweep_tests.comparison_funcs import comp_pcc, comp_equal, comp_allclose

from models.utility_functions import (
    skip_for_wormhole_b0,
)
from models.utility_functions import is_wormhole_b0
from tests.ttnn.unit_tests.operations.backward.complex_ops.backward_complex_utility_funcs import (
    Complex,
    convert_to_torch_tensor,
    random_complex_tensor,
)


@pytest.mark.parametrize(
    "memcfg",
    (
        ttl.tensor.MemoryConfig(ttl.tensor.TensorMemoryLayout.INTERLEAVED, ttl.tensor.BufferType.DRAM),
        ttl.tensor.MemoryConfig(ttl.tensor.TensorMemoryLayout.INTERLEAVED, ttl.tensor.BufferType.L1),
    ),
    ids=["out_DRAM", "out_L1"],
)
@pytest.mark.parametrize("dtype", ((ttl.tensor.DataType.BFLOAT16,)))
@pytest.mark.parametrize("bs", ((1, 1), (1, 2), (2, 2)))
@pytest.mark.parametrize("hw", ((32, 64), (320, 384)))
def test_level2_complex_div_bw(bs, hw, memcfg, dtype, device, function_level_defaults):
    input_shape = torch.Size([bs[0], bs[1], hw[0], hw[1]])

    in_data = random_complex_tensor(input_shape, (-90, 90), (-70, 70))
    in_data.requires_grad = True

    other_data = random_complex_tensor(input_shape, (-20, 90), (-30, 100))
    other_data.requires_grad = True

    input_tensor = ttl.tensor.complex_tensor(
        ttl.tensor.Tensor(in_data.real, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg),
        ttl.tensor.Tensor(in_data.imag, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg),
    )
    other_tensor = ttl.tensor.complex_tensor(
        ttl.tensor.Tensor(other_data.real, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg),
        ttl.tensor.Tensor(other_data.imag, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg),
    )

    grad_data = random_complex_tensor(input_shape, (-50, 50), (-60, 60))
    grad_tensor = ttl.tensor.complex_tensor(
        ttl.tensor.Tensor(grad_data.real, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg),
        ttl.tensor.Tensor(grad_data.imag, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg),
    )
    tt_dev = ttnn.div_bw(grad_tensor, input_tensor, other_tensor, memory_config=memcfg)
    tt_dev = convert_to_torch_tensor(tt_dev)

    golden_function = ttnn.get_golden_function(ttnn.div_bw)
    golden_tensor = golden_function(grad_data, in_data, other_data)

    for i in range(len(tt_dev)):
        if is_wormhole_b0():
            passing, output = comp_pcc(golden_tensor[i], tt_dev[i])
        else:
            passing, output = comp_pcc(golden_tensor[i], tt_dev[i])
        logger.info(output)
        assert passing


@pytest.mark.parametrize(
    "memcfg",
    (
        ttl.tensor.MemoryConfig(ttl.tensor.TensorMemoryLayout.INTERLEAVED, ttl.tensor.BufferType.DRAM),
        ttl.tensor.MemoryConfig(ttl.tensor.TensorMemoryLayout.INTERLEAVED, ttl.tensor.BufferType.L1),
    ),
    ids=["out_DRAM", "out_L1"],
)
@pytest.mark.parametrize("dtype", ((ttl.tensor.DataType.BFLOAT16,)))
@pytest.mark.parametrize("bs", ((1, 1), (1, 2)))
@pytest.mark.parametrize("hw", ((32, 64), (320, 384)))
@skip_for_wormhole_b0()
def test_level2_complex_div_bw_other_zero(bs, hw, memcfg, dtype, device, function_level_defaults):
    input_shape = torch.Size([bs[0], bs[1], hw[0], hw[1]])

    in_data = random_complex_tensor(input_shape, (-90, 90), (-70, 70))
    in_data.requires_grad = True

    other_data = random_complex_tensor(input_shape, (0, 0), (0, 0))
    other_data.requires_grad = True

    input_tensor = ttl.tensor.complex_tensor(
        ttl.tensor.Tensor(in_data.real, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg),
        ttl.tensor.Tensor(in_data.imag, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg),
    )
    other_tensor = ttl.tensor.complex_tensor(
        ttl.tensor.Tensor(other_data.real, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg),
        ttl.tensor.Tensor(other_data.imag, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg),
    )

    grad_data = random_complex_tensor(input_shape, (-50, 50), (-60, 60))
    grad_tensor = ttl.tensor.complex_tensor(
        ttl.tensor.Tensor(grad_data.real, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg),
        ttl.tensor.Tensor(grad_data.imag, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg),
    )
    tt_dev = ttnn.div_bw(grad_tensor, input_tensor, other_tensor, memory_config=memcfg)
    tt_dev = convert_to_torch_tensor(tt_dev)

    golden_function = ttnn.get_golden_function(ttnn.div_bw)
    golden_tensor = golden_function(grad_data, in_data, other_data)

    for i in range(len(tt_dev)):
        if is_wormhole_b0():
            passing, output = comp_pcc(golden_tensor[i], tt_dev[i])
        else:
            passing, output = comp_pcc(golden_tensor[i], tt_dev[i])
        logger.info(output)
        assert passing
