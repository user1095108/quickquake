int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);

  QQuickView view;

  view.connect(view.engine(), &QQmlEngine::quit, &app, &QCoreApplication::quit);

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
