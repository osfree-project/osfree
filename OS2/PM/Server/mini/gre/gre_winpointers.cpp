/**
 * @brief
 *
 * @param[in] a
 * @param[in] b
 * @param[in] c
 *
 * @return 
 *
 * @retval 
 *
 * @note Not found any information yet and not found any driver used it?
 */
GRE_DRV_4 (GreDrvDeviceSetPointer)

/*!
 * @brief Dispatch for GreSetColorCursor — set the bitmaps that define a color cursor.
 *
 * @param [in] hdc           Device context handle.
 * @param [in] pPointerInfo  Pointer to a pointer information structure.
 * @param [in] pInstance     Pointer to instance data.
 * @param [in] lFunction     High-order WORD=flags; low-order
 *                           WORD=NGreSetColorCursor.
 *
 * @return BOOLEAN success indicator.
 * @retval TRUE   Successful.
 * @retval FALSE  Error.
 *
 * @par EDM2 Reference
 * GreSetColorCursor (https://www.edm2.com/index.php/GreSetColorCursor):
 * "GreSetColorCursor sets the bit maps that define a color cursor or pointer."
 *
 *
 * @par Support
 * | Simulated | Hooked  | Chained |
 * |-----------|---------|---------|
 * | Yes(oth)  |Must(dsp)|   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_DEV_FUNC_NOT_INSTALLED
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_CURSOR_BITMAP
 * - PMERR_INV_HDC
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 * @par Remarks
 * "The handling routine in the presentation driver updates its copy of the
 * pointer definition to that identified by pPointerInfo."
 *
 */
GRE_DRV_4 (GreDrvSetColorCursor)

#endif