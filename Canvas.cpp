#include <vector>
#include <stack>
#include "GCanvas.h"
#include "GBitmap.h"
#include "GRect.h"
#include "GPaint.h"
#include "GPoint.h"
#include "GShader.h"
#include "GMatrix.h"
#include "BitmapShader.h"
#include "Matrix.h"
#include "Clipper.h"
#include "GPath.h"
#include "Path.h"
#include "GradientShader.h"
#include "TriangleShader.h"
#include "ProxyShader.h"
#include "ComposeShader.h"
#include "Utils.h"


using namespace std;

void GDrawSomething_rects(GCanvas* canvas) {

}

GColor addColors(const GColor& c1, const GColor& c2) {
    return { GColor::MakeARGB(c1.fA+c2.fA,c1.fR+c2.fR,c1.fG+c2.fG,c1.fB+c2.fB) };
}

GColor multColor(const GColor& c, float a) {
    return { GColor::MakeARGB(c.fA*a,c.fR*a,c.fG*a,c.fB*a) };
}

GColor quadEqnColors(const GColor quad[4], float u, float v) {
  GColor a=multColor(quad[0],(1-u)*(1-v));
  GColor b=multColor(quad[3],(1-u)*v);
  GColor c=multColor(quad[1],u*(1-v));
  GColor d=multColor(quad[2],u*v);
  return addColors(addColors(a,b),addColors(c,d));
}

GPoint quadEqnPts(const GPoint quad[4], float u, float v) {
  return ((1-u)*(1-v)*quad[0] + (1-u)*v*quad[3] + u*(1-v)*quad[1] + u*v*quad[2]);
}


void pop_front(std::vector<GEdge>& v) {
    if (v.size() > 0) {
        v.erase(v.begin());
    }
}

// ------------------------
// ALL BLEND MODES HERE
// ------------------------
int kClear(int s, int d, int sa, int da) {
  assert ((0)<=255);
  return 0;
}
int kSrc(int s, int d, int sa, int da) {
  assert ((s)<=255);
  return s;
}
int kDst(int s, int d, int sa, int da) {
  assert ((d)<=255);
  return d;
}
int kSrcOver(int s, int d, int sa, int da) {
  assert ((s + divBy255((255 - sa) * d))<=255);
  return s + divBy255((255 - sa) * d);
}
int kDstOver(int s, int d, int sa, int da) {
  assert ((d + divBy255((255 - da)  * s))<=255);
  return d + divBy255((255 - da)  * s);
}
int kSrcIn(int s, int d, int sa, int da) {
  assert ((divBy255(da * s))<=255);
  return divBy255(da * s);
}
int kDstIn(int s, int d, int sa, int da) {
  assert ((divBy255(sa * d))<=255);
  return divBy255(sa * d);
}
int kSrcOut(int s, int d, int sa, int da) {
  assert ((divBy255((255 - da) * s))<=255);
  return divBy255((255 - da) * s);
}
int kDstOut(int s, int d, int sa, int da) {
  assert ((divBy255((255 - sa) * d))<=255);
  return divBy255((255 - sa) * d);
}
int kSrcATop(int s, int d, int sa, int da) {
  assert ((divBy255(da*s) + divBy255((255 - sa) * d))<=255);
  return divBy255(da*s) + divBy255((255 - sa) * d);
}
int kDstATop(int s, int d, int sa, int da) {
  assert ((divBy255(sa*d) + divBy255((255 - da) * s))<=255);
  return divBy255(sa*d) + divBy255((255 - da) * s);
}
int kXor(int s, int d, int sa, int da) {
  assert ((divBy255((255 - sa)*d) + divBy255((255 - da)*s))<=255);
  return divBy255(((255 - sa)*d) + ((255 - da)*s));
}




GIRect intersection(GIRect rect, GIRect canvas) {
  int l = fmax(rect.fLeft,   canvas.fLeft);
  int t = fmax(rect.fTop,    canvas.fTop);
  int r = fmin(rect.fRight,  canvas.fRight);
  int b = fmin(rect.fBottom, canvas.fBottom);
  // only return true if the resulting intersection is non-empty and non-inverted
  if (l < r && t < b) {
      return GIRect::MakeLTRB(l, t, r, b);
  } else {return GIRect::MakeLTRB(0,0,0,0);}
}

typedef int (*fcnPointer)(int, int, int, int);

