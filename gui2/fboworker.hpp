#ifndef FBOWORKER_HPP
# define FBOWORKER_HPP
# pragma once

class FBOWorker : public QQuickFramebufferObject
{
  class Renderer : QQuickFramebufferObject::Renderer;

public:
  using QQuickItem::QQuickItem;

private:
  QQuickFramebufferObject::Renderer*
    QQuickFramebufferObject::createRenderer() const final;
};

#endif // FBOWORKER_HPP
