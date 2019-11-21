#include <deque>

#include "GPoint.h"
#include "GRect.h"

struct GEdge {
    int topY;
    int bottomY;
    float slope;
    float currX;
    int wind;
};

GEdge* makeGEdge(GPoint point1, GPoint point2, int w) {
  int wind = w;
  if (point1.y() > point2.y()) { //make point1 the top point
      GPoint temp = point2;
      point2 = point1;
      point1 = temp;
      wind = -wind;
  }

  int topY = (point1.y() + 0.5);
  int bottomY = (point2.y() + 0.5);

  if (topY == bottomY) {
    return nullptr;
  }

  float slope = (point2.x() - point1.x()) / (point2.y() - point1.y());

  float currX = point1.x() + slope * (topY - point1.y() + 0.5);

  GEdge* edge = new GEdge();
  edge->topY = topY;
  edge->bottomY = bottomY;
  edge->currX = currX;
  edge->slope = slope;
  edge->wind = wind;

  return edge;
}

void clip(GPoint p0, GPoint p1, GRect bounds, std::vector<GEdge>& returns) {
    int wind = 1;

    float invslope = (p1.x() - p0.x()) / (p1.y() - p0.y());
    float slope = (p1.y() - p0.y()) / (p1.x() - p0.x());

    if (p0.y() > p1.y()) {
        std::swap(p0, p1);
        wind = -wind;
    }

    // stop if above or below
    if (p1.y() <= bounds.top() || p0.y() >= bounds.bottom()) {
        return;
    }

    // clip top
    if (p0.y() < bounds.top()) {
        float newX = p0.x() + (invslope * (bounds.top() - p0.y()));
        p0.set(newX, bounds.top());

        assert(p0.y() >= bounds.top());
        assert(p1.y() >= bounds.top());
    }

    // clip bottom
    if (p1.y() > bounds.bottom()) {
        float newX = p1.x() - (invslope * (p1.y() - bounds.bottom()));
        p1.set(newX, bounds.bottom());

        assert(p0.y() >= bounds.top() && p0.y() <= bounds.bottom());
        assert(p1.y() >= bounds.top() && p1.y() <= bounds.bottom());
    }

    // fix so that 0=left 1=right
    if (p0.x() > p1.x()) {
        std::swap(p0, p1);
        wind = -wind;
    }

    // project onto left
    if (p1.x() <= bounds.left()) {
        p0.fX = p1.fX = bounds.left();

        assert(p0.y() >= bounds.top() && p0.y() <= bounds.bottom());
        assert(p1.y() >= bounds.top() && p1.y() <= bounds.bottom());
        GEdge* newEdge = makeGEdge(p0, p1, wind);
        if (newEdge != nullptr) {
          returns.push_back(*newEdge);
        }
        return;
    }

    // project onto right
    if (p0.x() >= bounds.right()) {
        p0.fX = p1.fX = bounds.right();

        assert(p0.y() >= bounds.top() && p0.y() <= bounds.bottom());
        assert(p1.y() >= bounds.top() && p1.y() <= bounds.bottom());
        GEdge* newEdge = makeGEdge(p0, p1, wind);
        if (newEdge != nullptr) {
          returns.push_back(*newEdge);
        }
        return;
    }

    // Handle clipping and projection onto the right boundary
    if (p1.x() > bounds.right()) {
        float newY = p1.y() - (slope * (p1.x() - bounds.right()));
        assert(newY >= bounds.top() && newY <= bounds.bottom());
        GEdge* newEdge = makeGEdge(
            GPoint::Make(bounds.right(), newY),
            GPoint::Make(bounds.right(), p1.y()), wind);
        if (newEdge != nullptr) {
          returns.push_back(*newEdge);
        }
        p1.set(bounds.right(), newY);
    }

    // Handle clipping and projection onto the left boundary
    if (p0.x() < bounds.left()) {
        float newY = p0.y() + (slope * (bounds.left() - p0.x()));
        assert(newY >= bounds.top() && newY <= bounds.bottom());
        GEdge* newEdge = makeGEdge(
            GPoint::Make(bounds.left(), p0.y()),
            GPoint::Make(bounds.left(), newY), wind);
        if (newEdge != nullptr) {
            returns.push_back(*newEdge);
        }
        p0.set(bounds.left(), newY);
    }
    // Now that we've clipped the segment, we make an edge from what's left.
    assert(p0.x() >= bounds.left() && p0.x() <= bounds.right());
    assert(p1.x() >= bounds.left() && p1.x() <= bounds.right());
    assert(p0.y() >= bounds.top() && p0.y() <= bounds.bottom());
    assert(p1.y() >= bounds.top() && p1.y() <= bounds.bottom());
    GEdge* newEdge = makeGEdge(p0, p1, wind);
    if (newEdge != nullptr) {
      returns.push_back(*newEdge);
    }
}

bool edgeIsGreater(GEdge e1, GEdge e2) {
  if (e1.topY < e2.topY) {
      return true;
    }

  if (e2.topY < e1.topY) {
      return false;
  }

  if (e1.currX < e2.currX) {
      return true;
  }

  if (e2.currX < e1.currX) {
      return false;
  }

    return e1.slope < e2.slope;
}
