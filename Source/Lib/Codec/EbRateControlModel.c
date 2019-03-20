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
 * @param {EbRateControlComplexityModel*} model_inter. Model corresponding to average inter frame complexity.
 * @parem {uint32_t} complexity_inter.
 * @parem {uint64_t} base_qp. Intra QP.
 * @return {uint64_t}. Predicted size in bits of the inter frames in the GOP
 */
static uint64_t compute_inter_size(EbRateControlModel *model_ptr, EbRateControlGopInfo *gop_ptr, EbRateControlComplexityModel *model_inter, uint32_t complexity_inter, uint64_t base_qp);

static uint64_t scale_size_up(uint64_t area_in_pixels, uint64_t size);
static uint64_t scale_size_back(uint64_t area_in_pixels, uint64_t size);

/*
 * @variable uint8_t[7]. Delta QP to apply to inter frames from intra frames
 * depending on temporal layer level
 */
static uint8_t DELTA_LEVELS[7] = {3, 5, 7, 8, 9, 10, 11};

static EbRateControlComplexityModel DEFAULT_INTRA_COMPLEXITY_MODEL[] = {
    {0, 2000, {
        {9896720, 11102240},
        {8276607, 9478995},
        {6656494, 7855750},
        {5036382, 6232506},
        {3416269, 4609261},
        {1796156, 2986016},
        {1526012, 2637186},
        {1255869, 2288355},
        {985725, 1939525},
        {715582, 1590694},
        {445438, 1241864},
        {412530, 1153470},
        {379621, 1065077},
        {346713, 976683},
        {313804, 888290},
        {280896, 799896},
        {273643, 764634},
        {266390, 729371},
        {259136, 694109},
        {251883, 658846},
        {244630, 623584},
        {237377, 588322},
        {230124, 553059},
        {222870, 517797},
        {215617, 482534},
        {208364, 447272},
        {197278, 427400},
        {186191, 407528},
        {175105, 387656},
        {164018, 367784},
        {152932, 347912},
        {141846, 328040},
        {130759, 308168},
        {119673, 288296},
        {108586, 268424},
        {97500, 248552},
        {94225, 236857},
        {90950, 225162},
        {87674, 213466},
        {84399, 201771},
        {81124, 190076},
        {77849, 178381},
        {74574, 166686},
        {71298, 154990},
        {68023, 143295},
        {64748, 131600},
        {62009, 125071},
        {59270, 118542},
        {56530, 112014},
        {53791, 105485},
        {51052, 98956},
        {48313, 92427},
        {45574, 85898},
        {42834, 79370},
        {40095, 72841},
        {37356, 66312},
        {35383, 61713},
        {33409, 57114},
        {31436, 52515},
        {29463, 47916},
        {27489, 43317},
        {25516, 38718},
        {23543, 34119},
        {21570, 29520}
        }},
    {2001, 6000, {
        {11102240, 12824512},
        {9478995, 11126184},
        {7855750, 9427856},
        {6232506, 7729528},
        {4609261, 6031200},
        {2986016, 4332872},
        {2637186, 4106298},
        {2288355, 3879723},
        {1939525, 3653149},
        {1590694, 3426574},
        {1241864, 3200000},
        {1153470, 2971816},
        {1065077, 2743632},
        {976683, 2515448},
        {888290, 2287264},
        {799896, 2059080},
        {764634, 1989549},
        {729371, 1920018},
        {694109, 1850486},
        {658846, 1780955},
        {623584, 1711424},
        {588322, 1641893},
        {553059, 1572362},
        {517797, 1502830},
        {482534, 1433299},
        {447272, 1363768},
        {427400, 1300230},
        {407528, 1236691},
        {387656, 1173153},
        {367784, 1109614},
        {347912, 1046076},
        {328040, 982538},
        {308168, 918999},
        {288296, 855461},
        {268424, 791922},
        {248552, 728384},
        {236857, 687447},
        {225162, 646510},
        {213466, 605574},
        {201771, 564637},
        {190076, 523700},
        {178381, 482763},
        {166686, 441826},
        {154990, 400890},
        {143295, 359953},
        {131600, 319016},
        {125071, 299466},
        {118542, 279915},
        {112014, 260365},
        {105485, 240814},
        {98956, 221264},
        {92427, 201714},
        {85898, 182163},
        {79370, 162613},
        {72841, 143062},
        {66312, 123512},
        {61713, 113436},
        {57114, 103360},
        {52515, 93284},
        {47916, 83208},
        {43317, 73132},
        {38718, 63056},
        {34119, 52980},
        {29520, 42904}
        }},
    {6001, 14000, {
        {12824512, 17601088},
        {11126184, 15799776},
        {9427856, 13998464},
        {7729528, 12197152},
        {6031200, 10395840},
        {4332872, 8594528},
        {4106298, 7972053},
        {3879723, 7349578},
        {3653149, 6727102},
        {3426574, 6104627},
        {3200000, 5482152},
        {2971816, 5145557},
        {2743632, 4808962},
        {2515448, 4472366},
        {2287264, 4135771},
        {2059080, 3799176},
        {1989549, 3614424},
        {1920018, 3429672},
        {1850486, 3244920},
        {1780955, 3060168},
        {1711424, 2875416},
        {1641893, 2690664},
        {1572362, 2505912},
        {1502830, 2321160},
        {1433299, 2136408},
        {1363768, 1951656},
        {1300230, 1872235},
        {1236691, 1792814},
        {1173153, 1713394},
        {1109614, 1633973},
        {1046076, 1554552},
        {982538, 1475131},
        {918999, 1395710},
        {855461, 1316290},
        {791922, 1236869},
        {728384, 1157448},
        {687447, 1101797},
        {646510, 1046146},
        {605574, 990494},
        {564637, 934843},
        {523700, 879192},
        {482763, 823541},
        {441826, 767890},
        {400890, 712238},
        {359953, 656587},
        {319016, 600936},
        {299466, 568455},
        {279915, 535974},
        {260365, 503494},
        {240814, 471013},
        {221264, 438532},
        {201714, 406051},
        {182163, 373570},
        {162613, 341090},
        {143062, 308609},
        {123512, 276128},
        {113436, 254060},
        {103360, 231992},
        {93284, 209924},
        {83208, 187856},
        {73132, 165788},
        {63056, 143720},
        {52980, 121652},
        {42904, 99584}
        }},
    {14001, MAX_COMPLEXITY, {{0}}}
    };

