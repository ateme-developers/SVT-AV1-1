/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef RateControlCBR_h
#define RateControlCBR_h

#include "RateControlModel.h"

/*
 * @function rate_control_cbr_init. Initilaize model_ptr too meet
 * rate control constant bitrate criteria
 * @param {EbRateControlModel*} model_ptr.
 * @return {void}.
 */
void rate_control_cbr_init(EbRateControlModel *model_ptr);

/*
 * @function rate_control_record_new_gop_cbr. Initilaize gop_ptr to meet model_ptr
 * rate control constant bitrate criteria
 * @param {EbRateControlModel*} model_ptr.
 * @param {EbRateControlGopInfo*} gop_ptr.
 * @return {void}.
 */
void rate_control_record_new_gop_cbr(EbRateControlModel *model_ptr, EbRateControlGopInfo *gop_ptr);

#endif /* RateControlCBR_h */