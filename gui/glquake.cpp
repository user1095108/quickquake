#include "glquake.hpp"

class GLQuakeRenderThread : public QThread
{
  friend class GLQuake;

  Q_OBJECT

  QMutex mutex_;

  QScopedPointer<QOffscreenSurface> surface_;
  QScopedPointer<QOpenGLContext> context_;

  QScopedPointer<QOpenGLFramebufferObject> renderFbo_;
  QScopedPointer<QOpenGLFramebufferObject> displayFbo_;

  QSize size_;

public:
  using QThread::QThread;

  void setSize(QSize const& size)
  {
    //qDebug() << "setSize" << size;
    QMutexLocker m(&mutex_);

    if (!size.isEmpty() && (size != size_))
    {
      size_ = size;
    }
  }

public slots:
  void render()
  {
    Q_ASSERT(!size_.isEmpty());
    context_->makeCurrent(surface_.get());

    QMutexLocker m(&mutex_);

    //qDebug() << "render" << renderFbo_.get();

    if (!renderFbo_ || (renderFbo_->size() != size_))
    {
      QOpenGLFramebufferObjectFormat format;
      format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

      renderFbo_.reset(new QOpenGLFramebufferObject(size_, format));
      displayFbo_.reset(new QOpenGLFramebufferObject(size_, format));
    }

    renderFbo_->bind();

    context_->functions()->glViewport(0, 0, size_.width(), size_.height());

    // we render a quake frame

    {
      QOpenGLPaintDevice opd(size_);
      QPainter painter(&opd);

      painter.fillRect(0, 0, size_.width(), size_.height(), Qt::yellow);
    }

    context_->functions()->glFlush();

    //renderFbo_->bindDefault();

    emit textureReady(renderFbo_->texture(), size_);

    renderFbo_.swap(displayFbo_);
  }

  void shutdown()
  {
    if (isRunning())
    {
      QMutexLocker m(&mutex_);

      if (context_)
      {
        context_->makeCurrent(surface_.get());

        renderFbo_.reset();
        displayFbo_.reset();

        context_->doneCurrent();

        context_.reset();
        surface_.reset();
      }

      exit();
    }
  }

signals:
  void textureReady(int to, QSize const& size);
};

class TextureNode : public QObject, public QSGSimpleTextureNode
{
  Q_OBJECT

  QQuickItem* item_;

  QScopedPointer<QSGTexture> texture_;

public:
  explicit TextureNode(QQuickItem* const item) :
    item_(item)
  {
    setFiltering(QSGTexture::Nearest);

    texture_.reset(item_->window()->createTextureFromId(0, QSize(1, 1)));
    setTexture(texture_.get());
  }

public slots:
  void updateNode(int const id, QSize const& size)
  {
    texture_.reset(item_->window()->createTextureFromId(id, size));
    setTexture(texture_.get());

//  markDirty(DirtyMaterial);

    item_->update();

    emit nodeUpdated();
  }

signals:
  void nodeUpdated();
};

//////////////////////////////////////////////////////////////////////////////
GLQuake::GLQuake(QQuickItem* const parent) :
  QQuickItem(parent),
  renderThread_(new GLQuakeRenderThread(this))
{
  setFlag(ItemHasContents);

  connect(this, &QQuickItem::parentChanged,
    renderThread_, &GLQuakeRenderThread::shutdown, Qt::DirectConnection);
}

//////////////////////////////////////////////////////////////////////////////
GLQuake::~GLQuake()
{
  renderThread_->shutdown();
  renderThread_->wait();
}

//////////////////////////////////////////////////////////////////////////////
void GLQuake::geometryChanged(QRectF const& newGeometry,
  QRectF const& oldGeometry)
{
  QQuickItem::geometryChanged(newGeometry, oldGeometry);

  renderThread_->setSize(newGeometry.size().toSize());
}

//////////////////////////////////////////////////////////////////////////////
QSGNode* GLQuake::updatePaintNode(QSGNode* const n,
  QQuickItem::UpdatePaintNodeData*)
{
  if (!renderThread_->context_)
  {
    auto const w(window());

    auto const ccontext(w->openglContext());
    Q_ASSERT(ccontext);

    ccontext->doneCurrent();

    auto f(ccontext->format());
    f.setProfile(QSurfaceFormat::CompatibilityProfile);

    renderThread_->context_.reset(new QOpenGLContext);
    renderThread_->surface_.reset(new QOffscreenSurface);

    renderThread_->context_->setFormat(f);
    renderThread_->context_->setShareContext(ccontext);
    renderThread_->context_->create();
    Q_ASSERT(renderThread_->context_->isValid());

    renderThread_->surface_->setFormat(f);
    renderThread_->surface_->create();
    Q_ASSERT(renderThread_->surface_->isValid());

    connect(w, &QQuickWindow::sceneGraphInvalidated,
      renderThread_, &GLQuakeRenderThread::shutdown, Qt::DirectConnection);

    renderThread_->start();

    ccontext->makeCurrent(w);
  }

  auto node(static_cast<TextureNode*>(n));

  if (!node)
  {
    node = new TextureNode(this);

    connect(renderThread_, &GLQuakeRenderThread::textureReady,
      node, &TextureNode::updateNode, Qt::QueuedConnection);
    connect(node, &TextureNode::nodeUpdated,
      renderThread_, &GLQuakeRenderThread::render, Qt::QueuedConnection);

    QMetaObject::invokeMethod(renderThread_, "render", Qt::QueuedConnection);
  }

  node->setRect(boundingRect());

  return node;
}

#include "glquake.moc"
