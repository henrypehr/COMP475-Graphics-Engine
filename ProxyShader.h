#ifndef PROXYSHADER_H
#define PROXYSHADER_H

#include <math.h>

#include "GColor.h"
#include "GMatrix.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GShader.h"
#include "GMath.h"
#include "GPixel.h"


class ProxyShader : public GShader {
    GShader* fRealShader = nullptr;
    GMatrix  fInverse;
    GMatrix fP;
    GMatrix fT;
public:
    ProxyShader(GPoint p0, GPoint p1, GPoint p2, GPoint t0, GPoint t1, GPoint t2, GShader* shader) : fRealShader(shader) {
      fP.set6(p1.x()-p0.x(), p2.x()-p0.x(), p0.x(),
              p1.y()-p0.y(), p2.y()-p0.y(), p0.y());

      fT.set6(t1.x()-t0.x(), t2.x()-t0.x(), t0.x(),
              t1.y()-t0.y(), t2.y()-t0.y(), t0.y());
    }

    ProxyShader() {}

    bool isOpaque() override {
      return fRealShader->isOpaque();
    }

    bool setContext(const GMatrix& ctm) override {
        GMatrix mult1, mult2;
        fT.invert(&fInverse);
        mult1.setConcat(fP, fInverse);
        mult2.setConcat(ctm,mult1);
        return fRealShader->setContext(mult2);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        fRealShader->shadeRow(x, y, count, row);
    }
};

#endif
