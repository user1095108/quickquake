#include "fboworker.hpp"
#include "glquake.hpp"

int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);
  //app.setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);
  //app.setAttribute(Qt::AA_ShareOpenGLContexts);

  qputenv("QSG_RHI_BACKEND", QByteArray("opengl"));

  qmlRegisterType<FBOWorker>("Quake", 1, 0, "FBOWorker");
  qmlRegisterType<GLQuake>("Quake", 1, 0, "GLQuake");

  //
  QQmlApplicationEngine engine;
  engine.load(QStringLiteral("qrc:///main.qml"));

  return app.exec();
}

//view.setPersistentOpenGLContext(true);
//view.setPersistentSceneGraph(true);
