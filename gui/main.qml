import QtQuick 2.0

import Quake 1.0

Rectangle {
  id: rootitem

  color: "green"

  // quake item
  GLQuake {
    width: .75 * parent.width
    height: .75 * parent.height

    x: .5 * (parent.width - width)
    y: .5 * (parent.height - height)

//  rotation: 30
//  color: "yellow"
  }
}
