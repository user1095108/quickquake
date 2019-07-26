int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);

  QQuickView view;
  view.connect(view.engine(), &QQmlEngine::quit, &app, &QCoreApplication::quit);

  view.show();

  return app.exec();
}
