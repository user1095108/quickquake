#include "fboworker.hpp"
#include "glquake.hpp"

int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);
  //app.setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);

  qputenv("QSG_RHI_BACKEND", QByteArray("opengl"));

  qmlRegisterType<FBOWorker>("Quake", 1, 0, "FBOWorker");
  qmlRegisterType<GLQuake>("Quake", 1, 0, "GLQuake");

  QQuickView view;

//view.setPersistentOpenGLContext(true);
//view.setPersistentSceneGraph(true);

  view.setResizeMode(QQuickView::SizeRootObjectToView);
  view.resize(640, 480);

  view.setColor(Qt::black);
  view.setSource(QUrl(QStringLiteral("qrc:main.qml")));

  return QQuickView::Error == view.status() ?
    EXIT_FAILURE : (view.show(), app.exec());
}
