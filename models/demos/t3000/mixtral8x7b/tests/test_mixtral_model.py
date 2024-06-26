# SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.

# SPDX-License-Identifier: Apache-2.0
import os
import torch
import pytest
import numpy as np
from loguru import logger
from sklearn.metrics import top_k_accuracy_score
import ttnn
from models.demos.t3000.mixtral8x7b.tt.mixtral_common import prepare_inputs_ttnn, prepare_rotation_mat_ttnn
from models.demos.t3000.mixtral8x7b.tt.mixtral_model import TtTransformer
from models.demos.t3000.mixtral8x7b.reference.model import Transformer
from models.demos.t3000.mixtral8x7b.reference.tokenizer import Tokenizer
from models.utility_functions import comp_pcc, comp_allclose, get_devices_for_t3000

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


@pytest.mark.parametrize(
    "validation_type",
    ("pcc", "output"),
)
@pytest.mark.parametrize(
    "n_layers",
    (1, 32),
)
@pytest.mark.parametrize(
    "iterations",
    (1, 10, 127),
)
def test_mixtral_model_inference(all_devices, iterations, n_layers, validation_type, use_program_cache, reset_seeds):
    pcc = 0.97
    dtype = ttnn.bfloat8_b

    devices = all_devices
    num_devices = len(devices)
    assert num_devices == 8, "This test requires a T3000 (8 devices)"
    devices = get_devices_for_t3000(devices, num_devices)

    model_args = TtModelArgs(devices[0])
    model_args.n_layers = n_layers

    state_dict = torch.load(model_args.state_dict_path)
    keys_dict = list(state_dict.keys())[:]
    remv = [f"layers.{i}" for i in range(n_layers, 32)]
    for k in keys_dict:
        if any([r in k for r in remv]):
            state_dict.pop(k)

    tokenizer = Tokenizer(model_args.tokenizer_path)

    prompts = ["Once"] * 32

    encoded_prompts = [tokenizer.encode(prompt) for prompt in prompts]

    if validation_type == "pcc":
        reference_model = Transformer(args=model_args)
        reference_model.load_state_dict(state_dict)
        reference_model.eval()

    # Embedding on host
    embd = Emb()
    embd.load_state_dict({"emb.weight": state_dict["tok_embeddings.weight"]})

    # Load TTNN model
    tt_model = TtTransformer(
        devices=devices,
        state_dict=state_dict,
        args=model_args,
        layers=list(range(model_args.n_layers)),
        dtype=dtype,
    )

    rot_mat = prepare_rotation_mat_ttnn(
        model_args.head_dim,
        model_args.max_seq_len,
        tt_model.devices,
    )

    generation_start_pos = 0
    generation_length = iterations
    all_tests_pass = True

    seqlen = 1  # Generating one token per user at a time
    batch = 32

    # Select the first token from the prompts for initial decoding
    encoded_prompts_tensor = torch.tensor(encoded_prompts)  # [:,0]
    pt_decode_input = embd(encoded_prompts_tensor[:, 0]).view(batch, seqlen, -1)

    tt_decode_input = pt_decode_input
    ref_tokens = []
    tt_tokens = []

    for i in range(generation_length):
        logger.info(f"[Decode] Generating token {i}")

        start_pos = generation_start_pos + i
        current_pos = start_pos % model_args.sliding_window

        decode_input = prepare_inputs_ttnn(
            tt_decode_input,
            model_args.dim,
            tt_model.devices,
        )

        # Run TT model
        tt_out = tt_model(decode_input, start_pos, current_pos, rot_mat)

        # Convert ttnn tensor to torch tensor
        tt_output_torch = ttnn.to_torch(tt_out[0]).squeeze(1).view(batch, seqlen, -1).detach().float()

        # Measure PCC
        if validation_type == "pcc":
            positions = torch.LongTensor([start_pos])
            ref_output = reference_model(pt_decode_input, positions).detach().float()

            passing, pcc_message = comp_pcc(
                ref_output.view(batch, seqlen, -1), tt_output_torch.view(batch, seqlen, -1), pcc
            )
            logger.info(comp_allclose(ref_output, tt_output_torch))
            logger.info(pcc_message)

            reference_top1 = np.argmax(ref_output, axis=-1).squeeze()
            top1_acc = top_k_accuracy_score(
                reference_top1, tt_output_torch.squeeze(), k=1, labels=np.arange(tt_output_torch.shape[-1])
            )
            top5_acc = top_k_accuracy_score(
                reference_top1, tt_output_torch.squeeze(), k=5, labels=np.arange(tt_output_torch.shape[-1])
            )
            logger.info(f"Mean Top-1: {top1_acc}")
            logger.info(f"Mean Top-5: {top5_acc}")

            ref_token_batch = ref_output.squeeze().argmax(axis=-1)
            tt_token_batch = tt_output_torch.squeeze().argmax(axis=-1)
            logger.info(f"ref_output: {tokenizer.decode(ref_token_batch[0].item())}")
            logger.info(f"tt_output: {tokenizer.decode(tt_token_batch[0].item())}")
            pt_decode_input = embd(ref_token_batch).view(batch, seqlen, -1)
            tt_decode_input = pt_decode_input  # teacher forcing for PCC test
        else:
            tt_token_batch = tt_output_torch.squeeze().argmax(axis=-1)
            tt_tokens.append(tt_token_batch[0].item())
            logger.info(f'tt_output_torch: {"".join(tokenizer.decode(tt_tokens))}')
            tt_decode_input = embd(tt_token_batch).view(batch, seqlen, -1)

        if validation_type == "pcc":
            if passing:
                logger.info("Mistral Model Passed!")
            else:
                logger.warning("Mistral Model Failed!")
                all_tests_pass = False

    if validation_type == "output":
        if iterations == 1:  # First generated token will be a empty character, so just ignore output validation
            all_tests_pass = True
        elif iterations == 10:
            expected_output = "# The 10 Best Places to Live"
            logger.info(f"Expected output: {expected_output}")
            if "".join(tokenizer.decode(tt_tokens)) == expected_output:
                all_tests_pass = True
            else:
                all_tests_pass = False
        elif iterations == 127:  # TODO Check the baseline output for 127 iterations
            logger.info("Output validation not yet implemented for 127 iterations.")
            all_tests_pass = True
        else:
            logger.info("Output validation not  implemented for this iteration count!")
            all_tests_pass = True

    if all_tests_pass:
        logger.info(f"All {generation_length} Mistral decode iterations Passed!")
    else:
        logger.warning("One or more iterations of Mistral decode Failed!")
        if validation_type == "pcc":
            assert all_tests_pass, f"PCC value is lower than {pcc} for some of the outputs. Check Warnings!"
        else:
            logger.info(f'Generated output: {"".join(tokenizer.decode(tt_tokens))}')
            assert all_tests_pass, f"Expected output did not match the generated output!"
