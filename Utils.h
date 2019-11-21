#ifndef UTILS_H
#define UTILS_H


int divBy255(unsigned x) {
  int b = 1<<23;
  int k = (1<<24)/255;
  return (x*k+b)>>24;
}

GPixel colorToPixel(GPaint p) {
  GColor c = p.getColor();
  float a = fmax(fmin(c.fA, 1),0);
  float r = fmax(fmin(c.fR, 1),0);
  float g = fmax(fmin(c.fG, 1),0);
  float b = fmax(fmin(c.fB, 1),0);
  r = r * a;
  g = g * a;
  b = b * a;
  int a8 = (int) (a*255+0.5);
  int r8 = (int) (r*255+0.5);
  int g8 = (int) (g*255+0.5);
  int b8 = (int) (b*255+0.5);
  return GPixel_PackARGB(a8,r8,g8,b8);
}
#endif
