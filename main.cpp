#if defined(_WIN32)
#  define _USE_MATH_DEFINES
#  define _CRT_SECURE_NO_WARNINGS
#  include <GL/glut.h>
#  include "glext.h"
#elif defined(__APPLE__) || defined(MACOSX)
#  define GL_SILENCE_DEPRECATION
#  include <GLUT/glut.h>
#  include <GL/glext.h>
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/glut.h>
#  include <GL/glext.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <cmath>

/*
** トラックボール処理用関数の宣言
*/
#include "trackball.h"

/*
** 形状データ
*/
#include "Obj.h"
Obj *data;

/*
** 視点の距離
*/
static double offset = -200.0;

/*
** 光源
*/
static const GLfloat lightpos[] = { 0.0, 0.0, 1.0, 0.0 };  /* 位置 */

/*
** テクスチャ
*/
#define TEXWIDTH  256                      /* テクスチャの幅　　　 */
#define TEXHEIGHT 256                      /* テクスチャの高さ　　 */
static const char texture1[] = "toon.raw"; /* テクスチャファイル名 */

/*
** 初期化
*/
static void init(void)
{
  /* テクスチャの読み込みに使う配列 */
  GLubyte texture[TEXHEIGHT][TEXWIDTH][3];
  FILE *fp;
  
  /* テクスチャ画像の読み込み */
  if ((fp = fopen(texture1, "rb")) != NULL) {
    fread(texture, sizeof texture, 1, fp);
    fclose(fp);
  }
  else {
    perror(texture1);
  }

  /* テクスチャ画像はバイト単位に詰め込まれている */
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  
  /* テクスチャの割り当て */
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXWIDTH, TEXHEIGHT, 0,
    GL_RGB, GL_UNSIGNED_BYTE, texture);
    
  /* テクスチャを拡大・縮小する方法の指定 */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  /* テクスチャの繰り返しの指定 */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  /* テクスチャ環境 */
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  /* 初期設定 */
  glClearColor(0.3f, 0.3f, 1.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  
  /* 光源の初期設定 */
  glDisable(GL_LIGHTING);

  /* 形状データオブジェクトの作成 */
  data = new Obj;

  /* 形状データの読み込み */
  data->load("bunny.obj");
}

/*
** シーンの描画
*/
void scene(void)
{
  GLfloat eyepos[] = { 0.0f, 0.0f, (GLfloat)-offset, 1.0f };
  GLfloat lpos[4], epos[4];
  const double *rt;
  int i, j;

  /*
  ** 回転の変換行列を取り出す
  */
  rt = trackballRotation();

  /*
  ** 光源を物体の回転と逆方向に回転する（回転行列の転置行列をかける）
  */
  for (i = 0; i < 4; ++i) {
    lpos[i] = 0.0;
    for (j = 0; j < 4; ++j) {
      lpos[i] += (GLfloat)(lightpos[j] * rt[i * 4 + j]);
    }
  }

  /*
  ** 視点を物体の回転と逆方向に回転する（回転行列の転置行列をかける）
  */
  for (i = 0; i < 4; ++i) {
    epos[i] = 0.0;
    for (j = 0; j < 4; ++j) {
      epos[i] += (GLfloat)(eyepos[j] * rt[i * 4 + j]);
    }
  }

  data->calcTexCoord(lpos, epos);
  data->draw();
}

/****************************
** GLUT のコールバック関数 **
****************************/

static void display(void)
{
  /* モデルビュー変換行列の設定 */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  /* 視点の移動（物体の方を奥に移動）*/
  glTranslated(0.0, 0.0, offset);
  
  /* 光源の位置を設定 */
  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
  
  /* トラックボール処理による回転 */
  glMultMatrixd(trackballRotation());
  
  /* 画面クリア */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  /* シーンの描画 */
  scene();
  
  /* ダブルバッファリング */
  glutSwapBuffers();
}

static void resize(int w, int h)
{
  /* トラックボールする範囲 */
  trackballRegion(w, h);
  
  /* ウィンドウ全体をビューポートにする */
  glViewport(0, 0, w, h);
  
  /* 透視変換行列の指定 */
  glMatrixMode(GL_PROJECTION);
  
  /* 透視変換行列の初期化 */
  glLoadIdentity();
  gluPerspective(60.0, (double)w / (double)h, 1.0, 500.0);
}

static void idle(void)
{
  /* 画面の描き替え */
  glutPostRedisplay();
}

static void mouse(int button, int state, int x, int y)
{
  switch (button) {
  case GLUT_LEFT_BUTTON:
    switch (state) {
    case GLUT_DOWN:
      /* トラックボール開始 */
      trackballStart(x, y);
      glutIdleFunc(idle);
      break;
    case GLUT_UP:
      /* トラックボール停止 */
      glutIdleFunc(0);
      trackballStop(x, y);
      break;
    default:
      break;
    }
    break;
    default:
      break;
  }
}

static void motion(int x, int y)
{
  /* トラックボール移動 */
  trackballMotion(x, y);
}

static void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
  case '+':
    offset += 10.0;
    glutPostRedisplay();
    break;
  case '-':
    offset -= 10.0;
    glutPostRedisplay();
    break;
  case 'q':
  case 'Q':
  case '\033':
    /* ESC か q か Q をタイプしたら終了 */
    exit(0);
  default:
    break;
  }
}

/*
** メインプログラム
*/
int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow(argv[0]);
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboard);
  init();
  glutMainLoop();
  return 0;
}