static EbRateControlComplexityModel DEFAULT_INTER_COMPLEXITY_MODEL[] = {
    {0, 215, {
        {538139, 2690695},
        {435110, 2175552},
        {332082, 1660409},
        {229053, 1145265},
        {126024, 630122},
        {22996, 114979},
        {19413, 97064},
        {15830, 79149},
        {12247, 61234},
        {8664, 43319},
        {5081, 25404},
        {4721, 23607},
        {4362, 21811},
        {4003, 20014},
        {3644, 18218},
        {3284, 16421},
        {3122, 15608},
        {2959, 14796},
        {2797, 13983},
        {2634, 13170},
        {2472, 12358},
        {2309, 11545},
        {2146, 10732},
        {1984, 9919},
        {1821, 9107},
        {1659, 8294},
        {1593, 7964},
        {1527, 7634},
        {1461, 7304},
        {1395, 6974},
        {1329, 6644},
        {1263, 6314},
        {1197, 5984},
        {1131, 5654},
        {1065, 5324},
        {999, 4994},
        {964, 4822},
        {930, 4650},
        {896, 4478},
        {861, 4306},
        {827, 4135},
        {793, 3963},
        {758, 3791},
        {724, 3619},
        {689, 3447},
        {655, 3275},
        {628, 3142},
        {602, 3009},
        {575, 2876},
        {549, 2743},
        {522, 2610},
        {495, 2477},
        {469, 2344},
        {442, 2211},
        {416, 2078},
        {389, 1945},
        {368, 1839},
        {347, 1733},
        {325, 1627},
        {304, 1521},
        {283, 1415},
        {262, 1309},
        {241, 1203},
        {219, 1097}
    }},
    {216, 310, {
        {2690695, 7731821},
        {2175552, 6283058},
        {1660409, 4834295},
        {1145265, 3385532},
        {630122, 1936770},
        {114979, 488007},
        {97064, 409568},
        {79149, 331130},
        {61234, 252691},
        {43319, 174252},
        {25404, 95814},
        {23607, 84638},
        {21811, 73463},
        {20014, 62287},
        {18218, 51111},
        {16421, 39936},
        {15608, 37678},
        {14796, 35421},
        {13983, 33163},
        {13170, 30906},
        {12358, 28648},
        {11545, 26391},
        {10732, 24133},
        {9919, 21876},
        {9107, 19618},
        {8294, 17361},
        {7964, 16461},
        {7634, 15562},
        {7304, 14662},
        {6974, 13763},
        {6644, 12863},
        {6314, 11964},
        {5984, 11064},
        {5654, 10165},
        {5324, 9265},
        {4994, 8366},
        {4822, 7969},
        {4650, 7571},
        {4478, 7174},
        {4306, 6777},
        {4135, 6379},
        {3963, 5982},
        {3791, 5585},
        {3619, 5187},
        {3447, 4790},
        {3275, 4393},
        {3142, 4276},
        {3009, 4160},
        {2876, 4044},
        {2743, 3928},
        {2610, 3812},
        {2477, 3696},
        {2344, 3580},
        {2211, 3464},
        {2078, 3348},
        {1945, 3232},
        {1839, 3110},
        {1733, 2987},
        {1627, 2865},
        {1521, 2742},
        {1415, 2620},
        {1309, 2498},
        {1203, 2375},
        {1097, 2253}
    }},
    {311, 1669, {
        {7731821, 6069916},
        {6283058, 5189098},
        {4834295, 4308280},
        {3385532, 3427461},
        {1936770, 2546643},
        {488007, 1665825},
        {409568, 1430539},
        {331130, 1195253},
        {252691, 959967},
        {174252, 724681},
        {95814, 489395},
        {84638, 449688},
        {73463, 409981},
        {62287, 370275},
        {51111, 330568},
        {39936, 290861},
        {37678, 276177},
        {35421, 261493},
        {33163, 246810},
        {30906, 232126},
        {28648, 217442},
        {26391, 202758},
        {24133, 188074},
        {21876, 173391},
        {19618, 158707},
        {17361, 144023},
        {16461, 136104},
        {15562, 128185},
        {14662, 120265},
        {13763, 112346},
        {12863, 104427},
        {11964, 96508},
        {11064, 88589},
        {10165, 80669},
        {9265, 72750},
        {8366, 64831},
        {7969, 60970},
        {7571, 57109},
        {7174, 53247},
        {6777, 49386},
        {6379, 45525},
        {5982, 41664},
        {5585, 37803},
        {5187, 33941},
        {4790, 30080},
        {4393, 26219},
        {4276, 24677},
        {4160, 23135},
        {4044, 21594},
        {3928, 20052},
        {3812, 18510},
        {3696, 16968},
        {3580, 15426},
        {3464, 13885},
        {3348, 12343},
        {3232, 10801},
        {3110, 9950},
        {2987, 9098},
        {2865, 8247},
        {2742, 7395},
        {2620, 6544},
        {2498, 5692},
        {2375, 4841},
        {2253, 3989}
    }},
    {1670, 2400, {
        {6069916, 12960889},
        {5189098, 11840371},
        {4308280, 10719853},
        {3427461, 9599335},
        {2546643, 8478817},
        {1665825, 7358299},
        {1430539, 6478919},
        {1195253, 5599539},
        {959967, 4720160},
        {724681, 3840780},
        {489395, 2961401},
        {449688, 2680052},
        {409981, 2398703},
        {370275, 2117354},
        {330568, 1836005},
        {290861, 1554656},
        {276177, 1424038},
        {261493, 1293420},
        {246810, 1162802},
        {232126, 1032185},
        {217442, 901567},
        {202758, 770949},
        {188074, 640332},
        {173391, 509714},
        {158707, 379096},
        {144023, 248479},
        {136104, 229750},
        {128185, 211020},
        {120265, 192291},
        {112346, 173562},
        {104427, 154833},
        {96508, 136103},
        {88589, 117374},
        {80669, 98645},
        {72750, 79916},
        {61187, 64831},
        {57483, 60970},
        {53780, 57109},
        {50076, 53247},
        {46373, 49386},
        {42669, 45525},
        {38966, 41664},
        {35262, 37803},
        {31559, 33941},
        {27855, 30080},
        {24152, 26219},
        {22881, 24677},
        {21611, 23135},
        {20340, 21594},
        {19070, 20052},
        {17800, 18510},
        {16529, 16968},
        {15259, 15426},
        {13885, 13989},
        {12343, 12718},
        {10801, 11448},
        {9950, 10474},
        {9098, 9500},
        {8247, 8525},
        {7395, 7551},
        {6544, 6577},
        {5603, 5692},
        {4628, 4841},
        {3654, 3989}
    }},
    {2401, 14000, {
        {12960889, 15223996},
        {11840371, 13834390},
        {10719853, 12444785},
        {9599335, 11055179},
        {8478817, 9665574},
        {7358299, 8275968},
        {6478919, 7428389},
        {5599539, 6580810},
        {4720160, 5733232},
        {3840780, 4885653},
        {2961401, 4038074},
        {2680052, 3792426},
        {2398703, 3546777},
        {2117354, 3301129},
        {1836005, 3055480},
        {1554656, 2809832},
        {1424038, 2696211},
        {1293420, 2582589},
        {1162802, 2468968},
        {1032185, 2355347},
        {901567, 2241726},
        {770949, 2128104},
        {640332, 2014483},
        {509714, 1900862},
        {379096, 1787240},
        {248479, 1673619},
        {229750, 1591362},
        {211020, 1509105},
        {192291, 1426848},
        {173562, 1344591},
        {154833, 1262334},
        {136103, 1180076},
        {117374, 1097819},
        {98645, 1015562},
        {79916, 933305},
        {61187, 851048},
        {57483, 800449},
        {53780, 749849},
        {50076, 699250},
        {46373, 648650},
        {42669, 598051},
        {38966, 547452},
        {35262, 496852},
        {31559, 446253},
        {27855, 395653},
        {24152, 345054},
        {22881, 322499},
        {21611, 299943},
        {20340, 277388},
        {19070, 254832},
        {17800, 232277},
        {16529, 209721},
        {15259, 187166},
        {13989, 164610},
        {12718, 142055},
        {11448, 119499},
        {10474, 108925},
        {9500, 98352},
        {8525, 87778},
        {7551, 77204},
        {6577, 66630},
        {5603, 56057},
        {4628, 45483},
        {3654, 34909}
    }},
    {14001, MAX_COMPLEXITY, {{0}}}
};

