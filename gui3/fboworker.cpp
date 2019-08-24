#include <atomic>

#include "fboworker.hpp"

class TextureNode : public QThread, public QSGSimpleTextureNode
{
  friend class FBOWorker;

  Q_OBJECT

  QMutex mutex_;

  QScopedPointer<QOpenGLContext> context_;
  QOffscreenSurface surface_;

  struct
  {
    QScopedPointer<QOpenGLFramebufferObject> fbo;
    QScopedPointer<QSGTexture> texture;
  } fbo_[2];

  unsigned char i_{};
  std::atomic<bool> workFinished_{};

  QQuickItem* item_;

public:
  explicit TextureNode(QQuickItem* const item) :
    QThread(item),
    item_(item)
  {
    setFiltering(QSGTexture::Nearest);
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

        fbo_[0].fbo.reset();
        fbo_[1].fbo.reset();

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

      if (auto& fbo(fbo_[i_]); !fbo.fbo || (fbo.fbo->size() != size))
      {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::Depth);

        fbo.fbo.reset(new QOpenGLFramebufferObject(size, format));
        fbo.texture.reset(item_->window()->
          createTextureFromId(fbo.fbo->texture(), size));
      }

      Q_ASSERT(fbo_[i_].fbo->isValid());
      fbo_[i_].fbo->bind();
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

//  connect(w, &QQuickWindow::frameSwapped,
//    this, &QQuickItem::update, Qt::QueuedConnection);
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
        Q_ASSERT(node->context_->isValid());

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
      Q_ASSERT(node->fbo_[node->i_].fbo);

      if (size().toSize() == node->rect().size().toSize())
      {
        node->workFinished_.store(false, std::memory_order_relaxed);

        QMetaObject::invokeMethod(node, "work", Qt::QueuedConnection);
      }

      node->setTexture(node->fbo_[node->i_].texture.get());
    }

    node->setRect(br);
  }

  update();

  return node;
}

#include "fboworker.moc"
