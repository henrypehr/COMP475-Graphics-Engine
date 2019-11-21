
#include <math.h>

#include "GColor.h"
#include "GMatrix.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GShader.h"
#include "GMath.h"
#include "GPixel.h"
#include "TriangleShader.h"


class ComposeShader : public GShader {
    GShader* fShader1 = nullptr;
    GShader* fShader2 = nullptr;
  public:
      ComposeShader(GShader *shader1, GShader *shader2) : fShader1(shader1), fShader2(shader2) {}
      ComposeShader() {}

      bool isOpaque() override {
        return fShader1->isOpaque() && fShader2->isOpaque();
      }

      bool setContext(const GMatrix& ctm) override {
          bool op1 = fShader1->setContext(ctm);
          bool op2 = fShader2->setContext(ctm);
          return op1 && op2;
      }

      void shadeRow(int x, int y, int count, GPixel row[]) override {
          GPixel scrap[count];
          fShader1->shadeRow(x,y,count,scrap);
          fShader2->shadeRow(x,y,count,row);
          for(int i=0; i<count; i++) {
            float a = divBy255(GPixel_GetA(scrap[i]) * GPixel_GetA(row[i]));
            float r = divBy255(GPixel_GetR(scrap[i]) * GPixel_GetR(row[i]));
            float g = divBy255(GPixel_GetG(scrap[i]) * GPixel_GetG(row[i]));
            float b = divBy255(GPixel_GetB(scrap[i]) * GPixel_GetB(row[i]));

            row[i] = GPixel_PackARGB(a,r,g,b);

          }
      }
};
