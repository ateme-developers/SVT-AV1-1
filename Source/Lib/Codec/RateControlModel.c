/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#include "EbUtility.h"

#include "EbPictureControlSet.h"
#include "RateControlGopInfo.h"
#include "RateControlModel.h"

/*
 * @private
 * @function get_inter_qp_for_size. Helper to extract the suggested QP from the
 * prediction model for a desired size
 * @param {EbRateControlModel*} model_ptr.
 * @param {uint32_t} desired_size.
 * @return {uint32_t}.
 */
static uint32_t get_inter_qp_for_size(EbRateControlModel *model_ptr,
                                      uint32_t desired_size,
                                      uint32_t complexity);

/*
 * @private
 * @function record_new_gop. Take into account a new group of picture in the
 * model
 * @param {EbRateControlModel*} model_ptr.
 * @param {PictureParentControlSet_t*} picture_ptr. Picture holding the intra frame.
 * @return {void}.
 */
static void record_new_gop(EbRateControlModel *model_ptr, PictureParentControlSet_t *picture_ptr);

/*
 * Average size in bits for and inter frame per QP for a 1920x1080 reference video clip
 */
static const size_t DEFAULT_REF_INTER_PICTURE_COMPRESSION_RATIO[64] = {
    12459127, 10817116, 9175105, 7533094, 5891083, 4249072, 3819718, 3390366,
    2961014, 2531662, 2102309, 1931469, 1760631, 1589793, 1418955, 1248117,
    1165879, 1083646, 1001413, 919180, 836947, 783885, 730823, 677761,
    624699, 571634, 531742, 491850, 451958, 412066, 372174, 344015,
    315858, 287701, 259544, 231387, 213604, 195823, 178042, 160261, 142480,
    131278, 120077, 108876, 97675, 86474, 79681, 72892, 66103, 59314,
    52525, 48203, 43882, 39561, 35240, 30919, 28284, 25651, 23018,
    20385, 17752, 15465, 13178, 10891};

static EbRateControlComplexityModel DEFAULT_COMPLEXITY_MODEL[] = {
    {0, 26000, {38000000, 35926667, 33853333, 31780000, 29706667, 27633333, 25560000, 23486667, 21413333, 19340000, 17266667, 15193333, 13120000, 11046667, 8973333, 6900000, 6640000, 6380000, 6120000, 5860000, 5600000, 5340000, 5080000, 4820000, 4560000, 4300000, 4070000, 3840000, 3610000, 3380000, 3150000, 2920000, 2690000, 2460000, 2230000, 2000000, 1892100, 1784200, 1676300, 1568400, 1460500, 1352600, 1244700, 1136800, 1028900, 921000, 855900, 790800, 725700, 660600, 595500, 530400, 465300, 400200, 335100, 270000, 250125, 230250, 210375, 190500, 170625, 150750, 130875, 111000}},
    {26001, 162000, {452000000, 423106667, 394213333, 365320000, 336426667, 307533333, 278640000, 249746667, 220853333, 191960000, 163066667, 134173333, 105280000, 76386667, 47493333, 18600000, 17630000, 16660000, 15690000, 14720000, 13750000, 12780000, 11810000, 10840000, 9870000, 8900000, 8610000, 8320000, 8030000, 7740000, 7450000, 7160000, 6870000, 6580000, 6290000, 6000000, 5640000, 5280000, 4920000, 4560000, 4200000, 3840000, 3480000, 3120000, 2760000, 2400000, 2233000, 2066000, 1899000, 1732000, 1565000, 1398000, 1231000, 1064000, 897000, 730000, 681875, 633750, 585625, 537500, 489375, 441250, 393125, 345000}},
    {162001, 360000, {686000000, 646466667, 606933333, 567400000, 527866667, 488333333, 448800000, 409266667, 369733333, 330200000, 290666667, 251133333, 211600000, 172066667, 132533333, 93000000, 88920000, 84840000, 80760000, 76680000, 72600000, 68520000, 64440000, 60360000, 56280000, 52200000, 49510000, 46820000, 44130000, 41440000, 38750000, 36060000, 33370000, 30680000, 27990000, 25300000, 23820000, 22340000, 20860000, 19380000, 17900000, 16420000, 14940000, 13460000, 11980000, 10500000, 9850000, 9200000, 8550000, 7900000, 7250000, 6600000, 5950000, 5300000, 4650000, 4000000, 3668750, 3337500, 3006250, 2675000, 2343750, 2012500, 1681250, 1350000}},
    {0, 0, {0}}};

