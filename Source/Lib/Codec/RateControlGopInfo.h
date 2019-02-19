/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/


#ifndef RateControlGopInfo_h
#define RateControlGopInfo_h

struct  EbRateControlGopInfo_s;

/*
 * @struct Holds rate control information for a group of picture
 */
typedef struct  EbRateControlGopInfo_s {
    /*
     * @variable uint8_t. Represents an intra if not zero.
     */
    EbBool      exists;

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
     * @variable uint8_t. Number of encoded frames in this GOP.
     */
    uint32_t    reported_frames;

    /*
     * @variable uint8_t. Number of frames in the GOP.
     */
    size_t      length;

    /*
     * @variable uint32_t. Assigned QP.
     */
    uint32_t    qp;

    /*
     * @variable uint32_t. Complexity score of the intra.
     */
    uint32_t    complexity;

    PictureParentControlSet_t *picture_ptr;
} EbRateControlGopInfo;

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

#endif /* RateControlGopInfo_h */
