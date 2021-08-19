#ifndef FBOWORKER_HPP
# define FBOWORKER_HPP
# pragma once

class FBOWorker : public QQuickItem
{
  QOpenGLContext context_;
  QOffscreenSurface surface_;

  QScopedPointer<QOpenGLFramebufferObject> fbo_;

public:
  explicit FBOWorker(QQuickItem* = nullptr);

private:
  QSGNode* updatePaintNode(QSGNode*, QQuickItem::UpdatePaintNodeData*) final;
};

#endif // FBOWORKER_HPP