static EbRateControlComplexityModelDeviation COMPLEXITY_DEVIATION[] = {
    {0, 10000, 1, 0},
    {10001, 20000, 1, 0},
    {20001, 30000, 1, 0},
    {30001, 40000, 1, 0},
    {40001, 50000, 1, 0},
    {50001, 60000, 1, 0},
    {60001, 70000, 1, 0},
    {70001, 80000, 1, 0},
    {80001, 90000, 1, 0},
    {90001, 100000, 1, 0},
    {100001, 110000, 1, 0},
    {110001, 120000, 1, 0},
    {120001, 130000, 1, 0},
    {130001, 140000, 1, 0},
    {140001, 150000, 1, 0},
    {150001, 160000, 1, 0},
    {160001, 170000, 1, 0},
    {170001, 170000, 1, 0},
    {180001, 190000, 1, 0},
    {190001, 200000, 1, 0},
    {200001, 210000, 1, 0},
    {210001, 220000, 1, 0},
    {220001, 230000, 1, 0},
    {230001, 240000, 1, 0},
    {240001, 250000, 1, 0},
    {250001, 260000, 1, 0},
    {260001, 270000, 1, 0},
    {270001, 280000, 1, 0},
    {280001, 290000, 1, 0},
    {290001, 300000, 1, 0},
    {300001, 310000, 1, 0},
    {310001, 320000, 1, 0},
    {320001, 330000, 1, 0},
    {330001, 340000, 1, 0},
    {340001, 350000, 1, 0},
    {350001, 360000, 1, 0},
    {360001, 370000, 1, 0},
    {370001, 380000, 1, 0},
    {380001, 390000, 1, 0},
    {390001, 400000, 1, 0},
    {400001, 999999999, 1, 0},
};

EbErrorType rate_control_model_ctor(EbRateControlModel **object_doubble_ptr) {
    EbRateControlModel *model_ptr;

    EB_MALLOC(EbRateControlModel *, model_ptr, sizeof(EbRateControlModel), EB_N_PTR);
    *object_doubble_ptr = (void *)model_ptr;

    EB_MEMSET(model_ptr, 0, sizeof(EbRateControlModel));
    EB_MEMCPY(model_ptr->inter_size_predictions, (void *)DEFAULT_REF_INTER_PICTURE_COMPRESSION_RATIO, sizeof(DEFAULT_REF_INTER_PICTURE_COMPRESSION_RATIO));

    EB_CREATEMUTEX(EbHandle, model_ptr->model_mutex, sizeof(EbHandle), EB_MUTEX);

    EB_MALLOC(EbRateControlComplexityModelDeviation *, model_ptr->complexity_variation_model, sizeof(EbRateControlComplexityModelDeviation) * sizeof(COMPLEXITY_DEVIATION), EB_N_PTR);
    EB_MEMCPY(model_ptr->complexity_variation_model, (void *)COMPLEXITY_DEVIATION, sizeof(EbRateControlComplexityModelDeviation) * sizeof(COMPLEXITY_DEVIATION));

    return EB_ErrorNone;
}

