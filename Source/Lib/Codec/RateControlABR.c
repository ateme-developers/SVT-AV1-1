/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#include "RateControlABR.h"

void rate_control_abr_gop_completed(EbRateControlModel *model_ptr, EbRateControlGopInfo *gop_ptr) {
    EbRateControlComplexityModelDeviation *deviation_model = EB_NULL;
    float variation = calculate_gop_variation(model_ptr, gop_ptr, &deviation_model);

    deviation_model->deviation = ((deviation_model->deviation * deviation_model->deviation_reported) + variation) / (deviation_model->deviation_reported + 1);
    if (deviation_model->deviation_reported != MAX_COMPLEXITY_MODEL_DEVIATION_REPORTED) {
        deviation_model->deviation_reported++;
    }
}

void rate_control_record_new_gop_abr(EbRateControlModel *model_ptr, EbRateControlGopInfo *gop_ptr) {
    uint32_t gop_per_second = (model_ptr->frame_rate << 8) / model_ptr->intra_period;
    uint32_t gop_size = ((model_ptr->desired_bitrate << 8) / gop_per_second);

    // Ideally the gop size is equal to bitrate devide by the number of GOP per second
    gop_ptr->desired_size = gop_size;

    // But because we are always a little bit above or bellow target we need to compensate that on the next GOP
    uint32_t desired_total_bytes = (model_ptr->desired_bitrate / model_ptr->frame_rate) * model_ptr->reported_frames;
    int64_t delta_bytes = desired_total_bytes - model_ptr->total_bytes;
    int64_t extra = delta_bytes / 4; // GOPs are handled in parallel. Each new GOP should only try to compensate 1/4th of the deviation.

    if (extra < 0 && gop_ptr->desired_size < (uint64_t)-extra) { // GOP size cannot be less than 0.
        extra = -gop_ptr->desired_size + 1;
    }

    gop_ptr->desired_size += extra;

    gop_ptr->qp = rate_control_get_qp_for_size(model_ptr, gop_ptr->desired_size, gop_ptr->complexity);
}
