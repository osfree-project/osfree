#ifndef __glext_h_
#define __glext_h_

#ifdef __cplusplus
extern "C" {
#endif

/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
**
** http://oss.sgi.com/projects/FreeB
**
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
**
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2002 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
**
** Additional Notice Provisions: This software was created using the
** OpenGL(R) version 1.2.1 Sample Implementation published by SGI, but has
** not been independently verified as being compliant with the OpenGL(R)
** version 1.2.1 Specification.
*/

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

//change all " APIENTRY" to " GLAPIENTRY"
#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif

#ifndef GLAPI
#define GLAPI extern
#endif

/*************************************************************/

/* Header file version number, required by OpenGL ABI for Linux */
/* glext.h last updated 2003/5/9 */
/* Current version at http://oss.sgi.com/projects/ogl-sample/registry/ */
#define GL_GLEXT_VERSION 18

#ifndef GL_VERSION_1_2
#define GL_UNSIGNED_BYTE_3_3_2            0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_UNSIGNED_INT_8_8_8_8           0x8035
#define GL_UNSIGNED_INT_10_10_10_2        0x8036
#define GL_RESCALE_NORMAL                 0x803A
#define GL_TEXTURE_BINDING_3D             0x806A
#define GL_PACK_SKIP_IMAGES               0x806B
#define GL_PACK_IMAGE_HEIGHT              0x806C
#define GL_UNPACK_SKIP_IMAGES             0x806D
#define GL_UNPACK_IMAGE_HEIGHT            0x806E
#define GL_TEXTURE_3D                     0x806F
#define GL_PROXY_TEXTURE_3D               0x8070
#define GL_TEXTURE_DEPTH                  0x8071
#define GL_TEXTURE_WRAP_R                 0x8072
#define GL_MAX_3D_TEXTURE_SIZE            0x8073
#define GL_UNSIGNED_BYTE_2_3_3_REV        0x8362
#define GL_UNSIGNED_SHORT_5_6_5           0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV       0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV     0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV     0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368
#define GL_BGR                            0x80E0
#define GL_BGRA                           0x80E1
#define GL_MAX_ELEMENTS_VERTICES          0x80E8
#define GL_MAX_ELEMENTS_INDICES           0x80E9
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_TEXTURE_MIN_LOD                0x813A
#define GL_TEXTURE_MAX_LOD                0x813B
#define GL_TEXTURE_BASE_LEVEL             0x813C
#define GL_TEXTURE_MAX_LEVEL              0x813D
#define GL_LIGHT_MODEL_COLOR_CONTROL      0x81F8
#define GL_SINGLE_COLOR                   0x81F9
#define GL_SEPARATE_SPECULAR_COLOR        0x81FA
#define GL_SMOOTH_POINT_SIZE_RANGE        0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY  0x0B13
#define GL_SMOOTH_LINE_WIDTH_RANGE        0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY  0x0B23
#define GL_ALIASED_POINT_SIZE_RANGE       0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE       0x846E
#endif

#ifndef GL_ARB_imaging
#define GL_CONSTANT_COLOR                 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR       0x8002
#define GL_CONSTANT_ALPHA                 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA       0x8004
#define GL_BLEND_COLOR                    0x8005
#define GL_FUNC_ADD                       0x8006
#define GL_MIN                            0x8007
#define GL_MAX                            0x8008
#define GL_BLEND_EQUATION                 0x8009
#define GL_FUNC_SUBTRACT                  0x800A
#define GL_FUNC_REVERSE_SUBTRACT          0x800B
#define GL_CONVOLUTION_1D                 0x8010
#define GL_CONVOLUTION_2D                 0x8011
#define GL_SEPARABLE_2D                   0x8012
#define GL_CONVOLUTION_BORDER_MODE        0x8013
#define GL_CONVOLUTION_FILTER_SCALE       0x8014
#define GL_CONVOLUTION_FILTER_BIAS        0x8015
#define GL_REDUCE                         0x8016
#define GL_CONVOLUTION_FORMAT             0x8017
#define GL_CONVOLUTION_WIDTH              0x8018
#define GL_CONVOLUTION_HEIGHT             0x8019
#define GL_MAX_CONVOLUTION_WIDTH          0x801A
#define GL_MAX_CONVOLUTION_HEIGHT         0x801B
#define GL_POST_CONVOLUTION_RED_SCALE     0x801C
#define GL_POST_CONVOLUTION_GREEN_SCALE   0x801D
#define GL_POST_CONVOLUTION_BLUE_SCALE    0x801E
#define GL_POST_CONVOLUTION_ALPHA_SCALE   0x801F
#define GL_POST_CONVOLUTION_RED_BIAS      0x8020
#define GL_POST_CONVOLUTION_GREEN_BIAS    0x8021
#define GL_POST_CONVOLUTION_BLUE_BIAS     0x8022
#define GL_POST_CONVOLUTION_ALPHA_BIAS    0x8023
#define GL_HISTOGRAM                      0x8024
#define GL_PROXY_HISTOGRAM                0x8025
#define GL_HISTOGRAM_WIDTH                0x8026
#define GL_HISTOGRAM_FORMAT               0x8027
#define GL_HISTOGRAM_RED_SIZE             0x8028
#define GL_HISTOGRAM_GREEN_SIZE           0x8029
#define GL_HISTOGRAM_BLUE_SIZE            0x802A
#define GL_HISTOGRAM_ALPHA_SIZE           0x802B
#define GL_HISTOGRAM_LUMINANCE_SIZE       0x802C
#define GL_HISTOGRAM_SINK                 0x802D
#define GL_MINMAX                         0x802E
#define GL_MINMAX_FORMAT                  0x802F
#define GL_MINMAX_SINK                    0x8030
#define GL_TABLE_TOO_LARGE                0x8031
#define GL_COLOR_MATRIX                   0x80B1
#define GL_COLOR_MATRIX_STACK_DEPTH       0x80B2
#define GL_MAX_COLOR_MATRIX_STACK_DEPTH   0x80B3
#define GL_POST_COLOR_MATRIX_RED_SCALE    0x80B4
#define GL_POST_COLOR_MATRIX_GREEN_SCALE  0x80B5
#define GL_POST_COLOR_MATRIX_BLUE_SCALE   0x80B6
#define GL_POST_COLOR_MATRIX_ALPHA_SCALE  0x80B7
#define GL_POST_COLOR_MATRIX_RED_BIAS     0x80B8
#define GL_POST_COLOR_MATRIX_GREEN_BIAS   0x80B9
#define GL_POST_COLOR_MATRIX_BLUE_BIAS    0x80BA
#define GL_POST_COLOR_MATRIX_ALPHA_BIAS   0x80BB
#define GL_COLOR_TABLE                    0x80D0
#define GL_POST_CONVOLUTION_COLOR_TABLE   0x80D1
#define GL_POST_COLOR_MATRIX_COLOR_TABLE  0x80D2
#define GL_PROXY_COLOR_TABLE              0x80D3
#define GL_PROXY_POST_CONVOLUTION_COLOR_TABLE 0x80D4
#define GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE 0x80D5
#define GL_COLOR_TABLE_SCALE              0x80D6
#define GL_COLOR_TABLE_BIAS               0x80D7
#define GL_COLOR_TABLE_FORMAT             0x80D8
#define GL_COLOR_TABLE_WIDTH              0x80D9
#define GL_COLOR_TABLE_RED_SIZE           0x80DA
#define GL_COLOR_TABLE_GREEN_SIZE         0x80DB
#define GL_COLOR_TABLE_BLUE_SIZE          0x80DC
#define GL_COLOR_TABLE_ALPHA_SIZE         0x80DD
#define GL_COLOR_TABLE_LUMINANCE_SIZE     0x80DE
#define GL_COLOR_TABLE_INTENSITY_SIZE     0x80DF
#define GL_CONSTANT_BORDER                0x8151
#define GL_REPLICATE_BORDER               0x8153
#define GL_CONVOLUTION_BORDER_COLOR       0x8154
#endif

#ifndef GL_VERSION_1_3
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_TEXTURE16                      0x84D0
#define GL_TEXTURE17                      0x84D1
#define GL_TEXTURE18                      0x84D2
#define GL_TEXTURE19                      0x84D3
#define GL_TEXTURE20                      0x84D4
#define GL_TEXTURE21                      0x84D5
#define GL_TEXTURE22                      0x84D6
#define GL_TEXTURE23                      0x84D7
#define GL_TEXTURE24                      0x84D8
#define GL_TEXTURE25                      0x84D9
#define GL_TEXTURE26                      0x84DA
#define GL_TEXTURE27                      0x84DB
#define GL_TEXTURE28                      0x84DC
#define GL_TEXTURE29                      0x84DD
#define GL_TEXTURE30                      0x84DE
#define GL_TEXTURE31                      0x84DF
#define GL_ACTIVE_TEXTURE                 0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE          0x84E1
#define GL_MAX_TEXTURE_UNITS              0x84E2
#define GL_TRANSPOSE_MODELVIEW_MATRIX     0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX    0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX       0x84E5
#define GL_TRANSPOSE_COLOR_MATRIX         0x84E6
#define GL_MULTISAMPLE                    0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_ALPHA_TO_ONE            0x809F
#define GL_SAMPLE_COVERAGE                0x80A0
#define GL_SAMPLE_BUFFERS                 0x80A8
#define GL_SAMPLES                        0x80A9
#define GL_SAMPLE_COVERAGE_VALUE          0x80AA
#define GL_SAMPLE_COVERAGE_INVERT         0x80AB
#define GL_MULTISAMPLE_BIT                0x20000000
#define GL_NORMAL_MAP                     0x8511
#define GL_REFLECTION_MAP                 0x8512
#define GL_TEXTURE_CUBE_MAP               0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP       0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP         0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE      0x851C
#define GL_COMPRESSED_ALPHA               0x84E9
#define GL_COMPRESSED_LUMINANCE           0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA     0x84EB
#define GL_COMPRESSED_INTENSITY           0x84EC
#define GL_COMPRESSED_RGB                 0x84ED
#define GL_COMPRESSED_RGBA                0x84EE
#define GL_TEXTURE_COMPRESSION_HINT       0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE  0x86A0
#define GL_TEXTURE_COMPRESSED             0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS     0x86A3
#define GL_CLAMP_TO_BORDER                0x812D
#define GL_CLAMP_TO_BORDER_SGIS           0x812D
#define GL_COMBINE                        0x8570
#define GL_COMBINE_RGB                    0x8571
#define GL_COMBINE_ALPHA                  0x8572
#define GL_SOURCE0_RGB                    0x8580
#define GL_SOURCE1_RGB                    0x8581
#define GL_SOURCE2_RGB                    0x8582
#define GL_SOURCE0_ALPHA                  0x8588
#define GL_SOURCE1_ALPHA                  0x8589
#define GL_SOURCE2_ALPHA                  0x858A
#define GL_OPERAND0_RGB                   0x8590
#define GL_OPERAND1_RGB                   0x8591
#define GL_OPERAND2_RGB                   0x8592
#define GL_OPERAND0_ALPHA                 0x8598
#define GL_OPERAND1_ALPHA                 0x8599
#define GL_OPERAND2_ALPHA                 0x859A
#define GL_RGB_SCALE                      0x8573
#define GL_ADD_SIGNED                     0x8574
#define GL_INTERPOLATE                    0x8575
#define GL_SUBTRACT                       0x84E7
#define GL_CONSTANT                       0x8576
#define GL_PRIMARY_COLOR                  0x8577
#define GL_PREVIOUS                       0x8578
#define GL_DOT3_RGB                       0x86AE
#define GL_DOT3_RGBA                      0x86AF
#endif

#ifndef GL_VERSION_1_4
#define GL_BLEND_DST_RGB                  0x80C8
#define GL_BLEND_SRC_RGB                  0x80C9
#define GL_BLEND_DST_ALPHA                0x80CA
#define GL_BLEND_SRC_ALPHA                0x80CB
#define GL_POINT_SIZE_MIN                 0x8126
#define GL_POINT_SIZE_MAX                 0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE      0x8128
#define GL_POINT_DISTANCE_ATTENUATION     0x8129
#define GL_GENERATE_MIPMAP                0x8191
#define GL_GENERATE_MIPMAP_HINT           0x8192
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_MIRRORED_REPEAT                0x8370
#define GL_FOG_COORDINATE_SOURCE          0x8450
#define GL_FOG_COORDINATE                 0x8451
#define GL_FRAGMENT_DEPTH                 0x8452
#define GL_CURRENT_FOG_COORDINATE         0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE      0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE    0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER   0x8456
#define GL_FOG_COORDINATE_ARRAY           0x8457
#define GL_COLOR_SUM                      0x8458
#define GL_CURRENT_SECONDARY_COLOR        0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE     0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE     0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE   0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER  0x845D
#define GL_SECONDARY_COLOR_ARRAY          0x845E
#define GL_MAX_TEXTURE_LOD_BIAS           0x84FD
#define GL_TEXTURE_FILTER_CONTROL         0x8500
#define GL_TEXTURE_LOD_BIAS               0x8501
#define GL_INCR_WRAP                      0x8507
#define GL_DECR_WRAP                      0x8508
#define GL_TEXTURE_DEPTH_SIZE             0x884A
#define GL_DEPTH_TEXTURE_MODE             0x884B
#define GL_TEXTURE_COMPARE_MODE           0x884C
#define GL_TEXTURE_COMPARE_FUNC           0x884D
#define GL_COMPARE_R_TO_TEXTURE           0x884E
#endif

#ifndef GL_ARB_multitexture
#define GL_TEXTURE0_ARB                   0x84C0
#define GL_TEXTURE1_ARB                   0x84C1
#define GL_TEXTURE2_ARB                   0x84C2
#define GL_TEXTURE3_ARB                   0x84C3
#define GL_TEXTURE4_ARB                   0x84C4
#define GL_TEXTURE5_ARB                   0x84C5
#define GL_TEXTURE6_ARB                   0x84C6
#define GL_TEXTURE7_ARB                   0x84C7
#define GL_TEXTURE8_ARB                   0x84C8
#define GL_TEXTURE9_ARB                   0x84C9
#define GL_TEXTURE10_ARB                  0x84CA
#define GL_TEXTURE11_ARB                  0x84CB
#define GL_TEXTURE12_ARB                  0x84CC
#define GL_TEXTURE13_ARB                  0x84CD
#define GL_TEXTURE14_ARB                  0x84CE
#define GL_TEXTURE15_ARB                  0x84CF
#define GL_TEXTURE16_ARB                  0x84D0
#define GL_TEXTURE17_ARB                  0x84D1
#define GL_TEXTURE18_ARB                  0x84D2
#define GL_TEXTURE19_ARB                  0x84D3
#define GL_TEXTURE20_ARB                  0x84D4
#define GL_TEXTURE21_ARB                  0x84D5
#define GL_TEXTURE22_ARB                  0x84D6
#define GL_TEXTURE23_ARB                  0x84D7
#define GL_TEXTURE24_ARB                  0x84D8
#define GL_TEXTURE25_ARB                  0x84D9
#define GL_TEXTURE26_ARB                  0x84DA
#define GL_TEXTURE27_ARB                  0x84DB
#define GL_TEXTURE28_ARB                  0x84DC
#define GL_TEXTURE29_ARB                  0x84DD
#define GL_TEXTURE30_ARB                  0x84DE
#define GL_TEXTURE31_ARB                  0x84DF
#define GL_ACTIVE_TEXTURE_ARB             0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE_ARB      0x84E1
#define GL_MAX_TEXTURE_UNITS_ARB          0x84E2
#endif

#ifndef GL_ARB_transpose_matrix
#define GL_TRANSPOSE_MODELVIEW_MATRIX_ARB 0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX_ARB 0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX_ARB   0x84E5
#define GL_TRANSPOSE_COLOR_MATRIX_ARB     0x84E6
#endif

#ifndef GL_ARB_multisample
#define GL_MULTISAMPLE_ARB                0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE_ARB   0x809E
#define GL_SAMPLE_ALPHA_TO_ONE_ARB        0x809F
#define GL_SAMPLE_COVERAGE_ARB            0x80A0
#define GL_SAMPLE_BUFFERS_ARB             0x80A8
#define GL_SAMPLES_ARB                    0x80A9
#define GL_SAMPLE_COVERAGE_VALUE_ARB      0x80AA
#define GL_SAMPLE_COVERAGE_INVERT_ARB     0x80AB
#define GL_MULTISAMPLE_BIT_ARB            0x20000000
#endif

#ifndef GL_ARB_texture_env_add
#endif

#ifndef GL_ARB_texture_cube_map
#define GL_NORMAL_MAP_ARB                 0x8511
#define GL_REFLECTION_MAP_ARB             0x8512
#define GL_TEXTURE_CUBE_MAP_ARB           0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_ARB   0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARB     0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB  0x851C
#endif

#ifndef GL_ARB_texture_compression
#define GL_COMPRESSED_ALPHA_ARB           0x84E9
#define GL_COMPRESSED_LUMINANCE_ARB       0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA_ARB 0x84EB
#define GL_COMPRESSED_INTENSITY_ARB       0x84EC
#define GL_COMPRESSED_RGB_ARB             0x84ED
#define GL_COMPRESSED_RGBA_ARB            0x84EE
#define GL_TEXTURE_COMPRESSION_HINT_ARB   0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB 0x86A0
#define GL_TEXTURE_COMPRESSED_ARB         0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS_ARB 0x86A3
#endif

#ifndef GL_ARB_texture_border_clamp
#define GL_CLAMP_TO_BORDER_ARB            0x812D
#endif

#ifndef GL_ARB_point_parameters
#define GL_POINT_SIZE_MIN_ARB             0x8126
#define GL_POINT_SIZE_MAX_ARB             0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE_ARB  0x8128
#define GL_POINT_DISTANCE_ATTENUATION_ARB 0x8129
#endif

#ifndef GL_ARB_vertex_blend
#define GL_MAX_VERTEX_UNITS_ARB           0x86A4
#define GL_ACTIVE_VERTEX_UNITS_ARB        0x86A5
#define GL_WEIGHT_SUM_UNITY_ARB           0x86A6
#define GL_VERTEX_BLEND_ARB               0x86A7
#define GL_CURRENT_WEIGHT_ARB             0x86A8
#define GL_WEIGHT_ARRAY_TYPE_ARB          0x86A9
#define GL_WEIGHT_ARRAY_STRIDE_ARB        0x86AA
#define GL_WEIGHT_ARRAY_SIZE_ARB          0x86AB
#define GL_WEIGHT_ARRAY_POINTER_ARB       0x86AC
#define GL_WEIGHT_ARRAY_ARB               0x86AD
#define GL_MODELVIEW0_ARB                 0x1700
#define GL_MODELVIEW1_ARB                 0x850A
#define GL_MODELVIEW2_ARB                 0x8722
#define GL_MODELVIEW3_ARB                 0x8723
#define GL_MODELVIEW4_ARB                 0x8724
#define GL_MODELVIEW5_ARB                 0x8725
#define GL_MODELVIEW6_ARB                 0x8726
#define GL_MODELVIEW7_ARB                 0x8727
#define GL_MODELVIEW8_ARB                 0x8728
#define GL_MODELVIEW9_ARB                 0x8729
#define GL_MODELVIEW10_ARB                0x872A
#define GL_MODELVIEW11_ARB                0x872B
#define GL_MODELVIEW12_ARB                0x872C
#define GL_MODELVIEW13_ARB                0x872D
#define GL_MODELVIEW14_ARB                0x872E
#define GL_MODELVIEW15_ARB                0x872F
#define GL_MODELVIEW16_ARB                0x8730
#define GL_MODELVIEW17_ARB                0x8731
#define GL_MODELVIEW18_ARB                0x8732
#define GL_MODELVIEW19_ARB                0x8733
#define GL_MODELVIEW20_ARB                0x8734
#define GL_MODELVIEW21_ARB                0x8735
#define GL_MODELVIEW22_ARB                0x8736
#define GL_MODELVIEW23_ARB                0x8737
#define GL_MODELVIEW24_ARB                0x8738
#define GL_MODELVIEW25_ARB                0x8739
#define GL_MODELVIEW26_ARB                0x873A
#define GL_MODELVIEW27_ARB                0x873B
#define GL_MODELVIEW28_ARB                0x873C
#define GL_MODELVIEW29_ARB                0x873D
#define GL_MODELVIEW30_ARB                0x873E
#define GL_MODELVIEW31_ARB                0x873F
#endif

#ifndef GL_ARB_matrix_palette
#define GL_MATRIX_PALETTE_ARB             0x8840
#define GL_MAX_MATRIX_PALETTE_STACK_DEPTH_ARB 0x8841
#define GL_MAX_PALETTE_MATRICES_ARB       0x8842
#define GL_CURRENT_PALETTE_MATRIX_ARB     0x8843
#define GL_MATRIX_INDEX_ARRAY_ARB         0x8844
#define GL_CURRENT_MATRIX_INDEX_ARB       0x8845
#define GL_MATRIX_INDEX_ARRAY_SIZE_ARB    0x8846
#define GL_MATRIX_INDEX_ARRAY_TYPE_ARB    0x8847
#define GL_MATRIX_INDEX_ARRAY_STRIDE_ARB  0x8848
#define GL_MATRIX_INDEX_ARRAY_POINTER_ARB 0x8849
#endif

#ifndef GL_ARB_texture_env_combine
#define GL_COMBINE_ARB                    0x8570
#define GL_COMBINE_RGB_ARB                0x8571
#define GL_COMBINE_ALPHA_ARB              0x8572
#define GL_SOURCE0_RGB_ARB                0x8580
#define GL_SOURCE1_RGB_ARB                0x8581
#define GL_SOURCE2_RGB_ARB                0x8582
#define GL_SOURCE0_ALPHA_ARB              0x8588
#define GL_SOURCE1_ALPHA_ARB              0x8589
#define GL_SOURCE2_ALPHA_ARB              0x858A
#define GL_OPERAND0_RGB_ARB               0x8590
#define GL_OPERAND1_RGB_ARB               0x8591
#define GL_OPERAND2_RGB_ARB               0x8592
#define GL_OPERAND0_ALPHA_ARB             0x8598
#define GL_OPERAND1_ALPHA_ARB             0x8599
#define GL_OPERAND2_ALPHA_ARB             0x859A
#define GL_RGB_SCALE_ARB                  0x8573
#define GL_ADD_SIGNED_ARB                 0x8574
#define GL_INTERPOLATE_ARB                0x8575
#define GL_SUBTRACT_ARB                   0x84E7
#define GL_CONSTANT_ARB                   0x8576
#define GL_PRIMARY_COLOR_ARB              0x8577
#define GL_PREVIOUS_ARB                   0x8578
#endif

#ifndef GL_ARB_texture_env_crossbar
#endif

#ifndef GL_ARB_texture_env_dot3
#define GL_DOT3_RGB_ARB                   0x86AE
#define GL_DOT3_RGBA_ARB                  0x86AF
#endif

#ifndef GL_ARB_texture_mirrored_repeat
#define GL_MIRRORED_REPEAT_ARB            0x8370
#endif

#ifndef GL_ARB_depth_texture
#define GL_DEPTH_COMPONENT16_ARB          0x81A5
#define GL_DEPTH_COMPONENT24_ARB          0x81A6
#define GL_DEPTH_COMPONENT32_ARB          0x81A7
#define GL_TEXTURE_DEPTH_SIZE_ARB         0x884A
#define GL_DEPTH_TEXTURE_MODE_ARB         0x884B
#endif

#ifndef GL_ARB_shadow
#define GL_TEXTURE_COMPARE_MODE_ARB       0x884C
#define GL_TEXTURE_COMPARE_FUNC_ARB       0x884D
#define GL_COMPARE_R_TO_TEXTURE_ARB       0x884E
#endif

#ifndef GL_ARB_shadow_ambient
#define GL_TEXTURE_COMPARE_FAIL_VALUE_ARB 0x80BF
#endif

#ifndef GL_ARB_window_pos
#endif

#ifndef GL_ARB_vertex_program
#define GL_COLOR_SUM_ARB                  0x8458
#define GL_VERTEX_PROGRAM_ARB             0x8620
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB   0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB   0x8625
#define GL_CURRENT_VERTEX_ATTRIB_ARB      0x8626
#define GL_PROGRAM_LENGTH_ARB             0x8627
#define GL_PROGRAM_STRING_ARB             0x8628
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB 0x862E
#define GL_MAX_PROGRAM_MATRICES_ARB       0x862F
#define GL_CURRENT_MATRIX_STACK_DEPTH_ARB 0x8640
#define GL_CURRENT_MATRIX_ARB             0x8641
#define GL_VERTEX_PROGRAM_POINT_SIZE_ARB  0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_ARB    0x8643
#define GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB 0x8645
#define GL_PROGRAM_ERROR_POSITION_ARB     0x864B
#define GL_MAX_VERTEX_ATTRIBS_ARB         0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB 0x886A
#define GL_PROGRAM_ERROR_STRING_ARB       0x8874
#define GL_PROGRAM_FORMAT_ASCII_ARB       0x8875
#define GL_PROGRAM_FORMAT_ARB             0x8876
#define GL_PROGRAM_BINDING_ARB            0x8677
#define GL_PROGRAM_INSTRUCTIONS_ARB       0x88A0
#define GL_MAX_PROGRAM_INSTRUCTIONS_ARB   0x88A1
#define GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB 0x88A2
#define GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB 0x88A3
#define GL_PROGRAM_TEMPORARIES_ARB        0x88A4
#define GL_MAX_PROGRAM_TEMPORARIES_ARB    0x88A5
#define GL_PROGRAM_NATIVE_TEMPORARIES_ARB 0x88A6
#define GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB 0x88A7
#define GL_PROGRAM_PARAMETERS_ARB         0x88A8
#define GL_MAX_PROGRAM_PARAMETERS_ARB     0x88A9
#define GL_PROGRAM_NATIVE_PARAMETERS_ARB  0x88AA
#define GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB 0x88AB
#define GL_PROGRAM_ATTRIBS_ARB            0x88AC
#define GL_MAX_PROGRAM_ATTRIBS_ARB        0x88AD
#define GL_PROGRAM_NATIVE_ATTRIBS_ARB     0x88AE
#define GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB 0x88AF
#define GL_PROGRAM_ADDRESS_REGISTERS_ARB  0x88B0
#define GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB 0x88B1
#define GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB 0x88B2
#define GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB 0x88B3
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB 0x88B4
#define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB 0x88B5
#define GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB 0x88B6
#define GL_TRANSPOSE_CURRENT_MATRIX_ARB   0x88B7
#define GL_MATRIX0_ARB                    0x88C0
#define GL_MATRIX1_ARB                    0x88C1
#define GL_MATRIX2_ARB                    0x88C2
#define GL_MATRIX3_ARB                    0x88C3
#define GL_MATRIX4_ARB                    0x88C4
#define GL_MATRIX5_ARB                    0x88C5
#define GL_MATRIX6_ARB                    0x88C6
#define GL_MATRIX7_ARB                    0x88C7
#define GL_MATRIX8_ARB                    0x88C8
#define GL_MATRIX9_ARB                    0x88C9
#define GL_MATRIX10_ARB                   0x88CA
#define GL_MATRIX11_ARB                   0x88CB
#define GL_MATRIX12_ARB                   0x88CC
#define GL_MATRIX13_ARB                   0x88CD
#define GL_MATRIX14_ARB                   0x88CE
#define GL_MATRIX15_ARB                   0x88CF
#define GL_MATRIX16_ARB                   0x88D0
#define GL_MATRIX17_ARB                   0x88D1
#define GL_MATRIX18_ARB                   0x88D2
#define GL_MATRIX19_ARB                   0x88D3
#define GL_MATRIX20_ARB                   0x88D4
#define GL_MATRIX21_ARB                   0x88D5
#define GL_MATRIX22_ARB                   0x88D6
#define GL_MATRIX23_ARB                   0x88D7
#define GL_MATRIX24_ARB                   0x88D8
#define GL_MATRIX25_ARB                   0x88D9
#define GL_MATRIX26_ARB                   0x88DA
#define GL_MATRIX27_ARB                   0x88DB
#define GL_MATRIX28_ARB                   0x88DC
#define GL_MATRIX29_ARB                   0x88DD
#define GL_MATRIX30_ARB                   0x88DE
#define GL_MATRIX31_ARB                   0x88DF
#endif


#ifndef GL_ARB_fragment_program
#define GL_FRAGMENT_PROGRAM_ARB           0x8804
#define GL_PROGRAM_ALU_INSTRUCTIONS_ARB   0x8805
#define GL_PROGRAM_TEX_INSTRUCTIONS_ARB   0x8806
#define GL_PROGRAM_TEX_INDIRECTIONS_ARB   0x8807
#define GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x8808
#define GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x8809
#define GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x880A
#define GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB 0x880B
#define GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB 0x880C
#define GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB 0x880D
#define GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x880E
#define GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x880F
#define GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x8810
#define GL_MAX_TEXTURE_COORDS_ARB         0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB    0x8872
#endif

#ifndef GL_ARB_vertex_buffer_object
#define GL_BUFFER_SIZE_ARB                0x8764
#define GL_BUFFER_USAGE_ARB               0x8765
#define GL_ARRAY_BUFFER_ARB               0x8892
#define GL_ELEMENT_ARRAY_BUFFER_ARB       0x8893
#define GL_ARRAY_BUFFER_BINDING_ARB       0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB 0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING_ARB 0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING_ARB 0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING_ARB 0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING_ARB 0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB 0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB 0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB 0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB 0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB 0x889F
#define GL_READ_ONLY_ARB                  0x88B8
#define GL_WRITE_ONLY_ARB                 0x88B9
#define GL_READ_WRITE_ARB                 0x88BA
#define GL_BUFFER_ACCESS_ARB              0x88BB
#define GL_BUFFER_MAPPED_ARB              0x88BC
#define GL_BUFFER_MAP_POINTER_ARB         0x88BD
#define GL_STREAM_DRAW_ARB                0x88E0
#define GL_STREAM_READ_ARB                0x88E1
#define GL_STREAM_COPY_ARB                0x88E2
#define GL_STATIC_DRAW_ARB                0x88E4
#define GL_STATIC_READ_ARB                0x88E5
#define GL_STATIC_COPY_ARB                0x88E6
#define GL_DYNAMIC_DRAW_ARB               0x88E8
#define GL_DYNAMIC_READ_ARB               0x88E9
#define GL_DYNAMIC_COPY_ARB               0x88EA
#endif


#ifndef GL_EXT_abgr
#define GL_ABGR_EXT                       0x8000
#endif

#ifndef GL_EXT_blend_color
#define GL_CONSTANT_COLOR_EXT             0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR_EXT   0x8002
#define GL_CONSTANT_ALPHA_EXT             0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA_EXT   0x8004
#define GL_BLEND_COLOR_EXT                0x8005
#endif

#ifndef GL_EXT_polygon_offset
#define GL_POLYGON_OFFSET_EXT             0x8037
#define GL_POLYGON_OFFSET_FACTOR_EXT      0x8038
#define GL_POLYGON_OFFSET_BIAS_EXT        0x8039
#endif

#ifndef GL_EXT_texture
#define GL_ALPHA4_EXT                     0x803B
#define GL_ALPHA8_EXT                     0x803C
#define GL_ALPHA12_EXT                    0x803D
#define GL_ALPHA16_EXT                    0x803E
#define GL_LUMINANCE4_EXT                 0x803F
#define GL_LUMINANCE8_EXT                 0x8040
#define GL_LUMINANCE12_EXT                0x8041
#define GL_LUMINANCE16_EXT                0x8042
#define GL_LUMINANCE4_ALPHA4_EXT          0x8043
#define GL_LUMINANCE6_ALPHA2_EXT          0x8044
#define GL_LUMINANCE8_ALPHA8_EXT          0x8045
#define GL_LUMINANCE12_ALPHA4_EXT         0x8046
#define GL_LUMINANCE12_ALPHA12_EXT        0x8047
#define GL_LUMINANCE16_ALPHA16_EXT        0x8048
#define GL_INTENSITY_EXT                  0x8049
#define GL_INTENSITY4_EXT                 0x804A
#define GL_INTENSITY8_EXT                 0x804B
#define GL_INTENSITY12_EXT                0x804C
#define GL_INTENSITY16_EXT                0x804D
#define GL_RGB2_EXT                       0x804E
#define GL_RGB4_EXT                       0x804F
#define GL_RGB5_EXT                       0x8050
#define GL_RGB8_EXT                       0x8051
#define GL_RGB10_EXT                      0x8052
#define GL_RGB12_EXT                      0x8053
#define GL_RGB16_EXT                      0x8054
#define GL_RGBA2_EXT                      0x8055
#define GL_RGBA4_EXT                      0x8056
#define GL_RGB5_A1_EXT                    0x8057
#define GL_RGBA8_EXT                      0x8058
#define GL_RGB10_A2_EXT                   0x8059
#define GL_RGBA12_EXT                     0x805A
#define GL_RGBA16_EXT                     0x805B
#define GL_TEXTURE_RED_SIZE_EXT           0x805C
#define GL_TEXTURE_GREEN_SIZE_EXT         0x805D
#define GL_TEXTURE_BLUE_SIZE_EXT          0x805E
#define GL_TEXTURE_ALPHA_SIZE_EXT         0x805F
#define GL_TEXTURE_LUMINANCE_SIZE_EXT     0x8060
#define GL_TEXTURE_INTENSITY_SIZE_EXT     0x8061
#define GL_REPLACE_EXT                    0x8062
#define GL_PROXY_TEXTURE_1D_EXT           0x8063
#define GL_PROXY_TEXTURE_2D_EXT           0x8064
#define GL_TEXTURE_TOO_LARGE_EXT          0x8065
#endif

#ifndef GL_EXT_texture3D
#define GL_PACK_SKIP_IMAGES_EXT           0x806B
#define GL_PACK_IMAGE_HEIGHT_EXT          0x806C
#define GL_UNPACK_SKIP_IMAGES_EXT         0x806D
#define GL_UNPACK_IMAGE_HEIGHT_EXT        0x806E
#define GL_TEXTURE_3D_EXT                 0x806F
#define GL_PROXY_TEXTURE_3D_EXT           0x8070
#define GL_TEXTURE_DEPTH_EXT              0x8071
#define GL_TEXTURE_WRAP_R_EXT             0x8072
#define GL_MAX_3D_TEXTURE_SIZE_EXT        0x8073
#endif

#ifndef GL_SGIS_texture_filter4
#define GL_FILTER4_SGIS                   0x8146
#define GL_TEXTURE_FILTER4_SIZE_SGIS      0x8147
#endif

#ifndef GL_EXT_subtexture
#endif

#ifndef GL_EXT_copy_texture
#endif

#ifndef GL_EXT_histogram
#define GL_HISTOGRAM_EXT                  0x8024
#define GL_PROXY_HISTOGRAM_EXT            0x8025
#define GL_HISTOGRAM_WIDTH_EXT            0x8026
#define GL_HISTOGRAM_FORMAT_EXT           0x8027
#define GL_HISTOGRAM_RED_SIZE_EXT         0x8028
#define GL_HISTOGRAM_GREEN_SIZE_EXT       0x8029
#define GL_HISTOGRAM_BLUE_SIZE_EXT        0x802A
#define GL_HISTOGRAM_ALPHA_SIZE_EXT       0x802B
#define GL_HISTOGRAM_LUMINANCE_SIZE_EXT   0x802C
#define GL_HISTOGRAM_SINK_EXT             0x802D
#define GL_MINMAX_EXT                     0x802E
#define GL_MINMAX_FORMAT_EXT              0x802F
#define GL_MINMAX_SINK_EXT                0x8030
#define GL_TABLE_TOO_LARGE_EXT            0x8031
#endif

#ifndef GL_EXT_convolution
#define GL_CONVOLUTION_1D_EXT             0x8010
#define GL_CONVOLUTION_2D_EXT             0x8011
#define GL_SEPARABLE_2D_EXT               0x8012
#define GL_CONVOLUTION_BORDER_MODE_EXT    0x8013
#define GL_CONVOLUTION_FILTER_SCALE_EXT   0x8014
#define GL_CONVOLUTION_FILTER_BIAS_EXT    0x8015
#define GL_REDUCE_EXT                     0x8016
#define GL_CONVOLUTION_FORMAT_EXT         0x8017
#define GL_CONVOLUTION_WIDTH_EXT          0x8018
#define GL_CONVOLUTION_HEIGHT_EXT         0x8019
#define GL_MAX_CONVOLUTION_WIDTH_EXT      0x801A
#define GL_MAX_CONVOLUTION_HEIGHT_EXT     0x801B
#define GL_POST_CONVOLUTION_RED_SCALE_EXT 0x801C
#define GL_POST_CONVOLUTION_GREEN_SCALE_EXT 0x801D
#define GL_POST_CONVOLUTION_BLUE_SCALE_EXT 0x801E
#define GL_POST_CONVOLUTION_ALPHA_SCALE_EXT 0x801F
#define GL_POST_CONVOLUTION_RED_BIAS_EXT  0x8020
#define GL_POST_CONVOLUTION_GREEN_BIAS_EXT 0x8021
#define GL_POST_CONVOLUTION_BLUE_BIAS_EXT 0x8022
#define GL_POST_CONVOLUTION_ALPHA_BIAS_EXT 0x8023
#endif

#ifndef GL_SGI_color_matrix
#define GL_COLOR_MATRIX_SGI               0x80B1
#define GL_COLOR_MATRIX_STACK_DEPTH_SGI   0x80B2
#define GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI 0x80B3
#define GL_POST_COLOR_MATRIX_RED_SCALE_SGI 0x80B4
#define GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI 0x80B5
#define GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI 0x80B6
#define GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI 0x80B7
#define GL_POST_COLOR_MATRIX_RED_BIAS_SGI 0x80B8
#define GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI 0x80B9
#define GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI 0x80BA
#define GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI 0x80BB
#endif

#ifndef GL_SGI_color_table
#define GL_COLOR_TABLE_SGI                0x80D0
#define GL_POST_CONVOLUTION_COLOR_TABLE_SGI 0x80D1
#define GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI 0x80D2
#define GL_PROXY_COLOR_TABLE_SGI          0x80D3
#define GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI 0x80D4
#define GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI 0x80D5
#define GL_COLOR_TABLE_SCALE_SGI          0x80D6
#define GL_COLOR_TABLE_BIAS_SGI           0x80D7
#define GL_COLOR_TABLE_FORMAT_SGI         0x80D8
#define GL_COLOR_TABLE_WIDTH_SGI          0x80D9
#define GL_COLOR_TABLE_RED_SIZE_SGI       0x80DA
#define GL_COLOR_TABLE_GREEN_SIZE_SGI     0x80DB
#define GL_COLOR_TABLE_BLUE_SIZE_SGI      0x80DC
#define GL_COLOR_TABLE_ALPHA_SIZE_SGI     0x80DD
#define GL_COLOR_TABLE_LUMINANCE_SIZE_SGI 0x80DE
#define GL_COLOR_TABLE_INTENSITY_SIZE_SGI 0x80DF
#endif

#ifndef GL_SGIS_pixel_texture
#define GL_PIXEL_TEXTURE_SGIS             0x8353
#define GL_PIXEL_FRAGMENT_RGB_SOURCE_SGIS 0x8354
#define GL_PIXEL_FRAGMENT_ALPHA_SOURCE_SGIS 0x8355
#define GL_PIXEL_GROUP_COLOR_SGIS         0x8356
#endif

#ifndef GL_SGIX_pixel_texture
#define GL_PIXEL_TEX_GEN_SGIX             0x8139
#define GL_PIXEL_TEX_GEN_MODE_SGIX        0x832B
#endif

#ifndef GL_SGIS_texture4D
#define GL_PACK_SKIP_VOLUMES_SGIS         0x8130
#define GL_PACK_IMAGE_DEPTH_SGIS          0x8131
#define GL_UNPACK_SKIP_VOLUMES_SGIS       0x8132
#define GL_UNPACK_IMAGE_DEPTH_SGIS        0x8133
#define GL_TEXTURE_4D_SGIS                0x8134
#define GL_PROXY_TEXTURE_4D_SGIS          0x8135
#define GL_TEXTURE_4DSIZE_SGIS            0x8136
#define GL_TEXTURE_WRAP_Q_SGIS            0x8137
#define GL_MAX_4D_TEXTURE_SIZE_SGIS       0x8138
#define GL_TEXTURE_4D_BINDING_SGIS        0x814F
#endif

#ifndef GL_SGI_texture_color_table
#define GL_TEXTURE_COLOR_TABLE_SGI        0x80BC
#define GL_PROXY_TEXTURE_COLOR_TABLE_SGI  0x80BD
#endif

#ifndef GL_EXT_cmyka
#define GL_CMYK_EXT                       0x800C
#define GL_CMYKA_EXT                      0x800D
#define GL_PACK_CMYK_HINT_EXT             0x800E
#define GL_UNPACK_CMYK_HINT_EXT           0x800F
#endif

#ifndef GL_EXT_texture_object
#define GL_TEXTURE_PRIORITY_EXT           0x8066
#define GL_TEXTURE_RESIDENT_EXT           0x8067
#define GL_TEXTURE_1D_BINDING_EXT         0x8068
#define GL_TEXTURE_2D_BINDING_EXT         0x8069
#define GL_TEXTURE_3D_BINDING_EXT         0x806A
#endif

#ifndef GL_SGIS_detail_texture
#define GL_DETAIL_TEXTURE_2D_SGIS         0x8095
#define GL_DETAIL_TEXTURE_2D_BINDING_SGIS 0x8096
#define GL_LINEAR_DETAIL_SGIS             0x8097
#define GL_LINEAR_DETAIL_ALPHA_SGIS       0x8098
#define GL_LINEAR_DETAIL_COLOR_SGIS       0x8099
#define GL_DETAIL_TEXTURE_LEVEL_SGIS      0x809A
#define GL_DETAIL_TEXTURE_MODE_SGIS       0x809B
#define GL_DETAIL_TEXTURE_FUNC_POINTS_SGIS 0x809C
#endif

#ifndef GL_SGIS_sharpen_texture
#define GL_LINEAR_SHARPEN_SGIS            0x80AD
#define GL_LINEAR_SHARPEN_ALPHA_SGIS      0x80AE
#define GL_LINEAR_SHARPEN_COLOR_SGIS      0x80AF
#define GL_SHARPEN_TEXTURE_FUNC_POINTS_SGIS 0x80B0
#endif

#ifndef GL_EXT_packed_pixels
#define GL_UNSIGNED_BYTE_3_3_2_EXT        0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4_EXT     0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1_EXT     0x8034
#define GL_UNSIGNED_INT_8_8_8_8_EXT       0x8035
#define GL_UNSIGNED_INT_10_10_10_2_EXT    0x8036
#endif

#ifndef GL_SGIS_texture_lod
#define GL_TEXTURE_MIN_LOD_SGIS           0x813A
#define GL_TEXTURE_MAX_LOD_SGIS           0x813B
#define GL_TEXTURE_BASE_LEVEL_SGIS        0x813C
#define GL_TEXTURE_MAX_LEVEL_SGIS         0x813D
#endif

#ifndef GL_SGIS_multisample
#define GL_MULTISAMPLE_SGIS               0x809D
#define GL_SAMPLE_ALPHA_TO_MASK_SGIS      0x809E
#define GL_SAMPLE_ALPHA_TO_ONE_SGIS       0x809F
#define GL_SAMPLE_MASK_SGIS               0x80A0
#define GL_1PASS_SGIS                     0x80A1
#define GL_2PASS_0_SGIS                   0x80A2
#define GL_2PASS_1_SGIS                   0x80A3
#define GL_4PASS_0_SGIS                   0x80A4
#define GL_4PASS_1_SGIS                   0x80A5
#define GL_4PASS_2_SGIS                   0x80A6
#define GL_4PASS_3_SGIS                   0x80A7
#define GL_SAMPLE_BUFFERS_SGIS            0x80A8
#define GL_SAMPLES_SGIS                   0x80A9
#define GL_SAMPLE_MASK_VALUE_SGIS         0x80AA
#define GL_SAMPLE_MASK_INVERT_SGIS        0x80AB
#define GL_SAMPLE_PATTERN_SGIS            0x80AC
#endif

#ifndef GL_EXT_rescale_normal
#define GL_RESCALE_NORMAL_EXT             0x803A
#endif

#ifndef GL_EXT_vertex_array
#define GL_VERTEX_ARRAY_EXT               0x8074
#define GL_NORMAL_ARRAY_EXT               0x8075
#define GL_COLOR_ARRAY_EXT                0x8076
#define GL_INDEX_ARRAY_EXT                0x8077
#define GL_TEXTURE_COORD_ARRAY_EXT        0x8078
#define GL_EDGE_FLAG_ARRAY_EXT            0x8079
#define GL_VERTEX_ARRAY_SIZE_EXT          0x807A
#define GL_VERTEX_ARRAY_TYPE_EXT          0x807B
#define GL_VERTEX_ARRAY_STRIDE_EXT        0x807C
#define GL_VERTEX_ARRAY_COUNT_EXT         0x807D
#define GL_NORMAL_ARRAY_TYPE_EXT          0x807E
#define GL_NORMAL_ARRAY_STRIDE_EXT        0x807F
#define GL_NORMAL_ARRAY_COUNT_EXT         0x8080
#define GL_COLOR_ARRAY_SIZE_EXT           0x8081
#define GL_COLOR_ARRAY_TYPE_EXT           0x8082
#define GL_COLOR_ARRAY_STRIDE_EXT         0x8083
#define GL_COLOR_ARRAY_COUNT_EXT          0x8084
#define GL_INDEX_ARRAY_TYPE_EXT           0x8085
#define GL_INDEX_ARRAY_STRIDE_EXT         0x8086
#define GL_INDEX_ARRAY_COUNT_EXT          0x8087
#define GL_TEXTURE_COORD_ARRAY_SIZE_EXT   0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE_EXT   0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE_EXT 0x808A
#define GL_TEXTURE_COORD_ARRAY_COUNT_EXT  0x808B
#define GL_EDGE_FLAG_ARRAY_STRIDE_EXT     0x808C
#define GL_EDGE_FLAG_ARRAY_COUNT_EXT      0x808D
#define GL_VERTEX_ARRAY_POINTER_EXT       0x808E
#define GL_NORMAL_ARRAY_POINTER_EXT       0x808F
#define GL_COLOR_ARRAY_POINTER_EXT        0x8090
#define GL_INDEX_ARRAY_POINTER_EXT        0x8091
#define GL_TEXTURE_COORD_ARRAY_POINTER_EXT 0x8092
#define GL_EDGE_FLAG_ARRAY_POINTER_EXT    0x8093
#endif

#ifndef GL_EXT_misc_attribute
#endif

#ifndef GL_SGIS_generate_mipmap
#define GL_GENERATE_MIPMAP_SGIS           0x8191
#define GL_GENERATE_MIPMAP_HINT_SGIS      0x8192
#endif

#ifndef GL_SGIX_clipmap
#define GL_LINEAR_CLIPMAP_LINEAR_SGIX     0x8170
#define GL_TEXTURE_CLIPMAP_CENTER_SGIX    0x8171
#define GL_TEXTURE_CLIPMAP_FRAME_SGIX     0x8172
#define GL_TEXTURE_CLIPMAP_OFFSET_SGIX    0x8173
#define GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX 0x8174
#define GL_TEXTURE_CLIPMAP_LOD_OFFSET_SGIX 0x8175
#define GL_TEXTURE_CLIPMAP_DEPTH_SGIX     0x8176
#define GL_MAX_CLIPMAP_DEPTH_SGIX         0x8177
#define GL_MAX_CLIPMAP_VIRTUAL_DEPTH_SGIX 0x8178
#define GL_NEAREST_CLIPMAP_NEAREST_SGIX   0x844D
#define GL_NEAREST_CLIPMAP_LINEAR_SGIX    0x844E
#define GL_LINEAR_CLIPMAP_NEAREST_SGIX    0x844F
#endif

#ifndef GL_SGIX_shadow
#define GL_TEXTURE_COMPARE_SGIX           0x819A
#define GL_TEXTURE_COMPARE_OPERATOR_SGIX  0x819B
#define GL_TEXTURE_LEQUAL_R_SGIX          0x819C
#define GL_TEXTURE_GEQUAL_R_SGIX          0x819D
#endif

#ifndef GL_SGIS_texture_edge_clamp
#define GL_CLAMP_TO_EDGE_SGIS             0x812F
#endif

#ifndef GL_EXT_blend_minmax
#define GL_FUNC_ADD_EXT                   0x8006
#define GL_MIN_EXT                        0x8007
#define GL_MAX_EXT                        0x8008
#define GL_BLEND_EQUATION_EXT             0x8009
#endif

#ifndef GL_EXT_blend_subtract
#define GL_FUNC_SUBTRACT_EXT              0x800A
#define GL_FUNC_REVERSE_SUBTRACT_EXT      0x800B
#endif

#ifndef GL_EXT_blend_logic_op
#endif

#ifndef GL_SGIX_interlace
#define GL_INTERLACE_SGIX                 0x8094
#endif

#ifndef GL_SGIX_pixel_tiles
#define GL_PIXEL_TILE_BEST_ALIGNMENT_SGIX 0x813E
#define GL_PIXEL_TILE_CACHE_INCREMENT_SGIX 0x813F
#define GL_PIXEL_TILE_WIDTH_SGIX          0x8140
#define GL_PIXEL_TILE_HEIGHT_SGIX         0x8141
#define GL_PIXEL_TILE_GRID_WIDTH_SGIX     0x8142
#define GL_PIXEL_TILE_GRID_HEIGHT_SGIX    0x8143
#define GL_PIXEL_TILE_GRID_DEPTH_SGIX     0x8144
#define GL_PIXEL_TILE_CACHE_SIZE_SGIX     0x8145
#endif

#ifndef GL_SGIS_texture_select
#define GL_DUAL_ALPHA4_SGIS               0x8110
#define GL_DUAL_ALPHA8_SGIS               0x8111
#define GL_DUAL_ALPHA12_SGIS              0x8112
#define GL_DUAL_ALPHA16_SGIS              0x8113
#define GL_DUAL_LUMINANCE4_SGIS           0x8114
#define GL_DUAL_LUMINANCE8_SGIS           0x8115
#define GL_DUAL_LUMINANCE12_SGIS          0x8116
#define GL_DUAL_LUMINANCE16_SGIS          0x8117
#define GL_DUAL_INTENSITY4_SGIS           0x8118
#define GL_DUAL_INTENSITY8_SGIS           0x8119
#define GL_DUAL_INTENSITY12_SGIS          0x811A
#define GL_DUAL_INTENSITY16_SGIS          0x811B
#define GL_DUAL_LUMINANCE_ALPHA4_SGIS     0x811C
#define GL_DUAL_LUMINANCE_ALPHA8_SGIS     0x811D
#define GL_QUAD_ALPHA4_SGIS               0x811E
#define GL_QUAD_ALPHA8_SGIS               0x811F
#define GL_QUAD_LUMINANCE4_SGIS           0x8120
#define GL_QUAD_LUMINANCE8_SGIS           0x8121
#define GL_QUAD_INTENSITY4_SGIS           0x8122
#define GL_QUAD_INTENSITY8_SGIS           0x8123
#define GL_DUAL_TEXTURE_SELECT_SGIS       0x8124
#define GL_QUAD_TEXTURE_SELECT_SGIS       0x8125
#endif

#ifndef GL_SGIX_sprite
#define GL_SPRITE_SGIX                    0x8148
#define GL_SPRITE_MODE_SGIX               0x8149
#define GL_SPRITE_AXIS_SGIX               0x814A
#define GL_SPRITE_TRANSLATION_SGIX        0x814B
#define GL_SPRITE_AXIAL_SGIX              0x814C
#define GL_SPRITE_OBJECT_ALIGNED_SGIX     0x814D
#define GL_SPRITE_EYE_ALIGNED_SGIX        0x814E
#endif

#ifndef GL_SGIX_texture_multi_buffer
#define GL_TEXTURE_MULTI_BUFFER_HINT_SGIX 0x812E
#endif

#ifndef GL_EXT_point_parameters
#define GL_POINT_SIZE_MIN_EXT             0x8126
#define GL_POINT_SIZE_MAX_EXT             0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE_EXT  0x8128
#define GL_DISTANCE_ATTENUATION_EXT       0x8129
#endif

#ifndef GL_SGIS_point_parameters
#define GL_POINT_SIZE_MIN_SGIS            0x8126
#define GL_POINT_SIZE_MAX_SGIS            0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE_SGIS 0x8128
#define GL_DISTANCE_ATTENUATION_SGIS      0x8129
#endif

#ifndef GL_SGIX_instruments
#define GL_INSTRUMENT_BUFFER_POINTER_SGIX 0x8180
#define GL_INSTRUMENT_MEASUREMENTS_SGIX   0x8181
#endif

#ifndef GL_SGIX_texture_scale_bias
#define GL_POST_TEXTURE_FILTER_BIAS_SGIX  0x8179
#define GL_POST_TEXTURE_FILTER_SCALE_SGIX 0x817A
#define GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX 0x817B
#define GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX 0x817C
#endif

#ifndef GL_SGIX_framezoom
#define GL_FRAMEZOOM_SGIX                 0x818B
#define GL_FRAMEZOOM_FACTOR_SGIX          0x818C
#define GL_MAX_FRAMEZOOM_FACTOR_SGIX      0x818D
#endif

#ifndef GL_SGIX_tag_sample_buffer
#endif

#ifndef GL_FfdMaskSGIX
#define GL_TEXTURE_DEFORMATION_BIT_SGIX   0x00000001
#define GL_GEOMETRY_DEFORMATION_BIT_SGIX  0x00000002
#endif

#ifndef GL_SGIX_polynomial_ffd
#define GL_GEOMETRY_DEFORMATION_SGIX      0x8194
#define GL_TEXTURE_DEFORMATION_SGIX       0x8195
#define GL_DEFORMATIONS_MASK_SGIX         0x8196
#define GL_MAX_DEFORMATION_ORDER_SGIX     0x8197
#endif

#ifndef GL_SGIX_reference_plane
#define GL_REFERENCE_PLANE_SGIX           0x817D
#define GL_REFERENCE_PLANE_EQUATION_SGIX  0x817E
#endif

#ifndef GL_SGIX_flush_raster
#endif

#ifndef GL_SGIX_depth_texture
#define GL_DEPTH_COMPONENT16_SGIX         0x81A5
#define GL_DEPTH_COMPONENT24_SGIX         0x81A6
#define GL_DEPTH_COMPONENT32_SGIX         0x81A7
#endif

#ifndef GL_SGIS_fog_function
#define GL_FOG_FUNC_SGIS                  0x812A
#define GL_FOG_FUNC_POINTS_SGIS           0x812B
#define GL_MAX_FOG_FUNC_POINTS_SGIS       0x812C
#endif

#ifndef GL_SGIX_fog_offset
#define GL_FOG_OFFSET_SGIX                0x8198
#define GL_FOG_OFFSET_VALUE_SGIX          0x8199
#endif

#ifndef GL_HP_image_transform
#define GL_IMAGE_SCALE_X_HP               0x8155
#define GL_IMAGE_SCALE_Y_HP               0x8156
#define GL_IMAGE_TRANSLATE_X_HP           0x8157
#define GL_IMAGE_TRANSLATE_Y_HP           0x8158
#define GL_IMAGE_ROTATE_ANGLE_HP          0x8159
#define GL_IMAGE_ROTATE_ORIGIN_X_HP       0x815A
#define GL_IMAGE_ROTATE_ORIGIN_Y_HP       0x815B
#define GL_IMAGE_MAG_FILTER_HP            0x815C
#define GL_IMAGE_MIN_FILTER_HP            0x815D
#define GL_IMAGE_CUBIC_WEIGHT_HP          0x815E
#define GL_CUBIC_HP                       0x815F
#define GL_AVERAGE_HP                     0x8160
#define GL_IMAGE_TRANSFORM_2D_HP          0x8161
#define GL_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP 0x8162
#define GL_PROXY_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP 0x8163
#endif

#ifndef GL_HP_convolution_border_modes
#define GL_IGNORE_BORDER_HP               0x8150
#define GL_CONSTANT_BORDER_HP             0x8151
#define GL_REPLICATE_BORDER_HP            0x8153
#define GL_CONVOLUTION_BORDER_COLOR_HP    0x8154
#endif

#ifndef GL_INGR_palette_buffer
#endif

#ifndef GL_SGIX_texture_add_env
#define GL_TEXTURE_ENV_BIAS_SGIX          0x80BE
#endif

#ifndef GL_EXT_color_subtable
#endif

#ifndef GL_PGI_vertex_hints
#define GL_VERTEX_DATA_HINT_PGI           0x1A22A
#define GL_VERTEX_CONSISTENT_HINT_PGI     0x1A22B
#define GL_MATERIAL_SIDE_HINT_PGI         0x1A22C
#define GL_MAX_VERTEX_HINT_PGI            0x1A22D
#define GL_COLOR3_BIT_PGI                 0x00010000
#define GL_COLOR4_BIT_PGI                 0x00020000
#define GL_EDGEFLAG_BIT_PGI               0x00040000
#define GL_INDEX_BIT_PGI                  0x00080000
#define GL_MAT_AMBIENT_BIT_PGI            0x00100000
#define GL_MAT_AMBIENT_AND_DIFFUSE_BIT_PGI 0x00200000
#define GL_MAT_DIFFUSE_BIT_PGI            0x00400000
#define GL_MAT_EMISSION_BIT_PGI           0x00800000
#define GL_MAT_COLOR_INDEXES_BIT_PGI      0x01000000
#define GL_MAT_SHININESS_BIT_PGI          0x02000000
#define GL_MAT_SPECULAR_BIT_PGI           0x04000000
#define GL_NORMAL_BIT_PGI                 0x08000000
#define GL_TEXCOORD1_BIT_PGI              0x10000000
#define GL_TEXCOORD2_BIT_PGI              0x20000000
#define GL_TEXCOORD3_BIT_PGI              0x40000000
#define GL_TEXCOORD4_BIT_PGI              0x80000000
#define GL_VERTEX23_BIT_PGI               0x00000004
#define GL_VERTEX4_BIT_PGI                0x00000008
#endif

#ifndef GL_PGI_misc_hints
#define GL_PREFER_DOUBLEBUFFER_HINT_PGI   0x1A1F8
#define GL_CONSERVE_MEMORY_HINT_PGI       0x1A1FD
#define GL_RECLAIM_MEMORY_HINT_PGI        0x1A1FE
#define GL_NATIVE_GRAPHICS_HANDLE_PGI     0x1A202
#define GL_NATIVE_GRAPHICS_BEGIN_HINT_PGI 0x1A203
#define GL_NATIVE_GRAPHICS_END_HINT_PGI   0x1A204
#define GL_ALWAYS_FAST_HINT_PGI           0x1A20C
#define GL_ALWAYS_SOFT_HINT_PGI           0x1A20D
#define GL_ALLOW_DRAW_OBJ_HINT_PGI        0x1A20E
#define GL_ALLOW_DRAW_WIN_HINT_PGI        0x1A20F
#define GL_ALLOW_DRAW_FRG_HINT_PGI        0x1A210
#define GL_ALLOW_DRAW_MEM_HINT_PGI        0x1A211
#define GL_STRICT_DEPTHFUNC_HINT_PGI      0x1A216
#define GL_STRICT_LIGHTING_HINT_PGI       0x1A217
#define GL_STRICT_SCISSOR_HINT_PGI        0x1A218
#define GL_FULL_STIPPLE_HINT_PGI          0x1A219
#define GL_CLIP_NEAR_HINT_PGI             0x1A220
#define GL_CLIP_FAR_HINT_PGI              0x1A221
#define GL_WIDE_LINE_HINT_PGI             0x1A222
#define GL_BACK_NORMALS_HINT_PGI          0x1A223
#endif

#ifndef GL_EXT_paletted_texture
#define GL_COLOR_INDEX1_EXT               0x80E2
#define GL_COLOR_INDEX2_EXT               0x80E3
#define GL_COLOR_INDEX4_EXT               0x80E4
#define GL_COLOR_INDEX8_EXT               0x80E5
#define GL_COLOR_INDEX12_EXT              0x80E6
#define GL_COLOR_INDEX16_EXT              0x80E7
#define GL_TEXTURE_INDEX_SIZE_EXT         0x80ED
#endif

#ifndef GL_EXT_clip_volume_hint
#define GL_CLIP_VOLUME_CLIPPING_HINT_EXT  0x80F0
#endif

#ifndef GL_SGIX_list_priority
#define GL_LIST_PRIORITY_SGIX             0x8182
#endif

#ifndef GL_SGIX_ir_instrument1
#define GL_IR_INSTRUMENT1_SGIX            0x817F
#endif

#ifndef GL_SGIX_calligraphic_fragment
#define GL_CALLIGRAPHIC_FRAGMENT_SGIX     0x8183
#endif

#ifndef GL_SGIX_texture_lod_bias
#define GL_TEXTURE_LOD_BIAS_S_SGIX        0x818E
#define GL_TEXTURE_LOD_BIAS_T_SGIX        0x818F
#define GL_TEXTURE_LOD_BIAS_R_SGIX        0x8190
#endif

#ifndef GL_SGIX_shadow_ambient
#define GL_SHADOW_AMBIENT_SGIX            0x80BF
#endif

#ifndef GL_EXT_index_texture
#endif

#ifndef GL_EXT_index_material
#define GL_INDEX_MATERIAL_EXT             0x81B8
#define GL_INDEX_MATERIAL_PARAMETER_EXT   0x81B9
#define GL_INDEX_MATERIAL_FACE_EXT        0x81BA
#endif

#ifndef GL_EXT_index_func
#define GL_INDEX_TEST_EXT                 0x81B5
#define GL_INDEX_TEST_FUNC_EXT            0x81B6
#define GL_INDEX_TEST_REF_EXT             0x81B7
#endif

#ifndef GL_EXT_index_array_formats
#define GL_IUI_V2F_EXT                    0x81AD
#define GL_IUI_V3F_EXT                    0x81AE
#define GL_IUI_N3F_V2F_EXT                0x81AF
#define GL_IUI_N3F_V3F_EXT                0x81B0
#define GL_T2F_IUI_V2F_EXT                0x81B1
#define GL_T2F_IUI_V3F_EXT                0x81B2
#define GL_T2F_IUI_N3F_V2F_EXT            0x81B3
#define GL_T2F_IUI_N3F_V3F_EXT            0x81B4
#endif

#ifndef GL_EXT_compiled_vertex_array
#define GL_ARRAY_ELEMENT_LOCK_FIRST_EXT   0x81A8
#define GL_ARRAY_ELEMENT_LOCK_COUNT_EXT   0x81A9
#endif

#ifndef GL_EXT_cull_vertex
#define GL_CULL_VERTEX_EXT                0x81AA
#define GL_CULL_VERTEX_EYE_POSITION_EXT   0x81AB
#define GL_CULL_VERTEX_OBJECT_POSITION_EXT 0x81AC
#endif

#ifndef GL_SGIX_ycrcb
#define GL_YCRCB_422_SGIX                 0x81BB
#define GL_YCRCB_444_SGIX                 0x81BC
#endif

#ifndef GL_SGIX_fragment_lighting
#define GL_FRAGMENT_LIGHTING_SGIX         0x8400
#define GL_FRAGMENT_COLOR_MATERIAL_SGIX   0x8401
#define GL_FRAGMENT_COLOR_MATERIAL_FACE_SGIX 0x8402
#define GL_FRAGMENT_COLOR_MATERIAL_PARAMETER_SGIX 0x8403
#define GL_MAX_FRAGMENT_LIGHTS_SGIX       0x8404
#define GL_MAX_ACTIVE_LIGHTS_SGIX         0x8405
#define GL_CURRENT_RASTER_NORMAL_SGIX     0x8406
#define GL_LIGHT_ENV_MODE_SGIX            0x8407
#define GL_FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER_SGIX 0x8408
#define GL_FRAGMENT_LIGHT_MODEL_TWO_SIDE_SGIX 0x8409
#define GL_FRAGMENT_LIGHT_MODEL_AMBIENT_SGIX 0x840A
#define GL_FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION_SGIX 0x840B
#define GL_FRAGMENT_LIGHT0_SGIX           0x840C
#define GL_FRAGMENT_LIGHT1_SGIX           0x840D
#define GL_FRAGMENT_LIGHT2_SGIX           0x840E
#define GL_FRAGMENT_LIGHT3_SGIX           0x840F
#define GL_FRAGMENT_LIGHT4_SGIX           0x8410
#define GL_FRAGMENT_LIGHT5_SGIX           0x8411
#define GL_FRAGMENT_LIGHT6_SGIX           0x8412
#define GL_FRAGMENT_LIGHT7_SGIX           0x8413
#endif

#ifndef GL_IBM_rasterpos_clip
#define GL_RASTER_POSITION_UNCLIPPED_IBM  0x19262
#endif

#ifndef GL_HP_texture_lighting
#define GL_TEXTURE_LIGHTING_MODE_HP       0x8167
#define GL_TEXTURE_POST_SPECULAR_HP       0x8168
#define GL_TEXTURE_PRE_SPECULAR_HP        0x8169
#endif

#ifndef GL_EXT_draw_range_elements
#define GL_MAX_ELEMENTS_VERTICES_EXT      0x80E8
#define GL_MAX_ELEMENTS_INDICES_EXT       0x80E9
#endif

#ifndef GL_WIN_phong_shading
#define GL_PHONG_WIN                      0x80EA
#define GL_PHONG_HINT_WIN                 0x80EB
#endif

#ifndef GL_WIN_specular_fog
#define GL_FOG_SPECULAR_TEXTURE_WIN       0x80EC
#endif

#ifndef GL_EXT_light_texture
#define GL_FRAGMENT_MATERIAL_EXT          0x8349
#define GL_FRAGMENT_NORMAL_EXT            0x834A
#define GL_FRAGMENT_COLOR_EXT             0x834C
#define GL_ATTENUATION_EXT                0x834D
#define GL_SHADOW_ATTENUATION_EXT         0x834E
#define GL_TEXTURE_APPLICATION_MODE_EXT   0x834F
#define GL_TEXTURE_LIGHT_EXT              0x8350
#define GL_TEXTURE_MATERIAL_FACE_EXT      0x8351
#define GL_TEXTURE_MATERIAL_PARAMETER_EXT 0x8352
/* reuse GL_FRAGMENT_DEPTH_EXT */
#endif

#ifndef GL_SGIX_blend_alpha_minmax
#define GL_ALPHA_MIN_SGIX                 0x8320
#define GL_ALPHA_MAX_SGIX                 0x8321
#endif

#ifndef GL_SGIX_impact_pixel_texture
#define GL_PIXEL_TEX_GEN_Q_CEILING_SGIX   0x8184
#define GL_PIXEL_TEX_GEN_Q_ROUND_SGIX     0x8185
#define GL_PIXEL_TEX_GEN_Q_FLOOR_SGIX     0x8186
#define GL_PIXEL_TEX_GEN_ALPHA_REPLACE_SGIX 0x8187
#define GL_PIXEL_TEX_GEN_ALPHA_NO_REPLACE_SGIX 0x8188
#define GL_PIXEL_TEX_GEN_ALPHA_LS_SGIX    0x8189
#define GL_PIXEL_TEX_GEN_ALPHA_MS_SGIX    0x818A
#endif

#ifndef GL_EXT_bgra
#define GL_BGR_EXT                        0x80E0
#define GL_BGRA_EXT                       0x80E1
#endif

#ifndef GL_SGIX_async
#define GL_ASYNC_MARKER_SGIX              0x8329
#endif

#ifndef GL_SGIX_async_pixel
#define GL_ASYNC_TEX_IMAGE_SGIX           0x835C
#define GL_ASYNC_DRAW_PIXELS_SGIX         0x835D
#define GL_ASYNC_READ_PIXELS_SGIX         0x835E
#define GL_MAX_ASYNC_TEX_IMAGE_SGIX       0x835F
#define GL_MAX_ASYNC_DRAW_PIXELS_SGIX     0x8360
#define GL_MAX_ASYNC_READ_PIXELS_SGIX     0x8361
#endif

#ifndef GL_SGIX_async_histogram
#define GL_ASYNC_HISTOGRAM_SGIX           0x832C
#define GL_MAX_ASYNC_HISTOGRAM_SGIX       0x832D
#endif

#ifndef GL_INTEL_texture_scissor
#endif

#ifndef GL_INTEL_parallel_arrays
#define GL_PARALLEL_ARRAYS_INTEL          0x83F4
#define GL_VERTEX_ARRAY_PARALLEL_POINTERS_INTEL 0x83F5
#define GL_NORMAL_ARRAY_PARALLEL_POINTERS_INTEL 0x83F6
#define GL_COLOR_ARRAY_PARALLEL_POINTERS_INTEL 0x83F7
#define GL_TEXTURE_COORD_ARRAY_PARALLEL_POINTERS_INTEL 0x83F8
#endif

#ifndef GL_HP_occlusion_test
#define GL_OCCLUSION_TEST_HP              0x8165
#define GL_OCCLUSION_TEST_RESULT_HP       0x8166
#endif

#ifndef GL_EXT_pixel_transform
#define GL_PIXEL_TRANSFORM_2D_EXT         0x8330
#define GL_PIXEL_MAG_FILTER_EXT           0x8331
#define GL_PIXEL_MIN_FILTER_EXT           0x8332
#define GL_PIXEL_CUBIC_WEIGHT_EXT         0x8333
#define GL_CUBIC_EXT                      0x8334
#define GL_AVERAGE_EXT                    0x8335
#define GL_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT 0x8336
#define GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT 0x8337
#define GL_PIXEL_TRANSFORM_2D_MATRIX_EXT  0x8338
#endif

#ifndef GL_EXT_pixel_transform_color_table
#endif

#ifndef GL_EXT_shared_texture_palette
#define GL_SHARED_TEXTURE_PALETTE_EXT     0x81FB
#endif

#ifndef GL_EXT_separate_specular_color
#define GL_LIGHT_MODEL_COLOR_CONTROL_EXT  0x81F8
#define GL_SINGLE_COLOR_EXT               0x81F9
#define GL_SEPARATE_SPECULAR_COLOR_EXT    0x81FA
#endif

#ifndef GL_EXT_secondary_color
#define GL_COLOR_SUM_EXT                  0x8458
#define GL_CURRENT_SECONDARY_COLOR_EXT    0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE_EXT 0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE_EXT 0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT 0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER_EXT 0x845D
#define GL_SECONDARY_COLOR_ARRAY_EXT      0x845E
#endif

#ifndef GL_EXT_texture_perturb_normal
#define GL_PERTURB_EXT                    0x85AE
#define GL_TEXTURE_NORMAL_EXT             0x85AF
#endif

#ifndef GL_EXT_multi_draw_arrays
#endif

#ifndef GL_EXT_fog_coord
#define GL_FOG_COORDINATE_SOURCE_EXT      0x8450
#define GL_FOG_COORDINATE_EXT             0x8451
#define GL_FRAGMENT_DEPTH_EXT             0x8452
#define GL_CURRENT_FOG_COORDINATE_EXT     0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE_EXT  0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE_EXT 0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER_EXT 0x8456
#define GL_FOG_COORDINATE_ARRAY_EXT       0x8457
#endif

#ifndef GL_REND_screen_coordinates
#define GL_SCREEN_COORDINATES_REND        0x8490
#define GL_INVERTED_SCREEN_W_REND         0x8491
#endif

#ifndef GL_EXT_coordinate_frame
#define GL_TANGENT_ARRAY_EXT              0x8439
#define GL_BINORMAL_ARRAY_EXT             0x843A
#define GL_CURRENT_TANGENT_EXT            0x843B
#define GL_CURRENT_BINORMAL_EXT           0x843C
#define GL_TANGENT_ARRAY_TYPE_EXT         0x843E
#define GL_TANGENT_ARRAY_STRIDE_EXT       0x843F
#define GL_BINORMAL_ARRAY_TYPE_EXT        0x8440
#define GL_BINORMAL_ARRAY_STRIDE_EXT      0x8441
#define GL_TANGENT_ARRAY_POINTER_EXT      0x8442
#define GL_BINORMAL_ARRAY_POINTER_EXT     0x8443
#define GL_MAP1_TANGENT_EXT               0x8444
#define GL_MAP2_TANGENT_EXT               0x8445
#define GL_MAP1_BINORMAL_EXT              0x8446
#define GL_MAP2_BINORMAL_EXT              0x8447
#endif

#ifndef GL_EXT_texture_env_combine
#define GL_COMBINE_EXT                    0x8570
#define GL_COMBINE_RGB_EXT                0x8571
#define GL_COMBINE_ALPHA_EXT              0x8572
#define GL_RGB_SCALE_EXT                  0x8573
#define GL_ADD_SIGNED_EXT                 0x8574
#define GL_INTERPOLATE_EXT                0x8575
#define GL_CONSTANT_EXT                   0x8576
#define GL_PRIMARY_COLOR_EXT              0x8577
#define GL_PREVIOUS_EXT                   0x8578
#define GL_SOURCE0_RGB_EXT                0x8580
#define GL_SOURCE1_RGB_EXT                0x8581
#define GL_SOURCE2_RGB_EXT                0x8582
#define GL_SOURCE0_ALPHA_EXT              0x8588
#define GL_SOURCE1_ALPHA_EXT              0x8589
#define GL_SOURCE2_ALPHA_EXT              0x858A
#define GL_OPERAND0_RGB_EXT               0x8590
#define GL_OPERAND1_RGB_EXT               0x8591
#define GL_OPERAND2_RGB_EXT               0x8592
#define GL_OPERAND0_ALPHA_EXT             0x8598
#define GL_OPERAND1_ALPHA_EXT             0x8599
#define GL_OPERAND2_ALPHA_EXT             0x859A
#endif

#ifndef GL_APPLE_specular_vector
#define GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE 0x85B0
#endif

#ifndef GL_APPLE_transform_hint
#define GL_TRANSFORM_HINT_APPLE           0x85B1
#endif

#ifndef GL_SGIX_fog_scale
#define GL_FOG_SCALE_SGIX                 0x81FC
#define GL_FOG_SCALE_VALUE_SGIX           0x81FD
#endif

#ifndef GL_SUNX_constant_data
#define GL_UNPACK_CONSTANT_DATA_SUNX      0x81D5
#define GL_TEXTURE_CONSTANT_DATA_SUNX     0x81D6
#endif

#ifndef GL_SUN_global_alpha
#define GL_GLOBAL_ALPHA_SUN               0x81D9
#define GL_GLOBAL_ALPHA_FACTOR_SUN        0x81DA
#endif

#ifndef GL_SUN_triangle_list
#define GL_RESTART_SUN                    0x0001
#define GL_REPLACE_MIDDLE_SUN             0x0002
#define GL_REPLACE_OLDEST_SUN             0x0003
#define GL_TRIANGLE_LIST_SUN              0x81D7
#define GL_REPLACEMENT_CODE_SUN           0x81D8
#define GL_REPLACEMENT_CODE_ARRAY_SUN     0x85C0
#define GL_REPLACEMENT_CODE_ARRAY_TYPE_SUN 0x85C1
#define GL_REPLACEMENT_CODE_ARRAY_STRIDE_SUN 0x85C2
#define GL_REPLACEMENT_CODE_ARRAY_POINTER_SUN 0x85C3
#define GL_R1UI_V3F_SUN                   0x85C4
#define GL_R1UI_C4UB_V3F_SUN              0x85C5
#define GL_R1UI_C3F_V3F_SUN               0x85C6
#define GL_R1UI_N3F_V3F_SUN               0x85C7
#define GL_R1UI_C4F_N3F_V3F_SUN           0x85C8
#define GL_R1UI_T2F_V3F_SUN               0x85C9
#define GL_R1UI_T2F_N3F_V3F_SUN           0x85CA
#define GL_R1UI_T2F_C4F_N3F_V3F_SUN       0x85CB
#endif

#ifndef GL_SUN_vertex
#endif

#ifndef GL_EXT_blend_func_separate
#define GL_BLEND_DST_RGB_EXT              0x80C8
#define GL_BLEND_SRC_RGB_EXT              0x80C9
#define GL_BLEND_DST_ALPHA_EXT            0x80CA
#define GL_BLEND_SRC_ALPHA_EXT            0x80CB
#endif

#ifndef GL_INGR_color_clamp
#define GL_RED_MIN_CLAMP_INGR             0x8560
#define GL_GREEN_MIN_CLAMP_INGR           0x8561
#define GL_BLUE_MIN_CLAMP_INGR            0x8562
#define GL_ALPHA_MIN_CLAMP_INGR           0x8563
#define GL_RED_MAX_CLAMP_INGR             0x8564
#define GL_GREEN_MAX_CLAMP_INGR           0x8565
#define GL_BLUE_MAX_CLAMP_INGR            0x8566
#define GL_ALPHA_MAX_CLAMP_INGR           0x8567
#endif

#ifndef GL_INGR_interlace_read
#define GL_INTERLACE_READ_INGR            0x8568
#endif

#ifndef GL_EXT_stencil_wrap
#define GL_INCR_WRAP_EXT                  0x8507
#define GL_DECR_WRAP_EXT                  0x8508
#endif

#ifndef GL_EXT_422_pixels
#define GL_422_EXT                        0x80CC
#define GL_422_REV_EXT                    0x80CD
#define GL_422_AVERAGE_EXT                0x80CE
#define GL_422_REV_AVERAGE_EXT            0x80CF
#endif

#ifndef GL_NV_texgen_reflection
#define GL_NORMAL_MAP_NV                  0x8511
#define GL_REFLECTION_MAP_NV              0x8512
#endif

#ifndef GL_EXT_texture_cube_map
#define GL_NORMAL_MAP_EXT                 0x8511
#define GL_REFLECTION_MAP_EXT             0x8512
#define GL_TEXTURE_CUBE_MAP_EXT           0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_EXT   0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_EXT     0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT  0x851C
#endif

#ifndef GL_SUN_convolution_border_modes
#define GL_WRAP_BORDER_SUN                0x81D4
#endif

#ifndef GL_EXT_texture_env_add
#endif

#ifndef GL_EXT_texture_lod_bias
#define GL_MAX_TEXTURE_LOD_BIAS_EXT       0x84FD
#define GL_TEXTURE_FILTER_CONTROL_EXT     0x8500
#define GL_TEXTURE_LOD_BIAS_EXT           0x8501
#endif

#ifndef GL_EXT_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

#ifndef GL_EXT_vertex_weighting
#define GL_MODELVIEW0_STACK_DEPTH_EXT     GL_MODELVIEW_STACK_DEPTH
#define GL_MODELVIEW1_STACK_DEPTH_EXT     0x8502
#define GL_MODELVIEW0_MATRIX_EXT          GL_MODELVIEW_MATRIX
#define GL_MODELVIEW1_MATRIX_EXT          0x8506
#define GL_VERTEX_WEIGHTING_EXT           0x8509
#define GL_MODELVIEW0_EXT                 GL_MODELVIEW
#define GL_MODELVIEW1_EXT                 0x850A
#define GL_CURRENT_VERTEX_WEIGHT_EXT      0x850B
#define GL_VERTEX_WEIGHT_ARRAY_EXT        0x850C
#define GL_VERTEX_WEIGHT_ARRAY_SIZE_EXT   0x850D
#define GL_VERTEX_WEIGHT_ARRAY_TYPE_EXT   0x850E
#define GL_VERTEX_WEIGHT_ARRAY_STRIDE_EXT 0x850F
#define GL_VERTEX_WEIGHT_ARRAY_POINTER_EXT 0x8510
#endif

#ifndef GL_NV_light_max_exponent
#define GL_MAX_SHININESS_NV               0x8504
#define GL_MAX_SPOT_EXPONENT_NV           0x8505
#endif

#ifndef GL_NV_vertex_array_range
#define GL_VERTEX_ARRAY_RANGE_NV          0x851D
#define GL_VERTEX_ARRAY_RANGE_LENGTH_NV   0x851E
#define GL_VERTEX_ARRAY_RANGE_VALID_NV    0x851F
#define GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV 0x8520
#define GL_VERTEX_ARRAY_RANGE_POINTER_NV  0x8521
#endif

#ifndef GL_NV_register_combiners
#define GL_REGISTER_COMBINERS_NV          0x8522
#define GL_VARIABLE_A_NV                  0x8523
#define GL_VARIABLE_B_NV                  0x8524
#define GL_VARIABLE_C_NV                  0x8525
#define GL_VARIABLE_D_NV                  0x8526
#define GL_VARIABLE_E_NV                  0x8527
#define GL_VARIABLE_F_NV                  0x8528
#define GL_VARIABLE_G_NV                  0x8529
#define GL_CONSTANT_COLOR0_NV             0x852A
#define GL_CONSTANT_COLOR1_NV             0x852B
#define GL_PRIMARY_COLOR_NV               0x852C
#define GL_SECONDARY_COLOR_NV             0x852D
#define GL_SPARE0_NV                      0x852E
#define GL_SPARE1_NV                      0x852F
#define GL_DISCARD_NV                     0x8530
#define GL_E_TIMES_F_NV                   0x8531
#define GL_SPARE0_PLUS_SECONDARY_COLOR_NV 0x8532
#define GL_UNSIGNED_IDENTITY_NV           0x8536
#define GL_UNSIGNED_INVERT_NV             0x8537
#define GL_EXPAND_NORMAL_NV               0x8538
#define GL_EXPAND_NEGATE_NV               0x8539
#define GL_HALF_BIAS_NORMAL_NV            0x853A
#define GL_HALF_BIAS_NEGATE_NV            0x853B
#define GL_SIGNED_IDENTITY_NV             0x853C
#define GL_SIGNED_NEGATE_NV               0x853D
#define GL_SCALE_BY_TWO_NV                0x853E
#define GL_SCALE_BY_FOUR_NV               0x853F
#define GL_SCALE_BY_ONE_HALF_NV           0x8540
#define GL_BIAS_BY_NEGATIVE_ONE_HALF_NV   0x8541
#define GL_COMBINER_INPUT_NV              0x8542
#define GL_COMBINER_MAPPING_NV            0x8543
#define GL_COMBINER_COMPONENT_USAGE_NV    0x8544
#define GL_COMBINER_AB_DOT_PRODUCT_NV     0x8545
#define GL_COMBINER_CD_DOT_PRODUCT_NV     0x8546
#define GL_COMBINER_MUX_SUM_NV            0x8547
#define GL_COMBINER_SCALE_NV              0x8548
#define GL_COMBINER_BIAS_NV               0x8549
#define GL_COMBINER_AB_OUTPUT_NV          0x854A
#define GL_COMBINER_CD_OUTPUT_NV          0x854B
#define GL_COMBINER_SUM_OUTPUT_NV         0x854C
#define GL_MAX_GENERAL_COMBINERS_NV       0x854D
#define GL_NUM_GENERAL_COMBINERS_NV       0x854E
#define GL_COLOR_SUM_CLAMP_NV             0x854F
#define GL_COMBINER0_NV                   0x8550
#define GL_COMBINER1_NV                   0x8551
#define GL_COMBINER2_NV                   0x8552
#define GL_COMBINER3_NV                   0x8553
#define GL_COMBINER4_NV                   0x8554
#define GL_COMBINER5_NV                   0x8555
#define GL_COMBINER6_NV                   0x8556
#define GL_COMBINER7_NV                   0x8557
/* reuse GL_TEXTURE0_ARB */
/* reuse GL_TEXTURE1_ARB */
/* reuse GL_ZERO */
/* reuse GL_NONE */
/* reuse GL_FOG */
#endif

#ifndef GL_NV_fog_distance
#define GL_FOG_DISTANCE_MODE_NV           0x855A
#define GL_EYE_RADIAL_NV                  0x855B
#define GL_EYE_PLANE_ABSOLUTE_NV          0x855C
/* reuse GL_EYE_PLANE */
#endif

#ifndef GL_NV_texgen_emboss
#define GL_EMBOSS_LIGHT_NV                0x855D
#define GL_EMBOSS_CONSTANT_NV             0x855E
#define GL_EMBOSS_MAP_NV                  0x855F
#endif

#ifndef GL_NV_blend_square
#endif

#ifndef GL_NV_texture_env_combine4
#define GL_COMBINE4_NV                    0x8503
#define GL_SOURCE3_RGB_NV                 0x8583
#define GL_SOURCE3_ALPHA_NV               0x858B
#define GL_OPERAND3_RGB_NV                0x8593
#define GL_OPERAND3_ALPHA_NV              0x859B
#endif

#ifndef GL_MESA_resize_buffers
#endif

#ifndef GL_MESA_window_pos
#endif

#ifndef GL_EXT_texture_compression_s3tc
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
#endif

#ifndef GL_IBM_cull_vertex
#define GL_CULL_VERTEX_IBM                103050
#endif

#ifndef GL_IBM_multimode_draw_arrays
#endif

#ifndef GL_IBM_vertex_array_lists
#define GL_VERTEX_ARRAY_LIST_IBM          103070
#define GL_NORMAL_ARRAY_LIST_IBM          103071
#define GL_COLOR_ARRAY_LIST_IBM           103072
#define GL_INDEX_ARRAY_LIST_IBM           103073
#define GL_TEXTURE_COORD_ARRAY_LIST_IBM   103074
#define GL_EDGE_FLAG_ARRAY_LIST_IBM       103075
#define GL_FOG_COORDINATE_ARRAY_LIST_IBM  103076
#define GL_SECONDARY_COLOR_ARRAY_LIST_IBM 103077
#define GL_VERTEX_ARRAY_LIST_STRIDE_IBM   103080
#define GL_NORMAL_ARRAY_LIST_STRIDE_IBM   103081
#define GL_COLOR_ARRAY_LIST_STRIDE_IBM    103082
#define GL_INDEX_ARRAY_LIST_STRIDE_IBM    103083
#define GL_TEXTURE_COORD_ARRAY_LIST_STRIDE_IBM 103084
#define GL_EDGE_FLAG_ARRAY_LIST_STRIDE_IBM 103085
#define GL_FOG_COORDINATE_ARRAY_LIST_STRIDE_IBM 103086
#define GL_SECONDARY_COLOR_ARRAY_LIST_STRIDE_IBM 103087
#endif

#ifndef GL_SGIX_subsample
#define GL_PACK_SUBSAMPLE_RATE_SGIX       0x85A0
#define GL_UNPACK_SUBSAMPLE_RATE_SGIX     0x85A1
#define GL_PIXEL_SUBSAMPLE_4444_SGIX      0x85A2
#define GL_PIXEL_SUBSAMPLE_2424_SGIX      0x85A3
#define GL_PIXEL_SUBSAMPLE_4242_SGIX      0x85A4
#endif

#ifndef GL_SGIX_ycrcb_subsample
#endif

#ifndef GL_SGIX_ycrcba
#define GL_YCRCB_SGIX                     0x8318
#define GL_YCRCBA_SGIX                    0x8319
#endif

#ifndef GL_SGI_depth_pass_instrument
#define GL_DEPTH_PASS_INSTRUMENT_SGIX     0x8310
#define GL_DEPTH_PASS_INSTRUMENT_COUNTERS_SGIX 0x8311
#define GL_DEPTH_PASS_INSTRUMENT_MAX_SGIX 0x8312
#endif

#ifndef GL_3DFX_texture_compression_FXT1
#define GL_COMPRESSED_RGB_FXT1_3DFX       0x86B0
#define GL_COMPRESSED_RGBA_FXT1_3DFX      0x86B1
#endif

#ifndef GL_3DFX_multisample
#define GL_MULTISAMPLE_3DFX               0x86B2
#define GL_SAMPLE_BUFFERS_3DFX            0x86B3
#define GL_SAMPLES_3DFX                   0x86B4
#define GL_MULTISAMPLE_BIT_3DFX           0x20000000
#endif

#ifndef GL_3DFX_tbuffer
#endif

#ifndef GL_EXT_multisample
#define GL_MULTISAMPLE_EXT                0x809D
#define GL_SAMPLE_ALPHA_TO_MASK_EXT       0x809E
#define GL_SAMPLE_ALPHA_TO_ONE_EXT        0x809F
#define GL_SAMPLE_MASK_EXT                0x80A0
#define GL_1PASS_EXT                      0x80A1
#define GL_2PASS_0_EXT                    0x80A2
#define GL_2PASS_1_EXT                    0x80A3
#define GL_4PASS_0_EXT                    0x80A4
#define GL_4PASS_1_EXT                    0x80A5
#define GL_4PASS_2_EXT                    0x80A6
#define GL_4PASS_3_EXT                    0x80A7
#define GL_SAMPLE_BUFFERS_EXT             0x80A8
#define GL_SAMPLES_EXT                    0x80A9
#define GL_SAMPLE_MASK_VALUE_EXT          0x80AA
#define GL_SAMPLE_MASK_INVERT_EXT         0x80AB
#define GL_SAMPLE_PATTERN_EXT             0x80AC
#define GL_MULTISAMPLE_BIT_EXT            0x20000000
#endif

#ifndef GL_SGIX_vertex_preclip
#define GL_VERTEX_PRECLIP_SGIX            0x83EE
#define GL_VERTEX_PRECLIP_HINT_SGIX       0x83EF
#endif

#ifndef GL_SGIX_convolution_accuracy
#define GL_CONVOLUTION_HINT_SGIX          0x8316
#endif

#ifndef GL_SGIX_resample
#define GL_PACK_RESAMPLE_SGIX             0x842C
#define GL_UNPACK_RESAMPLE_SGIX           0x842D
#define GL_RESAMPLE_REPLICATE_SGIX        0x842E
#define GL_RESAMPLE_ZERO_FILL_SGIX        0x842F
#define GL_RESAMPLE_DECIMATE_SGIX         0x8430
#endif

#ifndef GL_SGIS_point_line_texgen
#define GL_EYE_DISTANCE_TO_POINT_SGIS     0x81F0
#define GL_OBJECT_DISTANCE_TO_POINT_SGIS  0x81F1
#define GL_EYE_DISTANCE_TO_LINE_SGIS      0x81F2
#define GL_OBJECT_DISTANCE_TO_LINE_SGIS   0x81F3
#define GL_EYE_POINT_SGIS                 0x81F4
#define GL_OBJECT_POINT_SGIS              0x81F5
#define GL_EYE_LINE_SGIS                  0x81F6
#define GL_OBJECT_LINE_SGIS               0x81F7
#endif

#ifndef GL_SGIS_texture_color_mask
#define GL_TEXTURE_COLOR_WRITEMASK_SGIS   0x81EF
#endif

#ifndef GL_EXT_texture_env_dot3
#define GL_DOT3_RGB_EXT                   0x8740
#define GL_DOT3_RGBA_EXT                  0x8741
#endif

#ifndef GL_ATI_texture_mirror_once
#define GL_MIRROR_CLAMP_ATI               0x8742
#define GL_MIRROR_CLAMP_TO_EDGE_ATI       0x8743
#endif

#ifndef GL_NV_fence
#define GL_ALL_COMPLETED_NV               0x84F2
#define GL_FENCE_STATUS_NV                0x84F3
#define GL_FENCE_CONDITION_NV             0x84F4
#endif

#ifndef GL_IBM_texture_mirrored_repeat
#define GL_MIRRORED_REPEAT_IBM            0x8370
#endif

#ifndef GL_NV_evaluators
#define GL_EVAL_2D_NV                     0x86C0
#define GL_EVAL_TRIANGULAR_2D_NV          0x86C1
#define GL_MAP_TESSELLATION_NV            0x86C2
#define GL_MAP_ATTRIB_U_ORDER_NV          0x86C3
#define GL_MAP_ATTRIB_V_ORDER_NV          0x86C4
#define GL_EVAL_FRACTIONAL_TESSELLATION_NV 0x86C5
#define GL_EVAL_VERTEX_ATTRIB0_NV         0x86C6
#define GL_EVAL_VERTEX_ATTRIB1_NV         0x86C7
#define GL_EVAL_VERTEX_ATTRIB2_NV         0x86C8
#define GL_EVAL_VERTEX_ATTRIB3_NV         0x86C9
#define GL_EVAL_VERTEX_ATTRIB4_NV         0x86CA
#define GL_EVAL_VERTEX_ATTRIB5_NV         0x86CB
#define GL_EVAL_VERTEX_ATTRIB6_NV         0x86CC
#define GL_EVAL_VERTEX_ATTRIB7_NV         0x86CD
#define GL_EVAL_VERTEX_ATTRIB8_NV         0x86CE
#define GL_EVAL_VERTEX_ATTRIB9_NV         0x86CF
#define GL_EVAL_VERTEX_ATTRIB10_NV        0x86D0
#define GL_EVAL_VERTEX_ATTRIB11_NV        0x86D1
#define GL_EVAL_VERTEX_ATTRIB12_NV        0x86D2
#define GL_EVAL_VERTEX_ATTRIB13_NV        0x86D3
#define GL_EVAL_VERTEX_ATTRIB14_NV        0x86D4
#define GL_EVAL_VERTEX_ATTRIB15_NV        0x86D5
#define GL_MAX_MAP_TESSELLATION_NV        0x86D6
#define GL_MAX_RATIONAL_EVAL_ORDER_NV     0x86D7
#endif

#ifndef GL_NV_packed_depth_stencil
#define GL_DEPTH_STENCIL_NV               0x84F9
#define GL_UNSIGNED_INT_24_8_NV           0x84FA
#endif

#ifndef GL_NV_register_combiners2
#define GL_PER_STAGE_CONSTANTS_NV         0x8535
#endif

#ifndef GL_NV_texture_compression_vtc
#endif

#ifndef GL_NV_texture_rectangle
#define GL_TEXTURE_RECTANGLE_NV           0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_NV   0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_NV     0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_NV  0x84F8
#endif

#ifndef GL_NV_texture_shader
#define GL_OFFSET_TEXTURE_RECTANGLE_NV    0x864C
#define GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV 0x864D
#define GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV 0x864E
#define GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV 0x86D9
#define GL_UNSIGNED_INT_S8_S8_8_8_NV      0x86DA
#define GL_UNSIGNED_INT_8_8_S8_S8_REV_NV  0x86DB
#define GL_DSDT_MAG_INTENSITY_NV          0x86DC
#define GL_SHADER_CONSISTENT_NV           0x86DD
#define GL_TEXTURE_SHADER_NV              0x86DE
#define GL_SHADER_OPERATION_NV            0x86DF
#define GL_CULL_MODES_NV                  0x86E0
#define GL_OFFSET_TEXTURE_MATRIX_NV       0x86E1
#define GL_OFFSET_TEXTURE_SCALE_NV        0x86E2
#define GL_OFFSET_TEXTURE_BIAS_NV         0x86E3
#define GL_OFFSET_TEXTURE_2D_MATRIX_NV    GL_OFFSET_TEXTURE_MATRIX_NV
#define GL_OFFSET_TEXTURE_2D_SCALE_NV     GL_OFFSET_TEXTURE_SCALE_NV
#define GL_OFFSET_TEXTURE_2D_BIAS_NV      GL_OFFSET_TEXTURE_BIAS_NV
#define GL_PREVIOUS_TEXTURE_INPUT_NV      0x86E4
#define GL_CONST_EYE_NV                   0x86E5
#define GL_PASS_THROUGH_NV                0x86E6
#define GL_CULL_FRAGMENT_NV               0x86E7
#define GL_OFFSET_TEXTURE_2D_NV           0x86E8
#define GL_DEPENDENT_AR_TEXTURE_2D_NV     0x86E9
#define GL_DEPENDENT_GB_TEXTURE_2D_NV     0x86EA
#define GL_DOT_PRODUCT_NV                 0x86EC
#define GL_DOT_PRODUCT_DEPTH_REPLACE_NV   0x86ED
#define GL_DOT_PRODUCT_TEXTURE_2D_NV      0x86EE
#define GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV 0x86F0
#define GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV 0x86F1
#define GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV 0x86F2
#define GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV 0x86F3
#define GL_HILO_NV                        0x86F4
#define GL_DSDT_NV                        0x86F5
#define GL_DSDT_MAG_NV                    0x86F6
#define GL_DSDT_MAG_VIB_NV                0x86F7
#define GL_HILO16_NV                      0x86F8
#define GL_SIGNED_HILO_NV                 0x86F9
#define GL_SIGNED_HILO16_NV               0x86FA
#define GL_SIGNED_RGBA_NV                 0x86FB
#define GL_SIGNED_RGBA8_NV                0x86FC
#define GL_SIGNED_RGB_NV                  0x86FE
#define GL_SIGNED_RGB8_NV                 0x86FF
#define GL_SIGNED_LUMINANCE_NV            0x8701
#define GL_SIGNED_LUMINANCE8_NV           0x8702
#define GL_SIGNED_LUMINANCE_ALPHA_NV      0x8703
#define GL_SIGNED_LUMINANCE8_ALPHA8_NV    0x8704
#define GL_SIGNED_ALPHA_NV                0x8705
#define GL_SIGNED_ALPHA8_NV               0x8706
#define GL_SIGNED_INTENSITY_NV            0x8707
#define GL_SIGNED_INTENSITY8_NV           0x8708
#define GL_DSDT8_NV                       0x8709
#define GL_DSDT8_MAG8_NV                  0x870A
#define GL_DSDT8_MAG8_INTENSITY8_NV       0x870B
#define GL_SIGNED_RGB_UNSIGNED_ALPHA_NV   0x870C
#define GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV 0x870D
#define GL_HI_SCALE_NV                    0x870E
#define GL_LO_SCALE_NV                    0x870F
#define GL_DS_SCALE_NV                    0x8710
#define GL_DT_SCALE_NV                    0x8711
#define GL_MAGNITUDE_SCALE_NV             0x8712
#define GL_VIBRANCE_SCALE_NV              0x8713
#define GL_HI_BIAS_NV                     0x8714
#define GL_LO_BIAS_NV                     0x8715
#define GL_DS_BIAS_NV                     0x8716
#define GL_DT_BIAS_NV                     0x8717
#define GL_MAGNITUDE_BIAS_NV              0x8718
#define GL_VIBRANCE_BIAS_NV               0x8719
#define GL_TEXTURE_BORDER_VALUES_NV       0x871A
#define GL_TEXTURE_HI_SIZE_NV             0x871B
#define GL_TEXTURE_LO_SIZE_NV             0x871C
#define GL_TEXTURE_DS_SIZE_NV             0x871D
#define GL_TEXTURE_DT_SIZE_NV             0x871E
#define GL_TEXTURE_MAG_SIZE_NV            0x871F
#endif

#ifndef GL_NV_texture_shader2
#define GL_DOT_PRODUCT_TEXTURE_3D_NV      0x86EF
#endif

#ifndef GL_NV_vertex_array_range2
#define GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV 0x8533
#endif

#ifndef GL_NV_vertex_program
#define GL_VERTEX_PROGRAM_NV              0x8620
#define GL_VERTEX_STATE_PROGRAM_NV        0x8621
#define GL_ATTRIB_ARRAY_SIZE_NV           0x8623
#define GL_ATTRIB_ARRAY_STRIDE_NV         0x8624
#define GL_ATTRIB_ARRAY_TYPE_NV           0x8625
#define GL_CURRENT_ATTRIB_NV              0x8626
#define GL_PROGRAM_LENGTH_NV              0x8627
#define GL_PROGRAM_STRING_NV              0x8628
#define GL_MODELVIEW_PROJECTION_NV        0x8629
#define GL_IDENTITY_NV                    0x862A
#define GL_INVERSE_NV                     0x862B
#define GL_TRANSPOSE_NV                   0x862C
#define GL_INVERSE_TRANSPOSE_NV           0x862D
#define GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV 0x862E
#define GL_MAX_TRACK_MATRICES_NV          0x862F
#define GL_MATRIX0_NV                     0x8630
#define GL_MATRIX1_NV                     0x8631
#define GL_MATRIX2_NV                     0x8632
#define GL_MATRIX3_NV                     0x8633
#define GL_MATRIX4_NV                     0x8634
#define GL_MATRIX5_NV                     0x8635
#define GL_MATRIX6_NV                     0x8636
#define GL_MATRIX7_NV                     0x8637
#define GL_CURRENT_MATRIX_STACK_DEPTH_NV  0x8640
#define GL_CURRENT_MATRIX_NV              0x8641
#define GL_VERTEX_PROGRAM_POINT_SIZE_NV   0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_NV     0x8643
#define GL_PROGRAM_PARAMETER_NV           0x8644
#define GL_ATTRIB_ARRAY_POINTER_NV        0x8645
#define GL_PROGRAM_TARGET_NV              0x8646
#define GL_PROGRAM_RESIDENT_NV            0x8647
#define GL_TRACK_MATRIX_NV                0x8648
#define GL_TRACK_MATRIX_TRANSFORM_NV      0x8649
#define GL_VERTEX_PROGRAM_BINDING_NV      0x864A
#define GL_PROGRAM_ERROR_POSITION_NV      0x864B
#define GL_VERTEX_ATTRIB_ARRAY0_NV        0x8650
#define GL_VERTEX_ATTRIB_ARRAY1_NV        0x8651
#define GL_VERTEX_ATTRIB_ARRAY2_NV        0x8652
#define GL_VERTEX_ATTRIB_ARRAY3_NV        0x8653
#define GL_VERTEX_ATTRIB_ARRAY4_NV        0x8654
#define GL_VERTEX_ATTRIB_ARRAY5_NV        0x8655
#define GL_VERTEX_ATTRIB_ARRAY6_NV        0x8656
#define GL_VERTEX_ATTRIB_ARRAY7_NV        0x8657
#define GL_VERTEX_ATTRIB_ARRAY8_NV        0x8658
#define GL_VERTEX_ATTRIB_ARRAY9_NV        0x8659
#define GL_VERTEX_ATTRIB_ARRAY10_NV       0x865A
#define GL_VERTEX_ATTRIB_ARRAY11_NV       0x865B
#define GL_VERTEX_ATTRIB_ARRAY12_NV       0x865C
#define GL_VERTEX_ATTRIB_ARRAY13_NV       0x865D
#define GL_VERTEX_ATTRIB_ARRAY14_NV       0x865E
#define GL_VERTEX_ATTRIB_ARRAY15_NV       0x865F
#define GL_MAP1_VERTEX_ATTRIB0_4_NV       0x8660
#define GL_MAP1_VERTEX_ATTRIB1_4_NV       0x8661
#define GL_MAP1_VERTEX_ATTRIB2_4_NV       0x8662
#define GL_MAP1_VERTEX_ATTRIB3_4_NV       0x8663
#define GL_MAP1_VERTEX_ATTRIB4_4_NV       0x8664
#define GL_MAP1_VERTEX_ATTRIB5_4_NV       0x8665
#define GL_MAP1_VERTEX_ATTRIB6_4_NV       0x8666
#define GL_MAP1_VERTEX_ATTRIB7_4_NV       0x8667
#define GL_MAP1_VERTEX_ATTRIB8_4_NV       0x8668
#define GL_MAP1_VERTEX_ATTRIB9_4_NV       0x8669
#define GL_MAP1_VERTEX_ATTRIB10_4_NV      0x866A
#define GL_MAP1_VERTEX_ATTRIB11_4_NV      0x866B
#define GL_MAP1_VERTEX_ATTRIB12_4_NV      0x866C
#define GL_MAP1_VERTEX_ATTRIB13_4_NV      0x866D
#define GL_MAP1_VERTEX_ATTRIB14_4_NV      0x866E
#define GL_MAP1_VERTEX_ATTRIB15_4_NV      0x866F
#define GL_MAP2_VERTEX_ATTRIB0_4_NV       0x8670
#define GL_MAP2_VERTEX_ATTRIB1_4_NV       0x8671
#define GL_MAP2_VERTEX_ATTRIB2_4_NV       0x8672
#define GL_MAP2_VERTEX_ATTRIB3_4_NV       0x8673
#define GL_MAP2_VERTEX_ATTRIB4_4_NV       0x8674
#define GL_MAP2_VERTEX_ATTRIB5_4_NV       0x8675
#define GL_MAP2_VERTEX_ATTRIB6_4_NV       0x8676
#define GL_MAP2_VERTEX_ATTRIB7_4_NV       0x8677
#define GL_MAP2_VERTEX_ATTRIB8_4_NV       0x8678
#define GL_MAP2_VERTEX_ATTRIB9_4_NV       0x8679
#define GL_MAP2_VERTEX_ATTRIB10_4_NV      0x867A
#define GL_MAP2_VERTEX_ATTRIB11_4_NV      0x867B
#define GL_MAP2_VERTEX_ATTRIB12_4_NV      0x867C
#define GL_MAP2_VERTEX_ATTRIB13_4_NV      0x867D
#define GL_MAP2_VERTEX_ATTRIB14_4_NV      0x867E
#define GL_MAP2_VERTEX_ATTRIB15_4_NV      0x867F
#endif

#ifndef GL_SGIX_texture_coordinate_clamp
#define GL_TEXTURE_MAX_CLAMP_S_SGIX       0x8369
#define GL_TEXTURE_MAX_CLAMP_T_SGIX       0x836A
#define GL_TEXTURE_MAX_CLAMP_R_SGIX       0x836B
#endif

#ifndef GL_SGIX_scalebias_hint
#define GL_SCALEBIAS_HINT_SGIX            0x8322
#endif

#ifndef GL_OML_interlace
#define GL_INTERLACE_OML                  0x8980
#define GL_INTERLACE_READ_OML             0x8981
#endif

#ifndef GL_OML_subsample
#define GL_FORMAT_SUBSAMPLE_24_24_OML     0x8982
#define GL_FORMAT_SUBSAMPLE_244_244_OML   0x8983
#endif

#ifndef GL_OML_resample
#define GL_PACK_RESAMPLE_OML              0x8984
#define GL_UNPACK_RESAMPLE_OML            0x8985
#define GL_RESAMPLE_REPLICATE_OML         0x8986
#define GL_RESAMPLE_ZERO_FILL_OML         0x8987
#define GL_RESAMPLE_AVERAGE_OML           0x8988
#define GL_RESAMPLE_DECIMATE_OML          0x8989
#endif

#ifndef GL_NV_copy_depth_to_color
#define GL_DEPTH_STENCIL_TO_RGBA_NV       0x886E
#define GL_DEPTH_STENCIL_TO_BGRA_NV       0x886F
#endif

#ifndef GL_ATI_envmap_bumpmap
#define GL_BUMP_ROT_MATRIX_ATI            0x8775
#define GL_BUMP_ROT_MATRIX_SIZE_ATI       0x8776
#define GL_BUMP_NUM_TEX_UNITS_ATI         0x8777
#define GL_BUMP_TEX_UNITS_ATI             0x8778
#define GL_DUDV_ATI                       0x8779
#define GL_DU8DV8_ATI                     0x877A
#define GL_BUMP_ENVMAP_ATI                0x877B
#define GL_BUMP_TARGET_ATI                0x877C
#endif

#ifndef GL_ATI_fragment_shader
#define GL_FRAGMENT_SHADER_ATI            0x8920
#define GL_REG_0_ATI                      0x8921
#define GL_REG_1_ATI                      0x8922
#define GL_REG_2_ATI                      0x8923
#define GL_REG_3_ATI                      0x8924
#define GL_REG_4_ATI                      0x8925
#define GL_REG_5_ATI                      0x8926
#define GL_REG_6_ATI                      0x8927
#define GL_REG_7_ATI                      0x8928
#define GL_REG_8_ATI                      0x8929
#define GL_REG_9_ATI                      0x892A
#define GL_REG_10_ATI                     0x892B
#define GL_REG_11_ATI                     0x892C
#define GL_REG_12_ATI                     0x892D
#define GL_REG_13_ATI                     0x892E
#define GL_REG_14_ATI                     0x892F
#define GL_REG_15_ATI                     0x8930
#define GL_REG_16_ATI                     0x8931
#define GL_REG_17_ATI                     0x8932
#define GL_REG_18_ATI                     0x8933
#define GL_REG_19_ATI                     0x8934
#define GL_REG_20_ATI                     0x8935
#define GL_REG_21_ATI                     0x8936
#define GL_REG_22_ATI                     0x8937
#define GL_REG_23_ATI                     0x8938
#define GL_REG_24_ATI                     0x8939
#define GL_REG_25_ATI                     0x893A
#define GL_REG_26_ATI                     0x893B
#define GL_REG_27_ATI                     0x893C
#define GL_REG_28_ATI                     0x893D
#define GL_REG_29_ATI                     0x893E
#define GL_REG_30_ATI                     0x893F
#define GL_REG_31_ATI                     0x8940
#define GL_CON_0_ATI                      0x8941
#define GL_CON_1_ATI                      0x8942
#define GL_CON_2_ATI                      0x8943
#define GL_CON_3_ATI                      0x8944
#define GL_CON_4_ATI                      0x8945
#define GL_CON_5_ATI                      0x8946
#define GL_CON_6_ATI                      0x8947
#define GL_CON_7_ATI                      0x8948
#define GL_CON_8_ATI                      0x8949
#define GL_CON_9_ATI                      0x894A
#define GL_CON_10_ATI                     0x894B
#define GL_CON_11_ATI                     0x894C
#define GL_CON_12_ATI                     0x894D
#define GL_CON_13_ATI                     0x894E
#define GL_CON_14_ATI                     0x894F
#define GL_CON_15_ATI                     0x8950
#define GL_CON_16_ATI                     0x8951
#define GL_CON_17_ATI                     0x8952
#define GL_CON_18_ATI                     0x8953
#define GL_CON_19_ATI                     0x8954
#define GL_CON_20_ATI                     0x8955
#define GL_CON_21_ATI                     0x8956
#define GL_CON_22_ATI                     0x8957
#define GL_CON_23_ATI                     0x8958
#define GL_CON_24_ATI                     0x8959
#define GL_CON_25_ATI                     0x895A
#define GL_CON_26_ATI                     0x895B
#define GL_CON_27_ATI                     0x895C
#define GL_CON_28_ATI                     0x895D
#define GL_CON_29_ATI                     0x895E
#define GL_CON_30_ATI                     0x895F
#define GL_CON_31_ATI                     0x8960
#define GL_MOV_ATI                        0x8961
#define GL_ADD_ATI                        0x8963
#define GL_MUL_ATI                        0x8964
#define GL_SUB_ATI                        0x8965
#define GL_DOT3_ATI                       0x8966
#define GL_DOT4_ATI                       0x8967
#define GL_MAD_ATI                        0x8968
#define GL_LERP_ATI                       0x8969
#define GL_CND_ATI                        0x896A
#define GL_CND0_ATI                       0x896B
#define GL_DOT2_ADD_ATI                   0x896C
#define GL_SECONDARY_INTERPOLATOR_ATI     0x896D
#define GL_NUM_FRAGMENT_REGISTERS_ATI     0x896E
#define GL_NUM_FRAGMENT_CONSTANTS_ATI     0x896F
#define GL_NUM_PASSES_ATI                 0x8970
#define GL_NUM_INSTRUCTIONS_PER_PASS_ATI  0x8971
#define GL_NUM_INSTRUCTIONS_TOTAL_ATI     0x8972
#define GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI 0x8973
#define GL_NUM_LOOPBACK_COMPONENTS_ATI    0x8974
#define GL_COLOR_ALPHA_PAIRING_ATI        0x8975
#define GL_SWIZZLE_STR_ATI                0x8976
#define GL_SWIZZLE_STQ_ATI                0x8977
#define GL_SWIZZLE_STR_DR_ATI             0x8978
#define GL_SWIZZLE_STQ_DQ_ATI             0x8979
#define GL_SWIZZLE_STRQ_ATI               0x897A
#define GL_SWIZZLE_STRQ_DQ_ATI            0x897B
#define GL_RED_BIT_ATI                    0x00000001
#define GL_GREEN_BIT_ATI                  0x00000002
#define GL_BLUE_BIT_ATI                   0x00000004
#define GL_2X_BIT_ATI                     0x00000001
#define GL_4X_BIT_ATI                     0x00000002
#define GL_8X_BIT_ATI                     0x00000004
#define GL_HALF_BIT_ATI                   0x00000008
#define GL_QUARTER_BIT_ATI                0x00000010
#define GL_EIGHTH_BIT_ATI                 0x00000020
#define GL_SATURATE_BIT_ATI               0x00000040
#define GL_COMP_BIT_ATI                   0x00000002
#define GL_NEGATE_BIT_ATI                 0x00000004
#define GL_BIAS_BIT_ATI                   0x00000008
#endif

#ifndef GL_ATI_pn_triangles
#define GL_PN_TRIANGLES_ATI               0x87F0
#define GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI 0x87F1
#define GL_PN_TRIANGLES_POINT_MODE_ATI    0x87F2
#define GL_PN_TRIANGLES_NORMAL_MODE_ATI   0x87F3
#define GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI 0x87F4
#define GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI 0x87F5
#define GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI 0x87F6
#define GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI 0x87F7
#define GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI 0x87F8
#endif

#ifndef GL_ATI_vertex_array_object
#define GL_STATIC_ATI                     0x8760
#define GL_DYNAMIC_ATI                    0x8761
#define GL_PRESERVE_ATI                   0x8762
#define GL_DISCARD_ATI                    0x8763
#define GL_OBJECT_BUFFER_SIZE_ATI         0x8764
#define GL_OBJECT_BUFFER_USAGE_ATI        0x8765
#define GL_ARRAY_OBJECT_BUFFER_ATI        0x8766
#define GL_ARRAY_OBJECT_OFFSET_ATI        0x8767
#endif

#ifndef GL_EXT_vertex_shader
#define GL_VERTEX_SHADER_EXT              0x8780
#define GL_VERTEX_SHADER_BINDING_EXT      0x8781
#define GL_OP_INDEX_EXT                   0x8782
#define GL_OP_NEGATE_EXT                  0x8783
#define GL_OP_DOT3_EXT                    0x8784
#define GL_OP_DOT4_EXT                    0x8785
#define GL_OP_MUL_EXT                     0x8786
#define GL_OP_ADD_EXT                     0x8787
#define GL_OP_MADD_EXT                    0x8788
#define GL_OP_FRAC_EXT                    0x8789
#define GL_OP_MAX_EXT                     0x878A
#define GL_OP_MIN_EXT                     0x878B
#define GL_OP_SET_GE_EXT                  0x878C
#define GL_OP_SET_LT_EXT                  0x878D
#define GL_OP_CLAMP_EXT                   0x878E
#define GL_OP_FLOOR_EXT                   0x878F
#define GL_OP_ROUND_EXT                   0x8790
#define GL_OP_EXP_BASE_2_EXT              0x8791
#define GL_OP_LOG_BASE_2_EXT              0x8792
#define GL_OP_POWER_EXT                   0x8793
#define GL_OP_RECIP_EXT                   0x8794
#define GL_OP_RECIP_SQRT_EXT              0x8795
#define GL_OP_SUB_EXT                     0x8796
#define GL_OP_CROSS_PRODUCT_EXT           0x8797
#define GL_OP_MULTIPLY_MATRIX_EXT         0x8798
#define GL_OP_MOV_EXT                     0x8799
#define GL_OUTPUT_VERTEX_EXT              0x879A
#define GL_OUTPUT_COLOR0_EXT              0x879B
#define GL_OUTPUT_COLOR1_EXT              0x879C
#define GL_OUTPUT_TEXTURE_COORD0_EXT      0x879D
#define GL_OUTPUT_TEXTURE_COORD1_EXT      0x879E
#define GL_OUTPUT_TEXTURE_COORD2_EXT      0x879F
#define GL_OUTPUT_TEXTURE_COORD3_EXT      0x87A0
#define GL_OUTPUT_TEXTURE_COORD4_EXT      0x87A1
#define GL_OUTPUT_TEXTURE_COORD5_EXT      0x87A2
#define GL_OUTPUT_TEXTURE_COORD6_EXT      0x87A3
#define GL_OUTPUT_TEXTURE_COORD7_EXT      0x87A4
#define GL_OUTPUT_TEXTURE_COORD8_EXT      0x87A5
#define GL_OUTPUT_TEXTURE_COORD9_EXT      0x87A6
#define GL_OUTPUT_TEXTURE_COORD10_EXT     0x87A7
#define GL_OUTPUT_TEXTURE_COORD11_EXT     0x87A8
#define GL_OUTPUT_TEXTURE_COORD12_EXT     0x87A9
#define GL_OUTPUT_TEXTURE_COORD13_EXT     0x87AA
#define GL_OUTPUT_TEXTURE_COORD14_EXT     0x87AB
#define GL_OUTPUT_TEXTURE_COORD15_EXT     0x87AC
#define GL_OUTPUT_TEXTURE_COORD16_EXT     0x87AD
#define GL_OUTPUT_TEXTURE_COORD17_EXT     0x87AE
#define GL_OUTPUT_TEXTURE_COORD18_EXT     0x87AF
#define GL_OUTPUT_TEXTURE_COORD19_EXT     0x87B0
#define GL_OUTPUT_TEXTURE_COORD20_EXT     0x87B1
#define GL_OUTPUT_TEXTURE_COORD21_EXT     0x87B2
#define GL_OUTPUT_TEXTURE_COORD22_EXT     0x87B3
#define GL_OUTPUT_TEXTURE_COORD23_EXT     0x87B4
#define GL_OUTPUT_TEXTURE_COORD24_EXT     0x87B5
#define GL_OUTPUT_TEXTURE_COORD25_EXT     0x87B6
#define GL_OUTPUT_TEXTURE_COORD26_EXT     0x87B7
#define GL_OUTPUT_TEXTURE_COORD27_EXT     0x87B8
#define GL_OUTPUT_TEXTURE_COORD28_EXT     0x87B9
#define GL_OUTPUT_TEXTURE_COORD29_EXT     0x87BA
#define GL_OUTPUT_TEXTURE_COORD30_EXT     0x87BB
#define GL_OUTPUT_TEXTURE_COORD31_EXT     0x87BC
#define GL_OUTPUT_FOG_EXT                 0x87BD
#define GL_SCALAR_EXT                     0x87BE
#define GL_VECTOR_EXT                     0x87BF
#define GL_MATRIX_EXT                     0x87C0
#define GL_VARIANT_EXT                    0x87C1
#define GL_INVARIANT_EXT                  0x87C2
#define GL_LOCAL_CONSTANT_EXT             0x87C3
#define GL_LOCAL_EXT                      0x87C4
#define GL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT 0x87C5
#define GL_MAX_VERTEX_SHADER_VARIANTS_EXT 0x87C6
#define GL_MAX_VERTEX_SHADER_INVARIANTS_EXT 0x87C7
#define GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT 0x87C8
#define GL_MAX_VERTEX_SHADER_LOCALS_EXT   0x87C9
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT 0x87CA
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT 0x87CB
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT 0x87CC
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT 0x87CD
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT 0x87CE
#define GL_VERTEX_SHADER_INSTRUCTIONS_EXT 0x87CF
#define GL_VERTEX_SHADER_VARIANTS_EXT     0x87D0
#define GL_VERTEX_SHADER_INVARIANTS_EXT   0x87D1
#define GL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT 0x87D2
#define GL_VERTEX_SHADER_LOCALS_EXT       0x87D3
#define GL_VERTEX_SHADER_OPTIMIZED_EXT    0x87D4
#define GL_X_EXT                          0x87D5
#define GL_Y_EXT                          0x87D6
#define GL_Z_EXT                          0x87D7
#define GL_W_EXT                          0x87D8
#define GL_NEGATIVE_X_EXT                 0x87D9
#define GL_NEGATIVE_Y_EXT                 0x87DA
#define GL_NEGATIVE_Z_EXT                 0x87DB
#define GL_NEGATIVE_W_EXT                 0x87DC
#define GL_ZERO_EXT                       0x87DD
#define GL_ONE_EXT                        0x87DE
#define GL_NEGATIVE_ONE_EXT               0x87DF
#define GL_NORMALIZED_RANGE_EXT           0x87E0
#define GL_FULL_RANGE_EXT                 0x87E1
#define GL_CURRENT_VERTEX_EXT             0x87E2
#define GL_MVP_MATRIX_EXT                 0x87E3
#define GL_VARIANT_VALUE_EXT              0x87E4
#define GL_VARIANT_DATATYPE_EXT           0x87E5
#define GL_VARIANT_ARRAY_STRIDE_EXT       0x87E6
#define GL_VARIANT_ARRAY_TYPE_EXT         0x87E7
#define GL_VARIANT_ARRAY_EXT              0x87E8
#define GL_VARIANT_ARRAY_POINTER_EXT      0x87E9
#define GL_INVARIANT_VALUE_EXT            0x87EA
#define GL_INVARIANT_DATATYPE_EXT         0x87EB
#define GL_LOCAL_CONSTANT_VALUE_EXT       0x87EC
#define GL_LOCAL_CONSTANT_DATATYPE_EXT    0x87ED
#endif

#ifndef GL_ATI_vertex_streams
#define GL_MAX_VERTEX_STREAMS_ATI         0x876B
#define GL_VERTEX_STREAM0_ATI             0x876C
#define GL_VERTEX_STREAM1_ATI             0x876D
#define GL_VERTEX_STREAM2_ATI             0x876E
#define GL_VERTEX_STREAM3_ATI             0x876F
#define GL_VERTEX_STREAM4_ATI             0x8770
#define GL_VERTEX_STREAM5_ATI             0x8771
#define GL_VERTEX_STREAM6_ATI             0x8772
#define GL_VERTEX_STREAM7_ATI             0x8773
#define GL_VERTEX_SOURCE_ATI              0x8774
#endif

#ifndef GL_ATI_element_array
#define GL_ELEMENT_ARRAY_ATI              0x8768
#define GL_ELEMENT_ARRAY_TYPE_ATI         0x8769
#define GL_ELEMENT_ARRAY_POINTER_ATI      0x876A
#endif

#ifndef GL_SUN_mesh_array
#define GL_QUAD_MESH_SUN                  0x8614
#define GL_TRIANGLE_MESH_SUN              0x8615
#endif

#ifndef GL_SUN_slice_accum
#define GL_SLICE_ACCUM_SUN                0x85CC
#endif

#ifndef GL_NV_multisample_filter_hint
#define GL_MULTISAMPLE_FILTER_HINT_NV     0x8534
#endif

#ifndef GL_NV_depth_clamp
#define GL_DEPTH_CLAMP_NV                 0x864F
#endif

#ifndef GL_NV_occlusion_query
#define GL_PIXEL_COUNTER_BITS_NV          0x8864
#define GL_CURRENT_OCCLUSION_QUERY_ID_NV  0x8865
#define GL_PIXEL_COUNT_NV                 0x8866
#define GL_PIXEL_COUNT_AVAILABLE_NV       0x8867
#endif

#ifndef GL_NV_point_sprite
#define GL_POINT_SPRITE_NV                0x8861
#define GL_COORD_REPLACE_NV               0x8862
#define GL_POINT_SPRITE_R_MODE_NV         0x8863
#endif

#ifndef GL_NV_texture_shader3
#define GL_OFFSET_PROJECTIVE_TEXTURE_2D_NV 0x8850
#define GL_OFFSET_PROJECTIVE_TEXTURE_2D_SCALE_NV 0x8851
#define GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_NV 0x8852
#define GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_SCALE_NV 0x8853
#define GL_OFFSET_HILO_TEXTURE_2D_NV      0x8854
#define GL_OFFSET_HILO_TEXTURE_RECTANGLE_NV 0x8855
#define GL_OFFSET_HILO_PROJECTIVE_TEXTURE_2D_NV 0x8856
#define GL_OFFSET_HILO_PROJECTIVE_TEXTURE_RECTANGLE_NV 0x8857
#define GL_DEPENDENT_HILO_TEXTURE_2D_NV   0x8858
#define GL_DEPENDENT_RGB_TEXTURE_3D_NV    0x8859
#define GL_DEPENDENT_RGB_TEXTURE_CUBE_MAP_NV 0x885A
#define GL_DOT_PRODUCT_PASS_THROUGH_NV    0x885B
#define GL_DOT_PRODUCT_TEXTURE_1D_NV      0x885C
#define GL_DOT_PRODUCT_AFFINE_DEPTH_REPLACE_NV 0x885D
#define GL_HILO8_NV                       0x885E
#define GL_SIGNED_HILO8_NV                0x885F
#define GL_FORCE_BLUE_TO_ONE_NV           0x8860
#endif

#ifndef GL_NV_vertex_program1_1
#endif

#ifndef GL_EXT_shadow_funcs
#endif

#ifndef GL_EXT_stencil_two_side
#define GL_STENCIL_TEST_TWO_SIDE_EXT      0x8910
#define GL_ACTIVE_STENCIL_FACE_EXT        0x8911
#endif
#ifndef GL_ATI_text_fragment_shader
#define GL_TEXT_FRAGMENT_SHADER_ATI       0x8200
#endif

#ifndef GL_APPLE_client_storage
#define GL_UNPACK_CLIENT_STORAGE_APPLE    0x85B2
#endif

#ifndef GL_APPLE_element_array
#define GL_ELEMENT_ARRAY_APPLE            0x8768
#define GL_ELEMENT_ARRAY_TYPE_APPLE       0x8769
#define GL_ELEMENT_ARRAY_POINTER_APPLE    0x876A
#endif

#ifndef GL_APPLE_fence
#define GL_DRAW_PIXELS_APPLE              0x8A0A
#define GL_FENCE_APPLE                    0x8A0B
#endif

#ifndef GL_APPLE_vertex_array_object
#define GL_VERTEX_ARRAY_BINDING_APPLE     0x85B5
#endif

#ifndef GL_APPLE_vertex_array_range
#define GL_VERTEX_ARRAY_RANGE_APPLE       0x851D
#define GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE 0x851E
#define GL_VERTEX_ARRAY_STORAGE_HINT_APPLE 0x851F
#define GL_VERTEX_ARRAY_RANGE_POINTER_APPLE 0x8521
#define GL_STORAGE_CACHED_APPLE           0x85BE
#define GL_STORAGE_SHARED_APPLE           0x85BF
#endif

#ifndef GL_APPLE_ycbcr_422
#define GL_YCBCR_422_APPLE                0x85B9
#define GL_UNSIGNED_SHORT_8_8_APPLE       0x85BA
#define GL_UNSIGNED_SHORT_8_8_REV_APPLE   0x85BB
#endif

#ifndef GL_S3_s3tc
#define GL_RGB_S3TC                       0x83A0
#define GL_RGB4_S3TC                      0x83A1
#define GL_RGBA_S3TC                      0x83A2
#define GL_RGBA4_S3TC                     0x83A3
#endif

#ifndef GL_ATI_draw_buffers
#define GL_MAX_DRAW_BUFFERS_ATI           0x8824
#define GL_DRAW_BUFFER0_ATI               0x8825
#define GL_DRAW_BUFFER1_ATI               0x8826
#define GL_DRAW_BUFFER2_ATI               0x8827
#define GL_DRAW_BUFFER3_ATI               0x8828
#define GL_DRAW_BUFFER4_ATI               0x8829
#define GL_DRAW_BUFFER5_ATI               0x882A
#define GL_DRAW_BUFFER6_ATI               0x882B
#define GL_DRAW_BUFFER7_ATI               0x882C
#define GL_DRAW_BUFFER8_ATI               0x882D
#define GL_DRAW_BUFFER9_ATI               0x882E
#define GL_DRAW_BUFFER10_ATI              0x882F
#define GL_DRAW_BUFFER11_ATI              0x8830
#define GL_DRAW_BUFFER12_ATI              0x8831
#define GL_DRAW_BUFFER13_ATI              0x8832
#define GL_DRAW_BUFFER14_ATI              0x8833
#define GL_DRAW_BUFFER15_ATI              0x8834
#endif

#ifndef GL_ATI_texture_env_combine3
#define GL_MODULATE_ADD_ATI               0x8744
#define GL_MODULATE_SIGNED_ADD_ATI        0x8745
#define GL_MODULATE_SUBTRACT_ATI          0x8746
#endif


#ifndef GL_ATI_texture_float
#define GL_RGBA_FLOAT32_ATI               0x8814
#define GL_RGB_FLOAT32_ATI                0x8815
#define GL_ALPHA_FLOAT32_ATI              0x8816
#define GL_INTENSITY_FLOAT32_ATI          0x8817
#define GL_LUMINANCE_FLOAT32_ATI          0x8818
#define GL_LUMINANCE_ALPHA_FLOAT32_ATI    0x8819
#define GL_RGBA_FLOAT16_ATI               0x881A
#define GL_RGB_FLOAT16_ATI                0x881B
#define GL_ALPHA_FLOAT16_ATI              0x881C
#define GL_INTENSITY_FLOAT16_ATI          0x881D
#define GL_LUMINANCE_FLOAT16_ATI          0x881E
#define GL_LUMINANCE_ALPHA_FLOAT16_ATI    0x881F
#endif

#ifndef GL_NV_float_buffer
#define GL_FLOAT_R_NV                     0x8880
#define GL_FLOAT_RG_NV                    0x8881
#define GL_FLOAT_RGB_NV                   0x8882
#define GL_FLOAT_RGBA_NV                  0x8883
#define GL_FLOAT_R16_NV                   0x8884
#define GL_FLOAT_R32_NV                   0x8885
#define GL_FLOAT_RG16_NV                  0x8886
#define GL_FLOAT_RG32_NV                  0x8887
#define GL_FLOAT_RGB16_NV                 0x8888
#define GL_FLOAT_RGB32_NV                 0x8889
#define GL_FLOAT_RGBA16_NV                0x888A
#define GL_FLOAT_RGBA32_NV                0x888B
#define GL_TEXTURE_FLOAT_COMPONENTS_NV    0x888C
#define GL_FLOAT_CLEAR_COLOR_VALUE_NV     0x888D
#define GL_FLOAT_RGBA_MODE_NV             0x888E
#endif

#ifndef GL_NV_fragment_program
#define GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV 0x8868
#define GL_FRAGMENT_PROGRAM_NV            0x8870
#define GL_MAX_TEXTURE_COORDS_NV          0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS_NV     0x8872
#define GL_FRAGMENT_PROGRAM_BINDING_NV    0x8873
#define GL_PROGRAM_ERROR_STRING_NV        0x8874
#endif

#ifndef GL_NV_half_float
#define GL_HALF_FLOAT_NV                  0x140B
#endif

#ifndef GL_NV_pixel_data_range
#define GL_WRITE_PIXEL_DATA_RANGE_NV      0x8878
#define GL_READ_PIXEL_DATA_RANGE_NV       0x8879
#define GL_WRITE_PIXEL_DATA_RANGE_LENGTH_NV 0x887A
#define GL_READ_PIXEL_DATA_RANGE_LENGTH_NV 0x887B
#define GL_WRITE_PIXEL_DATA_RANGE_POINTER_NV 0x887C
#define GL_READ_PIXEL_DATA_RANGE_POINTER_NV 0x887D
#endif

#ifndef GL_NV_primitive_restart
#define GL_PRIMITIVE_RESTART_NV           0x8558
#define GL_PRIMITIVE_RESTART_INDEX_NV     0x8559
#endif

#ifndef GL_NV_texture_expand_normal
#define GL_TEXTURE_UNSIGNED_REMAP_MODE_NV 0x888F
#endif

#ifndef GL_NV_vertex_program2
#endif

#ifndef GL_ATI_map_object_buffer
#endif

#ifndef GL_ATI_separate_stencil
#define GL_STENCIL_BACK_FUNC_ATI          0x8800
#define GL_STENCIL_BACK_FAIL_ATI          0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL_ATI 0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS_ATI 0x8803
#endif

#ifndef GL_ATI_vertex_attrib_array_object
#endif


/*************************************************************/

#ifndef GL_VERSION_1_2
#define GL_VERSION_1_2 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glBlendColor (GLclampf, GLclampf, GLclampf, GLclampf);
GLAPI void GLAPIENTRY glBlendEquation (GLenum);
GLAPI void GLAPIENTRY glDrawRangeElements (GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glColorTable (GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glColorTableParameterfv (GLenum, GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glColorTableParameteriv (GLenum, GLenum, const GLint *);
GLAPI void GLAPIENTRY glCopyColorTable (GLenum, GLenum, GLint, GLint, GLsizei);
GLAPI void GLAPIENTRY glGetColorTable (GLenum, GLenum, GLenum, GLvoid *);
GLAPI void GLAPIENTRY glGetColorTableParameterfv (GLenum, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetColorTableParameteriv (GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glColorSubTable (GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glCopyColorSubTable (GLenum, GLsizei, GLint, GLint, GLsizei);
GLAPI void GLAPIENTRY glConvolutionFilter1D (GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glConvolutionFilter2D (GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glConvolutionParameterf (GLenum, GLenum, GLfloat);
GLAPI void GLAPIENTRY glConvolutionParameterfv (GLenum, GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glConvolutionParameteri (GLenum, GLenum, GLint);
GLAPI void GLAPIENTRY glConvolutionParameteriv (GLenum, GLenum, const GLint *);
GLAPI void GLAPIENTRY glCopyConvolutionFilter1D (GLenum, GLenum, GLint, GLint, GLsizei);
GLAPI void GLAPIENTRY glCopyConvolutionFilter2D (GLenum, GLenum, GLint, GLint, GLsizei, GLsizei);
GLAPI void GLAPIENTRY glGetConvolutionFilter (GLenum, GLenum, GLenum, GLvoid *);
GLAPI void GLAPIENTRY glGetConvolutionParameterfv (GLenum, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetConvolutionParameteriv (GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetSeparableFilter (GLenum, GLenum, GLenum, GLvoid *, GLvoid *, GLvoid *);
GLAPI void GLAPIENTRY glSeparableFilter2D (GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *, const GLvoid *);
GLAPI void GLAPIENTRY glGetHistogram (GLenum, GLboolean, GLenum, GLenum, GLvoid *);
GLAPI void GLAPIENTRY glGetHistogramParameterfv (GLenum, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetHistogramParameteriv (GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetMinmax (GLenum, GLboolean, GLenum, GLenum, GLvoid *);
GLAPI void GLAPIENTRY glGetMinmaxParameterfv (GLenum, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetMinmaxParameteriv (GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glHistogram (GLenum, GLsizei, GLenum, GLboolean);
GLAPI void GLAPIENTRY glMinmax (GLenum, GLenum, GLboolean);
GLAPI void GLAPIENTRY glResetHistogram (GLenum);
GLAPI void GLAPIENTRY glResetMinmax (GLenum);
GLAPI void GLAPIENTRY glTexImage3D (GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glTexSubImage3D (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glCopyTexSubImage3D (GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLBLENDCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef FNGLBLENDCOLORPROC *PFNGLBLENDCOLORPROC;

typedef void (GLAPIENTRY FNGLBLENDEQUATIONPROC) (GLenum mode);
typedef FNGLBLENDEQUATIONPROC *PFNGLBLENDEQUATIONPROC;

typedef void (GLAPIENTRY FNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
typedef FNGLDRAWRANGEELEMENTSPROC *PFNGLDRAWRANGEELEMENTSPROC;

typedef void (GLAPIENTRY FNGLCOLORTABLEPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef FNGLCOLORTABLEPROC *PFNGLCOLORTABLEPROC;

typedef void (GLAPIENTRY FNGLCOLORTABLEPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef FNGLCOLORTABLEPARAMETERFVPROC *PFNGLCOLORTABLEPARAMETERFVPROC;

typedef void (GLAPIENTRY FNGLCOLORTABLEPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef FNGLCOLORTABLEPARAMETERIVPROC *PFNGLCOLORTABLEPARAMETERIVPROC;

typedef void (GLAPIENTRY FNGLCOPYCOLORTABLEPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef FNGLCOPYCOLORTABLEPROC *PFNGLCOPYCOLORTABLEPROC;

typedef void (GLAPIENTRY FNGLGETCOLORTABLEPROC) (GLenum target, GLenum format, GLenum type, GLvoid *table);
typedef FNGLGETCOLORTABLEPROC *PFNGLGETCOLORTABLEPROC;

typedef void (GLAPIENTRY FNGLGETCOLORTABLEPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef FNGLGETCOLORTABLEPARAMETERFVPROC *FNGLGETCOLORTABLEPARAMETERFVPROC;

typedef void (GLAPIENTRY FNGLGETCOLORTABLEPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef FNGLGETCOLORTABLEPARAMETERIVPROC *PFNGLGETCOLORTABLEPARAMETERIVPROC;

typedef void (GLAPIENTRY FNGLCOLORSUBTABLEPROC) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
typedef FNGLCOLORSUBTABLEPROC *PFNGLCOLORSUBTABLEPROC;

typedef void (GLAPIENTRY FNGLCOPYCOLORSUBTABLEPROC) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
typedef FNGLCOPYCOLORSUBTABLEPROC *PFNGLCOPYCOLORSUBTABLEPROC;

typedef void (GLAPIENTRY FNGLCONVOLUTIONFILTER1DPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
typedef FNGLCONVOLUTIONFILTER1DPROC *PFNGLCONVOLUTIONFILTER1DPROC;

typedef void (GLAPIENTRY FNGLCONVOLUTIONFILTER2DPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
typedef FNGLCONVOLUTIONFILTER2DPROC *PFNGLCONVOLUTIONFILTER2DPROC;

typedef void (GLAPIENTRY FNGLCONVOLUTIONPARAMETERFPROC) (GLenum target, GLenum pname, GLfloat params);
typedef FNGLCONVOLUTIONPARAMETERFPROC * PFNGLCONVOLUTIONPARAMETERFPROC;

typedef void (GLAPIENTRY FNGLCONVOLUTIONPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef FNGLCONVOLUTIONPARAMETERFVPROC  *PFNGLCONVOLUTIONPARAMETERFVPROC;

typedef void (GLAPIENTRY FNGLCONVOLUTIONPARAMETERIPROC) (GLenum target, GLenum pname, GLint params);
typedef FNGLCONVOLUTIONPARAMETERIPROC *PFNGLCONVOLUTIONPARAMETERIPROC;

typedef void (GLAPIENTRY FNGLCONVOLUTIONPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef FNGLCONVOLUTIONPARAMETERIVPROC *PFNGLCONVOLUTIONPARAMETERIVPROC;

typedef void (GLAPIENTRY FNGLCOPYCONVOLUTIONFILTER1DPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef FNGLCOPYCONVOLUTIONFILTER1DPROC *PFNGLCOPYCONVOLUTIONFILTER1DPROC;

typedef void (GLAPIENTRY FNGLCOPYCONVOLUTIONFILTER2DPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
typedef FNGLCOPYCONVOLUTIONFILTER2DPROC *PFNGLCOPYCONVOLUTIONFILTER2DPROC;

typedef void (GLAPIENTRY FNGLGETCONVOLUTIONFILTERPROC) (GLenum target, GLenum format, GLenum type, GLvoid *image);
typedef FNGLGETCONVOLUTIONFILTERPROC *PFNGLGETCONVOLUTIONFILTERPROC;

typedef void (GLAPIENTRY FNGLGETCONVOLUTIONPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef FNGLGETCONVOLUTIONPARAMETERFVPROC *PFNGLGETCONVOLUTIONPARAMETERFVPROC;

typedef void (GLAPIENTRY FNGLGETCONVOLUTIONPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef FNGLGETCONVOLUTIONPARAMETERIVPROC *PFNGLGETCONVOLUTIONPARAMETERIVPROC;

typedef void (GLAPIENTRY FNGLGETSEPARABLEFILTERPROC) (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
typedef FNGLGETSEPARABLEFILTERPROC *PFNGLGETSEPARABLEFILTERPROC;

typedef void (GLAPIENTRY FNGLSEPARABLEFILTER2DPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
typedef FNGLSEPARABLEFILTER2DPROC *PFNGLSEPARABLEFILTER2DPROC;

typedef void (GLAPIENTRY FNGLGETHISTOGRAMPROC) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef FNGLGETHISTOGRAMPROC *PFNGLGETHISTOGRAMPROC;

typedef void (GLAPIENTRY FNGLGETHISTOGRAMPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef FNGLGETHISTOGRAMPARAMETERFVPROC *PFNGLGETHISTOGRAMPARAMETERFVPROC;

typedef void (GLAPIENTRY FNGLGETHISTOGRAMPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef FNGLGETHISTOGRAMPARAMETERIVPROC *PFNGLGETHISTOGRAMPARAMETERIVPROC;

typedef void (GLAPIENTRY FNGLGETMINMAXPROC) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef FNGLGETMINMAXPROC *PFNGLGETMINMAXPROC;

typedef void (GLAPIENTRY FNGLGETMINMAXPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef FNGLGETMINMAXPARAMETERFVPROC *PFNGLGETMINMAXPARAMETERFVPROC;

typedef void (GLAPIENTRY FNGLGETMINMAXPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef FNGLGETMINMAXPARAMETERIVPROC *PFNGLGETMINMAXPARAMETERIVPROC;

typedef void (GLAPIENTRY FNGLHISTOGRAMPROC) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
typedef FNGLHISTOGRAMPROC *PFNGLHISTOGRAMPROC;

typedef void (GLAPIENTRY FNGLMINMAXPROC) (GLenum target, GLenum internalformat, GLboolean sink);
typedef FNGLMINMAXPROC *PFNGLMINMAXPROC;

typedef void (GLAPIENTRY FNGLRESETHISTOGRAMPROC) (GLenum target);
typedef FNGLRESETHISTOGRAMPROC *PFNGLRESETHISTOGRAMPROC;

typedef void (GLAPIENTRY FNGLRESETMINMAXPROC) (GLenum target);
typedef FNGLRESETMINMAXPROC *PFNGLRESETMINMAXPROC;

typedef void (GLAPIENTRY FNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef FNGLTEXIMAGE3DPROC *PFNGLTEXIMAGE3DPROC;

typedef void (GLAPIENTRY FNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef FNGLTEXSUBIMAGE3DPROC *PFNGLTEXSUBIMAGE3DPROC;

typedef void (GLAPIENTRY FNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef FNGLCOPYTEXSUBIMAGE3DPROC *PFNGLCOPYTEXSUBIMAGE3DPROC;

#endif

#ifndef GL_VERSION_1_3
#define GL_VERSION_1_3 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glActiveTexture (GLenum);
GLAPI void GLAPIENTRY glClientActiveTexture (GLenum);
GLAPI void GLAPIENTRY glMultiTexCoord1d (GLenum, GLdouble);
GLAPI void GLAPIENTRY glMultiTexCoord1dv (GLenum, const GLdouble *);
GLAPI void GLAPIENTRY glMultiTexCoord1f (GLenum, GLfloat);
GLAPI void GLAPIENTRY glMultiTexCoord1fv (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glMultiTexCoord1i (GLenum, GLint);
GLAPI void GLAPIENTRY glMultiTexCoord1iv (GLenum, const GLint *);
GLAPI void GLAPIENTRY glMultiTexCoord1s (GLenum, GLshort);
GLAPI void GLAPIENTRY glMultiTexCoord1sv (GLenum, const GLshort *);
GLAPI void GLAPIENTRY glMultiTexCoord2d (GLenum, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glMultiTexCoord2dv (GLenum, const GLdouble *);
GLAPI void GLAPIENTRY glMultiTexCoord2f (GLenum, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glMultiTexCoord2fv (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glMultiTexCoord2i (GLenum, GLint, GLint);
GLAPI void GLAPIENTRY glMultiTexCoord2iv (GLenum, const GLint *);
GLAPI void GLAPIENTRY glMultiTexCoord2s (GLenum, GLshort, GLshort);
GLAPI void GLAPIENTRY glMultiTexCoord2sv (GLenum, const GLshort *);
GLAPI void GLAPIENTRY glMultiTexCoord3d (GLenum, GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glMultiTexCoord3dv (GLenum, const GLdouble *);
GLAPI void GLAPIENTRY glMultiTexCoord3f (GLenum, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glMultiTexCoord3fv (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glMultiTexCoord3i (GLenum, GLint, GLint, GLint);
GLAPI void GLAPIENTRY glMultiTexCoord3iv (GLenum, const GLint *);
GLAPI void GLAPIENTRY glMultiTexCoord3s (GLenum, GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glMultiTexCoord3sv (GLenum, const GLshort *);
GLAPI void GLAPIENTRY glMultiTexCoord4d (GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glMultiTexCoord4dv (GLenum, const GLdouble *);
GLAPI void GLAPIENTRY glMultiTexCoord4f (GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glMultiTexCoord4fv (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glMultiTexCoord4i (GLenum, GLint, GLint, GLint, GLint);
GLAPI void GLAPIENTRY glMultiTexCoord4iv (GLenum, const GLint *);
GLAPI void GLAPIENTRY glMultiTexCoord4s (GLenum, GLshort, GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glMultiTexCoord4sv (GLenum, const GLshort *);
GLAPI void GLAPIENTRY glLoadTransposeMatrixf (const GLfloat *);
GLAPI void GLAPIENTRY glLoadTransposeMatrixd (const GLdouble *);
GLAPI void GLAPIENTRY glMultTransposeMatrixf (const GLfloat *);
GLAPI void GLAPIENTRY glMultTransposeMatrixd (const GLdouble *);
GLAPI void GLAPIENTRY glSampleCoverage (GLclampf, GLboolean);
GLAPI void GLAPIENTRY glCompressedTexImage3D (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glCompressedTexImage2D (GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glCompressedTexImage1D (GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glCompressedTexSubImage3D (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glCompressedTexSubImage2D (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glCompressedTexSubImage1D (GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glGetCompressedTexImage (GLenum, GLint, void *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLACTIVETEXTUREPROC) (GLenum texture);
typedef FNGLACTIVETEXTUREPROC *PFNGLACTIVETEXTUREPROC;
typedef void (GLAPIENTRY FNGLCLIENTACTIVETEXTUREPROC) (GLenum texture);
typedef FNGLCLIENTACTIVETEXTUREPROC *PFNGLCLIENTACTIVETEXTUREPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD1DPROC) (GLenum target, GLdouble s);
typedef FNGLMULTITEXCOORD1DPROC *PFNGLMULTITEXCOORD1DPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD1DVPROC) (GLenum target, const GLdouble *v);
typedef FNGLMULTITEXCOORD1DVPROC *PFNGLMULTITEXCOORD1DVPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD1FPROC) (GLenum target, GLfloat s);
typedef FNGLMULTITEXCOORD1FPROC *PFNGLMULTITEXCOORD1FPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD1FVPROC) (GLenum target, const GLfloat *v);
typedef FNGLMULTITEXCOORD1FVPROC *PFNGLMULTITEXCOORD1FVPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD1IPROC) (GLenum target, GLint s);
typedef FNGLMULTITEXCOORD1IPROC *PFNGLMULTITEXCOORD1IPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD1IVPROC) (GLenum target, const GLint *v);
typedef FNGLMULTITEXCOORD1IVPROC *PFNGLMULTITEXCOORD1IVPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD1SPROC) (GLenum target, GLshort s);
typedef FNGLMULTITEXCOORD1SPROC *PFNGLMULTITEXCOORD1SPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD1SVPROC) (GLenum target, const GLshort *v);
typedef FNGLMULTITEXCOORD1SVPROC *PFNGLMULTITEXCOORD1SVPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD2DPROC) (GLenum target, GLdouble s, GLdouble t);
typedef FNGLMULTITEXCOORD2DPROC *PFNGLMULTITEXCOORD2DPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD2DVPROC) (GLenum target, const GLdouble *v);
typedef FNGLMULTITEXCOORD2DVPROC *PFNGLMULTITEXCOORD2DVPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD2FPROC) (GLenum target, GLfloat s, GLfloat t);
typedef FNGLMULTITEXCOORD2FPROC *PFNGLMULTITEXCOORD2FPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD2FVPROC) (GLenum target, const GLfloat *v);
typedef FNGLMULTITEXCOORD2FVPROC *PFNGLMULTITEXCOORD2FVPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD2IPROC) (GLenum target, GLint s, GLint t);
typedef FNGLMULTITEXCOORD2IPROC *PFNGLMULTITEXCOORD2IPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD2IVPROC) (GLenum target, const GLint *v);
typedef FNGLMULTITEXCOORD2IVPROC *PFNGLMULTITEXCOORD2IVPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD2SPROC) (GLenum target, GLshort s, GLshort t);
typedef FNGLMULTITEXCOORD2SPROC *PFNGLMULTITEXCOORD2SPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD2SVPROC) (GLenum target, const GLshort *v);
typedef FNGLMULTITEXCOORD2SVPROC *PFNGLMULTITEXCOORD2SVPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD3DPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef FNGLMULTITEXCOORD3DPROC *PFNGLMULTITEXCOORD3DPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD3DVPROC) (GLenum target, const GLdouble *v);
typedef FNGLMULTITEXCOORD3DVPROC *PFNGLMULTITEXCOORD3DVPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD3FPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef FNGLMULTITEXCOORD3FPROC *PFNGLMULTITEXCOORD3FPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD3FVPROC) (GLenum target, const GLfloat *v);
typedef FNGLMULTITEXCOORD3FVPROC *PFNGLMULTITEXCOORD3FVPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD3IPROC) (GLenum target, GLint s, GLint t, GLint r);
typedef FNGLMULTITEXCOORD3IPROC *PFNGLMULTITEXCOORD3IPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD3IVPROC) (GLenum target, const GLint *v);
typedef FNGLMULTITEXCOORD3IVPROC *PFNGLMULTITEXCOORD3IVPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD3SPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef FNGLMULTITEXCOORD3SPROC *PFNGLMULTITEXCOORD3SPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD3SVPROC) (GLenum target, const GLshort *v);
typedef FNGLMULTITEXCOORD3SVPROC *PFNGLMULTITEXCOORD3SVPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD4DPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef FNGLMULTITEXCOORD4DPROC *PFNGLMULTITEXCOORD4DPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD4DVPROC) (GLenum target, const GLdouble *v);
typedef FNGLMULTITEXCOORD4DVPROC *PFNGLMULTITEXCOORD4DVPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD4FPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef FNGLMULTITEXCOORD4FPROC *PFNGLMULTITEXCOORD4FPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD4FVPROC) (GLenum target, const GLfloat *v);
typedef FNGLMULTITEXCOORD4FVPROC *PFNGLMULTITEXCOORD4FVPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD4IPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef FNGLMULTITEXCOORD4IPROC *PFNGLMULTITEXCOORD4IPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD4IVPROC) (GLenum target, const GLint *v);
typedef FNGLMULTITEXCOORD4IVPROC *PFNGLMULTITEXCOORD4IVPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD4SPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef FNGLMULTITEXCOORD4SPROC *PFNGLMULTITEXCOORD4SPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD4SVPROC) (GLenum target, const GLshort *v);
typedef FNGLMULTITEXCOORD4SVPROC *PFNGLMULTITEXCOORD4SVPROC;
typedef void (GLAPIENTRY FNGLLOADTRANSPOSEMATRIXFPROC) (const GLfloat *m);
typedef FNGLLOADTRANSPOSEMATRIXFPROC *PFNGLLOADTRANSPOSEMATRIXFPROC;
typedef void (GLAPIENTRY FNGLLOADTRANSPOSEMATRIXDPROC) (const GLdouble *m);
typedef FNGLLOADTRANSPOSEMATRIXDPROC *PFNGLLOADTRANSPOSEMATRIXDPROC;
typedef void (GLAPIENTRY FNGLMULTTRANSPOSEMATRIXFPROC) (const GLfloat *m);
typedef FNGLMULTTRANSPOSEMATRIXFPROC *PFNGLMULTTRANSPOSEMATRIXFPROC;
typedef void (GLAPIENTRY FNGLMULTTRANSPOSEMATRIXDPROC) (const GLdouble *m);
typedef FNGLMULTTRANSPOSEMATRIXDPROC *PFNGLMULTTRANSPOSEMATRIXDPROC;
typedef void (GLAPIENTRY FNGLSAMPLECOVERAGEPROC) (GLclampf value, GLboolean invert);
typedef FNGLSAMPLECOVERAGEPROC *PFNGLSAMPLECOVERAGEPROC;
typedef void (GLAPIENTRY FNGLCOMPRESSEDTEXIMAGE3DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
typedef FNGLCOMPRESSEDTEXIMAGE3DPROC *PFNGLCOMPRESSEDTEXIMAGE3DPROC;
typedef void (GLAPIENTRY FNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
typedef FNGLCOMPRESSEDTEXIMAGE2DPROC *PFNGLCOMPRESSEDTEXIMAGE2DPROC;
typedef void (GLAPIENTRY FNGLCOMPRESSEDTEXIMAGE1DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
typedef FNGLCOMPRESSEDTEXIMAGE1DPROC *PFNGLCOMPRESSEDTEXIMAGE1DPROC;
typedef void (GLAPIENTRY FNGLCOMPRESSEDTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef FNGLCOMPRESSEDTEXSUBIMAGE3DPROC *PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC;
typedef void (GLAPIENTRY FNGLCOMPRESSEDTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef FNGLCOMPRESSEDTEXSUBIMAGE2DPROC *PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC;
typedef void (GLAPIENTRY FNGLCOMPRESSEDTEXSUBIMAGE1DPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef FNGLCOMPRESSEDTEXSUBIMAGE1DPROC *PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC;
typedef void (GLAPIENTRY FNGLGETCOMPRESSEDTEXIMAGEPROC) (GLenum target, GLint level, void *img);
typedef FNGLGETCOMPRESSEDTEXIMAGEPROC *PFNGLGETCOMPRESSEDTEXIMAGEPROC;
#endif

#ifndef GL_VERSION_1_4
#define GL_VERSION_1_4 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glBlendFuncSeparate (GLenum, GLenum, GLenum, GLenum);
GLAPI void GLAPIENTRY glFogCoordf (GLfloat);
GLAPI void GLAPIENTRY glFogCoordfv (const GLfloat *);
GLAPI void GLAPIENTRY glFogCoordd (GLdouble);
GLAPI void GLAPIENTRY glFogCoorddv (const GLdouble *);
GLAPI void GLAPIENTRY glFogCoordPointer (GLenum, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glMultiDrawArrays (GLenum, GLint *, GLsizei *, GLsizei);
GLAPI void GLAPIENTRY glMultiDrawElements (GLenum, const GLsizei *, GLenum, const GLvoid* *, GLsizei);
GLAPI void GLAPIENTRY glPointParameterf (GLenum, GLfloat);
GLAPI void GLAPIENTRY glPointParameterfv (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glPointParameteri (GLenum, GLint);
GLAPI void GLAPIENTRY glPointParameteriv (GLenum, const GLint *);
GLAPI void GLAPIENTRY glSecondaryColor3b (GLbyte, GLbyte, GLbyte);
GLAPI void GLAPIENTRY glSecondaryColor3bv (const GLbyte *);
GLAPI void GLAPIENTRY glSecondaryColor3d (GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glSecondaryColor3dv (const GLdouble *);
GLAPI void GLAPIENTRY glSecondaryColor3f (GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glSecondaryColor3fv (const GLfloat *);
GLAPI void GLAPIENTRY glSecondaryColor3i (GLint, GLint, GLint);
GLAPI void GLAPIENTRY glSecondaryColor3iv (const GLint *);
GLAPI void GLAPIENTRY glSecondaryColor3s (GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glSecondaryColor3sv (const GLshort *);
GLAPI void GLAPIENTRY glSecondaryColor3ub (GLubyte, GLubyte, GLubyte);
GLAPI void GLAPIENTRY glSecondaryColor3ubv (const GLubyte *);
GLAPI void GLAPIENTRY glSecondaryColor3ui (GLuint, GLuint, GLuint);
GLAPI void GLAPIENTRY glSecondaryColor3uiv (const GLuint *);
GLAPI void GLAPIENTRY glSecondaryColor3us (GLushort, GLushort, GLushort);
GLAPI void GLAPIENTRY glSecondaryColor3usv (const GLushort *);
GLAPI void GLAPIENTRY glSecondaryColorPointer (GLint, GLenum, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glWindowPos2d (GLdouble, GLdouble);
GLAPI void GLAPIENTRY glWindowPos2dv (const GLdouble *);
GLAPI void GLAPIENTRY glWindowPos2f (GLfloat, GLfloat);
GLAPI void GLAPIENTRY glWindowPos2fv (const GLfloat *);
GLAPI void GLAPIENTRY glWindowPos2i (GLint, GLint);
GLAPI void GLAPIENTRY glWindowPos2iv (const GLint *);
GLAPI void GLAPIENTRY glWindowPos2s (GLshort, GLshort);
GLAPI void GLAPIENTRY glWindowPos2sv (const GLshort *);
GLAPI void GLAPIENTRY glWindowPos3d (GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glWindowPos3dv (const GLdouble *);
GLAPI void GLAPIENTRY glWindowPos3f (GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glWindowPos3fv (const GLfloat *);
GLAPI void GLAPIENTRY glWindowPos3i (GLint, GLint, GLint);
GLAPI void GLAPIENTRY glWindowPos3iv (const GLint *);
GLAPI void GLAPIENTRY glWindowPos3s (GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glWindowPos3sv (const GLshort *);
#endif /* GL_GLEXT_PROTOTYPES */

typedef void (GLAPIENTRY FNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef FNGLBLENDFUNCSEPARATEPROC *PFNGLBLENDFUNCSEPARATEPROC;
typedef void (GLAPIENTRY FNGLFOGCOORDFPROC) (GLfloat coord);
typedef FNGLFOGCOORDFPROC *PFNGLFOGCOORDFPROC;
typedef void (GLAPIENTRY FNGLFOGCOORDFVPROC) (const GLfloat *coord);
typedef FNGLFOGCOORDFVPROC *PFNGLFOGCOORDFVPROC;
typedef void (GLAPIENTRY FNGLFOGCOORDDPROC) (GLdouble coord);
typedef FNGLFOGCOORDDPROC *PFNGLFOGCOORDDPROC;
typedef void (GLAPIENTRY FNGLFOGCOORDDVPROC) (const GLdouble *coord);
typedef FNGLFOGCOORDDVPROC *PFNGLFOGCOORDDVPROC;
typedef void (GLAPIENTRY FNGLFOGCOORDPOINTERPROC) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef FNGLFOGCOORDPOINTERPROC *PFNGLFOGCOORDPOINTERPROC;
typedef void (GLAPIENTRY FNGLMULTIDRAWARRAYSPROC) (GLenum mode, GLint *first, GLsizei *count, GLsizei primcount);
typedef FNGLMULTIDRAWARRAYSPROC *PFNGLMULTIDRAWARRAYSPROC;
typedef void (GLAPIENTRY FNGLMULTIDRAWELEMENTSPROC) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount);
typedef FNGLMULTIDRAWELEMENTSPROC *PFNGLMULTIDRAWELEMENTSPROC;
typedef void (GLAPIENTRY FNGLPOINTPARAMETERFPROC) (GLenum pname, GLfloat param);
typedef FNGLPOINTPARAMETERFPROC *PFNGLPOINTPARAMETERFPROC;
typedef void (GLAPIENTRY FNGLPOINTPARAMETERFVPROC) (GLenum pname, const GLfloat *params);
typedef FNGLPOINTPARAMETERFVPROC *PFNGLPOINTPARAMETERFVPROC;
typedef void (GLAPIENTRY FNGLPOINTPARAMETERIPROC) (GLenum pname, GLint param);
typedef FNGLPOINTPARAMETERIPROC *PFNGLPOINTPARAMETERIPROC;
typedef void (GLAPIENTRY FNGLPOINTPARAMETERIVPROC) (GLenum pname, const GLint *params);
typedef FNGLPOINTPARAMETERIVPROC *PFNGLPOINTPARAMETERIVPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3BPROC) (GLbyte red, GLbyte green, GLbyte blue);
typedef FNGLSECONDARYCOLOR3BPROC *PFNGLSECONDARYCOLOR3BPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3BVPROC) (const GLbyte *v);
typedef FNGLSECONDARYCOLOR3BVPROC *PFNGLSECONDARYCOLOR3BVPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3DPROC) (GLdouble red, GLdouble green, GLdouble blue);
typedef FNGLSECONDARYCOLOR3DPROC *PFNGLSECONDARYCOLOR3DPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3DVPROC) (const GLdouble *v);
typedef FNGLSECONDARYCOLOR3DVPROC *PFNGLSECONDARYCOLOR3DVPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3FPROC) (GLfloat red, GLfloat green, GLfloat blue);
typedef FNGLSECONDARYCOLOR3FPROC *PFNGLSECONDARYCOLOR3FPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3FVPROC) (const GLfloat *v);
typedef FNGLSECONDARYCOLOR3FVPROC *PFNGLSECONDARYCOLOR3FVPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3IPROC) (GLint red, GLint green, GLint blue);
typedef FNGLSECONDARYCOLOR3IPROC *PFNGLSECONDARYCOLOR3IPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3IVPROC) (const GLint *v);
typedef FNGLSECONDARYCOLOR3IVPROC *PFNGLSECONDARYCOLOR3IVPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3SPROC) (GLshort red, GLshort green, GLshort blue);
typedef FNGLSECONDARYCOLOR3SPROC *PFNGLSECONDARYCOLOR3SPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3SVPROC) (const GLshort *v);
typedef FNGLSECONDARYCOLOR3SVPROC *PFNGLSECONDARYCOLOR3SVPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3UBPROC) (GLubyte red, GLubyte green, GLubyte blue);
typedef FNGLSECONDARYCOLOR3UBPROC *PFNGLSECONDARYCOLOR3UBPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3UBVPROC) (const GLubyte *v);
typedef FNGLSECONDARYCOLOR3UBVPROC *PFNGLSECONDARYCOLOR3UBVPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3UIPROC) (GLuint red, GLuint green, GLuint blue);
typedef FNGLSECONDARYCOLOR3UIPROC *PFNGLSECONDARYCOLOR3UIPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3UIVPROC) (const GLuint *v);
typedef FNGLSECONDARYCOLOR3UIVPROC *PFNGLSECONDARYCOLOR3UIVPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3USPROC) (GLushort red, GLushort green, GLushort blue);
typedef FNGLSECONDARYCOLOR3USPROC *PFNGLSECONDARYCOLOR3USPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3USVPROC) (const GLushort *v);
typedef FNGLSECONDARYCOLOR3USVPROC *PFNGLSECONDARYCOLOR3USVPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLORPOINTERPROC) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef FNGLSECONDARYCOLORPOINTERPROC *PFNGLSECONDARYCOLORPOINTERPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2DPROC) (GLdouble x, GLdouble y);
typedef FNGLWINDOWPOS2DPROC *PFNGLWINDOWPOS2DPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2DVPROC) (const GLdouble *v);
typedef FNGLWINDOWPOS2DVPROC *PFNGLWINDOWPOS2DVPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2FPROC) (GLfloat x, GLfloat y);
typedef FNGLWINDOWPOS2FPROC *PFNGLWINDOWPOS2FPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2FVPROC) (const GLfloat *v);
typedef FNGLWINDOWPOS2FVPROC *PFNGLWINDOWPOS2FVPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2IPROC) (GLint x, GLint y);
typedef FNGLWINDOWPOS2IPROC *PFNGLWINDOWPOS2IPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2IVPROC) (const GLint *v);
typedef FNGLWINDOWPOS2IVPROC *PFNGLWINDOWPOS2IVPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2SPROC) (GLshort x, GLshort y);
typedef FNGLWINDOWPOS2SPROC *PFNGLWINDOWPOS2SPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2SVPROC) (const GLshort *v);
typedef FNGLWINDOWPOS2SVPROC *PFNGLWINDOWPOS2SVPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3DPROC) (GLdouble x, GLdouble y, GLdouble z);
typedef FNGLWINDOWPOS3DPROC *PFNGLWINDOWPOS3DPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3DVPROC) (const GLdouble *v);
typedef FNGLWINDOWPOS3DVPROC *PFNGLWINDOWPOS3DVPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3FPROC) (GLfloat x, GLfloat y, GLfloat z);
typedef FNGLWINDOWPOS3FPROC *PFNGLWINDOWPOS3FPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3FVPROC) (const GLfloat *v);
typedef FNGLWINDOWPOS3FVPROC *PFNGLWINDOWPOS3FVPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3IPROC) (GLint x, GLint y, GLint z);
typedef FNGLWINDOWPOS3IPROC *PFNGLWINDOWPOS3IPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3IVPROC) (const GLint *v);
typedef FNGLWINDOWPOS3IVPROC *PFNGLWINDOWPOS3IVPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3SPROC) (GLshort x, GLshort y, GLshort z);
typedef FNGLWINDOWPOS3SPROC *PFNGLWINDOWPOS3SPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3SVPROC) (const GLshort *v);
typedef FNGLWINDOWPOS3SVPROC *PFNGLWINDOWPOS3SVPROC;
#endif

#ifndef GL_ARB_multitexture
#define GL_ARB_multitexture 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glActiveTextureARB (GLenum);
GLAPI void GLAPIENTRY glClientActiveTextureARB (GLenum);
GLAPI void GLAPIENTRY glMultiTexCoord1dARB (GLenum, GLdouble);
GLAPI void GLAPIENTRY glMultiTexCoord1dvARB (GLenum, const GLdouble *);
GLAPI void GLAPIENTRY glMultiTexCoord1fARB (GLenum, GLfloat);
GLAPI void GLAPIENTRY glMultiTexCoord1fvARB (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glMultiTexCoord1iARB (GLenum, GLint);
GLAPI void GLAPIENTRY glMultiTexCoord1ivARB (GLenum, const GLint *);
GLAPI void GLAPIENTRY glMultiTexCoord1sARB (GLenum, GLshort);
GLAPI void GLAPIENTRY glMultiTexCoord1svARB (GLenum, const GLshort *);
GLAPI void GLAPIENTRY glMultiTexCoord2dARB (GLenum, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glMultiTexCoord2dvARB (GLenum, const GLdouble *);
GLAPI void GLAPIENTRY glMultiTexCoord2fARB (GLenum, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glMultiTexCoord2fvARB (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glMultiTexCoord2iARB (GLenum, GLint, GLint);
GLAPI void GLAPIENTRY glMultiTexCoord2ivARB (GLenum, const GLint *);
GLAPI void GLAPIENTRY glMultiTexCoord2sARB (GLenum, GLshort, GLshort);
GLAPI void GLAPIENTRY glMultiTexCoord2svARB (GLenum, const GLshort *);
GLAPI void GLAPIENTRY glMultiTexCoord3dARB (GLenum, GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glMultiTexCoord3dvARB (GLenum, const GLdouble *);
GLAPI void GLAPIENTRY glMultiTexCoord3fARB (GLenum, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glMultiTexCoord3fvARB (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glMultiTexCoord3iARB (GLenum, GLint, GLint, GLint);
GLAPI void GLAPIENTRY glMultiTexCoord3ivARB (GLenum, const GLint *);
GLAPI void GLAPIENTRY glMultiTexCoord3sARB (GLenum, GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glMultiTexCoord3svARB (GLenum, const GLshort *);
GLAPI void GLAPIENTRY glMultiTexCoord4dARB (GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glMultiTexCoord4dvARB (GLenum, const GLdouble *);
GLAPI void GLAPIENTRY glMultiTexCoord4fARB (GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glMultiTexCoord4fvARB (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glMultiTexCoord4iARB (GLenum, GLint, GLint, GLint, GLint);
GLAPI void GLAPIENTRY glMultiTexCoord4ivARB (GLenum, const GLint *);
GLAPI void GLAPIENTRY glMultiTexCoord4sARB (GLenum, GLshort, GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glMultiTexCoord4svARB (GLenum, const GLshort *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLACTIVETEXTUREARBPROC) (GLenum texture);
typedef FNGLACTIVETEXTUREARBPROC *PFNGLACTIVETEXTUREARBPROC;
typedef void (GLAPIENTRY FNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
typedef FNGLCLIENTACTIVETEXTUREARBPROC *PFNGLCLIENTACTIVETEXTUREARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD1DARBPROC) (GLenum target, GLdouble s);
typedef FNGLMULTITEXCOORD1DARBPROC *PFNGLMULTITEXCOORD1DARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD1DVARBPROC) (GLenum target, const GLdouble *v);
typedef FNGLMULTITEXCOORD1DVARBPROC *PFNGLMULTITEXCOORD1DVARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD1FARBPROC) (GLenum target, GLfloat s);
typedef FNGLMULTITEXCOORD1FARBPROC *PFNGLMULTITEXCOORD1FARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD1FVARBPROC) (GLenum target, const GLfloat *v);
typedef FNGLMULTITEXCOORD1FVARBPROC *PFNGLMULTITEXCOORD1FVARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD1IARBPROC) (GLenum target, GLint s);
typedef FNGLMULTITEXCOORD1IARBPROC *PFNGLMULTITEXCOORD1IARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD1IVARBPROC) (GLenum target, const GLint *v);
typedef FNGLMULTITEXCOORD1IVARBPROC *PFNGLMULTITEXCOORD1IVARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD1SARBPROC) (GLenum target, GLshort s);
typedef FNGLMULTITEXCOORD1SARBPROC *PFNGLMULTITEXCOORD1SARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD1SVARBPROC) (GLenum target, const GLshort *v);
typedef FNGLMULTITEXCOORD1SVARBPROC *PFNGLMULTITEXCOORD1SVARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD2DARBPROC) (GLenum target, GLdouble s, GLdouble t);
typedef FNGLMULTITEXCOORD2DARBPROC *PFNGLMULTITEXCOORD2DARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD2DVARBPROC) (GLenum target, const GLdouble *v);
typedef FNGLMULTITEXCOORD2DVARBPROC *PFNGLMULTITEXCOORD2DVARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef FNGLMULTITEXCOORD2FARBPROC *PFNGLMULTITEXCOORD2FARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD2FVARBPROC) (GLenum target, const GLfloat *v);
typedef FNGLMULTITEXCOORD2FVARBPROC *PFNGLMULTITEXCOORD2FVARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD2IARBPROC) (GLenum target, GLint s, GLint t);
typedef FNGLMULTITEXCOORD2IARBPROC *PFNGLMULTITEXCOORD2IARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD2IVARBPROC) (GLenum target, const GLint *v);
typedef FNGLMULTITEXCOORD2IVARBPROC *PFNGLMULTITEXCOORD2IVARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD2SARBPROC) (GLenum target, GLshort s, GLshort t);
typedef FNGLMULTITEXCOORD2SARBPROC *PFNGLMULTITEXCOORD2SARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD2SVARBPROC) (GLenum target, const GLshort *v);
typedef FNGLMULTITEXCOORD2SVARBPROC *PFNGLMULTITEXCOORD2SVARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD3DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef FNGLMULTITEXCOORD3DARBPROC *PFNGLMULTITEXCOORD3DARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD3DVARBPROC) (GLenum target, const GLdouble *v);
typedef FNGLMULTITEXCOORD3DVARBPROC *PFNGLMULTITEXCOORD3DVARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD3FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef FNGLMULTITEXCOORD3FARBPROC *PFNGLMULTITEXCOORD3FARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD3FVARBPROC) (GLenum target, const GLfloat *v);
typedef FNGLMULTITEXCOORD3FVARBPROC *PFNGLMULTITEXCOORD3FVARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD3IARBPROC) (GLenum target, GLint s, GLint t, GLint r);
typedef FNGLMULTITEXCOORD3IARBPROC *PFNGLMULTITEXCOORD3IARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD3IVARBPROC) (GLenum target, const GLint *v);
typedef FNGLMULTITEXCOORD3IVARBPROC *PFNGLMULTITEXCOORD3IVARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD3SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef FNGLMULTITEXCOORD3SARBPROC *PFNGLMULTITEXCOORD3SARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD3SVARBPROC) (GLenum target, const GLshort *v);
typedef FNGLMULTITEXCOORD3SVARBPROC *PFNGLMULTITEXCOORD3SVARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD4DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef FNGLMULTITEXCOORD4DARBPROC *PFNGLMULTITEXCOORD4DARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD4DVARBPROC) (GLenum target, const GLdouble *v);
typedef FNGLMULTITEXCOORD4DVARBPROC *PFNGLMULTITEXCOORD4DVARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD4FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef FNGLMULTITEXCOORD4FARBPROC *PFNGLMULTITEXCOORD4FARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD4FVARBPROC) (GLenum target, const GLfloat *v);
typedef FNGLMULTITEXCOORD4FVARBPROC *PFNGLMULTITEXCOORD4FVARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD4IARBPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef FNGLMULTITEXCOORD4IARBPROC *PFNGLMULTITEXCOORD4IARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD4IVARBPROC) (GLenum target, const GLint *v);
typedef FNGLMULTITEXCOORD4IVARBPROC *PFNGLMULTITEXCOORD4IVARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD4SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef FNGLMULTITEXCOORD4SARBPROC *PFNGLMULTITEXCOORD4SARBPROC;
typedef void (GLAPIENTRY FNGLMULTITEXCOORD4SVARBPROC) (GLenum target, const GLshort *v);
typedef FNGLMULTITEXCOORD4SVARBPROC *PFNGLMULTITEXCOORD4SVARBPROC;
#endif

#ifndef GL_ARB_transpose_matrix
#define GL_ARB_transpose_matrix 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glLoadTransposeMatrixfARB (const GLfloat *);
GLAPI void GLAPIENTRY glLoadTransposeMatrixdARB (const GLdouble *);
GLAPI void GLAPIENTRY glMultTransposeMatrixfARB (const GLfloat *);
GLAPI void GLAPIENTRY glMultTransposeMatrixdARB (const GLdouble *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLLOADTRANSPOSEMATRIXFARBPROC) (const GLfloat *m);
typedef FNGLLOADTRANSPOSEMATRIXFARBPROC *PFNGLLOADTRANSPOSEMATRIXFARBPROC;
typedef void (GLAPIENTRY FNGLLOADTRANSPOSEMATRIXDARBPROC) (const GLdouble *m);
typedef FNGLLOADTRANSPOSEMATRIXDARBPROC *PFNGLLOADTRANSPOSEMATRIXDARBPROC;
typedef void (GLAPIENTRY FNGLMULTTRANSPOSEMATRIXFARBPROC) (const GLfloat *m);
typedef FNGLMULTTRANSPOSEMATRIXFARBPROC *PFNGLMULTTRANSPOSEMATRIXFARBPROC;
typedef void (GLAPIENTRY FNGLMULTTRANSPOSEMATRIXDARBPROC) (const GLdouble *m);
typedef FNGLMULTTRANSPOSEMATRIXDARBPROC *PFNGLMULTTRANSPOSEMATRIXDARBPROC;
#endif

#ifndef GL_ARB_multisample
#define GL_ARB_multisample 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glSampleCoverageARB (GLclampf, GLboolean);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLSAMPLECOVERAGEARBPROC) (GLclampf value, GLboolean invert);
typedef FNGLSAMPLECOVERAGEARBPROC *PFNGLSAMPLECOVERAGEARBPROC;
#endif

#ifndef GL_ARB_texture_env_add
#define GL_ARB_texture_env_add 1
#endif

#ifndef GL_ARB_texture_cube_map
#define GL_ARB_texture_cube_map 1
#endif

#ifndef GL_ARB_texture_compression
#define GL_ARB_texture_compression 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glCompressedTexImage3DARB (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glCompressedTexImage2DARB (GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glCompressedTexImage1DARB (GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glCompressedTexSubImage3DARB (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glCompressedTexSubImage2DARB (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glCompressedTexSubImage1DARB (GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glGetCompressedTexImageARB (GLenum, GLint, void *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLCOMPRESSEDTEXIMAGE3DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
typedef FNGLCOMPRESSEDTEXIMAGE3DARBPROC *PFNGLCOMPRESSEDTEXIMAGE3DARBPROC;
typedef void (GLAPIENTRY FNGLCOMPRESSEDTEXIMAGE2DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
typedef FNGLCOMPRESSEDTEXIMAGE2DARBPROC *PFNGLCOMPRESSEDTEXIMAGE2DARBPROC;
typedef void (GLAPIENTRY FNGLCOMPRESSEDTEXIMAGE1DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
typedef FNGLCOMPRESSEDTEXIMAGE1DARBPROC *PFNGLCOMPRESSEDTEXIMAGE1DARBPROC;
typedef void (GLAPIENTRY FNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef FNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC *PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC;
typedef void (GLAPIENTRY FNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef FNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC *PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC;
typedef void (GLAPIENTRY FNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef FNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC *PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC;
typedef void (GLAPIENTRY FNGLGETCOMPRESSEDTEXIMAGEARBPROC) (GLenum target, GLint level, void *img);
typedef FNGLGETCOMPRESSEDTEXIMAGEARBPROC *PFNGLGETCOMPRESSEDTEXIMAGEARBPROC;
#endif

#ifndef GL_ARB_texture_border_clamp
#define GL_ARB_texture_border_clamp 1
#endif

#ifndef GL_ARB_point_parameters
#define GL_ARB_point_parameters 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glPointParameterfARB (GLenum, GLfloat);
GLAPI void GLAPIENTRY glPointParameterfvARB (GLenum, const GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLPOINTPARAMETERFARBPROC) (GLenum pname, GLfloat param);
typedef FNGLPOINTPARAMETERFARBPROC *PFNGLPOINTPARAMETERFARBPROC;
typedef void (GLAPIENTRY FNGLPOINTPARAMETERFVARBPROC) (GLenum pname, const GLfloat *params);
typedef FNGLPOINTPARAMETERFVARBPROC *PFNGLPOINTPARAMETERFVARBPROC;
#endif

#ifndef GL_ARB_vertex_blend
#define GL_ARB_vertex_blend 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glWeightbvARB (GLint, const GLbyte *);
GLAPI void GLAPIENTRY glWeightsvARB (GLint, const GLshort *);
GLAPI void GLAPIENTRY glWeightivARB (GLint, const GLint *);
GLAPI void GLAPIENTRY glWeightfvARB (GLint, const GLfloat *);
GLAPI void GLAPIENTRY glWeightdvARB (GLint, const GLdouble *);
GLAPI void GLAPIENTRY glWeightubvARB (GLint, const GLubyte *);
GLAPI void GLAPIENTRY glWeightusvARB (GLint, const GLushort *);
GLAPI void GLAPIENTRY glWeightuivARB (GLint, const GLuint *);
GLAPI void GLAPIENTRY glWeightPointerARB (GLint, GLenum, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glVertexBlendARB (GLint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLWEIGHTBVARBPROC) (GLint size, const GLbyte *weights);
typedef FNGLWEIGHTBVARBPROC *PFNGLWEIGHTBVARBPROC;
typedef void (GLAPIENTRY FNGLWEIGHTSVARBPROC) (GLint size, const GLshort *weights);
typedef FNGLWEIGHTSVARBPROC *PFNGLWEIGHTSVARBPROC;
typedef void (GLAPIENTRY FNGLWEIGHTIVARBPROC) (GLint size, const GLint *weights);
typedef FNGLWEIGHTIVARBPROC *PFNGLWEIGHTIVARBPROC;
typedef void (GLAPIENTRY FNGLWEIGHTFVARBPROC) (GLint size, const GLfloat *weights);
typedef FNGLWEIGHTFVARBPROC *PFNGLWEIGHTFVARBPROC;
typedef void (GLAPIENTRY FNGLWEIGHTDVARBPROC) (GLint size, const GLdouble *weights);
typedef FNGLWEIGHTDVARBPROC *PFNGLWEIGHTDVARBPROC;
typedef void (GLAPIENTRY FNGLWEIGHTUBVARBPROC) (GLint size, const GLubyte *weights);
typedef FNGLWEIGHTUBVARBPROC *PFNGLWEIGHTUBVARBPROC;
typedef void (GLAPIENTRY FNGLWEIGHTUSVARBPROC) (GLint size, const GLushort *weights);
typedef FNGLWEIGHTUSVARBPROC *PFNGLWEIGHTUSVARBPROC;
typedef void (GLAPIENTRY FNGLWEIGHTUIVARBPROC) (GLint size, const GLuint *weights);
typedef FNGLWEIGHTUIVARBPROC *PFNGLWEIGHTUIVARBPROC;
typedef void (GLAPIENTRY FNGLWEIGHTPOINTERARBPROC) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef FNGLWEIGHTPOINTERARBPROC *PFNGLWEIGHTPOINTERARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXBLENDARBPROC) (GLint count);
typedef FNGLVERTEXBLENDARBPROC *PFNGLVERTEXBLENDARBPROC;
#endif

#ifndef GL_ARB_matrix_palette
#define GL_ARB_matrix_palette 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glCurrentPaletteMatrixARB (GLint);
GLAPI void GLAPIENTRY glMatrixIndexubvARB (GLint, const GLubyte *);
GLAPI void GLAPIENTRY glMatrixIndexusvARB (GLint, const GLushort *);
GLAPI void GLAPIENTRY glMatrixIndexuivARB (GLint, const GLuint *);
GLAPI void GLAPIENTRY glMatrixIndexPointerARB (GLint, GLenum, GLsizei, const GLvoid *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLCURRENTPALETTEMATRIXARBPROC) (GLint index);
typedef FNGLCURRENTPALETTEMATRIXARBPROC *PFNGLCURRENTPALETTEMATRIXARBPROC;
typedef void (GLAPIENTRY FNGLMATRIXINDEXUBVARBPROC) (GLint size, const GLubyte *indices);
typedef FNGLMATRIXINDEXUBVARBPROC *PFNGLMATRIXINDEXUBVARBPROC;
typedef void (GLAPIENTRY FNGLMATRIXINDEXUSVARBPROC) (GLint size, const GLushort *indices);
typedef FNGLMATRIXINDEXUSVARBPROC *PFNGLMATRIXINDEXUSVARBPROC;
typedef void (GLAPIENTRY FNGLMATRIXINDEXUIVARBPROC) (GLint size, const GLuint *indices);
typedef FNGLMATRIXINDEXUIVARBPROC *PFNGLMATRIXINDEXUIVARBPROC;
typedef void (GLAPIENTRY FNGLMATRIXINDEXPOINTERARBPROC) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef FNGLMATRIXINDEXPOINTERARBPROC *PFNGLMATRIXINDEXPOINTERARBPROC;
#endif

#ifndef GL_ARB_texture_env_combine
#define GL_ARB_texture_env_combine 1
#endif

#ifndef GL_ARB_texture_env_crossbar
#define GL_ARB_texture_env_crossbar 1
#endif

#ifndef GL_ARB_texture_env_dot3
#define GL_ARB_texture_env_dot3 1
#endif

#ifndef GL_ARB_texture_mirror_repeat
#define GL_ARB_texture_mirror_repeat 1
#endif

#ifndef GL_ARB_depth_texture
#define GL_ARB_depth_texture 1
#endif

#ifndef GL_ARB_shadow
#define GL_ARB_shadow 1
#endif

#ifndef GL_ARB_shadow_ambient
#define GL_ARB_shadow_ambient 1
#endif

#ifndef GL_ARB_window_pos
#define GL_ARB_window_pos 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glWindowPos2dARB (GLdouble, GLdouble);
GLAPI void GLAPIENTRY glWindowPos2dvARB (const GLdouble *);
GLAPI void GLAPIENTRY glWindowPos2fARB (GLfloat, GLfloat);
GLAPI void GLAPIENTRY glWindowPos2fvARB (const GLfloat *);
GLAPI void GLAPIENTRY glWindowPos2iARB (GLint, GLint);
GLAPI void GLAPIENTRY glWindowPos2ivARB (const GLint *);
GLAPI void GLAPIENTRY glWindowPos2sARB (GLshort, GLshort);
GLAPI void GLAPIENTRY glWindowPos2svARB (const GLshort *);
GLAPI void GLAPIENTRY glWindowPos3dARB (GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glWindowPos3dvARB (const GLdouble *);
GLAPI void GLAPIENTRY glWindowPos3fARB (GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glWindowPos3fvARB (const GLfloat *);
GLAPI void GLAPIENTRY glWindowPos3iARB (GLint, GLint, GLint);
GLAPI void GLAPIENTRY glWindowPos3ivARB (const GLint *);
GLAPI void GLAPIENTRY glWindowPos3sARB (GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glWindowPos3svARB (const GLshort *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLWINDOWPOS2DARBPROC) (GLdouble x, GLdouble y);
typedef FNGLWINDOWPOS2DARBPROC *PFNGLWINDOWPOS2DARBPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2DVARBPROC) (const GLdouble *v);
typedef FNGLWINDOWPOS2DVARBPROC *PFNGLWINDOWPOS2DVARBPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2FARBPROC) (GLfloat x, GLfloat y);
typedef FNGLWINDOWPOS2FARBPROC *PFNGLWINDOWPOS2FARBPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2FVARBPROC) (const GLfloat *v);
typedef FNGLWINDOWPOS2FVARBPROC *PFNGLWINDOWPOS2FVARBPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2IARBPROC) (GLint x, GLint y);
typedef FNGLWINDOWPOS2IARBPROC *PFNGLWINDOWPOS2IARBPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2IVARBPROC) (const GLint *v);
typedef FNGLWINDOWPOS2IVARBPROC *PFNGLWINDOWPOS2IVARBPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2SARBPROC) (GLshort x, GLshort y);
typedef FNGLWINDOWPOS2SARBPROC *PFNGLWINDOWPOS2SARBPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2SVARBPROC) (const GLshort *v);
typedef FNGLWINDOWPOS2SVARBPROC *PFNGLWINDOWPOS2SVARBPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3DARBPROC) (GLdouble x, GLdouble y, GLdouble z);
typedef FNGLWINDOWPOS3DARBPROC *PFNGLWINDOWPOS3DARBPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3DVARBPROC) (const GLdouble *v);
typedef FNGLWINDOWPOS3DVARBPROC *PFNGLWINDOWPOS3DVARBPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3FARBPROC) (GLfloat x, GLfloat y, GLfloat z);
typedef FNGLWINDOWPOS3FARBPROC *PFNGLWINDOWPOS3FARBPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3FVARBPROC) (const GLfloat *v);
typedef FNGLWINDOWPOS3FVARBPROC *PFNGLWINDOWPOS3FVARBPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3IARBPROC) (GLint x, GLint y, GLint z);
typedef FNGLWINDOWPOS3IARBPROC *PFNGLWINDOWPOS3IARBPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3IVARBPROC) (const GLint *v);
typedef FNGLWINDOWPOS3IVARBPROC *PFNGLWINDOWPOS3IVARBPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3SARBPROC) (GLshort x, GLshort y, GLshort z);
typedef FNGLWINDOWPOS3SARBPROC *PFNGLWINDOWPOS3SARBPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3SVARBPROC) (const GLshort *v);
typedef FNGLWINDOWPOS3SVARBPROC *PFNGLWINDOWPOS3SVARBPROC;
#endif

#ifndef GL_ARB_vertex_program
#define GL_ARB_vertex_program 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glVertexAttrib1dARB (GLuint, GLdouble);
GLAPI void GLAPIENTRY glVertexAttrib1dvARB (GLuint, const GLdouble *);
GLAPI void GLAPIENTRY glVertexAttrib1fARB (GLuint, GLfloat);
GLAPI void GLAPIENTRY glVertexAttrib1fvARB (GLuint, const GLfloat *);
GLAPI void GLAPIENTRY glVertexAttrib1sARB (GLuint, GLshort);
GLAPI void GLAPIENTRY glVertexAttrib1svARB (GLuint, const GLshort *);
GLAPI void GLAPIENTRY glVertexAttrib2dARB (GLuint, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glVertexAttrib2dvARB (GLuint, const GLdouble *);
GLAPI void GLAPIENTRY glVertexAttrib2fARB (GLuint, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glVertexAttrib2fvARB (GLuint, const GLfloat *);
GLAPI void GLAPIENTRY glVertexAttrib2sARB (GLuint, GLshort, GLshort);
GLAPI void GLAPIENTRY glVertexAttrib2svARB (GLuint, const GLshort *);
GLAPI void GLAPIENTRY glVertexAttrib3dARB (GLuint, GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glVertexAttrib3dvARB (GLuint, const GLdouble *);
GLAPI void GLAPIENTRY glVertexAttrib3fARB (GLuint, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glVertexAttrib3fvARB (GLuint, const GLfloat *);
GLAPI void GLAPIENTRY glVertexAttrib3sARB (GLuint, GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glVertexAttrib3svARB (GLuint, const GLshort *);
GLAPI void GLAPIENTRY glVertexAttrib4NbvARB (GLuint, const GLbyte *);
GLAPI void GLAPIENTRY glVertexAttrib4NivARB (GLuint, const GLint *);
GLAPI void GLAPIENTRY glVertexAttrib4NsvARB (GLuint, const GLshort *);
GLAPI void GLAPIENTRY glVertexAttrib4NubARB (GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
GLAPI void GLAPIENTRY glVertexAttrib4NubvARB (GLuint, const GLubyte *);
GLAPI void GLAPIENTRY glVertexAttrib4NuivARB (GLuint, const GLuint *);
GLAPI void GLAPIENTRY glVertexAttrib4NusvARB (GLuint, const GLushort *);
GLAPI void GLAPIENTRY glVertexAttrib4bvARB (GLuint, const GLbyte *);
GLAPI void GLAPIENTRY glVertexAttrib4dARB (GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glVertexAttrib4dvARB (GLuint, const GLdouble *);
GLAPI void GLAPIENTRY glVertexAttrib4fARB (GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glVertexAttrib4fvARB (GLuint, const GLfloat *);
GLAPI void GLAPIENTRY glVertexAttrib4ivARB (GLuint, const GLint *);
GLAPI void GLAPIENTRY glVertexAttrib4sARB (GLuint, GLshort, GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glVertexAttrib4svARB (GLuint, const GLshort *);
GLAPI void GLAPIENTRY glVertexAttrib4ubvARB (GLuint, const GLubyte *);
GLAPI void GLAPIENTRY glVertexAttrib4uivARB (GLuint, const GLuint *);
GLAPI void GLAPIENTRY glVertexAttrib4usvARB (GLuint, const GLushort *);
GLAPI void GLAPIENTRY glVertexAttribPointerARB (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glEnableVertexAttribArrayARB (GLuint);
GLAPI void GLAPIENTRY glDisableVertexAttribArrayARB (GLuint);
GLAPI void GLAPIENTRY glProgramStringARB (GLenum, GLenum, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glBindProgramARB (GLenum, GLuint);
GLAPI void GLAPIENTRY glDeleteProgramsARB (GLsizei, const GLuint *);
GLAPI void GLAPIENTRY glGenProgramsARB (GLsizei, GLuint *);
GLAPI void GLAPIENTRY glProgramEnvParameter4dARB (GLenum, GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glProgramEnvParameter4dvARB (GLenum, GLuint, const GLdouble *);
GLAPI void GLAPIENTRY glProgramEnvParameter4fARB (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glProgramEnvParameter4fvARB (GLenum, GLuint, const GLfloat *);
GLAPI void GLAPIENTRY glProgramLocalParameter4dARB (GLenum, GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glProgramLocalParameter4dvARB (GLenum, GLuint, const GLdouble *);
GLAPI void GLAPIENTRY glProgramLocalParameter4fARB (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glProgramLocalParameter4fvARB (GLenum, GLuint, const GLfloat *);
GLAPI void GLAPIENTRY glGetProgramEnvParameterdvARB (GLenum, GLuint, GLdouble *);
GLAPI void GLAPIENTRY glGetProgramEnvParameterfvARB (GLenum, GLuint, GLfloat *);
GLAPI void GLAPIENTRY glGetProgramLocalParameterdvARB (GLenum, GLuint, GLdouble *);
GLAPI void GLAPIENTRY glGetProgramLocalParameterfvARB (GLenum, GLuint, GLfloat *);
GLAPI void GLAPIENTRY glGetProgramivARB (GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetProgramStringARB (GLenum, GLenum, GLvoid *);
GLAPI void GLAPIENTRY glGetVertexAttribdvARB (GLuint, GLenum, GLdouble *);
GLAPI void GLAPIENTRY glGetVertexAttribfvARB (GLuint, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetVertexAttribivARB (GLuint, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetVertexAttribPointervARB (GLuint, GLenum, GLvoid* *);
GLAPI GLboolean GLAPIENTRY glIsProgramARB (GLuint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLVERTEXATTRIB1DARBPROC) (GLuint index, GLdouble x);
typedef FNGLVERTEXATTRIB1DARBPROC *PFNGLVERTEXATTRIB1DARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB1DVARBPROC) (GLuint index, const GLdouble *v);
typedef FNGLVERTEXATTRIB1DVARBPROC *PFNGLVERTEXATTRIB1DVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB1FARBPROC) (GLuint index, GLfloat x);
typedef FNGLVERTEXATTRIB1FARBPROC *PFNGLVERTEXATTRIB1FARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB1FVARBPROC) (GLuint index, const GLfloat *v);
typedef FNGLVERTEXATTRIB1FVARBPROC *PFNGLVERTEXATTRIB1FVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB1SARBPROC) (GLuint index, GLshort x);
typedef FNGLVERTEXATTRIB1SARBPROC *PFNGLVERTEXATTRIB1SARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB1SVARBPROC) (GLuint index, const GLshort *v);
typedef FNGLVERTEXATTRIB1SVARBPROC *PFNGLVERTEXATTRIB1SVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB2DARBPROC) (GLuint index, GLdouble x, GLdouble y);
typedef FNGLVERTEXATTRIB2DARBPROC *PFNGLVERTEXATTRIB2DARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB2DVARBPROC) (GLuint index, const GLdouble *v);
typedef FNGLVERTEXATTRIB2DVARBPROC *PFNGLVERTEXATTRIB2DVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB2FARBPROC) (GLuint index, GLfloat x, GLfloat y);
typedef FNGLVERTEXATTRIB2FARBPROC *PFNGLVERTEXATTRIB2FARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB2FVARBPROC) (GLuint index, const GLfloat *v);
typedef FNGLVERTEXATTRIB2FVARBPROC *PFNGLVERTEXATTRIB2FVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB2SARBPROC) (GLuint index, GLshort x, GLshort y);
typedef FNGLVERTEXATTRIB2SARBPROC *PFNGLVERTEXATTRIB2SARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB2SVARBPROC) (GLuint index, const GLshort *v);
typedef FNGLVERTEXATTRIB2SVARBPROC *PFNGLVERTEXATTRIB2SVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB3DARBPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef FNGLVERTEXATTRIB3DARBPROC *PFNGLVERTEXATTRIB3DARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB3DVARBPROC) (GLuint index, const GLdouble *v);
typedef FNGLVERTEXATTRIB3DVARBPROC *PFNGLVERTEXATTRIB3DVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB3FARBPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLVERTEXATTRIB3FARBPROC *PFNGLVERTEXATTRIB3FARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB3FVARBPROC) (GLuint index, const GLfloat *v);
typedef FNGLVERTEXATTRIB3FVARBPROC *PFNGLVERTEXATTRIB3FVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB3SARBPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef FNGLVERTEXATTRIB3SARBPROC *PFNGLVERTEXATTRIB3SARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB3SVARBPROC) (GLuint index, const GLshort *v);
typedef FNGLVERTEXATTRIB3SVARBPROC *PFNGLVERTEXATTRIB3SVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4NBVARBPROC) (GLuint index, const GLbyte *v);
typedef FNGLVERTEXATTRIB4NBVARBPROC *PFNGLVERTEXATTRIB4NBVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4NIVARBPROC) (GLuint index, const GLint *v);
typedef FNGLVERTEXATTRIB4NIVARBPROC *PFNGLVERTEXATTRIB4NIVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4NSVARBPROC) (GLuint index, const GLshort *v);
typedef FNGLVERTEXATTRIB4NSVARBPROC *PFNGLVERTEXATTRIB4NSVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4NUBARBPROC) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef FNGLVERTEXATTRIB4NUBARBPROC *PFNGLVERTEXATTRIB4NUBARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4NUBVARBPROC) (GLuint index, const GLubyte *v);
typedef FNGLVERTEXATTRIB4NUBVARBPROC *PFNGLVERTEXATTRIB4NUBVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4NUIVARBPROC) (GLuint index, const GLuint *v);
typedef FNGLVERTEXATTRIB4NUIVARBPROC *PFNGLVERTEXATTRIB4NUIVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4NUSVARBPROC) (GLuint index, const GLushort *v);
typedef FNGLVERTEXATTRIB4NUSVARBPROC *PFNGLVERTEXATTRIB4NUSVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4BVARBPROC) (GLuint index, const GLbyte *v);
typedef FNGLVERTEXATTRIB4BVARBPROC *PFNGLVERTEXATTRIB4BVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4DARBPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef FNGLVERTEXATTRIB4DARBPROC *PFNGLVERTEXATTRIB4DARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4DVARBPROC) (GLuint index, const GLdouble *v);
typedef FNGLVERTEXATTRIB4DVARBPROC *PFNGLVERTEXATTRIB4DVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4FARBPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef FNGLVERTEXATTRIB4FARBPROC *PFNGLVERTEXATTRIB4FARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4FVARBPROC) (GLuint index, const GLfloat *v);
typedef FNGLVERTEXATTRIB4FVARBPROC *PFNGLVERTEXATTRIB4FVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4IVARBPROC) (GLuint index, const GLint *v);
typedef FNGLVERTEXATTRIB4IVARBPROC *PFNGLVERTEXATTRIB4IVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4SARBPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef FNGLVERTEXATTRIB4SARBPROC *PFNGLVERTEXATTRIB4SARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4SVARBPROC) (GLuint index, const GLshort *v);
typedef FNGLVERTEXATTRIB4SVARBPROC *PFNGLVERTEXATTRIB4SVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4UBVARBPROC) (GLuint index, const GLubyte *v);
typedef FNGLVERTEXATTRIB4UBVARBPROC *PFNGLVERTEXATTRIB4UBVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4UIVARBPROC) (GLuint index, const GLuint *v);
typedef FNGLVERTEXATTRIB4UIVARBPROC *PFNGLVERTEXATTRIB4UIVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4USVARBPROC) (GLuint index, const GLushort *v);
typedef FNGLVERTEXATTRIB4USVARBPROC *PFNGLVERTEXATTRIB4USVARBPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIBPOINTERARBPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef FNGLVERTEXATTRIBPOINTERARBPROC *PFNGLVERTEXATTRIBPOINTERARBPROC;
typedef void (GLAPIENTRY FNGLENABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
typedef FNGLENABLEVERTEXATTRIBARRAYARBPROC *PFNGLENABLEVERTEXATTRIBARRAYARBPROC;
typedef void (GLAPIENTRY FNGLDISABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
typedef FNGLDISABLEVERTEXATTRIBARRAYARBPROC *PFNGLDISABLEVERTEXATTRIBARRAYARBPROC;
typedef void (GLAPIENTRY FNGLPROGRAMSTRINGARBPROC) (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
typedef FNGLPROGRAMSTRINGARBPROC *PFNGLPROGRAMSTRINGARBPROC;
typedef void (GLAPIENTRY FNGLBINDPROGRAMARBPROC) (GLenum target, GLuint program);
typedef FNGLBINDPROGRAMARBPROC *PFNGLBINDPROGRAMARBPROC;
typedef void (GLAPIENTRY FNGLDELETEPROGRAMSARBPROC) (GLsizei n, const GLuint *programs);
typedef FNGLDELETEPROGRAMSARBPROC *PFNGLDELETEPROGRAMSARBPROC;
typedef void (GLAPIENTRY FNGLGENPROGRAMSARBPROC) (GLsizei n, GLuint *programs);
typedef FNGLGENPROGRAMSARBPROC *PFNGLGENPROGRAMSARBPROC;
typedef void (GLAPIENTRY FNGLPROGRAMENVPARAMETER4DARBPROC) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef FNGLPROGRAMENVPARAMETER4DARBPROC *PFNGLPROGRAMENVPARAMETER4DARBPROC;
typedef void (GLAPIENTRY FNGLPROGRAMENVPARAMETER4DVARBPROC) (GLenum target, GLuint index, const GLdouble *params);
typedef FNGLPROGRAMENVPARAMETER4DVARBPROC *PFNGLPROGRAMENVPARAMETER4DVARBPROC;
typedef void (GLAPIENTRY FNGLPROGRAMENVPARAMETER4FARBPROC) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef FNGLPROGRAMENVPARAMETER4FARBPROC *PFNGLPROGRAMENVPARAMETER4FARBPROC;
typedef void (GLAPIENTRY FNGLPROGRAMENVPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
typedef FNGLPROGRAMENVPARAMETER4FVARBPROC *PFNGLPROGRAMENVPARAMETER4FVARBPROC;
typedef void (GLAPIENTRY FNGLPROGRAMLOCALPARAMETER4DARBPROC) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef FNGLPROGRAMLOCALPARAMETER4DARBPROC *PFNGLPROGRAMLOCALPARAMETER4DARBPROC;
typedef void (GLAPIENTRY FNGLPROGRAMLOCALPARAMETER4DVARBPROC) (GLenum target, GLuint index, const GLdouble *params);
typedef FNGLPROGRAMLOCALPARAMETER4DVARBPROC *PFNGLPROGRAMLOCALPARAMETER4DVARBPROC;
typedef void (GLAPIENTRY FNGLPROGRAMLOCALPARAMETER4FARBPROC) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef FNGLPROGRAMLOCALPARAMETER4FARBPROC *PFNGLPROGRAMLOCALPARAMETER4FARBPROC;
typedef void (GLAPIENTRY FNGLPROGRAMLOCALPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
typedef FNGLPROGRAMLOCALPARAMETER4FVARBPROC *PFNGLPROGRAMLOCALPARAMETER4FVARBPROC;
typedef void (GLAPIENTRY FNGLGETPROGRAMENVPARAMETERDVARBPROC) (GLenum target, GLuint index, GLdouble *params);
typedef FNGLGETPROGRAMENVPARAMETERDVARBPROC *PFNGLGETPROGRAMENVPARAMETERDVARBPROC;
typedef void (GLAPIENTRY FNGLGETPROGRAMENVPARAMETERFVARBPROC) (GLenum target, GLuint index, GLfloat *params);
typedef FNGLGETPROGRAMENVPARAMETERFVARBPROC *PFNGLGETPROGRAMENVPARAMETERFVARBPROC;
typedef void (GLAPIENTRY FNGLGETPROGRAMLOCALPARAMETERDVARBPROC) (GLenum target, GLuint index, GLdouble *params);
typedef FNGLGETPROGRAMLOCALPARAMETERDVARBPROC *PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC;
typedef void (GLAPIENTRY FNGLGETPROGRAMLOCALPARAMETERFVARBPROC) (GLenum target, GLuint index, GLfloat *params);
typedef FNGLGETPROGRAMLOCALPARAMETERFVARBPROC *PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC;
typedef void (GLAPIENTRY FNGLGETPROGRAMIVARBPROC) (GLenum target, GLenum pname, GLint *params);
typedef FNGLGETPROGRAMIVARBPROC *PFNGLGETPROGRAMIVARBPROC;
typedef void (GLAPIENTRY FNGLGETPROGRAMSTRINGARBPROC) (GLenum target, GLenum pname, GLvoid *string);
typedef FNGLGETPROGRAMSTRINGARBPROC *PFNGLGETPROGRAMSTRINGARBPROC;
typedef void (GLAPIENTRY FNGLGETVERTEXATTRIBDVARBPROC) (GLuint index, GLenum pname, GLdouble *params);
typedef FNGLGETVERTEXATTRIBDVARBPROC *PFNGLGETVERTEXATTRIBDVARBPROC;
typedef void (GLAPIENTRY FNGLGETVERTEXATTRIBFVARBPROC) (GLuint index, GLenum pname, GLfloat *params);
typedef FNGLGETVERTEXATTRIBFVARBPROC *PFNGLGETVERTEXATTRIBFVARBPROC;
typedef void (GLAPIENTRY FNGLGETVERTEXATTRIBIVARBPROC) (GLuint index, GLenum pname, GLint *params);
typedef FNGLGETVERTEXATTRIBIVARBPROC *PFNGLGETVERTEXATTRIBIVARBPROC;
typedef void (GLAPIENTRY FNGLGETVERTEXATTRIBPOINTERVARBPROC) (GLuint index, GLenum pname, GLvoid* *pointer);
typedef FNGLGETVERTEXATTRIBPOINTERVARBPROC *PFNGLGETVERTEXATTRIBPOINTERVARBPROC;
typedef GLboolean (GLAPIENTRY FNGLISPROGRAMARBPROC) (GLuint program);
typedef FNGLISPROGRAMARBPROC *PFNGLISPROGRAMARBPROC;
#endif

#ifndef GL_ARB_fragment_program
#define GL_ARB_fragment_program 1
/* All ARB_fragment_program entry points are shared with ARB_vertex_program. */
#endif

#ifndef GL_ARB_vertex_buffer_object
#define GL_ARB_vertex_buffer_object 1
/* GL types for handling large vertex buffer objects */
/* Only used by this extension for now; later needs to be moved earlier in glext.h */
#include <stddef.h>
typedef ptrdiff_t GLintptrARB;
typedef ptrdiff_t GLsizeiptrARB;
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glBindBufferARB (GLenum, GLuint);
GLAPI void GLAPIENTRY glDeleteBuffersARB (GLsizei, const GLuint *);
GLAPI void GLAPIENTRY glGenBuffersARB (GLsizei, GLuint *);
GLAPI GLboolean GLAPIENTRY glIsBufferARB (GLuint);
GLAPI void GLAPIENTRY glBufferDataARB (GLenum, GLsizeiptrARB, const GLvoid *, GLenum);
GLAPI void GLAPIENTRY glBufferSubDataARB (GLenum, GLintptrARB, GLsizeiptrARB, const GLvoid *);
GLAPI void GLAPIENTRY glGetBufferSubDataARB (GLenum, GLintptrARB, GLsizeiptrARB, GLvoid *);
GLAPI GLvoid* GLAPIENTRY glMapBufferARB (GLenum, GLenum);
GLAPI GLboolean GLAPIENTRY glUnmapBufferARB (GLenum);
GLAPI void GLAPIENTRY glGetBufferParameterivARB (GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetBufferPointervARB (GLenum, GLenum, GLvoid* *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef GLboolean (APIENTRY * PFNGLISBUFFERARBPROC) (GLuint buffer);
typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
typedef void (APIENTRY * PFNGLBUFFERSUBDATAARBPROC) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data);
typedef void (APIENTRY * PFNGLGETBUFFERSUBDATAARBPROC) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data);
typedef GLvoid* (APIENTRY * PFNGLMAPBUFFERARBPROC) (GLenum target, GLenum access);
typedef GLboolean (APIENTRY * PFNGLUNMAPBUFFERARBPROC) (GLenum target);
typedef void (APIENTRY * PFNGLGETBUFFERPARAMETERIVARBPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETBUFFERPOINTERVARBPROC) (GLenum target, GLenum pname, GLvoid* *params);
#endif

#ifndef GL_EXT_abgr
#define GL_EXT_abgr 1
#endif

#ifndef GL_EXT_blend_color
#define GL_EXT_blend_color 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glBlendColorEXT (GLclampf, GLclampf, GLclampf, GLclampf);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLBLENDCOLOREXTPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef FNGLBLENDCOLOREXTPROC *PFNGLBLENDCOLOREXTPROC;
#endif

#ifndef GL_EXT_polygon_offset
#define GL_EXT_polygon_offset 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glPolygonOffsetEXT (GLfloat, GLfloat);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLPOLYGONOFFSETEXTPROC) (GLfloat factor, GLfloat bias);
typedef FNGLPOLYGONOFFSETEXTPROC *PFNGLPOLYGONOFFSETEXTPROC;
#endif

#ifndef GL_EXT_texture
#define GL_EXT_texture 1
#endif

#ifndef GL_EXT_texture3D
#define GL_EXT_texture3D 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glTexImage3DEXT (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glTexSubImage3DEXT (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLTEXIMAGE3DEXTPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef FNGLTEXIMAGE3DEXTPROC *PFNGLTEXIMAGE3DEXTPROC;
typedef void (GLAPIENTRY FNGLTEXSUBIMAGE3DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef FNGLTEXSUBIMAGE3DEXTPROC *PFNGLTEXSUBIMAGE3DEXTPROC;
#endif

#ifndef GL_SGIS_texture_filter4
#define GL_SGIS_texture_filter4 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glGetTexFilterFuncSGIS (GLenum, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glTexFilterFuncSGIS (GLenum, GLenum, GLsizei, const GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLGETTEXFILTERFUNCSGISPROC) (GLenum target, GLenum filter, GLfloat *weights);
typedef FNGLGETTEXFILTERFUNCSGISPROC *PFNGLGETTEXFILTERFUNCSGISPROC;
typedef void (GLAPIENTRY FNGLTEXFILTERFUNCSGISPROC) (GLenum target, GLenum filter, GLsizei n, const GLfloat *weights);
typedef FNGLTEXFILTERFUNCSGISPROC *PFNGLTEXFILTERFUNCSGISPROC;
#endif

#ifndef GL_EXT_subtexture
#define GL_EXT_subtexture 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glTexSubImage1DEXT (GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glTexSubImage2DEXT (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLTEXSUBIMAGE1DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
typedef FNGLTEXSUBIMAGE1DEXTPROC *PFNGLTEXSUBIMAGE1DEXTPROC;
typedef void (GLAPIENTRY FNGLTEXSUBIMAGE2DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef FNGLTEXSUBIMAGE2DEXTPROC *PFNGLTEXSUBIMAGE2DEXTPROC;
#endif

#ifndef GL_EXT_copy_texture
#define GL_EXT_copy_texture 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glCopyTexImage1DEXT (GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
GLAPI void GLAPIENTRY glCopyTexImage2DEXT (GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
GLAPI void GLAPIENTRY glCopyTexSubImage1DEXT (GLenum, GLint, GLint, GLint, GLint, GLsizei);
GLAPI void GLAPIENTRY glCopyTexSubImage2DEXT (GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
GLAPI void GLAPIENTRY glCopyTexSubImage3DEXT (GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLCOPYTEXIMAGE1DEXTPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef FNGLCOPYTEXIMAGE1DEXTPROC *PFNGLCOPYTEXIMAGE1DEXTPROC;
typedef void (GLAPIENTRY FNGLCOPYTEXIMAGE2DEXTPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef FNGLCOPYTEXIMAGE2DEXTPROC *PFNGLCOPYTEXIMAGE2DEXTPROC;
typedef void (GLAPIENTRY FNGLCOPYTEXSUBIMAGE1DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef FNGLCOPYTEXSUBIMAGE1DEXTPROC *PFNGLCOPYTEXSUBIMAGE1DEXTPROC;
typedef void (GLAPIENTRY FNGLCOPYTEXSUBIMAGE2DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef FNGLCOPYTEXSUBIMAGE2DEXTPROC *PFNGLCOPYTEXSUBIMAGE2DEXTPROC;
typedef void (GLAPIENTRY FNGLCOPYTEXSUBIMAGE3DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef FNGLCOPYTEXSUBIMAGE3DEXTPROC *PFNGLCOPYTEXSUBIMAGE3DEXTPROC;
#endif

#ifndef GL_EXT_histogram
#define GL_EXT_histogram 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glGetHistogramEXT (GLenum, GLboolean, GLenum, GLenum, GLvoid *);
GLAPI void GLAPIENTRY glGetHistogramParameterfvEXT (GLenum, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetHistogramParameterivEXT (GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetMinmaxEXT (GLenum, GLboolean, GLenum, GLenum, GLvoid *);
GLAPI void GLAPIENTRY glGetMinmaxParameterfvEXT (GLenum, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetMinmaxParameterivEXT (GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glHistogramEXT (GLenum, GLsizei, GLenum, GLboolean);
GLAPI void GLAPIENTRY glMinmaxEXT (GLenum, GLenum, GLboolean);
GLAPI void GLAPIENTRY glResetHistogramEXT (GLenum);
GLAPI void GLAPIENTRY glResetMinmaxEXT (GLenum);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLGETHISTOGRAMEXTPROC) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef FNGLGETHISTOGRAMEXTPROC *PFNGLGETHISTOGRAMEXTPROC;
typedef void (GLAPIENTRY FNGLGETHISTOGRAMPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef FNGLGETHISTOGRAMPARAMETERFVEXTPROC *PFNGLGETHISTOGRAMPARAMETERFVEXTPROC;
typedef void (GLAPIENTRY FNGLGETHISTOGRAMPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef FNGLGETHISTOGRAMPARAMETERIVEXTPROC *PFNGLGETHISTOGRAMPARAMETERIVEXTPROC;
typedef void (GLAPIENTRY FNGLGETMINMAXEXTPROC) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef FNGLGETMINMAXEXTPROC *PFNGLGETMINMAXEXTPROC;
typedef void (GLAPIENTRY FNGLGETMINMAXPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef FNGLGETMINMAXPARAMETERFVEXTPROC *PFNGLGETMINMAXPARAMETERFVEXTPROC;
typedef void (GLAPIENTRY FNGLGETMINMAXPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef FNGLGETMINMAXPARAMETERIVEXTPROC *PFNGLGETMINMAXPARAMETERIVEXTPROC;
typedef void (GLAPIENTRY FNGLHISTOGRAMEXTPROC) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
typedef FNGLHISTOGRAMEXTPROC *PFNGLHISTOGRAMEXTPROC;
typedef void (GLAPIENTRY FNGLMINMAXEXTPROC) (GLenum target, GLenum internalformat, GLboolean sink);
typedef FNGLMINMAXEXTPROC *PFNGLMINMAXEXTPROC;
typedef void (GLAPIENTRY FNGLRESETHISTOGRAMEXTPROC) (GLenum target);
typedef FNGLRESETHISTOGRAMEXTPROC *PFNGLRESETHISTOGRAMEXTPROC;
typedef void (GLAPIENTRY FNGLRESETMINMAXEXTPROC) (GLenum target);
typedef FNGLRESETMINMAXEXTPROC *PFNGLRESETMINMAXEXTPROC;
#endif

#ifndef GL_EXT_convolution
#define GL_EXT_convolution 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glConvolutionFilter1DEXT (GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glConvolutionFilter2DEXT (GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glConvolutionParameterfEXT (GLenum, GLenum, GLfloat);
GLAPI void GLAPIENTRY glConvolutionParameterfvEXT (GLenum, GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glConvolutionParameteriEXT (GLenum, GLenum, GLint);
GLAPI void GLAPIENTRY glConvolutionParameterivEXT (GLenum, GLenum, const GLint *);
GLAPI void GLAPIENTRY glCopyConvolutionFilter1DEXT (GLenum, GLenum, GLint, GLint, GLsizei);
GLAPI void GLAPIENTRY glCopyConvolutionFilter2DEXT (GLenum, GLenum, GLint, GLint, GLsizei, GLsizei);
GLAPI void GLAPIENTRY glGetConvolutionFilterEXT (GLenum, GLenum, GLenum, GLvoid *);
GLAPI void GLAPIENTRY glGetConvolutionParameterfvEXT (GLenum, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetConvolutionParameterivEXT (GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetSeparableFilterEXT (GLenum, GLenum, GLenum, GLvoid *, GLvoid *, GLvoid *);
GLAPI void GLAPIENTRY glSeparableFilter2DEXT (GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *, const GLvoid *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLCONVOLUTIONFILTER1DEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
typedef FNGLCONVOLUTIONFILTER1DEXTPROC *PFNGLCONVOLUTIONFILTER1DEXTPROC;
typedef void (GLAPIENTRY FNGLCONVOLUTIONFILTER2DEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
typedef FNGLCONVOLUTIONFILTER2DEXTPROC *PFNGLCONVOLUTIONFILTER2DEXTPROC;
typedef void (GLAPIENTRY FNGLCONVOLUTIONPARAMETERFEXTPROC) (GLenum target, GLenum pname, GLfloat params);
typedef FNGLCONVOLUTIONPARAMETERFEXTPROC *PFNGLCONVOLUTIONPARAMETERFEXTPROC;
typedef void (GLAPIENTRY FNGLCONVOLUTIONPARAMETERFVEXTPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef FNGLCONVOLUTIONPARAMETERFVEXTPROC *PFNGLCONVOLUTIONPARAMETERFVEXTPROC;
typedef void (GLAPIENTRY FNGLCONVOLUTIONPARAMETERIEXTPROC) (GLenum target, GLenum pname, GLint params);
typedef FNGLCONVOLUTIONPARAMETERIEXTPROC *PFNGLCONVOLUTIONPARAMETERIEXTPROC;
typedef void (GLAPIENTRY FNGLCONVOLUTIONPARAMETERIVEXTPROC) (GLenum target, GLenum pname, const GLint *params);
typedef FNGLCONVOLUTIONPARAMETERIVEXTPROC *PFNGLCONVOLUTIONPARAMETERIVEXTPROC;
typedef void (GLAPIENTRY FNGLCOPYCONVOLUTIONFILTER1DEXTPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef FNGLCOPYCONVOLUTIONFILTER1DEXTPROC *PFNGLCOPYCONVOLUTIONFILTER1DEXTPROC;
typedef void (GLAPIENTRY FNGLCOPYCONVOLUTIONFILTER2DEXTPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
typedef FNGLCOPYCONVOLUTIONFILTER2DEXTPROC *PFNGLCOPYCONVOLUTIONFILTER2DEXTPROC;
typedef void (GLAPIENTRY FNGLGETCONVOLUTIONFILTEREXTPROC) (GLenum target, GLenum format, GLenum type, GLvoid *image);
typedef FNGLGETCONVOLUTIONFILTEREXTPROC *PFNGLGETCONVOLUTIONFILTEREXTPROC;
typedef void (GLAPIENTRY FNGLGETCONVOLUTIONPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef FNGLGETCONVOLUTIONPARAMETERFVEXTPROC *PFNGLGETCONVOLUTIONPARAMETERFVEXTPROC;
typedef void (GLAPIENTRY FNGLGETCONVOLUTIONPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef FNGLGETCONVOLUTIONPARAMETERIVEXTPROC *PFNGLGETCONVOLUTIONPARAMETERIVEXTPROC;
typedef void (GLAPIENTRY FNGLGETSEPARABLEFILTEREXTPROC) (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
typedef FNGLGETSEPARABLEFILTEREXTPROC *PFNGLGETSEPARABLEFILTEREXTPROC;
typedef void (GLAPIENTRY FNGLSEPARABLEFILTER2DEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
typedef FNGLSEPARABLEFILTER2DEXTPROC *PFNGLSEPARABLEFILTER2DEXTPROC;
#endif

#ifndef GL_EXT_color_matrix
#define GL_EXT_color_matrix 1
#endif

#ifndef GL_SGI_color_table
#define GL_SGI_color_table 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glColorTableSGI (GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glColorTableParameterfvSGI (GLenum, GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glColorTableParameterivSGI (GLenum, GLenum, const GLint *);
GLAPI void GLAPIENTRY glCopyColorTableSGI (GLenum, GLenum, GLint, GLint, GLsizei);
GLAPI void GLAPIENTRY glGetColorTableSGI (GLenum, GLenum, GLenum, GLvoid *);
GLAPI void GLAPIENTRY glGetColorTableParameterfvSGI (GLenum, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetColorTableParameterivSGI (GLenum, GLenum, GLint *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLCOLORTABLESGIPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef FNGLCOLORTABLESGIPROC *PFNGLCOLORTABLESGIPROC;
typedef void (GLAPIENTRY FNGLCOLORTABLEPARAMETERFVSGIPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef FNGLCOLORTABLEPARAMETERFVSGIPROC *PFNGLCOLORTABLEPARAMETERFVSGIPROC;
typedef void (GLAPIENTRY FNGLCOLORTABLEPARAMETERIVSGIPROC) (GLenum target, GLenum pname, const GLint *params);
typedef FNGLCOLORTABLEPARAMETERIVSGIPROC *PFNGLCOLORTABLEPARAMETERIVSGIPROC;
typedef void (GLAPIENTRY FNGLCOPYCOLORTABLESGIPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef FNGLCOPYCOLORTABLESGIPROC *PFNGLCOPYCOLORTABLESGIPROC;
typedef void (GLAPIENTRY FNGLGETCOLORTABLESGIPROC) (GLenum target, GLenum format, GLenum type, GLvoid *table);
typedef FNGLGETCOLORTABLESGIPROC *PFNGLGETCOLORTABLESGIPROC;
typedef void (GLAPIENTRY FNGLGETCOLORTABLEPARAMETERFVSGIPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef FNGLGETCOLORTABLEPARAMETERFVSGIPROC *PFNGLGETCOLORTABLEPARAMETERFVSGIPROC;
typedef void (GLAPIENTRY FNGLGETCOLORTABLEPARAMETERIVSGIPROC) (GLenum target, GLenum pname, GLint *params);
typedef FNGLGETCOLORTABLEPARAMETERIVSGIPROC *PFNGLGETCOLORTABLEPARAMETERIVSGIPROC;
#endif

#ifndef GL_SGIX_pixel_texture
#define GL_SGIX_pixel_texture 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glPixelTexGenSGIX (GLenum);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLPIXELTEXGENSGIXPROC) (GLenum mode);
typedef FNGLPIXELTEXGENSGIXPROC *PFNGLPIXELTEXGENSGIXPROC;
#endif

#ifndef GL_SGIS_pixel_texture
#define GL_SGIS_pixel_texture 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glPixelTexGenParameteriSGIS (GLenum, GLint);
GLAPI void GLAPIENTRY glPixelTexGenParameterivSGIS (GLenum, const GLint *);
GLAPI void GLAPIENTRY glPixelTexGenParameterfSGIS (GLenum, GLfloat);
GLAPI void GLAPIENTRY glPixelTexGenParameterfvSGIS (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glGetPixelTexGenParameterivSGIS (GLenum, GLint *);
GLAPI void GLAPIENTRY glGetPixelTexGenParameterfvSGIS (GLenum, GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLPIXELTEXGENPARAMETERISGISPROC) (GLenum pname, GLint param);
typedef FNGLPIXELTEXGENPARAMETERISGISPROC *PFNGLPIXELTEXGENPARAMETERISGISPROC;
typedef void (GLAPIENTRY FNGLPIXELTEXGENPARAMETERIVSGISPROC) (GLenum pname, const GLint *params);
typedef FNGLPIXELTEXGENPARAMETERIVSGISPROC *PFNGLPIXELTEXGENPARAMETERIVSGISPROC;
typedef void (GLAPIENTRY FNGLPIXELTEXGENPARAMETERFSGISPROC) (GLenum pname, GLfloat param);
typedef FNGLPIXELTEXGENPARAMETERFSGISPROC *PFNGLPIXELTEXGENPARAMETERFSGISPROC;
typedef void (GLAPIENTRY FNGLPIXELTEXGENPARAMETERFVSGISPROC) (GLenum pname, const GLfloat *params);
typedef FNGLPIXELTEXGENPARAMETERFVSGISPROC *PFNGLPIXELTEXGENPARAMETERFVSGISPROC;
typedef void (GLAPIENTRY FNGLGETPIXELTEXGENPARAMETERIVSGISPROC) (GLenum pname, GLint *params);
typedef FNGLGETPIXELTEXGENPARAMETERIVSGISPROC *PFNGLGETPIXELTEXGENPARAMETERIVSGISPROC;
typedef void (GLAPIENTRY FNGLGETPIXELTEXGENPARAMETERFVSGISPROC) (GLenum pname, GLfloat *params);
typedef FNGLGETPIXELTEXGENPARAMETERFVSGISPROC *PFNGLGETPIXELTEXGENPARAMETERFVSGISPROC;
#endif

#ifndef GL_SGIS_texture4D
#define GL_SGIS_texture4D 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glTexImage4DSGIS (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glTexSubImage4DSGIS (GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLTEXIMAGE4DSGISPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef FNGLTEXIMAGE4DSGISPROC *PFNGLTEXIMAGE4DSGISPROC;
typedef void (GLAPIENTRY FNGLTEXSUBIMAGE4DSGISPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint woffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLenum format, GLenum type, const GLvoid *pixels);
typedef FNGLTEXSUBIMAGE4DSGISPROC *PFNGLTEXSUBIMAGE4DSGISPROC;
#endif

#ifndef GL_SGI_texture_color_table
#define GL_SGI_texture_color_table 1
#endif

#ifndef GL_EXT_cmyka
#define GL_EXT_cmyka 1
#endif

#ifndef GL_EXT_texture_object
#define GL_EXT_texture_object 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI GLboolean GLAPIENTRY glAreTexturesResidentEXT (GLsizei, const GLuint *, GLboolean *);
GLAPI void GLAPIENTRY glBindTextureEXT (GLenum, GLuint);
GLAPI void GLAPIENTRY glDeleteTexturesEXT (GLsizei, const GLuint *);
GLAPI void GLAPIENTRY glGenTexturesEXT (GLsizei, GLuint *);
GLAPI GLboolean GLAPIENTRY glIsTextureEXT (GLuint);
GLAPI void GLAPIENTRY glPrioritizeTexturesEXT (GLsizei, const GLuint *, const GLclampf *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef GLboolean (GLAPIENTRY FNGLARETEXTURESRESIDENTEXTPROC) (GLsizei n, const GLuint *textures, GLboolean *residences);
typedef FNGLARETEXTURESRESIDENTEXTPROC *PFNGLARETEXTURESRESIDENTEXTPROC;
typedef void (GLAPIENTRY FNGLBINDTEXTUREEXTPROC) (GLenum target, GLuint texture);
typedef FNGLBINDTEXTUREEXTPROC *PFNGLBINDTEXTUREEXTPROC;
typedef void (GLAPIENTRY FNGLDELETETEXTURESEXTPROC) (GLsizei n, const GLuint *textures);
typedef FNGLDELETETEXTURESEXTPROC *PFNGLDELETETEXTURESEXTPROC;
typedef void (GLAPIENTRY FNGLGENTEXTURESEXTPROC) (GLsizei n, GLuint *textures);
typedef FNGLGENTEXTURESEXTPROC *PFNGLGENTEXTURESEXTPROC;
typedef GLboolean (GLAPIENTRY FNGLISTEXTUREEXTPROC) (GLuint texture);
typedef FNGLISTEXTUREEXTPROC *PFNGLISTEXTUREEXTPROC;
typedef void (GLAPIENTRY FNGLPRIORITIZETEXTURESEXTPROC) (GLsizei n, const GLuint *textures, const GLclampf *priorities);
typedef FNGLPRIORITIZETEXTURESEXTPROC *PFNGLPRIORITIZETEXTURESEXTPROC;
#endif

#ifndef GL_SGIS_detail_texture
#define GL_SGIS_detail_texture 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glDetailTexFuncSGIS (GLenum, GLsizei, const GLfloat *);
GLAPI void GLAPIENTRY glGetDetailTexFuncSGIS (GLenum, GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLDETAILTEXFUNCSGISPROC) (GLenum target, GLsizei n, const GLfloat *points);
typedef FNGLDETAILTEXFUNCSGISPROC *PFNGLDETAILTEXFUNCSGISPROC;
typedef void (GLAPIENTRY FNGLGETDETAILTEXFUNCSGISPROC) (GLenum target, GLfloat *points);
typedef FNGLGETDETAILTEXFUNCSGISPROC *PFNGLGETDETAILTEXFUNCSGISPROC;
#endif

#ifndef GL_SGIS_sharpen_texture
#define GL_SGIS_sharpen_texture 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glSharpenTexFuncSGIS (GLenum, GLsizei, const GLfloat *);
GLAPI void GLAPIENTRY glGetSharpenTexFuncSGIS (GLenum, GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLSHARPENTEXFUNCSGISPROC) (GLenum target, GLsizei n, const GLfloat *points);
typedef FNGLSHARPENTEXFUNCSGISPROC *PFNGLSHARPENTEXFUNCSGISPROC;
typedef void (GLAPIENTRY FNGLGETSHARPENTEXFUNCSGISPROC) (GLenum target, GLfloat *points);
typedef FNGLGETSHARPENTEXFUNCSGISPROC *PFNGLGETSHARPENTEXFUNCSGISPROC;
#endif

#ifndef GL_EXT_packed_pixels
#define GL_EXT_packed_pixels 1
#endif

#ifndef GL_SGIS_texture_lod
#define GL_SGIS_texture_lod 1
#endif

#ifndef GL_SGIS_multisample
#define GL_SGIS_multisample 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glSampleMaskSGIS (GLclampf, GLboolean);
GLAPI void GLAPIENTRY glSamplePatternSGIS (GLenum);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLSAMPLEMASKSGISPROC) (GLclampf value, GLboolean invert);
typedef FNGLSAMPLEMASKSGISPROC *PFNGLSAMPLEMASKSGISPROC;
typedef void (GLAPIENTRY FNGLSAMPLEPATTERNSGISPROC) (GLenum pattern);
typedef FNGLSAMPLEPATTERNSGISPROC *PFNGLSAMPLEPATTERNSGISPROC;
#endif

#ifndef GL_EXT_rescale_normal
#define GL_EXT_rescale_normal 1
#endif

#ifndef GL_EXT_vertex_array
#define GL_EXT_vertex_array 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glArrayElementEXT (GLint);
GLAPI void GLAPIENTRY glColorPointerEXT (GLint, GLenum, GLsizei, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glDrawArraysEXT (GLenum, GLint, GLsizei);
GLAPI void GLAPIENTRY glEdgeFlagPointerEXT (GLsizei, GLsizei, const GLboolean *);
GLAPI void GLAPIENTRY glGetPointervEXT (GLenum, GLvoid* *);
GLAPI void GLAPIENTRY glIndexPointerEXT (GLenum, GLsizei, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glNormalPointerEXT (GLenum, GLsizei, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glTexCoordPointerEXT (GLint, GLenum, GLsizei, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glVertexPointerEXT (GLint, GLenum, GLsizei, GLsizei, const GLvoid *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLARRAYELEMENTEXTPROC) (GLint i);
typedef FNGLARRAYELEMENTEXTPROC *PFNGLARRAYELEMENTEXTPROC;
typedef void (GLAPIENTRY FNGLCOLORPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef FNGLCOLORPOINTEREXTPROC *PFNGLCOLORPOINTEREXTPROC;
typedef void (GLAPIENTRY FNGLDRAWARRAYSEXTPROC) (GLenum mode, GLint first, GLsizei count);
typedef FNGLDRAWARRAYSEXTPROC *PFNGLDRAWARRAYSEXTPROC;
typedef void (GLAPIENTRY FNGLEDGEFLAGPOINTEREXTPROC) (GLsizei stride, GLsizei count, const GLboolean *pointer);
typedef FNGLEDGEFLAGPOINTEREXTPROC *PFNGLEDGEFLAGPOINTEREXTPROC;
typedef void (GLAPIENTRY FNGLGETPOINTERVEXTPROC) (GLenum pname, GLvoid* *params);
typedef FNGLGETPOINTERVEXTPROC *PFNGLGETPOINTERVEXTPROC;
typedef void (GLAPIENTRY FNGLINDEXPOINTEREXTPROC) (GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef FNGLINDEXPOINTEREXTPROC *PFNGLINDEXPOINTEREXTPROC;
typedef void (GLAPIENTRY FNGLNORMALPOINTEREXTPROC) (GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef FNGLNORMALPOINTEREXTPROC *PFNGLNORMALPOINTEREXTPROC;
typedef void (GLAPIENTRY FNGLTEXCOORDPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef FNGLTEXCOORDPOINTEREXTPROC *PFNGLTEXCOORDPOINTEREXTPROC;
typedef void (GLAPIENTRY FNGLVERTEXPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef FNGLVERTEXPOINTEREXTPROC *PFNGLVERTEXPOINTEREXTPROC;
#endif

#ifndef GL_EXT_misc_attribute
#define GL_EXT_misc_attribute 1
#endif

#ifndef GL_SGIS_generate_mipmap
#define GL_SGIS_generate_mipmap 1
#endif

#ifndef GL_SGIX_clipmap
#define GL_SGIX_clipmap 1
#endif

#ifndef GL_SGIX_shadow
#define GL_SGIX_shadow 1
#endif

#ifndef GL_SGIS_texture_edge_clamp
#define GL_SGIS_texture_edge_clamp 1
#endif

#ifndef GL_SGIS_texture_border_clamp
#define GL_SGIS_texture_border_clamp 1
#endif

#ifndef GL_EXT_blend_minmax
#define GL_EXT_blend_minmax 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glBlendEquationEXT (GLenum);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLBLENDEQUATIONEXTPROC) (GLenum mode);
typedef FNGLBLENDEQUATIONEXTPROC *PFNGLBLENDEQUATIONEXTPROC;
#endif

#ifndef GL_EXT_blend_subtract
#define GL_EXT_blend_subtract 1
#endif

#ifndef GL_EXT_blend_logic_op
#define GL_EXT_blend_logic_op 1
#endif

#ifndef GL_SGIX_interlace
#define GL_SGIX_interlace 1
#endif

#ifndef GL_SGIX_pixel_tiles
#define GL_SGIX_pixel_tiles 1
#endif

#ifndef GL_SGIX_texture_select
#define GL_SGIX_texture_select 1
#endif

#ifndef GL_SGIX_sprite
#define GL_SGIX_sprite 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glSpriteParameterfSGIX (GLenum, GLfloat);
GLAPI void GLAPIENTRY glSpriteParameterfvSGIX (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glSpriteParameteriSGIX (GLenum, GLint);
GLAPI void GLAPIENTRY glSpriteParameterivSGIX (GLenum, const GLint *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLSPRITEPARAMETERFSGIXPROC) (GLenum pname, GLfloat param);
typedef FNGLSPRITEPARAMETERFSGIXPROC *PFNGLSPRITEPARAMETERFSGIXPROC;
typedef void (GLAPIENTRY FNGLSPRITEPARAMETERFVSGIXPROC) (GLenum pname, const GLfloat *params);
typedef FNGLSPRITEPARAMETERFVSGIXPROC *PFNGLSPRITEPARAMETERFVSGIXPROC;
typedef void (GLAPIENTRY FNGLSPRITEPARAMETERISGIXPROC) (GLenum pname, GLint param);
typedef FNGLSPRITEPARAMETERISGIXPROC *PFNGLSPRITEPARAMETERISGIXPROC;
typedef void (GLAPIENTRY FNGLSPRITEPARAMETERIVSGIXPROC) (GLenum pname, const GLint *params);
typedef FNGLSPRITEPARAMETERIVSGIXPROC *PFNGLSPRITEPARAMETERIVSGIXPROC;
#endif

#ifndef GL_SGIX_texture_multi_buffer
#define GL_SGIX_texture_multi_buffer 1
#endif

#ifndef GL_EXT_point_parameters
#define GL_EXT_point_parameters 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glPointParameterfEXT (GLenum, GLfloat);
GLAPI void GLAPIENTRY glPointParameterfvEXT (GLenum, const GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLPOINTPARAMETERFEXTPROC) (GLenum pname, GLfloat param);
typedef FNGLPOINTPARAMETERFEXTPROC *PFNGLPOINTPARAMETERFEXTPROC;
typedef void (GLAPIENTRY FNGLPOINTPARAMETERFVEXTPROC) (GLenum pname, const GLfloat *params);
typedef FNGLPOINTPARAMETERFVEXTPROC *PFNGLPOINTPARAMETERFVEXTPROC;
#endif

#ifndef GL_SGIS_point_parameters
#define GL_SGIS_point_parameters 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glPointParameterfSGIS (GLenum, GLfloat);
GLAPI void GLAPIENTRY glPointParameterfvSGIS (GLenum, const GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLPOINTPARAMETERFSGISPROC) (GLenum pname, GLfloat param);
typedef FNGLPOINTPARAMETERFSGISPROC *PFNGLPOINTPARAMETERFSGISPROC;
typedef void (GLAPIENTRY FNGLPOINTPARAMETERFVSGISPROC) (GLenum pname, const GLfloat *params);
typedef FNGLPOINTPARAMETERFVSGISPROC *PFNGLPOINTPARAMETERFVSGISPROC;
#endif

#ifndef GL_SGIX_instruments
#define GL_SGIX_instruments 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI GLint GLAPIENTRY glGetInstrumentsSGIX (void);
GLAPI void GLAPIENTRY glInstrumentsBufferSGIX (GLsizei, GLint *);
GLAPI GLint GLAPIENTRY glPollInstrumentsSGIX (GLint *);
GLAPI void GLAPIENTRY glReadInstrumentsSGIX (GLint);
GLAPI void GLAPIENTRY glStartInstrumentsSGIX (void);
GLAPI void GLAPIENTRY glStopInstrumentsSGIX (GLint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef GLint (GLAPIENTRY FNGLGETINSTRUMENTSSGIXPROC) (void);
typedef FNGLGETINSTRUMENTSSGIXPROC *PFNGLGETINSTRUMENTSSGIXPROC;
typedef void (GLAPIENTRY FNGLINSTRUMENTSBUFFERSGIXPROC) (GLsizei size, GLint *buffer);
typedef FNGLINSTRUMENTSBUFFERSGIXPROC *PFNGLINSTRUMENTSBUFFERSGIXPROC;
typedef GLint (GLAPIENTRY FNGLPOLLINSTRUMENTSSGIXPROC) (GLint *marker_p);
typedef FNGLPOLLINSTRUMENTSSGIXPROC *PFNGLPOLLINSTRUMENTSSGIXPROC;
typedef void (GLAPIENTRY FNGLREADINSTRUMENTSSGIXPROC) (GLint marker);
typedef FNGLREADINSTRUMENTSSGIXPROC *PFNGLREADINSTRUMENTSSGIXPROC;
typedef void (GLAPIENTRY FNGLSTARTINSTRUMENTSSGIXPROC) (void);
typedef FNGLSTARTINSTRUMENTSSGIXPROC *PFNGLSTARTINSTRUMENTSSGIXPROC;
typedef void (GLAPIENTRY FNGLSTOPINSTRUMENTSSGIXPROC) (GLint marker);
typedef FNGLSTOPINSTRUMENTSSGIXPROC *PFNGLSTOPINSTRUMENTSSGIXPROC;
#endif

#ifndef GL_SGIX_texture_scale_bias
#define GL_SGIX_texture_scale_bias 1
#endif

#ifndef GL_SGIX_framezoom
#define GL_SGIX_framezoom 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glFrameZoomSGIX (GLint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLFRAMEZOOMSGIXPROC) (GLint factor);
typedef FNGLFRAMEZOOMSGIXPROC *PFNGLFRAMEZOOMSGIXPROC;
#endif

#ifndef GL_SGIX_tag_sample_buffer
#define GL_SGIX_tag_sample_buffer 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glTagSampleBufferSGIX (void);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLTAGSAMPLEBUFFERSGIXPROC) (void);
typedef FNGLTAGSAMPLEBUFFERSGIXPROC *PFNGLTAGSAMPLEBUFFERSGIXPROC;
#endif

#ifndef GL_SGIX_polynomial_ffd
#define GL_SGIX_polynomial_ffd 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glDeformationMap3dSGIX (GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, const GLdouble *);
GLAPI void GLAPIENTRY glDeformationMap3fSGIX (GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, const GLfloat *);
GLAPI void GLAPIENTRY glDeformSGIX (GLbitfield);
GLAPI void GLAPIENTRY glLoadIdentityDeformationMapSGIX (GLbitfield);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLDEFORMATIONMAP3DSGIXPROC) (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, GLdouble w1, GLdouble w2, GLint wstride, GLint worder, const GLdouble *points);
typedef FNGLDEFORMATIONMAP3DSGIXPROC *PFNGLDEFORMATIONMAP3DSGIXPROC;
typedef void (GLAPIENTRY FNGLDEFORMATIONMAP3FSGIXPROC) (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, GLfloat w1, GLfloat w2, GLint wstride, GLint worder, const GLfloat *points);
typedef FNGLDEFORMATIONMAP3FSGIXPROC *PFNGLDEFORMATIONMAP3FSGIXPROC;
typedef void (GLAPIENTRY FNGLDEFORMSGIXPROC) (GLbitfield mask);
typedef FNGLDEFORMSGIXPROC *PFNGLDEFORMSGIXPROC;
typedef void (GLAPIENTRY FNGLLOADIDENTITYDEFORMATIONMAPSGIXPROC) (GLbitfield mask);
typedef FNGLLOADIDENTITYDEFORMATIONMAPSGIXPROC *PFNGLLOADIDENTITYDEFORMATIONMAPSGIXPROC;
#endif

#ifndef GL_SGIX_reference_plane
#define GL_SGIX_reference_plane 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glReferencePlaneSGIX (const GLdouble *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLREFERENCEPLANESGIXPROC) (const GLdouble *equation);
typedef FNGLREFERENCEPLANESGIXPROC *PFNGLREFERENCEPLANESGIXPROC;
#endif

#ifndef GL_SGIX_flush_raster
#define GL_SGIX_flush_raster 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glFlushRasterSGIX (void);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLFLUSHRASTERSGIXPROC) (void);
typedef FNGLFLUSHRASTERSGIXPROC *PFNGLFLUSHRASTERSGIXPROC;
#endif

#ifndef GL_SGIX_depth_texture
#define GL_SGIX_depth_texture 1
#endif

#ifndef GL_SGIS_fog_function
#define GL_SGIS_fog_function 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glFogFuncSGIS (GLsizei, const GLfloat *);
GLAPI void GLAPIENTRY glGetFogFuncSGIS (GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLFOGFUNCSGISPROC) (GLsizei n, const GLfloat *points);
typedef FNGLFOGFUNCSGISPROC *PFNGLFOGFUNCSGISPROC;
typedef void (GLAPIENTRY FNGLGETFOGFUNCSGISPROC) (GLfloat *points);
typedef FNGLGETFOGFUNCSGISPROC *PFNGLGETFOGFUNCSGISPROC;
#endif

#ifndef GL_SGIX_fog_offset
#define GL_SGIX_fog_offset 1
#endif

#ifndef GL_HP_image_transform
#define GL_HP_image_transform 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glImageTransformParameteriHP (GLenum, GLenum, GLint);
GLAPI void GLAPIENTRY glImageTransformParameterfHP (GLenum, GLenum, GLfloat);
GLAPI void GLAPIENTRY glImageTransformParameterivHP (GLenum, GLenum, const GLint *);
GLAPI void GLAPIENTRY glImageTransformParameterfvHP (GLenum, GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glGetImageTransformParameterivHP (GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetImageTransformParameterfvHP (GLenum, GLenum, GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLIMAGETRANSFORMPARAMETERIHPPROC) (GLenum target, GLenum pname, GLint param);
typedef FNGLIMAGETRANSFORMPARAMETERIHPPROC *PFNGLIMAGETRANSFORMPARAMETERIHPPROC;
typedef void (GLAPIENTRY FNGLIMAGETRANSFORMPARAMETERFHPPROC) (GLenum target, GLenum pname, GLfloat param);
typedef FNGLIMAGETRANSFORMPARAMETERFHPPROC *PFNGLIMAGETRANSFORMPARAMETERFHPPROC;
typedef void (GLAPIENTRY FNGLIMAGETRANSFORMPARAMETERIVHPPROC) (GLenum target, GLenum pname, const GLint *params);
typedef FNGLIMAGETRANSFORMPARAMETERIVHPPROC *PFNGLIMAGETRANSFORMPARAMETERIVHPPROC;
typedef void (GLAPIENTRY FNGLIMAGETRANSFORMPARAMETERFVHPPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef FNGLIMAGETRANSFORMPARAMETERFVHPPROC *PFNGLIMAGETRANSFORMPARAMETERFVHPPROC;
typedef void (GLAPIENTRY FNGLGETIMAGETRANSFORMPARAMETERIVHPPROC) (GLenum target, GLenum pname, GLint *params);
typedef FNGLGETIMAGETRANSFORMPARAMETERIVHPPROC *PFNGLGETIMAGETRANSFORMPARAMETERIVHPPROC;
typedef void (GLAPIENTRY FNGLGETIMAGETRANSFORMPARAMETERFVHPPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef FNGLGETIMAGETRANSFORMPARAMETERFVHPPROC *PFNGLGETIMAGETRANSFORMPARAMETERFVHPPROC;
#endif

#ifndef GL_HP_convolution_border_modes
#define GL_HP_convolution_border_modes 1
#endif

#ifndef GL_SGIX_texture_add_env
#define GL_SGIX_texture_add_env 1
#endif

#ifndef GL_EXT_color_subtable
#define GL_EXT_color_subtable 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glColorSubTableEXT (GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glCopyColorSubTableEXT (GLenum, GLsizei, GLint, GLint, GLsizei);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLCOLORSUBTABLEEXTPROC) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
typedef FNGLCOLORSUBTABLEEXTPROC *PFNGLCOLORSUBTABLEEXTPROC;
typedef void (GLAPIENTRY FNGLCOPYCOLORSUBTABLEEXTPROC) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
typedef FNGLCOPYCOLORSUBTABLEEXTPROC *PFNGLCOPYCOLORSUBTABLEEXTPROC;
#endif

#ifndef GL_PGI_vertex_hints
#define GL_PGI_vertex_hints 1
#endif

#ifndef GL_PGI_misc_hints
#define GL_PGI_misc_hints 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glHintPGI (GLenum, GLint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLHINTPGIPROC) (GLenum target, GLint mode);
typedef FNGLHINTPGIPROC *PFNGLHINTPGIPROC;
#endif

#ifndef GL_EXT_paletted_texture
#define GL_EXT_paletted_texture 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glColorTableEXT (GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glGetColorTableEXT (GLenum, GLenum, GLenum, GLvoid *);
GLAPI void GLAPIENTRY glGetColorTableParameterivEXT (GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetColorTableParameterfvEXT (GLenum, GLenum, GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLCOLORTABLEEXTPROC) (GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef FNGLCOLORTABLEEXTPROC *PFNGLCOLORTABLEEXTPROC;
typedef void (GLAPIENTRY FNGLGETCOLORTABLEEXTPROC) (GLenum target, GLenum format, GLenum type, GLvoid *data);
typedef FNGLGETCOLORTABLEEXTPROC *PFNGLGETCOLORTABLEEXTPROC;
typedef void (GLAPIENTRY FNGLGETCOLORTABLEPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef FNGLGETCOLORTABLEPARAMETERIVEXTPROC *PFNGLGETCOLORTABLEPARAMETERIVEXTPROC;
typedef void (GLAPIENTRY FNGLGETCOLORTABLEPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef FNGLGETCOLORTABLEPARAMETERFVEXTPROC *PFNGLGETCOLORTABLEPARAMETERFVEXTPROC;
#endif

#ifndef GL_EXT_clip_volume_hint
#define GL_EXT_clip_volume_hint 1
#endif

#ifndef GL_SGIX_list_priority
#define GL_SGIX_list_priority 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glGetListParameterfvSGIX (GLuint, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetListParameterivSGIX (GLuint, GLenum, GLint *);
GLAPI void GLAPIENTRY glListParameterfSGIX (GLuint, GLenum, GLfloat);
GLAPI void GLAPIENTRY glListParameterfvSGIX (GLuint, GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glListParameteriSGIX (GLuint, GLenum, GLint);
GLAPI void GLAPIENTRY glListParameterivSGIX (GLuint, GLenum, const GLint *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLGETLISTPARAMETERFVSGIXPROC) (GLuint list, GLenum pname, GLfloat *params);
typedef FNGLGETLISTPARAMETERFVSGIXPROC *PFNGLGETLISTPARAMETERFVSGIXPROC;
typedef void (GLAPIENTRY FNGLGETLISTPARAMETERIVSGIXPROC) (GLuint list, GLenum pname, GLint *params);
typedef FNGLGETLISTPARAMETERIVSGIXPROC *PFNGLGETLISTPARAMETERIVSGIXPROC;
typedef void (GLAPIENTRY FNGLLISTPARAMETERFSGIXPROC) (GLuint list, GLenum pname, GLfloat param);
typedef FNGLLISTPARAMETERFSGIXPROC *PFNGLLISTPARAMETERFSGIXPROC;
typedef void (GLAPIENTRY FNGLLISTPARAMETERFVSGIXPROC) (GLuint list, GLenum pname, const GLfloat *params);
typedef FNGLLISTPARAMETERFVSGIXPROC *PFNGLLISTPARAMETERFVSGIXPROC;
typedef void (GLAPIENTRY FNGLLISTPARAMETERISGIXPROC) (GLuint list, GLenum pname, GLint param);
typedef FNGLLISTPARAMETERISGIXPROC *PFNGLLISTPARAMETERISGIXPROC;
typedef void (GLAPIENTRY FNGLLISTPARAMETERIVSGIXPROC) (GLuint list, GLenum pname, const GLint *params);
typedef FNGLLISTPARAMETERIVSGIXPROC *PFNGLLISTPARAMETERIVSGIXPROC;
#endif

#ifndef GL_SGIX_ir_instrument1
#define GL_SGIX_ir_instrument1 1
#endif

#ifndef GL_SGIX_calligraphic_fragment
#define GL_SGIX_calligraphic_fragment 1
#endif

#ifndef GL_SGIX_texture_lod_bias
#define GL_SGIX_texture_lod_bias 1
#endif

#ifndef GL_SGIX_shadow_ambient
#define GL_SGIX_shadow_ambient 1
#endif

#ifndef GL_EXT_index_texture
#define GL_EXT_index_texture 1
#endif

#ifndef GL_EXT_index_material
#define GL_EXT_index_material 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glIndexMaterialEXT (GLenum, GLenum);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLINDEXMATERIALEXTPROC) (GLenum face, GLenum mode);
typedef FNGLINDEXMATERIALEXTPROC *PFNGLINDEXMATERIALEXTPROC;
#endif

#ifndef GL_EXT_index_func
#define GL_EXT_index_func 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glIndexFuncEXT (GLenum, GLclampf);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLINDEXFUNCEXTPROC) (GLenum func, GLclampf ref);
typedef FNGLINDEXFUNCEXTPROC *PFNGLINDEXFUNCEXTPROC;
#endif

#ifndef GL_EXT_index_array_formats
#define GL_EXT_index_array_formats 1
#endif

#ifndef GL_EXT_compiled_vertex_array
#define GL_EXT_compiled_vertex_array 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glLockArraysEXT (GLint, GLsizei);
GLAPI void GLAPIENTRY glUnlockArraysEXT (void);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLLOCKARRAYSEXTPROC) (GLint first, GLsizei count);
typedef FNGLLOCKARRAYSEXTPROC *PFNGLLOCKARRAYSEXTPROC;
typedef void (GLAPIENTRY FNGLUNLOCKARRAYSEXTPROC) (void);
typedef FNGLUNLOCKARRAYSEXTPROC *PFNGLUNLOCKARRAYSEXTPROC;
#endif

#ifndef GL_EXT_cull_vertex
#define GL_EXT_cull_vertex 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glCullParameterdvEXT (GLenum, GLdouble *);
GLAPI void GLAPIENTRY glCullParameterfvEXT (GLenum, GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLCULLPARAMETERDVEXTPROC) (GLenum pname, GLdouble *params);
typedef FNGLCULLPARAMETERDVEXTPROC *PFNGLCULLPARAMETERDVEXTPROC;
typedef void (GLAPIENTRY FNGLCULLPARAMETERFVEXTPROC) (GLenum pname, GLfloat *params);
typedef FNGLCULLPARAMETERFVEXTPROC *PFNGLCULLPARAMETERFVEXTPROC;
#endif

#ifndef GL_SGIX_ycrcb
#define GL_SGIX_ycrcb 1
#endif

#ifndef GL_SGIX_fragment_lighting
#define GL_SGIX_fragment_lighting 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glFragmentColorMaterialSGIX (GLenum, GLenum);
GLAPI void GLAPIENTRY glFragmentLightfSGIX (GLenum, GLenum, GLfloat);
GLAPI void GLAPIENTRY glFragmentLightfvSGIX (GLenum, GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glFragmentLightiSGIX (GLenum, GLenum, GLint);
GLAPI void GLAPIENTRY glFragmentLightivSGIX (GLenum, GLenum, const GLint *);
GLAPI void GLAPIENTRY glFragmentLightModelfSGIX (GLenum, GLfloat);
GLAPI void GLAPIENTRY glFragmentLightModelfvSGIX (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glFragmentLightModeliSGIX (GLenum, GLint);
GLAPI void GLAPIENTRY glFragmentLightModelivSGIX (GLenum, const GLint *);
GLAPI void GLAPIENTRY glFragmentMaterialfSGIX (GLenum, GLenum, GLfloat);
GLAPI void GLAPIENTRY glFragmentMaterialfvSGIX (GLenum, GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glFragmentMaterialiSGIX (GLenum, GLenum, GLint);
GLAPI void GLAPIENTRY glFragmentMaterialivSGIX (GLenum, GLenum, const GLint *);
GLAPI void GLAPIENTRY glGetFragmentLightfvSGIX (GLenum, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetFragmentLightivSGIX (GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetFragmentMaterialfvSGIX (GLenum, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetFragmentMaterialivSGIX (GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glLightEnviSGIX (GLenum, GLint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLFRAGMENTCOLORMATERIALSGIXPROC) (GLenum face, GLenum mode);
typedef FNGLFRAGMENTCOLORMATERIALSGIXPROC *PFNGLFRAGMENTCOLORMATERIALSGIXPROC;
typedef void (GLAPIENTRY FNGLFRAGMENTLIGHTFSGIXPROC) (GLenum light, GLenum pname, GLfloat param);
typedef FNGLFRAGMENTLIGHTFSGIXPROC *PFNGLFRAGMENTLIGHTFSGIXPROC;
typedef void (GLAPIENTRY FNGLFRAGMENTLIGHTFVSGIXPROC) (GLenum light, GLenum pname, const GLfloat *params);
typedef FNGLFRAGMENTLIGHTFVSGIXPROC *PFNGLFRAGMENTLIGHTFVSGIXPROC;
typedef void (GLAPIENTRY FNGLFRAGMENTLIGHTISGIXPROC) (GLenum light, GLenum pname, GLint param);
typedef FNGLFRAGMENTLIGHTISGIXPROC *PFNGLFRAGMENTLIGHTISGIXPROC;
typedef void (GLAPIENTRY FNGLFRAGMENTLIGHTIVSGIXPROC) (GLenum light, GLenum pname, const GLint *params);
typedef FNGLFRAGMENTLIGHTIVSGIXPROC *PFNGLFRAGMENTLIGHTIVSGIXPROC;
typedef void (GLAPIENTRY FNGLFRAGMENTLIGHTMODELFSGIXPROC) (GLenum pname, GLfloat param);
typedef FNGLFRAGMENTLIGHTMODELFSGIXPROC *PFNGLFRAGMENTLIGHTMODELFSGIXPROC;
typedef void (GLAPIENTRY FNGLFRAGMENTLIGHTMODELFVSGIXPROC) (GLenum pname, const GLfloat *params);
typedef FNGLFRAGMENTLIGHTMODELFVSGIXPROC *PFNGLFRAGMENTLIGHTMODELFVSGIXPROC;
typedef void (GLAPIENTRY FNGLFRAGMENTLIGHTMODELISGIXPROC) (GLenum pname, GLint param);
typedef FNGLFRAGMENTLIGHTMODELISGIXPROC *PFNGLFRAGMENTLIGHTMODELISGIXPROC;
typedef void (GLAPIENTRY FNGLFRAGMENTLIGHTMODELIVSGIXPROC) (GLenum pname, const GLint *params);
typedef FNGLFRAGMENTLIGHTMODELIVSGIXPROC *PFNGLFRAGMENTLIGHTMODELIVSGIXPROC;
typedef void (GLAPIENTRY FNGLFRAGMENTMATERIALFSGIXPROC) (GLenum face, GLenum pname, GLfloat param);
typedef FNGLFRAGMENTMATERIALFSGIXPROC *PFNGLFRAGMENTMATERIALFSGIXPROC;
typedef void (GLAPIENTRY FNGLFRAGMENTMATERIALFVSGIXPROC) (GLenum face, GLenum pname, const GLfloat *params);
typedef FNGLFRAGMENTMATERIALFVSGIXPROC *PFNGLFRAGMENTMATERIALFVSGIXPROC;
typedef void (GLAPIENTRY FNGLFRAGMENTMATERIALISGIXPROC) (GLenum face, GLenum pname, GLint param);
typedef FNGLFRAGMENTMATERIALISGIXPROC *PFNGLFRAGMENTMATERIALISGIXPROC;
typedef void (GLAPIENTRY FNGLFRAGMENTMATERIALIVSGIXPROC) (GLenum face, GLenum pname, const GLint *params);
typedef FNGLFRAGMENTMATERIALIVSGIXPROC *PFNGLFRAGMENTMATERIALIVSGIXPROC;
typedef void (GLAPIENTRY FNGLGETFRAGMENTLIGHTFVSGIXPROC) (GLenum light, GLenum pname, GLfloat *params);
typedef FNGLGETFRAGMENTLIGHTFVSGIXPROC *PFNGLGETFRAGMENTLIGHTFVSGIXPROC;
typedef void (GLAPIENTRY FNGLGETFRAGMENTLIGHTIVSGIXPROC) (GLenum light, GLenum pname, GLint *params);
typedef FNGLGETFRAGMENTLIGHTIVSGIXPROC *PFNGLGETFRAGMENTLIGHTIVSGIXPROC;
typedef void (GLAPIENTRY FNGLGETFRAGMENTMATERIALFVSGIXPROC) (GLenum face, GLenum pname, GLfloat *params);
typedef FNGLGETFRAGMENTMATERIALFVSGIXPROC *PFNGLGETFRAGMENTMATERIALFVSGIXPROC;
typedef void (GLAPIENTRY FNGLGETFRAGMENTMATERIALIVSGIXPROC) (GLenum face, GLenum pname, GLint *params);
typedef FNGLGETFRAGMENTMATERIALIVSGIXPROC *PFNGLGETFRAGMENTMATERIALIVSGIXPROC;
typedef void (GLAPIENTRY FNGLLIGHTENVISGIXPROC) (GLenum pname, GLint param);
typedef FNGLLIGHTENVISGIXPROC *PFNGLLIGHTENVISGIXPROC;
#endif

#ifndef GL_IBM_rasterpos_clip
#define GL_IBM_rasterpos_clip 1
#endif

#ifndef GL_HP_texture_lighting
#define GL_HP_texture_lighting 1
#endif

#ifndef GL_EXT_draw_range_elements
#define GL_EXT_draw_range_elements 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glDrawRangeElementsEXT (GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLDRAWRANGEELEMENTSEXTPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
typedef FNGLDRAWRANGEELEMENTSEXTPROC *PFNGLDRAWRANGEELEMENTSEXTPROC;
#endif

#ifndef GL_WIN_phong_shading
#define GL_WIN_phong_shading 1
#endif

#ifndef GL_WIN_specular_fog
#define GL_WIN_specular_fog 1
#endif

#ifndef GL_EXT_light_texture
#define GL_EXT_light_texture 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glApplyTextureEXT (GLenum);
GLAPI void GLAPIENTRY glTextureLightEXT (GLenum);
GLAPI void GLAPIENTRY glTextureMaterialEXT (GLenum, GLenum);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLAPPLYTEXTUREEXTPROC) (GLenum mode);
typedef FNGLAPPLYTEXTUREEXTPROC *PFNGLAPPLYTEXTUREEXTPROC;
typedef void (GLAPIENTRY FNGLTEXTURELIGHTEXTPROC) (GLenum pname);
typedef FNGLTEXTURELIGHTEXTPROC *PFNGLTEXTURELIGHTEXTPROC;
typedef void (GLAPIENTRY FNGLTEXTUREMATERIALEXTPROC) (GLenum face, GLenum mode);
typedef FNGLTEXTUREMATERIALEXTPROC *PFNGLTEXTUREMATERIALEXTPROC;
#endif

#ifndef GL_SGIX_blend_alpha_minmax
#define GL_SGIX_blend_alpha_minmax 1
#endif

#ifndef GL_EXT_bgra
#define GL_EXT_bgra 1
#endif

#ifndef GL_SGIX_async
#define GL_SGIX_async 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glAsyncMarkerSGIX (GLuint);
GLAPI GLint GLAPIENTRY glFinishAsyncSGIX (GLuint *);
GLAPI GLint GLAPIENTRY glPollAsyncSGIX (GLuint *);
GLAPI GLuint GLAPIENTRY glGenAsyncMarkersSGIX (GLsizei);
GLAPI void GLAPIENTRY glDeleteAsyncMarkersSGIX (GLuint, GLsizei);
GLAPI GLboolean GLAPIENTRY glIsAsyncMarkerSGIX (GLuint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLASYNCMARKERSGIXPROC) (GLuint marker);
typedef FNGLASYNCMARKERSGIXPROC *PFNGLASYNCMARKERSGIXPROC;
typedef GLint (GLAPIENTRY FNGLFINISHASYNCSGIXPROC) (GLuint *markerp);
typedef FNGLFINISHASYNCSGIXPROC *PFNGLFINISHASYNCSGIXPROC;
typedef GLint (GLAPIENTRY FNGLPOLLASYNCSGIXPROC) (GLuint *markerp);
typedef FNGLPOLLASYNCSGIXPROC *PFNGLPOLLASYNCSGIXPROC;
typedef GLuint (GLAPIENTRY FNGLGENASYNCMARKERSSGIXPROC) (GLsizei range);
typedef FNGLGENASYNCMARKERSSGIXPROC *PFNGLGENASYNCMARKERSSGIXPROC;
typedef void (GLAPIENTRY FNGLDELETEASYNCMARKERSSGIXPROC) (GLuint marker, GLsizei range);
typedef FNGLDELETEASYNCMARKERSSGIXPROC *PFNGLDELETEASYNCMARKERSSGIXPROC;
typedef GLboolean (GLAPIENTRY FNGLISASYNCMARKERSGIXPROC) (GLuint marker);
typedef FNGLISASYNCMARKERSGIXPROC *PFNGLISASYNCMARKERSGIXPROC;
#endif

#ifndef GL_SGIX_async_pixel
#define GL_SGIX_async_pixel 1
#endif

#ifndef GL_SGIX_async_histogram
#define GL_SGIX_async_histogram 1
#endif

#ifndef GL_INTEL_parallel_arrays
#define GL_INTEL_parallel_arrays 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glVertexPointervINTEL (GLint, GLenum, const GLvoid* *);
GLAPI void GLAPIENTRY glNormalPointervINTEL (GLenum, const GLvoid* *);
GLAPI void GLAPIENTRY glColorPointervINTEL (GLint, GLenum, const GLvoid* *);
GLAPI void GLAPIENTRY glTexCoordPointervINTEL (GLint, GLenum, const GLvoid* *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLVERTEXPOINTERVINTELPROC) (GLint size, GLenum type, const GLvoid* *pointer);
typedef FNGLVERTEXPOINTERVINTELPROC *PFNGLVERTEXPOINTERVINTELPROC;
typedef void (GLAPIENTRY FNGLNORMALPOINTERVINTELPROC) (GLenum type, const GLvoid* *pointer);
typedef FNGLNORMALPOINTERVINTELPROC *PFNGLNORMALPOINTERVINTELPROC;
typedef void (GLAPIENTRY FNGLCOLORPOINTERVINTELPROC) (GLint size, GLenum type, const GLvoid* *pointer);
typedef FNGLCOLORPOINTERVINTELPROC *PFNGLCOLORPOINTERVINTELPROC;
typedef void (GLAPIENTRY FNGLTEXCOORDPOINTERVINTELPROC) (GLint size, GLenum type, const GLvoid* *pointer);
typedef FNGLTEXCOORDPOINTERVINTELPROC *PFNGLTEXCOORDPOINTERVINTELPROC;
#endif

#ifndef GL_HP_occlusion_test
#define GL_HP_occlusion_test 1
#endif

#ifndef GL_EXT_pixel_transform
#define GL_EXT_pixel_transform 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glPixelTransformParameteriEXT (GLenum, GLenum, GLint);
GLAPI void GLAPIENTRY glPixelTransformParameterfEXT (GLenum, GLenum, GLfloat);
GLAPI void GLAPIENTRY glPixelTransformParameterivEXT (GLenum, GLenum, const GLint *);
GLAPI void GLAPIENTRY glPixelTransformParameterfvEXT (GLenum, GLenum, const GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLPIXELTRANSFORMPARAMETERIEXTPROC) (GLenum target, GLenum pname, GLint param);
typedef FNGLPIXELTRANSFORMPARAMETERIEXTPROC *PFNGLPIXELTRANSFORMPARAMETERIEXTPROC;
typedef void (GLAPIENTRY FNGLPIXELTRANSFORMPARAMETERFEXTPROC) (GLenum target, GLenum pname, GLfloat param);
typedef FNGLPIXELTRANSFORMPARAMETERFEXTPROC *PFNGLPIXELTRANSFORMPARAMETERFEXTPROC;
typedef void (GLAPIENTRY FNGLPIXELTRANSFORMPARAMETERIVEXTPROC) (GLenum target, GLenum pname, const GLint *params);
typedef FNGLPIXELTRANSFORMPARAMETERIVEXTPROC *PFNGLPIXELTRANSFORMPARAMETERIVEXTPROC;
typedef void (GLAPIENTRY FNGLPIXELTRANSFORMPARAMETERFVEXTPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef FNGLPIXELTRANSFORMPARAMETERFVEXTPROC *PFNGLPIXELTRANSFORMPARAMETERFVEXTPROC;
#endif

#ifndef GL_EXT_pixel_transform_color_table
#define GL_EXT_pixel_transform_color_table 1
#endif

#ifndef GL_EXT_shared_texture_palette
#define GL_EXT_shared_texture_palette 1
#endif

#ifndef GL_EXT_separate_specular_color
#define GL_EXT_separate_specular_color 1
#endif

#ifndef GL_EXT_secondary_color
#define GL_EXT_secondary_color 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glSecondaryColor3bEXT (GLbyte, GLbyte, GLbyte);
GLAPI void GLAPIENTRY glSecondaryColor3bvEXT (const GLbyte *);
GLAPI void GLAPIENTRY glSecondaryColor3dEXT (GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glSecondaryColor3dvEXT (const GLdouble *);
GLAPI void GLAPIENTRY glSecondaryColor3fEXT (GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glSecondaryColor3fvEXT (const GLfloat *);
GLAPI void GLAPIENTRY glSecondaryColor3iEXT (GLint, GLint, GLint);
GLAPI void GLAPIENTRY glSecondaryColor3ivEXT (const GLint *);
GLAPI void GLAPIENTRY glSecondaryColor3sEXT (GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glSecondaryColor3svEXT (const GLshort *);
GLAPI void GLAPIENTRY glSecondaryColor3ubEXT (GLubyte, GLubyte, GLubyte);
GLAPI void GLAPIENTRY glSecondaryColor3ubvEXT (const GLubyte *);
GLAPI void GLAPIENTRY glSecondaryColor3uiEXT (GLuint, GLuint, GLuint);
GLAPI void GLAPIENTRY glSecondaryColor3uivEXT (const GLuint *);
GLAPI void GLAPIENTRY glSecondaryColor3usEXT (GLushort, GLushort, GLushort);
GLAPI void GLAPIENTRY glSecondaryColor3usvEXT (const GLushort *);
GLAPI void GLAPIENTRY glSecondaryColorPointerEXT (GLint, GLenum, GLsizei, const GLvoid *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3BEXTPROC) (GLbyte red, GLbyte green, GLbyte blue);
typedef FNGLSECONDARYCOLOR3BEXTPROC *PFNGLSECONDARYCOLOR3BEXTPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3BVEXTPROC) (const GLbyte *v);
typedef FNGLSECONDARYCOLOR3BVEXTPROC *PFNGLSECONDARYCOLOR3BVEXTPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3DEXTPROC) (GLdouble red, GLdouble green, GLdouble blue);
typedef FNGLSECONDARYCOLOR3DEXTPROC *PFNGLSECONDARYCOLOR3DEXTPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3DVEXTPROC) (const GLdouble *v);
typedef FNGLSECONDARYCOLOR3DVEXTPROC *PFNGLSECONDARYCOLOR3DVEXTPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3FEXTPROC) (GLfloat red, GLfloat green, GLfloat blue);
typedef FNGLSECONDARYCOLOR3FEXTPROC *PFNGLSECONDARYCOLOR3FEXTPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3FVEXTPROC) (const GLfloat *v);
typedef FNGLSECONDARYCOLOR3FVEXTPROC *PFNGLSECONDARYCOLOR3FVEXTPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3IEXTPROC) (GLint red, GLint green, GLint blue);
typedef FNGLSECONDARYCOLOR3IEXTPROC *PFNGLSECONDARYCOLOR3IEXTPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3IVEXTPROC) (const GLint *v);
typedef FNGLSECONDARYCOLOR3IVEXTPROC *PFNGLSECONDARYCOLOR3IVEXTPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3SEXTPROC) (GLshort red, GLshort green, GLshort blue);
typedef FNGLSECONDARYCOLOR3SEXTPROC *PFNGLSECONDARYCOLOR3SEXTPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3SVEXTPROC) (const GLshort *v);
typedef FNGLSECONDARYCOLOR3SVEXTPROC *PFNGLSECONDARYCOLOR3SVEXTPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3UBEXTPROC) (GLubyte red, GLubyte green, GLubyte blue);
typedef FNGLSECONDARYCOLOR3UBEXTPROC *PFNGLSECONDARYCOLOR3UBEXTPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3UBVEXTPROC) (const GLubyte *v);
typedef FNGLSECONDARYCOLOR3UBVEXTPROC *PFNGLSECONDARYCOLOR3UBVEXTPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3UIEXTPROC) (GLuint red, GLuint green, GLuint blue);
typedef FNGLSECONDARYCOLOR3UIEXTPROC *PFNGLSECONDARYCOLOR3UIEXTPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3UIVEXTPROC) (const GLuint *v);
typedef FNGLSECONDARYCOLOR3UIVEXTPROC *PFNGLSECONDARYCOLOR3UIVEXTPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3USEXTPROC) (GLushort red, GLushort green, GLushort blue);
typedef FNGLSECONDARYCOLOR3USEXTPROC *PFNGLSECONDARYCOLOR3USEXTPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLOR3USVEXTPROC) (const GLushort *v);
typedef FNGLSECONDARYCOLOR3USVEXTPROC *PFNGLSECONDARYCOLOR3USVEXTPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLORPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef FNGLSECONDARYCOLORPOINTEREXTPROC *PFNGLSECONDARYCOLORPOINTEREXTPROC;
#endif

#ifndef GL_EXT_texture_perturb_normal
#define GL_EXT_texture_perturb_normal 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glTextureNormalEXT (GLenum);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLTEXTURENORMALEXTPROC) (GLenum mode);
typedef FNGLTEXTURENORMALEXTPROC *PFNGLTEXTURENORMALEXTPROC;
#endif

#ifndef GL_EXT_multi_draw_arrays
#define GL_EXT_multi_draw_arrays 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glMultiDrawArraysEXT (GLenum, GLint *, GLsizei *, GLsizei);
GLAPI void GLAPIENTRY glMultiDrawElementsEXT (GLenum, const GLsizei *, GLenum, const GLvoid* *, GLsizei);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLMULTIDRAWARRAYSEXTPROC) (GLenum mode, GLint *first, GLsizei *count, GLsizei primcount);
typedef FNGLMULTIDRAWARRAYSEXTPROC *PFNGLMULTIDRAWARRAYSEXTPROC;
typedef void (GLAPIENTRY FNGLMULTIDRAWELEMENTSEXTPROC) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount);
typedef FNGLMULTIDRAWELEMENTSEXTPROC *PFNGLMULTIDRAWELEMENTSEXTPROC;
#endif

#ifndef GL_EXT_fog_coord
#define GL_EXT_fog_coord 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glFogCoordfEXT (GLfloat);
GLAPI void GLAPIENTRY glFogCoordfvEXT (const GLfloat *);
GLAPI void GLAPIENTRY glFogCoorddEXT (GLdouble);
GLAPI void GLAPIENTRY glFogCoorddvEXT (const GLdouble *);
GLAPI void GLAPIENTRY glFogCoordPointerEXT (GLenum, GLsizei, const GLvoid *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLFOGCOORDFEXTPROC) (GLfloat coord);
typedef FNGLFOGCOORDFEXTPROC *PFNGLFOGCOORDFEXTPROC;
typedef void (GLAPIENTRY FNGLFOGCOORDFVEXTPROC) (const GLfloat *coord);
typedef FNGLFOGCOORDFVEXTPROC *PFNGLFOGCOORDFVEXTPROC;
typedef void (GLAPIENTRY FNGLFOGCOORDDEXTPROC) (GLdouble coord);
typedef FNGLFOGCOORDDEXTPROC *PFNGLFOGCOORDDEXTPROC;
typedef void (GLAPIENTRY FNGLFOGCOORDDVEXTPROC) (const GLdouble *coord);
typedef FNGLFOGCOORDDVEXTPROC *PFNGLFOGCOORDDVEXTPROC;
typedef void (GLAPIENTRY FNGLFOGCOORDPOINTEREXTPROC) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef FNGLFOGCOORDPOINTEREXTPROC *PFNGLFOGCOORDPOINTEREXTPROC;
#endif

#ifndef GL_REND_screen_coordinates
#define GL_REND_screen_coordinates 1
#endif

#ifndef GL_EXT_coordinate_frame
#define GL_EXT_coordinate_frame 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glTangent3bEXT (GLbyte, GLbyte, GLbyte);
GLAPI void GLAPIENTRY glTangent3bvEXT (const GLbyte *);
GLAPI void GLAPIENTRY glTangent3dEXT (GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glTangent3dvEXT (const GLdouble *);
GLAPI void GLAPIENTRY glTangent3fEXT (GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glTangent3fvEXT (const GLfloat *);
GLAPI void GLAPIENTRY glTangent3iEXT (GLint, GLint, GLint);
GLAPI void GLAPIENTRY glTangent3ivEXT (const GLint *);
GLAPI void GLAPIENTRY glTangent3sEXT (GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glTangent3svEXT (const GLshort *);
GLAPI void GLAPIENTRY glBinormal3bEXT (GLbyte, GLbyte, GLbyte);
GLAPI void GLAPIENTRY glBinormal3bvEXT (const GLbyte *);
GLAPI void GLAPIENTRY glBinormal3dEXT (GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glBinormal3dvEXT (const GLdouble *);
GLAPI void GLAPIENTRY glBinormal3fEXT (GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glBinormal3fvEXT (const GLfloat *);
GLAPI void GLAPIENTRY glBinormal3iEXT (GLint, GLint, GLint);
GLAPI void GLAPIENTRY glBinormal3ivEXT (const GLint *);
GLAPI void GLAPIENTRY glBinormal3sEXT (GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glBinormal3svEXT (const GLshort *);
GLAPI void GLAPIENTRY glTangentPointerEXT (GLenum, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glBinormalPointerEXT (GLenum, GLsizei, const GLvoid *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLTANGENT3BEXTPROC) (GLbyte tx, GLbyte ty, GLbyte tz);
typedef FNGLTANGENT3BEXTPROC *PFNGLTANGENT3BEXTPROC;
typedef void (GLAPIENTRY FNGLTANGENT3BVEXTPROC) (const GLbyte *v);
typedef FNGLTANGENT3BVEXTPROC *PFNGLTANGENT3BVEXTPROC;
typedef void (GLAPIENTRY FNGLTANGENT3DEXTPROC) (GLdouble tx, GLdouble ty, GLdouble tz);
typedef FNGLTANGENT3DEXTPROC *PFNGLTANGENT3DEXTPROC;
typedef void (GLAPIENTRY FNGLTANGENT3DVEXTPROC) (const GLdouble *v);
typedef FNGLTANGENT3DVEXTPROC *PFNGLTANGENT3DVEXTPROC;
typedef void (GLAPIENTRY FNGLTANGENT3FEXTPROC) (GLfloat tx, GLfloat ty, GLfloat tz);
typedef FNGLTANGENT3FEXTPROC *PFNGLTANGENT3FEXTPROC;
typedef void (GLAPIENTRY FNGLTANGENT3FVEXTPROC) (const GLfloat *v);
typedef FNGLTANGENT3FVEXTPROC *PFNGLTANGENT3FVEXTPROC;
typedef void (GLAPIENTRY FNGLTANGENT3IEXTPROC) (GLint tx, GLint ty, GLint tz);
typedef FNGLTANGENT3IEXTPROC *PFNGLTANGENT3IEXTPROC;
typedef void (GLAPIENTRY FNGLTANGENT3IVEXTPROC) (const GLint *v);
typedef FNGLTANGENT3IVEXTPROC *PFNGLTANGENT3IVEXTPROC;
typedef void (GLAPIENTRY FNGLTANGENT3SEXTPROC) (GLshort tx, GLshort ty, GLshort tz);
typedef FNGLTANGENT3SEXTPROC *PFNGLTANGENT3SEXTPROC;
typedef void (GLAPIENTRY FNGLTANGENT3SVEXTPROC) (const GLshort *v);
typedef FNGLTANGENT3SVEXTPROC *PFNGLTANGENT3SVEXTPROC;
typedef void (GLAPIENTRY FNGLBINORMAL3BEXTPROC) (GLbyte bx, GLbyte by, GLbyte bz);
typedef FNGLBINORMAL3BEXTPROC *PFNGLBINORMAL3BEXTPROC;
typedef void (GLAPIENTRY FNGLBINORMAL3BVEXTPROC) (const GLbyte *v);
typedef FNGLBINORMAL3BVEXTPROC *PFNGLBINORMAL3BVEXTPROC;
typedef void (GLAPIENTRY FNGLBINORMAL3DEXTPROC) (GLdouble bx, GLdouble by, GLdouble bz);
typedef FNGLBINORMAL3DEXTPROC *PFNGLBINORMAL3DEXTPROC;
typedef void (GLAPIENTRY FNGLBINORMAL3DVEXTPROC) (const GLdouble *v);
typedef FNGLBINORMAL3DVEXTPROC *PFNGLBINORMAL3DVEXTPROC;
typedef void (GLAPIENTRY FNGLBINORMAL3FEXTPROC) (GLfloat bx, GLfloat by, GLfloat bz);
typedef FNGLBINORMAL3FEXTPROC *PFNGLBINORMAL3FEXTPROC;
typedef void (GLAPIENTRY FNGLBINORMAL3FVEXTPROC) (const GLfloat *v);
typedef FNGLBINORMAL3FVEXTPROC *PFNGLBINORMAL3FVEXTPROC;
typedef void (GLAPIENTRY FNGLBINORMAL3IEXTPROC) (GLint bx, GLint by, GLint bz);
typedef FNGLBINORMAL3IEXTPROC *PFNGLBINORMAL3IEXTPROC;
typedef void (GLAPIENTRY FNGLBINORMAL3IVEXTPROC) (const GLint *v);
typedef FNGLBINORMAL3IVEXTPROC *PFNGLBINORMAL3IVEXTPROC;
typedef void (GLAPIENTRY FNGLBINORMAL3SEXTPROC) (GLshort bx, GLshort by, GLshort bz);
typedef FNGLBINORMAL3SEXTPROC *PFNGLBINORMAL3SEXTPROC;
typedef void (GLAPIENTRY FNGLBINORMAL3SVEXTPROC) (const GLshort *v);
typedef FNGLBINORMAL3SVEXTPROC *PFNGLBINORMAL3SVEXTPROC;
typedef void (GLAPIENTRY FNGLTANGENTPOINTEREXTPROC) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef FNGLTANGENTPOINTEREXTPROC *PFNGLTANGENTPOINTEREXTPROC;
typedef void (GLAPIENTRY FNGLBINORMALPOINTEREXTPROC) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef FNGLBINORMALPOINTEREXTPROC *PFNGLBINORMALPOINTEREXTPROC;
#endif

#ifndef GL_EXT_texture_env_combine
#define GL_EXT_texture_env_combine 1
#endif

#ifndef GL_APPLE_specular_vector
#define GL_APPLE_specular_vector 1
#endif

#ifndef GL_APPLE_transform_hint
#define GL_APPLE_transform_hint 1
#endif

#ifndef GL_SGIX_fog_scale
#define GL_SGIX_fog_scale 1
#endif

#ifndef GL_SUNX_constant_data
#define GL_SUNX_constant_data 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glFinishTextureSUNX (void);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLFINISHTEXTURESUNXPROC) (void);
typedef FNGLFINISHTEXTURESUNXPROC *PFNGLFINISHTEXTURESUNXPROC;
#endif

#ifndef GL_SUN_global_alpha
#define GL_SUN_global_alpha 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glGlobalAlphaFactorbSUN (GLbyte);
GLAPI void GLAPIENTRY glGlobalAlphaFactorsSUN (GLshort);
GLAPI void GLAPIENTRY glGlobalAlphaFactoriSUN (GLint);
GLAPI void GLAPIENTRY glGlobalAlphaFactorfSUN (GLfloat);
GLAPI void GLAPIENTRY glGlobalAlphaFactordSUN (GLdouble);
GLAPI void GLAPIENTRY glGlobalAlphaFactorubSUN (GLubyte);
GLAPI void GLAPIENTRY glGlobalAlphaFactorusSUN (GLushort);
GLAPI void GLAPIENTRY glGlobalAlphaFactoruiSUN (GLuint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLGLOBALALPHAFACTORBSUNPROC) (GLbyte factor);
typedef FNGLGLOBALALPHAFACTORBSUNPROC *PFNGLGLOBALALPHAFACTORBSUNPROC;
typedef void (GLAPIENTRY FNGLGLOBALALPHAFACTORSSUNPROC) (GLshort factor);
typedef FNGLGLOBALALPHAFACTORSSUNPROC *PFNGLGLOBALALPHAFACTORSSUNPROC;
typedef void (GLAPIENTRY FNGLGLOBALALPHAFACTORISUNPROC) (GLint factor);
typedef FNGLGLOBALALPHAFACTORISUNPROC *PFNGLGLOBALALPHAFACTORISUNPROC;
typedef void (GLAPIENTRY FNGLGLOBALALPHAFACTORFSUNPROC) (GLfloat factor);
typedef FNGLGLOBALALPHAFACTORFSUNPROC *PFNGLGLOBALALPHAFACTORFSUNPROC;
typedef void (GLAPIENTRY FNGLGLOBALALPHAFACTORDSUNPROC) (GLdouble factor);
typedef FNGLGLOBALALPHAFACTORDSUNPROC *PFNGLGLOBALALPHAFACTORDSUNPROC;
typedef void (GLAPIENTRY FNGLGLOBALALPHAFACTORUBSUNPROC) (GLubyte factor);
typedef FNGLGLOBALALPHAFACTORUBSUNPROC *PFNGLGLOBALALPHAFACTORUBSUNPROC;
typedef void (GLAPIENTRY FNGLGLOBALALPHAFACTORUSSUNPROC) (GLushort factor);
typedef FNGLGLOBALALPHAFACTORUSSUNPROC *PFNGLGLOBALALPHAFACTORUSSUNPROC;
typedef void (GLAPIENTRY FNGLGLOBALALPHAFACTORUISUNPROC) (GLuint factor);
typedef FNGLGLOBALALPHAFACTORUISUNPROC *PFNGLGLOBALALPHAFACTORUISUNPROC;
#endif

#ifndef GL_SUN_triangle_list
#define GL_SUN_triangle_list 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glReplacementCodeuiSUN (GLuint);
GLAPI void GLAPIENTRY glReplacementCodeusSUN (GLushort);
GLAPI void GLAPIENTRY glReplacementCodeubSUN (GLubyte);
GLAPI void GLAPIENTRY glReplacementCodeuivSUN (const GLuint *);
GLAPI void GLAPIENTRY glReplacementCodeusvSUN (const GLushort *);
GLAPI void GLAPIENTRY glReplacementCodeubvSUN (const GLubyte *);
GLAPI void GLAPIENTRY glReplacementCodePointerSUN (GLenum, GLsizei, const GLvoid* *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUISUNPROC) (GLuint code);
typedef FNGLREPLACEMENTCODEUISUNPROC *PFNGLREPLACEMENTCODEUISUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUSSUNPROC) (GLushort code);
typedef FNGLREPLACEMENTCODEUSSUNPROC *PFNGLREPLACEMENTCODEUSSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUBSUNPROC) (GLubyte code);
typedef FNGLREPLACEMENTCODEUBSUNPROC *PFNGLREPLACEMENTCODEUBSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUIVSUNPROC) (const GLuint *code);
typedef FNGLREPLACEMENTCODEUIVSUNPROC *PFNGLREPLACEMENTCODEUIVSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUSVSUNPROC) (const GLushort *code);
typedef FNGLREPLACEMENTCODEUSVSUNPROC *PFNGLREPLACEMENTCODEUSVSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUBVSUNPROC) (const GLubyte *code);
typedef FNGLREPLACEMENTCODEUBVSUNPROC *PFNGLREPLACEMENTCODEUBVSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEPOINTERSUNPROC) (GLenum type, GLsizei stride, const GLvoid* *pointer);
typedef FNGLREPLACEMENTCODEPOINTERSUNPROC *PFNGLREPLACEMENTCODEPOINTERSUNPROC;
#endif

#ifndef GL_SUN_vertex
#define GL_SUN_vertex 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glColor4ubVertex2fSUN (GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glColor4ubVertex2fvSUN (const GLubyte *, const GLfloat *);
GLAPI void GLAPIENTRY glColor4ubVertex3fSUN (GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glColor4ubVertex3fvSUN (const GLubyte *, const GLfloat *);
GLAPI void GLAPIENTRY glColor3fVertex3fSUN (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glColor3fVertex3fvSUN (const GLfloat *, const GLfloat *);
GLAPI void GLAPIENTRY glNormal3fVertex3fSUN (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glNormal3fVertex3fvSUN (const GLfloat *, const GLfloat *);
GLAPI void GLAPIENTRY glColor4fNormal3fVertex3fSUN (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glColor4fNormal3fVertex3fvSUN (const GLfloat *, const GLfloat *, const GLfloat *);
GLAPI void GLAPIENTRY glTexCoord2fVertex3fSUN (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glTexCoord2fVertex3fvSUN (const GLfloat *, const GLfloat *);
GLAPI void GLAPIENTRY glTexCoord4fVertex4fSUN (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glTexCoord4fVertex4fvSUN (const GLfloat *, const GLfloat *);
GLAPI void GLAPIENTRY glTexCoord2fColor4ubVertex3fSUN (GLfloat, GLfloat, GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glTexCoord2fColor4ubVertex3fvSUN (const GLfloat *, const GLubyte *, const GLfloat *);
GLAPI void GLAPIENTRY glTexCoord2fColor3fVertex3fSUN (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glTexCoord2fColor3fVertex3fvSUN (const GLfloat *, const GLfloat *, const GLfloat *);
GLAPI void GLAPIENTRY glTexCoord2fNormal3fVertex3fSUN (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glTexCoord2fNormal3fVertex3fvSUN (const GLfloat *, const GLfloat *, const GLfloat *);
GLAPI void GLAPIENTRY glTexCoord2fColor4fNormal3fVertex3fSUN (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glTexCoord2fColor4fNormal3fVertex3fvSUN (const GLfloat *, const GLfloat *, const GLfloat *, const GLfloat *);
GLAPI void GLAPIENTRY glTexCoord4fColor4fNormal3fVertex4fSUN (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glTexCoord4fColor4fNormal3fVertex4fvSUN (const GLfloat *, const GLfloat *, const GLfloat *, const GLfloat *);
GLAPI void GLAPIENTRY glReplacementCodeuiVertex3fSUN (GLuint, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glReplacementCodeuiVertex3fvSUN (const GLuint *, const GLfloat *);
GLAPI void GLAPIENTRY glReplacementCodeuiColor4ubVertex3fSUN (GLuint, GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glReplacementCodeuiColor4ubVertex3fvSUN (const GLuint *, const GLubyte *, const GLfloat *);
GLAPI void GLAPIENTRY glReplacementCodeuiColor3fVertex3fSUN (GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glReplacementCodeuiColor3fVertex3fvSUN (const GLuint *, const GLfloat *, const GLfloat *);
GLAPI void GLAPIENTRY glReplacementCodeuiNormal3fVertex3fSUN (GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glReplacementCodeuiNormal3fVertex3fvSUN (const GLuint *, const GLfloat *, const GLfloat *);
GLAPI void GLAPIENTRY glReplacementCodeuiColor4fNormal3fVertex3fSUN (GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glReplacementCodeuiColor4fNormal3fVertex3fvSUN (const GLuint *, const GLfloat *, const GLfloat *, const GLfloat *);
GLAPI void GLAPIENTRY glReplacementCodeuiTexCoord2fVertex3fSUN (GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glReplacementCodeuiTexCoord2fVertex3fvSUN (const GLuint *, const GLfloat *, const GLfloat *);
GLAPI void GLAPIENTRY glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN (GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN (const GLuint *, const GLfloat *, const GLfloat *, const GLfloat *);
GLAPI void GLAPIENTRY glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN (GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN (const GLuint *, const GLfloat *, const GLfloat *, const GLfloat *, const GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLCOLOR4UBVERTEX2FSUNPROC) (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y);
typedef FNGLCOLOR4UBVERTEX2FSUNPROC *PFNGLCOLOR4UBVERTEX2FSUNPROC;
typedef void (GLAPIENTRY FNGLCOLOR4UBVERTEX2FVSUNPROC) (const GLubyte *c, const GLfloat *v);
typedef FNGLCOLOR4UBVERTEX2FVSUNPROC *PFNGLCOLOR4UBVERTEX2FVSUNPROC;
typedef void (GLAPIENTRY FNGLCOLOR4UBVERTEX3FSUNPROC) (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLCOLOR4UBVERTEX3FSUNPROC *PFNGLCOLOR4UBVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLCOLOR4UBVERTEX3FVSUNPROC) (const GLubyte *c, const GLfloat *v);
typedef FNGLCOLOR4UBVERTEX3FVSUNPROC *PFNGLCOLOR4UBVERTEX3FVSUNPROC;
typedef void (GLAPIENTRY FNGLCOLOR3FVERTEX3FSUNPROC) (GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLCOLOR3FVERTEX3FSUNPROC *PFNGLCOLOR3FVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLCOLOR3FVERTEX3FVSUNPROC) (const GLfloat *c, const GLfloat *v);
typedef FNGLCOLOR3FVERTEX3FVSUNPROC *PFNGLCOLOR3FVERTEX3FVSUNPROC;
typedef void (GLAPIENTRY FNGLNORMAL3FVERTEX3FSUNPROC) (GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLNORMAL3FVERTEX3FSUNPROC *PFNGLNORMAL3FVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLNORMAL3FVERTEX3FVSUNPROC) (const GLfloat *n, const GLfloat *v);
typedef FNGLNORMAL3FVERTEX3FVSUNPROC *PFNGLNORMAL3FVERTEX3FVSUNPROC;
typedef void (GLAPIENTRY FNGLCOLOR4FNORMAL3FVERTEX3FSUNPROC) (GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLCOLOR4FNORMAL3FVERTEX3FSUNPROC *PFNGLCOLOR4FNORMAL3FVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLCOLOR4FNORMAL3FVERTEX3FVSUNPROC) (const GLfloat *c, const GLfloat *n, const GLfloat *v);
typedef FNGLCOLOR4FNORMAL3FVERTEX3FVSUNPROC *PFNGLCOLOR4FNORMAL3FVERTEX3FVSUNPROC;
typedef void (GLAPIENTRY FNGLTEXCOORD2FVERTEX3FSUNPROC) (GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLTEXCOORD2FVERTEX3FSUNPROC *PFNGLTEXCOORD2FVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLTEXCOORD2FVERTEX3FVSUNPROC) (const GLfloat *tc, const GLfloat *v);
typedef FNGLTEXCOORD2FVERTEX3FVSUNPROC *PFNGLTEXCOORD2FVERTEX3FVSUNPROC;
typedef void (GLAPIENTRY FNGLTEXCOORD4FVERTEX4FSUNPROC) (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef FNGLTEXCOORD4FVERTEX4FSUNPROC *PFNGLTEXCOORD4FVERTEX4FSUNPROC;
typedef void (GLAPIENTRY FNGLTEXCOORD4FVERTEX4FVSUNPROC) (const GLfloat *tc, const GLfloat *v);
typedef FNGLTEXCOORD4FVERTEX4FVSUNPROC *PFNGLTEXCOORD4FVERTEX4FVSUNPROC;
typedef void (GLAPIENTRY FNGLTEXCOORD2FCOLOR4UBVERTEX3FSUNPROC) (GLfloat s, GLfloat t, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLTEXCOORD2FCOLOR4UBVERTEX3FSUNPROC *PFNGLTEXCOORD2FCOLOR4UBVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLTEXCOORD2FCOLOR4UBVERTEX3FVSUNPROC) (const GLfloat *tc, const GLubyte *c, const GLfloat *v);
typedef FNGLTEXCOORD2FCOLOR4UBVERTEX3FVSUNPROC *PFNGLTEXCOORD2FCOLOR4UBVERTEX3FVSUNPROC;
typedef void (GLAPIENTRY FNGLTEXCOORD2FCOLOR3FVERTEX3FSUNPROC) (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLTEXCOORD2FCOLOR3FVERTEX3FSUNPROC *PFNGLTEXCOORD2FCOLOR3FVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLTEXCOORD2FCOLOR3FVERTEX3FVSUNPROC) (const GLfloat *tc, const GLfloat *c, const GLfloat *v);
typedef FNGLTEXCOORD2FCOLOR3FVERTEX3FVSUNPROC *PFNGLTEXCOORD2FCOLOR3FVERTEX3FVSUNPROC;
typedef void (GLAPIENTRY FNGLTEXCOORD2FNORMAL3FVERTEX3FSUNPROC) (GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLTEXCOORD2FNORMAL3FVERTEX3FSUNPROC *PFNGLTEXCOORD2FNORMAL3FVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLTEXCOORD2FNORMAL3FVERTEX3FVSUNPROC) (const GLfloat *tc, const GLfloat *n, const GLfloat *v);
typedef FNGLTEXCOORD2FNORMAL3FVERTEX3FVSUNPROC *PFNGLTEXCOORD2FNORMAL3FVERTEX3FVSUNPROC;
typedef void (GLAPIENTRY FNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC) (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC *PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC) (const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
typedef FNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC *PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC;
typedef void (GLAPIENTRY FNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUNPROC) (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef FNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUNPROC *PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUNPROC;
typedef void (GLAPIENTRY FNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUNPROC) (const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
typedef FNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUNPROC *PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUIVERTEX3FSUNPROC) (GLuint rc, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLREPLACEMENTCODEUIVERTEX3FSUNPROC *PFNGLREPLACEMENTCODEUIVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUIVERTEX3FVSUNPROC) (const GLuint *rc, const GLfloat *v);
typedef FNGLREPLACEMENTCODEUIVERTEX3FVSUNPROC *PFNGLREPLACEMENTCODEUIVERTEX3FVSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUNPROC) (GLuint rc, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUNPROC *PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUNPROC) (const GLuint *rc, const GLubyte *c, const GLfloat *v);
typedef FNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUNPROC *PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUNPROC) (GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUNPROC *PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUNPROC) (const GLuint *rc, const GLfloat *c, const GLfloat *v);
typedef FNGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUNPROC *PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUNPROC) (GLuint rc, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUNPROC *PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUNPROC) (const GLuint *rc, const GLfloat *n, const GLfloat *v);
typedef FNGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUNPROC *PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUNPROC) (GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUNPROC *PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUNPROC) (const GLuint *rc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
typedef FNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUNPROC *PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUNPROC) (GLuint rc, GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUNPROC *PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUNPROC) (const GLuint *rc, const GLfloat *tc, const GLfloat *v);
typedef FNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUNPROC *PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUNPROC) (GLuint rc, GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUNPROC *PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUNPROC) (const GLuint *rc, const GLfloat *tc, const GLfloat *n, const GLfloat *v);
typedef FNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUNPROC *PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC) (GLuint rc, GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC *PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC;
typedef void (GLAPIENTRY FNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC) (const GLuint *rc, const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
typedef FNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC *PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC;
#endif

#ifndef GL_EXT_blend_func_separate
#define GL_EXT_blend_func_separate 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glBlendFuncSeparateEXT (GLenum, GLenum, GLenum, GLenum);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLBLENDFUNCSEPARATEEXTPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef FNGLBLENDFUNCSEPARATEEXTPROC *PFNGLBLENDFUNCSEPARATEEXTPROC;
#endif

#ifndef GL_INGR_blend_func_separate
#define GL_INGR_blend_func_separate 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glBlendFuncSeparateINGR (GLenum, GLenum, GLenum, GLenum);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLBLENDFUNCSEPARATEINGRPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef FNGLBLENDFUNCSEPARATEINGRPROC *PFNGLBLENDFUNCSEPARATEINGRPROC;
#endif

#ifndef GL_INGR_color_clamp
#define GL_INGR_color_clamp 1
#endif

#ifndef GL_INGR_interlace_read
#define GL_INGR_interlace_read 1
#endif

#ifndef GL_EXT_stencil_wrap
#define GL_EXT_stencil_wrap 1
#endif

#ifndef GL_EXT_422_pixels
#define GL_EXT_422_pixels 1
#endif

#ifndef GL_NV_texgen_reflection
#define GL_NV_texgen_reflection 1
#endif

#ifndef GL_SUN_convolution_border_modes
#define GL_SUN_convolution_border_modes 1
#endif

#ifndef GL_EXT_texture_env_add
#define GL_EXT_texture_env_add 1
#endif

#ifndef GL_EXT_texture_lod_bias
#define GL_EXT_texture_lod_bias 1
#endif

#ifndef GL_EXT_texture_filter_anisotropic
#define GL_EXT_texture_filter_anisotropic 1
#endif

#ifndef GL_EXT_vertex_weighting
#define GL_EXT_vertex_weighting 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glVertexWeightfEXT (GLfloat);
GLAPI void GLAPIENTRY glVertexWeightfvEXT (const GLfloat *);
GLAPI void GLAPIENTRY glVertexWeightPointerEXT (GLsizei, GLenum, GLsizei, const GLvoid *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLVERTEXWEIGHTFEXTPROC) (GLfloat weight);
typedef FNGLVERTEXWEIGHTFEXTPROC *PFNGLVERTEXWEIGHTFEXTPROC;
typedef void (GLAPIENTRY FNGLVERTEXWEIGHTFVEXTPROC) (const GLfloat *weight);
typedef FNGLVERTEXWEIGHTFVEXTPROC *PFNGLVERTEXWEIGHTFVEXTPROC;
typedef void (GLAPIENTRY FNGLVERTEXWEIGHTPOINTEREXTPROC) (GLsizei size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef FNGLVERTEXWEIGHTPOINTEREXTPROC *PFNGLVERTEXWEIGHTPOINTEREXTPROC;
#endif

#ifndef GL_NV_light_max_exponent
#define GL_NV_light_max_exponent 1
#endif

#ifndef GL_NV_vertex_array_range
#define GL_NV_vertex_array_range 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glFlushVertexArrayRangeNV (void);
GLAPI void GLAPIENTRY glVertexArrayRangeNV (GLsizei, const GLvoid *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLFLUSHVERTEXARRAYRANGENVPROC) (void);
typedef FNGLFLUSHVERTEXARRAYRANGENVPROC *PFNGLFLUSHVERTEXARRAYRANGENVPROC;
typedef void (GLAPIENTRY FNGLVERTEXARRAYRANGENVPROC) (GLsizei length, const GLvoid *pointer);
typedef FNGLVERTEXARRAYRANGENVPROC *PFNGLVERTEXARRAYRANGENVPROC;
#endif

#ifndef GL_NV_register_combiners
#define GL_NV_register_combiners 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glCombinerParameterfvNV (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glCombinerParameterfNV (GLenum, GLfloat);
GLAPI void GLAPIENTRY glCombinerParameterivNV (GLenum, const GLint *);
GLAPI void GLAPIENTRY glCombinerParameteriNV (GLenum, GLint);
GLAPI void GLAPIENTRY glCombinerInputNV (GLenum, GLenum, GLenum, GLenum, GLenum, GLenum);
GLAPI void GLAPIENTRY glCombinerOutputNV (GLenum, GLenum, GLenum, GLenum, GLenum, GLenum, GLenum, GLboolean, GLboolean, GLboolean);
GLAPI void GLAPIENTRY glFinalCombinerInputNV (GLenum, GLenum, GLenum, GLenum);
GLAPI void GLAPIENTRY glGetCombinerInputParameterfvNV (GLenum, GLenum, GLenum, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetCombinerInputParameterivNV (GLenum, GLenum, GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetCombinerOutputParameterfvNV (GLenum, GLenum, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetCombinerOutputParameterivNV (GLenum, GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetFinalCombinerInputParameterfvNV (GLenum, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetFinalCombinerInputParameterivNV (GLenum, GLenum, GLint *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLCOMBINERPARAMETERFVNVPROC) (GLenum pname, const GLfloat *params);
typedef FNGLCOMBINERPARAMETERFVNVPROC *PFNGLCOMBINERPARAMETERFVNVPROC;
typedef void (GLAPIENTRY FNGLCOMBINERPARAMETERFNVPROC) (GLenum pname, GLfloat param);
typedef FNGLCOMBINERPARAMETERFNVPROC *PFNGLCOMBINERPARAMETERFNVPROC;
typedef void (GLAPIENTRY FNGLCOMBINERPARAMETERIVNVPROC) (GLenum pname, const GLint *params);
typedef FNGLCOMBINERPARAMETERIVNVPROC *PFNGLCOMBINERPARAMETERIVNVPROC;
typedef void (GLAPIENTRY FNGLCOMBINERPARAMETERINVPROC) (GLenum pname, GLint param);
typedef FNGLCOMBINERPARAMETERINVPROC *PFNGLCOMBINERPARAMETERINVPROC;
typedef void (GLAPIENTRY FNGLCOMBINERINPUTNVPROC) (GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
typedef FNGLCOMBINERINPUTNVPROC *PFNGLCOMBINERINPUTNVPROC;
typedef void (GLAPIENTRY FNGLCOMBINEROUTPUTNVPROC) (GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum);
typedef FNGLCOMBINEROUTPUTNVPROC *PFNGLCOMBINEROUTPUTNVPROC;
typedef void (GLAPIENTRY FNGLFINALCOMBINERINPUTNVPROC) (GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
typedef FNGLFINALCOMBINERINPUTNVPROC *PFNGLFINALCOMBINERINPUTNVPROC;
typedef void (GLAPIENTRY FNGLGETCOMBINERINPUTPARAMETERFVNVPROC) (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat *params);
typedef FNGLGETCOMBINERINPUTPARAMETERFVNVPROC *PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC;
typedef void (GLAPIENTRY FNGLGETCOMBINERINPUTPARAMETERIVNVPROC) (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint *params);
typedef FNGLGETCOMBINERINPUTPARAMETERIVNVPROC *PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC;
typedef void (GLAPIENTRY FNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC) (GLenum stage, GLenum portion, GLenum pname, GLfloat *params);
typedef FNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC *PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC;
typedef void (GLAPIENTRY FNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC) (GLenum stage, GLenum portion, GLenum pname, GLint *params);
typedef FNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC *PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC;
typedef void (GLAPIENTRY FNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC) (GLenum variable, GLenum pname, GLfloat *params);
typedef FNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC *PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC;
typedef void (GLAPIENTRY FNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC) (GLenum variable, GLenum pname, GLint *params);
typedef FNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC *PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC;
#endif

#ifndef GL_NV_fog_distance
#define GL_NV_fog_distance 1
#endif

#ifndef GL_NV_texgen_emboss
#define GL_NV_texgen_emboss 1
#endif

#ifndef GL_NV_blend_square
#define GL_NV_blend_square 1
#endif

#ifndef GL_NV_texture_env_combine4
#define GL_NV_texture_env_combine4 1
#endif

#ifndef GL_MESA_resize_buffers
#define GL_MESA_resize_buffers 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glResizeBuffersMESA (void);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLRESIZEBUFFERSMESAPROC) (void);
typedef FNGLRESIZEBUFFERSMESAPROC *PFNGLRESIZEBUFFERSMESAPROC;
#endif

#ifndef GL_MESA_window_pos
#define GL_MESA_window_pos 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glWindowPos2dMESA (GLdouble, GLdouble);
GLAPI void GLAPIENTRY glWindowPos2dvMESA (const GLdouble *);
GLAPI void GLAPIENTRY glWindowPos2fMESA (GLfloat, GLfloat);
GLAPI void GLAPIENTRY glWindowPos2fvMESA (const GLfloat *);
GLAPI void GLAPIENTRY glWindowPos2iMESA (GLint, GLint);
GLAPI void GLAPIENTRY glWindowPos2ivMESA (const GLint *);
GLAPI void GLAPIENTRY glWindowPos2sMESA (GLshort, GLshort);
GLAPI void GLAPIENTRY glWindowPos2svMESA (const GLshort *);
GLAPI void GLAPIENTRY glWindowPos3dMESA (GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glWindowPos3dvMESA (const GLdouble *);
GLAPI void GLAPIENTRY glWindowPos3fMESA (GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glWindowPos3fvMESA (const GLfloat *);
GLAPI void GLAPIENTRY glWindowPos3iMESA (GLint, GLint, GLint);
GLAPI void GLAPIENTRY glWindowPos3ivMESA (const GLint *);
GLAPI void GLAPIENTRY glWindowPos3sMESA (GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glWindowPos3svMESA (const GLshort *);
GLAPI void GLAPIENTRY glWindowPos4dMESA (GLdouble, GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glWindowPos4dvMESA (const GLdouble *);
GLAPI void GLAPIENTRY glWindowPos4fMESA (GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glWindowPos4fvMESA (const GLfloat *);
GLAPI void GLAPIENTRY glWindowPos4iMESA (GLint, GLint, GLint, GLint);
GLAPI void GLAPIENTRY glWindowPos4ivMESA (const GLint *);
GLAPI void GLAPIENTRY glWindowPos4sMESA (GLshort, GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glWindowPos4svMESA (const GLshort *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLWINDOWPOS2DMESAPROC) (GLdouble x, GLdouble y);
typedef FNGLWINDOWPOS2DMESAPROC *PFNGLWINDOWPOS2DMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2DVMESAPROC) (const GLdouble *v);
typedef FNGLWINDOWPOS2DVMESAPROC *PFNGLWINDOWPOS2DVMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2FMESAPROC) (GLfloat x, GLfloat y);
typedef FNGLWINDOWPOS2FMESAPROC *PFNGLWINDOWPOS2FMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2FVMESAPROC) (const GLfloat *v);
typedef FNGLWINDOWPOS2FVMESAPROC *PFNGLWINDOWPOS2FVMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2IMESAPROC) (GLint x, GLint y);
typedef FNGLWINDOWPOS2IMESAPROC *PFNGLWINDOWPOS2IMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2IVMESAPROC) (const GLint *v);
typedef FNGLWINDOWPOS2IVMESAPROC *PFNGLWINDOWPOS2IVMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2SMESAPROC) (GLshort x, GLshort y);
typedef FNGLWINDOWPOS2SMESAPROC *PFNGLWINDOWPOS2SMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS2SVMESAPROC) (const GLshort *v);
typedef FNGLWINDOWPOS2SVMESAPROC *PFNGLWINDOWPOS2SVMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3DMESAPROC) (GLdouble x, GLdouble y, GLdouble z);
typedef FNGLWINDOWPOS3DMESAPROC *PFNGLWINDOWPOS3DMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3DVMESAPROC) (const GLdouble *v);
typedef FNGLWINDOWPOS3DVMESAPROC *PFNGLWINDOWPOS3DVMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3FMESAPROC) (GLfloat x, GLfloat y, GLfloat z);
typedef FNGLWINDOWPOS3FMESAPROC *PFNGLWINDOWPOS3FMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3FVMESAPROC) (const GLfloat *v);
typedef FNGLWINDOWPOS3FVMESAPROC *PFNGLWINDOWPOS3FVMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3IMESAPROC) (GLint x, GLint y, GLint z);
typedef FNGLWINDOWPOS3IMESAPROC *PFNGLWINDOWPOS3IMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3IVMESAPROC) (const GLint *v);
typedef FNGLWINDOWPOS3IVMESAPROC *PFNGLWINDOWPOS3IVMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3SMESAPROC) (GLshort x, GLshort y, GLshort z);
typedef FNGLWINDOWPOS3SMESAPROC *PFNGLWINDOWPOS3SMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS3SVMESAPROC) (const GLshort *v);
typedef FNGLWINDOWPOS3SVMESAPROC *PFNGLWINDOWPOS3SVMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS4DMESAPROC) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef FNGLWINDOWPOS4DMESAPROC *PFNGLWINDOWPOS4DMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS4DVMESAPROC) (const GLdouble *v);
typedef FNGLWINDOWPOS4DVMESAPROC *PFNGLWINDOWPOS4DVMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS4FMESAPROC) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef FNGLWINDOWPOS4FMESAPROC *PFNGLWINDOWPOS4FMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS4FVMESAPROC) (const GLfloat *v);
typedef FNGLWINDOWPOS4FVMESAPROC *PFNGLWINDOWPOS4FVMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS4IMESAPROC) (GLint x, GLint y, GLint z, GLint w);
typedef FNGLWINDOWPOS4IMESAPROC *PFNGLWINDOWPOS4IMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS4IVMESAPROC) (const GLint *v);
typedef FNGLWINDOWPOS4IVMESAPROC *PFNGLWINDOWPOS4IVMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS4SMESAPROC) (GLshort x, GLshort y, GLshort z, GLshort w);
typedef FNGLWINDOWPOS4SMESAPROC *PFNGLWINDOWPOS4SMESAPROC;
typedef void (GLAPIENTRY FNGLWINDOWPOS4SVMESAPROC) (const GLshort *v);
typedef FNGLWINDOWPOS4SVMESAPROC *PFNGLWINDOWPOS4SVMESAPROC;
#endif

#ifndef GL_IBM_cull_vertex
#define GL_IBM_cull_vertex 1
#endif

#ifndef GL_IBM_multimode_draw_arrays
#define GL_IBM_multimode_draw_arrays 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glMultiModeDrawArraysIBM (GLenum, const GLint *, const GLsizei *, GLsizei, GLint);
GLAPI void GLAPIENTRY glMultiModeDrawElementsIBM (const GLenum *, const GLsizei *, GLenum, const GLvoid* *, GLsizei, GLint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLMULTIMODEDRAWARRAYSIBMPROC) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount, GLint modestride);
typedef FNGLMULTIMODEDRAWARRAYSIBMPROC *PFNGLMULTIMODEDRAWARRAYSIBMPROC;
typedef void (GLAPIENTRY FNGLMULTIMODEDRAWELEMENTSIBMPROC) (const GLenum *mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount, GLint modestride);
typedef FNGLMULTIMODEDRAWELEMENTSIBMPROC *PFNGLMULTIMODEDRAWELEMENTSIBMPROC;
#endif

#ifndef GL_IBM_vertex_array_lists
#define GL_IBM_vertex_array_lists 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glColorPointerListIBM (GLint, GLenum, GLint, const GLvoid* *, GLint);
GLAPI void GLAPIENTRY glSecondaryColorPointerListIBM (GLint, GLenum, GLint, const GLvoid* *, GLint);
GLAPI void GLAPIENTRY glEdgeFlagPointerListIBM (GLint, const GLboolean* *, GLint);
GLAPI void GLAPIENTRY glFogCoordPointerListIBM (GLenum, GLint, const GLvoid* *, GLint);
GLAPI void GLAPIENTRY glIndexPointerListIBM (GLenum, GLint, const GLvoid* *, GLint);
GLAPI void GLAPIENTRY glNormalPointerListIBM (GLenum, GLint, const GLvoid* *, GLint);
GLAPI void GLAPIENTRY glTexCoordPointerListIBM (GLint, GLenum, GLint, const GLvoid* *, GLint);
GLAPI void GLAPIENTRY glVertexPointerListIBM (GLint, GLenum, GLint, const GLvoid* *, GLint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLCOLORPOINTERLISTIBMPROC) (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef FNGLCOLORPOINTERLISTIBMPROC *PFNGLCOLORPOINTERLISTIBMPROC;
typedef void (GLAPIENTRY FNGLSECONDARYCOLORPOINTERLISTIBMPROC) (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef FNGLSECONDARYCOLORPOINTERLISTIBMPROC *PFNGLSECONDARYCOLORPOINTERLISTIBMPROC;
typedef void (GLAPIENTRY FNGLEDGEFLAGPOINTERLISTIBMPROC) (GLint stride, const GLboolean* *pointer, GLint ptrstride);
typedef FNGLEDGEFLAGPOINTERLISTIBMPROC *PFNGLEDGEFLAGPOINTERLISTIBMPROC;
typedef void (GLAPIENTRY FNGLFOGCOORDPOINTERLISTIBMPROC) (GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef FNGLFOGCOORDPOINTERLISTIBMPROC *PFNGLFOGCOORDPOINTERLISTIBMPROC;
typedef void (GLAPIENTRY FNGLINDEXPOINTERLISTIBMPROC) (GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef FNGLINDEXPOINTERLISTIBMPROC *PFNGLINDEXPOINTERLISTIBMPROC;
typedef void (GLAPIENTRY FNGLNORMALPOINTERLISTIBMPROC) (GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef FNGLNORMALPOINTERLISTIBMPROC *PFNGLNORMALPOINTERLISTIBMPROC;
typedef void (GLAPIENTRY FNGLTEXCOORDPOINTERLISTIBMPROC) (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef FNGLTEXCOORDPOINTERLISTIBMPROC *PFNGLTEXCOORDPOINTERLISTIBMPROC;
typedef void (GLAPIENTRY FNGLVERTEXPOINTERLISTIBMPROC) (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
typedef FNGLVERTEXPOINTERLISTIBMPROC *PFNGLVERTEXPOINTERLISTIBMPROC;
#endif

#ifndef GL_SGIX_subsample
#define GL_SGIX_subsample 1
#endif

#ifndef GL_SGIX_ycrcba
#define GL_SGIX_ycrcba 1
#endif

#ifndef GL_SGIX_ycrcb_subsample
#define GL_SGIX_ycrcb_subsample 1
#endif

#ifndef GL_SGIX_depth_pass_instrument
#define GL_SGIX_depth_pass_instrument 1
#endif

#ifndef GL_3DFX_texture_compression_FXT1
#define GL_3DFX_texture_compression_FXT1 1
#endif

#ifndef GL_3DFX_multisample
#define GL_3DFX_multisample 1
#endif

#ifndef GL_3DFX_tbuffer
#define GL_3DFX_tbuffer 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glTbufferMask3DFX (GLuint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLTBUFFERMASK3DFXPROC) (GLuint mask);
typedef FNGLTBUFFERMASK3DFXPROC *PFNGLTBUFFERMASK3DFXPROC;
#endif

#ifndef GL_EXT_multisample
#define GL_EXT_multisample 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glSampleMaskEXT (GLclampf, GLboolean);
GLAPI void GLAPIENTRY glSamplePatternEXT (GLenum);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLSAMPLEMASKEXTPROC) (GLclampf value, GLboolean invert);
typedef FNGLSAMPLEMASKEXTPROC *PFNGLSAMPLEMASKEXTPROC;
typedef void (GLAPIENTRY FNGLSAMPLEPATTERNEXTPROC) (GLenum pattern);
typedef FNGLSAMPLEPATTERNEXTPROC *PFNGLSAMPLEPATTERNEXTPROC;
#endif

#ifndef GL_SGIX_vertex_preclip
#define GL_SGIX_vertex_preclip 1
#endif

#ifndef GL_SGIX_convolution_accuracy
#define GL_SGIX_convolution_accuracy 1
#endif

#ifndef GL_SGIX_resample
#define GL_SGIX_resample 1
#endif

#ifndef GL_SGIS_point_line_texgen
#define GL_SGIS_point_line_texgen 1
#endif

#ifndef GL_SGIS_texture_color_mask
#define GL_SGIS_texture_color_mask 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glTextureColorMaskSGIS (GLboolean, GLboolean, GLboolean, GLboolean);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLTEXTURECOLORMASKSGISPROC) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef FNGLTEXTURECOLORMASKSGISPROC *PFNGLTEXTURECOLORMASKSGISPROC;
#endif

#ifndef GL_SGIX_igloo_interface
#define GL_SGIX_igloo_interface 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glIglooInterfaceSGIX (GLenum, const GLvoid *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLIGLOOINTERFACESGIXPROC) (GLenum pname, const GLvoid *params);
typedef FNGLIGLOOINTERFACESGIXPROC *PFNGLIGLOOINTERFACESGIXPROC;
#endif

#ifndef GL_EXT_texture_env_dot3
#define GL_EXT_texture_env_dot3 1
#endif

#ifndef GL_ATI_texture_mirror_once
#define GL_ATI_texture_mirror_once 1
#endif

#ifndef GL_NV_fence
#define GL_NV_fence 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glDeleteFencesNV (GLsizei, const GLuint *);
GLAPI void GLAPIENTRY glGenFencesNV (GLsizei, GLuint *);
GLAPI GLboolean GLAPIENTRY glIsFenceNV (GLuint);
GLAPI GLboolean GLAPIENTRY glTestFenceNV (GLuint);
GLAPI void GLAPIENTRY glGetFenceivNV (GLuint, GLenum, GLint *);
GLAPI void GLAPIENTRY glFinishFenceNV (GLuint);
GLAPI void GLAPIENTRY glSetFenceNV (GLuint, GLenum);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLDELETEFENCESNVPROC) (GLsizei n, const GLuint *fences);
typedef FNGLDELETEFENCESNVPROC *PFNGLDELETEFENCESNVPROC;
typedef void (GLAPIENTRY FNGLGENFENCESNVPROC) (GLsizei n, GLuint *fences);
typedef FNGLGENFENCESNVPROC *PFNGLGENFENCESNVPROC;
typedef GLboolean (GLAPIENTRY FNGLISFENCENVPROC) (GLuint fence);
typedef FNGLISFENCENVPROC *PFNGLISFENCENVPROC;
typedef GLboolean (GLAPIENTRY FNGLTESTFENCENVPROC) (GLuint fence);
typedef FNGLTESTFENCENVPROC *PFNGLTESTFENCENVPROC;
typedef void (GLAPIENTRY FNGLGETFENCEIVNVPROC) (GLuint fence, GLenum pname, GLint *params);
typedef FNGLGETFENCEIVNVPROC *PFNGLGETFENCEIVNVPROC;
typedef void (GLAPIENTRY FNGLFINISHFENCENVPROC) (GLuint fence);
typedef FNGLFINISHFENCENVPROC *PFNGLFINISHFENCENVPROC;
typedef void (GLAPIENTRY FNGLSETFENCENVPROC) (GLuint fence, GLenum condition);
typedef FNGLSETFENCENVPROC *PFNGLSETFENCENVPROC;
#endif

#ifndef GL_NV_evaluators
#define GL_NV_evaluators 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glMapControlPointsNV (GLenum, GLuint, GLenum, GLsizei, GLsizei, GLint, GLint, GLboolean, const GLvoid *);
GLAPI void GLAPIENTRY glMapParameterivNV (GLenum, GLenum, const GLint *);
GLAPI void GLAPIENTRY glMapParameterfvNV (GLenum, GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glGetMapControlPointsNV (GLenum, GLuint, GLenum, GLsizei, GLsizei, GLboolean, GLvoid *);
GLAPI void GLAPIENTRY glGetMapParameterivNV (GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetMapParameterfvNV (GLenum, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetMapAttribParameterivNV (GLenum, GLuint, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetMapAttribParameterfvNV (GLenum, GLuint, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glEvalMapsNV (GLenum, GLenum);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLMAPCONTROLPOINTSNVPROC) (GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLint uorder, GLint vorder, GLboolean packed, const GLvoid *points);
typedef FNGLMAPCONTROLPOINTSNVPROC *PFNGLMAPCONTROLPOINTSNVPROC;
typedef void (GLAPIENTRY FNGLMAPPARAMETERIVNVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef FNGLMAPPARAMETERIVNVPROC *PFNGLMAPPARAMETERIVNVPROC;
typedef void (GLAPIENTRY FNGLMAPPARAMETERFVNVPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef FNGLMAPPARAMETERFVNVPROC *PFNGLMAPPARAMETERFVNVPROC;
typedef void (GLAPIENTRY FNGLGETMAPCONTROLPOINTSNVPROC) (GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLboolean packed, GLvoid *points);
typedef FNGLGETMAPCONTROLPOINTSNVPROC *PFNGLGETMAPCONTROLPOINTSNVPROC;
typedef void (GLAPIENTRY FNGLGETMAPPARAMETERIVNVPROC) (GLenum target, GLenum pname, GLint *params);
typedef FNGLGETMAPPARAMETERIVNVPROC *PFNGLGETMAPPARAMETERIVNVPROC;
typedef void (GLAPIENTRY FNGLGETMAPPARAMETERFVNVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef FNGLGETMAPPARAMETERFVNVPROC *PFNGLGETMAPPARAMETERFVNVPROC;
typedef void (GLAPIENTRY FNGLGETMAPATTRIBPARAMETERIVNVPROC) (GLenum target, GLuint index, GLenum pname, GLint *params);
typedef FNGLGETMAPATTRIBPARAMETERIVNVPROC *PFNGLGETMAPATTRIBPARAMETERIVNVPROC;
typedef void (GLAPIENTRY FNGLGETMAPATTRIBPARAMETERFVNVPROC) (GLenum target, GLuint index, GLenum pname, GLfloat *params);
typedef FNGLGETMAPATTRIBPARAMETERFVNVPROC *PFNGLGETMAPATTRIBPARAMETERFVNVPROC;
typedef void (GLAPIENTRY FNGLEVALMAPSNVPROC) (GLenum target, GLenum mode);
typedef FNGLEVALMAPSNVPROC *PFNGLEVALMAPSNVPROC;
#endif

#ifndef GL_NV_packed_depth_stencil
#define GL_NV_packed_depth_stencil 1
#endif

#ifndef GL_NV_register_combiners2
#define GL_NV_register_combiners2 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glCombinerStageParameterfvNV (GLenum, GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glGetCombinerStageParameterfvNV (GLenum, GLenum, GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLCOMBINERSTAGEPARAMETERFVNVPROC) (GLenum stage, GLenum pname, const GLfloat *params);
typedef FNGLCOMBINERSTAGEPARAMETERFVNVPROC *PFNGLCOMBINERSTAGEPARAMETERFVNVPROC;
typedef void (GLAPIENTRY FNGLGETCOMBINERSTAGEPARAMETERFVNVPROC) (GLenum stage, GLenum pname, GLfloat *params);
typedef FNGLGETCOMBINERSTAGEPARAMETERFVNVPROC *PFNGLGETCOMBINERSTAGEPARAMETERFVNVPROC;
#endif

#ifndef GL_NV_texture_compression_vtc
#define GL_NV_texture_compression_vtc 1
#endif

#ifndef GL_NV_texture_rectangle
#define GL_NV_texture_rectangle 1
#endif

#ifndef GL_NV_texture_shader
#define GL_NV_texture_shader 1
#endif

#ifndef GL_NV_texture_shader2
#define GL_NV_texture_shader2 1
#endif

#ifndef GL_NV_vertex_array_range2
#define GL_NV_vertex_array_range2 1
#endif

#ifndef GL_NV_vertex_program
#define GL_NV_vertex_program 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI GLboolean GLAPIENTRY glAreProgramsResidentNV (GLsizei, const GLuint *, GLboolean *);
GLAPI void GLAPIENTRY glBindProgramNV (GLenum, GLuint);
GLAPI void GLAPIENTRY glDeleteProgramsNV (GLsizei, const GLuint *);
GLAPI void GLAPIENTRY glExecuteProgramNV (GLenum, GLuint, const GLfloat *);
GLAPI void GLAPIENTRY glGenProgramsNV (GLsizei, GLuint *);
GLAPI void GLAPIENTRY glGetProgramParameterdvNV (GLenum, GLuint, GLenum, GLdouble *);
GLAPI void GLAPIENTRY glGetProgramParameterfvNV (GLenum, GLuint, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetProgramivNV (GLuint, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetProgramStringNV (GLuint, GLenum, GLubyte *);
GLAPI void GLAPIENTRY glGetTrackMatrixivNV (GLenum, GLuint, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetVertexAttribdvNV (GLuint, GLenum, GLdouble *);
GLAPI void GLAPIENTRY glGetVertexAttribfvNV (GLuint, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetVertexAttribivNV (GLuint, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetVertexAttribPointervNV (GLuint, GLenum, GLvoid* *);
GLAPI GLboolean GLAPIENTRY glIsProgramNV (GLuint);
GLAPI void GLAPIENTRY glLoadProgramNV (GLenum, GLuint, GLsizei, const GLubyte *);
GLAPI void GLAPIENTRY glProgramParameter4dNV (GLenum, GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glProgramParameter4dvNV (GLenum, GLuint, const GLdouble *);
GLAPI void GLAPIENTRY glProgramParameter4fNV (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glProgramParameter4fvNV (GLenum, GLuint, const GLfloat *);
GLAPI void GLAPIENTRY glProgramParameters4dvNV (GLenum, GLuint, GLuint, const GLdouble *);
GLAPI void GLAPIENTRY glProgramParameters4fvNV (GLenum, GLuint, GLuint, const GLfloat *);
GLAPI void GLAPIENTRY glRequestResidentProgramsNV (GLsizei, const GLuint *);
GLAPI void GLAPIENTRY glTrackMatrixNV (GLenum, GLuint, GLenum, GLenum);
GLAPI void GLAPIENTRY glVertexAttribPointerNV (GLuint, GLint, GLenum, GLsizei, const GLvoid *);
GLAPI void GLAPIENTRY glVertexAttrib1dNV (GLuint, GLdouble);
GLAPI void GLAPIENTRY glVertexAttrib1dvNV (GLuint, const GLdouble *);
GLAPI void GLAPIENTRY glVertexAttrib1fNV (GLuint, GLfloat);
GLAPI void GLAPIENTRY glVertexAttrib1fvNV (GLuint, const GLfloat *);
GLAPI void GLAPIENTRY glVertexAttrib1sNV (GLuint, GLshort);
GLAPI void GLAPIENTRY glVertexAttrib1svNV (GLuint, const GLshort *);
GLAPI void GLAPIENTRY glVertexAttrib2dNV (GLuint, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glVertexAttrib2dvNV (GLuint, const GLdouble *);
GLAPI void GLAPIENTRY glVertexAttrib2fNV (GLuint, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glVertexAttrib2fvNV (GLuint, const GLfloat *);
GLAPI void GLAPIENTRY glVertexAttrib2sNV (GLuint, GLshort, GLshort);
GLAPI void GLAPIENTRY glVertexAttrib2svNV (GLuint, const GLshort *);
GLAPI void GLAPIENTRY glVertexAttrib3dNV (GLuint, GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glVertexAttrib3dvNV (GLuint, const GLdouble *);
GLAPI void GLAPIENTRY glVertexAttrib3fNV (GLuint, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glVertexAttrib3fvNV (GLuint, const GLfloat *);
GLAPI void GLAPIENTRY glVertexAttrib3sNV (GLuint, GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glVertexAttrib3svNV (GLuint, const GLshort *);
GLAPI void GLAPIENTRY glVertexAttrib4dNV (GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glVertexAttrib4dvNV (GLuint, const GLdouble *);
GLAPI void GLAPIENTRY glVertexAttrib4fNV (GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glVertexAttrib4fvNV (GLuint, const GLfloat *);
GLAPI void GLAPIENTRY glVertexAttrib4sNV (GLuint, GLshort, GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glVertexAttrib4svNV (GLuint, const GLshort *);
GLAPI void GLAPIENTRY glVertexAttrib4ubNV (GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
GLAPI void GLAPIENTRY glVertexAttrib4ubvNV (GLuint, const GLubyte *);
GLAPI void GLAPIENTRY glVertexAttribs1dvNV (GLuint, GLsizei, const GLdouble *);
GLAPI void GLAPIENTRY glVertexAttribs1fvNV (GLuint, GLsizei, const GLfloat *);
GLAPI void GLAPIENTRY glVertexAttribs1svNV (GLuint, GLsizei, const GLshort *);
GLAPI void GLAPIENTRY glVertexAttribs2dvNV (GLuint, GLsizei, const GLdouble *);
GLAPI void GLAPIENTRY glVertexAttribs2fvNV (GLuint, GLsizei, const GLfloat *);
GLAPI void GLAPIENTRY glVertexAttribs2svNV (GLuint, GLsizei, const GLshort *);
GLAPI void GLAPIENTRY glVertexAttribs3dvNV (GLuint, GLsizei, const GLdouble *);
GLAPI void GLAPIENTRY glVertexAttribs3fvNV (GLuint, GLsizei, const GLfloat *);
GLAPI void GLAPIENTRY glVertexAttribs3svNV (GLuint, GLsizei, const GLshort *);
GLAPI void GLAPIENTRY glVertexAttribs4dvNV (GLuint, GLsizei, const GLdouble *);
GLAPI void GLAPIENTRY glVertexAttribs4fvNV (GLuint, GLsizei, const GLfloat *);
GLAPI void GLAPIENTRY glVertexAttribs4svNV (GLuint, GLsizei, const GLshort *);
GLAPI void GLAPIENTRY glVertexAttribs4ubvNV (GLuint, GLsizei, const GLubyte *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef GLboolean (GLAPIENTRY FNGLAREPROGRAMSRESIDENTNVPROC) (GLsizei n, const GLuint *programs, GLboolean *residences);
typedef FNGLAREPROGRAMSRESIDENTNVPROC *PFNGLAREPROGRAMSRESIDENTNVPROC;
typedef void (GLAPIENTRY FNGLBINDPROGRAMNVPROC) (GLenum target, GLuint id);
typedef FNGLBINDPROGRAMNVPROC *PFNGLBINDPROGRAMNVPROC;
typedef void (GLAPIENTRY FNGLDELETEPROGRAMSNVPROC) (GLsizei n, const GLuint *programs);
typedef FNGLDELETEPROGRAMSNVPROC *PFNGLDELETEPROGRAMSNVPROC;
typedef void (GLAPIENTRY FNGLEXECUTEPROGRAMNVPROC) (GLenum target, GLuint id, const GLfloat *params);
typedef FNGLEXECUTEPROGRAMNVPROC *PFNGLEXECUTEPROGRAMNVPROC;
typedef void (GLAPIENTRY FNGLGENPROGRAMSNVPROC) (GLsizei n, GLuint *programs);
typedef FNGLGENPROGRAMSNVPROC *PFNGLGENPROGRAMSNVPROC;
typedef void (GLAPIENTRY FNGLGETPROGRAMPARAMETERDVNVPROC) (GLenum target, GLuint index, GLenum pname, GLdouble *params);
typedef FNGLGETPROGRAMPARAMETERDVNVPROC *PFNGLGETPROGRAMPARAMETERDVNVPROC;
typedef void (GLAPIENTRY FNGLGETPROGRAMPARAMETERFVNVPROC) (GLenum target, GLuint index, GLenum pname, GLfloat *params);
typedef FNGLGETPROGRAMPARAMETERFVNVPROC *PFNGLGETPROGRAMPARAMETERFVNVPROC;
typedef void (GLAPIENTRY FNGLGETPROGRAMIVNVPROC) (GLuint id, GLenum pname, GLint *params);
typedef FNGLGETPROGRAMIVNVPROC *PFNGLGETPROGRAMIVNVPROC;
typedef void (GLAPIENTRY FNGLGETPROGRAMSTRINGNVPROC) (GLuint id, GLenum pname, GLubyte *program);
typedef FNGLGETPROGRAMSTRINGNVPROC *PFNGLGETPROGRAMSTRINGNVPROC;
typedef void (GLAPIENTRY FNGLGETTRACKMATRIXIVNVPROC) (GLenum target, GLuint address, GLenum pname, GLint *params);
typedef FNGLGETTRACKMATRIXIVNVPROC *PFNGLGETTRACKMATRIXIVNVPROC;
typedef void (GLAPIENTRY FNGLGETVERTEXATTRIBDVNVPROC) (GLuint index, GLenum pname, GLdouble *params);
typedef FNGLGETVERTEXATTRIBDVNVPROC *PFNGLGETVERTEXATTRIBDVNVPROC;
typedef void (GLAPIENTRY FNGLGETVERTEXATTRIBFVNVPROC) (GLuint index, GLenum pname, GLfloat *params);
typedef FNGLGETVERTEXATTRIBFVNVPROC *PFNGLGETVERTEXATTRIBFVNVPROC;
typedef void (GLAPIENTRY FNGLGETVERTEXATTRIBIVNVPROC) (GLuint index, GLenum pname, GLint *params);
typedef FNGLGETVERTEXATTRIBIVNVPROC *PFNGLGETVERTEXATTRIBIVNVPROC;
typedef void (GLAPIENTRY FNGLGETVERTEXATTRIBPOINTERVNVPROC) (GLuint index, GLenum pname, GLvoid* *pointer);
typedef FNGLGETVERTEXATTRIBPOINTERVNVPROC *PFNGLGETVERTEXATTRIBPOINTERVNVPROC;
typedef GLboolean (GLAPIENTRY FNGLISPROGRAMNVPROC) (GLuint id);
typedef FNGLISPROGRAMNVPROC *PFNGLISPROGRAMNVPROC;
typedef void (GLAPIENTRY FNGLLOADPROGRAMNVPROC) (GLenum target, GLuint id, GLsizei len, const GLubyte *program);
typedef FNGLLOADPROGRAMNVPROC *PFNGLLOADPROGRAMNVPROC;
typedef void (GLAPIENTRY FNGLPROGRAMPARAMETER4DNVPROC) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef FNGLPROGRAMPARAMETER4DNVPROC *PFNGLPROGRAMPARAMETER4DNVPROC;
typedef void (GLAPIENTRY FNGLPROGRAMPARAMETER4DVNVPROC) (GLenum target, GLuint index, const GLdouble *v);
typedef FNGLPROGRAMPARAMETER4DVNVPROC *PFNGLPROGRAMPARAMETER4DVNVPROC;
typedef void (GLAPIENTRY FNGLPROGRAMPARAMETER4FNVPROC) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef FNGLPROGRAMPARAMETER4FNVPROC *PFNGLPROGRAMPARAMETER4FNVPROC;
typedef void (GLAPIENTRY FNGLPROGRAMPARAMETER4FVNVPROC) (GLenum target, GLuint index, const GLfloat *v);
typedef FNGLPROGRAMPARAMETER4FVNVPROC *PFNGLPROGRAMPARAMETER4FVNVPROC;
typedef void (GLAPIENTRY FNGLPROGRAMPARAMETERS4DVNVPROC) (GLenum target, GLuint index, GLuint count, const GLdouble *v);
typedef FNGLPROGRAMPARAMETERS4DVNVPROC *PFNGLPROGRAMPARAMETERS4DVNVPROC;
typedef void (GLAPIENTRY FNGLPROGRAMPARAMETERS4FVNVPROC) (GLenum target, GLuint index, GLuint count, const GLfloat *v);
typedef FNGLPROGRAMPARAMETERS4FVNVPROC *PFNGLPROGRAMPARAMETERS4FVNVPROC;
typedef void (GLAPIENTRY FNGLREQUESTRESIDENTPROGRAMSNVPROC) (GLsizei n, const GLuint *programs);
typedef FNGLREQUESTRESIDENTPROGRAMSNVPROC *PFNGLREQUESTRESIDENTPROGRAMSNVPROC;
typedef void (GLAPIENTRY FNGLTRACKMATRIXNVPROC) (GLenum target, GLuint address, GLenum matrix, GLenum transform);
typedef FNGLTRACKMATRIXNVPROC *PFNGLTRACKMATRIXNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIBPOINTERNVPROC) (GLuint index, GLint fsize, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef FNGLVERTEXATTRIBPOINTERNVPROC *PFNGLVERTEXATTRIBPOINTERNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB1DNVPROC) (GLuint index, GLdouble x);
typedef FNGLVERTEXATTRIB1DNVPROC *PFNGLVERTEXATTRIB1DNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB1DVNVPROC) (GLuint index, const GLdouble *v);
typedef FNGLVERTEXATTRIB1DVNVPROC *PFNGLVERTEXATTRIB1DVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB1FNVPROC) (GLuint index, GLfloat x);
typedef FNGLVERTEXATTRIB1FNVPROC *PFNGLVERTEXATTRIB1FNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB1FVNVPROC) (GLuint index, const GLfloat *v);
typedef FNGLVERTEXATTRIB1FVNVPROC *PFNGLVERTEXATTRIB1FVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB1SNVPROC) (GLuint index, GLshort x);
typedef FNGLVERTEXATTRIB1SNVPROC *PFNGLVERTEXATTRIB1SNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB1SVNVPROC) (GLuint index, const GLshort *v);
typedef FNGLVERTEXATTRIB1SVNVPROC *PFNGLVERTEXATTRIB1SVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB2DNVPROC) (GLuint index, GLdouble x, GLdouble y);
typedef FNGLVERTEXATTRIB2DNVPROC *PFNGLVERTEXATTRIB2DNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB2DVNVPROC) (GLuint index, const GLdouble *v);
typedef FNGLVERTEXATTRIB2DVNVPROC *PFNGLVERTEXATTRIB2DVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB2FNVPROC) (GLuint index, GLfloat x, GLfloat y);
typedef FNGLVERTEXATTRIB2FNVPROC *PFNGLVERTEXATTRIB2FNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB2FVNVPROC) (GLuint index, const GLfloat *v);
typedef FNGLVERTEXATTRIB2FVNVPROC *PFNGLVERTEXATTRIB2FVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB2SNVPROC) (GLuint index, GLshort x, GLshort y);
typedef FNGLVERTEXATTRIB2SNVPROC *PFNGLVERTEXATTRIB2SNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB2SVNVPROC) (GLuint index, const GLshort *v);
typedef FNGLVERTEXATTRIB2SVNVPROC *PFNGLVERTEXATTRIB2SVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB3DNVPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef FNGLVERTEXATTRIB3DNVPROC *PFNGLVERTEXATTRIB3DNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB3DVNVPROC) (GLuint index, const GLdouble *v);
typedef FNGLVERTEXATTRIB3DVNVPROC *PFNGLVERTEXATTRIB3DVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB3FNVPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLVERTEXATTRIB3FNVPROC *PFNGLVERTEXATTRIB3FNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB3FVNVPROC) (GLuint index, const GLfloat *v);
typedef FNGLVERTEXATTRIB3FVNVPROC *PFNGLVERTEXATTRIB3FVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB3SNVPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef FNGLVERTEXATTRIB3SNVPROC *PFNGLVERTEXATTRIB3SNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB3SVNVPROC) (GLuint index, const GLshort *v);
typedef FNGLVERTEXATTRIB3SVNVPROC *PFNGLVERTEXATTRIB3SVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4DNVPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef FNGLVERTEXATTRIB4DNVPROC *PFNGLVERTEXATTRIB4DNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4DVNVPROC) (GLuint index, const GLdouble *v);
typedef FNGLVERTEXATTRIB4DVNVPROC *PFNGLVERTEXATTRIB4DVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4FNVPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef FNGLVERTEXATTRIB4FNVPROC *PFNGLVERTEXATTRIB4FNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4FVNVPROC) (GLuint index, const GLfloat *v);
typedef FNGLVERTEXATTRIB4FVNVPROC *PFNGLVERTEXATTRIB4FVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4SNVPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef FNGLVERTEXATTRIB4SNVPROC *PFNGLVERTEXATTRIB4SNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4SVNVPROC) (GLuint index, const GLshort *v);
typedef FNGLVERTEXATTRIB4SVNVPROC *PFNGLVERTEXATTRIB4SVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4UBNVPROC) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef FNGLVERTEXATTRIB4UBNVPROC *PFNGLVERTEXATTRIB4UBNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIB4UBVNVPROC) (GLuint index, const GLubyte *v);
typedef FNGLVERTEXATTRIB4UBVNVPROC *PFNGLVERTEXATTRIB4UBVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIBS1DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef FNGLVERTEXATTRIBS1DVNVPROC *PFNGLVERTEXATTRIBS1DVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIBS1FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef FNGLVERTEXATTRIBS1FVNVPROC *PFNGLVERTEXATTRIBS1FVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIBS1SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef FNGLVERTEXATTRIBS1SVNVPROC *PFNGLVERTEXATTRIBS1SVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIBS2DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef FNGLVERTEXATTRIBS2DVNVPROC *PFNGLVERTEXATTRIBS2DVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIBS2FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef FNGLVERTEXATTRIBS2FVNVPROC *PFNGLVERTEXATTRIBS2FVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIBS2SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef FNGLVERTEXATTRIBS2SVNVPROC *PFNGLVERTEXATTRIBS2SVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIBS3DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef FNGLVERTEXATTRIBS3DVNVPROC *PFNGLVERTEXATTRIBS3DVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIBS3FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef FNGLVERTEXATTRIBS3FVNVPROC *PFNGLVERTEXATTRIBS3FVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIBS3SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef FNGLVERTEXATTRIBS3SVNVPROC *PFNGLVERTEXATTRIBS3SVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIBS4DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef FNGLVERTEXATTRIBS4DVNVPROC *PFNGLVERTEXATTRIBS4DVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIBS4FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef FNGLVERTEXATTRIBS4FVNVPROC *PFNGLVERTEXATTRIBS4FVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIBS4SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef FNGLVERTEXATTRIBS4SVNVPROC *PFNGLVERTEXATTRIBS4SVNVPROC;
typedef void (GLAPIENTRY FNGLVERTEXATTRIBS4UBVNVPROC) (GLuint index, GLsizei count, const GLubyte *v);
typedef FNGLVERTEXATTRIBS4UBVNVPROC *PFNGLVERTEXATTRIBS4UBVNVPROC;
#endif

#ifndef GL_SGIX_texture_coordinate_clamp
#define GL_SGIX_texture_coordinate_clamp 1
#endif

#ifndef GL_SGIX_scalebias_hint
#define GL_SGIX_scalebias_hint 1
#endif

#ifndef GL_OML_interlace
#define GL_OML_interlace 1
#endif

#ifndef GL_OML_subsample
#define GL_OML_subsample 1
#endif

#ifndef GL_OML_resample
#define GL_OML_resample 1
#endif

#ifndef GL_NV_copy_depth_to_color
#define GL_NV_copy_depth_to_color 1
#endif

#ifndef GL_ATI_envmap_bumpmap
#define GL_ATI_envmap_bumpmap 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glTexBumpParameterivATI (GLenum, const GLint *);
GLAPI void GLAPIENTRY glTexBumpParameterfvATI (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glGetTexBumpParameterivATI (GLenum, GLint *);
GLAPI void GLAPIENTRY glGetTexBumpParameterfvATI (GLenum, GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLTEXBUMPPARAMETERIVATIPROC) (GLenum pname, const GLint *param);
typedef FNGLTEXBUMPPARAMETERIVATIPROC *PFNGLTEXBUMPPARAMETERIVATIPROC;
typedef void (GLAPIENTRY FNGLTEXBUMPPARAMETERFVATIPROC) (GLenum pname, const GLfloat *param);
typedef FNGLTEXBUMPPARAMETERFVATIPROC *PFNGLTEXBUMPPARAMETERFVATIPROC;
typedef void (GLAPIENTRY FNGLGETTEXBUMPPARAMETERIVATIPROC) (GLenum pname, GLint *param);
typedef FNGLGETTEXBUMPPARAMETERIVATIPROC *PFNGLGETTEXBUMPPARAMETERIVATIPROC;
typedef void (GLAPIENTRY FNGLGETTEXBUMPPARAMETERFVATIPROC) (GLenum pname, GLfloat *param);
typedef FNGLGETTEXBUMPPARAMETERFVATIPROC *PFNGLGETTEXBUMPPARAMETERFVATIPROC;
#endif

#ifndef GL_ATI_fragment_shader
#define GL_ATI_fragment_shader 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI GLuint GLAPIENTRY glGenFragmentShadersATI (GLuint);
GLAPI void GLAPIENTRY glBindFragmentShaderATI (GLuint);
GLAPI void GLAPIENTRY glDeleteFragmentShaderATI (GLuint);
GLAPI void GLAPIENTRY glBeginFragmentShaderATI (void);
GLAPI void GLAPIENTRY glEndFragmentShaderATI (void);
GLAPI void GLAPIENTRY glPassTexCoordATI (GLuint, GLuint, GLenum);
GLAPI void GLAPIENTRY glSampleMapATI (GLuint, GLuint, GLenum);
GLAPI void GLAPIENTRY glColorFragmentOp1ATI (GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
GLAPI void GLAPIENTRY glColorFragmentOp2ATI (GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
GLAPI void GLAPIENTRY glColorFragmentOp3ATI (GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
GLAPI void GLAPIENTRY glAlphaFragmentOp1ATI (GLenum, GLuint, GLuint, GLuint, GLuint, GLuint);
GLAPI void GLAPIENTRY glAlphaFragmentOp2ATI (GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
GLAPI void GLAPIENTRY glAlphaFragmentOp3ATI (GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
GLAPI void GLAPIENTRY glSetFragmentShaderConstantATI (GLuint, const GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef GLuint (GLAPIENTRY FNGLGENFRAGMENTSHADERSATIPROC) (GLuint range);
typedef FNGLGENFRAGMENTSHADERSATIPROC *PFNGLGENFRAGMENTSHADERSATIPROC;
typedef void (GLAPIENTRY FNGLBINDFRAGMENTSHADERATIPROC) (GLuint id);
typedef FNGLBINDFRAGMENTSHADERATIPROC *PFNGLBINDFRAGMENTSHADERATIPROC;
typedef void (GLAPIENTRY FNGLDELETEFRAGMENTSHADERATIPROC) (GLuint id);
typedef FNGLDELETEFRAGMENTSHADERATIPROC *PFNGLDELETEFRAGMENTSHADERATIPROC;
typedef void (GLAPIENTRY FNGLBEGINFRAGMENTSHADERATIPROC) (void);
typedef FNGLBEGINFRAGMENTSHADERATIPROC *PFNGLBEGINFRAGMENTSHADERATIPROC;
typedef void (GLAPIENTRY FNGLENDFRAGMENTSHADERATIPROC) (void);
typedef FNGLENDFRAGMENTSHADERATIPROC *PFNGLENDFRAGMENTSHADERATIPROC;
typedef void (GLAPIENTRY FNGLPASSTEXCOORDATIPROC) (GLuint dst, GLuint coord, GLenum swizzle);
typedef FNGLPASSTEXCOORDATIPROC *PFNGLPASSTEXCOORDATIPROC;
typedef void (GLAPIENTRY FNGLSAMPLEMAPATIPROC) (GLuint dst, GLuint interp, GLenum swizzle);
typedef FNGLSAMPLEMAPATIPROC *PFNGLSAMPLEMAPATIPROC;
typedef void (GLAPIENTRY FNGLCOLORFRAGMENTOP1ATIPROC) (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);
typedef FNGLCOLORFRAGMENTOP1ATIPROC *PFNGLCOLORFRAGMENTOP1ATIPROC;
typedef void (GLAPIENTRY FNGLCOLORFRAGMENTOP2ATIPROC) (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);
typedef FNGLCOLORFRAGMENTOP2ATIPROC *PFNGLCOLORFRAGMENTOP2ATIPROC;
typedef void (GLAPIENTRY FNGLCOLORFRAGMENTOP3ATIPROC) (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);
typedef FNGLCOLORFRAGMENTOP3ATIPROC *PFNGLCOLORFRAGMENTOP3ATIPROC;
typedef void (GLAPIENTRY FNGLALPHAFRAGMENTOP1ATIPROC) (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);
typedef FNGLALPHAFRAGMENTOP1ATIPROC *PFNGLALPHAFRAGMENTOP1ATIPROC;
typedef void (GLAPIENTRY FNGLALPHAFRAGMENTOP2ATIPROC) (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);
typedef FNGLALPHAFRAGMENTOP2ATIPROC *PFNGLALPHAFRAGMENTOP2ATIPROC;
typedef void (GLAPIENTRY FNGLALPHAFRAGMENTOP3ATIPROC) (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);
typedef FNGLALPHAFRAGMENTOP3ATIPROC *PFNGLALPHAFRAGMENTOP3ATIPROC;
typedef void (GLAPIENTRY FNGLSETFRAGMENTSHADERCONSTANTATIPROC) (GLuint dst, const GLfloat *value);
typedef FNGLSETFRAGMENTSHADERCONSTANTATIPROC *PFNGLSETFRAGMENTSHADERCONSTANTATIPROC;
#endif

#ifndef GL_ATI_pn_triangles
#define GL_ATI_pn_triangles 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glPNTrianglesiATI (GLenum, GLint);
GLAPI void GLAPIENTRY glPNTrianglesfATI (GLenum, GLfloat);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLPNTRIANGLESIATIPROC) (GLenum pname, GLint param);
typedef FNGLPNTRIANGLESIATIPROC *PFNGLPNTRIANGLESIATIPROC;
typedef void (GLAPIENTRY FNGLPNTRIANGLESFATIPROC) (GLenum pname, GLfloat param);
typedef FNGLPNTRIANGLESFATIPROC *PFNGLPNTRIANGLESFATIPROC;
#endif

#ifndef GL_ATI_vertex_array_object
#define GL_ATI_vertex_array_object 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI GLuint GLAPIENTRY glNewObjectBufferATI (GLsizei, const GLvoid *, GLenum);
GLAPI GLboolean GLAPIENTRY glIsObjectBufferATI (GLuint);
GLAPI void GLAPIENTRY glUpdateObjectBufferATI (GLuint, GLuint, GLsizei, const GLvoid *, GLenum);
GLAPI void GLAPIENTRY glGetObjectBufferfvATI (GLuint, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetObjectBufferivATI (GLuint, GLenum, GLint *);
GLAPI void GLAPIENTRY glDeleteObjectBufferATI (GLuint);
GLAPI void GLAPIENTRY glArrayObjectATI (GLenum, GLint, GLenum, GLsizei, GLuint, GLuint);
GLAPI void GLAPIENTRY glGetArrayObjectfvATI (GLenum, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetArrayObjectivATI (GLenum, GLenum, GLint *);
GLAPI void GLAPIENTRY glVariantArrayObjectATI (GLuint, GLenum, GLsizei, GLuint, GLuint);
GLAPI void GLAPIENTRY glGetVariantArrayObjectfvATI (GLuint, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetVariantArrayObjectivATI (GLuint, GLenum, GLint *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef GLuint (GLAPIENTRY FNGLNEWOBJECTBUFFERATIPROC) (GLsizei size, const GLvoid *pointer, GLenum usage);
typedef FNGLNEWOBJECTBUFFERATIPROC *PFNGLNEWOBJECTBUFFERATIPROC;
typedef GLboolean (GLAPIENTRY FNGLISOBJECTBUFFERATIPROC) (GLuint buffer);
typedef FNGLISOBJECTBUFFERATIPROC *PFNGLISOBJECTBUFFERATIPROC;
typedef void (GLAPIENTRY FNGLUPDATEOBJECTBUFFERATIPROC) (GLuint buffer, GLuint offset, GLsizei size, const GLvoid *pointer, GLenum preserve);
typedef FNGLUPDATEOBJECTBUFFERATIPROC *PFNGLUPDATEOBJECTBUFFERATIPROC;
typedef void (GLAPIENTRY FNGLGETOBJECTBUFFERFVATIPROC) (GLuint buffer, GLenum pname, GLfloat *params);
typedef FNGLGETOBJECTBUFFERFVATIPROC *PFNGLGETOBJECTBUFFERFVATIPROC;
typedef void (GLAPIENTRY FNGLGETOBJECTBUFFERIVATIPROC) (GLuint buffer, GLenum pname, GLint *params);
typedef FNGLGETOBJECTBUFFERIVATIPROC *PFNGLGETOBJECTBUFFERIVATIPROC;
typedef void (GLAPIENTRY FNGLDELETEOBJECTBUFFERATIPROC) (GLuint buffer);
typedef FNGLDELETEOBJECTBUFFERATIPROC *PFNGLDELETEOBJECTBUFFERATIPROC;
typedef void (GLAPIENTRY FNGLARRAYOBJECTATIPROC) (GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
typedef FNGLARRAYOBJECTATIPROC *PFNGLARRAYOBJECTATIPROC;
typedef void (GLAPIENTRY FNGLGETARRAYOBJECTFVATIPROC) (GLenum array, GLenum pname, GLfloat *params);
typedef FNGLGETARRAYOBJECTFVATIPROC *PFNGLGETARRAYOBJECTFVATIPROC;
typedef void (GLAPIENTRY FNGLGETARRAYOBJECTIVATIPROC) (GLenum array, GLenum pname, GLint *params);
typedef FNGLGETARRAYOBJECTIVATIPROC *PFNGLGETARRAYOBJECTIVATIPROC;
typedef void (GLAPIENTRY FNGLVARIANTARRAYOBJECTATIPROC) (GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
typedef FNGLVARIANTARRAYOBJECTATIPROC *PFNGLVARIANTARRAYOBJECTATIPROC;
typedef void (GLAPIENTRY FNGLGETVARIANTARRAYOBJECTFVATIPROC) (GLuint id, GLenum pname, GLfloat *params);
typedef FNGLGETVARIANTARRAYOBJECTFVATIPROC *PFNGLGETVARIANTARRAYOBJECTFVATIPROC;
typedef void (GLAPIENTRY FNGLGETVARIANTARRAYOBJECTIVATIPROC) (GLuint id, GLenum pname, GLint *params);
typedef FNGLGETVARIANTARRAYOBJECTIVATIPROC *PFNGLGETVARIANTARRAYOBJECTIVATIPROC;
#endif

#ifndef GL_EXT_vertex_shader
#define GL_EXT_vertex_shader 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glBeginVertexShaderEXT (void);
GLAPI void GLAPIENTRY glEndVertexShaderEXT (void);
GLAPI void GLAPIENTRY glBindVertexShaderEXT (GLuint);
GLAPI GLuint GLAPIENTRY glGenVertexShadersEXT (GLuint);
GLAPI void GLAPIENTRY glDeleteVertexShaderEXT (GLuint);
GLAPI void GLAPIENTRY glShaderOp1EXT (GLenum, GLuint, GLuint);
GLAPI void GLAPIENTRY glShaderOp2EXT (GLenum, GLuint, GLuint, GLuint);
GLAPI void GLAPIENTRY glShaderOp3EXT (GLenum, GLuint, GLuint, GLuint, GLuint);
GLAPI void GLAPIENTRY glSwizzleEXT (GLuint, GLuint, GLenum, GLenum, GLenum, GLenum);
GLAPI void GLAPIENTRY glWriteMaskEXT (GLuint, GLuint, GLenum, GLenum, GLenum, GLenum);
GLAPI void GLAPIENTRY glInsertComponentEXT (GLuint, GLuint, GLuint);
GLAPI void GLAPIENTRY glExtractComponentEXT (GLuint, GLuint, GLuint);
GLAPI GLuint GLAPIENTRY glGenSymbolsEXT (GLenum, GLenum, GLenum, GLuint);
GLAPI void GLAPIENTRY glSetInvariantEXT (GLuint, GLenum, const void *);
GLAPI void GLAPIENTRY glSetLocalConstantEXT (GLuint, GLenum, const void *);
GLAPI void GLAPIENTRY glVariantbvEXT (GLuint, const GLbyte *);
GLAPI void GLAPIENTRY glVariantsvEXT (GLuint, const GLshort *);
GLAPI void GLAPIENTRY glVariantivEXT (GLuint, const GLint *);
GLAPI void GLAPIENTRY glVariantfvEXT (GLuint, const GLfloat *);
GLAPI void GLAPIENTRY glVariantdvEXT (GLuint, const GLdouble *);
GLAPI void GLAPIENTRY glVariantubvEXT (GLuint, const GLubyte *);
GLAPI void GLAPIENTRY glVariantusvEXT (GLuint, const GLushort *);
GLAPI void GLAPIENTRY glVariantuivEXT (GLuint, const GLuint *);
GLAPI void GLAPIENTRY glVariantPointerEXT (GLuint, GLenum, GLuint, const void *);
GLAPI void GLAPIENTRY glEnableVariantClientStateEXT (GLuint);
GLAPI void GLAPIENTRY glDisableVariantClientStateEXT (GLuint);
GLAPI GLuint GLAPIENTRY glBindLightParameterEXT (GLenum, GLenum);
GLAPI GLuint GLAPIENTRY glBindMaterialParameterEXT (GLenum, GLenum);
GLAPI GLuint GLAPIENTRY glBindTexGenParameterEXT (GLenum, GLenum, GLenum);
GLAPI GLuint GLAPIENTRY glBindTextureUnitParameterEXT (GLenum, GLenum);
GLAPI GLuint GLAPIENTRY glBindParameterEXT (GLenum);
GLAPI GLboolean GLAPIENTRY glIsVariantEnabledEXT (GLuint, GLenum);
GLAPI void GLAPIENTRY glGetVariantBooleanvEXT (GLuint, GLenum, GLboolean *);
GLAPI void GLAPIENTRY glGetVariantIntegervEXT (GLuint, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetVariantFloatvEXT (GLuint, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetVariantPointervEXT (GLuint, GLenum, GLvoid* *);
GLAPI void GLAPIENTRY glGetInvariantBooleanvEXT (GLuint, GLenum, GLboolean *);
GLAPI void GLAPIENTRY glGetInvariantIntegervEXT (GLuint, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetInvariantFloatvEXT (GLuint, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetLocalConstantBooleanvEXT (GLuint, GLenum, GLboolean *);
GLAPI void GLAPIENTRY glGetLocalConstantIntegervEXT (GLuint, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetLocalConstantFloatvEXT (GLuint, GLenum, GLfloat *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLBEGINVERTEXSHADEREXTPROC) (void);
typedef FNGLBEGINVERTEXSHADEREXTPROC *PFNGLBEGINVERTEXSHADEREXTPROC;
typedef void (GLAPIENTRY FNGLENDVERTEXSHADEREXTPROC) (void);
typedef FNGLENDVERTEXSHADEREXTPROC *PFNGLENDVERTEXSHADEREXTPROC;
typedef void (GLAPIENTRY FNGLBINDVERTEXSHADEREXTPROC) (GLuint id);
typedef FNGLBINDVERTEXSHADEREXTPROC *PFNGLBINDVERTEXSHADEREXTPROC;
typedef GLuint (GLAPIENTRY FNGLGENVERTEXSHADERSEXTPROC) (GLuint range);
typedef FNGLGENVERTEXSHADERSEXTPROC *PFNGLGENVERTEXSHADERSEXTPROC;
typedef void (GLAPIENTRY FNGLDELETEVERTEXSHADEREXTPROC) (GLuint id);
typedef FNGLDELETEVERTEXSHADEREXTPROC *PFNGLDELETEVERTEXSHADEREXTPROC;
typedef void (GLAPIENTRY FNGLSHADEROP1EXTPROC) (GLenum op, GLuint res, GLuint arg1);
typedef FNGLSHADEROP1EXTPROC *PFNGLSHADEROP1EXTPROC;
typedef void (GLAPIENTRY FNGLSHADEROP2EXTPROC) (GLenum op, GLuint res, GLuint arg1, GLuint arg2);
typedef FNGLSHADEROP2EXTPROC *PFNGLSHADEROP2EXTPROC;
typedef void (GLAPIENTRY FNGLSHADEROP3EXTPROC) (GLenum op, GLuint res, GLuint arg1, GLuint arg2, GLuint arg3);
typedef FNGLSHADEROP3EXTPROC *PFNGLSHADEROP3EXTPROC;
typedef void (GLAPIENTRY FNGLSWIZZLEEXTPROC) (GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW);
typedef FNGLSWIZZLEEXTPROC *PFNGLSWIZZLEEXTPROC;
typedef void (GLAPIENTRY FNGLWRITEMASKEXTPROC) (GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW);
typedef FNGLWRITEMASKEXTPROC *PFNGLWRITEMASKEXTPROC;
typedef void (GLAPIENTRY FNGLINSERTCOMPONENTEXTPROC) (GLuint res, GLuint src, GLuint num);
typedef FNGLINSERTCOMPONENTEXTPROC *PFNGLINSERTCOMPONENTEXTPROC;
typedef void (GLAPIENTRY FNGLEXTRACTCOMPONENTEXTPROC) (GLuint res, GLuint src, GLuint num);
typedef FNGLEXTRACTCOMPONENTEXTPROC *PFNGLEXTRACTCOMPONENTEXTPROC;
typedef GLuint (GLAPIENTRY FNGLGENSYMBOLSEXTPROC) (GLenum datatype, GLenum storagetype, GLenum range, GLuint components);
typedef FNGLGENSYMBOLSEXTPROC *PFNGLGENSYMBOLSEXTPROC;
typedef void (GLAPIENTRY FNGLSETINVARIANTEXTPROC) (GLuint id, GLenum type, const void *addr);
typedef FNGLSETINVARIANTEXTPROC *PFNGLSETINVARIANTEXTPROC;
typedef void (GLAPIENTRY FNGLSETLOCALCONSTANTEXTPROC) (GLuint id, GLenum type, const void *addr);
typedef FNGLSETLOCALCONSTANTEXTPROC *PFNGLSETLOCALCONSTANTEXTPROC;
typedef void (GLAPIENTRY FNGLVARIANTBVEXTPROC) (GLuint id, const GLbyte *addr);
typedef FNGLVARIANTBVEXTPROC *PFNGLVARIANTBVEXTPROC;
typedef void (GLAPIENTRY FNGLVARIANTSVEXTPROC) (GLuint id, const GLshort *addr);
typedef FNGLVARIANTSVEXTPROC *PFNGLVARIANTSVEXTPROC;
typedef void (GLAPIENTRY FNGLVARIANTIVEXTPROC) (GLuint id, const GLint *addr);
typedef FNGLVARIANTIVEXTPROC *PFNGLVARIANTIVEXTPROC;
typedef void (GLAPIENTRY FNGLVARIANTFVEXTPROC) (GLuint id, const GLfloat *addr);
typedef FNGLVARIANTFVEXTPROC *PFNGLVARIANTFVEXTPROC;
typedef void (GLAPIENTRY FNGLVARIANTDVEXTPROC) (GLuint id, const GLdouble *addr);
typedef FNGLVARIANTDVEXTPROC *PFNGLVARIANTDVEXTPROC;
typedef void (GLAPIENTRY FNGLVARIANTUBVEXTPROC) (GLuint id, const GLubyte *addr);
typedef FNGLVARIANTUBVEXTPROC *PFNGLVARIANTUBVEXTPROC;
typedef void (GLAPIENTRY FNGLVARIANTUSVEXTPROC) (GLuint id, const GLushort *addr);
typedef FNGLVARIANTUSVEXTPROC *PFNGLVARIANTUSVEXTPROC;
typedef void (GLAPIENTRY FNGLVARIANTUIVEXTPROC) (GLuint id, const GLuint *addr);
typedef FNGLVARIANTUIVEXTPROC *PFNGLVARIANTUIVEXTPROC;
typedef void (GLAPIENTRY FNGLVARIANTPOINTEREXTPROC) (GLuint id, GLenum type, GLuint stride, const void *addr);
typedef FNGLVARIANTPOINTEREXTPROC *PFNGLVARIANTPOINTEREXTPROC;
typedef void (GLAPIENTRY FNGLENABLEVARIANTCLIENTSTATEEXTPROC) (GLuint id);
typedef FNGLENABLEVARIANTCLIENTSTATEEXTPROC *PFNGLENABLEVARIANTCLIENTSTATEEXTPROC;
typedef void (GLAPIENTRY FNGLDISABLEVARIANTCLIENTSTATEEXTPROC) (GLuint id);
typedef FNGLDISABLEVARIANTCLIENTSTATEEXTPROC *PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC;
typedef GLuint (GLAPIENTRY FNGLBINDLIGHTPARAMETEREXTPROC) (GLenum light, GLenum value);
typedef FNGLBINDLIGHTPARAMETEREXTPROC *PFNGLBINDLIGHTPARAMETEREXTPROC;
typedef GLuint (GLAPIENTRY FNGLBINDMATERIALPARAMETEREXTPROC) (GLenum face, GLenum value);
typedef FNGLBINDMATERIALPARAMETEREXTPROC *PFNGLBINDMATERIALPARAMETEREXTPROC;
typedef GLuint (GLAPIENTRY FNGLBINDTEXGENPARAMETEREXTPROC) (GLenum unit, GLenum coord, GLenum value);
typedef FNGLBINDTEXGENPARAMETEREXTPROC *PFNGLBINDTEXGENPARAMETEREXTPROC;
typedef GLuint (GLAPIENTRY FNGLBINDTEXTUREUNITPARAMETEREXTPROC) (GLenum unit, GLenum value);
typedef FNGLBINDTEXTUREUNITPARAMETEREXTPROC *PFNGLBINDTEXTUREUNITPARAMETEREXTPROC;
typedef GLuint (GLAPIENTRY FNGLBINDPARAMETEREXTPROC) (GLenum value);
typedef FNGLBINDPARAMETEREXTPROC *PFNGLBINDPARAMETEREXTPROC;
typedef GLboolean (GLAPIENTRY FNGLISVARIANTENABLEDEXTPROC) (GLuint id, GLenum cap);
typedef FNGLISVARIANTENABLEDEXTPROC *PFNGLISVARIANTENABLEDEXTPROC;
typedef void (GLAPIENTRY FNGLGETVARIANTBOOLEANVEXTPROC) (GLuint id, GLenum value, GLboolean *data);
typedef FNGLGETVARIANTBOOLEANVEXTPROC *PFNGLGETVARIANTBOOLEANVEXTPROC;
typedef void (GLAPIENTRY FNGLGETVARIANTINTEGERVEXTPROC) (GLuint id, GLenum value, GLint *data);
typedef FNGLGETVARIANTINTEGERVEXTPROC *PFNGLGETVARIANTINTEGERVEXTPROC;
typedef void (GLAPIENTRY FNGLGETVARIANTFLOATVEXTPROC) (GLuint id, GLenum value, GLfloat *data);
typedef FNGLGETVARIANTFLOATVEXTPROC *PFNGLGETVARIANTFLOATVEXTPROC;
typedef void (GLAPIENTRY FNGLGETVARIANTPOINTERVEXTPROC) (GLuint id, GLenum value, GLvoid* *data);
typedef FNGLGETVARIANTPOINTERVEXTPROC *PFNGLGETVARIANTPOINTERVEXTPROC;
typedef void (GLAPIENTRY FNGLGETINVARIANTBOOLEANVEXTPROC) (GLuint id, GLenum value, GLboolean *data);
typedef FNGLGETINVARIANTBOOLEANVEXTPROC *PFNGLGETINVARIANTBOOLEANVEXTPROC;
typedef void (GLAPIENTRY FNGLGETINVARIANTINTEGERVEXTPROC) (GLuint id, GLenum value, GLint *data);
typedef FNGLGETINVARIANTINTEGERVEXTPROC *PFNGLGETINVARIANTINTEGERVEXTPROC;
typedef void (GLAPIENTRY FNGLGETINVARIANTFLOATVEXTPROC) (GLuint id, GLenum value, GLfloat *data);
typedef FNGLGETINVARIANTFLOATVEXTPROC *PFNGLGETINVARIANTFLOATVEXTPROC;
typedef void (GLAPIENTRY FNGLGETLOCALCONSTANTBOOLEANVEXTPROC) (GLuint id, GLenum value, GLboolean *data);
typedef FNGLGETLOCALCONSTANTBOOLEANVEXTPROC *PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC;
typedef void (GLAPIENTRY FNGLGETLOCALCONSTANTINTEGERVEXTPROC) (GLuint id, GLenum value, GLint *data);
typedef FNGLGETLOCALCONSTANTINTEGERVEXTPROC *PFNGLGETLOCALCONSTANTINTEGERVEXTPROC;
typedef void (GLAPIENTRY FNGLGETLOCALCONSTANTFLOATVEXTPROC) (GLuint id, GLenum value, GLfloat *data);
typedef FNGLGETLOCALCONSTANTFLOATVEXTPROC *PFNGLGETLOCALCONSTANTFLOATVEXTPROC;
#endif

#ifndef GL_ATI_vertex_streams
#define GL_ATI_vertex_streams 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glVertexStream1sATI (GLenum, GLshort);
GLAPI void GLAPIENTRY glVertexStream1svATI (GLenum, const GLshort *);
GLAPI void GLAPIENTRY glVertexStream1iATI (GLenum, GLint);
GLAPI void GLAPIENTRY glVertexStream1ivATI (GLenum, const GLint *);
GLAPI void GLAPIENTRY glVertexStream1fATI (GLenum, GLfloat);
GLAPI void GLAPIENTRY glVertexStream1fvATI (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glVertexStream1dATI (GLenum, GLdouble);
GLAPI void GLAPIENTRY glVertexStream1dvATI (GLenum, const GLdouble *);
GLAPI void GLAPIENTRY glVertexStream2sATI (GLenum, GLshort, GLshort);
GLAPI void GLAPIENTRY glVertexStream2svATI (GLenum, const GLshort *);
GLAPI void GLAPIENTRY glVertexStream2iATI (GLenum, GLint, GLint);
GLAPI void GLAPIENTRY glVertexStream2ivATI (GLenum, const GLint *);
GLAPI void GLAPIENTRY glVertexStream2fATI (GLenum, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glVertexStream2fvATI (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glVertexStream2dATI (GLenum, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glVertexStream2dvATI (GLenum, const GLdouble *);
GLAPI void GLAPIENTRY glVertexStream3sATI (GLenum, GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glVertexStream3svATI (GLenum, const GLshort *);
GLAPI void GLAPIENTRY glVertexStream3iATI (GLenum, GLint, GLint, GLint);
GLAPI void GLAPIENTRY glVertexStream3ivATI (GLenum, const GLint *);
GLAPI void GLAPIENTRY glVertexStream3fATI (GLenum, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glVertexStream3fvATI (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glVertexStream3dATI (GLenum, GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glVertexStream3dvATI (GLenum, const GLdouble *);
GLAPI void GLAPIENTRY glVertexStream4sATI (GLenum, GLshort, GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glVertexStream4svATI (GLenum, const GLshort *);
GLAPI void GLAPIENTRY glVertexStream4iATI (GLenum, GLint, GLint, GLint, GLint);
GLAPI void GLAPIENTRY glVertexStream4ivATI (GLenum, const GLint *);
GLAPI void GLAPIENTRY glVertexStream4fATI (GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glVertexStream4fvATI (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glVertexStream4dATI (GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glVertexStream4dvATI (GLenum, const GLdouble *);
GLAPI void GLAPIENTRY glNormalStream3bATI (GLenum, GLbyte, GLbyte, GLbyte);
GLAPI void GLAPIENTRY glNormalStream3bvATI (GLenum, const GLbyte *);
GLAPI void GLAPIENTRY glNormalStream3sATI (GLenum, GLshort, GLshort, GLshort);
GLAPI void GLAPIENTRY glNormalStream3svATI (GLenum, const GLshort *);
GLAPI void GLAPIENTRY glNormalStream3iATI (GLenum, GLint, GLint, GLint);
GLAPI void GLAPIENTRY glNormalStream3ivATI (GLenum, const GLint *);
GLAPI void GLAPIENTRY glNormalStream3fATI (GLenum, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glNormalStream3fvATI (GLenum, const GLfloat *);
GLAPI void GLAPIENTRY glNormalStream3dATI (GLenum, GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glNormalStream3dvATI (GLenum, const GLdouble *);
GLAPI void GLAPIENTRY glClientActiveVertexStreamATI (GLenum);
GLAPI void GLAPIENTRY glVertexBlendEnviATI (GLenum, GLint);
GLAPI void GLAPIENTRY glVertexBlendEnvfATI (GLenum, GLfloat);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLVERTEXSTREAM1SATIPROC) (GLenum stream, GLshort x);
typedef FNGLVERTEXSTREAM1SATIPROC *PFNGLVERTEXSTREAM1SATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM1SVATIPROC) (GLenum stream, const GLshort *coords);
typedef FNGLVERTEXSTREAM1SVATIPROC *PFNGLVERTEXSTREAM1SVATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM1IATIPROC) (GLenum stream, GLint x);
typedef FNGLVERTEXSTREAM1IATIPROC *PFNGLVERTEXSTREAM1IATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM1IVATIPROC) (GLenum stream, const GLint *coords);
typedef FNGLVERTEXSTREAM1IVATIPROC *PFNGLVERTEXSTREAM1IVATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM1FATIPROC) (GLenum stream, GLfloat x);
typedef FNGLVERTEXSTREAM1FATIPROC *PFNGLVERTEXSTREAM1FATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM1FVATIPROC) (GLenum stream, const GLfloat *coords);
typedef FNGLVERTEXSTREAM1FVATIPROC *PFNGLVERTEXSTREAM1FVATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM1DATIPROC) (GLenum stream, GLdouble x);
typedef FNGLVERTEXSTREAM1DATIPROC *PFNGLVERTEXSTREAM1DATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM1DVATIPROC) (GLenum stream, const GLdouble *coords);
typedef FNGLVERTEXSTREAM1DVATIPROC *PFNGLVERTEXSTREAM1DVATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM2SATIPROC) (GLenum stream, GLshort x, GLshort y);
typedef FNGLVERTEXSTREAM2SATIPROC *PFNGLVERTEXSTREAM2SATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM2SVATIPROC) (GLenum stream, const GLshort *coords);
typedef FNGLVERTEXSTREAM2SVATIPROC *PFNGLVERTEXSTREAM2SVATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM2IATIPROC) (GLenum stream, GLint x, GLint y);
typedef FNGLVERTEXSTREAM2IATIPROC *PFNGLVERTEXSTREAM2IATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM2IVATIPROC) (GLenum stream, const GLint *coords);
typedef FNGLVERTEXSTREAM2IVATIPROC *PFNGLVERTEXSTREAM2IVATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM2FATIPROC) (GLenum stream, GLfloat x, GLfloat y);
typedef FNGLVERTEXSTREAM2FATIPROC *PFNGLVERTEXSTREAM2FATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM2FVATIPROC) (GLenum stream, const GLfloat *coords);
typedef FNGLVERTEXSTREAM2FVATIPROC *PFNGLVERTEXSTREAM2FVATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM2DATIPROC) (GLenum stream, GLdouble x, GLdouble y);
typedef FNGLVERTEXSTREAM2DATIPROC *PFNGLVERTEXSTREAM2DATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM2DVATIPROC) (GLenum stream, const GLdouble *coords);
typedef FNGLVERTEXSTREAM2DVATIPROC *PFNGLVERTEXSTREAM2DVATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM3SATIPROC) (GLenum stream, GLshort x, GLshort y, GLshort z);
typedef FNGLVERTEXSTREAM3SATIPROC *PFNGLVERTEXSTREAM3SATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM3SVATIPROC) (GLenum stream, const GLshort *coords);
typedef FNGLVERTEXSTREAM3SVATIPROC *PFNGLVERTEXSTREAM3SVATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM3IATIPROC) (GLenum stream, GLint x, GLint y, GLint z);
typedef FNGLVERTEXSTREAM3IATIPROC *PFNGLVERTEXSTREAM3IATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM3IVATIPROC) (GLenum stream, const GLint *coords);
typedef FNGLVERTEXSTREAM3IVATIPROC *PFNGLVERTEXSTREAM3IVATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM3FATIPROC) (GLenum stream, GLfloat x, GLfloat y, GLfloat z);
typedef FNGLVERTEXSTREAM3FATIPROC *PFNGLVERTEXSTREAM3FATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM3FVATIPROC) (GLenum stream, const GLfloat *coords);
typedef FNGLVERTEXSTREAM3FVATIPROC *PFNGLVERTEXSTREAM3FVATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM3DATIPROC) (GLenum stream, GLdouble x, GLdouble y, GLdouble z);
typedef FNGLVERTEXSTREAM3DATIPROC *PFNGLVERTEXSTREAM3DATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM3DVATIPROC) (GLenum stream, const GLdouble *coords);
typedef FNGLVERTEXSTREAM3DVATIPROC *PFNGLVERTEXSTREAM3DVATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM4SATIPROC) (GLenum stream, GLshort x, GLshort y, GLshort z, GLshort w);
typedef FNGLVERTEXSTREAM4SATIPROC *PFNGLVERTEXSTREAM4SATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM4SVATIPROC) (GLenum stream, const GLshort *coords);
typedef FNGLVERTEXSTREAM4SVATIPROC *PFNGLVERTEXSTREAM4SVATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM4IATIPROC) (GLenum stream, GLint x, GLint y, GLint z, GLint w);
typedef FNGLVERTEXSTREAM4IATIPROC *PFNGLVERTEXSTREAM4IATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM4IVATIPROC) (GLenum stream, const GLint *coords);
typedef FNGLVERTEXSTREAM4IVATIPROC *PFNGLVERTEXSTREAM4IVATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM4FATIPROC) (GLenum stream, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef FNGLVERTEXSTREAM4FATIPROC *PFNGLVERTEXSTREAM4FATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM4FVATIPROC) (GLenum stream, const GLfloat *coords);
typedef FNGLVERTEXSTREAM4FVATIPROC *PFNGLVERTEXSTREAM4FVATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM4DATIPROC) (GLenum stream, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef FNGLVERTEXSTREAM4DATIPROC *PFNGLVERTEXSTREAM4DATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXSTREAM4DVATIPROC) (GLenum stream, const GLdouble *coords);
typedef FNGLVERTEXSTREAM4DVATIPROC *PFNGLVERTEXSTREAM4DVATIPROC;
typedef void (GLAPIENTRY FNGLNORMALSTREAM3BATIPROC) (GLenum stream, GLbyte nx, GLbyte ny, GLbyte nz);
typedef FNGLNORMALSTREAM3BATIPROC *PFNGLNORMALSTREAM3BATIPROC;
typedef void (GLAPIENTRY FNGLNORMALSTREAM3BVATIPROC) (GLenum stream, const GLbyte *coords);
typedef FNGLNORMALSTREAM3BVATIPROC *PFNGLNORMALSTREAM3BVATIPROC;
typedef void (GLAPIENTRY FNGLNORMALSTREAM3SATIPROC) (GLenum stream, GLshort nx, GLshort ny, GLshort nz);
typedef FNGLNORMALSTREAM3SATIPROC *PFNGLNORMALSTREAM3SATIPROC;
typedef void (GLAPIENTRY FNGLNORMALSTREAM3SVATIPROC) (GLenum stream, const GLshort *coords);
typedef FNGLNORMALSTREAM3SVATIPROC *PFNGLNORMALSTREAM3SVATIPROC;
typedef void (GLAPIENTRY FNGLNORMALSTREAM3IATIPROC) (GLenum stream, GLint nx, GLint ny, GLint nz);
typedef FNGLNORMALSTREAM3IATIPROC *PFNGLNORMALSTREAM3IATIPROC;
typedef void (GLAPIENTRY FNGLNORMALSTREAM3IVATIPROC) (GLenum stream, const GLint *coords);
typedef FNGLNORMALSTREAM3IVATIPROC *PFNGLNORMALSTREAM3IVATIPROC;
typedef void (GLAPIENTRY FNGLNORMALSTREAM3FATIPROC) (GLenum stream, GLfloat nx, GLfloat ny, GLfloat nz);
typedef FNGLNORMALSTREAM3FATIPROC *PFNGLNORMALSTREAM3FATIPROC;
typedef void (GLAPIENTRY FNGLNORMALSTREAM3FVATIPROC) (GLenum stream, const GLfloat *coords);
typedef FNGLNORMALSTREAM3FVATIPROC *PFNGLNORMALSTREAM3FVATIPROC;
typedef void (GLAPIENTRY FNGLNORMALSTREAM3DATIPROC) (GLenum stream, GLdouble nx, GLdouble ny, GLdouble nz);
typedef FNGLNORMALSTREAM3DATIPROC *PFNGLNORMALSTREAM3DATIPROC;
typedef void (GLAPIENTRY FNGLNORMALSTREAM3DVATIPROC) (GLenum stream, const GLdouble *coords);
typedef FNGLNORMALSTREAM3DVATIPROC *PFNGLNORMALSTREAM3DVATIPROC;
typedef void (GLAPIENTRY FNGLCLIENTACTIVEVERTEXSTREAMATIPROC) (GLenum stream);
typedef FNGLCLIENTACTIVEVERTEXSTREAMATIPROC *PFNGLCLIENTACTIVEVERTEXSTREAMATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXBLENDENVIATIPROC) (GLenum pname, GLint param);
typedef FNGLVERTEXBLENDENVIATIPROC *PFNGLVERTEXBLENDENVIATIPROC;
typedef void (GLAPIENTRY FNGLVERTEXBLENDENVFATIPROC) (GLenum pname, GLfloat param);
typedef FNGLVERTEXBLENDENVFATIPROC *PFNGLVERTEXBLENDENVFATIPROC;
#endif

#ifndef GL_ATI_element_array
#define GL_ATI_element_array 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glElementPointerATI (GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glDrawElementArrayATI (GLenum, GLsizei);
GLAPI void GLAPIENTRY glDrawRangeElementArrayATI (GLenum, GLuint, GLuint, GLsizei);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLELEMENTPOINTERATIPROC) (GLenum type, const GLvoid *pointer);
typedef FNGLELEMENTPOINTERATIPROC *PFNGLELEMENTPOINTERATIPROC;
typedef void (GLAPIENTRY FNGLDRAWELEMENTARRAYATIPROC) (GLenum mode, GLsizei count);
typedef FNGLDRAWELEMENTARRAYATIPROC *PFNGLDRAWELEMENTARRAYATIPROC;
typedef void (GLAPIENTRY FNGLDRAWRANGEELEMENTARRAYATIPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count);
typedef FNGLDRAWRANGEELEMENTARRAYATIPROC *PFNGLDRAWRANGEELEMENTARRAYATIPROC;
#endif

#ifndef GL_SUN_mesh_array
#define GL_SUN_mesh_array 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glDrawMeshArraysSUN (GLenum, GLint, GLsizei, GLsizei);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLDRAWMESHARRAYSSUNPROC) (GLenum mode, GLint first, GLsizei count, GLsizei width);
typedef FNGLDRAWMESHARRAYSSUNPROC *PFNGLDRAWMESHARRAYSSUNPROC;
#endif

#ifndef GL_SUN_slice_accum
#define GL_SUN_slice_accum 1
#endif

#ifndef GL_NV_multisample_filter_hint
#define GL_NV_multisample_filter_hint 1
#endif

#ifndef GL_NV_depth_clamp
#define GL_NV_depth_clamp 1
#endif

#ifndef GL_NV_occlusion_query
#define GL_NV_occlusion_query 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glGenOcclusionQueriesNV (GLsizei, GLuint *);
GLAPI void GLAPIENTRY glDeleteOcclusionQueriesNV (GLsizei, const GLuint *);
GLAPI GLboolean GLAPIENTRY glIsOcclusionQueryNV (GLuint);
GLAPI void GLAPIENTRY glBeginOcclusionQueryNV (GLuint);
GLAPI void GLAPIENTRY glEndOcclusionQueryNV (void);
GLAPI void GLAPIENTRY glGetOcclusionQueryivNV (GLuint, GLenum, GLint *);
GLAPI void GLAPIENTRY glGetOcclusionQueryuivNV (GLuint, GLenum, GLuint *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLGENOCCLUSIONQUERIESNVPROC) (GLsizei n, GLuint *ids);
typedef FNGLGENOCCLUSIONQUERIESNVPROC *PFNGLGENOCCLUSIONQUERIESNVPROC;
typedef void (GLAPIENTRY FNGLDELETEOCCLUSIONQUERIESNVPROC) (GLsizei n, const GLuint *ids);
typedef FNGLDELETEOCCLUSIONQUERIESNVPROC *PFNGLDELETEOCCLUSIONQUERIESNVPROC;
typedef GLboolean (GLAPIENTRY FNGLISOCCLUSIONQUERYNVPROC) (GLuint id);
typedef FNGLISOCCLUSIONQUERYNVPROC *PFNGLISOCCLUSIONQUERYNVPROC;
typedef void (GLAPIENTRY FNGLBEGINOCCLUSIONQUERYNVPROC) (GLuint id);
typedef FNGLBEGINOCCLUSIONQUERYNVPROC *PFNGLBEGINOCCLUSIONQUERYNVPROC;
typedef void (GLAPIENTRY FNGLENDOCCLUSIONQUERYNVPROC) (void);
typedef FNGLENDOCCLUSIONQUERYNVPROC *PFNGLENDOCCLUSIONQUERYNVPROC;
typedef void (GLAPIENTRY FNGLGETOCCLUSIONQUERYIVNVPROC) (GLuint id, GLenum pname, GLint *params);
typedef FNGLGETOCCLUSIONQUERYIVNVPROC *PFNGLGETOCCLUSIONQUERYIVNVPROC;
typedef void (GLAPIENTRY FNGLGETOCCLUSIONQUERYUIVNVPROC) (GLuint id, GLenum pname, GLuint *params);
typedef FNGLGETOCCLUSIONQUERYUIVNVPROC *PFNGLGETOCCLUSIONQUERYUIVNVPROC;
#endif

#ifndef GL_NV_point_sprite
#define GL_NV_point_sprite 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glPointParameteriNV (GLenum, GLint);
GLAPI void GLAPIENTRY glPointParameterivNV (GLenum, const GLint *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLPOINTPARAMETERINVPROC) (GLenum pname, GLint param);
typedef FNGLPOINTPARAMETERINVPROC *PFNGLPOINTPARAMETERINVPROC;
typedef void (GLAPIENTRY FNGLPOINTPARAMETERIVNVPROC) (GLenum pname, const GLint *params);
typedef FNGLPOINTPARAMETERIVNVPROC *PFNGLPOINTPARAMETERIVNVPROC;
#endif

#ifndef GL_NV_texture_shader3
#define GL_NV_texture_shader3 1
#endif

#ifndef GL_NV_vertex_program1_1
#define GL_NV_vertex_program1_1 1
#endif

#ifndef GL_EXT_shadow_funcs
#define GL_EXT_shadow_funcs 1
#endif

#ifndef GL_EXT_stencil_two_side
#define GL_EXT_stencil_two_side 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glActiveStencilFaceEXT (GLenum);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GLAPIENTRY FNGLACTIVESTENCILFACEEXTPROC) (GLenum face);
typedef FNGLACTIVESTENCILFACEEXTPROC *PFNGLACTIVESTENCILFACEEXTPROC;
#endif


#ifndef GL_ATI_text_fragment_shader
#define GL_ATI_text_fragment_shader 1
#endif

#ifndef GL_APPLE_client_storage
#define GL_APPLE_client_storage 1
#endif

#ifndef GL_APPLE_element_array
#define GL_APPLE_element_array 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glElementPointerAPPLE (GLenum, const GLvoid *);
GLAPI void GLAPIENTRY glDrawElementArrayAPPLE (GLenum, GLint, GLsizei);
GLAPI void GLAPIENTRY glDrawRangeElementArrayAPPLE (GLenum, GLuint, GLuint, GLint, GLsizei);
GLAPI void GLAPIENTRY glMultiDrawElementArrayAPPLE (GLenum, const GLint *, const GLsizei *, GLsizei);
GLAPI void GLAPIENTRY glMultiDrawRangeElementArrayAPPLE (GLenum, GLuint, GLuint, const GLint *, const GLsizei *, GLsizei);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (APIENTRY * PFNGLELEMENTPOINTERAPPLEPROC) (GLenum type, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLDRAWELEMENTARRAYAPPLEPROC) (GLenum mode, GLint first, GLsizei count);
typedef void (APIENTRY * PFNGLDRAWRANGEELEMENTARRAYAPPLEPROC) (GLenum mode, GLuint start, GLuint end, GLint first, GLsizei count);
typedef void (APIENTRY * PFNGLMULTIDRAWELEMENTARRAYAPPLEPROC) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount);
typedef void (APIENTRY * PFNGLMULTIDRAWRANGEELEMENTARRAYAPPLEPROC) (GLenum mode, GLuint start, GLuint end, const GLint *first, const GLsizei *count, GLsizei primcount);
#endif

#ifndef GL_APPLE_fence
#define GL_APPLE_fence 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glGenFencesAPPLE (GLsizei, GLuint *);
GLAPI void GLAPIENTRY glDeleteFencesAPPLE (GLsizei, const GLuint *);
GLAPI void GLAPIENTRY glSetFenceAPPLE (GLuint);
GLAPI GLboolean GLAPIENTRY glIsFenceAPPLE (GLuint);
GLAPI GLboolean GLAPIENTRY glTestFenceAPPLE (GLuint);
GLAPI void GLAPIENTRY glFinishFenceAPPLE (GLuint);
GLAPI GLboolean GLAPIENTRY glTestObjectAPPLE (GLenum, GLuint);
GLAPI void GLAPIENTRY glFinishObjectAPPLE (GLenum, GLint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (APIENTRY * PFNGLGENFENCESAPPLEPROC) (GLsizei n, GLuint *fences);
typedef void (APIENTRY * PFNGLDELETEFENCESAPPLEPROC) (GLsizei n, const GLuint *fences);
typedef void (APIENTRY * PFNGLSETFENCEAPPLEPROC) (GLuint fence);
typedef GLboolean (APIENTRY * PFNGLISFENCEAPPLEPROC) (GLuint fence);
typedef GLboolean (APIENTRY * PFNGLTESTFENCEAPPLEPROC) (GLuint fence);
typedef void (APIENTRY * PFNGLFINISHFENCEAPPLEPROC) (GLuint fence);
typedef GLboolean (APIENTRY * PFNGLTESTOBJECTAPPLEPROC) (GLenum object, GLuint name);
typedef void (APIENTRY * PFNGLFINISHOBJECTAPPLEPROC) (GLenum object, GLint name);
#endif

#ifndef GL_APPLE_vertex_array_object
#define GL_APPLE_vertex_array_object 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glBindVertexArrayAPPLE (GLuint);
GLAPI void GLAPIENTRY glDeleteVertexArraysAPPLE (GLsizei, const GLuint *);
GLAPI void GLAPIENTRY glGenVertexArraysAPPLE (GLsizei, const GLuint *);
GLAPI GLboolean GLAPIENTRY glIsVertexArrayAPPLE (GLuint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (APIENTRY * PFNGLBINDVERTEXARRAYAPPLEPROC) (GLuint array);
typedef void (APIENTRY * PFNGLDELETEVERTEXARRAYSAPPLEPROC) (GLsizei n, const GLuint *arrays);
typedef void (APIENTRY * PFNGLGENVERTEXARRAYSAPPLEPROC) (GLsizei n, const GLuint *arrays);
typedef GLboolean (APIENTRY * PFNGLISVERTEXARRAYAPPLEPROC) (GLuint array);
#endif

#ifndef GL_APPLE_vertex_array_range
#define GL_APPLE_vertex_array_range 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glVertexArrayRangeAPPLE (GLsizei, GLvoid *);
GLAPI void GLAPIENTRY glFlushVertexArrayRangeAPPLE (GLsizei, GLvoid *);
GLAPI void GLAPIENTRY glVertexArrayParameteriAPPLE (GLenum, GLint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (APIENTRY * PFNGLVERTEXARRAYRANGEAPPLEPROC) (GLsizei length, GLvoid *pointer);
typedef void (APIENTRY * PFNGLFLUSHVERTEXARRAYRANGEAPPLEPROC) (GLsizei length, GLvoid *pointer);
typedef void (APIENTRY * PFNGLVERTEXARRAYPARAMETERIAPPLEPROC) (GLenum pname, GLint param);
#endif

#ifndef GL_APPLE_ycbcr_422
#define GL_APPLE_ycbcr_422 1
#endif

#ifndef GL_S3_s3tc
#define GL_S3_s3tc 1
#endif

#ifndef GL_ATI_draw_buffers
#define GL_ATI_draw_buffers 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glDrawBuffersATI (GLsizei, const GLenum *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (APIENTRY * PFNGLDRAWBUFFERSATIPROC) (GLsizei n, const GLenum *bufs);
#endif

#ifndef GL_ATI_texture_env_combine3
#define GL_ATI_texture_env_combine3 1
#endif

#ifndef GL_ATI_texture_float
#define GL_ATI_texture_float 1
#endif

#ifndef GL_NV_float_buffer
#define GL_NV_float_buffer 1
#endif

#ifndef GL_NV_fragment_program
#define GL_NV_fragment_program 1
/* Some NV_fragment_program entry points are shared with ARB_vertex_program. */
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glProgramNamedParameter4fNV (GLuint, GLsizei, const GLubyte *, GLfloat, GLfloat, GLfloat, GLfloat);
GLAPI void GLAPIENTRY glProgramNamedParameter4dNV (GLuint, GLsizei, const GLubyte *, GLdouble, GLdouble, GLdouble, GLdouble);
GLAPI void GLAPIENTRY glProgramNamedParameter4fvNV (GLuint, GLsizei, const GLubyte *, const GLfloat *);
GLAPI void GLAPIENTRY glProgramNamedParameter4dvNV (GLuint, GLsizei, const GLubyte *, const GLdouble *);
GLAPI void GLAPIENTRY glGetProgramNamedParameterfvNV (GLuint, GLsizei, const GLubyte *, GLfloat *);
GLAPI void GLAPIENTRY glGetProgramNamedParameterdvNV (GLuint, GLsizei, const GLubyte *, GLdouble *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (APIENTRY * PFNGLPROGRAMNAMEDPARAMETER4FNVPROC) (GLuint id, GLsizei len, const GLubyte *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * PFNGLPROGRAMNAMEDPARAMETER4DNVPROC) (GLuint id, GLsizei len, const GLubyte *name, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * PFNGLPROGRAMNAMEDPARAMETER4FVNVPROC) (GLuint id, GLsizei len, const GLubyte *name, const GLfloat *v);
typedef void (APIENTRY * PFNGLPROGRAMNAMEDPARAMETER4DVNVPROC) (GLuint id, GLsizei len, const GLubyte *name, const GLdouble *v);
typedef void (APIENTRY * PFNGLGETPROGRAMNAMEDPARAMETERFVNVPROC) (GLuint id, GLsizei len, const GLubyte *name, GLfloat *params);
typedef void (APIENTRY * PFNGLGETPROGRAMNAMEDPARAMETERDVNVPROC) (GLuint id, GLsizei len, const GLubyte *name, GLdouble *params);
#endif

#ifndef GL_NV_half_float
#define GL_NV_half_float 1
/* GL type for representing NVIDIA "half" floating point type in host memory */
/* Only used by this extension for now; later needs to be moved earlier in glext.h */
typedef unsigned short GLhalfNV;
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glVertex2hNV (GLhalfNV, GLhalfNV);
GLAPI void GLAPIENTRY glVertex2hvNV (const GLhalfNV *);
GLAPI void GLAPIENTRY glVertex3hNV (GLhalfNV, GLhalfNV, GLhalfNV);
GLAPI void GLAPIENTRY glVertex3hvNV (const GLhalfNV *);
GLAPI void GLAPIENTRY glVertex4hNV (GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
GLAPI void GLAPIENTRY glVertex4hvNV (const GLhalfNV *);
GLAPI void GLAPIENTRY glNormal3hNV (GLhalfNV, GLhalfNV, GLhalfNV);
GLAPI void GLAPIENTRY glNormal3hvNV (const GLhalfNV *);
GLAPI void GLAPIENTRY glColor3hNV (GLhalfNV, GLhalfNV, GLhalfNV);
GLAPI void GLAPIENTRY glColor3hvNV (const GLhalfNV *);
GLAPI void GLAPIENTRY glColor4hNV (GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
GLAPI void GLAPIENTRY glColor4hvNV (const GLhalfNV *);
GLAPI void GLAPIENTRY glTexCoord1hNV (GLhalfNV);
GLAPI void GLAPIENTRY glTexCoord1hvNV (const GLhalfNV *);
GLAPI void GLAPIENTRY glTexCoord2hNV (GLhalfNV, GLhalfNV);
GLAPI void GLAPIENTRY glTexCoord2hvNV (const GLhalfNV *);
GLAPI void GLAPIENTRY glTexCoord3hNV (GLhalfNV, GLhalfNV, GLhalfNV);
GLAPI void GLAPIENTRY glTexCoord3hvNV (const GLhalfNV *);
GLAPI void GLAPIENTRY glTexCoord4hNV (GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
GLAPI void GLAPIENTRY glTexCoord4hvNV (const GLhalfNV *);
GLAPI void GLAPIENTRY glMultiTexCoord1hNV (GLenum, GLhalfNV);
GLAPI void GLAPIENTRY glMultiTexCoord1hvNV (GLenum, const GLhalfNV *);
GLAPI void GLAPIENTRY glMultiTexCoord2hNV (GLenum, GLhalfNV, GLhalfNV);
GLAPI void GLAPIENTRY glMultiTexCoord2hvNV (GLenum, const GLhalfNV *);
GLAPI void GLAPIENTRY glMultiTexCoord3hNV (GLenum, GLhalfNV, GLhalfNV, GLhalfNV);
GLAPI void GLAPIENTRY glMultiTexCoord3hvNV (GLenum, const GLhalfNV *);
GLAPI void GLAPIENTRY glMultiTexCoord4hNV (GLenum, GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
GLAPI void GLAPIENTRY glMultiTexCoord4hvNV (GLenum, const GLhalfNV *);
GLAPI void GLAPIENTRY glFogCoordhNV (GLhalfNV);
GLAPI void GLAPIENTRY glFogCoordhvNV (const GLhalfNV *);
GLAPI void GLAPIENTRY glSecondaryColor3hNV (GLhalfNV, GLhalfNV, GLhalfNV);
GLAPI void GLAPIENTRY glSecondaryColor3hvNV (const GLhalfNV *);
GLAPI void GLAPIENTRY glVertexWeighthNV (GLhalfNV);
GLAPI void GLAPIENTRY glVertexWeighthvNV (const GLhalfNV *);
GLAPI void GLAPIENTRY glVertexAttrib1hNV (GLuint, GLhalfNV);
GLAPI void GLAPIENTRY glVertexAttrib1hvNV (GLuint, const GLhalfNV *);
GLAPI void GLAPIENTRY glVertexAttrib2hNV (GLuint, GLhalfNV, GLhalfNV);
GLAPI void GLAPIENTRY glVertexAttrib2hvNV (GLuint, const GLhalfNV *);
GLAPI void GLAPIENTRY glVertexAttrib3hNV (GLuint, GLhalfNV, GLhalfNV, GLhalfNV);
GLAPI void GLAPIENTRY glVertexAttrib3hvNV (GLuint, const GLhalfNV *);
GLAPI void GLAPIENTRY glVertexAttrib4hNV (GLuint, GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
GLAPI void GLAPIENTRY glVertexAttrib4hvNV (GLuint, const GLhalfNV *);
GLAPI void GLAPIENTRY glVertexAttribs1hvNV (GLuint, GLsizei, const GLhalfNV *);
GLAPI void GLAPIENTRY glVertexAttribs2hvNV (GLuint, GLsizei, const GLhalfNV *);
GLAPI void GLAPIENTRY glVertexAttribs3hvNV (GLuint, GLsizei, const GLhalfNV *);
GLAPI void GLAPIENTRY glVertexAttribs4hvNV (GLuint, GLsizei, const GLhalfNV *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (APIENTRY * PFNGLVERTEX2HNVPROC) (GLhalfNV x, GLhalfNV y);
typedef void (APIENTRY * PFNGLVERTEX2HVNVPROC) (const GLhalfNV *v);
typedef void (APIENTRY * PFNGLVERTEX3HNVPROC) (GLhalfNV x, GLhalfNV y, GLhalfNV z);
typedef void (APIENTRY * PFNGLVERTEX3HVNVPROC) (const GLhalfNV *v);
typedef void (APIENTRY * PFNGLVERTEX4HNVPROC) (GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w);
typedef void (APIENTRY * PFNGLVERTEX4HVNVPROC) (const GLhalfNV *v);
typedef void (APIENTRY * PFNGLNORMAL3HNVPROC) (GLhalfNV nx, GLhalfNV ny, GLhalfNV nz);
typedef void (APIENTRY * PFNGLNORMAL3HVNVPROC) (const GLhalfNV *v);
typedef void (APIENTRY * PFNGLCOLOR3HNVPROC) (GLhalfNV red, GLhalfNV green, GLhalfNV blue);
typedef void (APIENTRY * PFNGLCOLOR3HVNVPROC) (const GLhalfNV *v);
typedef void (APIENTRY * PFNGLCOLOR4HNVPROC) (GLhalfNV red, GLhalfNV green, GLhalfNV blue, GLhalfNV alpha);
typedef void (APIENTRY * PFNGLCOLOR4HVNVPROC) (const GLhalfNV *v);
typedef void (APIENTRY * PFNGLTEXCOORD1HNVPROC) (GLhalfNV s);
typedef void (APIENTRY * PFNGLTEXCOORD1HVNVPROC) (const GLhalfNV *v);
typedef void (APIENTRY * PFNGLTEXCOORD2HNVPROC) (GLhalfNV s, GLhalfNV t);
typedef void (APIENTRY * PFNGLTEXCOORD2HVNVPROC) (const GLhalfNV *v);
typedef void (APIENTRY * PFNGLTEXCOORD3HNVPROC) (GLhalfNV s, GLhalfNV t, GLhalfNV r);
typedef void (APIENTRY * PFNGLTEXCOORD3HVNVPROC) (const GLhalfNV *v);
typedef void (APIENTRY * PFNGLTEXCOORD4HNVPROC) (GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q);
typedef void (APIENTRY * PFNGLTEXCOORD4HVNVPROC) (const GLhalfNV *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1HNVPROC) (GLenum target, GLhalfNV s);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1HVNVPROC) (GLenum target, const GLhalfNV *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2HNVPROC) (GLenum target, GLhalfNV s, GLhalfNV t);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2HVNVPROC) (GLenum target, const GLhalfNV *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3HNVPROC) (GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3HVNVPROC) (GLenum target, const GLhalfNV *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4HNVPROC) (GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4HVNVPROC) (GLenum target, const GLhalfNV *v);
typedef void (APIENTRY * PFNGLFOGCOORDHNVPROC) (GLhalfNV fog);
typedef void (APIENTRY * PFNGLFOGCOORDHVNVPROC) (const GLhalfNV *fog);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3HNVPROC) (GLhalfNV red, GLhalfNV green, GLhalfNV blue);
typedef void (APIENTRY * PFNGLSECONDARYCOLOR3HVNVPROC) (const GLhalfNV *v);
typedef void (APIENTRY * PFNGLVERTEXWEIGHTHNVPROC) (GLhalfNV weight);
typedef void (APIENTRY * PFNGLVERTEXWEIGHTHVNVPROC) (const GLhalfNV *weight);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1HNVPROC) (GLuint index, GLhalfNV x);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1HVNVPROC) (GLuint index, const GLhalfNV *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2HNVPROC) (GLuint index, GLhalfNV x, GLhalfNV y);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2HVNVPROC) (GLuint index, const GLhalfNV *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3HNVPROC) (GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3HVNVPROC) (GLuint index, const GLhalfNV *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4HNVPROC) (GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4HVNVPROC) (GLuint index, const GLhalfNV *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS1HVNVPROC) (GLuint index, GLsizei n, const GLhalfNV *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS2HVNVPROC) (GLuint index, GLsizei n, const GLhalfNV *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS3HVNVPROC) (GLuint index, GLsizei n, const GLhalfNV *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS4HVNVPROC) (GLuint index, GLsizei n, const GLhalfNV *v);
#endif

#ifndef GL_NV_pixel_data_range
#define GL_NV_pixel_data_range 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glPixelDataRangeNV (GLenum, GLsizei, GLvoid *);
GLAPI void GLAPIENTRY glFlushPixelDataRangeNV (GLenum);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (APIENTRY * PFNGLPIXELDATARANGENVPROC) (GLenum target, GLsizei length, GLvoid *pointer);
typedef void (APIENTRY * PFNGLFLUSHPIXELDATARANGENVPROC) (GLenum target);
#endif

#ifndef GL_NV_primitive_restart
#define GL_NV_primitive_restart 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glPrimitiveRestartNV (void);
GLAPI void GLAPIENTRY glPrimitiveRestartIndexNV (GLuint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (APIENTRY * PFNGLPRIMITIVERESTARTNVPROC) (void);
typedef void (APIENTRY * PFNGLPRIMITIVERESTARTINDEXNVPROC) (GLuint index);
#endif

#ifndef GL_NV_texture_expand_normal
#define GL_NV_texture_expand_normal 1
#endif

#ifndef GL_NV_vertex_program2
#define GL_NV_vertex_program2 1
#endif

#ifndef GL_ATI_map_object_buffer
#define GL_ATI_map_object_buffer 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI GLvoid* GLAPIENTRY glMapObjectBufferATI (GLuint);
GLAPI void GLAPIENTRY glUnmapObjectBufferATI (GLuint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef GLvoid* (APIENTRY * PFNGLMAPOBJECTBUFFERATIPROC) (GLuint buffer);
typedef void (APIENTRY * PFNGLUNMAPOBJECTBUFFERATIPROC) (GLuint buffer);
#endif

#ifndef GL_ATI_separate_stencil
#define GL_ATI_separate_stencil 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glStencilOpSeparateATI (GLenum, GLenum, GLenum, GLenum);
GLAPI void GLAPIENTRY glStencilFuncSeparateATI (GLenum, GLenum, GLint, GLuint);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (APIENTRY * PFNGLSTENCILOPSEPARATEATIPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
typedef void (APIENTRY * PFNGLSTENCILFUNCSEPARATEATIPROC) (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
#endif

#ifndef GL_ATI_vertex_attrib_array_object
#define GL_ATI_vertex_attrib_array_object 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void GLAPIENTRY glVertexAttribArrayObjectATI (GLuint, GLint, GLenum, GLboolean, GLsizei, GLuint, GLuint);
GLAPI void GLAPIENTRY glGetVertexAttribArrayObjectfvATI (GLuint, GLenum, GLfloat *);
GLAPI void GLAPIENTRY glGetVertexAttribArrayObjectivATI (GLuint, GLenum, GLint *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (APIENTRY * PFNGLVERTEXATTRIBARRAYOBJECTATIPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLuint buffer, GLuint offset);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBARRAYOBJECTFVATIPROC) (GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBARRAYOBJECTIVATIPROC) (GLuint index, GLenum pname, GLint *params);
#endif


#ifdef __cplusplus
}
#endif

#endif
