#ifdef INCL_AVIOP
   /**
    * @brief Output characters in a rectangle.
    *
    * @param[in] hdc  Device context handle.
    * @param[in] pPs  Presentation space.
    * @param[in] pRect Pointer to a grid rectangle reference.
    *
    * @return ULONG result.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreCharRect 0x0000403BL
   #define GreCharRect(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(VioPresentationSpace *)(b),(ULONG)(LPGridRectRef)(c),0L,NGreCharRect)

   /**
    * @brief Draw a string of character cells from the LVB to the device context.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] pPs   Presentation space.
    * @param[in] pStr  Pointer to a grid string reference.
    *
    * @return LONG value indicating success or an error code.
    *
    * @retval NO_ERROR  Successful.
    *
    * @note If an error is returned, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_DEV_FUNC_NOT_INSTALLED
    *       - PMERR_INV_HDC
    */
   #define NGreCharStr 0x0000403CL
   #define GreCharStr(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(VioPresentationSpace *)(b),(ULONG)(LPGridStringRef)(c),0L,NGreCharStr)

   /**
    * @brief Scroll a rectangle.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] pPs   Presentation space.
    * @param[in] pRect Pointer to a scroll rectangle reference.
    *
    * @return LONG value indicating success or an error code.
    *
    * @retval NO_ERROR  Successful.
    * @retval CE_INVALID_PRESENTATION_SPACE  Error.
    *
    * @note If CE_INVALID_PRESENTATION_SPACE is returned, the handling routine
    *       must call WinSetErrorInfo.
    */
   #define NGreScrollRect 0x0000403DL
   #define GreScrollRect(a,b,c) (ULONG) Gre32Entry5((ULONG)(HDC)(a),(ULONG)(VioPresentationSpace *)(b),(ULONG)(LPScrollRectRef)(c),0L,NGreScrollRect)

   /* GreUpdateCursor — no verifiable error list found; left undocumented. */
   #define NGreUpdateCursor 0x0000403EL
   #define GreUpdateCursor(a,b) (ULONG) Gre32Entry4((ULONG)(HDC)(a),(ULONG)(VioPresentationSpace *)(b),0L,NGreUpdateCursor)
#endif
