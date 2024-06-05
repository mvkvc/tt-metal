# SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.

# SPDX-License-Identifier: Apache-2.0

import pytest

import torch

import ttnn
import tt_lib
from models.utility_functions import is_grayskull

from tests.ttnn.utils_for_testing import assert_with_pcc
from models.utility_functions import torch_random


def run_math_unary_test(device, h, w, ttnn_function, torch_function, pcc=0.9999):
    torch.manual_seed(0)

    torch_input_tensor = torch.rand((h, w), dtype=torch.bfloat16)
    if "digamma" in str(torch_function):
        torch_input_tensor += 100.0
    torch_output_tensor = torch_function(torch_input_tensor)

    input_tensor = ttnn.from_torch(torch_input_tensor, layout=ttnn.TILE_LAYOUT, device=device)
    output_tensor = ttnn_function(input_tensor)
    output_tensor = ttnn.to_torch(output_tensor)

    assert_with_pcc(torch_output_tensor, output_tensor, pcc)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_i0(device, h, w):
    run_math_unary_test(device, h, w, ttnn.i0, torch.i0, pcc=0.998)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_isfinite(device, h, w):
    run_math_unary_test(device, h, w, ttnn.isfinite, torch.isfinite, pcc=0.993)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_isinf(device, h, w):
    run_math_unary_test(device, h, w, ttnn.isinf, torch.isinf, pcc=0.9997)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_isnan(device, h, w):
    run_math_unary_test(device, h, w, ttnn.isnan, torch.isnan)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_isneginf(device, h, w):
    run_math_unary_test(device, h, w, ttnn.isneginf, torch.isneginf)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_isposinf(device, h, w):
    run_math_unary_test(device, h, w, ttnn.isposinf, torch.isposinf)


@pytest.mark.parametrize("h", [5])
@pytest.mark.parametrize("w", [5])
def test_lgamma(device, h, w):
    run_math_unary_test(device, h, w, ttnn.lgamma, torch.lgamma, pcc=0.999)


@pytest.mark.parametrize("h", [32])
@pytest.mark.parametrize("w", [32])
@pytest.mark.parametrize("output_dtype", [ttnn.DataType.BFLOAT16, ttnn.DataType.UINT16, ttnn.DataType.UINT32])
def test_eq(device, h, w, output_dtype):
    if is_grayskull() and output_dtype in (ttnn.DataType.UINT32, ttnn.DataType.UINT16):
        pytest.skip("GS does not support fp32/uint32/uint16 data types")

    torch.manual_seed(0)

    same = 50
    torch_input_tensor_a = torch.rand((h, w), dtype=torch.bfloat16)
    torch_input_tensor_a[0, 0] = same
    torch_input_tensor_a[0, 1] = same
    torch_input_tensor_a[0, 2] = same

    torch_input_tensor_b = torch.rand((h, w), dtype=torch.bfloat16)
    torch_input_tensor_b[0, 0] = same
    torch_input_tensor_b[0, 1] = same
    torch_input_tensor_b[0, 2] = same

    torch_output_tensor = torch.eq(torch_input_tensor_a, torch_input_tensor_b)

    input_tensor_a = ttnn.from_torch(
        torch_input_tensor_a, layout=ttnn.TILE_LAYOUT, device=device, memory_config=ttnn.L1_MEMORY_CONFIG
    )
    input_tensor_b = ttnn.from_torch(
        torch_input_tensor_b, layout=ttnn.TILE_LAYOUT, device=device, memory_config=ttnn.L1_MEMORY_CONFIG
    )

    pages_before = ttnn._ttnn.reports.get_buffer_pages()
    output_tensor = ttnn.eq(input_tensor_a, input_tensor_b, dtype=output_dtype)
    assert output_tensor.get_dtype() == output_dtype
    assert len(pages_before) == len(ttnn._ttnn.reports.get_buffer_pages()) - 1
    output_tensor = ttnn.to_torch(output_tensor)
    assert_with_pcc(torch_output_tensor, output_tensor, 0.999)

    # EQ with a preallocated output tensor
    output_tensor_preallocated_bfloat16 = ttnn.ones(
        [h, w], ttnn.DataType.BFLOAT16, ttnn.TILE_LAYOUT, device, ttnn.L1_MEMORY_CONFIG
    )
    output_tensor_preallocated = output_tensor_preallocated_bfloat16
    # There is no good way to create uint16 tensor in ttnn/torch, so we create bfloat16 and typecast to target
    if output_dtype != ttnn.DataType.BFLOAT16:
        output_tensor_preallocated = tt_lib.tensor.typecast(
            output_tensor_preallocated_bfloat16, output_dtype, ttnn.L1_MEMORY_CONFIG
        )

    pages_before = ttnn._ttnn.reports.get_buffer_pages()
    ttnn.eq(input_tensor_a, input_tensor_b, dtype=output_dtype, output_tensor=output_tensor_preallocated)
    assert len(pages_before) == len(ttnn._ttnn.reports.get_buffer_pages())
    torch_output_tensor_preallocated = ttnn.to_torch(output_tensor_preallocated)
    assert_with_pcc(torch_output_tensor, torch_output_tensor_preallocated, 0.999)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_log10(device, h, w):
    run_math_unary_test(device, h, w, ttnn.log10, torch.log10)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_log1p(device, h, w):
    run_math_unary_test(device, h, w, ttnn.log1p, torch.log1p, pcc=0.999)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_log2(device, h, w):
    run_math_unary_test(device, h, w, ttnn.log2, torch.log2, pcc=0.999)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_neg(device, h, w):
    run_math_unary_test(device, h, w, ttnn.neg, torch.neg)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_abs(device, h, w):
    run_math_unary_test(device, h, w, ttnn.abs, torch.abs)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_rad2deg(device, h, w):
    run_math_unary_test(device, h, w, ttnn.rad2deg, torch.rad2deg)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_cbrt(device, h, w):
    run_math_unary_test(device, h, w, ttnn.cbrt, torch_cbrt, pcc=0.999)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_tril(device, h, w):
    run_math_unary_test(device, h, w, ttnn.tril, torch.tril)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_deg2rad(device, h, w):
    run_math_unary_test(device, h, w, ttnn.deg2rad, torch.deg2rad)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_sqrt(device, h, w):
    run_math_unary_test(device, h, w, ttnn.sqrt, torch.sqrt)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_digamma(device, h, w):
    run_math_unary_test(device, h, w, ttnn.digamma, torch.digamma)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_erf(device, h, w):
    run_math_unary_test(device, h, w, ttnn.erf, torch.erf)