static EbRateControlComplexityModelDeviation COMPLEXITY_DEVIATION_INTRA[] = {
    {0, 250, 1 << RC_DEVIATION_PRECISION, 5},
    {251, 500, 1 << RC_DEVIATION_PRECISION, 5},
    {501, 1000, 1 << RC_DEVIATION_PRECISION, 5},
    {1001, 2000, 1 << RC_DEVIATION_PRECISION, 5},
    {2001, 3000, 1 << RC_DEVIATION_PRECISION, 5},
    {3001, 4000, 1 << RC_DEVIATION_PRECISION, 5},
    {4001, 5000, 1 << RC_DEVIATION_PRECISION, 5},
    {5001, 6000, 1 << RC_DEVIATION_PRECISION, 5},
    {6001, 7000, 1 << RC_DEVIATION_PRECISION, 5},
    {7001, 8000, 1 << RC_DEVIATION_PRECISION, 5},
    {8001, 9000, 1 << RC_DEVIATION_PRECISION, 5},
    {9001, 10000, 1 << RC_DEVIATION_PRECISION, 5},
    {10001, 11000, 1 << RC_DEVIATION_PRECISION, 5},
    {11001, 12000, 1 << RC_DEVIATION_PRECISION, 5},
    {12001, 13000, 1 << RC_DEVIATION_PRECISION, 5},
    {13001, 14000, 1 << RC_DEVIATION_PRECISION, 5},
    {14001, 17000, 1 << RC_DEVIATION_PRECISION, 5},
    {17001, 21000, 1 << RC_DEVIATION_PRECISION, 5},
    {21001, 26000, 1 << RC_DEVIATION_PRECISION, 5},
    {26001, MAX_COMPLEXITY, 1 << RC_DEVIATION_PRECISION, 5}
};

