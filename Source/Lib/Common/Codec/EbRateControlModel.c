/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#include "EbUtility.h"

#include "EbPictureControlSet.h"
#include "EbRateControlModel.h"

/*
 * @private
 * @function record_new_gop. Take into account a new group of picture in the
 * model
 * @param {EbRateControlModel*} model_ptr.
 * @param {PictureParentControlSet_t*} picture_control_set_ptr. Picture holding the intra frame.
 * @return {void}.
 */
static void record_new_gop(EbRateControlModel *model_ptr, PictureParentControlSet_t *picture_control_set_ptr);

/*
 * @private
 * @function compute_inter_size. Calculate size of inter frames with delta QP
 * @param {EbRateControlModel*} model_ptr.
 * @param {EbRateControlGopInfo*} gop_ptr.
 * @parem {uint64_t} base_qp. Intra QP.
 * @return {uint64_t}. Predicted size in bits of the inter frames in the GOP
 */
static uint64_t compute_inter_size(EbRateControlModel *model_ptr, EbRateControlGopInfo *gop_ptr, uint64_t base_qp);

/*
 * @variable uint8_t[7]. Delta QP to apply to inter frames from intra frames
 * depending on temporal layer level
 */
static uint8_t DELTA_LEVELS[7] = {3, 5, 7, 8, 9, 10, 11};

static EbRateControlComplexityModel DEFAULT_INTRA_COMPLEXITY_MODEL[] = {
    {0, 500, {
        {10499464, 12334840},
        {8766146, 10615002},
        {7032827, 8895163},
        {5299509, 7175325},
        {3566190, 5455486},
        {1832872, 3735648},
        {1605941, 3389518},
        {1379009, 3043389},
        {1152078, 2697259},
        {925146, 2351130},
        {698215, 2005000},
        {634572, 1882861},
        {570929, 1760722},
        {507286, 1638582},
        {443643, 1516443},
        {380000, 1394304},
        {366000, 1340184},
        {352000, 1286064},
        {338000, 1231944},
        {324000, 1177824},
        {310000, 1123704},
        {296000, 1069584},
        {282000, 1015464},
        {268000, 961344},
        {254000, 907224},
        {240000, 853104},
        {232000, 816786},
        {224000, 780467},
        {216000, 744149},
        {208000, 707830},
        {200000, 671512},
        {192000, 635194},
        {184000, 598875},
        {176000, 562557},
        {168000, 526238},
        {160000, 489920},
        {152100, 461311},
        {144200, 432702},
        {136300, 404094},
        {128400, 375485},
        {120500, 346876},
        {112600, 318267},
        {104700, 289658},
        {96800, 261050},
        {88900, 232441},
        {81000, 203832},
        {77000, 192230},
        {73000, 180627},
        {69000, 169025},
        {65000, 157422},
        {61000, 145820},
        {57000, 134218},
        {53000, 122615},
        {49000, 111013},
        {45000, 99410},
        {41000, 87808},
        {38375, 81082},
        {35750, 74356},
        {33125, 67630},
        {30500, 60904},
        {27875, 54178},
        {25250, 47452},
        {22625, 40726},
        {20000, 34000}}},
    {501, 1600, {
        {12334840, 15073944},
        {10615002, 13155238},
        {8895163, 11236533},
        {7175325, 9317827},
        {5455486, 7399122},
        {3735648, 5480416},
        {3389518, 5122035},
        {3043389, 4763654},
        {2697259, 4405274},
        {2351130, 4046893},
        {2005000, 3688512},
        {1882861, 3522419},
        {1760722, 3356326},
        {1638582, 3190234},
        {1516443, 3024141},
        {1394304, 2858048},
        {1340184, 2757622},
        {1286064, 2657195},
        {1231944, 2556769},
        {1177824, 2456342},
        {1123704, 2355916},
        {1069584, 2255490},
        {1015464, 2155063},
        {961344, 2054637},
        {907224, 1954210},
        {853104, 1853784},
        {816786, 1752406},
        {780467, 1651027},
        {744149, 1549649},
        {707830, 1448270},
        {671512, 1346892},
        {635194, 1245514},
        {598875, 1144135},
        {562557, 1042757},
        {526238, 941378},
        {489920, 840000},
        {461311, 798000},
        {432702, 756000},
        {404094, 714000},
        {375485, 672000},
        {346876, 630000},
        {318267, 588000},
        {289658, 546000},
        {261050, 504000},
        {232441, 462000},
        {203832, 420000},
        {192230, 396794},
        {180627, 373589},
        {169025, 350383},
        {157422, 327178},
        {145820, 303972},
        {134218, 280766},
        {122615, 257561},
        {111013, 234355},
        {99410, 211150},
        {87808, 187944},
        {81082, 174314},
        {74356, 160683},
        {67630, 147053},
        {60904, 133422},
        {54178, 119792},
        {47452, 106161},
        {40726, 92531},
        {34000, 78900}
        }},
    {1601, MAX_COMPLEXITY, {{0}}}
    };

