# SPDX-FileCopyrightText: © 2024 Tenstorrent Inc.

# SPDX-License-Identifier: Apache-2.0

from loguru import logger
import pytest
import torch
import ttnn

from tests.tt_eager.python_api_testing.sweep_tests.comparison_funcs import (
    comp_allclose,
    comp_pcc,
)
from models.demos.wormhole.mamba.tt.preprocessing import split_sequence_length


@pytest.mark.parametrize(
    "layout",
    [ttnn.ROW_MAJOR_LAYOUT],
)
@pytest.mark.parametrize(
    "B, L, chunk_size, num_chunks",
    (
        (32, 32, 32, 1),
        (32, 64, 32, 2),
        (32, 128, 32, 4),
        (32, 128, 32, 2),
        (32, 1024, 32, 8),
    ),
)
def test_splitting_sequence_length(
    B: int, L: int, chunk_size: int, num_chunks: int, layout: ttnn.Layout, device: ttnn.Device
):
    expected = torch.randint(0, 255, (1, 1, B, L), dtype=torch.int32)

    x = ttnn.from_torch(expected, dtype=ttnn.int32, device=device, layout=layout)

    result = []
    for batch_idx in range(B):
        chunks = []
        for chunk in split_sequence_length(x, batch=batch_idx, chunk_size=chunk_size):
            assert list(chunk.shape) == [1, 1, 1, chunk_size]
            chunks.append(chunk)
        result.append(ttnn.to_torch(ttnn.concat(chunks, dim=-1)))

    actual = torch.concat(result, dim=-2)
    assert actual.shape == x.shape, "Expected input shape to match output shape"

    does_pass, output_pcc = comp_pcc(expected, actual, 1.0)
    logger.info(f"PCC value: {output_pcc}")
    assert does_pass, f"PCC value ({output_pcc}) is lower than 1.0"

    does_pass, output_allclose = comp_allclose(expected, actual)
    assert does_pass, "Allclose check failed: {output_allclose}"
