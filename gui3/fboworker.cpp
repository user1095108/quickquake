#include <atomic>

#include "fboworker.hpp"

//////////////////////////////////////////////////////////////////////////////
FBOWorker::FBOWorker(QQuickItem* const parent) :
  QQuickItem(parent)
{
  setFlag(ItemHasContents);

  connect(this, &QQuickItem::visibleChanged,
    this,
    [&]()
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

    auto node(static_cast<QSGSimpleTextureNode*>(n));

    if (!node)
    {
      node = new QSGSimpleTextureNode;
      node->setFiltering(QSGTexture::Nearest);
      node->setTextureCoordinatesTransform(
        QSGSimpleTextureNode::MirrorVertically);

      node->setRect(br);
      node->setTexture(w->createTextureFromId(0, QSize()));

      {
        auto const ccontext(w->openglContext());
        Q_ASSERT(ccontext);
        auto const csurface(ccontext->surface());

        // this is done to safely share context resources
        ccontext->doneCurrent();

        auto f(ccontext->format());
        f.setProfile(contextProfile_);

        {
          context_.reset(new QOpenGLContext);

          auto& context(*context_);
          context.setFormat(f);
          context.setShareContext(ccontext);
          context.create();
          Q_ASSERT(context.isValid());

          surface_.setFormat(f);
          surface_.create();
          Q_ASSERT(surface_.isValid());
        }

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

      auto const ccontext(w->openglContext());
      auto const csurface(ccontext->surface());

      context_->makeCurrent(&surface_);

      auto const size((this->size() *
        w->effectiveDevicePixelRatio()).toSize());
      Q_ASSERT(!size.isEmpty());

      if (!fbo_ || (fbo_->size() != size))
      {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::Depth);

        fbo_.reset(new QOpenGLFramebufferObject(size, format));
        Q_ASSERT(fbo_->isValid());
        fbo_->bind();

        texture_.reset(w->createTextureFromId(fbo_->texture(), size));
        node->setTexture(texture_.get());
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
      texture_.reset();

      node->setTexture(w->createTextureFromId(0, QSize()));
    }

    return node;
  }
}