static EbRateControlComplexityModelDeviation COMPLEXITY_DEVIATION_INTER[] = {
    {0, 100, 1, 0},
    {101, 200, 1, 0},
    {201, 250, 1, 0},
    {251, 300, 1, 0},
    {301, 350, 1, 0},
    {351, 400, 1, 0},
    {401, 600, 1, 0},
    {601, 800, 1, 0},
    {801, 1000, 1, 0},
    {1001, 1250, 1, 0},
    {1251, 1500, 1, 0},
    {1501, 2000, 1, 0},
    {2001, 2500, 1, 0},
    {2501, 3000, 1, 0},
    {3001, 3500, 1, 0},
    {3501, MAX_COMPLEXITY, 1, 0}
};

EbErrorType rate_control_model_ctor(EbRateControlModel **object_doubble_ptr) {
    EbRateControlModel *model_ptr;

    EB_MALLOC(EbRateControlModel *, model_ptr, sizeof(EbRateControlModel), EB_N_PTR);
    *object_doubble_ptr = (void *)model_ptr;

    EB_CREATEMUTEX(EbHandle, model_ptr->model_mutex, sizeof(EbHandle), EB_MUTEX);

    return EB_ErrorNone;
}

EbErrorType rate_control_model_init(EbRateControlModel *model_ptr, SequenceControlSet_t *sequence_control_set_ptr) {
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
    model_ptr->intra_default_variation = 1 << RC_DEVIATION_PRECISION;
    model_ptr->inter_default_variation = 1 << RC_DEVIATION_PRECISION;
    model_ptr->intra_default_variation_reported = 0;
    model_ptr->inter_default_variation_reported = 0;

    EB_MALLOC(EbRateControlComplexityModelDeviation *, model_ptr->complexity_variation_intra_model, sizeof(EbRateControlComplexityModelDeviation) * 20, EB_N_PTR);
    EB_MEMCPY(model_ptr->complexity_variation_intra_model, (void *)COMPLEXITY_DEVIATION_INTRA, sizeof(EbRateControlComplexityModelDeviation) * 20);

    EB_MALLOC(EbRateControlComplexityModelDeviation *, model_ptr->complexity_variation_inter_model, sizeof(EbRateControlComplexityModelDeviation) * 16, EB_N_PTR);
    EB_MEMCPY(model_ptr->complexity_variation_inter_model, (void *)COMPLEXITY_DEVIATION_INTER, sizeof(EbRateControlComplexityModelDeviation) * 16);

    printf("\n{\"type\": \"rateControlInit\", \"bitrate\": \"%d\", \"frameRate\": \"%d\", \"width\": \"%d\", \"height\": \"%d\", \"intraPeriod\": \"%d\"}\n",
        model_ptr->desired_bitrate,
        model_ptr->frame_rate,
        model_ptr->width,
        model_ptr->height,
        model_ptr->intra_period);

    return EB_ErrorNone;
}

