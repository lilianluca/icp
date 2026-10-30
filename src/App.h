#pragma once
#include <opencv2/opencv.hpp>
#include <vector>


class App {
public:
  App();

  bool init(void);

  std::vector<uchar> lossy_bw_limit(cv::Mat &input_img, size_t size_limit);

  int run(void);

  ~App();

private:
  cv::VideoCapture capture;
};
