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

#ifndef SPATIUMLIB_VECTOR_H
#define SPATIUMLIB_VECTOR_H

#include "Matrix.h"

namespace spatium {

/// \class Vector
/// \brief Vector in column-major order
class Vector : public Matrix
{
public:
  /// Constructor
  ///
  /// \param[in] rows Number of rows
  Vector(size_t rows)
    : Matrix(rows, 1)
  {
  }

  /// Constructor
  ///
  /// \param[in] array Initializer list
  Vector(std::initializer_list<double> array)
    : Matrix(array.size(), 1)
  {
    if (m_rows == 0)
    {
      m_cols = 0;
    }

    // Set values
    auto rowIt = array.begin();
    auto dataIt = m_data.begin();
    while(rowIt != array.end() && dataIt != m_data.end())
    {
      *dataIt = *rowIt;

      ++rowIt;
      ++dataIt;
    }
  }

  /// Constructor
  ///
  /// \param[in] vector STD vector
  Vector(const std::vector<double> &vector)
   : Matrix(vector.size(), 1)
  {
    m_data = vector;
  }

  /// Copy constructor
  ///
  /// \param[in] other Other vector
  Vector(const Vector &other)
    : Matrix(other)
  {
  }

  /// Copy constructor
  ///
  /// \param[in] other Other matrix
  /// \throw std::out_of_range Matrix dimensions out of range
  Vector(const Matrix &other)
    : Matrix(other.rows(), 1)
  {
    if (other.cols() != 1)
    {
      throw std::out_of_range("Matrix dimensions do not match vector dimensions");
    }

    for (size_t i = 0; i < m_rows; i++)
    {
        m_data[i] = other(i,0);
    }
  }

  /// Assignment operator
  Vector& operator=(const Vector &other) = default;

  /// Destructor
  ~Vector() = default;

  // Operators

  /// Access element by value.
  ///
  /// \param[in] row Row of element
  /// \throw std::out_of_range Matrix element out of range
  /// \return Element value
  double operator() (size_t row) const
  {
    if (row >= m_rows)
    {
      throw std::out_of_range ("Vector element out of range");
    }
    return m_data[row];
  }

  /// Access element by reference.
  ///
  /// \param[in] row Row of element
  /// \param[in] col Column of element
  /// \throw std::out_of_range Matrix element out of range
  /// \return Element reference
  double& operator() (size_t row)
  {
    if (row >= m_rows)
    {
      throw std::out_of_range ("Vector element out of range");
    }
    return m_data[row];
  }

  /// Add matrix to vector.
  ///
  /// \param[in] other Matrix to add
  /// \throw std::out_of_range Matrix dimensions out of range
  /// \return Added vector
  Vector operator+(const Matrix &other) const
  {
    /// \todo Use Matrix function and cast or make virtual?
    /// return Matrix::operator+(other);

    if (other.rows() != m_rows
        || other.cols() != m_cols)
    {
      throw std::out_of_range("Matrix dimensions do not match vector dimensions");
    }

    Vector result(m_rows);
    for (size_t row = 0; row < m_rows; row++)
    {
        result(row) = operator()(row) + other(row, 0);
    }
    return result;
  }

  /// Subtract matrix from vector.
  ///
  /// \param[in] other Matrix to subtract
  /// \throw std::out_of_range Matrix dimensions out of range
  /// \return Subtracted vector
  Vector operator-(const Matrix &other) const
  {
    /// \todo Use Matrix function and cast or make virtual?
    //return Matrix::operator-(other);

    // Check bounds
    if (other.rows() != m_rows
        || other.cols() != m_cols)
    {
      throw std::out_of_range("Matrix dimensions do not match vector dimensions");
    }

    // Subtract
    Vector result(m_rows);
    for (size_t row = 0; row < m_rows; row++)
    {
        result(row) = operator()(row) - other(row, 0);
    }
    return result;
  }

  /// Multiply by scalar.
  ///
  /// \param[in] scalar Scalar
  /// \return Multiplied vector
  Vector operator*(double scalar) const
  {
    /// \todo Use Matrix function and cast or make virtual?
    //return Matrix::operator*(scalar);

    Vector result(m_rows);
    for (size_t row = 0; row < m_rows; row++)
    {
        result(row) += operator()(row) * scalar;
    }
    return result;
  }

  /// Divide by scalar.
  ///
  /// \param[in] scalar Scalar
  /// \return Divided vector
  Vector operator/(double scalar) const
  {
    /// \todo Use Matrix function and cast?
    //return Matrix::operator/(scalar);

    Vector result(m_rows);
    for (size_t row = 0; row < m_rows; row++)
    {
        result(row) += operator()(row) / scalar;
    }
    return result;
  }

  // Other functions

  /// Resize.
  ///
  /// This will set all values to 0.
  ///
  /// \param[in] rows Number of rows
  void resize(size_t rows)
  {
    Matrix::resize(rows, 1);
  }

  /// Output to ostream
  friend std::ostream &operator<<(std::ostream &os, const Vector &vector)
  {
    os << "Vector(" << vector.m_rows << ")" << std::endl;
    for (size_t row = 0; row < vector.m_rows; row++)
    {
      os << vector(row) << " ";
      if (row < vector.m_rows - 1)
      {
        os << std::endl;
      }
    }
    return os;
  }

private:
  using Matrix::cols;
  using Matrix::resize;
};

} // namespace spatium

#endif // SPATIUMLIB_VECTOR_H
