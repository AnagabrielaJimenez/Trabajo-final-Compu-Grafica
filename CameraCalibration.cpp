#include "CameraCalibration.hpp"

CameraCalibration::CameraCalibration()
{
}
// Constructor que inicializa con parámetros intrínsecos de la cámara
CameraCalibration::CameraCalibration(float _fx, float _fy, float _cx, float _cy)
{
    m_intrinsic = cv::Matx33f::zeros();

    fx() = _fx;
    fy() = _fy;
    cx() = _cx;
    cy() = _cy;

    m_distortion.create(5,1);
    for (int i=0; i<5; i++)
        m_distortion(i) = 0;
}
// Constructor que inicializa con parámetros intrínsecos y coeficientes de distorsión

CameraCalibration::CameraCalibration(float _fx, float _fy, float _cx, float _cy, float distorsionCoeff[5])
{
    m_intrinsic = cv::Matx33f::zeros();

    fx() = _fx;
    fy() = _fy;
    cx() = _cx;
    cy() = _cy;

    m_distortion.create(5,1);
    for (int i=0; i<5; i++)
        m_distortion(i) = distorsionCoeff[i];
}
// Devuelve la matriz intrínseca de la cámara
const cv::Matx33f& CameraCalibration::getIntrinsic() const
{
    return m_intrinsic;
}

// Devuelve la matriz de distorsión de la cámara

const cv::Mat_<float>&  CameraCalibration::getDistorsion() const
{
    return m_distortion;
}
// Devuelve y permite modificar la distancia focal fx
float& CameraCalibration::fx()
{
    return m_intrinsic(1,1);
}
// Devuelve y permite modificar la distancia focal fy
float& CameraCalibration::fy()
{
    return m_intrinsic(0,0);
}
// Devuelve y permite modificar el punto principal cx
float& CameraCalibration::cx()
{
    return m_intrinsic(0,2);
}
// Devuelve y permite modificar el punto principal cy
float& CameraCalibration::cy()
{
    return m_intrinsic(1,2);
}
// Devuelve la distancia focal fx (constante)
float CameraCalibration::fx() const
{
    return m_intrinsic(1,1);
}
// Devuelve la distancia focal fy (constante)
float CameraCalibration::fy() const
{
    return m_intrinsic(0,0);
}
// Devuelve el punto principal cx (constante)
float CameraCalibration::cx() const
{
    return m_intrinsic(0,2);
}
// Devuelve el punto principal cy (constante)
float CameraCalibration::cy() const
{
    return m_intrinsic(1,2);
}