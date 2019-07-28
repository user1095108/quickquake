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

    if (size.isEmpty())
    {
      shutdown();
    }
    else
    {
      size_ = size;
    }
  }

public slots:
  void render()
  {
    //qDebug() << "render" << renderFbo_.get();
    Q_ASSERT(!size_.isEmpty());
    context_->makeCurrent(surface_.get());

    QMutexLocker m(&mutex_);

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

    // some test code to see if fbo rendering is ok
    {
      QOpenGLPaintDevice opd(size_);
      QPainter painter(&opd);

      painter.fillRect(0, 0, size_.width(), size_.height(), Qt::yellow);

      painter.setPen(Qt::red);
      painter.drawLine(0, 0, size_.width(), size_.height());
    }

    // these calls are probably unnecessary, but can be found in the Qt example
    //context_->functions()->glFlush();
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
  void textureReady(uint to, QSize const& size);
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

    setOwnsTexture(true);

    setTexture(item_->window()->createTextureFromId(0, QSize(1, 1)));
  }

public slots:
  void updateNode(uint const id, QSize const& size)
  {
    //qDebug() << "updateNode";
    setTexture(item_->window()->createTextureFromId(id, size));

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
  auto const br(boundingRect());

  if (br.isEmpty())
  {
    renderThread_->shutdown();

    delete n;

    return nullptr;
  }
  else if (!renderThread_->context_)
  {
    auto const w(window());
    Q_ASSERT(w);

    auto const ccontext(w->openglContext());
    Q_ASSERT(ccontext);

    // this is done to safely share context resources
    ccontext->doneCurrent();

    auto f(ccontext->format());
    // quake needs OpenGL compatibility profile
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

    ccontext->makeCurrent(w);

    renderThread_->start();

    connect(ccontext, &QOpenGLContext::aboutToBeDestroyed,
      renderThread_, &GLQuakeRenderThread::shutdown, Qt::DirectConnection);
    connect(w, &QQuickWindow::sceneGraphInvalidated,
      renderThread_, &GLQuakeRenderThread::shutdown, Qt::DirectConnection);
  }

  auto node(static_cast<TextureNode*>(n));

  if (!node)
  {
    node = new TextureNode(this);

    connect(renderThread_, &GLQuakeRenderThread::textureReady,
      node, &TextureNode::updateNode, Qt::QueuedConnection);

    // establish the endless rendering loop
    connect(node, &TextureNode::nodeUpdated,
      renderThread_, &GLQuakeRenderThread::render, Qt::QueuedConnection);

    QMetaObject::invokeMethod(renderThread_, "render", Qt::QueuedConnection);
  }

  node->setRect(br);

  return node;
}

#include "glquake.moc"