@pytest.mark.parametrize("h", [2])
@pytest.mark.parametrize("w", [3])
def test_erfc(device, h, w):
    run_math_unary_test(device, h, w, ttnn.erfc, torch.erfc)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_erfinv(device, h, w):
    run_math_unary_test(device, h, w, ttnn.erfinv, torch.erfinv, pcc=0.999)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_square(device, h, w):
    run_math_unary_test(device, h, w, ttnn.square, torch.square)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_exp2(device, h, w):
    run_math_unary_test(device, h, w, ttnn.exp2, torch.exp2, pcc=0.98)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_expm1(device, h, w):
    run_math_unary_test(device, h, w, ttnn.expm1, torch.expm1, pcc=0.99)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_triu(device, h, w):
    run_math_unary_test(device, h, w, ttnn.triu, torch.triu)


def run_math_unary_test_recip(device, h, w, ttnn_function, torch_function, pcc=0.9999):
    torch.manual_seed(0)

    low = -100
    high = 100

    torch_input_tensor = torch.empty((h, w), dtype=torch.bfloat16).uniform_(low, high) + 0.0001
    torch_output_tensor = torch_function(torch_input_tensor)

    input_tensor = ttnn.from_torch(torch_input_tensor, layout=ttnn.TILE_LAYOUT, device=device)
    output_tensor = ttnn_function(input_tensor)
    output_tensor = ttnn.to_torch(output_tensor)
    assert_with_pcc(torch_output_tensor, output_tensor, pcc)


@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_recip(device, h, w):
    run_math_unary_test_recip(device, h, w, ttnn.reciprocal, torch.reciprocal, pcc=0.999)


def run_math_unary_test_range(device, h, w, ttnn_function, torch_function, pcc=0.9999):
    torch.manual_seed(0)
    low = 1.6
    high = 100

    torch_input_tensor = torch_random((h, w), low, high, dtype=torch.bfloat16)
    torch_output_tensor = torch_function(torch_input_tensor)

    input_tensor = ttnn.from_torch(torch_input_tensor, layout=ttnn.TILE_LAYOUT, device=device)
    output_tensor = ttnn_function(input_tensor)
    output_tensor = ttnn.to_torch(output_tensor)

    assert_with_pcc(torch_output_tensor, output_tensor, pcc)


def torch_cbrt(x, *args, **kwargs):
    return torch.sgn(x) * torch.pow(torch.abs(x), 1.0 / 3)


def torch_multigammaln(x, *args, **kwargs):
    result = torch.lgamma(x)
    result += torch.lgamma(x - 0.5)
    result += torch.lgamma(x - 1.0)
    result += torch.lgamma(x - 1.5)
    result += 3.434189657547
    return result


@pytest.mark.parametrize("h", [5])
@pytest.mark.parametrize("w", [5])
def test_multigammaln(device, h, w):
    run_math_unary_test_range(device, h, w, ttnn.multigammaln, torch_multigammaln, pcc=0.999)


def run_math_test_polygamma(device, h, w, scalar, ttnn_function, torch_function, pcc=0.9999):
    torch.manual_seed(0)

    low = 1
    high = 10

    torch_input_tensor = torch_random((h, w), low, high, dtype=torch.bfloat16)
    torch_output_tensor = torch_function(scalar, torch_input_tensor)

    input_tensor = ttnn.from_torch(torch_input_tensor, layout=ttnn.TILE_LAYOUT, device=device)
    output_tensor = ttnn_function(input_tensor, scalar)
    output_tensor = ttnn.to_torch(output_tensor)

    assert_with_pcc(torch_output_tensor, output_tensor, pcc)


@pytest.mark.parametrize("scalar", [1, 2, 5, 10])
@pytest.mark.parametrize("h", [64])
@pytest.mark.parametrize("w", [128])
def test_polygamma(device, h, w, scalar):
    run_math_test_polygamma(device, h, w, scalar, ttnn.polygamma, torch.polygamma, pcc=0.999)
