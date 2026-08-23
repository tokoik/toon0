#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>

#if defined(WIN32)
#  include "glut.h"
#elif defined(__APPLE__) || defined(MACOSX)
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include "Obj.h"

/*
** オブジェクトのコピー
*/
void Obj::copy(const Obj &o)
{
  nv = o.nv;
  nf = o.nf;

  try {
    if (nv > 0) {
      vert = new vec[nv];
      norm = new vec[nv];
      texc = new tex[nv];

      memcpy(vert, o.vert, sizeof(vec) * nv);
      memcpy(norm, o.norm, sizeof(vec) * nv);
      memcpy(texc, o.texc, sizeof(tex) * nv);
    }
    else {
      vert = norm = 0;
      texc = 0;
    }
    if (nf > 0) {
      fnorm = new vec[nf];
      face = new idx[nf];

      memcpy(fnorm, o.fnorm, sizeof(vec) * nf);
      memcpy(face, o.face, sizeof(idx) * nf);
    }
    else {
      fnorm = 0;
      face = 0;
    }
  }
  catch (std::bad_alloc e) {
    std::cerr << "メモリが足りません"<< std::endl;
    abort();
  }
}

/*
** コンストラクタ
*/
Obj::Obj(void)
{
  nv = nf = 0;
  vert = norm = fnorm = 0;
  texc = 0;
  face = 0;
}

/*
** コピーコンストラクタ
*/
Obj::Obj(const Obj &o)
{
  copy(o);
}

/*
** デストラクタ
*/
Obj::~Obj()
{
  if (vert) delete[] vert;
  if (norm) delete[] norm;
  if (texc) delete[] texc;
  if (fnorm) delete[] fnorm;
  if (face) delete[] face;
}

/*
** 代入演算子
*/
Obj &Obj::operator=(const Obj &o)
{
  if (this != &o) {
    this->~Obj();
    copy(o);
  }
  return *this;
}

/*
** ファイルの読み込み
*/
int Obj::load(char *name)
{
  std::ifstream file(name, std::ios::in | std::ios::binary);
  char buf[1024];
  int i, v, f;

  if (!file) {
    std::cerr << name << " が開けません" << std::endl;
    return 1;
  }

  /* データの数を調べる */
  v = f = 0;
  while (file.getline(buf, sizeof buf)) {
    if (buf[0] == 'v' && buf[1] == ' ') {
      ++v;
    }
    else if (buf[0] == 'f' && buf[1] == ' ') {
      ++f;
    }
  }

  nv = v;
  nf = f;

  try {
    vert = new vec[v];
    norm = new vec[v];
    texc = new tex[v];
    fnorm = new vec[f];
    face = new idx[f];
  }
  catch (std::bad_alloc e) {
    std::cerr << "メモリが足りません" << std::endl;
    abort();
  }

  /* ファイルの巻き戻し */
  file.clear();
  file.seekg(0L, std::ios::beg);

  /* データの読み込み */
  v = f = 0;
  while (file.getline(buf, sizeof buf)) {
    if (buf[0] == 'v' && buf[1] == ' ') {
      sscanf(buf, "%*s %f %f %f", vert[v], vert[v] + 1, vert[v] + 2);
      ++v;
    }
    else if (buf[0] == 'f' && buf[1] == ' ') {
      if (sscanf(buf + 2, "%d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", face[f], face[f] + 1, face[f] + 2) != 3) {
        if (sscanf(buf + 2, "%d//%*d %d//%*d %d//%*d", face[f], face[f] + 1, face[f] + 2) != 3) {
          sscanf(buf + 2, "%d %d %d", face[f], face[f] + 1, face[f] + 2);
        }
      }
      --face[f][0];
      --face[f][1];
      --face[f][2];
      ++f;
    }
  }

  /* 面法線ベクトルの算出 */
  for (i = 0; i < f; ++i) {
    float dx1 = vert[face[i][1]][0] - vert[face[i][0]][0];
    float dy1 = vert[face[i][1]][1] - vert[face[i][0]][1];
    float dz1 = vert[face[i][1]][2] - vert[face[i][0]][2];
    float dx2 = vert[face[i][2]][0] - vert[face[i][0]][0];
    float dy2 = vert[face[i][2]][1] - vert[face[i][0]][1];
    float dz2 = vert[face[i][2]][2] - vert[face[i][0]][2];

    fnorm[i][0] = dy1 * dz2 - dz1 * dy2;
    fnorm[i][1] = dz1 * dx2 - dx1 * dz2;
    fnorm[i][2] = dx1 * dy2 - dy1 * dx2;
  }

  /* 頂点の仮想法線ベクトルの算出 */
  for (i = 0; i < v; ++i) {
    norm[i][0] = norm[i][1] = norm[i][2] = 0.0;
  }
  
  for (i = 0; i < f; ++i) {
    norm[face[i][0]][0] += fnorm[i][0];
    norm[face[i][0]][1] += fnorm[i][1];
    norm[face[i][0]][2] += fnorm[i][2];

    norm[face[i][1]][0] += fnorm[i][0];
    norm[face[i][1]][1] += fnorm[i][1];
    norm[face[i][1]][2] += fnorm[i][2];

    norm[face[i][2]][0] += fnorm[i][0];
    norm[face[i][2]][1] += fnorm[i][1];
    norm[face[i][2]][2] += fnorm[i][2];
  }

  /* 頂点の仮想法線ベクトルの正規化 */
  for (i = 0; i < v; ++i) {
    float a = sqrt(norm[i][0] * norm[i][0]
                 + norm[i][1] * norm[i][1]
                 + norm[i][2] * norm[i][2]);

    if (a != 0.0) {
      norm[i][0] /= a;
      norm[i][1] /= a;
      norm[i][2] /= a;
    }
  }

  return 0;
}

