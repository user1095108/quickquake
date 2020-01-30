#ifndef FBOWORKER_HPP
# define FBOWORKER_HPP
# pragma once

class FBOWorker : public QQuickItem
{
  Q_OBJECT

  Q_ENUMS(QSurfaceFormat::OpenGLContextProfile)

  Q_PROPERTY(QSurfaceFormat::OpenGLContextProfile contextProfile READ contextProfile WRITE setContextProfile NOTIFY contextProfileChanged)

  QOpenGLContext context_;
  QOffscreenSurface surface_;

  QScopedPointer<QOpenGLFramebufferObject> fbo_;

  QSurfaceFormat::OpenGLContextProfile contextProfile_{QSurfaceFormat::CompatibilityProfile};

public:
  explicit FBOWorker(QQuickItem* = nullptr);

  auto contextProfile() const noexcept;
  void setContextProfile(QSurfaceFormat::OpenGLContextProfile);

private:
  QSGNode* updatePaintNode(QSGNode*, QQuickItem::UpdatePaintNodeData*) final;

signals:
  void contextProfileChanged();
};

//////////////////////////////////////////////////////////////////////////////
inline auto FBOWorker::contextProfile() const noexcept
{
  return contextProfile_;
}

#endif // FBOWORKER_HPP
