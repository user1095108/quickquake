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

    auto const ccontext(w->openglContext());
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
        qDebug() << f;

        context_.moveToThread(QThread::currentThread());

        context_.setFormat(f);
        ccontext->doneCurrent();
        context_.setShareContext(ccontext);
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

        auto const id(fbo_->texture());
        node->setTexture(w->createTextureFromNativeObject(
          QQuickWindow::NativeObjectTexture, &id, {}, size,
          QQuickWindow::TextureIsOpaque));
      }

      {
        QQmlListReference const ref(this, "resources");

        for (int i{}, c(ref.count()); i != c; ++i)
        {
          QMetaObject::invokeMethod(ref.at(i), "render", Qt::DirectConnection,
            Q_ARG(QSize, size));
        }
      }

      ccontext->makeCurrent(csurface);
    }
    else
    {
      fbo_.reset();

      GLuint id{};
      node->setTexture(w->createTextureFromNativeObject(
        QQuickWindow::NativeObjectTexture, &id, {}, {}));
    }

    return node;
  }
}
