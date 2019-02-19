/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#include "RateControlModel.h"
#include "RateControlGopInfo.h"

EbRateControlGopInfo *get_gop_infos(EbRateControlGopInfo *gop_info,
                                    uint64_t position)
{
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
                                 EbRateControlGopInfo *gop_ptr)
{
    uint32_t                complexity = 0;
    EbRateControlGopInfo    *current;
    int64_t                 position = 0;
    uint32_t                reported_complexity = 0;

    while ((position + gop_ptr->index) < model_ptr->number_of_frame &&
           position < (int64_t)model_ptr->intra_period)
    {
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
