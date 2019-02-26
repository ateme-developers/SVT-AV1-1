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
 * @function record_new_gop. Take into account a new group of picture in the
 * model
 * @param {EbRateControlModel*} model_ptr.
 * @param {PictureParentControlSet_t*} picture_ptr. Picture holding the intra frame.
 * @return {void}.
 */
static void record_new_gop(EbRateControlModel *model_ptr, PictureParentControlSet_t *picture_ptr);

static EbRateControlComplexityModel DEFAULT_INTRA_COMPLEXITY_MODEL[] = {
    {0, 100, {
        {89364, 8936371},
        {75106, 7510642},
        {60849, 6084913},
        {46592, 4659184},
        {32335, 3233455},
        {18077, 1807726},
        {15597, 1559687},
        {13116, 1311648},
        {10636, 1063609},
        {8156, 815570},
        {5675, 567531},
        {5238, 523796},
        {4801, 480061},
        {4363, 436326},
        {3926, 392591},
        {3489, 348856},
        {3350, 335003},
        {3212, 321150},
        {3073, 307297},
        {2934, 293444},
        {2796, 279591},
        {2657, 265737},
        {2519, 251884},
        {2380, 238031},
        {2242, 224178},
        {2103, 210325},
        {2010, 201037},
        {1917, 191749},
        {1825, 182461},
        {1732, 173173},
        {1639, 163885},
        {1546, 154597},
        {1453, 145309},
        {1360, 136021},
        {1267, 126733},
        {1174, 117445},
        {1117, 111729},
        {1060, 106013},
        {1003, 100297},
        {946, 94581},
        {889, 88866},
        {832, 83150},
        {774, 77434},
        {717, 71718},
        {660, 66002},
        {603, 60286},
        {570, 57043},
        {538, 53800},
        {506, 50556},
        {473, 47313},
        {441, 44070},
        {408, 40827},
        {376, 37584},
        {343, 34340},
        {311, 31097},
        {279, 27854},
        {260, 26045},
        {242, 24235},
        {224, 22426},
        {206, 20617},
        {188, 18807},
        {170, 16998},
        {152, 15188},
        {134, 13379}}},
    {101, 250, {
        {8936371, 10229520},
        {7510642, 8659096},
        {6084913, 7088672},
        {4659184, 5518248},
        {3233455, 3947824},
        {1807726, 2377400},
        {1559687, 2144240},
        {1311648, 1911080},
        {1063609, 1677920},
        {815570, 1444760},
        {567531, 1211600},
        {523796, 1128762},
        {480061, 1045923},
        {436326, 963085},
        {392591, 880246},
        {348856, 797408},
        {335003, 761554},
        {321150, 725699},
        {307297, 689845},
        {293444, 653990},
        {279591, 618136},
        {265737, 582282},
        {251884, 546427},
        {238031, 510573},
        {224178, 474718},
        {210325, 438864},
        {201037, 417165},
        {191749, 395466},
        {182461, 373766},
        {173173, 352067},
        {163885, 330368},
        {154597, 308669},
        {145309, 286970},
        {136021, 265270},
        {126733, 243571},
        {117445, 221872},
        {111729, 210240},
        {106013, 198608},
        {100297, 186976},
        {94581, 175344},
        {88866, 163712},
        {83150, 152080},
        {77434, 140448},
        {71718, 128816},
        {66002, 117184},
        {60286, 105552},
        {57043, 99946},
        {53800, 94341},
        {50556, 88735},
        {47313, 83130},
        {44070, 77524},
        {40827, 71918},
        {37584, 66313},
        {34340, 60707},
        {31097, 55102},
        {27854, 49496},
        {26045, 45833},
        {24235, 42170},
        {22426, 38507},
        {20617, 34844},
        {18807, 31181},
        {16998, 27518},
        {15188, 23855},
        {13379, 20192}}},
    {251, 1000, {
        {10229520, 16127460},
        {8659096, 14303210},
        {7088672, 12478961},
        {5518248, 10654711},
        {3947824, 8830462},
        {2377400, 7006212},
        {2144240, 6497035},
        {1911080, 5987858},
        {1677920, 5478682},
        {1444760, 4969505},
        {1211600, 4460328},
        {1128762, 4208098},
        {1045923, 3955867},
        {963085, 3703637},
        {880246, 3451406},
        {797408, 3199176},
        {761554, 3067304},
        {725699, 2935433},
        {689845, 2803561},
        {653990, 2671690},
        {618136, 2539818},
        {582282, 2407946},
        {546427, 2276075},
        {510573, 2144203},
        {474718, 2012332},
        {438864, 1880460},
        {417165, 1795642},
        {395466, 1710824},
        {373766, 1626007},
        {352067, 1541189},
        {330368, 1456371},
        {308669, 1371553},
        {286970, 1286735},
        {265270, 1201918},
        {243571, 1117100},
        {221872, 1032282},
        {210240, 978984},
        {198608, 925686},
        {186976, 872388},
        {175344, 819090},
        {163712, 765792},
        {152080, 712494},
        {140448, 659196},
        {128816, 605898},
        {117184, 552600},
        {105552, 499302},
        {99946, 469204},
        {94341, 439106},
        {88735, 409009},
        {83130, 378911},
        {77524, 348813},
        {71918, 318715},
        {66313, 288617},
        {60707, 258520},
        {55102, 228422},
        {49496, 198324},
        {45833, 181238},
        {42170, 164151},
        {38507, 147065},
        {34844, 129978},
        {31181, 112892},
        {27518, 95805},
        {23855, 78719},
        {20192, 61632}}},
    {1000, 3000, {
        {10229520, 18188196},
        {8659096, 16291650},
        {7088672, 14395103},
        {5518248, 12498557},
        {3947824, 10602010},
        {2377400, 8705464},
        {2144240, 8184339},
        {1911080, 7663214},
        {1677920, 7142090},
        {1444760, 6620965},
        {1211600, 6099840},
        {1128762, 5850609},
        {1045923, 5601378},
        {963085, 5352148},
        {880246, 5102917},
        {797408, 4853686},
        {761554, 4710096},
        {725699, 4566506},
        {689845, 4422915},
        {653990, 4279325},
        {618136, 4135735},
        {582282, 3992145},
        {546427, 3848555},
        {510573, 3704964},
        {474718, 3561374},
        {438864, 3417784},
        {417165, 3290444},
        {395466, 3163103},
        {373766, 3035763},
        {352067, 2908423},
        {330368, 2781083},
        {308669, 2653742},
        {286970, 2526402},
        {265270, 2399062},
        {243571, 2271721},
        {221872, 2144381},
        {210240, 2055937},
        {198608, 1967494},
        {186976, 1879050},
        {175344, 1790606},
        {163712, 1702163},
        {152080, 1613719},
        {140448, 1525275},
        {128816, 1436831},
        {117184, 1348388},
        {105552, 1259944},
        {99946, 1199913},
        {94341, 1139881},
        {88735, 1079850},
        {83130, 1019818},
        {77524, 959787},
        {71918, 899756},
        {66313, 839724},
        {60707, 779693},
        {55102, 719661},
        {49496, 659630},
        {45833, 601393},
        {42170, 543156},
        {38507, 484919},
        {34844, 426683},
        {31181, 368446},
        {27518, 310209},
        {23855, 251972},
        {20192, 193735}}},
    {3001, MAX_COMPLEXITY, {0}}
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
        {64831, 61187},
        {60970, 57483},
        {57109, 53780},
        {53247, 50076},
        {49386, 46373},
        {45525, 42669},
        {41664, 38966},
        {37803, 35262},
        {33941, 31559},
        {30080, 27855},
        {26219, 24152},
        {24677, 22881},
        {23135, 21611},
        {21594, 20340},
        {20052, 19070},
        {18510, 17800},
        {16968, 16529},
        {15426, 15259},
        {13885, 13989},
        {12343, 12718},
        {10801, 11448},
        {9950, 10474},
        {9098, 9500},
        {8247, 8525},
        {7395, 7551},
        {6544, 6577},
        {5692, 5603},
        {4841, 4628},
        {3989, 3654}
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
    {14001, MAX_COMPLEXITY, {0}}
};

