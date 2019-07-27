import QtQuick 2.0

Rectangle {
  id: rootitem

  color: "black"

  // placeholder for quake item
  Rectangle {
    width: .75 * parent.width
    height: .75 * parent.height

    x: .5 * (parent.width - width)
    y: .5 * (parent.height - height)

    color: "yellow"
  }
}
