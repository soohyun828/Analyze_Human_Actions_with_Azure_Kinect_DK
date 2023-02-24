#pragma comment(lib, "k4a.lib")
#define _CRT_SECURE_NO_WARNINGS
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <k4a/k4a.h>
#include <k4abt.h>
#include <k4arecord/record.h>
#include <k4arecord/playback.h>
#include <k4a/k4atypes.h>

#include <stdio.h>
#include <stdlib.h>

#define VERIFY(result, error)                                                                            \
    if(result != K4A_RESULT_SUCCEEDED)                                                                   \
    {                                                                                                    \
        printf("%s \n - (File: %s, Function: %s, Line: %d)\n", error, __FILE__, __FUNCTION__, __LINE__); \
        exit(1);                                                                                         \
    }  

int main()
{
    //MKV 파일 열기
    k4a_playback_t playback_handle;
    if (k4a_playback_open("m1_5.mkv", &playback_handle) != K4A_RESULT_SUCCEEDED)
    {
        printf("Failed to open recording\n");
        return 1;
    }

    // Open the first plugged in Kinect device
    k4a_device_t device = NULL;

    uint8_t deviceIndex = 0;
    if (K4A_RESULT_SUCCEEDED != k4a_device_open(deviceIndex, &device))
    {
        printf("%d: Failed to open device\n", deviceIndex);
    }
    printf("Success");

    // Configure a stream of 4096x3072 BRGA color data at 15 frames per second
    k4a_device_configuration_t deviceConfig = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    deviceConfig.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
    deviceConfig.color_resolution = K4A_COLOR_RESOLUTION_OFF;
    //k4a_device_start_cameras(device, &deviceConfig);
    deviceConfig.camera_fps = K4A_FRAMES_PER_SECOND_15;
    deviceConfig.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
    deviceConfig.color_resolution = K4A_COLOR_RESOLUTION_3072P;

    k4a_calibration_t sensor_calibration;
    VERIFY(k4a_device_get_calibration(device, deviceConfig.depth_mode, deviceConfig.color_resolution, &sensor_calibration),
        "Get depth camera calibration failed!");

    k4abt_tracker_t tracker = NULL;
    k4abt_tracker_configuration_t tracker_config = K4ABT_TRACKER_CONFIG_DEFAULT;
    if (K4A_FAILED(k4abt_tracker_create(&sensor_calibration, tracker_config, &tracker)))
    {
        printf("Failed to create tracker\n");
        k4a_device_stop_cameras(device);
        k4a_device_close(device);
        return 1;
    }

    // 재생 시작
    k4a_playback_set_color_conversion(playback_handle, K4A_IMAGE_FORMAT_COLOR_BGRA32);
    k4a_playback_set_color_conversion(playback_handle, K4A_IMAGE_FORMAT_DEPTH16);
    k4a_playback_set_color_conversion(playback_handle, K4A_IMAGE_FORMAT_IR16);
    k4a_playback_set_color_conversion(playback_handle, K4A_IMAGE_FORMAT_CUSTOM8);
    k4a_playback_set_color_conversion(playback_handle, K4A_IMAGE_FORMAT_CUSTOM16);
    k4a_playback_set_color_conversion(playback_handle, K4A_IMAGE_FORMAT_CUSTOM);
    k4a_playback_set_color_conversion(playback_handle, K4A_IMAGE_FORMAT_COLOR_MJPG);

    k4a_playback_set_color_conversion(playback_handle, K4A_IMAGE_FORMAT_COLOR_NV12);
    //k4a_playback_set_color_conversion(playback_handle, K4A_IMAGE_FORMAT_DEPTH16);
    k4a_playback_set_color_conversion(playback_handle, K4A_IMAGE_FORMAT_IR16);
    k4a_playback_set_color_conversion(playback_handle, K4A_IMAGE_FORMAT_COLOR_YUY2);

    FILE* fp = fopen("m1_5.txt", "w");

    k4a_capture_t capture;
    k4a_stream_result_t stream_result;

    while ((stream_result = k4a_playback_get_next_capture(playback_handle, &capture)) == K4A_STREAM_RESULT_SUCCEEDED)
    {
        // Body Tracking 실행
        k4a_wait_result_t queue_capture_result = k4abt_tracker_enqueue_capture(tracker, capture, K4A_WAIT_INFINITE);
        k4a_capture_release(capture);
        if (queue_capture_result == K4A_WAIT_RESULT_FAILED)
        {
            printf("Failed to enqueue capture into tracker\n");
            break;
        }
        k4abt_frame_t body_frame = NULL;
        k4a_wait_result_t pop_frame_result = k4abt_tracker_pop_result(tracker, &body_frame, K4A_WAIT_INFINITE);
        if (pop_frame_result == K4A_WAIT_RESULT_SUCCEEDED)
        {
            size_t num_bodies = k4abt_frame_get_num_bodies(body_frame);
            /*printf("Detected bodies:%zu\n", num_bodies);
            fprintf(fp, "Detected bodies:%zu\n", num_bodies);*/
            // 사람의 좌표 정보를 출력합니다.
            for (size_t i = 0; i < num_bodies; i++)
            {
                k4abt_skeleton_t skeleton;
                k4a_result_t result = k4abt_frame_get_body_skeleton(body_frame, i, &skeleton);

                if (result == K4A_RESULT_SUCCEEDED)
                {
                    /*printf("Body ID: %u\n", k4abt_frame_get_body_id(body_frame, i));
                    fprintf(fp, "Body ID: %u\n", k4abt_frame_get_body_id(body_frame, i));
                    printf("\t x \t y \t z \n");
                    fprintf(fp, "\t x \t y \t z \n");*/
                    for (int j = 0; j < static_cast<int>(K4ABT_JOINT_COUNT); j++)
                    {
                        k4a_float3_t position = skeleton.joints[j].position;
                        printf("%d:(%.3f,%.3f,%.3f)\n", j, position.v[0], position.v[1], position.v[2]);
                        fprintf(fp, "%d:(%.3f,%.3f,%.3f)\n", j, position.v[0], position.v[1], position.v[2]);
                    }
                }
            }
        }
    }
    fclose(fp);
    printf("Finished Data Convertion!\n");
    k4a_device_close(device);
}