#include "fboworker.hpp"

class TextureNode : public QThread, public QSGSimpleTextureNode
{
  friend class FBOWorker;

  Q_OBJECT

  QMutex mutex_;

  unsigned char i_{};

  QScopedPointer<QOffscreenSurface> surface_;
  QScopedPointer<QOpenGLContext> context_;

  QScopedPointer<QOpenGLFramebufferObject> fbo_[2];

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

public slots:
  void shutdown() noexcept
  {
    if (isRunning())
    {
      QMutexLocker m(&mutex_);

      if (context_)
      {
        context_->makeCurrent(surface_.get());

        QOpenGLFramebufferObject::bindDefault();

        fbo_[0].reset();
        fbo_[1].reset();

        context_->doneCurrent();

        context_.reset();
        surface_.reset();
      }

      exit();
    }
  }

  void work()
  {
    QMutexLocker m(&mutex_);

    i_ = (i_ + 1) % 2;

    auto const size(rect().size().toSize());
    Q_ASSERT(!size.isEmpty());

    context_->makeCurrent(surface_.get());

    if (!fbo_[i_] || (fbo_[i_]->size() != size))
    {
      QOpenGLFramebufferObjectFormat format;
      format.setAttachment(QOpenGLFramebufferObject::Depth);

      fbo_[i_].reset(new QOpenGLFramebufferObject(size, format));
    }

    auto& fbo(*fbo_[i_]);
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
      QQmlListReference const ref(item_, "resources");

      for (int i{}, c(ref.count()); i != c; ++i)
      {
        QMetaObject::invokeMethod(ref.at(i), "render", Qt::DirectConnection,
          Q_ARG(QSize, size));
      }
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

  if (node)
  {
    auto const w(window());
    Q_ASSERT(w);

    QMutexLocker l(&node->mutex_);

    node->setRect(br);

    if (node->fbo_[node->i_])
    {
      auto& fbo(*node->fbo_[node->i_]);

      node->setTexture(w->createTextureFromId(fbo.texture(), fbo.size()));

      QMetaObject::invokeMethod(node, "work", Qt::QueuedConnection);
    }
    else if (!node->context_)
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

      node->surface_.reset(new QOffscreenSurface);
      node->surface_->setFormat(f);
      node->surface_->create();
      Q_ASSERT(node->surface_->isValid());

      ccontext->makeCurrent(w);

      node->start();

      QMetaObject::invokeMethod(node, "work", Qt::QueuedConnection);

      connect(ccontext, &QOpenGLContext::aboutToBeDestroyed,
        node, &TextureNode::shutdown, Qt::DirectConnection);
    }
  }
  else
  {
    node = new TextureNode(this);
    node->setRect(br);

    auto const w(window());
    Q_ASSERT(w);

    connect(w, &QQuickWindow::frameSwapped,
      this, &FBOWorker::update, Qt::DirectConnection);
    connect(w, &QQuickWindow::sceneGraphInvalidated,
      node, &TextureNode::shutdown, Qt::DirectConnection);
  }

  return node;
}

#include "fboworker.moc"
