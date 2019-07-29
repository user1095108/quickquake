import QtQuick 2.0

import QtQuick.Controls 2.5

import Quake 1.0

Rectangle {
  id: rootitem

  color: "green"

  Slider{
    id: rotslider

    x: 5
    y: 5

    height: 200

    orientation: Qt.Vertical

    from: -180
    to: 180

    stepSize: 1

    value: 0
  }

  // quake item
  GLQuake {
    width: .75 * parent.width
    height: .75 * parent.height

//  width: 640
//  height: 480

    x: .5 * (parent.width - width)
    y: .5 * (parent.height - height)

    rotation: rotslider.value
  }
}
