void drawDebugGrid(cv::Mat& frame, int step = 100) {
    cv::Scalar gridColor(100, 100, 100);  // light gray
    int thickness = 1;
    int font = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 0.4;

    // Vertical lines
    for (int x = 0; x < frame.cols; x += step) {
        cv::line(frame, cv::Point(x, 0), cv::Point(x, frame.rows), gridColor, thickness);
        cv::putText(frame, std::to_string(x), cv::Point(x + 2, 15), font, fontScale, gridColor, 1);
    }

    // Horizontal lines
    for (int y = 0; y < frame.rows; y += step) {
        cv::line(frame, cv::Point(0, y), cv::Point(frame.cols, y), gridColor, thickness);
        cv::putText(frame, std::to_string(y), cv::Point(5, y - 2), font, fontScale, gridColor, 1);
    }
}
