#include "fboworker.hpp"

class TextureNode : public QThread, public QSGSimpleTextureNode
{
  Q_OBJECT

  friend class FBOWorker;

  std::atomic<bool> workFinished_{};
  QScopedPointer<QSGTexture> texture_;

  QOffscreenSurface surface_;
  QScopedPointer<QOpenGLContext> context_;

  QScopedPointer<QOpenGLFramebufferObject> fbo_;

  QQuickItem* item_;

public:
  explicit TextureNode(QQuickItem* const item) :
    item_(item)
  {
    setFiltering(QSGTexture::Nearest);
    setOwnsTexture(true);
    setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);

    setTexture(item_->window()->createTextureFromId(0, QSize(1, 1)));
  }

  ~TextureNode() noexcept
  {
    shutdown();
  }

  void shutdown() noexcept
  {
    if (isRunning())
    {
      exit();
      wait();

      if (context_)
      {
        context_.reset();
        surface_.destroy();

        fbo_.reset();
      }
    }
  }

  Q_INVOKABLE void work()
  {
    Q_ASSERT(!workFinished_.load(std::memory_order_relaxed));
    QSize size;

    {
      size = (item_->size() *
        item_->window()->effectiveDevicePixelRatio()).toSize();
      Q_ASSERT(!size.isEmpty());

      context_->makeCurrent(&surface_);

      if (!fbo_ || (fbo_->size() != size))
      {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::Depth);

        fbo_.reset(new QOpenGLFramebufferObject(size, format));
      }

      Q_ASSERT(fbo_->isValid());
      fbo_->bind();

      //context_->functions()->glViewport(0, 0, size.width(), size.height());

/*
      {
        QOpenGLPaintDevice opd(size);
        QPainter painter(&opd);

        painter.fillRect(0, 0, size.width(), size.height(), Qt::yellow);

        painter.setPen(Qt::red);
        painter.drawLine(0, 0, size.width(), size.height());
      }
*/
    }

    {
      QQmlListReference const ref(item_, "resources");

      for (int i{}, c(ref.count()); i != c; ++i)
      {
        QMetaObject::invokeMethod(ref.at(i), "render", Qt::DirectConnection,
          Q_ARG(QSize, size));
      }
    }

    //context_->functions()->glFinish();

    texture_.reset(
      item_->window()->createTextureFromId(fbo_->takeTexture(),
        size,
        QQuickWindow::TextureOwnsGLTexture
      )
    );

    workFinished_.store(true, std::memory_order_relaxed);
  }
};

//////////////////////////////////////////////////////////////////////////////
FBOWorker::FBOWorker(QQuickItem* const parent) :
  QQuickItem(parent)
{
  setFlag(ItemHasContents);
}

//////////////////////////////////////////////////////////////////////////////
void FBOWorker::setContextProfile(
  QSurfaceFormat::OpenGLContextProfile const e)
{
  if (e != contextProfile_)
  {
    contextProfile_ = e;

    emit contextProfileChanged();
  }
}

//////////////////////////////////////////////////////////////////////////////
QSGNode* FBOWorker::updatePaintNode(QSGNode* const n,
  QQuickItem::UpdatePaintNodeData*)
{
  auto const br(boundingRect());

  if (br.isEmpty())
  {
    return delete n, nullptr;
  }

  auto const w(window());
  Q_ASSERT(w);

  auto node(static_cast<TextureNode*>(n));

  if (!node)
  {
    node = new TextureNode(this);

    connect(w, &QQuickWindow::sceneGraphInvalidated,
      node, &TextureNode::shutdown, Qt::DirectConnection);
  }

  if (!node->context_)
  {
    auto const ccontext(w->openglContext());
    Q_ASSERT(ccontext);

    // this is done to safely share context resources
    ccontext->doneCurrent();

    auto f(ccontext->format());
    f.setProfile(contextProfile_);

    {
      node->context_.reset(new QOpenGLContext);
      auto& context(*node->context_);

      context.setFormat(f);
      context.setShareContext(ccontext);
      context.create();
      Q_ASSERT(context.isValid());

      auto& surface(node->surface_);

      surface.setFormat(f);
      surface.create();
      Q_ASSERT(surface.isValid());
    }

    ccontext->makeCurrent(w);

    connect(ccontext, &QOpenGLContext::aboutToBeDestroyed,
      node, &TextureNode::shutdown, Qt::DirectConnection);

    node->start();

    QMetaObject::invokeMethod(node, "work", Qt::QueuedConnection);
  }
  else if (node->workFinished_.load(std::memory_order_relaxed))
  {
    if (br == node->rect())
    {
      node->workFinished_.store(false, std::memory_order_relaxed);
      node->setTexture(node->texture_.take());

      QMetaObject::invokeMethod(node, "work", Qt::QueuedConnection);
    }
    else
    {
      node->setRect(br);
    }
  }

  update();

  return node;
}

#include "fboworker.moc"
