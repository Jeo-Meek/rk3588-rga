cmake_minimum_required(VERSION 3.10)
project(RK3588_RGA_Demo LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-std=c++11 -O0 -Wall")

# RGA
include_directories(/usr/include/rga)
set(RGA_LIBRARY rga)

# OpenCV
include_directories(/home/linaro/inst/opencv-4.5.5/include)
include_directories(/home/linaro/inst/opencv-4.5.5/include/opencv4)
link_directories(/home/linaro/inst/opencv-4.5.5/lib)
set(OpenCV_LIBS libopencv_core.so libopencv_features2d.so libopencv_highgui.so libopencv_imgcodecs.so
        libopencv_imgproc.so libopencv_videoio.so libopencv_video.so libopencv_calib3d.so
        libopencv_dnn.so libopencv_flann.so libopencv_gapi.so libopencv_ml.so libopencv_objdetect.so
        libopencv_photo.so libopencv_stitching.so)

# ffmpeg 4.4
link_directories(/home/linaro/inst/ffmpeg-4.4/lib/)
include_directories(/home/linaro/inst/ffmpeg-4.4/include/)

# mpp
include_directories(/opt/mpp/inc)
include_directories(/opt/mpp/osal/inc)
include_directories(/opt/mpp/mpp/codec/inc)
link_directories(/opt/mpp/build/linux/aarch64/mpp)
set(MPP_LIBRARY rockchip_mpp swscale avutil avcodec avformat)

# fmt (用于格式化输出)
include_directories(/home/linaro/inst/fmt/include)
link_directories(/home/linaro/inst/fmt/build)
set(FMT_LIBRARY fmt)

# 添加可执行文件
add_executable(rga_demo src/main.cpp)

# 链接库
target_link_libraries(rga_demo
        ${RGA_LIBRARY}
        ${OpenCV_LIBS}
        ${MPP_LIBRARY}
        ${FMT_LIBRARY}
        pthread
        dl)