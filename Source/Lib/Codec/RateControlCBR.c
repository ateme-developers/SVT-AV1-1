/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#include "RateControlCBR.h"

void rate_control_cbr_init(EbRateControlModel *model_ptr) {
    EbRateControlComplexityModelDeviation *current = EB_NULL;

    for (unsigned int i = 0; model_ptr->complexity_variation_model[i].scope_end != 0; i++) {
        current = &model_ptr->complexity_variation_model[i];

        current->deviation = -5;
    }
}

void rate_control_cbr_gop_completed(EbRateControlModel *model_ptr, EbRateControlGopInfo *gop_ptr) {
    EbRateControlComplexityModelDeviation *deviation_model = EB_NULL;
    float variation = calculate_gop_variation(model_ptr, gop_ptr, &deviation_model);

    if (variation < 1.0) {
        deviation_model->deviation = ((deviation_model->deviation * deviation_model->deviation_reported) + variation) / (deviation_model->deviation_reported + 1);
        if (deviation_model->deviation_reported != MAX_COMPLEXITY_MODEL_DEVIATION_REPORTED) {
            deviation_model->deviation_reported++;
        }
    }
}

void rate_control_record_new_gop_cbr(EbRateControlModel *model_ptr, EbRateControlGopInfo *gop_ptr) {
    uint32_t gop_per_second = (model_ptr->frame_rate << 8) / model_ptr->intra_period;
    uint32_t gop_size = ((model_ptr->desired_bitrate << 8) / gop_per_second);

    // Ideally the gop size is equal to bitrate devide by the number of GOP per second
    gop_ptr->desired_size = gop_size;

    if (gop_ptr->index == 0) {
        gop_ptr->desired_size /= 2;
    }

    gop_ptr->qp = rate_control_get_qp_for_size(model_ptr, gop_ptr->desired_size, gop_ptr->complexity);
}