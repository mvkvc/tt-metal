# SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.

# SPDX-License-Identifier: Apache-2.0

import math
from pathlib import Path
import sys

import torch

import tt_lib as ttl
import ttnn
from models.utility_functions import print_diff_argmax
import pytest
from loguru import logger

from tests.tt_eager.python_api_testing.sweep_tests.comparison_funcs import comp_pcc, comp_equal, comp_allclose
from models.utility_functions import is_wormhole_b0
from functools import partial


class Complex:
    def __init__(self, input_shape: torch.Size = None, re=None, im=None):
        if input_shape:
            val = 1.0 + torch.arange(0, input_shape.numel()).reshape(input_shape).bfloat16()
            self._cplx = val[:, :, :, : input_shape[-1] // 2] + val[:, :, :, input_shape[-1] // 2 :] * 1j
        else:
            self._cplx = re + im * 1j

    def reset(self, val: torch.Tensor):
        self._cplx = val

    def is_imag(self):
        return self.real == 0.0

    def is_real(self):
        return self.imag == 0.0

    @property
    def angle(self):
        return torch.angle(self._cplx)

    @property
    def real(self):
        return self._cplx.real

    @property
    def imag(self):
        return self._cplx.imag

    @property
    def metal(self):
        return torch.cat([self.real, self.imag], -1)

    ## operations
    def abs(self):
        return (self.real**2 + self.imag**2).sqrt()

    def conj(self) -> "Complex":
        self._cplx = self._cplx.conj()
        return self

    def recip(self):
        self._cplx = 1.0 / self._cplx
        return self

    def add(self, that: "Complex"):
        self._cplx += that._cplx
        return self

    def sub(self, that: "Complex"):
        self._cplx -= that._cplx
        return self

    def __mul__(self, scale):
        self._cplx *= scale
        return self

    def mul(self, that: "Complex"):
        self._cplx *= that._cplx
        return self

    def div(self, that: "Complex"):
        self._cplx /= that._cplx
        return self


@pytest.mark.parametrize(
    "memcfg",
    (
        ttl.tensor.MemoryConfig(ttl.tensor.TensorMemoryLayout.INTERLEAVED, ttl.tensor.BufferType.DRAM),
        ttl.tensor.MemoryConfig(ttl.tensor.TensorMemoryLayout.INTERLEAVED, ttl.tensor.BufferType.L1),
    ),
    ids=["out_DRAM", "out_L1"],
)
@pytest.mark.parametrize("dtype", ((ttl.tensor.DataType.BFLOAT16,)))
def test_level1_is_real(memcfg, dtype, device, function_level_defaults):
    input_shape = torch.Size([1, 1, 32, 64])
    # check real
    x = Complex(input_shape)
    x = x.add(x.conj())
    xtt = ttl.tensor.Tensor(x.metal, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg)
    tt_dev = ttl.tensor.is_real(xtt, memcfg)
    tt_dev = tt_dev.cpu().to(ttl.tensor.Layout.ROW_MAJOR).to_torch()
    tt_cpu = x.is_real()
    passing, output = comp_pcc(tt_cpu, tt_dev)
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
def test_level1_is_imag(memcfg, dtype, device, function_level_defaults):
    input_shape = torch.Size([1, 1, 32, 64])
    # check imag
    x = Complex(input_shape)
    x = x.sub(x.conj())
    xtt = ttl.tensor.Tensor(x.metal, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg)
    tt_dev = ttl.tensor.is_imag(xtt, memcfg)
    tt_dev = tt_dev.cpu().to(ttl.tensor.Layout.ROW_MAJOR).to_torch()
    tt_cpu = x.is_imag()
    passing, output = comp_pcc(tt_cpu, tt_dev)
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
def test_level1_angle(memcfg, dtype, device, function_level_defaults):
    input_shape = torch.Size([1, 1, 32, 64])
    # check angle
    x = Complex(input_shape)
    xtt = ttl.tensor.Tensor(x.metal, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg)
    tt_dev = ttl.tensor.angle(xtt, memcfg)
    tt_dev = tt_dev.cpu().to(ttl.tensor.Layout.ROW_MAJOR).to_torch()
    tt_cpu = x.angle
    passing, output = comp_pcc(tt_cpu, tt_dev)
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
def test_level1_real(memcfg, dtype, device, function_level_defaults):
    input_shape = torch.Size([1, 1, 32, 64])
    # check real
    x = Complex(input_shape)
    xtt = ttl.tensor.Tensor(x.metal, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg)
    tt_dev = ttl.tensor.real(xtt, memcfg)
    tt_dev = tt_dev.cpu().to(ttl.tensor.Layout.ROW_MAJOR).to_torch()
    tt_cpu = x.real
    passing, output = comp_equal(tt_cpu, tt_dev)
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
@pytest.mark.parametrize("layout", ((ttl.tensor.Layout.TILE,)))
@pytest.mark.parametrize("bs", ((1, 1), (1, 2)))
def test_level1_imag(bs, memcfg, dtype, device, function_level_defaults, layout):
    input_shape = torch.Size([bs[0], bs[1], 32, 64])
    # check imag
    x = Complex(input_shape)
    tt_cpu = x.imag
    xtt = ttl.tensor.Tensor(x.metal, dtype).to(layout).to(device, memcfg)
    tt_dev = ttl.tensor.imag(xtt)
    tt_dev = tt_dev.cpu().to(ttl.tensor.Layout.ROW_MAJOR).to_torch()
    passing, output = comp_equal(tt_cpu, tt_dev)
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
@pytest.mark.parametrize("layout", ((ttl.tensor.Layout.TILE,)))
@pytest.mark.parametrize("bs", ((1, 1), (1, 2)))
def test_level1_abs(bs, memcfg, dtype, device, function_level_defaults, layout):
    input_shape = torch.Size([bs[0], bs[1], 32, 64])
    # check abs
    x = Complex(input_shape)
    xtt = ttl.tensor.Tensor(x.metal, dtype).to(layout).to(device, memcfg)
    tt_dev = ttl.tensor.complex_abs(xtt, memcfg)
    tt_dev = tt_dev.cpu().to(ttl.tensor.Layout.ROW_MAJOR).to_torch()
    tt_cpu = x.abs()
    passing, output = comp_pcc(tt_cpu, tt_dev)
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
def test_level1_conj(bs, memcfg, dtype, device, function_level_defaults):
    input_shape = torch.Size([bs[0], bs[1], 32, 64])
    # check conj
    x = Complex(input_shape)
    xtt = ttl.tensor.Tensor(x.metal, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg)
    tt_dev = ttl.tensor.conj(xtt, memcfg)
    tt_dev = tt_dev.cpu().to(ttl.tensor.Layout.ROW_MAJOR).to_torch()
    tt_cpu = x.conj().metal
    if is_wormhole_b0():
        passing, output = comp_pcc(tt_cpu, tt_dev, pcc=0.8)
    else:
        passing, output = comp_pcc(tt_cpu, tt_dev)
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
def test_level1_add(bs, memcfg, dtype, device, function_level_defaults):
    input_shape = torch.Size([bs[0], bs[1], 32, 64])
    # check add
    x = Complex(input_shape)
    y = Complex(input_shape) * -0.5

    xtt = ttl.tensor.Tensor(x.metal, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg)
    ytt = ttl.tensor.Tensor(y.metal, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg)

    tt_dev = ttnn.add(xtt, ytt)
    tt_dev = tt_dev.cpu().to(ttl.tensor.Layout.ROW_MAJOR).to_torch()

    tt_cpu = x.add(y).metal

    passing, output = comp_pcc(tt_cpu, tt_dev)
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
@pytest.mark.parametrize("bs", ((1, 1), (2, 1), (2, 2)))
def test_level1_sub(bs, memcfg, dtype, device, function_level_defaults):
    input_shape = torch.Size([1, 1, 32, 64])
    # check sub
    x = Complex(input_shape)
    y = Complex(input_shape) * 0.5

    xtt = ttl.tensor.Tensor(x.metal, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg)
    ytt = ttl.tensor.Tensor(y.metal, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg)

    tt_dev = ttnn.sub(xtt, ytt)
    tt_dev = tt_dev.cpu().to(ttl.tensor.Layout.ROW_MAJOR).to_torch()

    tt_cpu = x.sub(y).metal

    passing, output = comp_pcc(tt_cpu, tt_dev)
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
@pytest.mark.parametrize("bs", ((1, 1), (2, 1), (2, 2)))
def test_level1_mul_bs(bs, memcfg, dtype, device, function_level_defaults):
    input_shape = torch.Size([bs[0], bs[1], 32, 64])
    # check abs
    x = Complex(input_shape)
    y = Complex(input_shape) * 0.75

    xtt = ttl.tensor.Tensor(x.metal, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg)
    ytt = ttl.tensor.Tensor(y.metal, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg)

    tt_dev = ttl.tensor.complex_mul(xtt, ytt, memcfg)
    tt_dev = tt_dev.cpu().to(ttl.tensor.Layout.ROW_MAJOR).to_torch()

    tt_cpu = x.mul(y).metal

    passing, output = comp_pcc(tt_cpu, tt_dev, pcc=0.96)
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
def test_level1_mul(bs, memcfg, dtype, device, function_level_defaults):
    input_shape = torch.Size([bs[0], bs[1], 32, 64])
    # check abs
    x = Complex(input_shape)
    y = Complex(input_shape) * 0.75

    xtt = ttl.tensor.Tensor(x.metal, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg)
    ytt = ttl.tensor.Tensor(y.metal, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg)

    tt_dev = ttl.tensor.complex_mul(xtt, ytt, memcfg)
    tt_dev = tt_dev.cpu().to(ttl.tensor.Layout.ROW_MAJOR).to_torch()

    tt_cpu = x.mul(y).metal

    passing, output = comp_pcc(tt_cpu, tt_dev, pcc=0.96)
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
def test_level1_div(memcfg, dtype, device, function_level_defaults):
    input_shape = torch.Size([1, 1, 32, 64])
    # check abs
    x = Complex(input_shape)
    y = Complex(input_shape) * 0.75

    xtt = ttl.tensor.Tensor(x.metal, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg)
    ytt = ttl.tensor.Tensor(y.metal, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg)

    tt_dev = ttl.tensor.complex_div(ytt, xtt, memcfg)
    tt_dev = tt_dev.cpu().to(ttl.tensor.Layout.ROW_MAJOR).to_torch()

    tt_cpu = y.div(x).metal

    passing, output = comp_pcc(tt_cpu, tt_dev, pcc=0.96)
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
def test_level1_recip(memcfg, dtype, device, function_level_defaults):
    input_shape = torch.Size([1, 1, 32, 64])
    # check abs
    x = Complex(input_shape)
    x = x.div(x * 0.5)
    xtt = ttl.tensor.Tensor(x.metal, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg)

    tt_dev = ttl.tensor.complex_recip(xtt, memcfg)
    tt_dev = tt_dev.cpu().to(ttl.tensor.Layout.ROW_MAJOR).to_torch()

    tt_cpu = x.recip().metal

    passing, output = comp_pcc(tt_cpu, tt_dev, pcc=0.96)
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
@pytest.mark.parametrize("bs", ((1, 1), (1, 2), (2, 2)))
def test_level1_polar(bs, memcfg, dtype, device, function_level_defaults):
    input_shape = torch.Size([bs[0], bs[1], 32, 32])
    # check polar function

    # we set real = abs = 1 on unit circle
    # we set imag = angle theta
    x = Complex(None, re=torch.ones(input_shape), im=torch.rand(input_shape))

    xtt = ttl.tensor.complex_tensor(
        ttl.tensor.Tensor(x.real, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg),
        ttl.tensor.Tensor(x.imag, dtype).to(ttl.tensor.Layout.TILE).to(device, memcfg),
    )
    tt_dev = ttl.tensor.polar(xtt.real, xtt.imag, memcfg)
    tt_dev = tt_dev.cpu().to(ttl.tensor.Layout.ROW_MAJOR).to_torch()
    tt_cpu = torch.polar(x.real, x.imag)
    tt_cpu = x.metal

    passing, output = comp_allclose(tt_cpu, tt_dev, 0.0125, 1)
    logger.info(output)
    assert passing
