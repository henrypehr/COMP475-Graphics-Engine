#include "GPath.h"
#include "GPoint.h"
#include "GRect.h"
#include "GMatrix.h"
#include "GMath.h"
#include <cmath>

const float PI = 3.1415926;
const float tangent = std::tan(PI / 8.0);
const float rootTwo = 1 / std::sqrt(2);

GPath& GPath::addCircle(GPoint center, float radius, Direction dir) {


  GPoint points[16] = { {1, 0},
              			    {1, tangent},
              			    {rootTwo, rootTwo},
              			    {tangent, 1},
              			    {0, 1},
              			    {-tangent, 1},
              			    {-rootTwo, rootTwo},
              			    {-1, tangent},
              			    {-1, 0},
              			    {-1, -tangent},
              			    {-rootTwo, -rootTwo},
              			    {-tangent, -1},
              			    {0, -1},
              			    {tangent, -1},
              			    {rootTwo, -rootTwo},
              			    {1, -tangent}
  };
  GPoint translate;
  translate.set(center.x(), center.y());
  for (int i = 0; i < 16; i++) {
    points[i] = radius * points[i];
    points[i] = points[i] + translate;
  }

  moveTo(points[0]);
  if (dir == kCW_Direction) {
    for (int i = 15; i - 1 > 0; i -= 2) {
	     quadTo(points[i].x(), points[i].y(),points[i - 1].x(), points[i - 1].y());
	  }
    quadTo(points[0].x(), points[0].y(), points[15].x(), points[15].y());
  } else {
    for (int i = 1; i + 1 < 16; i += 2) {
       quadTo(points[i].x(), points[i].y(), points[i + 1].x(), points[i + 1].y());
    }
    quadTo(points[15].x(), points[15].y(), points[0].x(), points[0].y());
  }
  return *this;
}
  //create a unit circle and transform the points thru a matrix to get the right center and radius


void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
// do stuff
  dst[0] = src[0];
  dst[4] = src[2];
  dst[1] = (src[0]*(1-t))+(src[1]*(t));
  dst[3] = (src[1]*(1-t))+(src[2]*(t));

  dst[2] = (dst[1]*(1-t))+(dst[3]*(t));
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
// do stuff

  GPoint mid = (src[1]*(1-t))+(src[2]*(t));
  dst[0] = src[0];
  dst[6] = src[3];
  dst[1] = (src[0]*(1-t))+(src[1]*(t));
  dst[5] = (src[2]*(1-t))+(src[3]*(t));

  dst[2] = (dst[1]*(1-t))+(mid*(t));
  dst[4] = (mid*(1-t))+(dst[5]*(t));

  dst[3] = (dst[2]*(1-t))+(dst[4]*(t));
}

static inline float manyMax(float* nums, int count) {

    float max = nums[0];
    for (int i = 1; i < count; ++i) {
        max = std::max(max, nums[i]);
    }

    return max;
}

static inline float manyMin(float* nums, int count) {

    float min = nums[0];
    for (int i = 1; i < count; ++i) {
        min = std::min(min, nums[i]);
    }

    return min;
}

static inline float clamp(float val, float min, float max) {
    return std::max(min, std::min(max, val));
}


GPath& GPath::addRect(const GRect& r, Direction dir) {
  this->moveTo(GPoint::Make(r.left(),r.top()));

  GPoint tr = GPoint::Make(r.right(),r.top());
  GPoint br = GPoint::Make(r.right(),r.bottom());
  GPoint bl = GPoint::Make(r.left(),r.bottom());

  if (dir == Direction::kCW_Direction) {
    this->lineTo(tr);
    this->lineTo(br);
    this->lineTo(bl);
  } else {
    this->lineTo(bl);
    this->lineTo(br);
    this->lineTo(tr);
  }

  return *this;
}

GPath& GPath::addPolygon(const GPoint pts[], int count) {
  if (count <= 1) { return *this; }

    this->moveTo(pts[0]);
    for (int i = 1; i < count; ++i) {
        this->lineTo(pts[i]);
    }

    return *this;
}

GRect GPath::bounds() const {
  int size = this->fPts.size();

  if (size<1) {return GRect::MakeWH(0,0);}

  float xs[size], ys[size];
  // loop thru all points in x and y and save them to arrays
    for (int i = 0; i < size; ++i) {
        xs[i] = fPts[i].fX;
        ys[i] = fPts[i].fY;
    }
    // get the mins and maxs and make the bounding box from them
    return GRect::MakeLTRB(
        manyMin(xs, size),
        manyMin(ys, size),
        manyMax(xs, size),
        manyMax(ys, size));
}

void GPath::transform(const GMatrix& m) {
  m.mapPoints(this->fPts.data(), this->fPts.data(), this->fPts.size());
}
