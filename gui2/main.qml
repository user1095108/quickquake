import QtQuick 2.0

import QtQuick.Controls 2.5

import QtQuick.Layouts 1.13

import Quake 1.0

Rectangle {
  id: rootitem

  color: "green"

  ColumnLayout {
    height: parent.height

    Slider{
      id: rotslider

      Layout.alignment: Qt.AlignCenter
      Layout.preferredHeight: parent.height / 3

      orientation: Qt.Vertical

      from: -180
      to: 180

      stepSize: 1

      value: 0
    }

    Slider{
      id: xslider

      Layout.alignment: Qt.AlignCenter
      Layout.preferredHeight: parent.height / 3

      orientation: Qt.Vertical

      from: -10
      to: 10

      stepSize: 1

      value: 0
    }

    Slider{
      id: yslider

      Layout.alignment: Qt.AlignCenter
      Layout.fillHeight: true

      orientation: Qt.Vertical

      from: -10
      to: 10

      stepSize: 1

      value: 0
    }
  }

  // quake item
  FBOWorker {
    width: .75 * parent.width
    height: .75 * parent.height

//  width: 640
//  height: 480

    x: .5 * (parent.width - width) - xslider.value
    y: .5 * (parent.height - height) - yslider.value

    mirrorVertically: true

    rotation: rotslider.value

    GLQuake {}
  }
}