static EbRateControlComplexityModel DEFAULT_INTER_COMPLEXITY_MODEL[] = {
    {0, 17500, { // Temporal layer index 0
        {2000, 4200000},
        {2000, 3981250},
        {2000, 3762500},
        {2000, 3543750},
        {2000, 3325000},
        {2000, 3106250},
        {2000, 2887500},
        {2000, 2668750},
        {2000, 2450000},
        {2000, 2290200},
        {2000, 2130400},
        {2000, 1970600},
        {2000, 1810800},
        {2000, 1651000},
        {2000, 1574800},
        {2000, 1498600},
        {2000, 1422400},
        {2000, 1346200},
        {2000, 1270000},
        {2000, 1204500},
        {2000, 1139000},
        {2000, 1073500},
        {2000, 1008000},
        {2000, 942500},
        {2000, 877000},
        {2000, 811500},
        {2000, 746000},
        {2000, 680500},
        {2000, 615000},
        {2000, 583300},
        {2000, 551600},
        {2000, 519900},
        {2000, 488200},
        {2000, 456500},
        {2000, 424800},
        {2000, 393100},
        {2000, 361400},
        {2000, 329700},
        {2000, 298000},
        {2000, 281400},
        {2000, 264800},
        {2000, 248200},
        {2000, 231600},
        {2000, 215000},
        {2000, 198400},
        {2000, 181800},
        {2000, 165200},
        {2000, 148600},
        {2000, 132000},
        {2000, 124100},
        {2000, 125000},
        {2000, 117100},
        {2000, 109200},
        {2000, 101300},
        {2000, 93400},
        {2000, 85500},
        {2000, 77600},
        {2000, 69700},
        {2000, 53000},
        {2000, 45938},
        {2000, 38875},
        {2000, 31813},
        {2000, 24750},
        {2000, 29000}
    }},
    {0, 17500, { // Temporal layer index 1
        {2000, 4000000},
        {2000, 3835000},
        {2000, 3670000},
        {2000, 3505000},
        {2000, 3340000},
        {2000, 3175000},
        {2000, 3010000},
        {2000, 2845000},
        {2000, 2680000},
        {2000, 2515000},
        {2000, 2350000},
        {2000, 2210000},
        {2000, 2070000},
        {2000, 1930000},
        {2000, 1790000},
        {2000, 1650000},
        {2000, 1570000},
        {2000, 1490000},
        {2000, 1410000},
        {2000, 1330000},
        {2000, 1250000},
        {2000, 1181000},
        {2000, 1112000},
        {2000, 1043000},
        {2000, 974000},
        {2000, 905000},
        {2000, 836000},
        {2000, 767000},
        {2000, 698000},
        {2000, 629000},
        {2000, 560000},
        {2000, 528500},
        {2000, 497000},
        {2000, 465500},
        {2000, 434000},
        {2000, 402500},
        {2000, 371000},
        {2000, 339500},
        {2000, 308000},
        {2000, 276500},
        {2000, 245000},
        {2000, 231000},
        {2000, 217000},
        {2000, 203000},
        {2000, 189000},
        {2000, 175000},
        {2000, 161000},
        {2000, 147000},
        {2000, 133000},
        {2000, 119000},
        {2000, 105000},
        {2000, 98192},
        {2000, 91385},
        {2000, 84577},
        {2000, 77770},
        {2000, 70962},
        {2000, 64154},
        {2000, 57347},
        {2000, 50539},
        {2000, 43732},
        {2000, 36924},
        {2000, 31741},
        {2000, 26559},
        {2000, 29500}
    }},
    {0, 17500, { // Temporal layer index 2
        {2000, 4000000},
        {2000, 3880769},
        {2000, 3761538},
        {2000, 3642308},
        {2000, 3523077},
        {2000, 3403846},
        {2000, 3284615},
        {2000, 3165385},
        {2000, 3046154},
        {2000, 2988462},
        {2000, 2853846},
        {2000, 2719231},
        {2000, 2450000},
        {2000, 2290000},
        {2000, 2130000},
        {2000, 1970000},
        {2000, 1810000},
        {2000, 1650000},
        {2000, 1568000},
        {2000, 1486000},
        {2000, 1404000},
        {2000, 1322000},
        {2000, 1240000},
        {2000, 1161600},
        {2000, 1083200},
        {2000, 1004800},
        {2000, 926400},
        {2000, 848000},
        {2000, 769600},
        {2000, 691200},
        {2000, 612800},
        {2000, 534400},
        {2000, 456000},
        {2000, 428600},
        {2000, 401200},
        {2000, 373800},
        {2000, 346400},
        {2000, 319000},
        {2000, 291600},
        {2000, 264200},
        {2000, 236800},
        {2000, 209400},
        {2000, 182000},
        {2000, 170482},
        {2000, 158965},
        {2000, 147447},
        {2000, 135930},
        {2000, 124412},
        {2000, 112894},
        {2000, 101377},
        {2000, 89859},
        {2000, 78342},
        {2000, 66824},
        {2000, 61873},
        {2000, 56923},
        {2000, 51972},
        {2000, 47021},
        {2000, 42071},
        {2000, 37120},
        {2000, 32169},
        {2000, 27218},
        {2000, 22268},
        {2000, 17317},
        {2000, 15644}
    }},
    {0, 17500, { // Temporal layer index 3   
        {2000, 4000000},
        {2000, 3889286},
        {2000, 3778571},
        {2000, 3667857},
        {2000, 3557143},
        {2000, 3446429},
        {2000, 3335714},
        {2000, 3225000},
        {2000, 3114286},
        {2000, 3003571},
        {2000, 2892857},
        {2000, 2782143},
        {2000, 2671429},
        {2000, 2450000},
        {2000, 2290000},
        {2000, 2130000},
        {2000, 1970000},
        {2000, 1810000},
        {2000, 1650000},
        {2000, 1538000},
        {2000, 1426000},
        {2000, 1314000},
        {2000, 1202000},
        {2000, 1090000},
        {2000, 1016894},
        {2000, 943787},
        {2000, 870681},
        {2000, 797574},
        {2000, 724468},
        {2000, 651362},
        {2000, 578255},
        {2000, 505149},
        {2000, 432042},
        {2000, 358936},
        {2000, 335104},
        {2000, 311273},
        {2000, 287441},
        {2000, 263610},
        {2000, 239778},
        {2000, 215946},
        {2000, 192115},
        {2000, 168283},
        {2000, 144452},
        {2000, 120620},
        {2000, 112654},
        {2000, 104688},
        {2000, 96723},
        {2000, 88757},
        {2000, 80791},
        {2000, 72825},
        {2000, 64859},
        {2000, 56894},
        {2000, 48928},
        {2000, 40962},
        {2000, 37816},
        {2000, 34670},
        {2000, 31523},
        {2000, 28377},
        {2000, 25231},
        {2000, 22085},
        {2000, 18939},
        {2000, 15792},
        {2000, 12646},
        {2000, 9500}
    }},
    {0, 17500, { // Temporal layer index 4
        {2000, 3800000},
        {2000, 3696667},
        {2000, 3593333},
        {2000, 3490000},
        {2000, 3386667},
        {2000, 3283333},
        {2000, 3180000},
        {2000, 3076667},
        {2000, 2973333},
        {2000, 2870000},
        {2000, 2766667},
        {2000, 2663333},
        {2000, 2560000},
        {2000, 2456667},
        {2000, 2250000},
        {2000, 2095000},
        {2000, 1940000},
        {2000, 1785000},
        {2000, 1630000},
        {2000, 1475000},
        {2000, 1376000},
        {2000, 1277000},
        {2000, 1178000},
        {2000, 1079000},
        {2000, 980000},
        {2000, 906044},
        {2000, 832087},
        {2000, 758131},
        {2000, 684174},
        {2000, 610218},
        {2000, 536262},
        {2000, 462305},
        {2000, 388349},
        {2000, 314392},
        {2000, 240436},
        {2000, 222265},
        {2000, 204094},
        {2000, 185924},
        {2000, 167753},
        {2000, 149582},
        {2000, 131411},
        {2000, 113240},
        {2000, 95070},
        {2000, 76899},
        {2000, 58728},
        {2000, 54939},
        {2000, 51150},
        {2000, 47362},
        {2000, 43573},
        {2000, 39784},
        {2000, 35995},
        {2000, 32206},
        {2000, 28418},
        {2000, 24629},
        {2000, 20840},
        {2000, 18795},
        {2000, 16749},
        {2000, 14704},
        {2000, 12659},
        {2000, 10614},
        {2000, 8568},
        {2000, 6523},
        {2000, 4478},
        {2000, 4176}
    }},
    {2000, MAX_COMPLEXITY, {{0}}}
};

