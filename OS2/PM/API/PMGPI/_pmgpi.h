/****************************** Module Header ******************************\
*
* Module Name: PMGPI.H
*
* OS/2 Presentation Manager GPI constants, types and function declarations
*
* Copyright (c) International Business Machines Corporation 1981, 1988-1992
*
* ===========================================================================
*
* The folowing symbols are used in this file for conditional sections.
*
*   #define:                To include:
*
*   INCL_GPI                Include all of the GPI
*   INCL_GPIBITMAPS         Bitmaps and Pel Operations
*   INCL_GPICONTROL         Basic PS control
*   INCL_GPICORRELATION     Picking, Boundary and Correlation
*   INCL_GPIDEFAULTS        Default Primitive Attributes
*   INCL_GPIINK             Inking.
*   INCL_GPIERRORS          defined if INCL_ERRORS defined
*   INCL_GPILCIDS           Phyical and Logical Fonts with Lcids
*   INCL_GPILOGCOLORTABLE   Logical Color Tables
*   INCL_GPIMETAFILES       MetaFiles
*   INCL_GPIPATHS           Paths and Clipping with Paths
*   INCL_GPIPOLYGON         Polygons
*   INCL_GPIPRIMITIVES      Drawing Primitives and Primitive Attributes
*   INCL_GPIREGIONS         Regions and Clipping with Regions
*   INCL_GPISEGEDITING      Segment Editing via Elements
*   INCL_GPISEGMENTS        Segment Control and Drawing
*   INCL_GPITRANSFORMS      Transform and Transform Conversion
*
* There is a symbol used in this file called INCL_DDIDEFS. This is used to
* include only the definitions for the DDI. The programmer using the GPI
* can ignore this symbol
*
* There is a symbol used in this file called INCL_SAADEFS. This is used to
* include only the definitions for the SAA. The programmer using the GPI
* can ignore this symbol
*
* ===========================================================================
*
* Comments at the end of each typedef line give the name tags used in
* the assembler include version of this file.
*
* The assembler include version of this file excludes lines between NOINC
* and INC comments.
*
\***************************************************************************/

/* NOINC */
#if __IBMC__ || __IBMCPP__
   #pragma info( none )
   #ifndef __CHKHDR__
      #pragma info( none )
   #endif
   #pragma info( restore )
