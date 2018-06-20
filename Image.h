// Image.h
#ifndef IMAGE_H
#define IMAGE_H




#include "opencv2/highgui/highgui.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>

//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/imgcodecs.hpp"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cstdlib>

#include <list>
#include <vector>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>
#include <tesseract/strngs.h>
#include <zbar.h>


#define int64 int64_I_dont_care
#define uint64 uint64_I_dont_care
#include <my_global.h>
#undef int64
#undef uint64
#include <mysql.h>

#include <dmtx.h>

struct bcResult {
  cv::Point point;
  cv::Rect rect;
  std::string code;
  std::string type;
  bool found;
};

class Image{
public:
  Image();
  ~Image();
  void setImage(cv::Mat mat);
  void showImage();
  void showImage(cv::Mat& src);
  void showImage(cv::Mat& src,int ww);
  void open(const char* filename);

  void setDebug(bool val);
  void setDebugWindow(bool val);
  void setDebugTime(bool val);

  void _debugTime(std::string str);

  void barcode();

  DmtxImage* dmtxImage(cv::Mat &image);
  std::vector<cv::Mat> findDMTXRectangles(cv::Mat &gray);
  cv::Mat largestSimpleContour(cv::Mat& src);
  std::vector<std::string> getCodes();
  cv::Mat getImage();

  void save(std::string name,std::vector<int> params);

private:
  cv::Mat originalImage;
  cv::Mat debugImage;
  cv::Mat processImage;
  cv::Mat reducedImage;
  int showDebugWindowWaitTime;
  bool showDebug;
  bool showDebugWindow;
  bool showDebugTime;
  double debug_last_time;


  const char* fileName;
  void datamatrix(cv::Mat &image);
  bcResult barcode_internal(cv::Mat &part);
  bool is_digits(const std::string &str);
  int subtractMean;
  std::vector<std::string> codes;
};
#endif
