#include <atomic>

#include "fboworker.hpp"

class TextureNode : public QThread, public QSGSimpleTextureNode
{
  friend class FBOWorker;

  Q_OBJECT

  QMutex mutex_;

  QScopedPointer<QOpenGLContext> context_;
  QOffscreenSurface surface_;

  QScopedPointer<QOpenGLFramebufferObject> fbo_[2];

  unsigned char i_{};
  std::atomic<bool> workFinished_{};

  QQuickItem* item_;

public:
  explicit TextureNode(QQuickItem* const item) :
    QThread(item),
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

public slots:
  void shutdown() noexcept
  {
    if (isRunning())
    {
      QMutexLocker m(&mutex_);

      if (context_)
      {
        context_->makeCurrent(&surface_);

        QOpenGLFramebufferObject::bindDefault();

        fbo_[0].reset();
        fbo_[1].reset();

        context_->doneCurrent();

        context_.reset();
        surface_.destroy();
      }

      exit();
    }
  }

  void work()
  {
    QSize size;

    {
      QMutexLocker m(&mutex_);

      Q_ASSERT(!workFinished_.load(std::memory_order_relaxed));
      i_ = (i_ + 1) % 2;

      size = (rect().size() *
        item_->window()->effectiveDevicePixelRatio()).toSize();
      Q_ASSERT(!size.isEmpty());

      context_->makeCurrent(&surface_);

      if (!fbo_[i_] || (fbo_[i_]->size() != size))
      {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::Depth);

        fbo_[i_].reset(new QOpenGLFramebufferObject(size, format));
      }

      Q_ASSERT(fbo_[i_]->isValid());
      fbo_[i_]->bind();
    }

    {
      QQmlListReference const ref(item_, "resources");

      for (int i{}, c(ref.count()); i != c; ++i)
      {
        QMetaObject::invokeMethod(ref.at(i), "render", Qt::DirectConnection,
          Q_ARG(QSize, size));
      }
    }

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

      node->context_.reset(new QOpenGLContext);

      node->context_->setFormat(f);
      node->context_->setShareContext(ccontext);
      node->context_->create();
      Q_ASSERT(node->context_->isValid());

      node->surface_.setFormat(f);
      node->surface_.create();
      Q_ASSERT(node->surface_.isValid());

      ccontext->makeCurrent(w);

      node->start();

      QMetaObject::invokeMethod(node, "work", Qt::QueuedConnection);

      connect(ccontext, &QOpenGLContext::aboutToBeDestroyed,
        node, &TextureNode::shutdown, Qt::DirectConnection);
    }
    else if (node->workFinished_.load(std::memory_order_relaxed))
    {
      Q_ASSERT(node->fbo_[node->i_]);

      if (size().toSize() == node->rect().size().toSize())
      {
        node->workFinished_.store(false, std::memory_order_relaxed);

        QMetaObject::invokeMethod(node, "work", Qt::QueuedConnection);
      }

      {
        auto& fbo(*node->fbo_[node->i_]);
        auto const texture(fbo.texture());

        if (texture != uint(node->texture()->textureId()))
        {
          node->setTexture(w->createTextureFromId(texture, fbo.size()));
        }
      }
    }

    node->setRect(br);

    update();
  }

  return node;
}

#include "fboworker.moc"
