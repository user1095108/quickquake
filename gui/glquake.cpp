#include "glquake.hpp"

//////////////////////////////////////////////////////////////////////////////
GLQuake::GLQuake(QQuickItem* const parent) : QQuickItem(parent)
{
  setFlag(ItemHasContents);
}

//////////////////////////////////////////////////////////////////////////////
QSGNode* GLQuake::updatePaintNode(QSGNode*, QQuickItem::UpdatePaintNodeData*)
{
  if (!context_ && !surface_)
  {
    auto const context(window()->openglContext());
    Q_ASSERT(context);

    context_.reset(new QOpenGLContext);
    surface_.reset(new QOffscreenSurface);

    auto f(context->format());
    f.setProfile(QSurfaceFormat::CompatibilityProfile);

    context_->setFormat(f);
    context_->setShareContext(context);
    context_->create();
    Q_ASSERT(context_->isValid());

    surface_->setFormat(f);
    surface_->create();
    Q_ASSERT(surface_->isValid());
  }

  return nullptr;
}
