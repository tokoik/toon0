#ifndef OBJ_H
#define OBJ_H

typedef float vec[3];
typedef float tex[2];
typedef unsigned int idx[3];

class Obj {
  int nv, nf;
  vec *vert, *norm, *fnorm;
  tex *texc;
  idx *face;
  void copy(const Obj &);
public:
  Obj(void);
  Obj(const Obj &);
  virtual ~Obj();
  Obj &operator=(const Obj &);
  int load(const char *);
  void calcTexCoord(const GLfloat *, const GLfloat *);
  void draw(void);
};

#endif
