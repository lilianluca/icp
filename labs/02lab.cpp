// 02lab.cpp
// LAB 02 - static image
// Task 1: Identify the object using luminance
// Task 2: Find the red cup in static image
// Task 3: Find the red object in camera stream or video

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

// our application class
class App {
public:
    App(); // default constructor, called on app instance definition

    // public methods
    bool init(void);
    int run(void);

    cv::Point2f find_object_luma(cv::Mat& frame);
    cv::Point2f find_object_chroma(cv::Mat& frame);

    void draw_cross_normalized(cv::Mat& img, cv::Point2f center_relative, int size);
    void draw_cross(cv::Mat& img, int x, int y, int size);

    ~App(); //default destructor, called on app instance destruction
protected:
    // camera or video file
    cv::VideoCapture capture;
private:
    // video file used when no camera is available
    static constexpr const char* VIDEO_FILE = "resources/video.mkv";

    // (optional) downscale factor of frames before analysis, 1.0 = full size
    static constexpr double ANALYSIS_SCALE = 1.0;

    // luminance threshold (0..255), pixels with Y >= threshold belong to object
    static constexpr unsigned char LUMA_THRESHOLD = 240;

    // red color in OpenCV HSV (H 0..179, S 0..255, V 0..255)
    // Red hue wraps around 0, so two hue ranges are needed: <0,H_LOW> and <H_HIGH,179>.
    // Without S and V limits, white, gray and black pixels would pass too
    // (hue is meaningless for unsaturated or dark colors).
    static constexpr int RED_H_LOW = 8;
    static constexpr int RED_H_HIGH = 172;
    static constexpr int RED_S_MIN = 150;
    static constexpr int RED_V_MIN = 100;

    // black & white results of the last thresholding, for visualization
    cv::Mat luma_threshold;
    cv::Mat chroma_threshold;
};

App::App()
{
    // default constructor
    // nothing to do here (so far...)
}

bool App::init()
{
    try {
        // Disable OpenCV info messages = display only warning and more severe
        cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);

        // open first available camera, using any API available (autodetect)
        capture = cv::VideoCapture(0, cv::CAP_ANY);

        if (capture.isOpened()) {
            std::cout << "Source: camera 0";
        }
        else {
            // no camera, use video file instead
            capture = cv::VideoCapture(VIDEO_FILE);
            if (!capture.isOpened())
                throw std::runtime_error("No camera and no video file: " + std::string(VIDEO_FILE));
            std::cout << "Source: " << VIDEO_FILE;
        }

        std::cout << ": width=" << capture.get(cv::CAP_PROP_FRAME_WIDTH)
                  << ", height=" << capture.get(cv::CAP_PROP_FRAME_HEIGHT) << '\n';
    }
    catch (std::exception const& e) {
        std::cerr << "Init failed : " << e.what() << std::endl;
        throw;
    }

    return true;
}

// Find object by luminance: convert RGB -> Y, threshold, compute centroid
// of white pixels. Returns normalized coordinates (0..1), or (-1,-1) if
// no pixel passed the threshold.
cv::Point2f App::find_object_luma(cv::Mat& frame)
{
    luma_threshold.create(frame.size(), frame.type());

    long long sum_x = 0;
    long long sum_y = 0;
    long long white_cnt = 0;

    for (int y = 0; y < frame.rows; y++) //y
    {
        for (int x = 0; x < frame.cols; x++) //x
        {
            // load source pixel (OpenCV stores pixels as BGR!)
            cv::Vec3b pixel = frame.at<cv::Vec3b>(y, x);

            // compute grayscale value (JPEG conversion from RGB to Y)
            float Y = 0.299f * pixel[2] + 0.587f * pixel[1] + 0.114f * pixel[0];

            if (Y < LUMA_THRESHOLD) {
                // set output pixel black
                luma_threshold.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
            }
            else {
                // set output pixel white
                luma_threshold.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);

                // update centroid
                sum_x += x;
                sum_y += y;
                white_cnt++;
            }
        }
    }

    if (white_cnt == 0)
        return { -1.0f, -1.0f };

    cv::Point2f center_absolute(static_cast<float>(sum_x) / white_cnt,
                                static_cast<float>(sum_y) / white_cnt);

    return { center_absolute.x / frame.cols, center_absolute.y / frame.rows };
}

