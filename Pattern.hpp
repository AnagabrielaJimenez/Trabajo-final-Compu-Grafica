#ifndef EXAMPLE_MARKERLESS_AR_PATTERN_HPP
#define EXAMPLE_MARKERLESS_AR_PATTERN_HPP

#include "GeometryTypes.hpp"
#include "CameraCalibration.hpp"
#include <opencv2/opencv.hpp>
// Estructura que representa un patrón en una imagen.
struct Pattern
{
  cv::Size                  size;
  cv::Mat                   frame;
  cv::Mat                   grayImg;

  std::vector<cv::KeyPoint> keypoints;
  cv::Mat                   descriptors;

  std::vector<cv::Point2f>  points2d;
  std::vector<cv::Point3f>  points3d;
};
// Estructura que contiene la información de seguimiento de un patrón.
struct PatternTrackingInfo
{
  cv::Mat                   homography;
  std::vector<cv::Point2f>  points2d;
  Transformation            pose3d;

  void draw2dContour(cv::Mat& image, cv::Scalar color) const;

  void computePose(const Pattern& pattern, const CameraCalibration& calibration);
};

#endif