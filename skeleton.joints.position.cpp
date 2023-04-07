#pragma comment(lib, "k4a.lib")
#define _CRT_SECURE_NO_WARNINGS
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <k4a/k4a.h>
#include <k4abt.h>
#include <k4arecord/record.h>
#include <k4arecord/playback.h>

#include <stdio.h>
#include <stdlib.h>

#define VERIFY(result, error)                                                                            \
    if(result != K4A_RESULT_SUCCEEDED)                                                                   \
    {                                                                                                    \
        printf("%s \n - (File: %s, Function: %s, Line: %d)\n", error, __FILE__, __FUNCTION__, __LINE__); \
        exit(1);                                                                                         \
    }                                                                                                    \

int main()
{
    uint32_t device_count = k4a_device_get_installed_count();

    printf("Found %d connected devices:\n", device_count);

    if (device_count == 0)
    {
        printf("No k4a devices attached!\n");
        return 1;
    }

    // Open the first plugged in Kinect device
    k4a_device_t device = NULL;

    uint8_t deviceIndex = 0;
    if (K4A_RESULT_SUCCEEDED != k4a_device_open(deviceIndex, &device))
    {
        printf("%d: Failed to open device\n", deviceIndex);
    }

    // Get the size of the serial number
    size_t serial_size = 0;
    k4a_device_get_serialnum(device, NULL, &serial_size);

    // Allocate memory for the serial, then acquire it
    char* serial = (char*)(malloc(serial_size));
    k4a_device_get_serialnum(device, serial, &serial_size);
    printf("Opened device: %s\n", serial);
    free(serial);

    // Configure a stream of 4096x3072 BRGA color data at 15 frames per second
    k4a_device_configuration_t deviceConfig = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    deviceConfig.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
    deviceConfig.color_resolution = K4A_COLOR_RESOLUTION_OFF;
    //k4a_device_start_cameras(device, &deviceConfig);
    deviceConfig.camera_fps = K4A_FRAMES_PER_SECOND_15;
    deviceConfig.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
    //deviceConfig.color_resolution = K4A_COLOR_RESOLUTION_3072P;

    // Start the camera with the given configuration
    if (K4A_FAILED(k4a_device_start_cameras(device, &deviceConfig)))
    {
        printf("Failed to start cameras!\n");
        k4a_device_close(device);
        return 1;
    }

    k4a_calibration_t sensor_calibration;
    VERIFY(k4a_device_get_calibration(device, deviceConfig.depth_mode, deviceConfig.color_resolution, &sensor_calibration),
        "Get depth camera calibration failed!");

    k4abt_tracker_t tracker = NULL;
    k4abt_tracker_configuration_t tracker_config = K4ABT_TRACKER_CONFIG_DEFAULT;
    if (K4A_FAILED(k4abt_tracker_create(&sensor_calibration ,tracker_config, &tracker)))
    {
        printf("Failed to create tracker\n");
        k4a_device_stop_cameras(device);
        k4a_device_close(device);
        return 1;
    }

    FILE* fp = fopen("output.txt", "w");

    while (true)
    {
        k4a_capture_t capture = NULL;
        k4a_wait_result_t result = k4a_device_get_capture(device, &capture, K4A_WAIT_INFINITE);
        if (result == K4A_WAIT_RESULT_SUCCEEDED)
        {
            // Body tracking Ã³¸®
            k4abt_frame_t body_frame = NULL;
            if (K4A_SUCCEEDED(k4abt_tracker_enqueue_capture(tracker, capture, K4A_WAIT_INFINITE)) &&
                K4A_SUCCEEDED(k4abt_tracker_pop_result(tracker, &body_frame, K4A_WAIT_INFINITE)))
            {
                // °¢ ½ºÄÌ·¹ÅæÀÇ Á¶ÀÎÆ® ÁÂÇ¥ Ãâ·Â
                uint32_t num_bodies = k4abt_frame_get_num_bodies(body_frame);
                for (uint32_t i = 0; i < num_bodies; i++)
                {
                    k4abt_skeleton_t skeleton;
                    k4abt_frame_get_body_skeleton(body_frame, i, &skeleton);

                    for (int j = 0; j < K4ABT_JOINT_COUNT; j++)
                    {
                        k4a_float3_t position = skeleton.joints[j].position;
                        printf("Joint %d: %f, %f, %f\n", j, position.v[0], position.v[1], position.v[2]);
                        fprintf(fp, "Joint %d: %f, %f, %f\n", j, position.v[0], position.v[1], position.v[2]);
                    }
                }

                k4abt_frame_release(body_frame);
            }

            k4a_capture_release(capture);
        }
    }
    fclose(fp);
    printf("Finished body tracking processing!\n");

    k4abt_tracker_shutdown(tracker);
    k4abt_tracker_destroy(tracker);
    k4a_device_stop_cameras(device);
    k4a_device_close(device);

    return 0;
}