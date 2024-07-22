#ifndef Example_MarkerBasedAR_GeometryTypes_hpp
#define Example_MarkerBasedAR_GeometryTypes_hpp
// Estructura que representa una matriz 4x4 y proporciona funciones para transposición e inversión.
struct Matrix44
{
  union
  {
    float data[16];
    float mat[4][4];
  };
  
  Matrix44 getTransposed() const;
  Matrix44 getInvertedRT() const;
  static Matrix44 identity();
};
// Estructura que representa una matriz 3x3 y proporciona una función para obtener la matriz identidad.
struct Matrix33
{
  union
  {
    float data[9];
    float mat[3][3];
  };
  
  static Matrix33 identity();
  Matrix33 getTransposed() const;
};
// Estructura que representa un vector 4D.
struct Vector4
{
  float data[4];
};
// Estructura que representa un vector 3D y proporciona funciones útiles para manipularlo.
struct Vector3
{
  float data[3];
  
  static Vector3 zero();
  Vector3 operator-() const;
};
// Estructura que representa una transformación con rotación y traslación.
struct Transformation
{
  Transformation();
  Transformation(const Matrix33& r, const Vector3& t);
  
  Matrix33& r();
  Vector3&  t();
  
  const Matrix33& r() const;
  const Vector3&  t() const;
  
  Matrix44 getMat44() const;
  
  Transformation getInverted() const;
private:
  Matrix33 m_rotation;
  Vector3  m_translation;
};

#endif
