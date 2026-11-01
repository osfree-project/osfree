/*!
   @file CMOCA_EXCEPTIONS.H
   @brief Exception Conditions (EC codes) of the CMOCA architecture.

   Source: AFP Consortium, "Color Management Object Content Architecture
   Reference".
*/

#ifndef CMOCA_EXCEPTIONS_H
#define CMOCA_EXCEPTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Invalid count value for the tag.
    @see CMOCA Reference, Halftone Subset Tag. */
#define CMOCA_EC_INVALID_COUNT          0x001105

/*! @brief Invalid field type for the tag.
    @see CMOCA Reference, Halftone Subset Tag. */
#define CMOCA_EC_INVALID_FIELD_TYPE     0x001106

/*! @brief Invalid tag sequence.
    @details The tag appears out of order or more than once.
    @see CMOCA Reference, Halftone Subset Tag. */
#define CMOCA_EC_INVALID_SEQUENCE       0x00110F

/*! @brief Invalid value.
    @details The number of components is zero or greater than 15.
    @see CMOCA Reference, Halftone Subset Tag. */
#define CMOCA_EC_INVALID_VALUE          0x001110

/*! @brief Required tag is missing.
    @see CMOCA Reference, Halftone Subset Tag. */
#define CMOCA_EC_MISSING_REQUIRED_TAG   0x101108

#ifdef __cplusplus
}
#endif

#endif /* CMOCA_EXCEPTIONS_H */
