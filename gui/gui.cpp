#include "fboworker.hpp"
#include "glquake.hpp"

//view.setPersistentOpenGLContext(true);
//view.setPersistentSceneGraph(true);

int main(int argc, char* argv[])
{
  //QCoreApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);
  //QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

  QGuiApplication app(argc, argv);

  qputenv("QSG_RHI_BACKEND", QByteArray("opengl"));

  qmlRegisterType<FBOWorker>("Quake", 1, 0, "FBOWorker");
  qmlRegisterType<GLQuake>("Quake", 1, 0, "GLQuake");

  //
  QQmlApplicationEngine engine;
  engine.load(QStringLiteral("qrc:///main.qml"));

  return app.exec();
}
