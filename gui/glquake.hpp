#ifndef GLQUAKE_HPP
# define GLQUAKE_HPP
# pragma once

class GLQuake : public QQuickItem
{
public:
  explicit GLQuake(QQuickItem* = nullptr);

private:
  QSGNode* updatePaintNode(QSGNode*, QQuickItem::UpdatePaintNodeData*) final;
};

#endif // QQUAKE_HPP