#endif
#ifdef __cplusplus
      extern "C" {
#endif
/* INC */

#define GPI_INCLUDED


#ifdef INCL_GPI /* include whole of the GPI */
   #define INCL_GPIBITMAPS
   #define INCL_GPICONTROL
   #define INCL_GPICORRELATION
   #define INCL_GPIDEFAULTS
   #define INCL_GPIINK
   #define INCL_GPILCIDS
   #define INCL_GPILOGCOLORTABLE
   #define INCL_GPIMETAFILES
   #define INCL_GPIPATHS
   #define INCL_GPIPOLYGON
   #define INCL_GPIPRIMITIVES
   #define INCL_GPIREGIONS
   #define INCL_GPISEGEDITING
   #define INCL_GPISEGMENTS
   #define INCL_GPITRANSFORMS
#endif /* INCL_GPI */


#ifdef INCL_ERRORS /* if errors are required then allow GPI errors */
   #define INCL_GPIERRORS
#endif /* INCL_ERRORS */


#ifdef INCL_DDIDEFS /* if only DDI required then enable DDI part of GPI */
   #define INCL_GPIBITMAPS
   #define INCL_GPIERRORS
   #define INCL_GPILCIDS
   #define INCL_GPILOGCOLORTABLE
   #define INCL_GPIPATHS
   #define INCL_GPIPOLYGON
   #define INCL_GPIPRIMITIVES
   #define INCL_GPIREGIONS
   #define INCL_GPITRANSFORMS
#endif /* INCL_DDIDEFS */


#ifdef INCL_SAADEFS /* if only SAA required then enable SAA part of GPI */
   #define INCL_GPIBITMAPS
   #define INCL_GPICONTROL
   #define INCL_GPICORRELATION
   #define INCL_GPIERRORS
   #define INCL_GPILCIDS
   #define INCL_GPILOGCOLORTABLE
   #define INCL_GPIMETAFILES
   #define INCL_GPIPATHS
   #define INCL_GPIPOLYGON
   #define INCL_GPIPRIMITIVES
   #define INCL_GPIREGIONS
   #define INCL_GPISEGEDITING
   #define INCL_GPISEGMENTS
   #define INCL_GPITRANSFORMS
#endif /* INCL_SAADEFS */

/* General GPI return values */
#define GPI_ERROR                       0L
#define GPI_OK                          1L
#define GPI_ALTERROR                  (-1L)

/* fixed point number - implicit binary point between 1st and 2nd hex digits */
typedef  USHORT FIXED88;  /* fx88 */

/* fixed point signed number - implicit binary point between bits 14 and 13. */
/*                             Bit 15 is the sign bit.                       */
/*                             Thus 1.0 is represented by 16384 (0x4000)     */
/*                             and -1.0 is represented by -16384 (0xc000)    */
typedef  USHORT FIXED114; /* fx114 */

/* make FIXED number from SHORT integer part and USHORT fractional part */
#define MAKEFIXED(intpart,fractpart) MAKELONG(fractpart,intpart)
/* extract fractional part from a fixed quantity */
#define FIXEDFRAC(fx)                (LOUSHORT(fx))
/* extract integer part from a fixed quantity */
#define FIXEDINT(fx)                 ((SHORT)HIUSHORT(fx))

/* structure for size parameters e.g. for GpiCreatePS */
typedef struct _SIZEL            /* sizl */
{
   LONG cx;
   LONG cy;
} SIZEL;
typedef SIZEL *PSIZEL;

/* return code on GpiQueryLogColorTable,GpiQueryRealColors and GpiQueryPel */
#define CLR_NOINDEX                  (-254L)

#if (defined(INCL_GPICONTROL) || !defined(INCL_NOCOMMON))
   
   /* units for GpiCreatePS and others */
   #define PU_ARBITRARY               0x0004L
   #define PU_PELS                    0x0008L
   #define PU_LOMETRIC                0x000CL
   #define PU_HIMETRIC                0x0010L
   #define PU_LOENGLISH               0x0014L
   #define PU_HIENGLISH               0x0018L
   #define PU_TWIPS                   0x001CL
   
   /* format for GpiCreatePS */
   #define GPIF_DEFAULT                    0L
   #define GPIF_SHORT                 0x0100L
   #define GPIF_LONG                  0x0200L
   
   
   /* PS type for GpiCreatePS */
   #define GPIT_NORMAL                     0L
   #define GPIT_MICRO                 0x1000L
   #define GPIT_INK                   0x2000L
   
   
   /* implicit associate flag for GpiCreatePS */
   #define GPIA_NOASSOC                    0L
   #define GPIA_ASSOC                 0x4000L
   
   /* PS Fill type for GpiCreatePS */
   #define GPIM_AREAEXCL              0x8000L
   
   
   #ifndef INCL_SAADEFS
      /* return error for GpiQueryDevice */
      #define HDC_ERROR                ((HDC)-1L)
   #endif /* no INCL_SAADEFS */
   
   /* common GPICONTROL functions */
   
   HPS   APIENTRY GpiCreatePS(HAB hab,
                              HDC hdc,
                              PSIZEL psizlSize,
                              ULONG flOptions);
   
   BOOL  APIENTRY GpiDestroyPS(HPS hps);
   
   BOOL  APIENTRY GpiAssociate(HPS hps,
                               HDC hdc);
   
   BOOL  APIENTRY GpiRestorePS(HPS hps,
                               LONG lPSid);
   
   LONG  APIENTRY GpiSavePS(HPS hps);
   
   BOOL  APIENTRY GpiErase(HPS hps);
   
   #ifndef INCL_SAADEFS
      
      HDC  APIENTRY GpiQueryDevice(HPS);
   #endif /* no INCL_SAADEFS */
   
#endif /* common GPICONTROL */
#ifdef INCL_GPICONTROL
   
   /* options for GpiResetPS */
   #define GRES_ATTRS                 0x0001L
   #define GRES_SEGMENTS              0x0002L
   #define GRES_ALL                   0x0004L
   
   /* option masks for PS options used by GpiQueryPs */
   #define PS_UNITS                   0x00FCL
   #define PS_FORMAT                  0x0F00L
   #define PS_TYPE                    0x1000L
   #define PS_MODE                    0x2000L
   #define PS_ASSOCIATE               0x4000L
   #define PS_NORESET                 0x8000L
   
   
   /* error context returned by GpiErrorSegmentData */
   #define GPIE_SEGMENT                    0L
   #define GPIE_ELEMENT                    1L
   #define GPIE_DATA                       2L
   
   #ifndef INCL_SAADEFS
      
      /* control parameter for GpiSetDrawControl */
      #define DCTL_ERASE                      1L
      #define DCTL_DISPLAY                    2L
      #define DCTL_BOUNDARY                   3L
      #define DCTL_DYNAMIC                    4L
      #define DCTL_CORRELATE                  5L
      
      /* constants for GpiSet/QueryDrawControl */
      #define DCTL_ERROR                     -1L
      #define DCTL_OFF                        0L
      #define DCTL_ON                         1L
      
      /* constants for GpiSet/QueryStopDraw */
      #define SDW_ERROR                      -1L
      #define SDW_OFF                         0L
      #define SDW_ON                          1L
      
   #endif /* no INCL_SAADEFS */
   
   /* drawing for GpiSet/QueryDrawingMode */
   #define DM_ERROR                        0L
   #define DM_DRAW                         1L
   #define DM_RETAIN                       2L
   #define DM_DRAWANDRETAIN                3L
   
   /*** other GPICONTROL functions */
   
   BOOL  APIENTRY GpiResetPS(HPS hps,
                             ULONG flOptions);
   
   #ifndef INCL_SAADEFS
      
      
      BOOL  APIENTRY GpiSetPS(HPS hps,
                              PSIZEL psizlsize,
                              ULONG flOptions);
      
   #endif /* no INCL_SAADEFS */
   
   
   ULONG  APIENTRY GpiQueryPS(HPS hps,
                              PSIZEL psizlSize);
   
   LONG   APIENTRY GpiErrorSegmentData(HPS hps,
                                       PLONG plSegment,
                                       PLONG plContext);
   
   #ifndef INCL_SAADEFS
      
      
      LONG  APIENTRY GpiQueryDrawControl(HPS hps,
                                         LONG lControl);
      
      BOOL  APIENTRY GpiSetDrawControl(HPS hps,
                                       LONG lControl,
                                       LONG lValue);
      
   #endif /* no INCL_SAADEFS */
   
   
   LONG  APIENTRY GpiQueryDrawingMode(HPS hps);
   
   BOOL  APIENTRY GpiSetDrawingMode(HPS hps,
                                    LONG lMode);
   
   #ifndef INCL_SAADEFS
      
      
      LONG  APIENTRY GpiQueryStopDraw(HPS hps);
      
      BOOL  APIENTRY GpiSetStopDraw(HPS hps,
                                    LONG lValue);
      
   #endif /* no INCL_SAADEFS */
   
#endif /* non-common GPICONTROL */
#ifdef INCL_GPICORRELATION
   
   /* options for GpiSetPickApertureSize */
   #define PICKAP_DEFAULT                  0L
   #define PICKAP_REC                      2L
   
   /* type of correlation for GpiCorrelateChain */
   #define PICKSEL_VISIBLE                 0L
   #define PICKSEL_ALL                     1L
   
   /* return code to indicate correlate hit(s) */
   #define GPI_HITS                        2L
   
   /*** picking,  correlation and boundary functions */
   
   LONG  APIENTRY GpiCorrelateChain(HPS hps,
                                    LONG lType,
                                    PPOINTL pptlPick,
                                    LONG lMaxHits,
                                    LONG lMaxDepth,
                                    PLONG pl2);
   
   BOOL  APIENTRY GpiQueryTag(HPS hps,
                              PLONG plTag);
   
   BOOL  APIENTRY GpiSetTag(HPS hps,
                            LONG lTag);
   
   BOOL  APIENTRY GpiQueryPickApertureSize(HPS hps,
                                           PSIZEL psizlSize);
   
   BOOL  APIENTRY GpiSetPickApertureSize(HPS hps,
                                         LONG lOptions,
                                         PSIZEL psizlSize);
   
   #ifndef INCL_SAADEFS
      
      BOOL  APIENTRY GpiQueryPickAperturePosition(HPS hps,
                                                  PPOINTL pptlPoint);
      
      BOOL  APIENTRY GpiSetPickAperturePosition(HPS hps,
                                                PPOINTL pptlPick);
      
      BOOL  APIENTRY GpiQueryBoundaryData(HPS hps,
                                          PRECTL prclBoundary);
      
      BOOL  APIENTRY GpiResetBoundaryData(HPS hps);
      
   #endif /* no INCL_SAADEFS */
   
   
   LONG APIENTRY GpiCorrelateFrom(HPS hps,
                                  LONG lFirstSegment,
                                  LONG lLastSegment,
                                  LONG lType,
                                  PPOINTL pptlPick,
                                  LONG lMaxHits,
                                  LONG lMaxDepth,
                                  PLONG plSegTag);
   
   LONG APIENTRY GpiCorrelateSegment(HPS hps,
                                     LONG lSegment,
                                     LONG lType,
                                     PPOINTL pptlPick,
                                     LONG lMaxHits,
                                     LONG lMaxDepth,
                                     PLONG alSegTag);
   
#endif /* non-common_GPICORRELATION */
#ifdef INCL_GPISEGMENTS
   
   /* data formats for GpiPutData and GpiGetData */
   #define DFORM_NOCONV                    0L
   
   #ifndef INCL_SAADEFS
      
      #define DFORM_S370SHORT                 1L
      #define DFORM_PCSHORT                   2L
      #define DFORM_PCLONG                    4L
      
   #endif /* no INCL_SAADEFS */
   
   /* segment attributes used by GpiSet/QuerySegmentAttrs and others */
   #define ATTR_ERROR                    (-1L)
   #define ATTR_DETECTABLE                 1L
   #define ATTR_VISIBLE                    2L
   #define ATTR_CHAINED                    6L
   
   #ifndef INCL_SAADEFS
      
      #define ATTR_DYNAMIC                    8L
      
   #endif /* no INCL_SAADEFS */
   
   #define ATTR_FASTCHAIN                  9L
   #define ATTR_PROP_DETECTABLE           10L
   #define ATTR_PROP_VISIBLE              11L
   
   /* attribute on/off values */
   #define ATTR_OFF                        0L
   #define ATTR_ON                         1L
   
   /* segment priority used by GpiSetSegmentPriority and others */
   #define LOWER_PRI                     (-1L)
   #define HIGHER_PRI                      1L
   
   /*** segment control functions */
   
   BOOL APIENTRY GpiOpenSegment(HPS hps,
                                LONG lSegment);
   
   BOOL APIENTRY GpiCloseSegment(HPS hps);
   
   BOOL APIENTRY GpiDeleteSegment(HPS hps,
                                  LONG lSegid);
   
   LONG APIENTRY GpiQueryInitialSegmentAttrs(HPS hps,
                                             LONG lAttribute);
   
   BOOL APIENTRY GpiSetInitialSegmentAttrs(HPS hps,
                                           LONG lAttribute,
                                           LONG lValue);
   
   LONG APIENTRY GpiQuerySegmentAttrs(HPS hps,
                                      LONG lSegid,
                                      LONG lAttribute);
   
   BOOL APIENTRY GpiSetSegmentAttrs(HPS hps,
                                    LONG lSegid,
                                    LONG lAttribute,
                                    LONG lValue);
   
   LONG APIENTRY GpiQuerySegmentPriority(HPS hps,
                                         LONG lRefSegid,
                                         LONG lOrder);
   
   BOOL APIENTRY GpiSetSegmentPriority(HPS hps,
                                       LONG lSegid,
                                       LONG lRefSegid,
                                       LONG lOrder);
   
   BOOL APIENTRY GpiDeleteSegments(HPS hps,
                                   LONG lFirstSegment,
                                   LONG lLastSegment);
   
   LONG APIENTRY GpiQuerySegmentNames(HPS hps,
                                      LONG lFirstSegid,
                                      LONG lLastSegid,
                                      LONG lMax,
                                      PLONG alSegids);
   
   /*** draw functions for segments */
   LONG APIENTRY GpiGetData(HPS hps,
                            LONG lSegid,
                            PLONG plOffset,
                            LONG lFormat,
                            LONG lLength,
                            PBYTE pbData);
   
   LONG APIENTRY GpiPutData(HPS hps,
                            LONG lFormat,
                            PLONG plCount,
                            PBYTE pbData);
   
   BOOL APIENTRY GpiDrawChain(HPS hps);
   
   BOOL APIENTRY GpiDrawFrom(HPS hps,
                             LONG lFirstSegment,
                             LONG lLastSegment);
   
   BOOL APIENTRY GpiDrawSegment(HPS hps,
                                LONG lSegment);
   
   #ifndef INCL_SAADEFS
      
      
      BOOL APIENTRY GpiDrawDynamics(HPS hps);
      
      BOOL APIENTRY GpiRemoveDynamics(HPS hps,
                                      LONG lFirstSegid,
                                      LONG lLastSegid);
      
   #endif /* no INCL_SAADEFS */
   
#endif /* non-common GPISEGMENTS */
#ifdef INCL_GPISEGEDITING
   
   /* edit modes used by GpiSet/QueryEditMode */
   #define SEGEM_ERROR                         0L
   #define SEGEM_INSERT                        1L
   #define SEGEM_REPLACE                       2L
   
   /*** segment editing by element functions */
   
#ifdef __cplusplus
   BOOL  APIENTRY GpiBeginElement(HPS hps,
                                  LONG lType,
                                  PCSZ  pszDesc);
#else
   BOOL  APIENTRY GpiBeginElement(HPS hps,
                                  LONG lType,
                                  PSZ pszDesc);
#endif
   
   BOOL  APIENTRY GpiEndElement(HPS hps);
   
   BOOL  APIENTRY GpiLabel(HPS hps,
                           LONG lLabel);
   
#ifdef __cplusplus
   LONG  APIENTRY GpiElement(HPS hps,
                             LONG lType,
                             PCSZ  pszDesc,
                             LONG lLength,
                             PBYTE pbData);
#else
   LONG  APIENTRY GpiElement(HPS hps,
                             LONG lType,
                             PSZ pszDesc,
                             LONG lLength,
                             PBYTE pbData);
#endif
   
   LONG  APIENTRY GpiQueryElement(HPS hps,
                                  LONG lOff,
                                  LONG lMaxLength,
                                  PBYTE pbData);
   
   BOOL  APIENTRY GpiDeleteElement(HPS hps);
   
   BOOL  APIENTRY GpiDeleteElementRange(HPS hps,
                                        LONG lFirstElement,
                                        LONG lLastElement);
   
   BOOL  APIENTRY GpiDeleteElementsBetweenLabels(HPS hps,
                                                 LONG lFirstLabel,
                                                 LONG lLastLabel);
   
   LONG  APIENTRY GpiQueryEditMode(HPS hps);
   
   BOOL  APIENTRY GpiSetEditMode(HPS hps,
                                 LONG lMode);
   
   LONG  APIENTRY GpiQueryElementPointer(HPS hps);
   
   BOOL  APIENTRY GpiSetElementPointer(HPS hps,
                                       LONG lElement);
   
   BOOL  APIENTRY GpiOffsetElementPointer(HPS hps,
                                          LONG loffset);
   
#ifdef __cplusplus
   LONG  APIENTRY GpiQueryElementType(HPS hps,
                                      PLONG plType,
                                      LONG lLength,
                                      PCSZ  pszData);
#else
   LONG  APIENTRY GpiQueryElementType(HPS hps,
                                      PLONG plType,
                                      LONG lLength,
                                      PSZ pszData);
#endif
   
   BOOL  APIENTRY GpiSetElementPointerAtLabel(HPS hps,
                                              LONG lLabel);
   
#endif /* non-common GPISEGEDITING */
#ifdef INCL_GPITRANSFORMS
   
   /* co-ordinates space for GpiConvert */
   #define CVTC_WORLD                      1L
   #define CVTC_MODEL                      2L
   #define CVTC_DEFAULTPAGE                3L
   #define CVTC_PAGE                       4L
   #define CVTC_DEVICE                     5L
   
   /* type of transformation for GpiSetSegmentTransformMatrix */
   #define TRANSFORM_REPLACE               0L
   #define TRANSFORM_ADD                   1L
   #define TRANSFORM_PREEMPT               2L
   
   #ifndef INCL_DDIDEFS
      
      
      /*** transform and transform conversion functions */
      BOOL  APIENTRY GpiQuerySegmentTransformMatrix(HPS hps,
                                                    LONG lSegid,
                                                    LONG lCount,
                                                    PMATRIXLF pmatlfArray);
      
      BOOL  APIENTRY GpiSetSegmentTransformMatrix(HPS hps,
                                                  LONG lSegid,
                                                  LONG lCount,
                                                  PMATRIXLF pmatlfarray,
                                                  LONG lOptions);
      
      BOOL  APIENTRY GpiConvert(HPS hps,
                                LONG lSrc,
                                LONG lTarg,
                                LONG lCount,
                                PPOINTL aptlPoints);
      
      BOOL  APIENTRY GpiConvertWithMatrix(HPS hps,
                                          LONG lCountp,
                                          PPOINTL aptlPoints,
                                          LONG lCount,
                                          PMATRIXLF pmatlfArray);
      
      BOOL  APIENTRY GpiQueryModelTransformMatrix(HPS hps,
                                                  LONG lCount,
                                                  PMATRIXLF pmatlfArray);
      
      BOOL  APIENTRY GpiSetModelTransformMatrix(HPS hps,
                                                LONG lCount,
                                                PMATRIXLF pmatlfArray,
                                                LONG lOptions);
      
      LONG  APIENTRY GpiCallSegmentMatrix(HPS hps,
                                          LONG lSegment,
                                          LONG lCount,
                                          PMATRIXLF pmatlfArray,
                                          LONG lOptions);
      
      BOOL  APIENTRY GpiQueryDefaultViewMatrix(HPS hps,
                                               LONG lCount,
                                               PMATRIXLF pmatlfArray);
      
      BOOL  APIENTRY GpiSetDefaultViewMatrix(HPS hps,
                                             LONG lCount,
                                             PMATRIXLF pmatlfarray,
                                             LONG lOptions);
      
      BOOL  APIENTRY GpiQueryPageViewport(HPS hps,
                                          PRECTL prclViewport);
      
      BOOL  APIENTRY GpiSetPageViewport(HPS hps,
                                        PRECTL prclViewport);
      
      BOOL  APIENTRY GpiQueryViewingTransformMatrix(HPS hps,
                                                    LONG lCount,
                                                    PMATRIXLF pmatlfArray);
      
      BOOL  APIENTRY GpiSetViewingTransformMatrix(HPS hps,
                                                  LONG lCount,
                                                  PMATRIXLF pmatlfArray,
                                                  LONG lOptions);
      
      /*** transform helper routines */
      BOOL APIENTRY GpiTranslate(HPS,
                                 PMATRIXLF,
                                 LONG,
                                 PPOINTL);
      
      BOOL APIENTRY GpiScale(HPS,
                             PMATRIXLF,
                             LONG,
                             PFIXED,
                             PPOINTL);
      
      BOOL APIENTRY GpiRotate(HPS,
                              PMATRIXLF,
                              LONG,
                              FIXED,
                              PPOINTL);
      
      /*** general clipping functions */
      BOOL APIENTRY GpiSetGraphicsField(HPS hps,
                                        PRECTL prclField);
      
      BOOL APIENTRY GpiQueryGraphicsField(HPS hps,
                                          PRECTL prclField);
      
      BOOL APIENTRY GpiSetViewingLimits(HPS hps,
                                        PRECTL prclLimits);
      
      BOOL APIENTRY GpiQueryViewingLimits(HPS hps,
                                          PRECTL prclLimits);
      
   #endif /* no INCL_DDIDEFS */
   
#endif /* non-common GPITRANSFORMS */
#ifdef INCL_GPIPATHS
   
   /* modes for GpiModifyPath */
   #define MPATH_STROKE                    6L
   
   /* modes for GpiFillPath */
   #define FPATH_ALTERNATE                 0L
   #define FPATH_WINDING                   2L
   #define FPATH_INCL                      0L
   #define FPATH_EXCL                      8L
   
   /* modes for GpiSetClipPath */
   #define SCP_ALTERNATE                   0L
   #define SCP_WINDING                     2L
   #define SCP_AND                         4L
   #define SCP_RESET                       0L
   #define SCP_INCL                        0L
   #define SCP_EXCL                        8L
   
   #ifndef INCL_DDIDEFS
      
      /*** Path and Clip Path functions */
      
      BOOL  APIENTRY GpiBeginPath(HPS hps,
                                  LONG lPath);
      
      BOOL  APIENTRY GpiEndPath(HPS hps);
      
      BOOL  APIENTRY GpiCloseFigure(HPS hps);
      
      BOOL  APIENTRY GpiModifyPath(HPS hps,
                                   LONG lPath,
                                   LONG lMode);
      
      LONG  APIENTRY GpiFillPath(HPS hps,
                                 LONG lPath,
                                 LONG lOptions);
      
      BOOL  APIENTRY GpiSetClipPath(HPS hps,
                                    LONG lPath,
                                    LONG lOptions);
      
      LONG  APIENTRY GpiOutlinePath(HPS hps,
                                    LONG lPath,
                                    LONG lOptions);
      
      HRGN  APIENTRY GpiPathToRegion(HPS GpiH,
                                     LONG lPath,
                                     LONG lOptions);
      
      #ifndef INCL_SAADEFS
         
         
         LONG  APIENTRY GpiStrokePath(HPS hps,
                                      LONG lPath,
                                      ULONG flOptions);
         
      #endif /* no INCL_SAADEFS */
      
   #endif /* no INCL_DDIDEFS */
   
#endif /* non-common GPIPATHS */
#ifdef INCL_GPILOGCOLORTABLE
   
   #ifndef INCL_GPIBITMAPS
      #define INCL_GPIBITMAPS
   #endif /* INCL_GPIBITMAPS */
   
   /* options for GpiCreateLogColorTable and others */
   #define LCOL_RESET                   0x0001L
   #define LCOL_REALIZABLE              0x0002L
   #define LCOL_PURECOLOR               0x0004L
   #define LCOL_OVERRIDE_DEFAULT_COLORS 0x0008L
   #define LCOL_REALIZED                0x0010L
   
   /* format of logical lColor table for GpiCreateLogColorTable and others */
   #define LCOLF_DEFAULT                   0L
   #define LCOLF_INDRGB                    1L
   #define LCOLF_CONSECRGB                 2L
   #define LCOLF_RGB                       3L
   #define LCOLF_PALETTE                   4L
   
   /* options for GpiQueryRealColors and others */
   #define LCOLOPT_REALIZED           0x0001L
   #define LCOLOPT_INDEX              0x0002L
   
   #ifndef INCL_SAADEFS
      
      /* return codes from GpiQueryLogColorTable to indicate it is in RGB mode */
      #define QLCT_ERROR                    (-1L)
      #define QLCT_RGB                      (-2L)
      
      /* GpiQueryLogColorTable index returned for colors not explicitly loaded */
      #define QLCT_NOTLOADED                (-1L)
      
   #endif /* no INCL_SAADEFS */
   
   /* return codes for GpiQueryColorData */
   #define QCD_LCT_FORMAT                  0L
   #define QCD_LCT_LOINDEX                 1L
   #define QCD_LCT_HIINDEX                 2L
   #define QCD_LCT_OPTIONS                 3L
   
   /* Palette manager return values */
   #define PAL_ERROR                     (-1L)
   
   /* color flags for GpiCreatePalette and others */
   #define PC_RESERVED                   0x01
   #define PC_EXPLICIT                   0x02
   #define PC_NOCOLLAPSE                 0x04
   
   #ifndef INCL_DDIDEFS
      
      /*** logical lColor table functions */
      
      BOOL  APIENTRY GpiCreateLogColorTable(HPS hps,
                                            ULONG flOptions,
                                            LONG lFormat,
                                            LONG lStart,
                                            LONG lCount,
                                            PLONG alTable);
      
      BOOL  APIENTRY GpiQueryColorData(HPS hps,
                                       LONG lCount,
                                       PLONG alArray);
      
      #ifndef INCL_SAADEFS
         
         
         LONG  APIENTRY GpiQueryLogColorTable(HPS hps,
                                              ULONG flOptions,
                                              LONG lStart,
                                              LONG lCount,
                                              PLONG alArray);
         
      #endif /* no INCL_SAADEFS */
      
      
      LONG  APIENTRY GpiQueryRealColors(HPS hps,
                                        ULONG flOptions,
                                        LONG lStart,
                                        LONG lCount,
                                        PLONG alColors);
      
      LONG  APIENTRY GpiQueryNearestColor(HPS hps,
                                          ULONG flOptions,
                                          LONG lRgbIn);
      
      LONG  APIENTRY GpiQueryColorIndex(HPS hps,
                                        ULONG flOptions,
                                        LONG lRgbColor);
      
      LONG  APIENTRY GpiQueryRGBColor(HPS hps,
                                      ULONG flOptions,
                                      LONG lColorIndex);
      
      #ifndef INCL_SAADEFS
         
         /*Palette manager functions */
         
         HPAL APIENTRY GpiCreatePalette(HAB hab,
                                        ULONG flOptions,
                                        ULONG ulFormat,
                                        ULONG ulCount,
                                        PULONG aulTable);
         
         BOOL APIENTRY GpiDeletePalette(HPAL hpal);
         
         HPAL APIENTRY GpiSelectPalette(HPS hps,
                                        HPAL hpal);
         
         LONG APIENTRY GpiAnimatePalette(HPAL hpal,
                                         ULONG ulFormat,
                                         ULONG ulStart,
                                         ULONG ulCount,
                                         PULONG aulTable);
         
         BOOL APIENTRY GpiSetPaletteEntries(HPAL hpal,
                                            ULONG ulFormat,
                                            ULONG ulStart,
                                            ULONG ulCount,
                                            PULONG aulTable);
         
         HPAL APIENTRY GpiQueryPalette(HPS hps);
         
         LONG APIENTRY GpiQueryPaletteInfo(HPAL hpal,
                                           HPS  hps,
                                           ULONG flOptions,
                                           ULONG ulStart,
                                           ULONG ulCount,
                                           PULONG aulArray);
         
         
         ULONG APIENTRY GpiResizePalette(HPAL hpal,
                                         ULONG newsize);
         
         LONG  APIENTRY GpiQueryNearestPaletteIndex(HPAL hpal,
                                                    ULONG color);
         
         
      #endif /* no INCL_SAADEFS */
      
   #endif /* no INCL_DDIDEFS */
   
#endif /* non-common GPILOGCOLORTABLE */
#if (defined(INCL_GPIPRIMITIVES) || !defined(INCL_NOCOMMON))
   
   /* default color table indices */
   
   #define CLR_FALSE                     (-5L)
   #define CLR_TRUE                      (-4L)
   
   #define CLR_ERROR                   (-255L)
   #define CLR_DEFAULT                   (-3L)
   #define CLR_WHITE                     (-2L)
   #define CLR_BLACK                     (-1L)
   #define CLR_BACKGROUND                  0L
   #define CLR_BLUE                        1L
   #define CLR_RED                         2L
   #define CLR_PINK                        3L
   #define CLR_GREEN                       4L
   #define CLR_CYAN                        5L
   #define CLR_YELLOW                      6L
   #define CLR_NEUTRAL                     7L
   
   #define CLR_DARKGRAY                    8L
   #define CLR_DARKBLUE                    9L
   #define CLR_DARKRED                    10L
   #define CLR_DARKPINK                   11L
   #define CLR_DARKGREEN                  12L
   #define CLR_DARKCYAN                   13L
   #define CLR_BROWN                      14L
   #define CLR_PALEGRAY                   15L
   
   /* rgb colors */
   #define RGB_ERROR                   (-255L)
   #define RGB_BLACK              0x00000000L
   #define RGB_BLUE               0x000000FFL
   #define RGB_GREEN              0x0000FF00L
   #define RGB_CYAN               0x0000FFFFL
   #define RGB_RED                0x00FF0000L
   #define RGB_PINK               0x00FF00FFL
   #define RGB_YELLOW             0x00FFFF00L
   #define RGB_WHITE              0x00FFFFFFL
   
   /* control flags used by GpiBeginArea */
   #define BA_NOBOUNDARY                   0L
   #define BA_BOUNDARY                0x0001L
   
   
   #define BA_ALTERNATE                    0L
   #define BA_WINDING                 0x0002L
   
   #define BA_INCL                         0L
   #define BA_EXCL                         8L
   
   
   /* fill options for GpiBox/GpiFullArc/GpiOval */
   #define DRO_FILL                        1L
   #define DRO_OUTLINE                     2L
   #define DRO_OUTLINEFILL                 3L
   #define DRO_EXCLUSIVE                   0x10000000 /* exclusive right and bottom in device */
   /* exclusive in         */
   /* device coordinate.   */
   
   /* basic pattern symbols */
   #define PATSYM_ERROR                  (-1L)
   #define PATSYM_DEFAULT                  0L
   #define PATSYM_DENSE1                   1L
   #define PATSYM_DENSE2                   2L
   #define PATSYM_DENSE3                   3L
   #define PATSYM_DENSE4                   4L
   #define PATSYM_DENSE5                   5L
   #define PATSYM_DENSE6                   6L
   #define PATSYM_DENSE7                   7L
   #define PATSYM_DENSE8                   8L
   #define PATSYM_VERT                     9L
   #define PATSYM_HORIZ                   10L
   #define PATSYM_DIAG1                   11L
   #define PATSYM_DIAG2                   12L
   #define PATSYM_DIAG3                   13L
   #define PATSYM_DIAG4                   14L
   #define PATSYM_NOSHADE                 15L
   #define PATSYM_SOLID                   16L
   #ifndef INCL_SAADEFS
      #define PATSYM_HALFTONE                17L
   #endif /* no INCL_SAADEFS */
   #define PATSYM_HATCH                   18L
   #define PATSYM_DIAGHATCH               19L
   #define PATSYM_BLANK                   64L
   
   /* lcid values for GpiSet/QueryPattern and others */
   #define LCID_ERROR                    (-1L)
   #define LCID_DEFAULT                    0L
   
   #ifndef INCL_DDIDEFS
      
      
      /*** global primitive functions */
      BOOL  APIENTRY GpiSetColor(HPS hps,
                                 LONG lColor);
      
      LONG  APIENTRY GpiQueryColor(HPS hps);
      
      /*** line primitive functions */
      
      LONG  APIENTRY GpiBox(HPS hps,
                            LONG lControl,
                            PPOINTL pptlPoint,
                            LONG lHRound,
                            LONG lVRound);
      
      BOOL  APIENTRY GpiMove(HPS hps,
                             PPOINTL pptlPoint);
      
      LONG  APIENTRY GpiLine(HPS hps,
                             PPOINTL pptlEndPoint);
      
      LONG  APIENTRY GpiPolyLine(HPS hps,
                                 LONG lCount,
                                 PPOINTL aptlPoints);
      
      LONG  APIENTRY GpiPolyLineDisjoint(HPS hps,
                                         LONG lCount,
                                         PPOINTL aptlPoints);
      
      /*** area primitive functions */
      BOOL  APIENTRY GpiSetPattern(HPS hps,
                                   LONG lPatternSymbol);
      
      LONG  APIENTRY GpiQueryPattern(HPS hps);
      
      BOOL  APIENTRY GpiBeginArea(HPS hps,
                                  ULONG flOptions);
      
      LONG  APIENTRY GpiEndArea(HPS hps);
      
      /*** character primitive functions */
      LONG  APIENTRY GpiCharString(HPS hps,
                                   LONG lCount,
                                   PCH pchString);
      
      LONG  APIENTRY GpiCharStringAt(HPS hps,
                                     PPOINTL pptlPoint,
                                     LONG lCount,
                                     PCH pchString);
      
   #endif /* no INCL_DDIDEFS */
   
#endif /* common GPIPRIMTIVES */
#ifdef INCL_GPIPRIMITIVES
   
   /* mode for GpiSetAttrMode */
   #define AM_ERROR                      (-1L)
   #define AM_PRESERVE                     0L
   #define AM_NOPRESERVE                   1L
   
   /* foreground mixes */
   #define FM_ERROR                      (-1L)
   #define FM_DEFAULT                      0L
   #define FM_OR                           1L
   #define FM_OVERPAINT                    2L
   #define FM_LEAVEALONE                   5L
   
   #define FM_XOR                          4L
   #define FM_AND                          6L
   #define FM_SUBTRACT                     7L
   #define FM_MASKSRCNOT                   8L
   #define FM_ZERO                         9L
   #define FM_NOTMERGESRC                 10L
   #define FM_NOTXORSRC                   11L
   #define FM_INVERT                      12L
   #define FM_MERGESRCNOT                 13L
   #define FM_NOTCOPYSRC                  14L
   #define FM_MERGENOTSRC                 15L
   #define FM_NOTMASKSRC                  16L
   #define FM_ONE                         17L
   
   
   /* background mixes */
   #define BM_ERROR                      (-1L)
   #define BM_DEFAULT                      0L
   #define BM_OR                           1L
   #define BM_OVERPAINT                    2L
   #define BM_LEAVEALONE                   5L
   
   #define BM_XOR                          4L
   #define BM_AND                          6L
   #define BM_SUBTRACT                     7L
   #define BM_MASKSRCNOT                   8L
   #define BM_ZERO                         9L
   #define BM_NOTMERGESRC                 10L
   #define BM_NOTXORSRC                   11L
   #define BM_INVERT                      12L
   #define BM_MERGESRCNOT                 13L
   #define BM_NOTCOPYSRC                  14L
   #define BM_MERGENOTSRC                 15L
   #define BM_NOTMASKSRC                  16L
   #define BM_ONE                         17L
   #define BM_SRCTRANSPARENT              18L
   #define BM_DESTTRANSPARENT             19L
   
   
   
   /* basic line type styles */
   #define LINETYPE_ERROR                (-1L)
   #define LINETYPE_DEFAULT                0L
   #define LINETYPE_DOT                    1L
   #define LINETYPE_SHORTDASH              2L
   #define LINETYPE_DASHDOT                3L
   #define LINETYPE_DOUBLEDOT              4L
   #define LINETYPE_LONGDASH               5L
   #define LINETYPE_DASHDOUBLEDOT          6L
   #define LINETYPE_SOLID                  7L
   #define LINETYPE_INVISIBLE              8L
   #ifndef INCL_SAADEFS
      #define LINETYPE_ALTERNATE              9L
   #endif /* no INCL_SAADEFS */
   
   /* cosmetic line widths */
   #define LINEWIDTH_ERROR               (-1L)
   #define LINEWIDTH_DEFAULT               0L
   #define LINEWIDTH_NORMAL       0x00010000L   /* MAKEFIXED(1,0) */
   #define LINEWIDTH_THICK        0x00020000L   /* MAKEFIXED(2,0) */
   
   /* actual line widths */
   #define LINEWIDTHGEOM_ERROR           (-1L)
   
   /* line end styles */
   #define LINEEND_ERROR                 (-1L)
   #define LINEEND_DEFAULT                 0L
   #define LINEEND_FLAT                    1L
   #define LINEEND_SQUARE                  2L
   #define LINEEND_ROUND                   3L
   
   /* line join styles */
   #define LINEJOIN_ERROR                (-1L)
   #define LINEJOIN_DEFAULT                0L
   #define LINEJOIN_BEVEL                  1L
   #define LINEJOIN_ROUND                  2L
   #define LINEJOIN_MITRE                  3L
   
   /* character directions */
   #define CHDIRN_ERROR                  (-1L)
   #define CHDIRN_DEFAULT                  0L
   #define CHDIRN_LEFTRIGHT                1L
   #define CHDIRN_TOPBOTTOM                2L
   #define CHDIRN_RIGHTLEFT                3L
   #define CHDIRN_BOTTOMTOP                4L
   
   /* character text alignments */
   #define TA_NORMAL_HORIZ     0x0001
   #define TA_LEFT             0x0002
   #define TA_CENTER           0x0003
   #define TA_RIGHT            0x0004
   #define TA_STANDARD_HORIZ   0x0005
   #define TA_NORMAL_VERT      0x0100
   #define TA_TOP              0x0200
   #define TA_HALF             0x0300
   #define TA_BASE             0x0400
   #define TA_BOTTOM           0x0500
   #define TA_STANDARD_VERT    0x0600
   
   /* character modes */
   #define CM_ERROR                      (-1L)
   #define CM_DEFAULT                      0L
   #define CM_MODE1                        1L
   #define CM_MODE2                        2L
   #define CM_MODE3                        3L
   
   /* basic marker symbols */
   #define MARKSYM_ERROR                 (-1L)
   #define MARKSYM_DEFAULT                 0L
   #define MARKSYM_CROSS                   1L
   #define MARKSYM_PLUS                    2L
   #define MARKSYM_DIAMOND                 3L
   #define MARKSYM_SQUARE                  4L
   #define MARKSYM_SIXPOINTSTAR            5L
   #define MARKSYM_EIGHTPOINTSTAR          6L
   #define MARKSYM_SOLIDDIAMOND            7L
   #define MARKSYM_SOLIDSQUARE             8L
   #define MARKSYM_DOT                     9L
   #define MARKSYM_SMALLCIRCLE            10L
   #define MARKSYM_BLANK                  64L
   
   /* formatting options for GpiCharStringPosAt */
   #define CHS_OPAQUE                 0x0001L
   #define CHS_VECTOR                 0x0002L
   #define CHS_LEAVEPOS               0x0008L
   #define CHS_CLIP                   0x0010L
   #define CHS_UNDERSCORE             0x0200L
   #define CHS_STRIKEOUT              0x0400L
   
   /* bundle codes for GpiSetAttributes and GpiQueryAttributes */
   #define PRIM_LINE                       1L
   #define PRIM_CHAR                       2L
   #define PRIM_MARKER                     3L
   #define PRIM_AREA                       4L
   #define PRIM_IMAGE                      5L
   
   /* line bundle mask bits */
   #define LBB_COLOR                  0x0001L
   #define LBB_BACK_COLOR             0x0002L
   #define LBB_MIX_MODE               0x0004L
   #define LBB_BACK_MIX_MODE          0x0008L
   #define LBB_WIDTH                  0x0010L
   #define LBB_GEOM_WIDTH             0x0020L
   #define LBB_TYPE                   0x0040L
   #define LBB_END                    0x0080L
   #define LBB_JOIN                   0x0100L
   
   /* character bundle mask bits */
   #define CBB_COLOR                  0x0001L
   #define CBB_BACK_COLOR             0x0002L
   #define CBB_MIX_MODE               0x0004L
   #define CBB_BACK_MIX_MODE          0x0008L
   #define CBB_SET                    0x0010L
   #define CBB_MODE                   0x0020L
   #define CBB_BOX                    0x0040L
   #define CBB_ANGLE                  0x0080L
   #define CBB_SHEAR                  0x0100L
   #define CBB_DIRECTION              0x0200L
   #define CBB_TEXT_ALIGN             0x0400L
   #define CBB_EXTRA                  0x0800L
   #define CBB_BREAK_EXTRA            0x1000L
   
   /* marker bundle mask bits */
   #define MBB_COLOR                  0x0001L
   #define MBB_BACK_COLOR             0x0002L
   #define MBB_MIX_MODE               0x0004L
   #define MBB_BACK_MIX_MODE          0x0008L
   #define MBB_SET                    0x0010L
   #define MBB_SYMBOL                 0x0020L
   #define MBB_BOX                    0x0040L
   
   /* pattern bundle mask bits */
   #define ABB_COLOR                  0x0001L
   #define ABB_BACK_COLOR             0x0002L
   #define ABB_MIX_MODE               0x0004L
   #define ABB_BACK_MIX_MODE          0x0008L
   #define ABB_SET                    0x0010L
   #define ABB_SYMBOL                 0x0020L
   #define ABB_REF_POINT              0x0040L
   
   /* image bundle mask bits */
   #define IBB_COLOR                  0x0001L
   #define IBB_BACK_COLOR             0x0002L
   #define IBB_MIX_MODE               0x0004L
   #define IBB_BACK_MIX_MODE          0x0008L
   
   /* structure for GpiSetArcParams and GpiQueryArcParams */
   typedef struct _ARCPARAMS       /* arcp */
   {
      LONG lP;
      LONG lQ;
      LONG lR;
      LONG lS;
   } ARCPARAMS;
   typedef ARCPARAMS *PARCPARAMS;
   
   /* variation of SIZE used for FIXEDs */
   typedef struct _SIZEF          /* sizfx */
   {
      FIXED cx;
      FIXED cy;
   } SIZEF;
   typedef SIZEF *PSIZEF;
   
   /* structure for gradient parameters e.g. for GpiSetCharAngle */
   typedef struct _GRADIENTL        /* gradl */
   {
      LONG x;
      LONG y;
   } GRADIENTL;
   typedef GRADIENTL *PGRADIENTL;
   
   /* line bundle for GpiSetAttributes and GpiQueryAttributes */
   typedef struct _LINEBUNDLE       /* lbnd */
   {
      LONG    lColor;
      LONG    lBackColor;
      USHORT  usMixMode;
      USHORT  usBackMixMode;
      FIXED   fxWidth;
      LONG    lGeomWidth;
      USHORT  usType;
      USHORT  usEnd;
      USHORT  usJoin;
      USHORT  usReserved;
   } LINEBUNDLE;
   typedef LINEBUNDLE *PLINEBUNDLE;
   
   /* character bundle for GpiSetAttributes and GpiQueryAttributes */
   typedef struct _CHARBUNDLE       /* cbnd */
   {
      LONG      lColor;
      LONG      lBackColor;
      USHORT    usMixMode;
      USHORT    usBackMixMode;
      USHORT    usSet;
      USHORT    usPrecision;
      SIZEF     sizfxCell;
      POINTL    ptlAngle;
      POINTL    ptlShear;
      USHORT    usDirection;
      USHORT    usTextAlign;
      FIXED     fxExtra;
      FIXED     fxBreakExtra;
   } CHARBUNDLE;
   typedef CHARBUNDLE *PCHARBUNDLE;
   
   /* marker bundle for GpiSetAttributes and GpiQueryAttributes */
   typedef struct _MARKERBUNDLE     /* mbnd */
   {
      LONG   lColor;
      LONG   lBackColor;
      USHORT usMixMode;
      USHORT usBackMixMode;
      USHORT usSet;
      USHORT usSymbol;
      SIZEF  sizfxCell;
   } MARKERBUNDLE;
   typedef MARKERBUNDLE *PMARKERBUNDLE;
   
   /* pattern bundle for GpiSetAttributes and GpiQueryAttributes */
   typedef struct _AREABUNDLE    /* pbnd */
   {
      LONG   lColor;
      LONG   lBackColor;
      USHORT usMixMode;
      USHORT usBackMixMode;
      USHORT usSet;
      USHORT usSymbol;
      POINTL ptlRefPoint ;
   } AREABUNDLE;
   typedef AREABUNDLE *PAREABUNDLE;
   
   /* image bundle for GpiSetAttributes and GpiQueryAttributes */
   typedef struct _IMAGEBUNDLE      /* ibmd */
   {
      LONG   lColor;
      LONG   lBackColor;
      USHORT usMixMode;
      USHORT usBackMixMode;
   } IMAGEBUNDLE;
   typedef IMAGEBUNDLE *PIMAGEBUNDLE;
   
   /* pointer to any bundle used by GpiSet/QueryAttrs */
   typedef PVOID PBUNDLE;
   
   /* array indices for GpiQueryTextBox */
   #define TXTBOX_TOPLEFT                  0L
   #define TXTBOX_BOTTOMLEFT               1L
   #define TXTBOX_TOPRIGHT                 2L
   #define TXTBOX_BOTTOMRIGHT              3L
   #define TXTBOX_CONCAT                   4L
   /* array count for GpiQueryTextBox */
   #define TXTBOX_COUNT                    5L
   
   /* return codes for GpiPtVisible */
   #define PVIS_ERROR                      0L
   #define PVIS_INVISIBLE                  1L
   #define PVIS_VISIBLE                    2L
   
   /* return codes for GpiRectVisible */
   #define RVIS_ERROR                      0L
   #define RVIS_INVISIBLE                  1L
   #define RVIS_PARTIAL                    2L
   #define RVIS_VISIBLE                    3L
   
   #ifndef INCL_DDIDEFS
      
      
      /*** attribute mode functions */
      BOOL  APIENTRY GpiSetAttrMode(HPS hps,
                                    LONG lMode);
      
      LONG  APIENTRY GpiQueryAttrMode(HPS hps);
      /*** bundle primitive functions */
      BOOL  APIENTRY GpiSetAttrs(HPS hps,
                                 LONG lPrimType,
                                 ULONG flAttrMask,
                                 ULONG flDefMask,
                                 PBUNDLE ppbunAttrs);
      #ifndef INCL_SAADEFS
         
         
         LONG  APIENTRY GpiQueryAttrs(HPS hps,
                                      LONG lPrimType,
                                      ULONG flAttrMask,
                                      PBUNDLE ppbunAttrs);
         
      #endif /* no INCL_SAADEFS */
      
      
      /*** global primitive functions */
      BOOL  APIENTRY GpiSetBackColor(HPS hps,
                                     LONG lColor);
      
      LONG  APIENTRY GpiQueryBackColor(HPS hps);
      
      BOOL  APIENTRY GpiSetMix(HPS hps,
                               LONG lMixMode);
      
      LONG  APIENTRY GpiQueryMix(HPS hps);
      
      BOOL  APIENTRY GpiSetBackMix(HPS hps,
                                   LONG lMixMode);
      
      LONG  APIENTRY GpiQueryBackMix(HPS hps);
      
      /*** line primitive functions */
      BOOL  APIENTRY GpiSetLineType(HPS hps,
                                    LONG lLineType);
      
      LONG  APIENTRY GpiQueryLineType(HPS hps);
      
      BOOL  APIENTRY GpiSetLineWidth(HPS hps,
                                     FIXED fxLineWidth);
      
      FIXED APIENTRY GpiQueryLineWidth(HPS hps);
      
      BOOL  APIENTRY GpiSetLineWidthGeom(HPS hps,
                                         LONG lLineWidth);
      
      LONG  APIENTRY GpiQueryLineWidthGeom(HPS hps);
      
      BOOL  APIENTRY GpiSetLineEnd(HPS hps,
                                   LONG lLineEnd);
      
      LONG  APIENTRY GpiQueryLineEnd(HPS hps);
      
      BOOL  APIENTRY GpiSetLineJoin(HPS hps,
                                    LONG lLineJoin);
      
      LONG  APIENTRY GpiQueryLineJoin(HPS hps);
      
      BOOL  APIENTRY GpiSetCurrentPosition(HPS hps,
                                           PPOINTL pptlPoint);
      
      BOOL  APIENTRY GpiQueryCurrentPosition(HPS hps,
                                             PPOINTL pptlPoint);
      
      /*** arc primitive functions */
      BOOL  APIENTRY GpiSetArcParams(HPS hps,
                                     PARCPARAMS parcpArcParams);
      
      BOOL  APIENTRY GpiQueryArcParams(HPS hps,
                                       PARCPARAMS parcpArcParams);
      
      LONG  APIENTRY GpiPointArc(HPS hps,
                                 PPOINTL pptl2);
      
      LONG  APIENTRY GpiFullArc(HPS hps,
                                LONG lControl,
                                FIXED fxMultiplier);
      
      LONG  APIENTRY GpiPartialArc(HPS hps,
                                   PPOINTL pptlCenter,
                                   FIXED fxMultiplier,
                                   FIXED fxStartAngle,
                                   FIXED fxSweepAngle);
      
      LONG  APIENTRY GpiOval(HPS hps,
                             PPOINTL aptlPoints,
                             FIXED fxStartAngle,
                             FIXED fxSweepAngle,
                             ULONG ulOptions);
      
      LONG  APIENTRY GpiPolyFillet(HPS hps,
                                   LONG lCount,
                                   PPOINTL aptlPoints);
      
      LONG  APIENTRY GpiPolySpline(HPS hps,
                                   LONG lCount,
                                   PPOINTL aptlPoints);
      
      LONG  APIENTRY GpiPolyFilletSharp(HPS hps,
                                        LONG lCount,
                                        PPOINTL aptlPoints,
                                        PFIXED afxPoints);
      
      /*** area primitive functions */
      BOOL  APIENTRY GpiSetPatternSet(HPS hps,
                                      LONG lSet);
      
      LONG  APIENTRY GpiQueryPatternSet(HPS hps);
      
      BOOL  APIENTRY GpiSetPatternRefPoint(HPS hps,
                                           PPOINTL pptlRefPoint);
      
      BOOL  APIENTRY GpiQueryPatternRefPoint(HPS hps,
                                             PPOINTL pptlRefPoint);
      
      /*** character primitive functions */
      BOOL  APIENTRY GpiQueryCharStringPos(HPS hps,
                                           ULONG flOptions,
                                           LONG lCount,
                                           PCH pchString,
                                           PLONG alXincrements,
                                           PPOINTL aptlPositions);
      
      BOOL  APIENTRY GpiQueryCharStringPosAt(HPS hps,
                                             PPOINTL pptlStart,
                                             ULONG flOptions,
                                             LONG lCount,
                                             PCH pchString,
                                             PLONG alXincrements,
                                             PPOINTL aptlPositions);
      
      LONG APIENTRY GpiQueryTabbedTextExtent(HPS hps,
                                             LONG lCount,
                                             PCH pchString,
                                             LONG lTabCount,
                                             PULONG puTabStops);
      
      BOOL  APIENTRY GpiQueryTextBox(HPS hps,
                                     LONG lCount1,
                                     PCH pchString,
                                     LONG lCount2,
                                     PPOINTL aptlPoints);
      
      BOOL  APIENTRY GpiQueryDefCharBox(HPS hps,
                                        PSIZEL psizlSize);
      
      BOOL  APIENTRY GpiSetCharSet(HPS hps,
                                   LONG llcid);
      
      LONG  APIENTRY GpiQueryCharSet(HPS hps);
      
      BOOL  APIENTRY GpiSetCharBox(HPS hps,
                                   PSIZEF psizfxBox);
      
      BOOL  APIENTRY GpiQueryCharBox(HPS hps,
                                     PSIZEF psizfxSize);
      
      BOOL  APIENTRY GpiSetCharAngle(HPS hps,
                                     PGRADIENTL pgradlAngle);
      
      BOOL  APIENTRY GpiQueryCharAngle(HPS hps,
                                       PGRADIENTL pgradlAngle);
      
      BOOL  APIENTRY GpiSetCharShear(HPS hps,
                                     PPOINTL pptlAngle);
      
      BOOL  APIENTRY GpiQueryCharShear(HPS hps,
                                       PPOINTL pptlShear);
      
      BOOL  APIENTRY GpiSetCharDirection(HPS hps,
                                         LONG lDirection);
      
      LONG  APIENTRY GpiQueryCharDirection(HPS hps);
      
      BOOL  APIENTRY GpiSetCharMode(HPS hps,
                                    LONG lMode);
      
      LONG  APIENTRY GpiQueryCharMode(HPS hps);
      
      BOOL  APIENTRY GpiSetTextAlignment(HPS hps,
                                         LONG lHoriz,
                                         LONG lVert);
      
      BOOL  APIENTRY GpiQueryTextAlignment(HPS hps,
                                           PLONG plHoriz,
                                           PLONG plVert);
      
      LONG  APIENTRY GpiCharStringPos(HPS hps,
                                      PRECTL prclRect,
                                      ULONG flOptions,
                                      LONG lCount,
                                      PCH pchString,
                                      PLONG alAdx);
      
      LONG APIENTRY GpiTabbedCharStringAt(HPS hps,
                                          PPOINTL pPtStart,
                                          PRECTL prclRect,
                                          ULONG flOptions,
                                          LONG lCount,
                                          PCH pchString,
                                          LONG lTabCount,
                                          PULONG puTabStops,
                                          LONG lTabOrigin);
      
      LONG  APIENTRY GpiCharStringPosAt(HPS hps,
                                        PPOINTL pptlStart,
                                        PRECTL prclRect,
                                        ULONG flOptions,
                                        LONG lCount,
                                        PCH pchString,
                                        PLONG alAdx);
      
      BOOL  APIENTRY GpiSetCharExtra(HPS hps,
                                     FIXED  Extra);
      
      BOOL  APIENTRY GpiSetCharBreakExtra(HPS hps,
                                          FIXED  BreakExtra);
      
      BOOL  APIENTRY GpiQueryCharExtra(HPS hps,
                                       PFIXED Extra);
      
      BOOL  APIENTRY GpiQueryCharBreakExtra(HPS hps,
                                            PFIXED BreakExtra);
      
      /*** marker primitive functions  */
      LONG  APIENTRY GpiMarker(HPS hps,
                               PPOINTL pptlPoint);
      
      LONG  APIENTRY GpiPolyMarker(HPS hps,
                                   LONG lCount,
                                   PPOINTL aptlPoints);
      
      BOOL  APIENTRY GpiSetMarker(HPS hps,
                                  LONG lSymbol);
      
      BOOL  APIENTRY GpiSetMarkerBox(HPS hps,
                                     PSIZEF psizfxSize);
      
      BOOL  APIENTRY GpiSetMarkerSet(HPS hps,
                                     LONG lSet);
      
      LONG  APIENTRY GpiQueryMarker(HPS hps);
      
      BOOL  APIENTRY GpiQueryMarkerBox(HPS hps,
                                       PSIZEF psizfxSize);
      
      LONG  APIENTRY GpiQueryMarkerSet(HPS hps);
      
      /*** image primitive functions */
      LONG  APIENTRY GpiImage(HPS hps,
                              LONG lFormat,
                              PSIZEL psizlImageSize,
                              LONG lLength,
                              PBYTE pbData);
      
      /*** miscellaneous primitive functions */
      BOOL  APIENTRY GpiPop(HPS hps,
                            LONG lCount);
      
      LONG  APIENTRY GpiPtVisible(HPS hps,
                                  PPOINTL pptlPoint);
      
      LONG  APIENTRY GpiRectVisible(HPS hps,
                                    PRECTL prclRectangle);
      
      BOOL  APIENTRY GpiComment(HPS hps,
                                LONG lLength,
                                PBYTE pbData);
      
   #endif /* no INCL_DDIDEFS */
   
#endif /* non-common GPIPRIMITIVES */
#ifdef INCL_GPILCIDS
   
   /* return codes from GpiCreateLogFont */
   #define FONT_DEFAULT                    1L
   #define FONT_MATCH                      2L
   
   /* lcid type for GpiQuerySetIds */
   #define LCIDT_FONT                      6L
   
   #define LCIDT_BITMAP                    7L
   
   /* constant used to delete all lcids by GpiDeleteSetId */
   #define LCID_ALL                      (-1L)
   
   /* kerning data returned by GpiQueryKerningPairs */
   typedef struct _KERNINGPAIRS     /* krnpr */
   {
      SHORT sFirstChar;
      SHORT sSecondChar;
      LONG  lKerningAmount;
   } KERNINGPAIRS;
   typedef KERNINGPAIRS *PKERNINGPAIRS;
   
   #define RC_ATMAVAIL       1
   #define RC_ATMENABLED     1
   #define RC_TTAVAIL        2
   #define RC_TTENABLED      2
   typedef struct _RASTERIZERCAPS    /* ffdescs2 */
   {
      ULONG  cb;
      ULONG  ulAvailFlags;
      ULONG  ulEnabledFlags;
   } RASTERIZERCAPS;
   typedef RASTERIZERCAPS *PRASTERIZERCAPS;
   
   BOOL APIENTRY GpiQueryRasterizerCaps(PRASTERIZERCAPS);
   
   /* data required by GpiQueryFaceString */
   typedef struct _FACENAMEDESC     /* fnd */
   {
      USHORT usSize;
      USHORT usWeightClass;
      USHORT usWidthClass;
      USHORT usReserved;
      ULONG  flOptions;
   } FACENAMEDESC;
   typedef FACENAMEDESC *PFACENAMEDESC;
   
   /* FACENAMEDESC 'WeightClass' options for GpiQueryFaceString */
   #define FWEIGHT_DONT_CARE      0L
   #define FWEIGHT_ULTRA_LIGHT    1L
   #define FWEIGHT_EXTRA_LIGHT    2L
   #define FWEIGHT_LIGHT          3L
   #define FWEIGHT_SEMI_LIGHT     4L
   #define FWEIGHT_NORMAL         5L
   #define FWEIGHT_SEMI_BOLD      6L
   #define FWEIGHT_BOLD           7L
   #define FWEIGHT_EXTRA_BOLD     8L
   #define FWEIGHT_ULTRA_BOLD     9L
   
   /* FACENAMEDESC 'WidthClass' options for GpiQueryFaceString */
   #define FWIDTH_DONT_CARE       0L
   #define FWIDTH_ULTRA_CONDENSED 1L
   #define FWIDTH_EXTRA_CONDENSED 2L
   #define FWIDTH_CONDENSED       3L
   #define FWIDTH_SEMI_CONDENSED  4L
   #define FWIDTH_NORMAL          5L
   #define FWIDTH_SEMI_EXPANDED   6L
   #define FWIDTH_EXPANDED        7L
   #define FWIDTH_EXTRA_EXPANDED  8L
   #define FWIDTH_ULTRA_EXPANDED  9L
   
   /* FACENAMEDESC 'options' for GpiQueryFaceString */
   #define FTYPE_ITALIC            0x0001
   #define FTYPE_ITALIC_DONT_CARE  0x0002
   #define FTYPE_OBLIQUE           0x0004
   #define FTYPE_OBLIQUE_DONT_CARE 0x0008
   #define FTYPE_ROUNDED           0x0010
   #define FTYPE_ROUNDED_DONT_CARE 0x0020
   
   /* actions for GpiQueryFontAction */
   #define QFA_PUBLIC      1L
   #define QFA_PRIVATE     2L
   #define QFA_ERROR       GPI_ALTERROR
   
   /* options for GpiQueryFonts */
   #define QF_PUBLIC                  0x0001L
   #define QF_PRIVATE                 0x0002L
   #define QF_NO_GENERIC              0x0004L
   #define QF_NO_DEVICE               0x0008L
   
   /* options for GpiSetFontMatchFlags */
   #define FONT_IGNORE_FONTRES             1L
   #define FONT_MATCH_NEAREST              2L
   
   #ifndef INCL_SAADEFS
      
      /* font file descriptions for GpiQueryFontFileDescriptions */
      typedef CHAR FFDESCS[2][FACESIZE]; /* ffdescs */
      typedef FFDESCS *PFFDESCS;
      typedef struct _FFDESCS2    /* ffdescs2 */
      {
         ULONG  cbLength;
         ULONG  cbFacenameOffset;
         BYTE   abFamilyName[1];
      } FFDESCS2;
      typedef FFDESCS2 *PFFDESCS2;
      
   #endif /* no INCL_SAADEFS */
   
   #ifndef INCL_DDIDEFS
      
      /*** physical and logical font functions */
      
      LONG  APIENTRY GpiCreateLogFont(HPS hps,
                                      PSTR8 pName,
                                      LONG lLcid,
                                      PFATTRS pfatAttrs);
      
      BOOL  APIENTRY GpiDeleteSetId(HPS hps,
                                    LONG lLcid);
      
#ifdef __cplusplus
      BOOL  APIENTRY GpiLoadFonts(HAB hab,
                                  PCSZ  pszFilename);
#else
      BOOL  APIENTRY GpiLoadFonts(HAB hab,
                                  PSZ pszFilename);
#endif
      
#ifdef __cplusplus
      BOOL  APIENTRY GpiUnloadFonts(HAB hab,
                                    PCSZ  pszFilename);
#else
      BOOL  APIENTRY GpiUnloadFonts(HAB hab,
                                    PSZ pszFilename);
#endif
      
#ifdef __cplusplus
      LONG  APIENTRY GpiQueryFonts(HPS hps,
                                   ULONG flOptions,
                                   PCSZ  pszFacename,
                                   PLONG plReqFonts,
                                   LONG lMetricsLength,
                                   PFONTMETRICS afmMetrics);
#else
      LONG  APIENTRY GpiQueryFonts(HPS hps,
                                   ULONG flOptions,
                                   PSZ pszFacename,
                                   PLONG plReqFonts,
                                   LONG lMetricsLength,
                                   PFONTMETRICS afmMetrics);
#endif
      
      BOOL  APIENTRY GpiQueryFontMetrics(HPS hps,
                                         LONG lMetricsLength,
                                         PFONTMETRICS pfmMetrics);
      
      LONG  APIENTRY GpiQueryKerningPairs(HPS hps,
                                          LONG lCount,
                                          PKERNINGPAIRS akrnprData);
      
      BOOL  APIENTRY GpiQueryWidthTable(HPS hps,
                                        LONG lFirstChar,
                                        LONG lCount,
                                        PLONG alData);
      
      LONG  APIENTRY GpiQueryNumberSetIds(HPS hps);
      
      BOOL  APIENTRY GpiQuerySetIds(HPS hps,
                                    LONG lCount,
                                    PLONG alTypes,
                                    PSTR8 aNames,
                                    PLONG allcids);
      
#ifdef __cplusplus
      ULONG APIENTRY GpiQueryFaceString(HPS PS,
                                        PCSZ  FamilyName,
                                        PFACENAMEDESC attrs,
                                        LONG length,
                                        PCSZ  CompoundFaceName);
#else
      ULONG APIENTRY GpiQueryFaceString(HPS PS,
                                        PSZ FamilyName,
                                        PFACENAMEDESC attrs,
                                        LONG length,
                                        PSZ CompoundFaceName);
#endif
      
      BOOL  APIENTRY GpiQueryLogicalFont(HPS PS,
                                         LONG lcid,
                                         PSTR8 name,
                                         PFATTRS attrs,
                                         LONG length);
      
      ULONG APIENTRY GpiQueryFontAction(HAB anchor,
                                        ULONG options);
      
#ifdef __cplusplus
      BOOL  APIENTRY GpiLoadPublicFonts(HAB,
                                        PCSZ );
#else
      BOOL  APIENTRY GpiLoadPublicFonts(HAB,
                                        PSZ);
#endif
      
#ifdef __cplusplus
      BOOL  APIENTRY GpiUnloadPublicFonts(HAB,
                                          PCSZ );
#else
      BOOL  APIENTRY GpiUnloadPublicFonts(HAB,
                                          PSZ);
#endif
      
      #ifndef INCL_SAADEFS
         
         BOOL    APIENTRY GpiSetCp(HPS hps,
                                   ULONG ulCodePage);
         
         ULONG   APIENTRY GpiQueryCp(HPS hps);
         
#ifdef __cplusplus
         LONG    APIENTRY GpiQueryFullFontFileDescs(HAB hab,
                                                    PCSZ  pszFilename,
                                                    PLONG plCount,
                                                    PVOID pNames,
                                                    PLONG plNamesBuffLength);
#else
         LONG    APIENTRY GpiQueryFullFontFileDescs(HAB hab,
                                                    PSZ pszFilename,
                                                    PLONG plCount,
                                                    PVOID pNames,
                                                    PLONG plNamesBuffLength);
#endif
         
         ULONG   APIENTRY GpiSetFontMappingFlags(HPS   hps,
                                                 ULONG flags);
         
         ULONG   APIENTRY GpiQueryFontMappingFlags(HPS   hps);
         
#ifdef __cplusplus
         LONG    APIENTRY GpiQueryFontFileDescriptions(HAB hab,
                                                       PCSZ  pszFilename,
                                                       PLONG plCount,
                                                       PFFDESCS affdescsNames);
#else
         LONG    APIENTRY GpiQueryFontFileDescriptions(HAB hab,
                                                       PSZ pszFilename,
                                                       PLONG plCount,
                                                       PFFDESCS affdescsNames);
#endif
         
      #endif /* no INCL_SAADEFS */
      
   #endif /* no INCL_DDIDEFS */
   
#endif /* non-common GPILCIDS */

#if (defined(INCL_GPIBITMAPS) || !defined(INCL_NOCOMMON))
   /* raster operations defined for GpiBitBlt */
   #define ROP_SRCCOPY                0x00CCL
   #define ROP_SRCPAINT               0x00EEL
   #define ROP_SRCAND                 0x0088L
   #define ROP_SRCINVERT              0x0066L
   #define ROP_SRCERASE               0x0044L
   #define ROP_NOTSRCCOPY             0x0033L
   #define ROP_NOTSRCERASE            0x0011L
   #define ROP_MERGECOPY              0x00C0L
   #define ROP_MERGEPAINT             0x00BBL
   #define ROP_PATCOPY                0x00F0L
   #define ROP_PATPAINT               0x00FBL
   #define ROP_PATINVERT              0x005AL
   #define ROP_DSTINVERT              0x0055L
   #define ROP_ZERO                   0x0000L
   #define ROP_ONE                    0x00FFL
   
   /* Blt options for GpiBitBlt */
   #define BBO_OR                          0L
   #define BBO_AND                         1L
   #define BBO_IGNORE                      2L
   #define BBO_PAL_COLORS                  4L
   #define BBO_NO_COLOR_INFO               8L
   
   /* Fill options for GpiFloodFill */
   #define FF_BOUNDARY                     0L
   #define FF_SURFACE                      1L
   
   /* error return for GpiSetBitmap */
   #define HBM_ERROR            ((HBITMAP)-1L)
   
   #ifndef INCL_DDIDEFS
      /*** bitmap and pel functions */
      
      LONG     APIENTRY GpiBitBlt(HPS hpsTarget,
                                  HPS hpsSource,
                                  LONG lCount,
                                  PPOINTL aptlPoints,
                                  LONG lRop,
                                  ULONG flOptions);
      
      BOOL     APIENTRY GpiDeleteBitmap(HBITMAP hbm);
      
      HBITMAP  APIENTRY GpiLoadBitmap(HPS hps,
                                      HMODULE Resource,
                                      ULONG idBitmap,
                                      LONG lWidth,
                                      LONG lHeight);
      
      HBITMAP  APIENTRY GpiSetBitmap(HPS hps,
                                     HBITMAP hbm);
      
      LONG     APIENTRY GpiWCBitBlt(HPS hpsTarget,
                                    HBITMAP hbmSource,
                                    LONG lCount,
                                    PPOINTL aptlPoints,
                                    LONG lRop,
                                    ULONG flOptions);
      
   #endif /* no INCL_DDIDEFS */
   
#endif /* common GPIBITMAPS */

#ifdef INCL_GPIBITMAPS
   
   /*** include bitmap structures and file formats *************************/
   #include <pmbitmap.h>
   
   /* usage flags for GpiCreateBitmap */
   #define CBM_INIT        0x0004L
   
   /* error return code for GpiSet/QueryBitmapBits */
   #define BMB_ERROR                     (-1L)
   
   #ifndef INCL_DDIDEFS
      
      /*** bitmap and pel functions */
      
      HBITMAP APIENTRY GpiCreateBitmap(HPS hps,
                                       PBITMAPINFOHEADER2 pbmpNew,
                                       ULONG flOptions,
                                       PBYTE pbInitData,
                                       PBITMAPINFO2 pbmiInfoTable);
      
      LONG    APIENTRY GpiSetBitmapBits(HPS hps,
                                        LONG lScanStart,
                                        LONG lScans,
                                        PBYTE pbBuffer,
                                        PBITMAPINFO2 pbmiInfoTable);
      
      BOOL    APIENTRY GpiSetBitmapDimension(HBITMAP hbm,
                                             PSIZEL psizlBitmapDimension);
      
      BOOL    APIENTRY GpiSetBitmapId(HPS hps,
                                      HBITMAP hbm,
                                      LONG lLcid);
      
      LONG    APIENTRY GpiQueryBitmapBits(HPS hps,
                                          LONG lScanStart,
                                          LONG lScans,
                                          PBYTE pbBuffer,
                                          PBITMAPINFO2 pbmiInfoTable);
      
      BOOL    APIENTRY GpiQueryBitmapDimension(HBITMAP hbm,
                                               PSIZEL psizlBitmapDimension);
      
      HBITMAP APIENTRY GpiQueryBitmapHandle(HPS hps,
                                            LONG lLcid);
      
      BOOL    APIENTRY GpiQueryBitmapParameters(HBITMAP hbm,
                                                PBITMAPINFOHEADER pbmpData);
      
      BOOL    APIENTRY GpiQueryBitmapInfoHeader(HBITMAP hbm,
                                                PBITMAPINFOHEADER2 pbmpData);
      
      BOOL    APIENTRY GpiQueryDeviceBitmapFormats(HPS hps,
                                                   LONG lCount,
                                                   PLONG alArray);
      
      LONG    APIENTRY GpiSetPel(HPS hps,
                                 PPOINTL pptlPoint);
      
      LONG    APIENTRY GpiQueryPel(HPS hps,
                                   PPOINTL pptlPoint);
      
      LONG    APIENTRY GpiFloodFill(HPS hps,
                                    LONG lOptions,
                                    LONG lColor);
      
      LONG    APIENTRY GpiDrawBits(HPS hps,
                                   PVOID pBits,
                                   PBITMAPINFO2 pbmiInfoTable,
                                   LONG lCount,
                                   PPOINTL aptlPoints,
                                   LONG lRop,
                                   ULONG flOptions);
      
   #endif /* no INCL_DDIDEFS */
   
   
#endif /* INCL_GPIBITMAPS */

#ifdef INCL_GPIREGIONS
   
   /* options for GpiCombineRegion */
   #define CRGN_OR                         1L
   #define CRGN_COPY                       2L
   #define CRGN_XOR                        4L
   #define CRGN_AND                        6L
   #define CRGN_DIFF                       7L
   
   /* usDirection of returned region data for GpiQueryRegionRects */
   #define RECTDIR_LFRT_TOPBOT             1L
   #define RECTDIR_RTLF_TOPBOT             2L
   #define RECTDIR_LFRT_BOTTOP             3L
   #define RECTDIR_RTLF_BOTTOP             4L
   
   /* control data for GpiQueryRegionRects */
   typedef struct _RGNRECT          /* rgnrc */
   {
      ULONG  ircStart;
      ULONG  crc;
      ULONG  crcReturned;
      ULONG  ulDirection;
   } RGNRECT;
   typedef RGNRECT *PRGNRECT;
   
   /* return code to indicate type of region for GpiCombineRegion and others */
   #define RGN_ERROR                       0L
   #define RGN_NULL                        1L
   #define RGN_RECT                        2L
   #define RGN_COMPLEX                     3L
   
   /* return codes for GpiPtInRegion */
   #define PRGN_ERROR                      0L
   #define PRGN_OUTSIDE                    1L
   #define PRGN_INSIDE                     2L
   
   /* return codes for GpiRectInRegion */
   #define RRGN_ERROR                      0L
   #define RRGN_OUTSIDE                    1L
   #define RRGN_PARTIAL                    2L
   #define RRGN_INSIDE                     3L
   
   /* return codes for GpiEqualRegion */
   #define EQRGN_ERROR                     0L
   #define EQRGN_NOTEQUAL                  1L
   #define EQRGN_EQUAL                     2L
   
   /* error return code for GpiSetRegion */
   #define HRGN_ERROR              ((HRGN)-1L)
   
   #ifndef INCL_DDIDEFS
      
      
      /*** main region functions */
      LONG  APIENTRY GpiCombineRegion(HPS hps,
                                      HRGN hrgnDest,
                                      HRGN hrgnSrc1,
                                      HRGN hrgnSrc2,
                                      LONG lMode);
      
      HRGN  APIENTRY GpiCreateRegion(HPS hps,
                                     LONG lCount,
                                     PRECTL arclRectangles);
      
      BOOL  APIENTRY GpiDestroyRegion(HPS hps,
                                      HRGN hrgn);
      
      LONG  APIENTRY GpiEqualRegion(HPS hps,
                                    HRGN hrgnSrc1,
                                    HRGN hrgnSrc2);
      
      BOOL  APIENTRY GpiOffsetRegion(HPS hps,
                                     HRGN Hrgn,
                                     PPOINTL pptlOffset);
      
      LONG  APIENTRY GpiPaintRegion(HPS hps,
                                    HRGN hrgn);
      
      LONG  APIENTRY GpiFrameRegion(HPS hps,
                                    HRGN hrgn,
                                    PSIZEL thickness);
      
      LONG  APIENTRY GpiPtInRegion(HPS hps,
                                   HRGN hrgn,
                                   PPOINTL pptlPoint);
      
      LONG  APIENTRY GpiQueryRegionBox(HPS hps,
                                       HRGN hrgn,
                                       PRECTL prclBound);
      
      BOOL  APIENTRY GpiQueryRegionRects(HPS hps,
                                         HRGN hrgn,
                                         PRECTL prclBound,
                                         PRGNRECT prgnrcControl,
                                         PRECTL prclRect);
      
      LONG  APIENTRY GpiRectInRegion(HPS hps,
                                     HRGN hrgn,
                                     PRECTL prclRect);
      
      BOOL  APIENTRY GpiSetRegion(HPS hps,
                                  HRGN hrgn,
                                  LONG lcount,
                                  PRECTL arclRectangles);
      
      /*** clip region functions */
      LONG  APIENTRY GpiSetClipRegion(HPS hps,
                                      HRGN hrgn,
                                      PHRGN phrgnOld);
      
      #ifndef INCL_SAADEFS
         
         
         HRGN  APIENTRY GpiQueryClipRegion(HPS hps);
         
         LONG  APIENTRY GpiQueryClipBox(HPS hps,
                                        PRECTL prclBound);
         
      #endif /* no INCL_SAADEFS */
      
      
      LONG  APIENTRY GpiExcludeClipRectangle(HPS hps,
                                             PRECTL prclRectangle);
      
      LONG  APIENTRY GpiIntersectClipRectangle(HPS hps,
                                               PRECTL prclRectangle);
      
      #ifndef INCL_SAADEFS
         
         
         LONG  APIENTRY GpiOffsetClipRegion(HPS hps,
                                            PPOINTL pptlPoint);
         
      #endif /* no INCL_SAADEFS */
      
      
      HRGN APIENTRY GpiCreateRoundRectRegion(HPS hps,
                                             PPOINTL pptlPoint,
                                             LONG lHround,
                                             LONG lVRound);
      
      HRGN APIENTRY GpiCreateEllipticRegion(HPS hps,
                                            PRECTL prclRect);
      
   #endif /* no INCL_DDIDEFS */
   
#endif /* non-common GPIREGIONS */
#ifdef INCL_GPIMETAFILES
   
   #ifndef INCL_DDIDEFS
      
      /* constants for index values of options array for GpiPlayMetaFile */
      #define PMF_SEGBASE                     0
      #define PMF_LOADTYPE                    1
      #define PMF_RESOLVE                     2
      #define PMF_LCIDS                       3
      #define PMF_RESET                       4
      #define PMF_SUPPRESS                    5
      #define PMF_COLORTABLES                 6
      #define PMF_COLORREALIZABLE             7
      #define PMF_DEFAULTS                    8
      #define PMF_DELETEOBJECTS               9
      #define PMF_PERPAGEINFOPTR             12
      
      
      #define PPI_PLAYSEQ               0       /* Play a specified range of pages */
      #define PPI_PLAYDEFINED           1       /* Play each specific page */
      
      /* structure for per-page metafiling               */
      typedef struct  _PERPAGEINFO              /* ppinf */	
      {
         ULONG         ulType;     /* Play-Type                                */
         ULONG         ulStart;    /* Start of range                           */
         ULONG         ulEnd;      /* End of range ( range can be descending ) */
         ULONG         ulSkip;     /* Skip count in range value                */
         ULONG         ulPageCnt;  /* Count of  individual pages               */
         PULONG        pulDefined; /* Pointer to individual page numbers       */
      } PERPAGEINFO;
      
      /* options for GpiPlayMetaFile */
      #define RS_DEFAULT                      0L
      #define RS_NODISCARD                    1L
      #define LC_DEFAULT                      0L
      #define LC_NOLOAD                       1L
      #define LC_LOADDISC                     3L
      #define LT_DEFAULT                      0L
      #define LT_NOMODIFY                     1L
      #define LT_ORIGINALVIEW                 4L
      #define RES_DEFAULT                     0L
      #define RES_NORESET                     1L
      #define RES_RESET                       2L
      #define SUP_DEFAULT                     0L
      #define SUP_NOSUPPRESS                  1L
      #define SUP_SUPPRESS                    2L
      #define CTAB_DEFAULT                    0L
      #define CTAB_NOMODIFY                   1L
      #define CTAB_REPLACE                    3L
      #define CTAB_REPLACEPALETTE             4L
      #define CREA_DEFAULT                    0L
      #define CREA_REALIZE                    1L
      #define CREA_NOREALIZE                  2L
      #define CREA_DOREALIZE                  3L
      
      #ifndef INCL_SAADEFS
         
         #define DDEF_DEFAULT                    0L
         #define DDEF_IGNORE                     1L
         #define DDEF_LOADDISC                   3L
         #define DOBJ_DEFAULT                    0L
         #define DOBJ_NODELETE                   1L
         #define DOBJ_DELETE                     2L
         #define RSP_DEFAULT                     0L
         #define RSP_NODISCARD                   1L
         
      #endif /* no INCL_SAADEFS */
      
      /*** MetaFile functions */
      
      BOOL  APIENTRY GpiResumePlay( HPS );
      
      BOOL  APIENTRY GpiSuspendPlay( HPS );
      HMF   APIENTRY GpiCopyMetaFile(HMF hmf);
      
      BOOL  APIENTRY GpiDeleteMetaFile(HMF hmf);
      
#ifdef __cplusplus
      HMF   APIENTRY GpiLoadMetaFile(HAB hab,
                                     PCSZ  pszFilename);
#else
      HMF   APIENTRY GpiLoadMetaFile(HAB hab,
                                     PSZ pszFilename);
#endif
      
#ifdef __cplusplus
      LONG  APIENTRY GpiPlayMetaFile(HPS hps,
                                     HMF hmf,
                                     LONG lCount1,
                                     PLONG alOptarray,
                                     PLONG plSegCount,
                                     LONG lCount2,
                                     PCSZ  pszDesc);
#else
      LONG  APIENTRY GpiPlayMetaFile(HPS hps,
                                     HMF hmf,
                                     LONG lCount1,
                                     PLONG alOptarray,
                                     PLONG plSegCount,
                                     LONG lCount2,
                                     PSZ pszDesc);
#endif
      
      BOOL  APIENTRY GpiQueryMetaFileBits(HMF hmf,
                                          LONG lOffset,
                                          LONG lLength,
                                          PBYTE pbData);
      
      LONG  APIENTRY GpiQueryMetaFileLength(HMF hmf);
      
#ifdef __cplusplus
      BOOL  APIENTRY GpiSaveMetaFile(HMF hmf,
                                     PCSZ  pszFilename);
#else
      BOOL  APIENTRY GpiSaveMetaFile(HMF hmf,
                                     PSZ pszFilename);
#endif
      
      #ifndef INCL_SAADEFS
         
         BOOL  APIENTRY GpiSetMetaFileBits(HMF hmf,
                                           LONG lOffset,
                                           LONG lLength,
                                           PBYTE pbBuffer);
      #endif /* no INCL_SAADEFS */
      
   #endif /* no INCL_DDIDEFS */
   
#endif /* non-common GPIMETAFILES */

#ifdef INCL_GPIDEFAULTS
   
   /*** default functions */
   
   BOOL  APIENTRY GpiQueryDefArcParams(HPS hps,
                                       PARCPARAMS parcpArcParams);
   
   BOOL  APIENTRY GpiQueryDefAttrs(HPS hps,
                                   LONG lPrimType,
                                   ULONG flAttrMask,
                                   PBUNDLE ppbunAttrs);
   
   BOOL  APIENTRY GpiQueryDefTag(HPS hps,
                                 PLONG plTag);
   
   BOOL  APIENTRY GpiQueryDefViewingLimits(HPS hps,
                                           PRECTL prclLimits);
   
   BOOL  APIENTRY GpiSetDefArcParams(HPS hps,
                                     PARCPARAMS parcpArcParams);
   
   BOOL  APIENTRY GpiSetDefAttrs(HPS hps,
                                 LONG lPrimType,
                                 ULONG flAttrMask,
                                 PBUNDLE ppbunAttrs);
   
   BOOL  APIENTRY GpiSetDefTag(HPS hps,
                               LONG lTag);
   
   BOOL  APIENTRY GpiSetDefViewingLimits(HPS hps,
                                         PRECTL prclLimits);
   
   
#endif /* GPIDEFAULTS */

#ifdef INCL_GPIPOLYGON
   typedef struct _POLYGON   /* plygn */
   {
      ULONG       ulPoints;
      PPOINTL     aPointl;
   } POLYGON;
   typedef POLYGON *PPOLYGON;
   
   typedef struct _POLYSET   /* plyst */
   {
      ULONG       ulPolys;
      POLYGON     aPolygon[1];
   } POLYSET;
   typedef POLYSET *PPOLYSET;
   
   /* control flags used by GpiPolygons */
   #define POLYGON_NOBOUNDARY                   0L
   #define POLYGON_BOUNDARY                0x0001L
   
   #define POLYGON_ALTERNATE                    0L
   #define POLYGON_WINDING                 0x0002L
   
   #define POLYGON_INCL                         0L
   #define POLYGON_EXCL                    0x0008L
   
   #define POLYGON_FILL                         0L
   #define POLYGON_NOFILL                  0x0010L
   
   #ifndef INCL_DDIDEFS
      
      LONG APIENTRY GpiPolygons(HPS hps,
                                ULONG ulCount,
                                PPOLYGON paplgn,
                                ULONG flOptions,
                                ULONG flModel);
      #ifdef INCL_GPIREGIONS
         
         HRGN APIENTRY GpiCreatePolygonRegion(HPS hps,
                                              ULONG ulCount,
                                              PPOLYGON paplgn,
                                              ULONG flOptions);
      #endif /* INCL_GPIREGIONS */
      
   #endif /* no INCL_DDIDEFS */
#endif /* non-common GPIPOLYGON */

#ifdef INCL_GPIINK
   /* options for GpiEndInkPath */
   #define PPE_KEEPPATH                         0L
   #define PPE_ERASEPATH                        1L
   
   /* options for GpiStrokeInkPath */
   #define PPS_INKMOVE                          0L
   #define PPS_INKDOWN                          1L
   #define PPS_INKUP                            2L
   
   #ifndef INCL_DDIDEFS
      BOOL APIENTRY GpiBeginInkPath (HPS hps,
                                     LONG lPath,
                                     ULONG flOptions);
      
      BOOL APIENTRY GpiEndInkPath (HPS hps,
                                   ULONG flOptions);
      
      LONG APIENTRY GpiStrokeInkPath (HPS hps,
                                      LONG lPath,
                                      LONG lCount,
                                      PPOINTL aptlPoints,
                                      ULONG flOptions);
   #endif /* no INCL_DDIDEFS */
#endif /* non-common GPIINK */

#ifdef INCL_GPIERRORS
   #include <pmerr.h>
#endif /* GPIERRORS */


/* NOINC */
#ifdef __cplusplus
        }
#endif
#if __IBMC__ || __IBMCPP__
   #pragma info( none )
   #ifndef __CHKHDR__
      #pragma info( restore )
   #endif
   #pragma info( restore )
#endif
/* INC */

/**************************** end of file **********************************/
