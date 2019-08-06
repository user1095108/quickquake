#include "fboworker.hpp"

//////////////////////////////////////////////////////////////////////////////
class FBOWorker::Renderer : public QQuickFramebufferObject::Renderer
{
public:
  QQuickItem* item_;

  Renderer(QQuickItem* const item) noexcept : item_(item)
  {
  }

  QOpenGLFramebufferObject* createFramebufferObject(QSize const& size)
  {
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::Depth);

    return new QOpenGLFramebufferObject(size, format);
  }

  void render() final
  {
    auto const w(item_->window());
    Q_ASSERT(w);

    auto const ccontext(w->openglContext());
    Q_ASSERT(ccontext);

    ccontext->functions()->glUseProgram(0);

    auto const size(framebufferObject()->size());

    {
      QQmlListReference const ref(item_, "resources");

      for (int i{}, c(ref.count()); i != c; ++i)
      {
        QMetaObject::invokeMethod(ref.at(i), "render", Qt::DirectConnection,
          Q_ARG(QSize, size));
      }
    }

    update();
  }
};

//////////////////////////////////////////////////////////////////////////////
QQuickFramebufferObject::Renderer* FBOWorker::createRenderer() const
{
  return new FBOWorker::Renderer(const_cast<FBOWorker*>(this));
}
