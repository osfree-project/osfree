/*!
   @file IOCA_EXCEPTIONS.H
   @brief Exception Conditions (EC codes) of the IOCA architecture.

   Source: AFP Consortium, "Image Object Content Architecture Reference".
*/

#ifndef IOCA_EXCEPTIONS_H
#define IOCA_EXCEPTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Invalid LENGTH value.
    @details The LENGTH value is outside the allowed range.
    @see IOCA Reference, Image Size parameter. */
#define IOCA_EC_INVALID_LENGTH          0x0003

/*! @brief Invalid parameter value.
    @details HRESOL, VRESOL, HSIZE, or VSIZE is outside the allowed range.
    @see IOCA Reference, Image Size parameter. */
#define IOCA_EC_INVALID_PARAM_VALUE     0x0004

/*! @brief Invalid LENGTH value (optional).
    @see IOCA Reference, IDE Structure parameter. */
#define IOCA_EC_INVALID_LENGTH_OPT      0x0005

/*! @brief Invalid sequence.
    @details The Image Size parameter is missing or appears out of order.
    @see IOCA Reference, Image Size parameter. */
#define IOCA_EC_INVALID_SEQUENCE        0x940F

/*! @brief Invalid or unsupported Image Data parameter value.
    @see IOCA Reference, Image Size parameter. */
#define IOCA_EC_INVALID_IMAGE_PARAM     0x9410

/*! @brief Inconsistent Image Data parameters.
    @details HSIZE or VSIZE is zero, and the size cannot be determined from
             the data.
    @see IOCA Reference, Image Size parameter. */
#define IOCA_EC_INCONSISTENT_IMAGE_DATA 0x9411

/*! @brief Inconsistent size between the parameter and the data.
    @details The size found in the data differs from HSIZE/VSIZE.
    @see IOCA Reference, Image Size parameter. */
#define IOCA_EC_INCONSISTENT_SIZE       0x9401

/*! @brief Invalid sequence for IDE Structure.
    @details The parameter is required but is missing or appears out of order.
    @see IOCA Reference, IDE Structure parameter. */
#define IOCA_EC_IDE_INVALID_SEQUENCE    0x9B0F

/*! @brief Invalid or unsupported IDE Structure value.
    @details Invalid ASFLAG, GRAYCODE, FORMAT, or SIZE values.
    @see IOCA Reference, IDE Structure parameter. */
#define IOCA_EC_IDE_INVALID_VALUE       0x9B10

/*! @brief Invalid IDE Structure parameter.
    @details Inconsistent sum of SIZE values, missing SIZE4 for CMYK, etc.
    @see IOCA Reference, IDE Structure parameter. */
#define IOCA_EC_IDE_INVALID             0x9B18

/*! @brief Compression decoder error.
    @details The decoder cannot process the data because a required function
             is missing.
    @see IOCA Reference, Image Encoding parameter. */
#define IOCA_EC_DECODER_ERROR           0x9511

/*! @brief Invalid use of Solid Fill Rectangle compression.
    @details The algorithm may be used only for tiled bilevel images.
    @see IOCA Reference, Image Encoding parameter. */
#define IOCA_EC_SOLID_FILL_INVALID      0x9510

/*! @brief Missing External Algorithm Specification.
    @details Required when COMPRID or RECID is X'FE'.
    @see IOCA Reference, Image Encoding parameter. */
#define IOCA_EC_MISSING_EXT_ALGORITHM   0x9F01

/*! @brief Invalid number of color names in nColor Names.
    @details The number of names does not match n in FORMAT X'8n'.
    @see IOCA Reference, nColor Names parameter. */
#define IOCA_EC_NCOLOR_NAME_COUNT       0xB311

#ifdef __cplusplus
}
#endif

#endif /* IOCA_EXCEPTIONS_H */