EbErrorType rate_control_report_complexity(EbRateControlModel *model_ptr, PictureParentControlSet_t *picture_control_set_ptr) {
    eb_block_on_mutex(model_ptr->model_mutex);

    model_ptr->gop_infos[picture_control_set_ptr->picture_number].complexity = picture_control_set_ptr->complexity;
    model_ptr->gop_infos[picture_control_set_ptr->picture_number].picture_control_set_ptr = picture_control_set_ptr;

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
    if (picture_control_set_ptr->av1FrameType != INTER_FRAME)
        gop->intra_size = size;
    else
        frame->actual_size = size;

    uint32_t desired_total_bytes = (model_ptr->desired_bitrate / model_ptr->frame_rate) * model_ptr->reported_frames;
    int64_t delta_bytes = desired_total_bytes - model_ptr->total_bytes;
    printf("\n{\"type\": \"rateControlFrameCompleted\", \"frameType\": \"%s\", \"pictureNumber\": \"%d\", \"size\": \"%ld\", \"qp\": \"%d\", \"extra\": \"%ld\", \"actualSize\": \"%ld\", \"complexity\": \"%ld\", \"desiredSize\": \"%ld\", \"deviation\": \"%f\"}\n",
        (picture_control_set_ptr->av1FrameType == INTER_FRAME) ? "inter" : "intra",
        picture_control_set_ptr->picture_number,
        size,
        picture_control_set_ptr->picture_qp,
        delta_bytes,
        size,
        picture_control_set_ptr->complexity,
        (picture_control_set_ptr->av1FrameType == INTER_FRAME) ? gop->expected_inter_size : gop->expected_intra_size,
        (picture_control_set_ptr->av1FrameType == INTER_FRAME) ? (float)gop->expected_inter_size / (float)size : (float)gop->expected_intra_size / (float)size);

    if (gop->reported_frames == gop->length) {
        size_t inter_size = ((gop->actual_size << RC_DEVIATION_PRECISION) - (gop->intra_size << RC_DEVIATION_PRECISION)) / (model_ptr->intra_period << RC_DEVIATION_PRECISION);
        int64_t intra_variation = 1;
        int64_t inter_variation = 1;

        intra_variation = (gop->expected_intra_size << RC_DEVIATION_PRECISION) / (gop->intra_size);
        inter_variation = (gop->expected_inter_size << RC_DEVIATION_PRECISION) / (inter_size);

        printf("\n{\"type\": \"rateControlGopCompleted\", \"gopNumber\": \"%ld\", \"intraVariation\": \"%ld\", \"interVariation\": \"%ld\", \"intraComplexity\": \"%ld\", \"interComplexity\": \"%ld\"}\n",
            gop->index / model_ptr->intra_period,
            intra_variation,
            inter_variation,
            gop->complexity,
            estimate_gop_complexity(model_ptr, gop));

        model_ptr->intra_default_variation = ((model_ptr->intra_default_variation * model_ptr->intra_default_variation_reported) + intra_variation) / (model_ptr->intra_default_variation_reported + 1);
        if (model_ptr->intra_default_variation_reported != MAX_COMPLEXITY_MODEL_DEVIATION_REPORTED)
            model_ptr->intra_default_variation_reported++;
        model_ptr->inter_default_variation = ((model_ptr->inter_default_variation * model_ptr->inter_default_variation_reported) + inter_variation) / (model_ptr->inter_default_variation_reported + 1);
        if (model_ptr->inter_default_variation_reported != MAX_COMPLEXITY_MODEL_DEVIATION_REPORTED)
            model_ptr->inter_default_variation_reported++;

        uint32_t complexity_inter = estimate_gop_complexity(model_ptr, gop);

        EbRateControlComplexityModelDeviation *model_deviation = EB_NULL;
        EbRateControlComplexityModelDeviation *model_inter_deviation = EB_NULL;

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

        model_deviation->deviation = ((model_deviation->deviation * model_deviation->deviation_reported) + intra_variation) / (model_deviation->deviation_reported + 1);
        if (model_deviation->deviation_reported != MAX_COMPLEXITY_MODEL_DEVIATION_REPORTED) 
            model_deviation->deviation_reported++;

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
    } else {
        picture_control_set_ptr->best_pred_qp = gop->qp;
    }

    if (gop->reported_frames > AMOUNT_OF_REPORTED_FRAMES_TO_TRIGGER_ON_THE_FLY_QP &&
        picture_control_set_ptr->temporal_layer_index < MAX_INTER_LEVEL_FOR_ON_THE_FLY_QP) {
        int64_t deviation = 1;
        int32_t delta_inter = 0;
        size_t expected_size = 0;
        size_t actual_size = 0;

        for (unsigned int i = gop->index + 1; !model_ptr->gop_infos[i].exists; i++) {
            EbRateControlGopInfo *current = &model_ptr->gop_infos[i];

            if (current->encoded) {
                expected_size += current->desired_size;
                actual_size += current->actual_size;
            }
        }

        if (actual_size != 0) {
            deviation = ((expected_size + gop->expected_intra_size) * 10) / (actual_size + gop->intra_size);
            delta_inter = 10 - ABS(deviation);
        }

        delta_inter = CLIP3(-MAX_DELTA_QP_WHITIN_GOP, MAX_DELTA_QP_WHITIN_GOP, delta_inter);

        if (delta_inter < 0 && picture_control_set_ptr->best_pred_qp < abs(delta_inter)) {
            delta_inter = -picture_control_set_ptr->best_pred_qp;
        }

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
    EbRateControlComplexityModel *model_inter = EB_NULL;
    EbRateControlComplexityModelDeviation *model_deviation = EB_NULL;
    EbRateControlComplexityModelDeviation *model_inter_deviation = EB_NULL;

    for (unsigned int i = 0; DEFAULT_INTRA_COMPLEXITY_MODEL[i].scope_end != MAX_COMPLEXITY; i++) {
        model = &DEFAULT_INTRA_COMPLEXITY_MODEL[i];

        if (gop->complexity >= model->scope_start && gop->complexity <= model->scope_end)
            break ;
    }

    for (unsigned int i = 0; DEFAULT_INTER_COMPLEXITY_MODEL[i].scope_end != MAX_COMPLEXITY; i++) {
        model_inter = &DEFAULT_INTER_COMPLEXITY_MODEL[i];

        if (complexity_inter >= model_inter->scope_start && complexity_inter <= model_inter->scope_end)
            break ;
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

    if (model_deviation->deviation_reported == 0) 
        model_deviation->deviation = model_ptr->intra_default_variation;

    if (model_inter_deviation->deviation_reported == 0)
        model_inter_deviation->deviation = model_ptr->inter_default_variation;

    uint32_t desired_total_bytes = (model_ptr->desired_bitrate / model_ptr->frame_rate) * model_ptr->reported_frames;
    int64_t delta_bytes = desired_total_bytes - model_ptr->total_bytes;
    int64_t extra = delta_bytes / DAMPING_FACTOR;

    if (extra < 0 && gop->desired_size < (uint64_t)-extra)
        gop->desired_size /= MAX_DOWNSIZE_FACTOR;
    else
        gop->desired_size += extra;

    size_t size = scale_size_up(model_ptr->pixels, gop->desired_size);

    for (unsigned int qp = 0; qp <= MAX_QP_VALUE; qp++) {
        EbRateControlComplexityQpMinMax *sizes = &model->size[qp];
        int64_t pitch = (sizes->max - sizes->min) / (model->scope_end - model->scope_start);

        if (model->scope_end == MAX_COMPLEXITY)
            pitch = PITCH_ON_MAX_COMPLEXITY_FOR_INTRA_FRAMES;

        size_t complexity_size_inter = compute_inter_size(model_ptr, gop,
            model_inter,
            complexity_inter,
            qp);
        size_t complexity_size_intra = sizes->min + (pitch * (gop->complexity - model->scope_start));
        size_t tmp_intra = (complexity_size_intra / model_deviation->deviation) << RC_DEVIATION_PRECISION;
        size_t tmp_inter = (complexity_size_inter / model_inter_deviation->deviation) << RC_DEVIATION_PRECISION;
        size_t projected_gop_size = tmp_intra + tmp_inter;

        if (size > projected_gop_size || qp == MAX_QP_VALUE) {
            printf("\nFRAME INTRA size %ld. Raw %ld\n", complexity_size_intra, tmp_intra);
            gop->qp = qp;
            gop->picture_control_set_ptr->best_pred_qp = qp;
            gop->expected_intra_size = scale_size_back(model_ptr->pixels, complexity_size_intra);
            gop->expected_inter_size = scale_size_back(model_ptr->pixels, complexity_size_inter / model_ptr->intra_period);
            break;
        }
    }

    // Update length in gopinfos
    if (pictureNumber != 0) {
        EbRateControlGopInfo *previousGop = get_gop_infos(model_ptr->gop_infos, pictureNumber - 1);

        previousGop->length = gop->index - previousGop->index;
    }

    printf("\n{\"type\": \"rateControlGopCreated\", \"gopNumber\": \"%ld\", \"qp\": \"%ld\", \"expectedSize\": \"%ld\", \"expectedIntraSize\": \"%ld\", \"expectedInterSize\": \"%ld\"}\n",
        gop->index / model_ptr->intra_period,
        gop->qp,
        gop->desired_size,
        gop->expected_intra_size,
        gop->expected_inter_size,
        estimate_gop_complexity(model_ptr, gop));

    eb_release_mutex(model_ptr->model_mutex);
}

static uint64_t compute_inter_size(EbRateControlModel *model_ptr, EbRateControlGopInfo *gop_ptr, EbRateControlComplexityModel *model_inter, uint32_t complexity_inter, uint64_t base_qp) {
    EbRateControlGopInfo    *current;
    int64_t                 position = 1;
    uint64_t                total_size = 0;
    EbRateControlComplexityQpMinMax *sizes_inter = EB_NULL;

    while ((position + gop_ptr->index) < model_ptr->number_of_frame &&
           position <= (int64_t)model_ptr->intra_period && 
           position <= gop_ptr->picture_control_set_ptr->frames_in_sw) {
        current = &(model_ptr->gop_infos[gop_ptr->index + position]);
        uint32_t inter_qp = CLIP3(0, MAX_QP_VALUE, (int64_t)base_qp + DELTA_LEVELS[current->picture_control_set_ptr->temporal_layer_index]);
        sizes_inter = &model_inter->size[inter_qp];

        int64_t                 pitch_inter = (sizes_inter->max - sizes_inter->min) / (model_inter->scope_end - model_inter->scope_start);

        current->picture_control_set_ptr->best_pred_qp = inter_qp;
        if (model_inter->scope_end == MAX_COMPLEXITY)
            pitch_inter = PITCH_ON_MAX_COMPLEXITY_FOR_INTER_FRAMES;

        int64_t alpha = 1;

        if (current->picture_control_set_ptr->complexity > 128 && current->picture_control_set_ptr->complexity < current->picture_control_set_ptr->pic_avg_variance) {
            alpha = (((current->picture_control_set_ptr->complexity * current->picture_control_set_ptr->complexity) + (current->picture_control_set_ptr->pic_avg_variance * current->picture_control_set_ptr->pic_avg_variance)) / (current->picture_control_set_ptr->complexity * current->picture_control_set_ptr->complexity)) / 2.0;
        }

        if (alpha > MAX_INTER_COMPLEXITY_DEVIATION) {
            alpha = MAX_INTER_COMPLEXITY_DEVIATION;
        }
        
        uint64_t inter_size = (sizes_inter->min + (pitch_inter * (complexity_inter - model_inter->scope_start))) * alpha;
        current->desired_size = ((inter_size << RC_DEVIATION_PRECISION) / MODEL_DEFAULT_PIXEL_AREA * model_ptr->pixels) >> RC_DEVIATION_PRECISION;

        total_size += inter_size;
        position++;
    }

    while ((position + gop_ptr->index) < model_ptr->number_of_frame &&
           position <= (int64_t)model_ptr->intra_period) {
        current = &(model_ptr->gop_infos[gop_ptr->index + position]);
        current->desired_size = total_size / gop_ptr->picture_control_set_ptr->frames_in_sw;
        position++;
    }

    return total_size;
}

size_t get_gop_size_in_bytes(EbRateControlModel *model_ptr) {
    return (model_ptr->desired_bitrate << RC_DEVIATION_PRECISION) / ((model_ptr->frame_rate << RC_DEVIATION_PRECISION) / model_ptr->intra_period);
}

EbRateControlGopInfo *get_gop_infos(EbRateControlGopInfo *gop_info,
                                    uint64_t position) {
    EbRateControlGopInfo    *current;

    while (1) { // First frame is always guaranteed to exist
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

static uint64_t scale_size_back(uint64_t area_in_pixels, uint64_t size) {
    double ratio = (double)area_in_pixels / (double)MODEL_DEFAULT_PIXEL_AREA;
    double proportion = sqrt(ratio);

    printf("\nSCALING BACK: Area in pixels: %ld. Ratio: %f. Proportion: %f. Result: %ld\n", area_in_pixels, ratio, proportion, (uint64_t)((double)size * proportion));

    return (uint64_t)((double)size * proportion);
}

static uint64_t scale_size_up(uint64_t area_in_pixels, uint64_t size) {
    double ratio = (double)MODEL_DEFAULT_PIXEL_AREA / (double)area_in_pixels;
    double proportion = sqrt(ratio);

    printf("\nSCALING UP: Area in pixels: %ld. Ratio: %f. Proportion: %f. Result: %ld\n", area_in_pixels, ratio, proportion, (uint64_t)((double)size * proportion));

    return (uint64_t)((double)size * proportion);
}