static EbRateControlComplexityModelDeviation COMPLEXITY_DEVIATION_INTRA[] = {
    {0, 25, 1, 0},
    {26, 50, 1, 0},
    {51, 75, 1, 0},
    {76, 100, 1, 0},
    {101, 125, 1, 0},
    {126, 150, 1, 0},
    {151, 175, 1, 0},
    {176, 200, 1, 0},
    {201, 225, 1, 0},
    {226, 250, 1, 0},
    {251, 275, 1, 0},
    {276, 300, 1, 0},
    {301, 350, 1, 0},
    {351, 400, 1, 0},
    {401, 500, 1, 0},
    {501, 600, 1, 0},
    {601, 1000, 1, 0},
    {1001, 2000, 1, 0},
    {2001, 3000, 1, 0},
    {3001, MAX_COMPLEXITY, 1, 0}
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
    model_ptr->model_variation = 1.0;

    EB_MALLOC(EbRateControlComplexityModelDeviation *, model_ptr->complexity_variation_intra_model, sizeof(EbRateControlComplexityModelDeviation) * 20, EB_N_PTR);
    EB_MEMCPY(model_ptr->complexity_variation_intra_model, (void *)COMPLEXITY_DEVIATION_INTRA, sizeof(EbRateControlComplexityModelDeviation) * 20);

    EB_MALLOC(EbRateControlComplexityModelDeviation *, model_ptr->complexity_variation_inter_model, sizeof(EbRateControlComplexityModelDeviation) * 16, EB_N_PTR);
    EB_MEMCPY(model_ptr->complexity_variation_inter_model, (void *)COMPLEXITY_DEVIATION_INTER, sizeof(EbRateControlComplexityModelDeviation) * 16);

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

    if (picture_ptr->av1FrameType != INTER_FRAME) {
        gop->intra_size = size;
    }

    if (gop->reported_frames == gop->length) {
        size_t inter_size = ((float)gop->actual_size - (float)gop->intra_size) / (float)model_ptr->intra_period;
        float intra_variation = 1;
        float inter_variation = 1;

        intra_variation = (float)gop->expected_intra_size / (float)gop->intra_size;
        inter_variation = (float)gop->expected_inter_size / (float)inter_size;

        uint32_t complexity_inter = estimate_gop_complexity(model_ptr, gop);

        EbRateControlComplexityModelDeviation *model_deviation = EB_NULL;
        EbRateControlComplexityModelDeviation *model_inter_deviation = EB_NULL;

        for (unsigned int i = 0; model_ptr->complexity_variation_intra_model[i].scope_end != MAX_COMPLEXITY; i++) {
            model_deviation = &model_ptr->complexity_variation_intra_model[i];

            if (gop->complexity >= model_deviation->scope_start && gop->complexity <= model_deviation->scope_end) {
                break ;
            }
        }

        for (unsigned int i = 0; model_ptr->complexity_variation_inter_model[i].scope_end != MAX_COMPLEXITY; i++) {
            model_inter_deviation = &model_ptr->complexity_variation_inter_model[i];

            if (complexity_inter >= model_inter_deviation->scope_start && complexity_inter <= model_inter_deviation->scope_end) {
                break ;
            }
        }

        model_deviation->deviation = ((model_deviation->deviation * model_deviation->deviation_reported) + intra_variation) / (model_deviation->deviation_reported + 1);
        if (model_deviation->deviation_reported != MAX_COMPLEXITY_MODEL_DEVIATION_REPORTED) {
            model_deviation->deviation_reported++;
        }

        model_inter_deviation->deviation = ((model_inter_deviation->deviation * model_inter_deviation->deviation_reported) + inter_variation) / (model_inter_deviation->deviation_reported + 1);
        if (model_inter_deviation->deviation_reported != MAX_COMPLEXITY_MODEL_DEVIATION_REPORTED) {
            model_inter_deviation->deviation_reported++;
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
    uint32_t complexity_inter = estimate_gop_complexity(model_ptr, gop);

    if (model_ptr->reported_frames < MAX_AMOUNT_OF_FRAME_FOR_ON_THE_FLY_QP && gop->reported_frames > AMOUNT_OF_REPORTED_FRAMES_TO_TRIGGER_ON_THE_FLY_QP) {
        float deviation = 1;
        uint32_t average_inter_size = (gop->actual_size - gop->intra_size) / (gop->reported_frames - 1);

        deviation = (float)gop->expected_inter_size / (float)average_inter_size;

        EbRateControlComplexityModel *model_inter = EB_NULL;
        for (unsigned int i = 0; DEFAULT_INTER_COMPLEXITY_MODEL[i].scope_end != MAX_COMPLEXITY; i++) {
            model_inter = &DEFAULT_INTER_COMPLEXITY_MODEL[i];

            if (complexity_inter >= model_inter->scope_start && complexity_inter <= model_inter->scope_end) {
                break ;
            }
        }

        size_t size = (float)gop->expected_inter_size / (float)model_ptr->pixels * MODEL_DEFAULT_PIXEL_AREA;
        size = size * deviation;
        for (unsigned int new_qp = 0; new_qp <= MAX_QP_VALUE; new_qp++) {
            EbRateControlComplexityQpMinMax *sizes_inter = &model_inter->size[new_qp];
            int64_t pitch_inter = (sizes_inter->max - sizes_inter->min) / (model_inter->scope_end - model_inter->scope_start);

            if (model_inter->scope_end == MAX_COMPLEXITY) {
                pitch_inter = PITCH_ON_MAX_COMPLEXITY_FOR_INTER_FRAMES;
            }

            uint64_t complexity_size_inter = sizes_inter->min + (pitch_inter * (complexity_inter - model_inter->scope_start));
            if (size > complexity_size_inter || new_qp == MAX_QP_VALUE) {
                int64_t min = 0;
                int64_t max = MAX_QP_VALUE;
                if (gop->qp < MAX_DELTA_QP_WHITIN_GOP) {
                    min = 0;
                } else {
                    min = gop->qp - MAX_DELTA_QP_WHITIN_GOP;
                }

                if (gop->qp > (MAX_QP_VALUE - MAX_DELTA_QP_WHITIN_GOP)) {
                    max = MAX_QP_VALUE;
                } else {
                    max = gop->qp + MAX_DELTA_QP_WHITIN_GOP;
                }

                return CLIP3(min, max, new_qp);
            }
        }
    }

    return gop->qp;
}

static void record_new_gop(EbRateControlModel *model_ptr, PictureParentControlSet_t *picture_ptr) {
    uint64_t pictureNumber = picture_ptr->picture_number;
    EbRateControlGopInfo *gop = &model_ptr->gop_infos[pictureNumber];

    EbBlockOnMutex(model_ptr->model_mutex);

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

        if (gop->complexity >= model->scope_start && gop->complexity <= model->scope_end) {
            break ;
        }
    }

    for (unsigned int i = 0; DEFAULT_INTER_COMPLEXITY_MODEL[i].scope_end != MAX_COMPLEXITY; i++) {
        model_inter = &DEFAULT_INTER_COMPLEXITY_MODEL[i];

        if (complexity_inter >= model_inter->scope_start && complexity_inter <= model_inter->scope_end) {
            break ;
        }
    }

    for (unsigned int i = 0; model_ptr->complexity_variation_intra_model[i].scope_end != MAX_COMPLEXITY; i++) {
        model_deviation = &model_ptr->complexity_variation_intra_model[i];

        if (gop->complexity >= model_deviation->scope_start && gop->complexity <= model_deviation->scope_end) {
            break ;
        }
    }

    for (unsigned int i = 0; model_ptr->complexity_variation_inter_model[i].scope_end != MAX_COMPLEXITY; i++) {
        model_inter_deviation = &model_ptr->complexity_variation_inter_model[i];

        if (complexity_inter >= model_inter_deviation->scope_start && complexity_inter <= model_inter_deviation->scope_end) {
            break ;
        }
    }

    uint32_t desired_total_bytes = (model_ptr->desired_bitrate / model_ptr->frame_rate) * model_ptr->reported_frames;
    int64_t delta_bytes = desired_total_bytes - model_ptr->total_bytes;
    int64_t extra = delta_bytes / MAX_PORTION_OF_EXTRA_BYTES;

    if (extra < 0 && gop->desired_size < (uint64_t)-extra) {
        gop->desired_size /= MAX_DOWNSIZE_FACTOR;
    } else {
        gop->desired_size += extra;
    }

    size_t size = (float)gop->desired_size / (float)model_ptr->pixels * MODEL_DEFAULT_PIXEL_AREA;

    for (unsigned int qp = 0; qp <= MAX_QP_VALUE; qp++) {
        EbRateControlComplexityQpMinMax *sizes = &model->size[qp];
        int64_t pitch = (sizes->max - sizes->min) / (model->scope_end - model->scope_start);
        EbRateControlComplexityQpMinMax *sizes_inter = &model_inter->size[qp];
        int64_t pitch_inter = (sizes_inter->max - sizes_inter->min) / (model_inter->scope_end - model_inter->scope_start);

        if (model->scope_end == MAX_COMPLEXITY) {
            pitch = PITCH_ON_MAX_COMPLEXITY_FOR_INTRA_FRAMES;
        }

        if (model_inter->scope_end == MAX_COMPLEXITY) {
            pitch_inter = PITCH_ON_MAX_COMPLEXITY_FOR_INTER_FRAMES;
        }

        // Estimate inter deviation from intra deviation
        if (model_inter_deviation->deviation_reported == 0) {
            model_inter_deviation->deviation = 1 / ((float)gop->complexity / 100);
        }

        uint64_t complexity_size_inter = (sizes_inter->min + (pitch_inter * (complexity_inter - model_inter->scope_start)));
        uint64_t complexity_size_intra = (sizes->min + (pitch * (gop->complexity - model->scope_start)));
        uint64_t projected_gop_size = ((float)complexity_size_intra * (1.0 / model_deviation->deviation)) + (((float)complexity_size_inter * (1.0 / model_inter_deviation->deviation)) * model_ptr->intra_period);

        if (size > projected_gop_size || qp == MAX_QP_VALUE) {
            gop->qp = qp;
            gop->expected_intra_size = (float)complexity_size_intra / MODEL_DEFAULT_PIXEL_AREA * (float)model_ptr->pixels;
            gop->expected_inter_size = (float)complexity_size_inter / MODEL_DEFAULT_PIXEL_AREA * (float)model_ptr->pixels;
            break;
        }
    }

    // Update length in gopinfos
    if (pictureNumber != 0) {
        EbRateControlGopInfo *previousGop = get_gop_infos(model_ptr->gop_infos, pictureNumber - 1);

        previousGop->length = gop->index - previousGop->index;
    }

    EbReleaseMutex(model_ptr->model_mutex);
}

uint32_t get_gop_size_in_bytes(EbRateControlModel *model_ptr) {
    return (float)model_ptr->desired_bitrate / (float)((float)model_ptr->frame_rate / (float)model_ptr->intra_period);
}
