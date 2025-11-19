#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>

#include <filesystem>
#include <chrono>
#include <utility>
#include <string>


std::pair<std::string, std::string> ingest_camera() {
    static bool first_time = true;   // <--- stays true only on first call

    const std::string url = "https://cameras.santoandre.sp.gov.br/coi04/ID_655";
    const int camera_id = 655;
    const std::string avenue_name = "Avenida dos Estados";
    const std::string output_dir = "../../resources/images/avenida_dos_estados";

    std::filesystem::create_directories(output_dir);

    cv::VideoCapture cap(url);
    if (!cap.isOpened()) {
        std::cerr << "Error: Unable to open stream\n";
        return {avenue_name, ""};
    }

    cv::Mat frame;

    // ---------- FIRST CALL ONLY ----------
    if (first_time) {
        std::cout << "Preview mode (first call only).\n";
        std::cout << "Press SPACE to capture the current viewpoint.\n";
        std::cout << "Press Q to quit without capturing.\n";

        while (true) {
            cap >> frame;
            if (frame.empty()) {
                std::cerr << "Error: Empty frame\n";
                break;
            }

            cv::imshow("Camera Preview", frame);
            int key = cv::waitKey(1);

            if (key == ' ') {  // SPACE pressed
                break;
            }
            if (key == 'q' || key == 'Q') {
                cap.release();
                cv::destroyAllWindows();
                return {avenue_name, ""};
            }
        }

        cv::destroyAllWindows();
        first_time = false;  // <--- future calls will skip preview
    }
    // ---------- SUBSEQUENT CALLS ----------
    else {
        cap >> frame;  // just grab one frame immediately
        if (frame.empty()) {
            std::cerr << "Error: Empty frame\n";
            return {avenue_name, ""};
        }
    }

    // Save the captured frame
    long ts = std::chrono::duration_cast<std::chrono::seconds>(
                  std::chrono::system_clock::now().time_since_epoch())
                  .count();

    std::string filename =
        output_dir + "/screenshot_" + std::to_string(camera_id) + "_" +
        std::to_string(ts) + ".jpg";

    cv::imwrite(filename, frame);
    std::cout << "Captured: " << filename << "\n";

    cap.release();
    return {avenue_name, filename};
}
