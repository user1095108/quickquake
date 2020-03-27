#ifndef GLQUAKE_HPP
# define GLQUAKE_HPP
# pragma once

class GLQuake : public QObject
{
  Q_OBJECT

  bool inited_{};

public:
  using QObject::QObject;

  Q_INVOKABLE void render(QSize const&);

  Q_INVOKABLE void keyEvent(int, bool);
};

#endif // QQUAKE_HPP
