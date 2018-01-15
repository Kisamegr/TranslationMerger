import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import Qt.labs.settings 1.0

import TranslationMerge 1.0

ColumnLayout {
  id: control
  property string title
  property var model
  property alias folder: fileDialog.folder
  property bool ready: false

  signal filesAccepted(var fileUrls)


  Layout.fillHeight: true
  Layout.margins: 3

  Settings {
    category: title
    property alias settingsFolderPath: fileDialog.folder
  }

  Label {
    text: title
  }

  RowLayout {
    TextField {
      id: pathField
      Layout.fillWidth: true
      readOnly: true
    }

    Button {
      text: "..."
      Layout.preferredWidth: 30
      onClicked: fileDialog.visible = true
    }
  }

  Rectangle {
    anchors.fill: view
    color: "#DDDDDD"
    radius: 5
    border.width: 2
    border.color: "#888888"
  }

  ListView {
    id: view
    Layout.fillHeight: true
    Layout.fillWidth: true
    model: control.model
    headerPositioning: ListView.OverlayHeader
    clip: true


    header: Rectangle {
      implicitWidth: view.width
      implicitHeight: headerRow.height + 10
      color: "#AAAAAA"
      border.width: 2
      border.color: "#888888"
      clip: true

      radius: 4
      RowLayout {
        id: headerRow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 5
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        Label {
          text: "#"
          Layout.alignment: Layout.Center
        }
        Label {
          text: "Filename"

          horizontalAlignment: Text.AlignHCenter
          Layout.fillWidth: true
        }
        Label {
          text: "Language"
          Layout.alignment: Layout.Center
        }

      }

    }

    delegate: ItemDelegate {
      implicitWidth: view.width

      hoverEnabled: true
      padding: 4

      property var status: model.status

      onStatusChanged: {
        if(status !== TSFile.NORMAL) control.ready = false
      }


      contentItem: Item {
        implicitWidth: view.width
        implicitHeight: delRow.height + 10

        RowLayout {
          id: delRow
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.margins: 5
          anchors.verticalCenter: parent.verticalCenter
          Label {
            text: index+1
            Layout.alignment: Layout.Center
          }
          Label {
            text: model.fileName
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
          }
          Label {
            text: model.language
            Layout.alignment: Layout.Center
          }
        }
      }

      background: Item {
        Rectangle {
          anchors.fill: parent
          anchors.topMargin: 2
          anchors.bottomMargin: 0
          anchors.leftMargin: 4
          anchors.rightMargin: 4


          color: {

            var status = model.status

            if(status === TSFile.PAIR_MISSING)
              return "lightcoral"
            if(status === TSFile.DUPLICATE)
              return "lightyellow"

            return "#EEEEEE"

          }
          radius: 6
          border.width: 1
          border.color: "grey"
        }

      }
    }


  }


  FileDialog {
    id: fileDialog
    selectMultiple: true
    title: "Please choose " + control.title
    folder: shortcuts.home
    nameFilters: [ "Translation files (*.ts)"]

    onAccepted: {
      var path = fileDialog.folder.toString()
      path = path.replace(/^(file:\/{3})/,"");

      pathField.text = path

      control.filesAccepted(fileUrls)
    }

  }
}
