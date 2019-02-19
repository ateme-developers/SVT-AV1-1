/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef RateControlABR_h
#define RateControlABR_h

#include "RateControlModel.h"

/*
 * @function rate_control_abr_gop_completed. Record a completed gop
 * @param {EbRateControlModel*} model_ptr.
 * @param {EbRateControlGopInfo*} gop_ptr.
 * @return {void}.
 */
void rate_control_abr_gop_completed(EbRateControlModel *model_ptr, EbRateControlGopInfo *gop_ptr);

/*
 * @function rate_control_record_new_gop_abr. Initilaize gop_ptr to meet model_ptr
 * rate control average bitrate criteria
 * @param {EbRateControlModel*} model_ptr.
 * @param {EbRateControlGopInfo*} gop_ptr.
 * @return {void}.
 */
void rate_control_record_new_gop_abr(EbRateControlModel *model_ptr, EbRateControlGopInfo *gop_ptr);

#endif /* RateControlABR_h */