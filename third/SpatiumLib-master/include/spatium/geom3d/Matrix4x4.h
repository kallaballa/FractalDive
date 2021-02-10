/*
 * Program: Spatium Library
 *
 * Copyright (C) Martijn Koopman
 * All Rights Reserved
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 */

#ifndef SPATIUMLIB_GEOM3D_MATRIX4X4_H
#define SPATIUMLIB_GEOM3D_MATRIX4X4_H

#include "spatium/Matrix.h"
#include "Vector3.h"

#include <cmath> // std::cos

namespace spatium {
namespace geom3d {

class Vector3;

/// \class Matrix4x4
/// \brief 4-by-4 matrix to represent and manipulate 3D transformations.
///
/// Matrix4x4 is a class to represent and manipulate 4x4 matrices.
/// Specifically, it is designed to work on 4x4 transformation matrices found
/// in 3D rendering using homogeneous coordinates [x y z w].
class Matrix4x4 : public Matrix
{
public:
  /// Constructor
  Matrix4x4()
    : Matrix(4, 4)
  {
    operator()(0,0) = 1;
    operator()(1,1) = 1;
    operator()(2,2) = 1;
    operator()(3,3) = 1;
  }

  /// Copy constructor
  Matrix4x4(const Matrix4x4 &other) // Needed? Implicit
    : Matrix(other)
  {
  }

  /// Copy constructor
  ///
  /// \param[in] other Other matrix
  /// \throw std::out_of_range Matrix dimenions != 4
  Matrix4x4(const Matrix &other)
    : Matrix(4,4)
  {
    if (other.rows() != 4
     || other.cols() != 4)
    {
      throw std::out_of_range("Matrix dimensions are not 4 by 4");
    }

    for (size_t row = 0; row < m_rows; row++)
    {
      for (size_t col = 0; col < m_cols; col++)
      {
        operator ()(row,col) = other(row,col);
      }
    }
  }

  /// Assignment operator
  Matrix4x4& operator=(const Matrix4x4 &other) = default;

//  Matrix4x4& operator=(Matrix4x4 other)
//  {
//    Matrix::operator=(other);
//    return *this;
//  }

  /// Destructor
  ~Matrix4x4() = default;

  // Construct affine transformation Matrix4x4

  /// Construct translation matrix.
  ///
  /// \param[in] x Translation on X axis
  /// \param[in] y Translation on Y axis
  /// \param[in] z Translation on Z axis
  /// \return Translation matrix
  static Matrix4x4 translation(double x, double y, double z)
  {
    Matrix4x4 result; // Identity matrix
    result(0,3) = x;
    result(1,3) = y;
    result(2,3) = z;
    return result;
  }

  /// Construct scaling matrix.
  ///
  /// \param[in] x Scaling on X axis
  /// \param[in] y Scaling on Y axis
  /// \param[in] z Scaling on Z axis
  /// \return Scaling matrix
  static Matrix4x4 scaling(double x, double y, double z)
  {
    Matrix4x4 result; // Identity matrix
    result(0,0) = x;
    result(1,1) = y;
    result(2,2) = z;
    return result;
  }

  /// Construct rotation matrix (counterclockwise).
  ///
  /// \param[in] x Rotation on X axis
  /// \param[in] y Rotation on Y axis
  /// \param[in] z Rotation on Z axis
  /// \return Rotation matrix
  static Matrix4x4 rotation(double x, double y, double z)
  {
    return Matrix4x4::rotationZ(z)
           * Matrix4x4::rotationY(y)
           * Matrix4x4::rotationX(x);
  }

  /// Construct matrix for rotation around X axis (counterclockwise).
  ///
  /// \param[in] angle Angle in radians
  /// \return Rotation matrix
  static Matrix4x4 rotationX(double angle)
  {
    Matrix4x4 result; // Identity matrix
    result(1,1) = cos(angle);
    result(1,2) = -sin(angle); // +?
    result(2,1) = sin(angle); // -?
    result(2,2) = cos(angle);
    return result;
  }

  /// Construct matrix for rotation around Y axis (counterclockwise).
  ///
  /// \param[in] angle Angle in radians
  /// \return Rotation matrix
  static Matrix4x4 rotationY(double angle)
  {
    Matrix4x4 result; // Identity matrix
    result(0,0) = cos(angle);
    result(0,2) = sin(angle);
    result(2,0) = -sin(angle);
    result(2,2) = cos(angle);
    return result;
  }

  /// Construct matrix for rotation around Z axis (counterclockwise).
  ///
  /// \param[in] angle Angle in radians
  /// \return Rotation matrix
  static Matrix4x4 rotationZ(double angle)
  {
    Matrix4x4 result; // Identity matrix
    result(0,0) = cos(angle);
    result(0,1) = -sin(angle); // +?
    result(1,0) = sin(angle); // -?
    result(1,1) = cos(angle);
    return result;
  }

  /// Construct matrix for rotation around axis (counterclockwise).
  ///
  /// \param[in] axis Axis vector
  /// \param[in] angle Angle in radians
  /// \return Rotation matrix
  static Matrix4x4 rotationAround(const Vector3 &axis, double angle)
  {
    Vector3 axisNormalized = axis.normalized();
    double q0 = cos(angle/2);
    double q1 = sin(angle/2) * axisNormalized.x();
    double q2 = sin(angle/2) * axisNormalized.y();
    double q3 = sin(angle/2) * axisNormalized.z();

    Matrix4x4 result;
    result(0,0) = q0*q0 + q1*q1 - q2*q2 - q3*q3;
    result(0,1) = 2*(q1*q2 - q0*q3);
    result(0,2) = 2*(q1*q3 + q0*q2);
    result(1,0) = 2*(q2*q1 + q0*q3);
    result(1,1) = (q0*q0 - q1*q1 + q2*q2 - q3*q3);
    result(1,2) = 2*(q2*q3 - q0*q1);
    result(2,0) = 2*(q3*q1 - q0*q2);
    result(2,1) = 2*(q3*q2 + q0*q1);
    result(2,2) = (q0*q0 - q1*q1 - q2*q2 + q3*q3);
    return result;
  }
};

} // namespace geom3d
} // namespace spatium

#endif // SPATIUMLIB_GEOM3D_MATRIX4X4_H
