/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#include "RateControlCBR.h"

/*
 * @private
 * @function rate_control_cbr_get_qp_for_size. Helper to extract the suggested QP from the
 * prediction model for a desired size
 * @param {EbRateControlModel*} model_ptr.
 * @param {uint32_t} desired_size.
 * @param {uint32_t} complexity.
 * @return {uint32_t}.
 */
static uint32_t rate_control_cbr_get_qp_for_size(EbRateControlModel *model_ptr, uint32_t desired_size, uint32_t complexity);

void rate_control_cbr_init(EbRateControlModel *model_ptr) {
}

void rate_control_record_new_gop_cbr(EbRateControlModel *model_ptr, EbRateControlGopInfo *gop_ptr) {
    uint32_t gop_per_second = (model_ptr->frame_rate << 8) / model_ptr->intra_period;
    uint32_t gop_size = ((model_ptr->desired_bitrate << 8) / gop_per_second);

    // Ideally the gop size is equal to bitrate devide by the number of GOP per second
    gop_ptr->desired_size = gop_size / 2;

    gop_ptr->qp = rate_control_cbr_get_qp_for_size(model_ptr, gop_ptr->desired_size, gop_ptr->complexity);
}

static uint32_t rate_control_cbr_get_qp_for_size(EbRateControlModel *model_ptr, uint32_t desired_size, uint32_t complexity) {
    return 35;
}
