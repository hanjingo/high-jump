#ifndef CAMERA_H
#define CAMERA_H

// #include <opencv2/opencv.hpp>
// #include <string>

// #ifdef __cplusplus
// extern "C" {
// #endif

// // C++ camera wrapper for OpenCV
// namespace libcpp {

// class camera {
// public:
//     camera() : cap_() {}

//     // Open camera by index (default 0)
//     bool open(int index = 0) {
//         return cap_.open(index);
//     }

//     // Open camera by device path or URL
//     bool open(const std::string& device) {
//         return cap_.open(device);
//     }

//     // Check if camera is opened
//     bool is_opened() const {
//         return cap_.isOpened();
//     }

//     // Read a frame from camera
//     bool read(cv::Mat& frame) {
//         return cap_.read(frame);
//     }

//     // Release the camera
//     void release() {
//         cap_.release();
//     }

//     // Set camera property (e.g., CV_CAP_PROP_FRAME_WIDTH)
//     bool set(int prop_id, double value) {
//         return cap_.set(prop_id, value);
//     }

//     // Get camera property
//     double get(int prop_id) const {
//         return cap_.get(prop_id);
//     }

// private:
//     cv::VideoCapture cap_;
// };

// } // namespace libcpp

// #ifdef __cplusplus
// }
// #endif

#endif  // CAMERA_H