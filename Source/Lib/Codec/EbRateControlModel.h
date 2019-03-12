/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef RateControlModel_h
#define RateControlModel_h

#include "EbSequenceControlSet.h"

#define MAX_COMPLEXITY_MODEL_DEVIATION_REPORTED 5
#define MODEL_DEFAULT_PIXEL_AREA (1920 * 1080)
#define MAX_COMPLEXITY 999999

#define PITCH_ON_MAX_COMPLEXITY_FOR_INTER_FRAMES 17
#define PITCH_ON_MAX_COMPLEXITY_FOR_INTRA_FRAMES 57

#define DAMPING_FACTOR 2
#define MAX_DOWNSIZE_FACTOR 15

#define AMOUNT_OF_REPORTED_FRAMES_TO_TRIGGER_ON_THE_FLY_QP 2
#define MAX_INTER_LEVEL_FOR_ON_THE_FLY_QP 4

#define MAX_DELTA_QP_WHITIN_GOP 12
#define MAX_INTER_COMPLEXITY_DEVIATION 24

// Variance above which mean variance is preferred
#define VARIANCE_CALC_PIVOT 320

#define RC_DEVIATION_PRECISION 16

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
     uint64_t    deviation;
     uint32_t    deviation_reported;
 } EbRateControlComplexityModelDeviation;

typedef struct  EbRateControlGopInfo_s {
    /*
     * @variable EbBool. Represents an intra if not zero.
     */
    EbBool      exists;

    /*
     * @variable EbBool. Is the frame encoded.
     */
    EbBool      encoded;

    /*
     * @variable uint64_t. Frame index.
     */
    uint64_t    index;

    /*
     * @variable size_t. Estimated size for the frame in bytes.
     */
    size_t      desired_size;

    /*
     * @variable size_t. Actual size of the gop once encoded.
     */
    size_t      actual_size;

    /*
     * @variable size_t. Size of the intra frame in bits.
     */
    size_t      intra_size;

    /*
     * @variable size_t. Expected size of the intra frame.
     */
    size_t      expected_intra_size;

    /*
     * @variable size_t. Expected size of the inter frames.
     */
    size_t      expected_inter_size;

    /*
     * @variable uint32_t. Number of encoded frames in this GOP.
     */
    uint32_t    reported_frames;

    /*
     * @variable size_t. Number of frames in the GOP.
     */
    size_t      length;

    /*
     * @variable uint8_t. Assigned QP.
     */
    uint8_t     qp;

    /*
     * @variable uint32_t. Complexity score of the intra.
     */
    uint64_t    complexity;

    /*
     * @variable PictureParentControlSet_t*. Pointer to the actual frame.
     */
    PictureParentControlSet_t *picture_control_set_ptr;
} EbRateControlGopInfo;

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

    /*
     * @variable EbRateControlComplexityModelDeviation[]. Copy of the reference table DEFAULT_INTRA_COMPLEXITY_MODEL.
     */
    EbRateControlComplexityModelDeviation      *complexity_variation_intra_model;

    /*
     * @variable EbRateControlComplexityModelDeviation[]. Copy of the reference table DEFAULT_INTRA_COMPLEXITY_MODEL.
     */
    EbRateControlComplexityModelDeviation      *complexity_variation_inter_model;

    /*
     * @variable uint32_t. Desired bitrate set in the configuration
     */
    uint64_t    desired_bitrate;

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
    size_t      total_bytes;
  
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
    uint64_t    pixels;

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
     * @variable EbHandle. The model is updated from the initial rate control
     * process so the model table should be thread safe.
     */
    EbHandle                model_mutex;

    /*
     * @variable uint64_t. Average sequence intra model variation.
     * Used as temporary variation before the complexity model sets itself
     */
    int64_t                 intra_default_variation;

    /*
     * @variable uint64_t. Number of intra model variation reported.
     */
    uint64_t                intra_default_variation_reported;

    /*
     * @variable uint64_t. Average sequence inter model variation.
     * Used as temporary variation before the complexity model sets itself
     */
    int64_t                 inter_default_variation;

    /*
     * @variable uint64_t. Number of inter model variation reported.
     */
    uint64_t                inter_default_variation_reported;
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
 * @param {PictureParentControlSet_t*} picture_control_set_ptr. Frame.
 * @return {EbErrorType}.
 */
EbErrorType rate_control_report_complexity(EbRateControlModel *model_ptr,
                                           PictureParentControlSet_t *picture_control_set_ptr);

/*
 * @function rate_control_update_model. Update a model with information from an encoded frame.
 * @param {EbRateControlModel*} model_ptr.
 * @param {PictureParentControlSet_t*} picture_control_set_ptr. Encoded frame.
 * @return {EbErrorType}.
 */
EbErrorType rate_control_update_model(EbRateControlModel *model_ptr,
                                      PictureParentControlSet_t *picture_control_set_ptr);

/*
 * @function rate_control_get_quantizer. Return a QP for the given frame to be encoded.
 * Uses data from the model and information from the given frame to make a decision
 * @param {EbRateControlModel*} model_ptr.
 * @param {PictureParentControlSet_t*} picture_control_set_ptr. Frame to be encoded.
 * @return {uint8_t}. Suggested QP for the given frame
 */
uint8_t rate_control_get_quantizer(EbRateControlModel *model_ptr,
                                   PictureParentControlSet_t *picture_control_set_ptr);

/*
 * @function get_gop_size_in_bytes. Return the size in bytes a new gop should take
 * to fit closer to the rate control constraints.
 * @param {EbRateControlModel*} model_ptr.
 * @return {size_t}. Ideal size the gop should take.
 */
size_t  get_gop_size_in_bytes(EbRateControlModel *model_ptr);

/*
 * @function get_gop_infos. Retreive the gop structure of the frame at the given
 * position from the list of GOP already recorded. Because intra frames are not
 * guaranteed to be processed in order, the previous GOP returned at the
 * moment of the call *may* not be the actual previous one in the final
 * bitsteam
 * @param {EbRateControlGopInfo*} gop_info. Typically RateControlModel->gopInfos
 * @param {uint64_t} position. Position of the GOP to start the search from.
 * @return {EbRateControlGopInfo*}. Pointer to the GOP structure.
 * or EB_NULL if not found (unlikely).
 */
EbRateControlGopInfo *get_gop_infos(EbRateControlGopInfo *gop_info,
                                    uint64_t position);

/*
 * @function estimate_gop_complexity. Estimate the complexity of the given gop.
 * @param {RateControlModel_t*} model_ptr. Rate control model.
 * @param {RateControlGopInfo_t} gop_ptr. GOP to estimate.
 * @return {uint32_t}. Estimation of the complexity or 0 if unknown.
 */
uint32_t estimate_gop_complexity(EbRateControlModel *model_ptr,
                                 EbRateControlGopInfo *gop_ptr);

#endif // RateControlModel_h
