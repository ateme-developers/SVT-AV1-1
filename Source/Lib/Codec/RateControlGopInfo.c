/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#include "RateControlGopInfo.h"
#include "RateControlModel.h"

EbRateControlGopInfo *get_gop_infos(EbRateControlGopInfo *gop_info,
                                    uint64_t position) {
    EbRateControlGopInfo *current;

    while (1) {  // First frame is always guaranteed to exist
        current = &gop_info[position];

        if (current->exists) {
            return current;
        }

        if (position == 0) {
            return EB_NULL;
        }
        position--;
    }

    return EB_NULL;
}

uint32_t estimate_gop_complexity(EbRateControlModel *model_ptr,
                                 EbRateControlGopInfo *gop_ptr) {
    uint32_t complexity = 0;
    EbRateControlGopInfo *current;
    int64_t position = 0;
    uint32_t reported_complexity = 0;

    while ((position + gop_ptr->index) < model_ptr->number_of_frame &&
           position < (int64_t)model_ptr->intra_period) {
        current = &(model_ptr->gop_infos[gop_ptr->index + position]);

        if (current->complexity) {
            complexity += current->complexity;
            reported_complexity++;
        }

        position++;
    }

    if (reported_complexity) {
        complexity = complexity / reported_complexity;
    }

    return complexity;
}

float calculate_gop_variation(EbRateControlModel *model_ptr,
                              EbRateControlGopInfo *gop_info,
                              EbRateControlComplexityModelDeviation **deviation_double_ptr) {
    float variation = 1;

    if (gop_info->actual_size > gop_info->desired_size) {
        variation = (float)gop_info->actual_size / (float)gop_info->desired_size;
    } else if (gop_info->actual_size < gop_info->desired_size) {
        variation = -((float)(gop_info->desired_size / (float)gop_info->actual_size));
    }

    EbRateControlComplexityModelDeviation *deviation_model = EB_NULL;
    for (unsigned int i = 0; 1; i++) {
        deviation_model = &model_ptr->complexity_variation_model[i];
        if (gop_info->complexity >= deviation_model->scope_start && gop_info->complexity <= deviation_model->scope_end) {
            float deviation = 0;
            if (gop_info->model_variation < -1.0) {
                deviation = variation + gop_info->model_variation;
            } else {
                deviation = variation - gop_info->model_variation;
            }

            *deviation_double_ptr = deviation_model;

            return variation;
            deviation_model->deviation = ((deviation_model->deviation * deviation_model->deviation_reported) + deviation) / (deviation_model->deviation_reported + 1);
            if (deviation_model->deviation_reported != MAX_COMPLEXITY_MODEL_DEVIATION_REPORTED) {
                deviation_model->deviation_reported++;
            }
            break;
        }
    }
}
