#
# A Makefile for pmdgpi.dll
# (c) osFree project,
# author, date
#

PROJ = pmgpi
TRGT = $(PROJ).dll
DESC = Presentation Manager GPI
srcfiles     = $(p)pmgpi16$(e) $(p)pmgpi$(e) $(p)unimpl$(e)
ADD_COPT     = -i=. -sg -of+ -bd -3s
ADD_LINKOPT  = segment   type DATA nonshared
NOLIBS       = 1
DLL          = 1
DLLOPT       = initinstance terminstance
OPTIONS      = manyautodata, caseexact
IMPORTS      = &
GpiLoadBitmap PMMERGE.5047, &
Gre32Entry2   PMGRE.00062,  &
Gre32Entry3   PMGRE.00063,  &
Gre32Entry4   PMGRE.00064,  &
Gre32Entry5   PMGRE.00065,  &
Gre32Entry6   PMGRE.00066,  &
Gre32Entry7   PMGRE.00067,  &
Gre32Entry8   PMGRE.00068,  &
Gre32Entry9   PMGRE.00069,  &
Gre32Entry10  PMGRE.00070

EXPORTS      = &
# 16-bit functions
DEVOPENDC                              .0001 RESIDENT, &
DEVCLOSEDC                             .0002 RESIDENT, &
DEVPOSTDEVICEMODES                     .0003 RESIDENT, &
DEVESCAPE                              .0004 RESIDENT, &
DEVQUERYHARDCOPYCAPS                   .0005 RESIDENT, &
DEVQUERYCAPS                           .0006 RESIDENT, &
GPICREATEPS                            .0007 RESIDENT, &
GPIQUERYPS                             .0009 RESIDENT, &
GPIDESTROYPS                           .0010 RESIDENT, &
GPIRESETPS                             .0011 RESIDENT, &
GPISAVEPS                              .0012 RESIDENT, &
GPIRESTOREPS                           .0013 RESIDENT, &
GPIASSOCIATE                           .0014 RESIDENT, &
GPIERRORSEGMENTDATA                    .0015 RESIDENT, &
GPIERASE                               .0016 RESIDENT, &
GPISETDRAWCONTROL                      .0017 RESIDENT, &
GPIQUERYDRAWCONTROL                    .0018 RESIDENT, &
GPIDRAWCHAIN                           .0019 RESIDENT, &
GPIDRAWFROM                            .0020 RESIDENT, &
GPIDRAWSEGMENT                         .0021 RESIDENT, &
GPISETSTOPDRAW                         .0022 RESIDENT, &
GPIQUERYSTOPDRAW                       .0023 RESIDENT, &
GPIREMOVEDYNAMICS                      .0024 RESIDENT, &
GPIDRAWDYNAMICS                        .0025 RESIDENT, &
GPISETDRAWINGMODE                      .0026 RESIDENT, &
GPIQUERYDRAWINGMODE                    .0027 RESIDENT, &
GPIGETDATA                             .0028 RESIDENT, &
GPIPUTDATA                             .0029 RESIDENT, &
GPISETPICKAPERTURESIZE                 .0030 RESIDENT, &
GPIQUERYPICKAPERTURESIZE               .0031 RESIDENT, &
GPISETPICKAPERTUREPOSITION             .0032 RESIDENT, &
GPIQUERYPICKAPERTUREPOSITION           .0033 RESIDENT, &
GPISETTAG                              .0034 RESIDENT, &
GPIQUERYTAG                            .0035 RESIDENT, &
GPICORRELATECHAIN                      .0036 RESIDENT, &
GPICORRELATEFROM                       .0037 RESIDENT, &
GPICORRELATESEGMENT                    .0038 RESIDENT, &
GPIRESETBOUNDARYDATA                   .0042 RESIDENT, &
GPIQUERYBOUNDARYDATA                   .0043 RESIDENT, &
GPIOPENSEGMENT                         .0044 RESIDENT, &
GPICLOSESEGMENT                        .0046 RESIDENT, &
GPIDELETESEGMENT                       .0047 RESIDENT, &
GPIDELETESEGMENTS                      .0048 RESIDENT, &
GPIQUERYSEGMENTNAMES                   .0049 RESIDENT, &
GPISETINITIALSEGMENTATTRS              .0051 RESIDENT, &
GPIQUERYINITIALSEGMENTATTRS            .0052 RESIDENT, &
GPISETSEGMENTATTRS                     .0053 RESIDENT, &
GPIQUERYSEGMENTATTRS                   .0054 RESIDENT, &
GPISETSEGMENTPRIORITY                  .0055 RESIDENT, &
GPIQUERYSEGMENTPRIORITY                .0056 RESIDENT, &
GPISETEDITMODE                         .0057 RESIDENT, &
GPIQUERYEDITMODE                       .0058 RESIDENT, &
GPISETELEMENTPOINTER                   .0059 RESIDENT, &
GPIQUERYELEMENTPOINTER                 .0060 RESIDENT, &
GPIOFFSETELEMENTPOINTER                .0061 RESIDENT, &
GPIDELETEELEMENT                       .0062 RESIDENT, &
GPIDELETEELEMENTRANGE                  .0063 RESIDENT, &
GPILABEL                               .0064 RESIDENT, &
GPISETELEMENTPOINTERATLABEL            .0065 RESIDENT, &
GPIDELETEELEMENTSBETWEENLABELS         .0066 RESIDENT, &
GPIQUERYELEMENTTYPE                    .0067 RESIDENT, &
GPIQUERYELEMENT                        .0068 RESIDENT, &
GPIELEMENT                             .0069 RESIDENT, &
GPIBEGINELEMENT                        .0070 RESIDENT, &
GPIENDELEMENT                          .0071 RESIDENT, &
GPISETSEGMENTTRANSFORMMATRIX           .0078 RESIDENT, &
GPIQUERYSEGMENTTRANSFORMMATRIX         .0079 RESIDENT, &
GPISETMODELTRANSFORMMATRIX             .0080 RESIDENT, &
GPIQUERYMODELTRANSFORMMATRIX           .0081 RESIDENT, &
GPICALLSEGMENTMATRIX                   .0082 RESIDENT, &
GPISETDEFAULTVIEWMATRIX                .0083 RESIDENT, &
GPIQUERYDEFAULTVIEWMATRIX              .0084 RESIDENT, &
GPISETPAGEVIEWPORT                     .0085 RESIDENT, &
GPIQUERYPAGEVIEWPORT                   .0086 RESIDENT, &
GPISETVIEWINGTRANSFORMMATRIX           .0087 RESIDENT, &
GPIQUERYVIEWINGTRANSFORMMATRIX         .0088 RESIDENT, &
GPISETGRAPHICSFIELD                    .0089 RESIDENT, &
GPIQUERYGRAPHICSFIELD                  .0090 RESIDENT, &
GPISETVIEWINGLIMITS                    .0091 RESIDENT, &
GPIQUERYVIEWINGLIMITS                  .0092 RESIDENT, &
GPICONVERT                             .0093 RESIDENT, &
GPISETATTRMODE                         .0094 RESIDENT, &
GPIQUERYATTRMODE                       .0095 RESIDENT, &
GPIPOP                                 .0096 RESIDENT, &
GPISETATTRS                            .0097 RESIDENT, &
GPIQUERYATTRS                          .0098 RESIDENT, &
GPICREATELOGCOLORTABLE                 .0099 RESIDENT, &
GPIREALIZECOLORTABLE                   .0100 RESIDENT, &
GPIUNREALIZECOLORTABLE                 .0101 RESIDENT, &
GPIQUERYCOLORDATA                      .0102 RESIDENT, &
GPIQUERYLOGCOLORTABLE                  .0103 RESIDENT, &
GPIQUERYREALCOLORS                     .0104 RESIDENT, &
GPIQUERYNEARESTCOLOR                   .0105 RESIDENT, &
GPIQUERYCOLORINDEX                     .0106 RESIDENT, &
GPIQUERYRGBCOLOR                       .0107 RESIDENT, &
GPISETCOLOR                            .0108 RESIDENT, &
GPIQUERYCOLOR                          .0109 RESIDENT, &
GPISETBACKCOLOR                        .0110 RESIDENT, &
GPIQUERYBACKCOLOR                      .0111 RESIDENT, &
GPISETMIX                              .0112 RESIDENT, &
GPIQUERYMIX                            .0113 RESIDENT, &
GPISETBACKMIX                          .0114 RESIDENT, &
GPIQUERYBACKMIX                        .0115 RESIDENT, &
GPISETLINETYPE                         .0116 RESIDENT, &
GPIQUERYLINETYPE                       .0117 RESIDENT, &
GPISETLINEWIDTH                        .0118 RESIDENT, &
GPIQUERYLINEWIDTH                      .0119 RESIDENT, &
GPISETLINEWIDTHGEOM                    .0120 RESIDENT, &
GPIQUERYLINEWIDTHGEOM                  .0121 RESIDENT, &
GPISETLINEEND                          .0122 RESIDENT, &
GPIQUERYLINEEND                        .0123 RESIDENT, &
GPISETLINEJOIN                         .0124 RESIDENT, &
GPIQUERYLINEJOIN                       .0125 RESIDENT, &
GPISETCURRENTPOSITION                  .0126 RESIDENT, &
GPIQUERYCURRENTPOSITION                .0127 RESIDENT, &
GPIMOVE                                .0128 RESIDENT, &
GPILINE                                .0129 RESIDENT, &
GPIPOLYLINE                            .0130 RESIDENT, &
GPIBOX                                 .0131 RESIDENT, &
GPIPTVISIBLE                           .0132 RESIDENT, &
GPIRECTVISIBLE                         .0133 RESIDENT, &
GPISETARCPARAMS                        .0134 RESIDENT, &
GPIQUERYARCPARAMS                      .0135 RESIDENT, &
GPIPOINTARC                            .0136 RESIDENT, &
GPIFULLARC                             .0137 RESIDENT, &
GPIPARTIALARC                          .0138 RESIDENT, &
GPIPOLYFILLET                          .0139 RESIDENT, &
GPIPOLYFILLETSHARP                     .0140 RESIDENT, &
GPIPOLYSPLINE                          .0141 RESIDENT, &
GPISETBITMAPID                         .0142 RESIDENT, &
GPIQUERYBITMAPHANDLE                   .0143 RESIDENT, &
GPISETPATTERNSET                       .0144 RESIDENT, &
GPIQUERYPATTERNSET                     .0145 RESIDENT, &
GPISETPATTERN                          .0146 RESIDENT, &
GPIQUERYPATTERN                        .0147 RESIDENT, &
GPISETPATTERNREFPOINT                  .0148 RESIDENT, &
GPIQUERYPATTERNREFPOINT                .0149 RESIDENT, &
GPIBEGINAREA                           .0150 RESIDENT, &
GPIENDAREA                             .0151 RESIDENT, &
GPILOADFONTS                           .0152 RESIDENT, &
GPIUNLOADFONTS                         .0153 RESIDENT, &
GPICREATELOGFONT                       .0154 RESIDENT, &
GPIDELETESETID                         .0155 RESIDENT, &
GPIQUERYNUMBERSETIDS                   .0156 RESIDENT, &
GPIQUERYSETIDS                         .0157 RESIDENT, &
GPIQUERYFONTS                          .0158 RESIDENT, &
GPIQUERYFONTMETRICS                    .0159 RESIDENT, &
GPIQUERYKERNINGPAIRS                   .0160 RESIDENT, &
GPIQUERYWIDTHTABLE                     .0161 RESIDENT, &
GPISETCP                               .0162 RESIDENT, &
GPIQUERYCP                             .0163 RESIDENT, &
GPIQUERYTEXTBOX                        .0164 RESIDENT, &
DEVQUERYDEVICENAMES                    .0165 RESIDENT, &
GPIQUERYDEFCHARBOX                     .0166 RESIDENT, &
GPIQUERYFONTFILEDESCRIPTIONS           .0167 RESIDENT, &
GPISETCHARSET                          .0168 RESIDENT, &
GPIQUERYCHARSET                        .0169 RESIDENT, &
GPISETCHARBOX                          .0170 RESIDENT, &
GPIQUERYCHARBOX                        .0171 RESIDENT, &
GPISETCHARANGLE                        .0172 RESIDENT, &
GPIQUERYCHARANGLE                      .0173 RESIDENT, &
GPISETCHARSHEAR                        .0174 RESIDENT, &
GPIQUERYCHARSHEAR                      .0175 RESIDENT, &
GPISETCHARDIRECTION                    .0176 RESIDENT, &
GPIQUERYCHARDIRECTION                  .0177 RESIDENT, &
GPISETCHARMODE                         .0178 RESIDENT, &
GPIQUERYCHARMODE                       .0179 RESIDENT, &
GPICHARSTRING                          .0180 RESIDENT, &
GPICHARSTRINGAT                        .0181 RESIDENT, &
GPICHARSTRINGPOS                       .0182 RESIDENT, &
GPICHARSTRINGPOSAT                     .0183 RESIDENT, &
GPISETMARKERSET                        .0184 RESIDENT, &
GPIQUERYMARKERSET                      .0185 RESIDENT, &
GPISETMARKER                           .0186 RESIDENT, &
GPIQUERYMARKER                         .0187 RESIDENT, &
GPISETMARKERBOX                        .0188 RESIDENT, &
GPIQUERYMARKERBOX                      .0189 RESIDENT, &
GPIMARKER                              .0190 RESIDENT, &
GPIPOLYMARKER                          .0191 RESIDENT, &
GPIIMAGE                               .0192 RESIDENT, &
GPICREATEBITMAP                        .0196 RESIDENT, &
GPIDELETEBITMAP                        .0197 RESIDENT, &
GPISETBITMAP                           .0198 RESIDENT, &
GPISETBITMAPDIMENSION                  .0199 RESIDENT, &
GPIQUERYBITMAPDIMENSION                .0200 RESIDENT, &
GPIQUERYDEVICEBITMAPFORMATS            .0201 RESIDENT, &
GPIQUERYBITMAPPARAMETERS               .0202 RESIDENT, &
GPISETBITMAPBITS                       .0203 RESIDENT, &
GPIQUERYBITMAPBITS                     .0204 RESIDENT, &
GPIBITBLT                              .0205 RESIDENT, &
GPISETPEL                              .0206 RESIDENT, &
GPIQUERYPEL                            .0207 RESIDENT, &
GPICREATEREGION                        .0208 RESIDENT, &
GPISETREGION                           .0209 RESIDENT, &
GPIDESTROYREGION                       .0210 RESIDENT, &
GPICOMBINEREGION                       .0211 RESIDENT, &
GPIEQUALREGION                         .0212 RESIDENT, &
GPIOFFSETREGION                        .0213 RESIDENT, &
GPIPTINREGION                          .0214 RESIDENT, &
GPIRECTINREGION                        .0215 RESIDENT, &
GPIQUERYREGIONBOX                      .0216 RESIDENT, &
GPIQUERYREGIONRECTS                    .0217 RESIDENT, &
GPISETCLIPREGION                       .0218 RESIDENT, &
GPIQUERYCLIPREGION                     .0219 RESIDENT, &
GPIQUERYCLIPBOX                        .0220 RESIDENT, &
GPIINTERSECTCLIPRECTANGLE              .0221 RESIDENT, &
GPIEXCLUDECLIPRECTANGLE                .0222 RESIDENT, &
GPIOFFSETCLIPREGION                    .0223 RESIDENT, &
GPIPAINTREGION                         .0224 RESIDENT, &
GPILOADMETAFILE                        .0225 RESIDENT, &
GPICOPYMETAFILE                        .0226 RESIDENT, &
GPIPLAYMETAFILE                        .0227 RESIDENT, &
GPISAVEMETAFILE                        .0228 RESIDENT, &
GPIDELETEMETAFILE                      .0229 RESIDENT, &
GPIQUERYMETAFILEBITS                   .0230 RESIDENT, &
GPISETMETAFILEBITS                     .0231 RESIDENT, &
GPIQUERYMETAFILELENGTH                 .0232 RESIDENT, &
GPISETCLIPPATH                         .0233 RESIDENT, &
GPIBEGINPATH                           .0234 RESIDENT, &
GPIENDPATH                             .0235 RESIDENT, &
GPICLOSEFIGURE                         .0236 RESIDENT, &
GPIMODIFYPATH                          .0237 RESIDENT, &
GPIFILLPATH                            .0238 RESIDENT, &
GPIQUERYDEVICE                         .0240 RESIDENT, &
GPIVECTORSYMBOL                        .0241 RESIDENT, &
GPIOPENMETAFILE                        .0242 RESIDENT, &
GPICLOSEMETAFILE                       .0243 RESIDENT, &
DEVSTDOPEN                             .0244 RESIDENT, &
GPIQUERYCHARSTRINGPOS                  .0245 RESIDENT, &
GPIQUERYCHARSTRINGPOSAT                .0246 RESIDENT, &
GPICONVPSH                             .0247 RESIDENT, &
GPISETPS                               .0248 RESIDENT, &
SEGSGWOPENSEGMENTWINDOW                .0249 RESIDENT, &
MTENDREADREQUEST                       .0250 RESIDENT, &
MTGETDESCRIPTION                       .0251 RESIDENT, &
MTGETCODEPAGE                          .0253 RESIDENT, &
MTGETLCT                               .0254 RESIDENT, &
MTGETGDDINFO                           .0255 RESIDENT, &
FMTCONVERTGOCAPOLY                     .0256 RESIDENT, &
MTGETFIRSTFONT                         .0257 RESIDENT, &
SEGSGWNEWPARTDATA                      .0258 RESIDENT, &
MTSTARTREADREQUEST                     .0259 RESIDENT, &
MTGETFIRSTGRAPHICSDATA                 .0260 RESIDENT, &
MTGETNEXTFONT                          .0261 RESIDENT, &
MTGETNEXTGRAPHICSDATA                  .0262 RESIDENT, &
GPILOADPUBLICFONTS                     .0263 RESIDENT, &
GPIUNLOADPUBLICFONTS                   .0264 RESIDENT, &
GPIACCESSMETAFILE                      .0265 RESIDENT, &
GPIMTASSOCIATE                         .0266 RESIDENT, &
GPIMTDISASSOCIATE                      .0267 RESIDENT, &
GPICOMMENT                             .0268 RESIDENT, &
GPIWCBITBLT                            .0269 RESIDENT, &
GPISTROKEPATH                          .0270 RESIDENT, &
SEGSGWNEXTORDERF                       .0271 RESIDENT, &
GPISUSPENDPLAY                         .0272 RESIDENT, &
GPIRESUMEPLAY                          .0273 RESIDENT, &
GPIOUTLINEPATH                         .0274 RESIDENT, &
GPISETDEFTAG                           .0275 RESIDENT, &
GPIQUERYDEFTAG                         .0276 RESIDENT, &
GPISETDEFATTRS                         .0277 RESIDENT, &
GPIQUERYDEFATTRS                       .0278 RESIDENT, &
GPISETDEFVIEWINGLIMITS                 .0279 RESIDENT, &
GPIQUERYDEFVIEWINGLIMITS               .0280 RESIDENT, &
GPISETDEFARCPARAMS                     .0281 RESIDENT, &
GPIQUERYDEFARCPARAMS                   .0282 RESIDENT, &
GPITRANSLATE                           .0283 RESIDENT, &
GPISCALE                               .0284 RESIDENT, &
GPIROTATE                              .0285 RESIDENT, &
GPIPOLYLINEDISJOINT                    .0286 RESIDENT, &
GPIPATHTOREGION                        .0287 RESIDENT, &
GPIFLOODFILL                           .0289 RESIDENT, &
GPIDRAWBITS                            .0290 RESIDENT, &
GPIQUERYBITMAPINFOHEADER               .0291 RESIDENT, &
GPIQUERYLOGICALFONT                    .0292 RESIDENT, &
GPIQUERYFACESTRING                     .0293 RESIDENT, &
GPIQUERYFONTACTION                     .0294 RESIDENT, &
GPICREATEPALETTE                       .0295 RESIDENT, &
GPIDELETEPALETTE                       .0296 RESIDENT, &
GPISELECTPALETTE                       .0297 RESIDENT, &
GPIANIMATEPALETTE                      .0298 RESIDENT, &
GPISETPALETTEENTRIES                   .0299 RESIDENT, &
GPIQUERYPALETTE                        .0300 RESIDENT, &
GPIQUERYPALETTEINFO                    .0301 RESIDENT, &
GPIQUERYCHAREXTRA                      .0302 RESIDENT, &
GPISETCHAREXTRA                        .0303 RESIDENT, &
GPIQUERYCHARBREAKEXTRA                 .0304 RESIDENT, &
GPISETCHARBREAKEXTRA                   .0305 RESIDENT, &
GPIFRAMEREGION                         .0306 RESIDENT, &
GPICONVERTWITHMATRIX                   .0307 RESIDENT, &
GPISETDCOWNER                          .0308 RESIDENT, &
GPISETBITMAPOWNER                      .0309 RESIDENT, &
GPISETREGIONOWNER                      .0310 RESIDENT, &
GPIINSTALLIFIFONT                      .0311 RESIDENT, &
DSPINITSYSTEMDRIVERNAME                .0312 RESIDENT, &
# 32-bit functions
GpiAssociate                    .0351, &
GpiBeginArea                    .0352, &
GpiBeginElement                 .0353, &
GpiBeginPath                    .0354, &
GpiBitBlt                       .0355, &
GpiBox                          .0356, &
GpiCallSegmentMatrix            .0357, &
GpiCharString                   .0358, &
GpiCharStringAt                 .0359, &
GpiCloseFigure                  .0360, &
GpiCloseSegment                 .0361, &
GpiCombineRegion                .0362, &
GpiComment                      .0363, &
GpiConvert                      .0364, &
GpiCopyMetaFile                 .0365, &
GpiCorrelateChain               .0366, &
GpiCorrelateFrom                .0367, &
GpiCreateLogFont                .0368, &
GpiCreatePS                     .0369, &
GpiCreateRegion                 .0370, &
GpiDeleteBitmap                 .0371, &
GpiDeleteElement                .0372, &
GpiDeleteElementRange           .0373, &
GpiDeleteElementsBetweenLabels  .0374, &
GpiDeleteMetaFile               .0375, &
GpiDeleteSegment                .0376, &
GpiDeleteSegments               .0377, &
GpiDeleteSetId                  .0378, &
GpiDestroyPS                    .0379, &
GpiDrawChain                    .0380, &
GpiDrawDynamics                 .0381, &
GpiDrawFrom                     .0382, &
GpiDrawSegment                  .0383, &
GpiElement                      .0384, &
GpiEndArea                      .0385, &
GpiEndElement                   .0386, &
GpiEndPath                      .0387, &
GpiEqualRegion                  .0388, &
GpiErase                        .0389, &
GpiErrorSegmentData             .0390, &
GpiExcludeClipRectangle         .0391, &
GpiFillPath                     .0392, &
GpiFullArc                      .0393, &
GpiGetData                      .0394, &
GpiImage                        .0395, &
GpiIntersectClipRectangle       .0396, &
GpiLabel                        .0397, &
GpiLine                         .0398, &
GpiLoadBitmap                   .0399, &
GpiLoadFonts                    .0400, &
GpiLoadMetaFile                 .0401, &
GpiMarker                       .0402, &
GpiModifyPath                   .0403, &
GpiMove                         .0404, &
GpiOffsetClipRegion             .0405, &
GpiOffsetElementPointer         .0406, &
GpiOffsetRegion                 .0407, &
GpiOpenSegment                  .0408, &
GpiPaintRegion                  .0409, &
GpiPlayMetaFile                 .0411, &
GpiPointArc                     .0412, &
GpiPolyFillet                   .0413, &
GpiPolyFilletSharp              .0414, &
GpiPolyLine                     .0415, &
GpiPolyMarker                   .0416, &
GpiPolySpline                   .0417, &
GpiPop                          .0418, &
GpiPtInRegion                   .0419, &
GpiPtVisible                    .0420, &
GpiPutData                      .0421, &
GpiQueryArcParams               .0422, &
GpiQueryAttrMode                .0423, &
GpiQueryBackColor               .0424, &
GpiQueryBackMix                 .0425, &
GpiQueryBitmapDimension         .0426, &
GpiQueryBitmapHandle            .0427, &
GpiQueryBoundaryData            .0428, &
GpiQueryCharAngle               .0429, &
GpiQueryCharBox                 .0430, &
GpiQueryCharDirection           .0431, &
GpiQueryCharMode                .0432, &
GpiQueryCharSet                 .0433, &
GpiQueryCharShear               .0434, &
GpiQueryClipBox                 .0435, &
GpiQueryClipRegion              .0436, &
GpiQueryColor                   .0437, &
GpiQueryColorData               .0438, &
GpiQueryColorIndex              .0439, &
GpiQueryCp                      .0440, &
GpiQueryCurrentPosition         .0441, &
GpiQueryDefCharBox              .0442, &
GpiQueryDefaultViewMatrix       .0443, &
GpiQueryDevice                  .0444, &
GpiQueryDeviceBitmapFormats     .0445, &
GpiQueryDrawControl             .0446, &
GpiQueryDrawingMode             .0447, &
GpiQueryEditMode                .0448, &
GpiQueryElement                 .0449, &
GpiQueryElementPointer          .0450, &
GpiQueryElementType             .0451, &
GpiQueryFontFileDescriptions    .0452, &
GpiQueryFontMetrics             .0453, &
GpiQueryGraphicsField           .0454, &
GpiQueryInitialSegmentAttrs     .0455, &
GpiQueryKerningPairs            .0456, &
GpiQueryLineEnd                 .0457, &
GpiQueryLineJoin                .0458, &
GpiQueryLineType                .0459, &
GpiQueryLineWidth               .0460, &
GpiQueryLineWidthGeom           .0461, &
GpiQueryMarker                  .0462, &
GpiQueryMarkerBox               .0463, &
GpiQueryMarkerSet               .0464, &
GpiQueryMetaFileBits            .0465, &
GpiQueryMetaFileLength          .0466, &
GpiQueryMix                     .0467, &
GpiQueryModelTransformMatrix    .0468, &
GpiQueryNearestColor            .0469, &
GpiQueryNumberSetIds            .0470, &
GpiQueryPS                      .0471, &
GpiQueryPageViewport            .0472, &
GpiQueryPattern                 .0473, &
GpiQueryPatternRefPoint         .0474, &
GpiQueryPatternSet              .0475, &
GpiQueryPel                     .0476, &
GpiQueryPickAperturePosition    .0477, &
GpiQueryPickApertureSize        .0478, &
GpiQueryRGBColor                .0479, &
GpiQueryRealColors              .0480, &
GpiQueryRegionBox               .0481, &
GpiQuerySegmentAttrs            .0482, &
GpiQuerySegmentNames            .0483, &
GpiQuerySegmentPriority         .0484, &
GpiQuerySegmentTransformMatrix  .0485, &
GpiQuerySetIds                  .0486, &
GpiQueryStopDraw                .0487, &
GpiQueryTag                     .0488, &
GpiQueryTextBox                 .0489, &
GpiQueryViewingLimits           .0490, &
GpiQueryViewingTransformMatrix  .0491, &
GpiQueryWidthTable              .0492, &
GpiRectInRegion                 .0494, &
GpiRectVisible                  .0495, &
GpiRemoveDynamics               .0496, &
GpiResetBoundaryData            .0497, &
GpiResetPS                      .0498, &
GpiRestorePS                    .0499, &
GpiSaveMetaFile                 .0500, &
GpiSavePS                       .0501, &
GpiSetArcParams                 .0502, &
GpiSetAttrMode                  .0503, &
GpiSetBackColor                 .0504, &
GpiSetBackMix                   .0505, &
GpiSetBitmap                    .0506, &
GpiSetBitmapDimension           .0507, &
GpiSetBitmapId                  .0508, &
GpiSetCharAngle                 .0509, &
GpiSetCharBox                   .0510, &
GpiSetCharDirection             .0511, &
GpiSetCharMode                  .0512, &
GpiSetCharSet                   .0513, &
GpiSetCharShear                 .0514, &
GpiSetClipPath                  .0515, &
GpiSetClipRegion                .0516, &
GpiSetColor                     .0517, &
GpiSetCp                        .0518, &
GpiSetCurrentPosition           .0519, &
GpiSetDefaultViewMatrix         .0520, &
GpiSetDrawControl               .0521, &
GpiSetDrawingMode               .0522, &
GpiSetEditMode                  .0523, &
GpiSetElementPointer            .0524, &
GpiSetElementPointerAtLabel     .0525, &
GpiSetGraphicsField             .0526, &
GpiSetInitialSegmentAttrs       .0527, &
GpiSetLineEnd                   .0528, &
GpiSetLineJoin                  .0529, &
GpiSetLineType                  .0530, &
GpiSetLineWidth                 .0531, &
GpiSetLineWidthGeom             .0532, &
GpiSetMarker                    .0533, &
GpiSetMarkerBox                 .0534, &
GpiSetMarkerSet                 .0535, &
GpiSetMetaFileBits              .0536, &
GpiSetMix                       .0537, &
GpiSetModelTransformMatrix      .0538, &
GpiSetPS                        .0539, &
GpiSetPageViewport              .0540, &
GpiSetPattern                   .0541, &
GpiSetPatternRefPoint           .0542, &
GpiSetPatternSet                .0543, &
GpiSetPel                       .0544, &
GpiSetPickAperturePosition      .0545, &
GpiSetRegion                    .0546, &
GpiSetSegmentAttrs              .0547, &
GpiSetSegmentPriority           .0548, &
GpiSetSegmentTransformMatrix    .0549, &
GpiSetStopDraw                  .0550, &
GpiSetTag                       .0551, &
GpiSetViewingLimits             .0552, &
GpiSetViewingTransformMatrix    .0553, &
GpiStrokePath                   .0554, &
GpiUnloadFonts                  .0555, &
GpiWCBitBlt                     .0557, &
GpiPolyLineDisjoint             .0558, &
GpiPathToRegion                 .0559, &
GpiFloodFill                    .0560, &
GpiSuspendPlay                  .0561, &
GpiResumePlay                   .0562, &
GpiOutlinePath                  .0563, &
GpiTranslate                    .0564, &
GpiScale                        .0565, &
GpiRotate                       .0566, &
GpiQueryDefArcParams            .0567, &
GpiQueryDefTag                  .0568, &
GpiQueryDefViewingLimits        .0569, &
GpiSetDefArcParams              .0570, &
GpiSetDefTag                    .0571, &
GpiSetDefViewingLimits          .0572, &
GpiQueryBitmapParameters        .0573, &
GpiQueryLogicalFont             .0574, &
GpiQueryFaceString              .0575, &
GpiQueryFontAction              .0576, &
GpiDeletePalette                .0577, &
GpiSelectPalette                .0578, &
GpiQueryPalette                 .0579, &
GpiCharStringPos                .0580, &
GpiCharStringPosAt              .0581, &
GpiCorrelateSegment             .0582, &
GpiQueryAttrs                   .0583, &
GpiQueryCharStringPos           .0584, &
GpiQueryCharStringPosAt         .0585, &
GpiQueryFonts                   .0586, &
GpiQueryRegionRects             .0587, &
GpiSetAttrs                     .0588, &
GpiSetPickApertureSize          .0589, &
GpiQueryDefAttrs                .0590, &
GpiSetDefAttrs                  .0591, &
GpiCreateLogColorTable          .0592, &
GpiQueryLogColorTable           .0593, &
GpiCreatePalette                .0594, &
GpiAnimatePalette               .0595, &
GpiSetPaletteEntries            .0596, &
GpiQueryPaletteInfo             .0597, &
GpiCreateBitmap                 .0598, &
GpiQueryBitmapBits              .0599, &
GpiQueryBitmapInfoHeader        .0601, &
GpiSetBitmapBits                .0602, &
GpiDrawBits                     .0603, &
DevCloseDC                      .0604, &
DevEscape                       .0605, &
DevQueryCaps                    .0606, &
DevQueryDeviceNames             .0607, &
DevQueryHardcopyCaps            .0608, &
DevPostDeviceModes              .0609, &
DevOpenDC                       .0610, &
GpiDestroyRegion                .0611, &
GpiPartialArc                   .0612, &
GpiQueryCharExtra               .0613, &
GpiSetCharExtra                 .0614, &
GpiQueryCharBreakExtra          .0615, &
GpiSetCharBreakExtra            .0616, &
GpiFrameRegion                  .0617, &
GpiConvertWithMatrix            .0618, &
GpiSetDCOwner                   .0619, &
GpiSetBitmapOwner               .0620, &
GpiSetRegionOwner               .0621, &
GpiLoadPublicFonts              .0622, &
GpiUnloadPublicFonts            .0623, &
GpiInstallIFIFont               .0624, &
undoc625                         .0625, &
undoc626                         .0626, &
GpiOpenMetafile                 .0627, &
GpiCloseMetafile                .0628, &
GpiConvPSH                      .0629, &
undoc630                         .0630, &
undoc631                         .0631, &
undoc632                         .0632, &
undoc633                         .0633, &
undoc634                         .0634, &
undoc635                         .0635, &
undoc636                         .0636, &
undoc637                         .0637, &
undoc638                         .0638, &
undoc639                         .0639, &
undoc640                         .0640, &
undoc641                         .0641, &
undoc642                         .0642, &
undoc643                         .0643, &
GpiMtAssociate                  .0644, &
GpiMtDisassociate               .0645, &
undoc646                         .0646, &
GpiQueryTextAlignment           .0648, &
GpiSetTextAlignment             .0649, &
GpiPolygons                     .0650, &
undoc651                         .0651, &
undoc652                         .0652, &
undoc653                         .0653, &
DspDefaultResolution            .0654, &
DevQueryDisplayResolutions      .0655, &
DspInitSystemDriverName         .0656, &
GpiQueryFullFontFileDescs       .0657, &
undoc658                         .0658, &
undoc659                         .0659, &
undoc660                         .0660, &
undoc661                         .0661, &
undoc662                         .0662, &
undoc663                         .0663, &
GpiAllocateDCData               .0664, &
GpiQueryDCData                  .0665, &
undoc666                         .0666, &
undoc667                         .0667, &
undoc668                         .0668, &
undoc669                         .0669, &
GPIBEGININKPATH                 .0670 RESIDENT, &
GPIENDINKPATH                   .0671 RESIDENT, &
GPISTROKEINKPATH                .0672 RESIDENT, &
PaxGetPolyFillMode              .0700, &
PaxSetPolyFillMode              .0701, &
PaxBeginPath                    .0703, &
PaxCloseFigure                  .0704, &
PaxEndPath                      .0705, &
PaxFillPath                     .0706, &
PaxFlattenPath                  .0707, &
PaxPathToRegion                 .0710, &
PaxStrokeAndFillPath            .0712, &
PaxStrokePath                   .0713, &
PaxWidenPath                    .0714, &
PaxArc                          .0715, &
PaxChord                        .0716, &
PaxEllipse                      .0717, &
PaxLineDDA                      .0718, &
PaxPie                          .0719, &
PaxPolyBezier                   .0720, &
PaxPolyBezierAt                 .0721, &
undoc722                         .0722, &
GpiEnableYInversion             .0723, &
undoc724                         .0724, &
undoc725                         .0725, &
GpiQueryYInversion              .0726, &
undoc727                         .0727, &
GpiSaveMetaFile2                .0728, &
DEVPOSTESCAPE                   .0729 RESIDENT, &
undoc730                         .0730

DEST         = os2$(SEP)dll
UNI2H        = 1

!include $(%ROOT)tools/mk/appsos2_pm.mk
