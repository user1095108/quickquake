#ifndef GLQUAKE_HPP
# define GLQUAKE_HPP
# pragma once

class GLQuake : public QQuickItem
{
  Q_OBJECT

  class GLQuakeRenderThread* renderThread_;

public:
  explicit GLQuake(QQuickItem* = nullptr);
  ~GLQuake();

private:
  void geometryChanged(QRectF const&, QRectF const&) final;
  QSGNode* updatePaintNode(QSGNode*, QQuickItem::UpdatePaintNodeData*) final;
};

#endif // QQUAKE_HPP
