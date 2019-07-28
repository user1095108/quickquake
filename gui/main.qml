import QtQuick 2.0

import Quake 1.0

Rectangle {
  id: rootitem

  color: "black"

  // quake item
  GLQuake {
    width: .75 * parent.width
    height: .75 * parent.height

    x: .5 * (parent.width - width)
    y: .5 * (parent.height - height)

//  color: "yellow"
  }
}
