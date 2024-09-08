#include <QtGui>
#include <math.h>
#include "scene3D.h"

const static float pi=3.141593, k=pi/180;
const int segments = 8;
GLfloat VertexArray[segments*2+3][3];
GLfloat ColorArray[segments*12][3];
GLubyte IndexArray[segments*4][3];


const GLuint np=segments; // число частей, на которое делится полуокружность
const GLfloat step=pi/np; // шаг изменения углов
QVector<GLfloat> vecVertices; // вектор вершин
QVector<GLfloat> vecTextures; // вектор текстурных координат
QVector<GLuint> vecIndices; // вектор индексов вершин
GLfloat vertices[2*np*np+1][3]; // массив вершин
GLfloat colors[np*np+np][3]; // массив цветов
GLfloat vertices2[2*np*np+1][3]; // массив вершин
GLubyte Index1[segments*4*np][4];
GLubyte Index2[segments*4*np][4];


Scene3D::Scene3D(QWidget* parent) : QGLWidget(parent)
{
   xRot=-90; yRot=0; zRot=0; zTra=0; nSca=1;
}

void Scene3D::initializeGL()
{
   qglClearColor(Qt::white);
   glEnable(GL_DEPTH_TEST);
   glShadeModel(GL_FLAT);
   glEnable(GL_CULL_FACE);

   getVertexArray();
   getColorArray();
   getIndexArray();

   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_COLOR_ARRAY);
}

void Scene3D::resizeGL(int nWidth, int nHeight)
{
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   GLfloat ratio=(GLfloat)nHeight/(GLfloat)nWidth;

   if (nWidth>=nHeight)
      glOrtho(-1.0/ratio, 1.0/ratio, -1.0, 1.0, -10.0, 1.0);
   else
      glOrtho(-1.0, 1.0, -1.0*ratio, 1.0*ratio, -10.0, 1.0);

   glViewport(0, 0, (GLint)nWidth, (GLint)nHeight);
}

void Scene3D::paintGL()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glScalef(nSca, nSca, nSca);
   glTranslatef(0.0f, zTra, 0.0f);
   glRotatef(xRot, 1.0f, 0.0f, 0.0f);
   glRotatef(yRot, 0.0f, 1.0f, 0.0f);
   glRotatef(zRot, 0.0f, 0.0f, 1.0f);

   drawAxis();
   drawFigure();
}

void Scene3D::mousePressEvent(QMouseEvent* pe)
{
   ptrMousePosition = pe->pos();
}

void Scene3D::mouseReleaseEvent(QMouseEvent* pe)
{

}

void Scene3D::mouseMoveEvent(QMouseEvent* pe)
{
   xRot += 180/nSca*(GLfloat)(pe->y()-ptrMousePosition.y())/height();
   zRot += 180/nSca*(GLfloat)(pe->x()-ptrMousePosition.x())/width();

   ptrMousePosition = pe->pos();

   updateGL();
}

void Scene3D::wheelEvent(QWheelEvent* pe)
{
   if ((pe->delta())>0) scale_plus(); else if ((pe->delta())<0) scale_minus();

   updateGL();
}

void Scene3D::keyPressEvent(QKeyEvent* pe)
{
   switch (pe->key())
   {
      case Qt::Key_Plus:
         scale_plus();
      break;

      case Qt::Key_Equal:
         scale_plus();
      break;

      case Qt::Key_Minus:
         scale_minus();
      break;

      case Qt::Key_Up:
         rotate_up();
      break;

      case Qt::Key_Down:
         rotate_down();
      break;

      case Qt::Key_Left:
        rotate_left();
      break;

      case Qt::Key_Right:
         rotate_right();
      break;

      case Qt::Key_Z:
         translate_down();
      break;

      case Qt::Key_X:
         translate_up();
      break;

      case Qt::Key_Space:
         defaultScene();
      break;

      case Qt::Key_Escape:
         this->close();
      break;
   }

   updateGL();
}

void Scene3D::scale_plus()
{
   nSca = nSca*1.1;
}

void Scene3D::scale_minus()
{
   nSca = nSca/1.1;
}

void Scene3D::rotate_up()
{
   xRot += 1.0;
}

void Scene3D::rotate_down()
{
   xRot -= 1.0;
}

void Scene3D::rotate_left()
{
   zRot += 1.0;
}

void Scene3D::rotate_right()
{
   zRot -= 1.0;
}

void Scene3D::translate_down()
{
   zTra -= 0.05;
}

void Scene3D::translate_up()
{
   zTra += 0.05;
}

void Scene3D::defaultScene()
{
   xRot=-90; yRot=0; zRot=0; zTra=0; nSca=1;
}

