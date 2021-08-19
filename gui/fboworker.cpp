#include "fboworker.hpp"

//////////////////////////////////////////////////////////////////////////////
FBOWorker::FBOWorker(QQuickItem* const parent) :
  QQuickItem(parent)
{
  setFlag(ItemHasContents);

  context_.moveToThread({});

  connect(this, &QQuickItem::visibleChanged,
    this,
    [this]
    {
      if (isVisible())
      {
        update();
      }
    },
    Qt::DirectConnection
  );
}

//////////////////////////////////////////////////////////////////////////////
QSGNode* FBOWorker::updatePaintNode(QSGNode* n,
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

    auto const ccontext(static_cast<QOpenGLContext*>(w->rendererInterface()->
      getResource(w, QSGRendererInterface::OpenGLContextResource)));
    auto const csurface(ccontext->surface());

    auto node(static_cast<QSGSimpleTextureNode*>(n));

    if (!node)
    {
      node = new QSGSimpleTextureNode;
      node->setFiltering(QSGTexture::Nearest);
      node->setOwnsTexture(true);
      node->setRect(br);
      node->setTextureCoordinatesTransform(
        QSGSimpleTextureNode::MirrorVertically);

      {
        auto f(ccontext->format());
        f.setProfile(QSurfaceFormat::CompatibilityProfile);

        //
        context_.moveToThread(QThread::currentThread());

        ccontext->doneCurrent();
        context_.setShareContext(ccontext);

        context_.setFormat(f);
        context_.create();

        surface_.setFormat(f);
        surface_.create();

        ccontext->makeCurrent(csurface);
      }
    }
    else if (node->rect() != br)
    {
      node->setRect(br);
    }
    else
    {
      node->markDirty(QSGNode::DirtyMaterial);
    }

    if (isVisible())
    {
      update();

      context_.makeCurrent(&surface_);

      auto const size((br.size() *
        w->effectiveDevicePixelRatio()).toSize());

      if (!fbo_ || (fbo_->size() != size))
      {
        fbo_.reset(new QOpenGLFramebufferObject(size, QOpenGLFramebufferObject::Depth));
        fbo_->bind();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        node->setTexture(QNativeInterface::QSGOpenGLTexture::fromNative(
          fbo_->texture(), w, size, QQuickWindow::TextureIsOpaque));
#else
        auto const id(fbo_->texture());
        node->setTexture(w->createTextureFromNativeObject(
          QQuickWindow::NativeObjectTexture, &id, {}, size,
          QQuickWindow::TextureIsOpaque));
#endif // QT_VERSION
      }

      {
        QQmlListReference const ref(this, "resources");

        for (int i{}, c(ref.count()); i != c; ++i)
        {
          QMetaObject::invokeMethod(ref.at(i), "render", Qt::DirectConnection,
            Q_ARG(QSize, size));
        }

        //fbo_->toImage().save(QStringLiteral("save.png"));
      }

      ccontext->makeCurrent(csurface);
    }
    else
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
      node->setTexture(
        QNativeInterface::QSGOpenGLTexture::fromNative({}, w, {}));
#else
      GLuint const id{};
      node->setTexture(w->createTextureFromNativeObject(
        QQuickWindow::NativeObjectTexture, &id, {}, {}));
#endif // QT_VERSION

      fbo_.reset();
    }

    return node;
  }
}
