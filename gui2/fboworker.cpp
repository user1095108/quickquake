#include "fboworker.hpp"

//////////////////////////////////////////////////////////////////////////////
QOpenGLFramebufferObject*
FBOWorker::Renderer::createFramebufferObject(QSize const& size)
{
  QScopedPointer<QOpenGLFramebufferObject> fbo(
    new QOpenGLFramebufferObject(size));
}

//////////////////////////////////////////////////////////////////////////////
QQuickFramebufferObject::Renderer*
QQuickFramebufferObject::createRenderer() const
{
}
