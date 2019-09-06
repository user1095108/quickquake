#include "fboworker.hpp"

class TextureNode : public QThread, public QSGSimpleTextureNode
{
  friend class FBOWorker;

  Q_OBJECT

  QMutex mutex_;

  QOffscreenSurface surface_;
  QScopedPointer<QOpenGLContext> context_;

  QScopedPointer<QOpenGLFramebufferObject> fbo_;

  QScopedPointer<QSGTexture> texture_;

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
    wait();
  }

  void shutdown() noexcept
  {
    if (isRunning())
    {
      QMutexLocker m(&mutex_);

      if (context_)
      {
        context_->makeCurrent(&surface_);

        QOpenGLFramebufferObject::bindDefault();

        fbo_.reset();

        context_->doneCurrent();

        context_.reset();
        surface_.destroy();
      }

      exit();
    }
  }

  Q_INVOKABLE void work()
  {
    QSize size;

    {
      QMutexLocker m(&mutex_);

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

    context_->functions()->glFinish();

    {
      QMutexLocker m(&mutex_);

      texture_.reset(item_->window()->createTextureFromId(fbo_->takeTexture(),
          size,
          QQuickWindow::TextureOwnsGLTexture
        )
      );
    }
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

  {
    QMutexLocker l(&node->mutex_);

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
    else if (node->texture_ &&
      (size().toSize() == node->rect().size().toSize()))
    {
      node->setTexture(node->texture_.take());
      node->setRect(br);

      QMetaObject::invokeMethod(node, "work", Qt::QueuedConnection);
    }
    else
    {
      node->setRect(br);
    }

    update();
  }

  return node;
}

#include "fboworker.moc"
