import QtQuick 2.0

import QtQuick.Controls 2.5

import QtQuick.Layouts 1.13

import Quake 1.0

ApplicationWindow {
  width: 800
  height: 600

  color: "black"

  visible: true

  FBOWorker {
    width: aslider.value * parent.width
    height: aslider.value * parent.height

    x: .5 * (parent.width - width) - xslider.value
    y: .5 * (parent.height - height)

    focus: true

    rotation: rotslider.value

    visible: checkbox.checked

    Keys.onPressed: (e) => {
      quake.keyEvent(e.key, true)
    }

    Keys.onReleased: (e) => {
      quake.keyEvent(e.key, false)
    }

    GLQuake {
      id: quake
    }
  }

  CheckBox {
    id: checkbox

    anchors.right: parent.right

    y: 5

    checked: true
    text: "Visible"
  }

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
      id: aslider

      Layout.alignment: Qt.AlignCenter
      Layout.fillHeight: true

      orientation: Qt.Vertical

      from: .1
      to: 1

      stepSize: .05

      value: 1
    }

    Slider{
      id: xslider

      Layout.alignment: Qt.AlignCenter
      Layout.preferredHeight: parent.height / 3

      orientation: Qt.Vertical

      from: -50
      to: 50

      stepSize: 1

      value: 0
    }
  }
}
