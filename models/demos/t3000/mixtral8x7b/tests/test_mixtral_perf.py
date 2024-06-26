# SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.

# SPDX-License-Identifier: Apache-2.0
import torch
import pytest
import ttnn
import os

from models.demos.t3000.mixtral8x7b.tt.mixtral_common import (
    prepare_inputs_ttnn,
    prepare_rotation_mat_ttnn,
)
from models.demos.t3000.mixtral8x7b.tt.mixtral_model import TtTransformer
from models.demos.t3000.mixtral8x7b.reference.tokenizer import Tokenizer
from models.utility_functions import get_devices_for_t3000


from models.perf.perf_utils import prep_perf_report
from models.utility_functions import profiler, enable_persistent_kernel_cache

# Set Mixtral flags for CI, if CI environment is setup
if os.getenv("CI") == "true":
    os.environ["MIXTRAL_CKPT_DIR"] = "/mnt/MLPerf/tt_dnn-models/Mistral/Mixtral-8x7B-v0.1/"
    os.environ["MIXTRAL_TOKENIZER_PATH"] = "/mnt/MLPerf/tt_dnn-models/Mistral/Mixtral-8x7B-v0.1/"
    os.environ["MIXTRAL_CACHE_PATH"] = "/mnt/MLPerf/tt_dnn-models/Mistral/Mixtral-8x7B-v0.1/"

from models.demos.t3000.mixtral8x7b.tt.model_config import TtModelArgs


class Emb(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.emb = torch.nn.Embedding(32000, 4096)

    def forward(self, x):
        return self.emb(x)


@pytest.mark.model_perf_t3000
@pytest.mark.parametrize(
    "generation_start_pos, expected_compile_time, expected_inference_time",
    (
        (32, 150, 7.5),
        (128, 150, 7.5),
        (1024, 150, 7.5),
        (2048, 150, 7.5),
    ),
)
def test_mixtral_model_perf(
    all_devices, generation_start_pos, expected_compile_time, expected_inference_time, use_program_cache, reset_seeds
):
    dtype = ttnn.bfloat8_b

    devices = all_devices
    num_devices = len(devices)
    assert num_devices == 8, "This test requires a T3000 (8 devices)"
    devices = get_devices_for_t3000(devices, num_devices)

    model_args = TtModelArgs(devices[0])
    model_args.n_layers = 32
    tokenizer = Tokenizer(model_args.tokenizer_path)

    # Clear global profiler state before starting measurements
    profiler.clear()

    profiler.start("weight_loading")
    state_dict = torch.load(model_args.state_dict_path)
    keys_dict = list(state_dict.keys())[:]
    # If needed to test with fewer layers, remove the rest of the layers
    remv = [f"layers.{i}" for i in range(model_args.n_layers, 32)]
    for k in keys_dict:
        if any([r in k for r in remv]):
            state_dict.pop(k)

    profiler.end("weight_loading")

    prompts = ["Once"] * 32
    encoded_prompts = [tokenizer.encode(prompt) for prompt in prompts]

    # Embedding on host
    embd = Emb()
    embd.load_state_dict({"emb.weight": state_dict["tok_embeddings.weight"]})

    generation_length = 1

    profiler.start("Mixtral_model_setup")

    # Load TTNN model
    tt_model = TtTransformer(
        devices=devices,
        state_dict=state_dict,
        args=model_args,
        layers=list(range(model_args.n_layers)),
        dtype=dtype,
    )

    rot_mat = prepare_rotation_mat_ttnn(
        tt_model.args.head_dim,
        tt_model.args.max_seq_len,
        tt_model.devices,
    )
    profiler.end("TtMistral_model_setup")

    # Call the function
    profiler.start(f"end_to_end_inference_with_compile")
    run_inference(tt_model, embd, encoded_prompts, generation_start_pos, generation_length, rot_mat)
    profiler.end(f"end_to_end_inference_with_compile")
    profiler.print()
    compile_and_iter_time = profiler.get("model_run_for_inference_0")

    profiler.clear()
    profiler.start(f"end_to_end_inference")
    run_inference(tt_model, embd, encoded_prompts, generation_start_pos, generation_length, rot_mat)
    profiler.end(f"end_to_end_inference")
    profiler.print()
    iter_time = profiler.get("model_run_for_inference_0")

    comment = f"kv_cache_len={generation_start_pos}_num_layers={model_args.n_layers}"

    "generation_start_pos, expected_compile_time, expected_inference_time",
    (
        (32, 30, 8.5),
        (128, 30, 8.5),
        (1024, 30, 8.5),
        (2048, 30, 8.5),
    ),
    prep_perf_report(
        model_name=f"Mixtral8x7B_{comment}",
        batch_size=model_args.max_batch_size,
        inference_and_compile_time=compile_and_iter_time,
        inference_time=iter_time,
        expected_compile_time=expected_compile_time,
        expected_inference_time=expected_inference_time,
        comments=comment,
    )


def run_inference(tt_model, embd, encoded_prompts, generation_start_pos, generation_length, rot_mat):
    seqlen = 1  # Generating one token per user at a time
    batch = tt_model.args.max_batch_size

    profiler.start(f"torch_embed_initial")
    # Select the first token from the prompts for initial decoding
    encoded_prompts_tensor = torch.tensor(encoded_prompts)  # [:,0]
    pt_decode_input = embd(encoded_prompts_tensor[:, 0]).view(batch, seqlen, -1)
    tt_decode_input = pt_decode_input
    profiler.end(f"torch_embed_initial")

    for i in range(generation_length):
        start_pos = generation_start_pos + i
        current_pos = start_pos % tt_model.args.sliding_window

        profiler.start(f"prepare_inputs_for_inference_{i}")
        decode_input = prepare_inputs_ttnn(
            tt_decode_input,
            tt_model.args.dim,
            tt_model.devices,
        )
        profiler.end(f"prepare_inputs_for_inference_{i}")

        # Run TT model
        profiler.start(f"model_run_for_inference_{i}")
        tt_out = tt_model(decode_input, start_pos, current_pos, rot_mat)

        # Convert ttnn tensor to torch tensor
        profiler.start(f"result_wait_for_inference_{i}")
        tt_output_torch = ttnn.to_torch(tt_out[0]).squeeze(1).view(batch, seqlen, -1).detach().float()

        profiler.end(f"model_run_for_inference_{i}")
        profiler.end(f"result_wait_for_inference_{i}")

        profiler.start(f"torch_argmax_and_embed_{i}")
        # Greedy decode the generated token and pass it back in, this is just a perf test
        tt_token_batch = tt_output_torch.squeeze().argmax(axis=-1)
        tt_decode_input = embd(tt_token_batch).view(batch, seqlen, -1)
        profiler.end(f"torch_argmax_and_embed_{i}")

        profiler.start(f"deallocate_tt_tensors_{i}")
        for t in decode_input + tt_out:
            t.deallocate(force=True)
        profiler.end(f"deallocate_tt_tensors_{i}")
