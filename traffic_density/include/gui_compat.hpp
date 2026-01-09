#ifndef GUI_COMPAT_HPP
#define GUI_COMPAT_HPP

// Compatibility header for GUI functions when building without OpenCV highgui module
// This allows code to compile with GUI function calls that are conditionally executed at runtime

#include <opencv2/core.hpp>
#include <string>

// Check if highgui is available
#ifdef HAVE_OPENCV_HIGHGUI
    #include <opencv2/highgui.hpp>
#else
    // Define GUI constants and function stubs when highgui is not available
    namespace cv {
        // Window flags
        constexpr int WINDOW_NORMAL = 0x00000000;
        constexpr int WINDOW_AUTOSIZE = 0x00000001;
        
        // Inline stub functions that do nothing
        inline void namedWindow(const std::string&, int = WINDOW_AUTOSIZE) {}
        inline void destroyWindow(const std::string&) {}
        inline void resizeWindow(const std::string&, int, int) {}
        inline void imshow(const std::string&, const cv::Mat&) {}
        inline int waitKey(int = 0) { return -1; }
    }
#endif

#endif // GUI_COMPAT_HPP