static EbRateControlComplexityModelDeviation COMPLEXITY_DEVIATION_INTRA[INTRA_DEVIATION_BRACKET_NUMBER] = {
    {0, 150, 1 << RC_DEVIATION_PRECISION, 0},
    {151, 300, 1 << RC_DEVIATION_PRECISION, 0},
    {301, 500, 1 << RC_DEVIATION_PRECISION, 0},
    {501, 750, 1 << RC_DEVIATION_PRECISION, 0},
    {751, 1000, 1 << RC_DEVIATION_PRECISION, 0},
    {1001, 2000, 1 << RC_DEVIATION_PRECISION, 0},
    {2001, 5000, 1 << RC_DEVIATION_PRECISION, 0},
    {5001, 10000, 1 << RC_DEVIATION_PRECISION, 0},
    {10001, 15000, 1 << RC_DEVIATION_PRECISION, 0},
    {3001, MAX_COMPLEXITY, 1 << RC_DEVIATION_PRECISION, 0}
};

static EbRateControlComplexityModelDeviation COMPLEXITY_DEVIATION_INTER[INTER_DEVIATION_BRACKET_NUMBER] = {
    {0, 1000, 1 << RC_DEVIATION_PRECISION, 0},
    {1001, 2500, 1 << RC_DEVIATION_PRECISION, 0},
    {2501, 5000, 1 << RC_DEVIATION_PRECISION, 0},
    {5001, 7500, 1 << RC_DEVIATION_PRECISION, 0},
    {7501, 10000, 1 << RC_DEVIATION_PRECISION, 0},
    {10001, 15000, 1 << RC_DEVIATION_PRECISION, 0},
    {15001, 25000, 1 << RC_DEVIATION_PRECISION, 0},
    {25001, MAX_COMPLEXITY, 1 << RC_DEVIATION_PRECISION, 0}
};

