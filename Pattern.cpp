#include "Pattern.hpp"
// Calcula la pose del patr�n utilizando la calibraci�n de la c�mara y los puntos 2D y 3D del patr�n.
void PatternTrackingInfo::computePose(const Pattern& pattern, const CameraCalibration& calibration)
{
  cv::Mat Rvec;
  cv::Mat_<float> Tvec;
  cv::Mat raux,taux;

  cv::solvePnP(pattern.points3d, points2d, calibration.getIntrinsic(), calibration.getDistorsion(),raux,taux);
  raux.convertTo(Rvec,CV_32F);
  taux.convertTo(Tvec ,CV_32F);

  cv::Mat_<float> rotMat(3,3); 
  cv::Rodrigues(Rvec, rotMat);

  for (int col=0; col<3; col++)
  {
    for (int row=0; row<3; row++)
    {        
     pose3d.r().mat[row][col] = rotMat(row,col); 
    }
    pose3d.t().data[col] = Tvec(col); 

  
  pose3d = pose3d.getInverted();
}
// Dibuja el contorno 2D del patr�n en la imagen con el color especificado.
void PatternTrackingInfo::draw2dContour(cv::Mat& image, cv::Scalar color) const
{
  for (size_t i = 0; i < points2d.size(); i++)
  {
    cv::line(image, points2d[i], points2d[ (i+1) % points2d.size() ], color, 2, CV_AA);
  }
}

