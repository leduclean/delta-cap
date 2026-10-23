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

/* Post-processing config of the model in Model/, same layout as the one the
 * model zoo generates (gen_h_file.py). Must match the trained model: classes,
 * input size (training/configs) and anchors (postprocessing.yolo_anchors). */

#include "arm_math.h"

/* Model Related Info */
#define POSTPROCESS_TYPE POSTPROCESS_OD_ST_YOLOX_UI

#define COLOR_BGR (0)
#define COLOR_RGB (1)
#define COLOR_MODE COLOR_RGB

/* Test model: ST's pretrained COCO-80 st_yoloxn_d100_w025 at 224, not
 * fine-tuned (training/configs/quantization_coco80_224.yaml). */
#define NB_CLASSES 80
#define CLASSES_TABLE const char* classes_table[NB_CLASSES] = {\
"person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck",\
"boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench",\
"bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra",\
"giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",\
"skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove",\
"skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup",\
"fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich", "orange",\
"broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",\
"potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse",\
"remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink",\
"refrigerator", "book", "clock", "vase", "scissors", "teddy bear",\
"hair drier", "toothbrush"}

/* I/O configuration: 224x224 input, strides 8/16/32 */
#define AI_OD_ST_YOLOX_PP_NB_CLASSES        (NB_CLASSES)
#define AI_OD_ST_YOLOX_PP_L_GRID_WIDTH      (28)
#define AI_OD_ST_YOLOX_PP_L_GRID_HEIGHT     (28)
#define AI_OD_ST_YOLOX_PP_L_NB_INPUT_BOXES  (AI_OD_ST_YOLOX_PP_L_GRID_WIDTH * AI_OD_ST_YOLOX_PP_L_GRID_HEIGHT)
#define AI_OD_ST_YOLOX_PP_M_GRID_WIDTH      (14)
#define AI_OD_ST_YOLOX_PP_M_GRID_HEIGHT     (14)
#define AI_OD_ST_YOLOX_PP_M_NB_INPUT_BOXES  (AI_OD_ST_YOLOX_PP_M_GRID_WIDTH * AI_OD_ST_YOLOX_PP_M_GRID_HEIGHT)
#define AI_OD_ST_YOLOX_PP_S_GRID_WIDTH      (7)
#define AI_OD_ST_YOLOX_PP_S_GRID_HEIGHT     (7)
#define AI_OD_ST_YOLOX_PP_S_NB_INPUT_BOXES  (AI_OD_ST_YOLOX_PP_S_GRID_WIDTH * AI_OD_ST_YOLOX_PP_S_GRID_HEIGHT)

/* Anchors = yolo_anchors [0.5,0.5, 0.07,0.25, 0.23,0.7] x grid size */
#define AI_OD_ST_YOLOX_PP_NB_ANCHORS        (3)
static const float32_t AI_OD_ST_YOLOX_PP_L_ANCHORS[2*AI_OD_ST_YOLOX_PP_NB_ANCHORS] = {14.0f, 14.0f, 1.96f, 7.0f, 6.44f, 19.6f};
static const float32_t AI_OD_ST_YOLOX_PP_M_ANCHORS[2*AI_OD_ST_YOLOX_PP_NB_ANCHORS] = {7.0f, 7.0f, 0.98f, 3.5f, 3.22f, 9.8f};
static const float32_t AI_OD_ST_YOLOX_PP_S_ANCHORS[2*AI_OD_ST_YOLOX_PP_NB_ANCHORS] = {3.5f, 3.5f, 0.49f, 1.75f, 1.61f, 4.9f};

/* --------  Tuning below can be modified by the application --------- */
#define AI_OD_ST_YOLOX_PP_CONF_THRESHOLD    (0.6f)
#define AI_OD_ST_YOLOX_PP_IOU_THRESHOLD     (0.5f)
#define AI_OD_ST_YOLOX_PP_MAX_BOXES_LIMIT   (10)

#endif
