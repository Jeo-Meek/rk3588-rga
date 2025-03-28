#include <iostream>
#include <rga/im2d.hpp>
#include <rga/rga.h>
#include <opencv2/opencv.hpp>

int main() {
    std::cout << "RGA缩放示例程序" << std::endl;
    
    // 打印RGA版本信息
    const char* version = querystring(0);
    std::cout << "RGA驱动版本: " << version << std::endl;
    
    // 加载测试图像
    cv::Mat src_img = cv::imread("../static/demo.jpg");
    if (src_img.empty()) {
        std::cerr << "无法加载测试图像" << std::endl;
        return -1;
    }
    
    // 获取原始图像尺寸
    int src_width = src_img.cols;
    int src_height = src_img.rows;
    std::cout << "原始图像大小: " << src_width << "x" << src_height << std::endl;
    
    // 目标尺寸
    int dst_width = 640;
    int dst_height = 360;
    
    // 使用OpenCV进行预缩小，缩小到一个合理的中间尺寸
    // 计算保持长宽比的中间尺寸，约为目标尺寸的2倍
    double scale = std::min(1280.0 / src_width, 720.0 / src_height);
    int mid_width = static_cast<int>(src_width * scale);
    int mid_height = static_cast<int>(src_height * scale);
    
    // 确保中间尺寸不小于目标尺寸
    mid_width = std::max(mid_width, dst_width);
    mid_height = std::max(mid_height, dst_height);
    
    std::cout << "OpenCV预缩小尺寸: " << mid_width << "x" << mid_height << std::endl;
    
    // 使用OpenCV进行预缩小
    cv::Mat mid_img;
    cv::resize(src_img, mid_img, cv::Size(mid_width, mid_height), 0, 0, cv::INTER_AREA);
    
    // BGR888格式要求宽度16字节对齐
    // 计算对齐后的宽度 (向上取整到16的倍数)
    int aligned_mid_width = (mid_width + 15) & ~15;
    int aligned_dst_width = (dst_width + 15) & ~15;
    
    std::cout << "对齐后的中间图像宽度: " << aligned_mid_width << std::endl;
    std::cout << "对齐后的目标图像宽度: " << aligned_dst_width << std::endl;
    
    // 创建对齐后的中间图像
    cv::Mat aligned_mid_img;
    cv::copyMakeBorder(mid_img, aligned_mid_img, 0, 0, 0, 
                        aligned_mid_width - mid_width, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
    
    // 准备源和目标缓冲区
    void *src_vir_addr = malloc(aligned_mid_width * mid_height * 3); // BGR格式每像素3字节
    void *dst_vir_addr = malloc(aligned_dst_width * dst_height * 3);
    
    if (!src_vir_addr || !dst_vir_addr) {
        std::cerr << "内存分配失败" << std::endl;
        return -1;
    }
    
    // 将OpenCV图像数据复制到源缓冲区
    memcpy(src_vir_addr, aligned_mid_img.data, aligned_mid_img.total() * aligned_mid_img.elemSize());
    
    // 导入缓冲区
    rga_buffer_handle_t src_handle = importbuffer_virtualaddr(src_vir_addr, 
                                                              aligned_mid_width, mid_height, 
                                                              RK_FORMAT_BGR_888);
    rga_buffer_handle_t dst_handle = importbuffer_virtualaddr(dst_vir_addr, 
                                                              aligned_dst_width, dst_height, 
                                                              RK_FORMAT_BGR_888);
    
    if (src_handle <= 0 || dst_handle <= 0) {
        std::cerr << "RGA缓冲区导入失败" << std::endl;
        free(src_vir_addr);
        free(dst_vir_addr);
        return -1;
    }
    
    // 封装RGA缓冲区
    rga_buffer_t src_buffer = wrapbuffer_handle(src_handle, 
                                               mid_width, mid_height, 
                                               RK_FORMAT_BGR_888,
                                               aligned_mid_width, mid_height);
    rga_buffer_t dst_buffer = wrapbuffer_handle(dst_handle, 
                                               dst_width, dst_height, 
                                               RK_FORMAT_BGR_888,
                                               aligned_dst_width, dst_height);
    
    // 执行RGA缩放操作
    IM_STATUS status = imresize(src_buffer, dst_buffer);
    
    if (status != IM_STATUS_SUCCESS) {
        std::cerr << "RGA缩放失败，错误码：" << status << std::endl;
        std::cerr << "错误信息：" << imStrError(status) << std::endl;
        
        // 如果RGA失败，使用OpenCV直接缩放到最终尺寸
        std::cout << "尝试使用OpenCV直接缩放..." << std::endl;
        cv::Mat final_img;
        cv::resize(src_img, final_img, cv::Size(dst_width, dst_height), 0, 0, cv::INTER_AREA);
        cv::imwrite("resized_opencv.jpg", final_img);
        std::cout << "已使用OpenCV完成缩放并保存为resized_opencv.jpg" << std::endl;
    } else {
        std::cout << "RGA缩放成功" << std::endl;
        
        // 创建OpenCV Mat封装目标缓冲区
        cv::Mat aligned_dst_img(dst_height, aligned_dst_width, CV_8UC3, dst_vir_addr);
        
        // 裁剪回原始宽度
        cv::Mat dst_img = aligned_dst_img(cv::Rect(0, 0, dst_width, dst_height));
        
        // 保存结果
        cv::imwrite("resized_rga.jpg", dst_img);
        std::cout << "缩放后的图像尺寸: " << dst_width << "x" << dst_height << std::endl;
        std::cout << "已保存为resized_rga.jpg" << std::endl;
    }
    
    // 释放资源
    releasebuffer_handle(src_handle);
    releasebuffer_handle(dst_handle);
    free(src_vir_addr);
    free(dst_vir_addr);
    
    return 0;
}