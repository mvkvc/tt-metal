// SPDX-FileCopyrightText: © 2023 Tenstorrent Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include "tt_eager/tt_dnn/kernels/compute/moreh_common.hpp"
namespace NAMESPACE {
void MAIN {
    constexpr int onetile = 1;
    ArgFetcher arg_fetcher;
    const uint32_t batch_num = arg_fetcher.get_next_arg_val<uint32_t>();
    const uint32_t Ht = arg_fetcher.get_next_arg_val<uint32_t>();
    const uint32_t Wt = arg_fetcher.get_next_arg_val<uint32_t>();
    const bool do_mask_h = (arg_fetcher.get_next_arg_val<uint32_t>() == 1);
    const bool do_mask_w = (arg_fetcher.get_next_arg_val<uint32_t>() == 1);

    constexpr auto cb_in0 = tt::CB::c_in0;
    constexpr auto cb_scaler = tt::CB::c_in1;
    constexpr auto cb_mask_h_w = tt::CB::c_in2;
    constexpr auto cb_intermed0 = tt::CB::c_intermed0;
    constexpr auto cb_intermed1 = tt::CB::c_intermed1;
    constexpr auto cb_out = tt::CB::c_out0;
    constexpr uint32_t dst0 = 0;
    constexpr uint32_t dst1 = 1;

    binary_op_init_common(tt::CB::c_in0, tt::CB::c_in0);
    cb_wait_front(cb_scaler, onetile);

    if (do_mask_h || do_mask_w) {
        cb_wait_front(cb_mask_h_w, onetile * 2);
    }

    bool enable_reload = false;
    uint32_t num_tiles = batch_num * Ht * Wt;
    uint32_t num_tile_done = 0;
    for (uint32_t b = 0; b < batch_num; ++b) {
        for (uint32_t ht = 0; ht < Ht; ++ht) {
            for (uint32_t wt = 0; wt < Wt; ++wt) {
                bool last_row = (ht == Ht - 1);
                bool last_col = (wt == Wt - 1);
                bool last_out = (num_tile_done == num_tiles - 1);
                bool do_mask = (do_mask_h && last_row) || (do_mask_w && last_col);

                // get tile from reader
                cb_wait_front(cb_in0, onetile);

                if (do_mask) {
                    tile_regs_acquire();
                    ACQ();
                    copy_tile_init();
                    copy_tile(cb_in0, 0, dst0);

                    if (do_mask_h && last_row) {
                        copy_tile_init();
                        copy_tile(cb_mask_h_w, 0, dst1);
                        mask_tile_init();
                        mask_tile(dst0, dst1);
                    }

                    if (do_mask_w && last_col) {
                        copy_tile_init();
                        copy_tile(cb_mask_h_w, 1, dst1);
                        mask_tile_init();
                        mask_tile(dst0, dst1);
                    }
                    tile_regs_commit();

                    tile_regs_wait();
                    cb_reserve_back(cb_intermed0, onetile);
                    pack_tile(dst0, cb_intermed0);
                    cb_push_back(cb_intermed0, onetile);
                    tile_regs_release();
                }

                tile_regs_acquire();
                if (enable_reload) {
                    cb_wait_front(cb_intermed1, onetile);
                    copy_tile_init();
                    copy_tile(cb_intermed1, 0, 0);
                    cb_pop_front(cb_intermed1, onetile);
                }

                if (do_mask)
                    cb_wait_front(cb_intermed0, onetile);

                reduce_init_delta<false>(REDUCE_OP, REDUCE_DIM);
                reduce_tile((do_mask) ? (cb_intermed0) : (cb_in0), cb_scaler, 0, 0, 0);
                reduce_revert_delta();

                if (do_mask)
                    cb_pop_front(cb_intermed0, onetile);

                cb_pop_front(cb_in0, onetile);
                tile_regs_commit();

                tile_regs_wait();
                if (last_out) {
                    cb_reserve_back(cb_out, onetile);
                    pack_tile(0, cb_out);
                    cb_push_back(cb_out, onetile);

                }
                else {
                    cb_reserve_back(cb_intermed1, onetile);
                    pack_tile(0, cb_intermed1);
                    cb_push_back(cb_intermed1, onetile);
                }
                tile_regs_release();

                enable_reload = true;
                num_tile_done++;
            }
        }
    }
}
}  // namespace NAMESPACE
