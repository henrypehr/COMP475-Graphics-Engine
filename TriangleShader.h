#ifndef TRIANGLESHADER_H
#define TRIANGLESHADER_H



#include <math.h>

#include "GColor.h"
#include "GMatrix.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GShader.h"
#include "GMath.h"
#include "GPixel.h"
#include "Utils.h"


class TriangleShader : public GShader {
    GColor* fColors;
    GMatrix  fInverse;
    GMatrix fLocalMatrix;
    GMatrix fBarryMatrix;
  public:
      TriangleShader(GPoint p0, GPoint p1, GPoint p2, const GColor colors[3]) {
        fColors = (GColor*) malloc(3*sizeof(GColor));
        memcpy(fColors, colors, 3*sizeof(GColor));

        fBarryMatrix.set6(p1.x()-p0.x(), p2.x()-p0.x(), p0.x(),
                          p1.y()-p0.y(), p2.y()-p0.y(), p0.y());

      }

      TriangleShader() {}

      bool isOpaque() override {
        return false;
      }

      bool setContext(const GMatrix& ctm) override {
          fLocalMatrix.setConcat(ctm, fBarryMatrix);
          return fLocalMatrix.invert(&fInverse);
      }

      void shadeRow(int x, int y, int count, GPixel row[]) override {
          for(int i=0; i<count; i++) {
            GPoint spot = fInverse.mapXY(x+i+0.5f,y+0.5f);
            //fColors[1]*spot.fX + fColors[2]*spot.fY + fColors[0]*(1-spot.fX-spot.fY)
            GColor color = GColor::MakeARGB((fColors[1].fA*spot.fX + fColors[2].fA*spot.fY + fColors[0].fA*(1-spot.fX-spot.fY)),
                                            (fColors[1].fR*spot.fX + fColors[2].fR*spot.fY + fColors[0].fR*(1-spot.fX-spot.fY)),
                                            (fColors[1].fG*spot.fX + fColors[2].fG*spot.fY + fColors[0].fG*(1-spot.fX-spot.fY)),
                                            (fColors[1].fB*spot.fX + fColors[2].fB*spot.fY + fColors[0].fB*(1-spot.fX-spot.fY))
                                          );

            row[i] = colorToPixel(color);
          }
      }
};



#endif
