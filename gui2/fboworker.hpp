#ifndef FBOWORKER_HPP
# define FBOWORKER_HPP
# pragma once

class FBOWorker : public QQuickFramebufferObject
{
  Q_OBJECT

public:
  class Renderer;

  using QQuickFramebufferObject::QQuickFramebufferObject;

private:
  QQuickFramebufferObject::Renderer* createRenderer() const final;
};

#endif // FBOWORKER_HPP
