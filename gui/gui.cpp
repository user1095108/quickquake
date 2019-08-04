#include "fboworker.hpp"
#include "glquake.hpp"

int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);

  qmlRegisterType<FBOWorker>("Quake", 1, 0, "FBOWorker");
  qmlRegisterType<GLQuake>("Quake", 1, 0, "GLQuake");

  QQuickView view;

//view.setPersistentOpenGLContext(true);
//view.setPersistentSceneGraph(true);

  view.setResizeMode(QQuickView::SizeRootObjectToView);
  view.resize(640, 480);

  view.setSource(QUrl(QStringLiteral("qrc:main.qml")));

  if (QQuickView::Error == view.status())
  {
    return EXIT_FAILURE;
  }

  view.show();

  return app.exec();
}
