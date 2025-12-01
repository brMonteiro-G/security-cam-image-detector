// Update preprocess_static to accept avenue_name
std::string preprocess_static(const Mat& frame, const std::string& avenue_name) {
    Mat result = apply_clahe_hsv(frame);
    result = apply_bilateral_filter(result);
    // result = apply_roi(result); // Uncomment if ROI is needed

    long timestamp = chrono::system_clock::to_time_t(chrono::system_clock::now());
    string output_dir = "./resources/images/";
    filesystem::create_directories(output_dir);

    ostringstream filename;
    filename << output_dir
             << "filtered_image_"
             << avenue_name << "_"
             << timestamp
             << ".png";
    string path = filename.str();
    imwrite(path, result);
    return path;
}

// Update test_static_image to accept avenue_name and return the processed path
std::string test_static_image(const std::string& image_path, const std::string& avenue_name) {
    if (!filesystem::exists(image_path)) {
        cout << "Image not found: " << image_path << endl;
        return "";
    }
    Mat frame = imread(image_path);
    if (frame.empty()) {
        cout << "Failed to load image: " << image_path << endl;
        return "";
    }
    std::string processed_path = preprocess_static(frame, avenue_name);

    // Side-by-side display
    Mat processed = imread(processed_path);
    Mat combined;
    hconcat(frame, processed, combined);
    imshow("Original | Processed", combined);

    return processed_path;
}