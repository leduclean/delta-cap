 /**
 ******************************************************************************
 * @file    app_config.h
 * @author  GPM Application Team
 *
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#ifndef APP_CONFIG
#define APP_CONFIG

/* Post-processing config of the ST model in Model/ (copied from ST
 * GettingStarted-ObjectDetection). Regenerate with the model. */

#include "arm_math.h"

/* Model Related Info */
#define POSTPROCESS_TYPE POSTPROCESS_OD_YOLO_V2_UI

#define COLOR_BGR (0)
#define COLOR_RGB (1)
#define COLOR_MODE COLOR_RGB

#define NB_CLASSES 2
#define CLASSES_TABLE const char* classes_table[NB_CLASSES] = {\
"person",   "not_person"}

/* I/O configuration */
#define AI_OD_YOLOV2_PP_NB_CLASSES        (1)
#define AI_OD_YOLOV2_PP_NB_ANCHORS        (5)
#define AI_OD_YOLOV2_PP_GRID_WIDTH        (7)
#define AI_OD_YOLOV2_PP_GRID_HEIGHT       (7)
#define AI_OD_YOLOV2_PP_NB_INPUT_BOXES    (AI_OD_YOLOV2_PP_GRID_WIDTH * AI_OD_YOLOV2_PP_GRID_HEIGHT)

/* Anchor boxes */
static const float32_t AI_OD_YOLOV2_PP_ANCHORS[2*AI_OD_YOLOV2_PP_NB_ANCHORS] = {
    0.9883000000f,     3.3606000000f,
    2.1194000000f,     5.3759000000f,
    3.0520000000f,     9.1336000000f,
    5.5517000000f,     9.3066000000f,
    9.7260000000f,     11.1422000000f,
  };

/* --------  Tuning below can be modified by the application --------- */
#define AI_OD_YOLOV2_PP_CONF_THRESHOLD    (0.6f)
#define AI_OD_YOLOV2_PP_IOU_THRESHOLD     (0.3f)
#define AI_OD_YOLOV2_PP_MAX_BOXES_LIMIT   (10)


#endif