// Find red object: convert BGR -> HSV, threshold red range, remove noise,
// compute centroid of white pixels. Returns normalized coordinates (0..1),
// or (-1,-1) if no pixel passed the threshold.
cv::Point2f App::find_object_chroma(cv::Mat& frame)
{
    cv::Mat frame_hsv;
    cv::cvtColor(frame, frame_hsv, cv::COLOR_BGR2HSV);

    // red hue wraps around 0 -> threshold two ranges and join them
    cv::Mat mask_low, mask_high, mask;
    cv::inRange(frame_hsv, cv::Scalar(0, RED_S_MIN, RED_V_MIN), cv::Scalar(RED_H_LOW, 255, 255), mask_low);
    cv::inRange(frame_hsv, cv::Scalar(RED_H_HIGH, RED_S_MIN, RED_V_MIN), cv::Scalar(179, 255, 255), mask_high);
    cv::bitwise_or(mask_low, mask_high, mask);

    // (optional) morphology: opening removes small noise,
    // closing fills small holes in the object
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

    // keep only the largest red blob - other red things in the scene
    // (buttons on the wall panel, ...) would shift the centroid
    cv::Mat labels, stats, centroids;
    int blob_cnt = cv::connectedComponentsWithStats(mask, labels, stats, centroids);
    int largest = 0;  // label 0 is background
    for (int i = 1; i < blob_cnt; i++)
        if (largest == 0 || stats.at<int>(i, cv::CC_STAT_AREA) > stats.at<int>(largest, cv::CC_STAT_AREA))
            largest = i;
    if (largest != 0)
        mask = (labels == largest);

    // keep color copy, so that the red cross is visible
    cv::cvtColor(mask, chroma_threshold, cv::COLOR_GRAY2BGR);

    // find all non-zero (ie. white) pixels in thresholded image
    std::vector<cv::Point> white_pixels;
    cv::findNonZero(mask, white_pixels);

    if (white_pixels.empty())
        return { -1.0f, -1.0f };

    // sum of X, Y coords of white pixels (double, so it cannot overflow)
    cv::Point2d white_accum = std::accumulate(white_pixels.begin(), white_pixels.end(), cv::Point2d(0.0, 0.0),
        [](cv::Point2d acc, cv::Point p) { return acc + cv::Point2d(p); });

    cv::Point2f center_absolute = white_accum / static_cast<double>(white_pixels.size());

    return { center_absolute.x / frame.cols, center_absolute.y / frame.rows };
}

int App::run(void)
{
    try {
        cv::Mat frame, frame_small;
        std::chrono::duration<double> total_time{ 0 };
        long frame_cnt = 0;

        while (true)
        {
            capture.read(frame);
            if (frame.empty()) {
                std::cerr << "Cam disconnected? End of file?\n";
                break;
            }

            //start timer
            auto start = std::chrono::steady_clock::now();

            // (optional) analyze smaller image - normalized result is the same
            if (ANALYSIS_SCALE != 1.0)
                cv::resize(frame, frame_small, cv::Size(), ANALYSIS_SCALE, ANALYSIS_SCALE, cv::INTER_NEAREST);
            else
                frame_small = frame;

            cv::Point2f center_normalized = find_object_chroma(frame_small);

            // how long the computation took?
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            total_time += elapsed_seconds;
            frame_cnt++;

            if (center_normalized.x < 0)
                std::cout << "Object not found";
            else
                std::cout << "Center normalized: " << center_normalized;
            std::cout << ", time: " << elapsed_seconds.count() * 1000.0 << " ms\n";

            // WARNING: the original image MUST NOT be modified. If you want to draw into image,
            // do your own COPY!
            cv::Mat scene_cross;
            frame.copyTo(scene_cross);
            if (center_normalized.x >= 0) {
                draw_cross_normalized(scene_cross, center_normalized, 30);
                draw_cross_normalized(chroma_threshold, center_normalized, 30);
            }
            cv::imshow("scene", scene_cross);
            cv::imshow("threshold", chroma_threshold);

            if (cv::waitKey(1) == 27) // test for ESC key
                break;
        }

        if (frame_cnt > 0)
            std::cout << "Average time per frame: " << total_time.count() / frame_cnt * 1000.0
                      << " ms (" << frame_cnt << " frames)\n";
    }
    catch (std::exception const& e) {
        std::cerr << "App failed : " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

App::~App()
{
    // clean-up
    cv::destroyAllWindows();
    std::cout << "Bye...\n";

    if (capture.isOpened())
        capture.release();
}

// define our application
App app;

// MAIN program function
int main()
{
    if (app.init())
        return app.run();
}

void App::draw_cross(cv::Mat& img, int x, int y, int size)
{
    cv::Point p1(x - size / 2, y);
    cv::Point p2(x + size / 2, y);
    cv::Point p3(x, y - size / 2);
    cv::Point p4(x, y + size / 2);

    cv::line(img, p1, p2, CV_RGB(255, 0, 0), 3);
    cv::line(img, p3, p4, CV_RGB(255, 0, 0), 3);
}

void App::draw_cross_normalized(cv::Mat& img, cv::Point2f center_normalized, int size)
{
    center_normalized.x = std::clamp(center_normalized.x, 0.0f, 1.0f);
    center_normalized.y = std::clamp(center_normalized.y, 0.0f, 1.0f);
    size = std::clamp(size, 1, std::min(img.cols, img.rows));

    cv::Point2f center_absolute(center_normalized.x * img.cols, center_normalized.y * img.rows);

    cv::Point2f p1(center_absolute.x - size / 2, center_absolute.y);
    cv::Point2f p2(center_absolute.x + size / 2, center_absolute.y);
    cv::Point2f p3(center_absolute.x, center_absolute.y - size / 2);
    cv::Point2f p4(center_absolute.x, center_absolute.y + size / 2);

    cv::line(img, p1, p2, CV_RGB(255, 0, 0), 3);
    cv::line(img, p3, p4, CV_RGB(255, 0, 0), 3);
}