EbErrorType rate_control_model_ctor(EbRateControlModel **object_doubble_ptr) {
    EbRateControlModel *model_ptr;

    EB_MALLOC(EbRateControlModel *, model_ptr, sizeof(EbRateControlModel), EB_N_PTR);
    *object_doubble_ptr = (void *)model_ptr;

    EB_CREATEMUTEX(EbHandle, model_ptr->model_mutex, sizeof(EbHandle), EB_MUTEX);

    return EB_ErrorNone;
}

EbErrorType rate_control_model_init(EbRateControlModel *model_ptr, struct SequenceControlSet *sequence_control_set_ptr) {
    uint32_t number_of_frame = sequence_control_set_ptr->static_config.frames_to_be_encoded;
    EbRateControlGopInfo *gop_infos;

    EB_MALLOC(EbRateControlGopInfo *, gop_infos, sizeof(EbRateControlModel) * number_of_frame, EB_N_PTR);
    memset(gop_infos, 0, sizeof(EbRateControlModel) * number_of_frame);

    model_ptr->desired_bitrate = sequence_control_set_ptr->static_config.target_bit_rate;
    model_ptr->frame_rate = sequence_control_set_ptr->static_config.frame_rate >> 16;
    model_ptr->width = sequence_control_set_ptr->luma_width;
    model_ptr->height = sequence_control_set_ptr->luma_height;
    model_ptr->pixels = model_ptr->width * model_ptr->height;
    model_ptr->gop_infos = gop_infos;
    model_ptr->intra_period = (sequence_control_set_ptr->static_config.intra_period_length < 1) ? 1 : sequence_control_set_ptr->static_config.intra_period_length;
    model_ptr->number_of_frame = number_of_frame;

    EB_MALLOC(EbRateControlComplexityModelDeviation *, model_ptr->complexity_variation_intra_model, sizeof(EbRateControlComplexityModelDeviation) * INTRA_DEVIATION_BRACKET_NUMBER, EB_N_PTR);
    EB_MEMCPY(model_ptr->complexity_variation_intra_model, (void *)COMPLEXITY_DEVIATION_INTRA, sizeof(EbRateControlComplexityModelDeviation) * INTRA_DEVIATION_BRACKET_NUMBER);

    EB_MALLOC(EbRateControlComplexityModelDeviation *, model_ptr->complexity_variation_inter_model, sizeof(EbRateControlComplexityModelDeviation) * INTER_DEVIATION_BRACKET_NUMBER, EB_N_PTR);
    EB_MEMCPY(model_ptr->complexity_variation_inter_model, (void *)COMPLEXITY_DEVIATION_INTER, sizeof(EbRateControlComplexityModelDeviation) * INTER_DEVIATION_BRACKET_NUMBER);

    return EB_ErrorNone;
}