void Scene3D::drawAxis()
{
   glLineWidth(3.0f);

   glColor4f(1.00f, 0.00f, 0.00f, 1.0f);
   glBegin(GL_LINES);
      glVertex3f( 1.0f,  0.0f,  0.0f);
      glVertex3f(-1.0f,  0.0f,  0.0f);
   glEnd();

   QColor halfGreen(0, 128, 0, 255);
   qglColor(halfGreen);
   glBegin(GL_LINES);
      glVertex3f( 0.0f,  1.0f,  0.0f);
      glVertex3f( 0.0f, -1.0f,  0.0f);

      glColor4f(0.00f, 0.00f, 1.00f, 1.0f);
      glVertex3f( 0.0f,  0.0f,  1.0f);
      glVertex3f( 0.0f,  0.0f, -1.0f);
   glEnd();
}

void Scene3D::getVertexArray()
{
   const GLfloat phiX = pi / 12;  // Угол наклона нижнего эллипса
   const GLfloat phiY = pi / 6;  // Угол наклона верхнего эллипса

   const GLfloat height = 1;          // Высота между основаниями

   const GLfloat upperHeight = 2*height;
   const GLfloat lowerHeight = height;

   const GLfloat upperRadiusX = tan(phiX)*2*height;  // Полуось X верхнего основания (больший эллипс)
   const GLfloat upperRadiusY = tan(phiY)*2*height;  // Полуось Y верхнего основания (больший эллипс)
   const GLfloat lowerRadiusX = tan(phiX)*height;  // Полуось X нижнего основания (меньший эллипс)
   const GLfloat lowerRadiusY = tan(phiY)*height;  // Полуось Y нижнего основания (меньший эллипс)






   const GLfloat Dmax = 2* height; // Высота усеченного конуса
   const GLfloat Dmin = height; // Высота проведения сферы
   const GLfloat alpha= phiX;
   const GLfloat beta= phiY;
   const GLfloat step_alpha = alpha/np;
   const GLfloat step_beta = beta/np;
   const GLfloat step_phi = 2*pi/np;
   GLfloat current_alpha = step_alpha;
   GLfloat current_beta = step_beta;

   vertices[0][0] = 0.0f; // x
   vertices[0][1] = 0.0f; // y
   vertices[0][2] = Dmax;   // z

   vertices2[0][0] = 0.0f; // x
   vertices2[0][1] = 0.0f; // y
   vertices2[0][2] = Dmin;   // z

   for(int i = 1; i <= np; i++, current_alpha += step_alpha, current_beta += step_beta) {
       double current_phi = 0;
       for (int j=0 ; current_phi < 2*pi ; j++, current_phi += step_phi){   
               vertices[(i-1)*np+j+1][0] = Dmax * sin(current_alpha) * cos(current_phi); // x
               vertices[(i-1)*np+j+1][1] = Dmax * sin(current_beta) * sin(current_phi); // y
               vertices[(i-1)*np+j+1][2] = sqrt(Dmax * Dmax - (Dmax * sin(current_alpha) * cos(current_phi) * Dmax * sin(current_alpha) * cos(current_phi) + Dmax * sin(current_beta) * sin(current_phi) * Dmax * sin(current_beta) * sin(current_phi)));

               vertices2[(i - 1) * np + j + 1][0] = Dmin * sin(current_alpha) * cos(current_phi); // x
               vertices2[(i - 1) * np + j + 1][1] = Dmin * sin(current_beta) * sin(current_phi); // y
               vertices2[(i - 1) * np + j + 1][2] = sqrt(Dmin * Dmin - (vertices2[(i - 1) * np + j + 1][0] * vertices2[(i - 1) * np + j + 1][0] + vertices2[(i - 1) * np + j + 1][1] * vertices2[(i - 1) * np + j + 1][1])); // z

       }
   }
   double current_phi = 0;

   for (int j=0 ; current_phi < 2*pi ; j++, current_phi += step_phi){


       // Вершины нижнего основания
       VertexArray[j][0] = Dmin * sin(current_alpha) * cos(current_phi);
       VertexArray[j][1] = Dmin * sin(current_beta)  * sin(current_phi);
       VertexArray[j][2] = sqrt(Dmin * Dmin - (Dmin * sin(current_alpha) * cos(current_phi) * Dmin * sin(current_alpha) * cos(current_phi) + Dmin * sin(current_beta) * sin(current_phi) * Dmin * sin(current_beta) * sin(current_phi)));

       // Вершины верхнего основания
       VertexArray[segments + j][0] = Dmax * sin(alpha) * cos(current_phi);
       VertexArray[segments + j][1] = Dmax * sin(beta) * sin(current_phi);
       VertexArray[segments + j][2] = sqrt(Dmax * Dmax - (Dmax * sin(current_alpha) * cos(current_phi) * Dmax * sin(current_alpha) * cos(current_phi) + Dmax * sin(current_beta) * sin(current_phi) * Dmax * sin(current_beta) * sin(current_phi)));
   }

   // Центр нижнего основания
   VertexArray[segments * 2][0] = 0.0;
   VertexArray[segments * 2][1] = 0.0;
   VertexArray[segments * 2][2] = Dmin;

   // Центр верхнего основания
   VertexArray[segments * 2 + 1][0] = 0.0;
   VertexArray[segments * 2 + 1][1] = 0.0;
   VertexArray[segments * 2 + 1][2] = Dmax;

}




