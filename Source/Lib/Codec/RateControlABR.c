/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#include "RateControlABR.h"

/*
 * @private
 * @function rate_control_abr_get_qp_for_size. Helper to extract the suggested QP from the
 * prediction model for a desired size
 * @param {EbRateControlModel*} model_ptr.
 * @param {uint32_t} desired_size.
 * @param {uint32_t} complexity.
 * @return {uint32_t}.
 */
static uint32_t rate_control_abr_get_qp_for_size(EbRateControlModel *model_ptr, uint32_t desired_size, uint32_t complexity);

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

    gop_ptr->qp = rate_control_abr_get_qp_for_size(model_ptr, gop_ptr->desired_size, gop_ptr->complexity);
}

static uint32_t rate_control_abr_get_qp_for_size(EbRateControlModel *model_ptr, uint32_t desired_size, uint32_t complexity) {
    uint32_t qp = 63;

    if (!complexity) {
        for (qp = 0; qp < MAX_QP_VALUE; qp++) {
            float size = model_ptr->inter_size_predictions[qp];

            size = (size / MODEL_DEFAULT_PIXEL_AREA) * model_ptr->pixels;
            // Scale size for current resolution
            if (desired_size > size) {
                break;
            }
        }
    } else {
        EbRateControlComplexityModelDeviation *deviation_model = EB_NULL;
        EbRateControlComplexityModel *complexity_model = EB_NULL;
        EbRateControlComplexityModel *previous_complexity_model = &G_RATE_CONTROL_DEFAULT_COMPLEXITY_MODEL[0];

        // Find the complexity model
        for (unsigned int i = 0; i < RATE_CONTROL_DEFAULT_COMPLEXITY_MODEL_SIZE; i++) {
            complexity_model = &G_RATE_CONTROL_DEFAULT_COMPLEXITY_MODEL[i];
            if (complexity >= complexity_model->scope_start && complexity <= complexity_model->scope_end) {
                break;
            }
            previous_complexity_model = complexity_model;
        }

        // Find known variation from the model
        for (unsigned int i = 0; model_ptr->complexity_variation_model[i].scope_end != 0; i++) {
            deviation_model = &model_ptr->complexity_variation_model[i];
            if (complexity >= deviation_model->scope_start && complexity <= deviation_model->scope_end) {
                break;
            }
        }

        // Adjust desired GOP size with known variation
        if (deviation_model->deviation < -1.0) {
            desired_size = (float)desired_size * -(float)deviation_model->deviation;
        } else if (deviation_model->deviation > 1.0) {
            desired_size = (float)desired_size / (float)deviation_model->deviation;
        }

        // Find corresponding QP for adjusted size in the model
        for (unsigned int i = 5; i < MAX_QP_VALUE; i++) {
            uint32_t max_size = complexity_model->max_size[i];
            uint32_t previous_max_size = (previous_complexity_model == complexity_model) ? 100000 : previous_complexity_model->max_size[i];
            uint32_t pitch = (max_size - previous_max_size) / (complexity_model->scope_end - complexity_model->scope_start);
            uint32_t complexity_steps = (complexity % max_size) - complexity_model->scope_start;
            uint32_t qp_size = previous_max_size + (pitch * complexity_steps);

            if (desired_size > qp_size) {
                qp = i;
                break;
            }
        }
    }

    return qp;
}