EbErrorType rate_control_report_complexity(EbRateControlModel *model_ptr, PictureParentControlSet_t *picture_control_set_ptr) {
    eb_block_on_mutex(model_ptr->model_mutex);

    model_ptr->gop_infos[picture_control_set_ptr->picture_number].complexity = picture_control_set_ptr->complexity;
    model_ptr->gop_infos[picture_control_set_ptr->picture_number].frames_in_sw = model_ptr->intra_period;
    model_ptr->gop_infos[picture_control_set_ptr->picture_number].temporal_layer_index = picture_control_set_ptr->temporal_layer_index;

    eb_release_mutex(model_ptr->model_mutex);

    return EB_ErrorNone;
}

EbErrorType rate_control_update_model(EbRateControlModel *model_ptr, PictureParentControlSet_t *picture_control_set_ptr) {
    uint64_t size = picture_control_set_ptr->total_num_bits;
    EbRateControlGopInfo *frame = &model_ptr->gop_infos[picture_control_set_ptr->picture_number];
    EbRateControlGopInfo *gop = get_gop_infos(model_ptr->gop_infos, picture_control_set_ptr->picture_number);

    eb_block_on_mutex(model_ptr->model_mutex);
    model_ptr->total_bytes += size;
    model_ptr->reported_frames++;
    gop->actual_size += size;
    gop->reported_frames++;

    frame->encoded = EB_TRUE;
    if (picture_control_set_ptr->av1FrameType != INTER_FRAME) {
        gop->intra_size = size;
    } else {
        frame->actual_size = size;
    }

    if (gop->reported_frames == gop->length) {
        size_t inter_size = ((gop->actual_size << RC_DEVIATION_PRECISION) - (gop->intra_size << RC_DEVIATION_PRECISION)) / (model_ptr->intra_period << RC_DEVIATION_PRECISION);
        int64_t intra_variation = 1;
        int64_t inter_variation = 1;

        intra_variation = (gop->expected_intra_size * gop->intra_deviation) / (gop->intra_size);
        inter_variation = (gop->expected_inter_size * gop->inter_deviation) / (inter_size);

        uint32_t complexity_inter = estimate_gop_complexity(model_ptr, gop);

        EbRateControlComplexityModelDeviation *model_deviation = EB_NULL;
        EbRateControlComplexityModelDeviation *model_inter_deviation = EB_NULL;

        for (unsigned int i = 0; model_ptr->complexity_variation_intra_model[i].scope_end != MAX_COMPLEXITY; i++) {
            model_deviation = &model_ptr->complexity_variation_intra_model[i];

            if (gop->complexity >= model_deviation->scope_start && gop->complexity <= model_deviation->scope_end)
                break ;
        }

        model_deviation->deviation = ((model_deviation->deviation * model_deviation->deviation_reported) + intra_variation) / (model_deviation->deviation_reported + 1);
        if (model_deviation->deviation_reported != MAX_COMPLEXITY_MODEL_DEVIATION_REPORTED)
            model_deviation->deviation_reported++;

        for (unsigned int i = 0; model_ptr->complexity_variation_inter_model[i].scope_end != MAX_COMPLEXITY; i++) {
            model_inter_deviation = &model_ptr->complexity_variation_inter_model[i];

            if (complexity_inter >= model_inter_deviation->scope_start && complexity_inter <= model_inter_deviation->scope_end)
                break ;
        }

        model_inter_deviation->deviation = ((model_inter_deviation->deviation * model_inter_deviation->deviation_reported) + inter_variation) / (model_inter_deviation->deviation_reported + 1);
        if (model_inter_deviation->deviation_reported != MAX_COMPLEXITY_MODEL_DEVIATION_REPORTED)
            model_inter_deviation->deviation_reported++;
    }

    eb_release_mutex(model_ptr->model_mutex);

    return EB_ErrorNone;
}