EbErrorType rate_control_model_init(EbRateControlModel *model_ptr, SequenceControlSet_t *sequenceControlSetPtr) {
    uint32_t number_of_frame = sequenceControlSetPtr->static_config.framesToBeEncoded;
    EbRateControlGopInfo *gop_infos;

    EB_MALLOC(EbRateControlGopInfo *, gop_infos, sizeof(EbRateControlModel) * number_of_frame, EB_N_PTR);
    memset(gop_infos, 0, sizeof(EbRateControlModel) * number_of_frame);

    model_ptr->desired_bitrate = sequenceControlSetPtr->static_config.target_bit_rate;
    model_ptr->frame_rate = sequenceControlSetPtr->static_config.frame_rate >> 16;
    model_ptr->width = sequenceControlSetPtr->luma_width;
    model_ptr->height = sequenceControlSetPtr->luma_height;
    model_ptr->pixels = model_ptr->width * model_ptr->height;
    model_ptr->gop_infos = gop_infos;
    model_ptr->intra_period = sequenceControlSetPtr->static_config.intra_period_length;
    model_ptr->number_of_frame = number_of_frame;

    return EB_ErrorNone;
}

EbErrorType rate_control_report_complexity(EbRateControlModel *model_ptr, PictureParentControlSet_t *picture_ptr) {
    EbBlockOnMutex(model_ptr->model_mutex);

    model_ptr->gop_infos[picture_ptr->picture_number].complexity = picture_ptr->complexity;

    EbReleaseMutex(model_ptr->model_mutex);

    return EB_ErrorNone;
}

EbErrorType rate_control_update_model(EbRateControlModel *model_ptr, PictureParentControlSet_t *picture_ptr) {
    uint64_t size = picture_ptr->total_num_bits;
    EbRateControlGopInfo *gop = get_gop_infos(model_ptr->gop_infos, picture_ptr->picture_number);

    EbBlockOnMutex(model_ptr->model_mutex);
    model_ptr->total_bytes += size;
    model_ptr->reported_frames++;
    gop->actual_size += size;
    gop->reported_frames++;

    if (gop->reported_frames == gop->length) {
        float variation = 1;

        if (gop->actual_size > gop->desired_size) {
            variation = (float)gop->actual_size / (float)gop->desired_size;
        } else if (gop->actual_size < gop->desired_size) {
            variation = -((float)(gop->desired_size / (float)gop->actual_size));
        }

        EbRateControlComplexityModelDeviation *deviation_model = EB_NULL;
        for (unsigned int i = 0; i < sizeof(COMPLEXITY_DEVIATION); i++) {
            deviation_model = &model_ptr->complexity_variation_model[i];
            if (gop->complexity >= deviation_model->scope_start && gop->complexity <= deviation_model->scope_end) {
                float deviation = 0;
                if (gop->model_variation < -1.0) {
                    deviation = variation + gop->model_variation;
                } else {
                    deviation = variation - gop->model_variation;
                }

                deviation_model->deviation = ((deviation_model->deviation * deviation_model->deviation_reported) + deviation) / (deviation_model->deviation_reported + 1);
                if (deviation_model->deviation_reported != MAX_COMPLEXITY_MODEL_DEVIATION_REPORTED) {
                    deviation_model->deviation_reported++;
                }
                break;
            }
        }
    }

    EbReleaseMutex(model_ptr->model_mutex);

    return EB_ErrorNone;
}

uint8_t rate_control_get_quantizer(EbRateControlModel *model_ptr, PictureParentControlSet_t *picture_ptr) {
    FRAME_TYPE type = picture_ptr->av1FrameType;

    if (type == INTRA_ONLY_FRAME || type == KEY_FRAME) {
        record_new_gop(model_ptr, picture_ptr);
    }

    EbRateControlGopInfo *gop = get_gop_infos(model_ptr->gop_infos, picture_ptr->picture_number);

    return gop->qp;
}

