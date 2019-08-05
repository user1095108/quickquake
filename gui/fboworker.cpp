#include "fboworker.hpp"

class TextureNode : public QThread, public QSGSimpleTextureNode
{
  friend class FBOWorker;

  Q_OBJECT

  QMutex mutex_;

  QScopedPointer<QOffscreenSurface> surface_;
  QScopedPointer<QOpenGLContext> context_;

  QScopedPointer<QOpenGLFramebufferObject> fbo_;

  QScopedPointer<QSGTexture> texture_;

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

  void consumeTexture() noexcept
  {
    QMutexLocker l(&mutex_);

    if (texture_)
    {
      setTexture(texture_.take());
    }
  }

public slots:
  void shutdown() noexcept
  {
    if (isRunning())
    {
      QMutexLocker m(&mutex_);

      if (context_)
      {
        context_->makeCurrent(surface_.get());

        fbo_.reset();

        context_->doneCurrent();

        context_.reset();
        surface_.reset();
      }

      exit();
    }
  }

  void work()
  {
    {
      QMutexLocker m(&mutex_);

      auto const size(rect().size().toSize());
      Q_ASSERT(!size.isEmpty());

      context_->makeCurrent(surface_.get());

      if (!fbo_ || (fbo_->size() != size))
      {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::Depth);

        fbo_.reset(new QOpenGLFramebufferObject(size, format));
      }

      auto& fbo(*fbo_);
      Q_ASSERT(fbo.isValid());

      fbo.bind();

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

      {
        QQmlListReference const ref(item_, "data");

        for (int i{}, c(ref.count()); i != c; ++i)
        {
          QMetaObject::invokeMethod(ref.at(i), "render", Qt::DirectConnection,
            Q_ARG(QSize, size));
        }
      }

      context_->functions()->glFinish();

      texture_.reset(item_->window()->createTextureFromId(fbo.takeTexture(),
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
    delete n;

    return nullptr;
  }

  auto node(static_cast<TextureNode*>(n));

  if (!node)
  {
    node = new TextureNode(this);
    node->setRect(br);

    auto const w(window());
    Q_ASSERT(w);

    connect(w, &QQuickWindow::sceneGraphInvalidated,
      node, &TextureNode::shutdown, Qt::DirectConnection);
    connect(w, &QQuickWindow::frameSwapped,
      this, &FBOWorker::update, Qt::DirectConnection);
  }
  else
  {
    bool eq;

    {
      QMutexLocker l(&node->mutex_);

      if (!(eq = node->rect() == br))
      {
        node->setRect(br);
      }
    }

    if (eq)
    {
      node->consumeTexture();
      QMetaObject::invokeMethod(node, "work", Qt::QueuedConnection);
    }
  }

  if (!node->context_)
  {
    auto const w(window());
    Q_ASSERT(w);

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

    node->surface_.reset(new QOffscreenSurface);
    node->surface_->setFormat(f);
    node->surface_->create();
    Q_ASSERT(node->surface_->isValid());

    ccontext->makeCurrent(w);

    node->start();

    connect(ccontext, &QOpenGLContext::aboutToBeDestroyed,
      node, &TextureNode::shutdown, Qt::DirectConnection);

    QMetaObject::invokeMethod(node, "work", Qt::QueuedConnection);
  }

  return node;
}

#include "fboworker.moc"