uint8_t rate_control_get_quantizer(EbRateControlModel *model_ptr, PictureParentControlSet_t *picture_control_set_ptr) {
    FRAME_TYPE type = picture_control_set_ptr->av1FrameType;

    if (type == INTRA_ONLY_FRAME || type == KEY_FRAME)
        record_new_gop(model_ptr, picture_control_set_ptr);

    EbRateControlGopInfo *gop = get_gop_infos(model_ptr->gop_infos, picture_control_set_ptr->picture_number);
    uint32_t base_qp = gop->qp;

    if (type == INTER_FRAME) {
        uint32_t inter_qp = CLIP3(0, MAX_QP_VALUE, (int64_t)base_qp + DELTA_LEVELS[picture_control_set_ptr->temporal_layer_index]);
        picture_control_set_ptr->best_pred_qp = inter_qp;
    } else
        picture_control_set_ptr->best_pred_qp = gop->qp;

    if (gop->reported_frames > AMOUNT_OF_REPORTED_FRAMES_TO_TRIGGER_ON_THE_FLY_QP &&
        picture_control_set_ptr->temporal_layer_index < MAX_INTER_LEVEL_FOR_ON_THE_FLY_QP) {
        int32_t delta_inter = 0;
        uint32_t desired_total_bytes = (model_ptr->desired_bitrate / model_ptr->frame_rate) * model_ptr->reported_frames;
        int64_t delta_bytes = desired_total_bytes - model_ptr->total_bytes;

        if (delta_bytes > 0)
            delta_inter = -3;
        else
            delta_inter = 3;

        if (delta_inter < 0 && picture_control_set_ptr->best_pred_qp < abs(delta_inter))
            delta_inter = -picture_control_set_ptr->best_pred_qp;

        picture_control_set_ptr->best_pred_qp += delta_inter;
    }

    return picture_control_set_ptr->best_pred_qp;
}