fcnPointer BLENDS[] = {
  kClear,    //!<     0
  kSrc,      //!<     S
  kDst,      //!<     D
  kSrcOver,  //!<     S + (1 - Sa)*D
  kDstOver,  //!<     D + (1 - Da)*S
  kSrcIn,    //!<     Da * S
  kDstIn,    //!<     Sa * D
  kSrcOut,   //!<     (1 - Da)*S
  kDstOut,   //!<     (1 - Sa)*D
  kSrcATop,  //!<     Da*S + (1 - Sa)*D
  kDstATop,  //!<     Sa*D + (1 - Da)*S
  kXor,      //!<     (1 - Sa)*D + (1 - Da)*S
};

GPixel blend(GPixel src, GPixel dst, GBlendMode blendmode) {
  // math is R = S + ((((255 - S) * D) * (1 << 24 / 255) + (1 << 23)) >> 24)

  int sA = GPixel_GetA(src);
  int sR = GPixel_GetR(src);
  int sG = GPixel_GetG(src);
  int sB = GPixel_GetB(src);

  int dA = GPixel_GetA(dst);
  int dR = GPixel_GetR(dst);
  int dG = GPixel_GetG(dst);
  int dB = GPixel_GetB(dst);

  int (*fcnPointer)(int,int,int,int) = BLENDS[static_cast<int>(blendmode)];
  int rA = fcnPointer(sA,dA,sA,dA);
  int rR = fcnPointer(sR,dR,sA,dA);
  int rG = fcnPointer(sG,dG,sA,dA);
  int rB = fcnPointer(sB,dB,sA,dA);

  return GPixel_PackARGB(rA,rR,rG,rB);
}

bool compareX(GEdge e1, GEdge e2) {
  return e1.currX <= e2.currX;
}

class EmptyCanvas : public GCanvas {
public:
    EmptyCanvas(const GBitmap& device) : fDevice(device) {
      GMatrix identitty;
      identitty.setIdentity();
      mCTMStack.push(identitty);
    }


    virtual void save() override {
      GMatrix ctm = mCTMStack.top();
      GMatrix copy(
          ctm[0], ctm[1], ctm[2],
          ctm[3], ctm[4], ctm[5]);

      mCTMStack.push(copy);
    }
    virtual void restore() override {
      mCTMStack.pop();
    }
    virtual void concat(const GMatrix& m) override {
      mCTMStack.top().preConcat(m);
    }

    void drawTriangle(const GPoint pts[3], const GColor colors[3], const GPoint texs[3], const GPaint& paint) {
      GShader* newShader;
      TriangleShader t;
      ProxyShader x;
      ComposeShader z;

      if (colors) {
        t = TriangleShader(pts[0],pts[1],pts[2],colors);
        newShader = &t;
      }
      if (texs) {
        x = ProxyShader(pts[0],pts[1],pts[2],texs[0],texs[1],texs[2], paint.getShader());
        newShader = &x;
      }

      if (colors && texs) {
        z = ComposeShader(&t,&x);
        newShader = &z;
      }

      GPaint p(newShader);

      drawConvexPolygon(pts,3,p);
    }