uint32_t get_inter_qp_for_size(EbRateControlModel *model_ptr, uint32_t desired_size, uint32_t complexity) {
    uint8_t qp = 63;

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
        EbRateControlComplexityModel *previous_complexity_model = &DEFAULT_COMPLEXITY_MODEL[0];
        for (unsigned int i = 0; DEFAULT_COMPLEXITY_MODEL[i].scope_end != 0; i++) {
            complexity_model = &DEFAULT_COMPLEXITY_MODEL[i];
            if (complexity >= complexity_model->scope_start && complexity <= complexity_model->scope_end) {
                break;
            }
            previous_complexity_model = complexity_model;
        }

        for (unsigned int i = 0; i < sizeof(COMPLEXITY_DEVIATION); i++) {
            deviation_model = &model_ptr->complexity_variation_model[i];
            if (complexity >= deviation_model->scope_start && complexity <= deviation_model->scope_end) {
                break;
            }
        }

        if (deviation_model->deviation < -1.0) {
            desired_size = (float)desired_size * -(float)deviation_model->deviation;
        } else if (deviation_model->deviation > 1.0) {
            desired_size = (float)desired_size / (float)deviation_model->deviation;
        }

        for (unsigned int i = 5; i < MAX_QP_VALUE; i++) {
            uint32_t max_size = complexity_model->max_size[i];
            uint32_t previous_max_size = (previous_complexity_model == complexity_model) ? 100000 : previous_complexity_model->max_size[i];
            uint32_t pitch = (max_size - previous_max_size) / (complexity_model->scope_end - complexity_model->scope_start);
            uint32_t complexity_steps = (complexity % max_size) - complexity_model->scope_start;
            uint32_t qp_size = previous_max_size + (pitch * complexity_steps);

            if (desired_size > (qp_size / 55)) {
                qp = i;
                break;
            }
        }
    }

    return qp;
}

static void record_new_gop(EbRateControlModel *model_ptr, PictureParentControlSet_t *picture_ptr) {
    uint64_t pictureNumber = picture_ptr->picture_number;
    EbRateControlGopInfo *gop = &model_ptr->gop_infos[pictureNumber];

    EbBlockOnMutex(model_ptr->model_mutex);

    gop->index = pictureNumber;
    gop->exists = EB_TRUE;
    gop->desired_size = get_gop_size_in_bytes(model_ptr);

    uint32_t desired_total_bytes = (model_ptr->desired_bitrate / model_ptr->frame_rate) * model_ptr->reported_frames;
    int64_t delta_bytes = desired_total_bytes - model_ptr->total_bytes;
    int64_t extra = delta_bytes / 4;

    if (extra < 0 && gop->desired_size < (uint64_t)-extra) {
        extra = -gop->desired_size + 1;
    }

    gop->desired_size += extra;

    uint32_t size = gop->desired_size / model_ptr->intra_period;
    uint32_t complexity = estimate_gop_complexity(model_ptr, gop);
    EbRateControlComplexityModelDeviation *deviation_model = EB_NULL;

    for (unsigned int i = 0; i < sizeof(COMPLEXITY_DEVIATION); i++) {
        deviation_model = &model_ptr->complexity_variation_model[i];
        if (complexity >= deviation_model->scope_start && complexity <= deviation_model->scope_end) {
            break;
        }
    }

    gop->complexity = complexity;
    gop->qp = get_inter_qp_for_size(model_ptr, size, complexity);
    gop->model_variation = deviation_model->deviation;

    // Update length in gopinfos
    if (pictureNumber != 0) {
        EbRateControlGopInfo *previousGop = get_gop_infos(model_ptr->gop_infos, pictureNumber - 1);

        previousGop->length = gop->index - previousGop->index;
    }

    EbReleaseMutex(model_ptr->model_mutex);
}

uint32_t get_gop_size_in_bytes(EbRateControlModel *model_ptr) {
    uint32_t gop_per_second = (model_ptr->frame_rate << 8) / model_ptr->intra_period;
    uint32_t gop_size = ((model_ptr->desired_bitrate << 8) / gop_per_second);

    return gop_size;
}