static void record_new_gop(EbRateControlModel *model_ptr, PictureParentControlSet_t *picture_control_set_ptr) {
    uint64_t pictureNumber = picture_control_set_ptr->picture_number;
    EbRateControlGopInfo *gop = &model_ptr->gop_infos[pictureNumber];

    eb_block_on_mutex(model_ptr->model_mutex);

    gop->index = pictureNumber;
    gop->exists = EB_TRUE;
    gop->desired_size = get_gop_size_in_bytes(model_ptr);
    gop->qp = 63;

    uint32_t complexity_inter = estimate_gop_complexity(model_ptr, gop);

    EbRateControlComplexityModel *model = EB_NULL;
    EbRateControlComplexityModel *previous_model = EB_NULL;
    EbRateControlComplexityModelDeviation *model_deviation = EB_NULL;
    EbRateControlComplexityModelDeviation *model_inter_deviation = EB_NULL;

    for (unsigned int i = 0; DEFAULT_INTRA_COMPLEXITY_MODEL[i].scope_end != MAX_COMPLEXITY; i++) {
        model = &DEFAULT_INTRA_COMPLEXITY_MODEL[i];

        if (gop->complexity >= model->scope_start && gop->complexity <= model->scope_end)
            break ;
        previous_model = model;
    }

    for (unsigned int i = 0; model_ptr->complexity_variation_intra_model[i].scope_end != MAX_COMPLEXITY; i++) {
        model_deviation = &model_ptr->complexity_variation_intra_model[i];

        if (gop->complexity >= model_deviation->scope_start && gop->complexity <= model_deviation->scope_end)
            break ;
    }

    for (unsigned int i = 0; model_ptr->complexity_variation_inter_model[i].scope_end != MAX_COMPLEXITY; i++) {
        model_inter_deviation = &model_ptr->complexity_variation_inter_model[i];

        if (complexity_inter >= model_inter_deviation->scope_start && complexity_inter <= model_inter_deviation->scope_end)
            break ;
    }

    uint32_t desired_total_bytes = (model_ptr->desired_bitrate / model_ptr->frame_rate) * model_ptr->reported_frames;
    int64_t delta_bytes = desired_total_bytes - model_ptr->total_bytes;
    uint32_t damping_factor = DAMPING_FACTOR;

    if (gop->index > 180)
        damping_factor *= 2;

    int64_t extra = delta_bytes / damping_factor;

    if (extra < 0 && gop->desired_size < (uint64_t)-extra)
        gop->desired_size /= MAX_DOWNSIZE_FACTOR;
    else
        gop->desired_size += extra;

    size_t size = ((gop->desired_size << RC_DEVIATION_PRECISION) / model_ptr->pixels * MODEL_DEFAULT_PIXEL_AREA) >> RC_DEVIATION_PRECISION;

    for (uint32_t qp = 0; qp <= MAX_QP_VALUE; qp++) {
        EbRateControlComplexityQpMinMax *sizes = &model->size[qp];
        int64_t pitch = (sizes->max - sizes->min) / (model->scope_end - model->scope_start);

        if (model->scope_end == MAX_COMPLEXITY) {
            EbRateControlComplexityQpMinMax *previous_sizes = &previous_model->size[qp];
            pitch = (previous_sizes->max - previous_sizes->min) / (previous_model->scope_end - previous_model->scope_start);
        }

        size_t complexity_size_inter = compute_inter_size(model_ptr, gop, qp);
        size_t complexity_size_intra = sizes->min + (pitch * (gop->complexity - model->scope_start));
        size_t tmp_intra = ((complexity_size_intra << RC_DEVIATION_PRECISION) / model_deviation->deviation);
        size_t tmp_inter = ((complexity_size_inter << RC_DEVIATION_PRECISION) / model_inter_deviation->deviation);
        size_t projected_gop_size = tmp_intra + tmp_inter;

        if (size > projected_gop_size || qp == MAX_QP_VALUE) {
            gop->qp = qp;
            gop->intra_deviation = model_deviation->deviation;
            gop->inter_deviation = model_inter_deviation->deviation;
            gop->expected_intra_size = ((tmp_intra << RC_DEVIATION_PRECISION) / MODEL_DEFAULT_PIXEL_AREA * model_ptr->pixels) >> RC_DEVIATION_PRECISION;
            gop->expected_inter_size = ((tmp_inter << RC_DEVIATION_PRECISION) / model_ptr->intra_period / MODEL_DEFAULT_PIXEL_AREA * model_ptr->pixels) >> RC_DEVIATION_PRECISION;
            break;
        }
    }

    // Update length in gopinfos
    if (pictureNumber != 0) {
        EbRateControlGopInfo *previousGop = get_gop_infos(model_ptr->gop_infos, pictureNumber - 1);

        previousGop->length = gop->index - previousGop->index;
    }

    eb_release_mutex(model_ptr->model_mutex);
}