void Scene3D::getIndexArray()
{
    // Индексы для боковой поверхности
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;

        IndexArray[2 * i][0] = i;
        IndexArray[2 * i][1] = segments + i;
        IndexArray[2 * i][2] = segments + next;

        IndexArray[2 * i + 1][0] = i;
        IndexArray[2 * i + 1][1] = segments + next;
        IndexArray[2 * i + 1][2] = next;
    }

//np=8
// Индексы для треугольников, сходящихся к верхней вершине (0,0,Dmax)
    for (int j = 0; j < np; ++j) {
         int next_j = (j + 1) % (np);
         Index1[j][0] = 0; // Верхняя вершина (0,0,Dmax)
         Index1[j][1] = 0; // Текущая вершина верхнего основания
         Index1[j][2] =  next_j+1; // Следующая вершина верхнего основания
         Index1[j][3] = j+1; // Следующая вершина верхнего основания
    }


    for (int i = 1; i < np; ++i) {
           for (int j = 0; j < np; ++j) {
               int next_j = (j + 1) % np;
               Index1[i * np + j][0] = (i-1) * np + j+1 ; // Текущая верхняя вершина
               Index1[i * np + j][1] = (i-1) * np + next_j+1 ; // Следующая верхняя вершина
               Index1[i * np + j][2] = (i) * np + next_j+1; // Следующая нижняя вершина
               Index1[i * np + j][3] = (i ) * np + j+1   ; // Текущая нижняя вершина
           }
       }

    for (int j = 0; j < np; ++j) {
         int next_j = (j + 1) % (np);
         Index2[j][0] = 0; // Верхняя вершина (0,0,Dmax)
         Index2[j][1] = 0; // Текущая вершина верхнего основания
         Index2[j][2] =  j+1; // Следующая вершина верхнего основания
         Index2[j][3] = next_j+1; // Следующая вершина верхнего основания
    }


    for (int i = 1; i < np; ++i) {
           for (int j = 0; j < np; ++j) {
               int next_j = (j + 1) % np;
               Index2[i * np + j][0] = (i -1 ) * np + next_j+1 ; // Текущая верхняя вершина
               Index2[i * np + j][1] =(i -1) * np + j+1  ; // Следующая верхняя вершина
               Index2[i * np + j][2] =(i ) * np + j+1 ; // Следующая нижняя вершина
               Index2[i * np + j][3] = (i  ) * np + next_j+1   ; // Текущая нижняя вершина
           }
       }














}





void Scene3D::getColorArray()
{


    // Задаем случайные цвета для боковой поверхности и вершин оснований
    for (int i = 0; i < 2 * segments + 2; ++i) {
        ColorArray[i][0] = 0.1f * (qrand() % 14);
        ColorArray[i][1] = 0.1f * (qrand() % 14);
        ColorArray[i][2] = 0.1f * (qrand() % 14);
    }
    // Задаем случайные цвета для каждой вершины
    for (GLuint i = 0; i < np*np+np; ++i) {

            colors[i][0] = 0.1f * (rand() % 11); // Красный компонент цвета
            colors[i][1] = 0.1f * (rand() % 11); // Зеленый компонент цвета
            colors[i][2] = 0.1f * (rand() % 11); // Синий компонент цвета

        }


}




void Scene3D::drawFigure()
{


   glVertexPointer(3, GL_FLOAT, 0, VertexArray);
   glColorPointer(3, GL_FLOAT, 0, ColorArray);
   glDrawElements(GL_TRIANGLES, segments*12, GL_UNSIGNED_BYTE, IndexArray);

   glVertexPointer(3, GL_FLOAT, 0, vertices);
  glColorPointer(3, GL_FLOAT, 0, colors);
   glDrawElements(GL_QUADS, np*np*6, GL_UNSIGNED_BYTE, Index1);

   glVertexPointer(3, GL_FLOAT, 0, vertices2);
  glColorPointer(3, GL_FLOAT, 0, colors);
   glDrawElements(GL_QUADS, np*np*6, GL_UNSIGNED_BYTE, Index2);






}