/*
** テクスチャ座標の計算
*/
void Obj::calcTexCoord(const GLfloat *lpos, const GLfloat *epos)
{
  GLfloat lp[3], lv[3], ep[3], ev[3];

  /*
  ** 光源位置／ベクトルの算出
  */
  if (lpos[3] != 0.0) {
    /* 実座標を求める */
    lp[0] = lpos[0] / lpos[3];
    lp[1] = lpos[1] / lpos[3];
    lp[2] = lpos[2] / lpos[3];
  }
  else {
    /* 光源方向の単位ベクトルを求める */
    lv[0] = lpos[0];
    lv[1] = lpos[1];
    lv[2] = lpos[2];
    GLfloat a = sqrtf(lv[0] * lv[0] + lv[1] * lv[1] + lv[2] * lv[2]);
    if (a != 0.0) {
      lv[0] /= a;
      lv[1] /= a;
      lv[2] /= a;
    }
  }

  /*
  ** 視点位置／ベクトルの算出
  */
  if (epos[3] != 0.0) {
    /* 実座標を求める */
    ep[0] = epos[0] / epos[3];
    ep[1] = epos[1] / epos[3];
    ep[2] = epos[2] / epos[3];
  }
  else {
    /* 視線方向の単位ベクトルを求める */
    ev[0] = epos[0];
    ev[1] = epos[1];
    ev[2] = epos[2];
    GLfloat a = sqrtf(ev[0] * ev[0] + ev[1] * ev[1] + ev[2] * ev[2]);
    if (a != 0.0) {
      ev[0] /= a;
      ev[1] /= a;
      ev[2] /= a;
    }
  }

  /*
  ** テクスチャ座標の算出
  */
  for (int i = 0; i < nv; ++i) {

    if (lpos[3] != 0.0) {
      /* 頂点ごとに光源方向の単位ベクトルを求める */
      lv[0] = lp[0] - vert[i][0];
      lv[1] = lp[1] - vert[i][1];
      lv[2] = lp[2] - vert[i][2];
      GLfloat a = sqrtf(lv[0] * lv[0] + lv[1] * lv[1] + lv[2] * lv[2]);
      if (a != 0.0) {
        lv[0] /= a;
        lv[1] /= a;
        lv[2] /= a;
      }
    }

    if (epos[3] != 0.0) {
      /* 頂点ごとに視線方向の単位ベクトルを求める */
      ev[0] = ep[0] - vert[i][0];
      ev[1] = ep[1] - vert[i][1];
      ev[2] = ep[2] - vert[i][2];
      GLfloat a = sqrtf(ev[0] * ev[0] + ev[1] * ev[1] + ev[2] * ev[2]);
      if (a != 0.0) {
        ev[0] /= a;
        ev[1] /= a;
        ev[2] /= a;
      }
    }

    /* テクスチャの s 座標は明度 */
    texc[i][0] = norm[i][0] * lv[0] + norm[i][1] * lv[1] + norm[i][2] * lv[2];

    /* テクスチャの t 座標はエッジ */
    texc[i][1] = norm[i][0] * ev[0] + norm[i][1] * ev[1] + norm[i][2] * ev[2];
  }
}

/*
** 図形の表示
*/
void Obj::draw(void)
{
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  
  glNormalPointer(GL_FLOAT, 0, norm);
  glVertexPointer(3, GL_FLOAT, 0, vert);
  glTexCoordPointer(2, GL_FLOAT, 0, texc);
  
  glEnable(GL_TEXTURE_2D);

  glDrawElements(GL_TRIANGLES, nf * 3, GL_UNSIGNED_INT, face);

  glDisable(GL_TEXTURE_2D);

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}