static uint64_t compute_inter_size(EbRateControlModel *model_ptr, EbRateControlGopInfo *gop_ptr, uint64_t base_qp) {
    EbRateControlGopInfo    *current;
    int64_t                 position = 1;
    uint64_t                total_size = 0;
    EbRateControlComplexityModel *model_inter = EB_NULL;
    EbRateControlComplexityQpMinMax *sizes_inter = EB_NULL;
    uint32_t complexity_inter = estimate_gop_complexity(model_ptr, gop_ptr);

    while ((position + gop_ptr->index) < model_ptr->number_of_frame &&
           position <= (int64_t)model_ptr->intra_period &&
           position <= gop_ptr->frames_in_sw) {
        current = &(model_ptr->gop_infos[gop_ptr->index + position]);
        uint32_t inter_qp = CLIP3(0, MAX_QP_VALUE, (int64_t)base_qp + DELTA_LEVELS[current->temporal_layer_index]);
        model_inter = &DEFAULT_INTER_COMPLEXITY_MODEL[current->temporal_layer_index];
        sizes_inter = &model_inter->size[inter_qp];

        int64_t                 pitch_inter = (sizes_inter->max - sizes_inter->min) / (model_inter->scope_end - model_inter->scope_start);

        current->qp = inter_qp;

        uint64_t inter_size = (sizes_inter->min + (pitch_inter * (complexity_inter - model_inter->scope_start)));
        current->desired_size = ((inter_size << RC_DEVIATION_PRECISION) / MODEL_DEFAULT_PIXEL_AREA * model_ptr->pixels) >> RC_DEVIATION_PRECISION;

        total_size += inter_size;
        position++;
    }

    while ((position + gop_ptr->index) < model_ptr->number_of_frame &&
           position <= (int64_t)model_ptr->intra_period) {
        current = &(model_ptr->gop_infos[gop_ptr->index + position]);
        current->desired_size = total_size / gop_ptr->frames_in_sw;
        position++;
    }

    return total_size;
}

size_t get_gop_size_in_bytes(EbRateControlModel *model_ptr) {
    return (model_ptr->desired_bitrate << RC_DEVIATION_PRECISION) /
    ((model_ptr->frame_rate << RC_DEVIATION_PRECISION) /
    model_ptr->intra_period);
}

EbRateControlGopInfo *get_gop_infos(EbRateControlGopInfo *gop_info,
                                    uint64_t position) {
    EbRateControlGopInfo    *current;

    while (1) { // First frame is always guaranteed to exist
        current = &gop_info[position];

        if (current->exists)
            return current;

        if (position == 0)
            return EB_NULL;
        position--;
    }

    return EB_NULL;
}

uint32_t estimate_gop_complexity(EbRateControlModel *model_ptr,
                                 EbRateControlGopInfo *gop_ptr) {
    uint32_t                complexity = 0;
    EbRateControlGopInfo    *current;
    int64_t                 position = 1;
    uint32_t                reported_complexity = 0;

    while ((position + gop_ptr->index) < model_ptr->number_of_frame &&
           position < (int64_t)model_ptr->intra_period) {
        current = &(model_ptr->gop_infos[gop_ptr->index + position]);

        if (current->complexity) {
            complexity += current->complexity;
            reported_complexity++;
        }

        position++;
    }

    if (reported_complexity)
        complexity = complexity / reported_complexity;

    return complexity;
}
