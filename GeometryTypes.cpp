#include "GeometryTypes.hpp"

// Devuelve la transposición de la matriz 4x4.

Matrix44 Matrix44::getTransposed() const
{
  Matrix44 t;
  
  for (int i=0;i<4; i++)
    for (int j=0;j<4;j++)
      t.mat[i][j] = mat[j][i];
    
  return t;
}
// Devuelve una matriz identidad 4x4.
Matrix44 Matrix44::identity()
{
  Matrix44 eye;
  
  for (int i=0;i<4; i++)
    for (int j=0;j<4;j++)
      eye.mat[i][j] = i == j ? 1 : 0;
  
  return eye;
}
// Devuelve la matriz 4x4 que representa la inversión de la rotación y la traslación.
Matrix44 Matrix44::getInvertedRT() const
{
  Matrix44 t = identity();
  
  for (int col=0;col<3; col++)
  {
    for (int row=0;row<3;row++)
    { 
      t.mat[row][col] = mat[col][row];
    }
    
    t.mat[3][col] = - mat[3][col];
  }
  return t;
}
// Devuelve una matriz identidad 3x3.
Matrix33 Matrix33::identity()
{
  Matrix33 eye;
  
  for (int i=0;i<3; i++)
    for (int j=0;j<3;j++)
      eye.mat[i][j] = i == j ? 1 : 0;
  
  return eye;
}
// Devuelve la transposición de la matriz 3x3.
Matrix33 Matrix33::getTransposed() const
{
  Matrix33 t;
  
  for (int i=0;i<3; i++)
    for (int j=0;j<3;j++)
      t.mat[i][j] = mat[j][i];
  
  return t;
}
// Devuelve un vector 3D con todos los componentes en 0.
Vector3 Vector3::zero()
{
  Vector3 v = { 0,0,0 };
  return v;
}
// Devuelve el vector 3D negado.
Vector3 Vector3::operator-() const
{
  Vector3 v = { -data[0],-data[1],-data[2] };
  return v;
}

Transformation::Transformation()
: m_rotation(Matrix33::identity())
, m_translation(Vector3::zero())
{
  
}
// Constructor que inicializa la transformación con una matriz de rotación y un vector de traslación dados.
Transformation::Transformation(const Matrix33& r, const Vector3& t)
: m_rotation(r)
, m_translation(t)
{
  
}
// Devuelve una referencia a la matriz de rotación en la transformación.
Matrix33& Transformation::r()
{
  return m_rotation;
}
// Devuelve una referencia al vector de traslación en la transformación.
Vector3&  Transformation::t()
{
  return  m_translation;
}
// Devuelve una referencia constante a la matriz de rotación en la transformación.
const Matrix33& Transformation::r() const
{
  return m_rotation;
}
// Devuelve una referencia constante al vector de traslación en la transformación.
const Vector3&  Transformation::t() const
{
  return  m_translation;
}
// Devuelve la transformación como una matriz 4x4.
Matrix44 Transformation::getMat44() const
{
  Matrix44 res = Matrix44::identity();
  
  for (int col=0;col<3;col++)
  {
    for (int row=0;row<3;row++)
    {
      res.mat[row][col] = m_rotation.mat[row][col];
    }
    
    res.mat[3][col] = m_translation.data[col];
  }
  
  return res;
}
// Devuelve la transformación invertida (rotación transpuesta y traslación negada).
Transformation Transformation::getInverted() const
{
  return Transformation(m_rotation.getTransposed(), -m_translation); 
}