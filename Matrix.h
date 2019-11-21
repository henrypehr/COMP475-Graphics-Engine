#include <math.h>
#include "GPoint.h"
#include "GMatrix.h"





/**
 *  Set this matrix to identity.
 */
void GMatrix::setIdentity() {
  this->set6(1, 0, 0, 0, 1, 0);
}

/**
 *  Set this matrix to translate by the specified amounts.
 */
void GMatrix::setTranslate(float tx, float ty) {
  this->setIdentity();
  this->fMat[GMatrix::TX] = tx;
  this->fMat[GMatrix::TY] = ty;
}

/**
 *  Set this matrix to scale by the specified amounts.
 */
void GMatrix::setScale(float sx, float sy) {
  this->setIdentity();
  this->fMat[GMatrix::SX] = sx;
  this->fMat[GMatrix::SY] = sy;
}

/**
 *  Set this matrix to rotate by the specified radians.
 *
 *  Note: since positive-Y goes down, a small angle of rotation will increase Y.
 */
void GMatrix::setRotate(float radians) {
  this->set6(cos(radians), -sin(radians), 0, sin(radians), cos(radians), 0);
}

/**
 *  Set this matrix to the concatenation of the two specified matrices, such that the resulting
 *  matrix, when applied to points will have the same effect as first applying the primo matrix
 *  to the points, and then applying the secundo matrix to the resulting points.
 *
 *  Pts' = Secundo * Primo * Pts
 */
void GMatrix::setConcat(const GMatrix& secundo, const GMatrix& primo) {
  this->set6(
      primo[0] * secundo[0] + primo[3] * secundo[1],
      primo[1] * secundo[0] + primo[4] * secundo[1],
      primo[2] * secundo[0] + primo[5] * secundo[1] + secundo[2],
      primo[0] * secundo[3] + primo[3] * secundo[4],
      primo[1] * secundo[3] + primo[4] * secundo[4],
      primo[2] * secundo[3] + primo[5] * secundo[4] + secundo[5]);
}

/*
 *  If this matrix is invertible, return true and (if not null) set the inverse parameter.
 *  If this matrix is not invertible, return false and ignore the inverse parameter.
 */
bool GMatrix::invert(GMatrix* inverse) const {
  float a = this->fMat[0];
  float b = this->fMat[1];
  float c = this->fMat[2];
  float d = this->fMat[3];
  float e = this->fMat[4];
  float f = this->fMat[5];

  float determinant = a * e - b * d;
  if (determinant == 0) {
      return false;
  }

  float divisor = 1 / determinant;

  inverse->set6(
      e * divisor, -b * divisor, -(c * e - b * f) * divisor,
      -d * divisor, a * divisor, (c * d - a * f) * divisor);

  return true;
}

/**
 *  Transform the set of points in src, storing the resulting points in dst, by applying this
 *  matrix. It is the caller's responsibility to allocate dst to be at least as large as src.
 *
 *  Note: It is legal for src and dst to point to the same memory (however, they may not
 *  partially overlap). Thus the following is supported.
 *
 *  GPoint pts[] = { ... };
 *  matrix.mapPoints(pts, pts, count);
 */
void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
  for (int i = 0; i < count; ++i) {
      GPoint point = src[i];

      float x0 = point.x();
      float y0 = point.y();

      float x = this->fMat[GMatrix::SX] * x0 + this->fMat[GMatrix::KX] * y0 + this->fMat[GMatrix::TX];
      float y = this->fMat[GMatrix::SY] * y0 + this->fMat[GMatrix::KY] * x0 + this->fMat[GMatrix::TY];

      dst[i] = GPoint::Make(x, y);
  }
}
