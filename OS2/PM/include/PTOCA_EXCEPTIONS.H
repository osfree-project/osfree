/*!
   @file PTOCA_EXCEPTIONS.H
   @brief Exception Conditions (EC codes) of the PTOCA architecture.

   Source: AFP Consortium, "Presentation Text Object Content Architecture
   Reference" (PTOCA Reference).
*/

#ifndef PTOCA_EXCEPTIONS_H
#define PTOCA_EXCEPTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Invalid length of the RPS/TRN source string or UCT data.
    @details Odd-length string for double-byte fonts.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_RPS_TRN_LENGTH         0x1A01

/*! @brief Invalid Unicode data.
    @details Broken UTF-16 surrogate-pair sequence or invalid UTF-8 byte
             sequence.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_INVALID_UNICODE        0x1A03

/*! @brief Invalid length of the RPS (Repeat String) target string.
    @details Odd-length repeat count for double-byte fonts.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_RPS_TARGET_LENGTH      0x1B01

/*! @brief Invalid control-sequence class.
    @details The X'2B' sequence class is not X'D3'.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_INVALID_CONTROL_CLASS  0x1C01

/*! @brief Invalid length of a control sequence or initial-condition
           parameter.
    @details A required parameter or part of a parameter is missing.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_INVALID_LENGTH         0x1E01

/*! @brief RPS length error.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_RPS_LENGTH             0x1F01

/*! @brief Invalid character.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_INVALID_CHARACTER      0x2100

/*! @brief Text orientation is incompatible with the selected font.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_ORIENTATION_FONT       0x3F02

/*! @brief Color or color attribute in STC is not supported.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_COLOR_NOT_SUPPORTED    0x5803

/*! @brief Invalid or unsupported initial I-orientation.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_INVALID_I_ORIENTATION  0x6802

/*! @brief Invalid or unsupported initial B-orientation.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_INVALID_B_ORIENTATION  0x6902

/*! @brief Invalid or unsupported initial I-displacement.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_INVALID_I_DISPLACEMENT 0x6A02

/*! @brief Invalid or unsupported initial B-displacement.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_INVALID_B_DISPLACEMENT 0x6B02

/*! @brief Invalid or unsupported rule width in DBR/DIR.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_INVALID_RULE_WIDTH     0x8002

/*! @brief Invalid or unsupported rule length in DBR/DIR.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_INVALID_RULE_LENGTH    0x8202

/*! @brief Invalid or unsupported LID in BSU or ESU.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_INVALID_BSU_ESU_LID    0x9801

/*! @brief Temporary Baseline Move (TBM) error.
    @details Includes unsupported TBM or unsupported character substitution.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_TBM_ERROR              0x9803

/*! @brief Invalid overstrike character.
    @details The character has an invalid increment or is not printable.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_INVALID_OVERSTRIKE     0x9A01

/*! @brief Invalid UCT parameters (CTLNGTH, UCTVERS, BIDICT, GLYPHCT).
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_INVALID_UCT_PARAM      0x9B01

/*! @brief Decryption is not available on the device.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_DECRYPTION_UNAVAILABLE 0x9D01

/*! @brief Decryption error.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_DECRYPTION_ERROR       0x9D02

/*! @brief No decryption key information was supplied.
    @see PTOCA Reference, Table 18. */
#define PTOCA_EC_NO_DECRYPTION_KEY      0x9D03

#ifdef __cplusplus
}
#endif

#endif /* PTOCA_EXCEPTIONS_H */
