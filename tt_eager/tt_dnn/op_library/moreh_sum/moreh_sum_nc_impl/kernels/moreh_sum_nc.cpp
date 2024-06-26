// SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include "tt_eager/tt_dnn/kernels/compute/moreh_common.hpp"

namespace NAMESPACE {
void MAIN {
    ArgFetcher arg_fetcher;
    const auto num_input_tiles = arg_fetcher.get_next_arg_val<uint32_t>();
    const auto num_output_tiles = arg_fetcher.get_next_arg_val<uint32_t>();

    constexpr auto cb_in0 = tt::CB::c_in0;
    constexpr auto cb_in1 = tt::CB::c_in1;
    constexpr auto cb_out0 = tt::CB::c_out0;
    constexpr auto cb_intermed0 = tt::CB::c_intermed0;
    constexpr uint32_t onetile = 1;
    constexpr uint32_t dst0 = 0;
    constexpr uint32_t dst1 = 1;
    constexpr uint32_t first_tile = 0;

    binary_op_init_common(tt::CB::c_in0, tt::CB::c_in1);
    cb_wait_front(cb_in1, onetile);

    for (uint32_t i = 0; i < num_output_tiles; i++) {
        bool enable_reload = false;
        for (uint32_t j = 0; j < num_input_tiles; ++j) {
            bool last_out = (j == num_input_tiles - 1);
            uint32_t cb_add  = (enable_reload) ? (cb_intermed0) : (cb_in1);

            ACQ();
            cb_wait_front(cb_in0, onetile);
            if (enable_reload) {
                cb_wait_front(cb_intermed0, onetile);
            }

            add_tiles_init();
            add_tiles(cb_in0, cb_add, first_tile, first_tile, dst0);

            cb_pop_front(cb_in0, onetile);
            if (enable_reload) {
                cb_pop_front(cb_intermed0, onetile);
            }

            uint32_t cb_out = (last_out) ? (cb_out0) : (cb_intermed0);
            cb_reserve_back(cb_out, onetile);
            pack_tile(dst0, cb_out);
            cb_push_back(cb_out, onetile);
            REL();
            enable_reload = true;
        }
    }
}
}  // namespace NAMESPACE
