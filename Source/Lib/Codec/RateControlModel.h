/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef RateControlModel_h
#define RateControlModel_h

#include "EbSequenceControlSet.h"
#include "RateControlGopInfo.h"

#define MAX_COMPLEXITY_MODEL_DEVIATION_REPORTED 5
#define MODEL_DEFAULT_PIXEL_AREA (1920 * 1080)
#define MAX_COMPLEXITY 999999

#define PITCH_ON_MAX_COMPLEXITY_FOR_INTER_FRAMES 17
#define PITCH_ON_MAX_COMPLEXITY_FOR_INTRA_FRAMES 57

#define MAX_PORTION_OF_EXTRA_BYTES 3
#define MAX_DOWNSIZE_FACTOR 15

#define MAX_AMOUNT_OF_FRAME_FOR_ON_THE_FLY_QP 150
#define AMOUNT_OF_REPORTED_FRAMES_TO_TRIGGER_ON_THE_FLY_QP 10

#define MAX_DELTA_QP_WHITIN_GOP 12

struct SequenceControlSet_s;

typedef struct EbRateControlComplexityQpMinMax_s
{
    uint32_t    min;
    uint32_t    max;
} EbRateControlComplexityQpMinMax;

typedef struct  EbRateControlComplexityModel_s
{
    uint32_t                        scope_start;
    uint32_t                        scope_end;
    EbRateControlComplexityQpMinMax size[MAX_QP_VALUE + 1];
} EbRateControlComplexityModel;

typedef struct  EbRateControlComplexityModelDeviation_s
 {
     uint32_t    scope_start;
     uint32_t    scope_end;
     float       deviation;
     uint32_t    deviation_reported;
 } EbRateControlComplexityModelDeviation;

/*
 * @struct Holds a prediction model for a sequence
 */
typedef struct    RateControlModel_s {
    /*
     * @variable uint32_t[64]. Predicted frame size for a given QP.
     * Indexed by QP. interSizePrediction[15] references the estimated size of a
     * frame at QP = 15
     */
    size_t      inter_size_predictions[64];

    EbRateControlComplexityModelDeviation      *complexity_variation_intra_model;
    EbRateControlComplexityModelDeviation      *complexity_variation_inter_model;

    /*
     * @variable uint32_t. Desired bitrate set in the configuration
     */
    uint32_t    desired_bitrate;

    /*
     * @variable EB_U32. Desired frame rate set in the configuration
     */
    uint32_t    frame_rate;

    /*
     * @variable EB_S32. Intra period length set in the configuration
     */
    int32_t     intra_period;
  
    /*
     * @variable uint32_t. Sum of the bytes of all the encoded frames
     */
    uint64_t    total_bytes;
  
    /*
     * @variable uint64_t. Number of frames encoded so far
     */
    uint64_t    reported_frames;

    /*
     * @variable uint32_t. Video width in pixels
     */
    uint32_t    width;

    /*
     * @variable uint32_t. Video height in pixels
     */
    uint32_t    height;

    /*
     * @variable uint32_t. Number on pixels per frame. width * height
     */
    uint32_t    pixels;

    /*
     * @variable uint32_t. Number of frame to encode
     */
    uint32_t    number_of_frame;

    /*
     * @variable EbRateControlGopInfo[]. Information about group of picture in
     * the current sequence. Dynamically allocated in RateControlInit.
     * Indexed by pictureNumber.
     */
    EbRateControlGopInfo    *gop_infos;

    /*
     * @variable uint32_t. The model is updated from the motion estimation
     * process so the model table should be thread safe.
     */
    EbHandle                model_mutex;

    /*
     * @variable float. Average sequence model variation.
     * Used as temporary variation before the complexity model sets itself
     */
    float model_variation;
} EbRateControlModel;

/*
 * @function rate_control_model_ctor. Allocate and initialize a new EbRateControlModel
 * with default values.
 * @param {EbRateControlModel*} object_double_ptr. Address of the pointer to EbRateControlModel
 * @return {EbErrorType}.
 */
EbErrorType rate_control_model_ctor(EbRateControlModel **object_double_ptr);

/*
 * @function rate_control_model_init. Initialize a model with data specific to a sequence.
 * Must be called before RateControlUpdateModel and RateControlGetQuantizer
 * @param {EbRateControlModel*} model_ptr.
 * @param {struct SequenceControlSet_s*} sequence_control_set_ptr. First frame used to initialize the model
 * @return {EbErrorType}.
 */
EbErrorType rate_control_model_init(EbRateControlModel *model_ptr,
                                    struct SequenceControlSet_s *sequence_control_set_ptr);

/*
 * @function rate_control_report_complexity. Record the complexity of a frame.
 * @param {EbRateControlModel*} model_ptr.
 * @param {PictureParentControlSet_t*} picture_ptr. Frame.
 * @return {EbErrorType}.
 */
EbErrorType rate_control_report_complexity(EbRateControlModel *model_ptr,
                                           PictureParentControlSet_t *picture_ptr);

/*
 * @function rate_control_update_model. Update a model with information from an encoded frame.
 * @param {EbRateControlModel*} model_ptr.
 * @param {PictureParentControlSet_t*} picture_ptr. Encoded frame.
 * @return {EbErrorType}.
 */
EbErrorType rate_control_update_model(EbRateControlModel *model_ptr,
                                      PictureParentControlSet_t *picture_ptr);

/*
 * @function rate_control_get_quantizer. Return a QP for the given frame to be encoded.
 * Uses data from the model and information from the given frame to make a decision
 * @param {EbRateControlModel*} model_ptr.
 * @param {PictureParentControlSet_t*} picture_ptr. Frame to be encoded.
 * @return {uint8_t}. Suggested QP for the given frame
 */
uint8_t rate_control_get_quantizer(EbRateControlModel *model_ptr,
                                   PictureParentControlSet_t *picture_ptr);

/*
 * @function get_gop_size_in_bytes. Return the size in bytes a new gop should take
 * to fit closer to the rate control constraints.
 * @param {EbRateControlModel*} model_ptr.
 * @return {uint32_t}. Ideal size the gop should take.
 */
uint32_t  get_gop_size_in_bytes(EbRateControlModel *model_ptr);

/*
 * @function estimate_gop_complexity. Estimate the complexity of the given gop.
 * @param {RateControlModel_t*} model_ptr. Rate control model.
 * @param {RateControlGopInfo_t} gop_ptr. GOP to estimate.
 * @return {uint32_t}. Estimation of the complexity or 0 if unknown.
 */
uint32_t estimate_gop_complexity(EbRateControlModel *model_ptr,
                                 EbRateControlGopInfo *gop_ptr);

#endif // RateControlModel_h