    virtual void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint& paint) override {
      int j=0;
      int a,b,c;
      GColor aColor,bColor,cColor;
      GPoint aTex,bTex,cTex;

      for(int i=0; i<count;i++) {
        a = indices[j];
        b = indices[j+1];
        c = indices[j+2];

        if(colors) {
          aColor = colors[j];
          bColor = colors[j+1];
          cColor = colors[j+2];
        }

        if(texs) {
          aTex = texs[j];
          bTex = texs[j+1];
          cTex = texs[j+2];
        }

        j+=3;

        GPoint pts[3] = {verts[a],verts[b],verts[c]};

        if(colors) {
          GColor newColors[3] = {colors[a],colors[b],colors[c]};
          drawTriangle(pts, newColors, texs, paint);
        }

        if(texs) {
          GPoint newTexs[3] = {texs[a],texs[b],texs[c]};
          drawTriangle(pts, colors, newTexs, paint);
        }

        if(colors && texs) {
          GColor newColors[3] = {colors[a],colors[b],colors[c]};
          GPoint newTexs[3] = {texs[a],texs[b],texs[c]};
          drawTriangle(pts, newColors, newTexs, paint);
        }

        if(!colors && !texs) {
          drawTriangle(pts, colors, texs, paint);
        }

      }
    }

    virtual void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint) override {
      float delta = (float) 1 / (float) (level + 1);
      GPoint pts1 [3];
      GPoint pts2 [3];
      GColor colors1 [3];
      GColor colors2 [3];
      GPoint texs1 [3];
      GPoint texs2 [3];

        for(float u = 0; u<1; u+=delta) {
          for(float v = 0; v<1; v+=delta) {
            GPoint p1 = quadEqnPts(verts, u,v);
            GPoint p2 = quadEqnPts(verts, u+delta,v);
            GPoint p3 = quadEqnPts(verts, u+delta,v+delta);
            GPoint p4 = quadEqnPts(verts, u,v+delta);
            pts1[0] = p1;pts1[1] = p2;pts1[2] = p3;
            pts2[0] = p1;pts2[1] = p4;pts2[2] = p3;

            if(colors) {
              GColor c1 = quadEqnColors(colors, u,v);
              GColor c2 = quadEqnColors(colors, u+delta,v);
              GColor c3 = quadEqnColors(colors, u+delta,v+delta);
              GColor c4 = quadEqnColors(colors, u,v+delta);
              colors1[0] = c1;colors1[1] = c2;colors1[2] = c3;
              colors2[0] = c1;colors2[1] = c4;colors2[2] = c3;
              drawTriangle(pts1, colors1, texs, paint);
              drawTriangle(pts2, colors2, texs, paint);
            }

            if(texs) {
              GPoint t1 = quadEqnPts(texs, u,v);
              GPoint t2 = quadEqnPts(texs, u+delta,v);
              GPoint t3 = quadEqnPts(texs, u+delta,v+delta);
              GPoint t4 = quadEqnPts(texs, u,v+delta);
              texs1[0] = t1;texs1[1] = t2;texs1[2] = t3;
              texs2[0] = t1;texs2[1] = t4;texs2[2] = t3;
              drawTriangle(pts1, colors, texs1, paint);
              drawTriangle(pts2, colors, texs2, paint);

            }

          }
        }

    }

    virtual void drawPath(const GPath& path, const GPaint& paint) override {

      GPath mypath = path;
      mypath.transform(mCTMStack.top());

      //first create vector for edges of polygon
      std::vector<GEdge> edges;

      GPath::Edger edger = GPath::Edger(mypath);
      GRect bounds = GRect::MakeWH(fDevice.width(),fDevice.height());
      bool isQuad = false;
      GPath::Verb verb;
      do {
        GPoint pt[4];
        verb = edger.next(pt);

        if (verb == GPath::Verb::kLine) {
          clip(pt[0],pt[1],bounds,edges);
        } else if (verb == GPath::Verb::kQuad) {
          isQuad = true;
          GPoint d = (pt[0] + pt[2] - 2*pt[1]) * 0.25;
          float mag = std::sqrt(d.x()*d.x()+d.y()*d.y());
          float n = std::sqrt(mag/0.25);
          GPoint prev = pt[0];
          //n is number of lines, n+1 is number of points
          GPoint a = pt[0]+pt[2]-2*pt[1];
          GPoint b = 2*pt[1]-2*pt[0];
          GPoint c = pt[0];
          for (int i=1; i <= n; i++) {
            float t = i/n;
            GPoint next = ((a)*t + b)*t + c;//QUAD EQUATION

            clip(prev,next,bounds,edges);

            prev = next;
          }
          clip(prev,pt[2],bounds,edges);

        } else if (verb == GPath::Verb::kCubic) {
          GPoint d = (pt[0] + pt[2] - 2*pt[1]) * 0.25;
          float mag = std::sqrt(d.x()*d.x()+d.y()*d.y());
          float n = std::sqrt(mag/0.25);
          GPoint prev = pt[0];
          //n is number of lines, n+1 is number of points
          for (int i=1; i <= n; i++) {
            float t = i/n;
            GPoint next = (1-t)*(1-t)*(1-t)*pt[0]+3*t*(1-t)*(1-t)*pt[1]+3*t*t*(1-t)*pt[2]+t*t*t*pt[3];//CUBIC EQUATION

            clip(prev,next,bounds,edges);

            prev = next;
          }
          clip(prev,pt[3],bounds,edges);
        }
        // handle kquad and kcubic
      } while (
        verb != GPath::Verb::kDone);
      if (isQuad) {
        int du = 1;
      }

      if (edges.size() <= 1) {return;}
      std::sort(edges.begin(),edges.end(),edgeIsGreater);

      //active = edges.top < y < edges.bottom
      for (int y=bounds.fTop;y<bounds.fBottom;) {
        //sort in x

        int wind = 0;
        int index = 0;
        int count = edges.size();
        int x0,x1;
        //loop in x (active)
        while (index < count && edges[index].topY <= y && edges[index].bottomY >= y) {
            // work with active[index];
            if (wind == 0) {
              x0 = GRoundToInt(edges[index].currX);
            }

            wind += edges[index].wind;
        // check w (for left) → did we go from 0 to non-0
           // update w → w += active[index].winding
           // check w (for right and blit) → did we go from non-0 to 0
            if (wind == 0) {
              x1 = GRoundToInt(edges[index].currX);
              drawRow(y,x0,x1,paint);
            }
           // if the edge is done, remove from array (and --count)
           // else update currX by slope (and ++index)
           if (edges[index].bottomY > y + 1) {
             edges[index].currX += edges[index].slope;
             index++;
           } else {
             edges.erase(edges.begin() + index);
             count--;
           }
        }

        y++;
        // move index to include edges that will be valid
        while (index < count && y == edges[index].topY) {
            index += 1;
        }

        std::sort(edges.begin(),edges.begin() + index,compareX);

      }
    }

    void drawPaint(const GPaint& color) override {
        // run thru bitmap and save all pixels as the color passed
        GPixel* origin = fDevice.pixels();

        for (int y=0;y<fDevice.height();y++) {
          drawRow(y, 0, fDevice.width(), color);
        }
    }

    void drawRect(const GRect& rect, const GPaint& color) override {
      GPoint newrect[4] = {
          GPoint::Make(rect.left(), rect.top()),
          GPoint::Make(rect.right(), rect.top()),
          GPoint::Make(rect.right(), rect.bottom()),
          GPoint::Make(rect.left(), rect.bottom())
      };

      EmptyCanvas::drawConvexPolygon(newrect, 4, color);
    }



    virtual void drawConvexPolygon(const GPoint mapPoints[], int count, const GPaint& paint) override {
      int index = 0;
      if (count <= 2) {
        //invalid number of points
        return;
      }

      GPoint points[count];
      mCTMStack.top().mapPoints(points, mapPoints, count);


      GBlendMode mode = paint.getBlendMode();
      GPixel pix = colorToPixel(paint);
      //first create queue for edges of polygon
      GRect bounds = GRect::MakeWH(fDevice.width(),fDevice.height());
      std::vector<GEdge> edges;
      for(int i=0;i<count;i++) {
        clip(points[i],points[(i+1)%count],bounds,edges);
      }
      if (edges.size() <= 1) {
        // you idiot you didn't even draw anything!
        return;
      }

      //assert(edges.size() > 2);
      std::sort(edges.begin(),edges.end(),edgeIsGreater);



        GEdge left = edges[index++];


        GEdge right = edges[index++];


        int curY = left.topY;
        float leftX = left.currX;
        float rightX = right.currX;
        int lastY = edges.back().bottomY;

        // loop thru

        while (curY < lastY) { //took out equals

            // leftX = fmax(leftX,0);
            // rightX = fmin(rightX,fDevice.width());

            if (leftX==rightX) {
              curY++;
            }

            if (leftX > rightX) {
              GEdge tmp = left;
              left = right;
              right = tmp;
            }
            assert(curY >= 0 && curY <= fDevice.height());
            assert(leftX >= 0 && leftX <= fDevice.width());
            assert(rightX >= 0 && rightX <= fDevice.width());
            drawRow(curY, GRoundToInt(leftX), GRoundToInt(rightX), paint);
            // if (curY >= 0 && curY <= fDevice.height()) {
            //   if (leftX>= 0 && rightX <= fDevice.width()) {
            //
            //   }
            // }

            //replace

            if (curY+1 >= left.bottomY) { // reached bottom of left
                left = edges[index];
                index++;
                leftX = left.currX;
            } else {
                leftX += left.slope;
            }

            if (curY+1 >= right.bottomY) { // reached bottom of right
                right = edges[index];
                index++;
                rightX = right.currX;
            } else {
                rightX += right.slope;
            }

            curY++;
          }
    }

    void drawRow(int y, int xLeft, int xRight, const GPaint& paint) {
        // GPoint mapPoints[count];
        // mCTMStack.top().mapPoints(mapPoints, points, count);

        // xLeft = std::max(0, xLeft);
        // xRight = std::min(fDevice.width(), xRight);

        GBlendMode mode = paint.getBlendMode();

        GShader* shader = paint.getShader();
        if (shader == nullptr) {
            GColor color = paint.getColor().pinToUnit();
            GPixel source = colorToPixel(color);

            for (int x = xLeft; x < xRight; ++x) {
                GPixel* addr = fDevice.getAddr(x, y);
                *addr = blend(source, *addr, mode);
            }
        } else {
            if (!shader->setContext(mCTMStack.top())) {
                return;
            }
            int count = xRight - xLeft;
            GPixel shaded[count];
            shader->shadeRow(xLeft, y, count, shaded);

            for (int x = xLeft; x < xRight; ++x) {
                GPixel* addr = fDevice.getAddr(x, y);
                *addr = blend(shaded[x - xLeft], *addr, mode);
            }
        }
    }

private:
    const GBitmap fDevice;
    std::stack<GMatrix> mCTMStack;
};

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    if (!device.pixels()) {
        return nullptr;
    }
    return std::unique_ptr<GCanvas>(new EmptyCanvas(device));
}
