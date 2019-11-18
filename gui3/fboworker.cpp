#include <atomic>

#include "fboworker.hpp"

class TextureNode : public QThread, public QSGSimpleTextureNode
{
  friend class FBOWorker;

  Q_OBJECT

  std::atomic<bool> workFinished_{};
  unsigned char i_{};

  struct
  {
    QScopedPointer<QOpenGLFramebufferObject> fbo;
    QScopedPointer<QSGTexture> texture;

    void reset()
    {
      fbo.reset();
      texture.reset();
    }
  } fbo_[2];

  QScopedPointer<QOpenGLContext> context_;
  QOffscreenSurface surface_;

  QQuickItem* item_;

public:
  explicit TextureNode(QQuickItem* const item) :
    item_(item)
  {
    setFiltering(QSGTexture::Nearest);
    setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);

    setTexture(item_->window()->createTextureFromId(0, QSize()));
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

        fbo_[0].reset();
        fbo_[1].reset();

        workFinished_.store(false, std::memory_order_relaxed);
      }
    }
  }

  void suspend() noexcept
  {
    if (isRunning())
    {
      exit();
      wait();

      setTexture(item_->window()->createTextureFromId(0, QSize()));

      fbo_[0].reset();
      fbo_[1].reset();

      workFinished_.store(false, std::memory_order_relaxed);
    }
  }

  Q_INVOKABLE void work()
  {
    context_->makeCurrent(&surface_);

    auto& fbo(fbo_[i_ = (i_ + 1) % 2]);

    auto const size((item_->size() *
      item_->window()->effectiveDevicePixelRatio()).toSize());
    Q_ASSERT(!size.isEmpty());

    if (!fbo.fbo || (fbo.fbo->size() != size))
    {
      QOpenGLFramebufferObjectFormat format;
      format.setAttachment(QOpenGLFramebufferObject::Depth);

      fbo.fbo.reset(new QOpenGLFramebufferObject(size, format));
      fbo.texture.reset(item_->window()->
        createTextureFromId(fbo.fbo->texture(), size));
    }

    Q_ASSERT(fbo.fbo->isValid());
    fbo.fbo->bind();

    {
      QQmlListReference const ref(item_, "resources");

      for (int i{}, c(ref.count()); i != c; ++i)
      {
        QMetaObject::invokeMethod(ref.at(i), "render", Qt::DirectConnection,
          Q_ARG(QSize, size));
      }
    }

    workFinished_.store(true, std::memory_order_relaxed);

    if (item_->isVisible())
    {
      context_->functions()->glFlush();

      QMetaObject::invokeMethod(item_, "update", Qt::AutoConnection);
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
  if (auto const br(boundingRect()); br.isEmpty())
  {
    return delete n, nullptr;
  }
  else
  {
    auto const w(window());
    Q_ASSERT(w);

    auto node(static_cast<TextureNode*>(n));

    if (!node)
    {
      node = new TextureNode(this);
      node->setRect(br);

      connect(this, &QQuickItem::visibleChanged,
        node,
        [this, node]()
        {
          if (isVisible())
          {
            // unsuspend
            Q_ASSERT(!node->isRunning());
            node->start();

            QMetaObject::invokeMethod(node, "work", Qt::QueuedConnection);
          }
          else
          {
            node->suspend();
          }
        },
        Qt::DirectConnection
      );

      connect(w, &QQuickWindow::sceneGraphInitialized,
        node, [&]() { update(); },
        Qt::DirectConnection
      );

      connect(w, &QQuickWindow::sceneGraphInvalidated,
        node, &TextureNode::shutdown,
        Qt::DirectConnection
      );

      Q_ASSERT(!node->context_);

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
        node, &TextureNode::shutdown,
        Qt::DirectConnection
      );

      node->start();

      QMetaObject::invokeMethod(node, "work", Qt::QueuedConnection);
    }
    else if (node->rect() == br)
    {
      if (node->workFinished_.load(std::memory_order_relaxed))
      {
        node->workFinished_.store(false, std::memory_order_relaxed);

        node->setTexture(node->fbo_[node->i_].texture.get());

        QMetaObject::invokeMethod(node, "work", Qt::QueuedConnection);
      }
    }
    else
    {
      node->setRect(br);

      if (isVisible())
      {
        update();
      }
    }

    return node;
  }
}

#include "fboworker.moc"
