import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2


import TranslationMerge 1.0

ApplicationWindow {
  visible: true
  width: 1280
  height: 800
  title: qsTr("Traslation Merger")
  color: "#b3b3b3"

  ColumnLayout {
    anchors.fill: parent
    anchors.margins: 10


    RowLayout {
      Layout.fillHeight: true
      Layout.fillWidth: true

      FileColumn {
        id: newFiles
        title: "New/Generated TS Files:"
        model: newTsModel

        onFilesAccepted: {
          newFiles.ready = true
          transFiles.ready = true
          manager.addFiles(fileUrls, false)
        }
      }


      Rectangle {Layout.fillHeight: true; Layout.preferredWidth: 5; color: "black"; opacity: 0.25}

      FileColumn {
        id: transFiles
        title: "Old/Translated TS Files:"
        model: transTsModel

        onFilesAccepted: {
          newFiles.ready = true
          transFiles.ready = true

          manager.addFiles(fileUrls, true)
          outDialog.folder = folder

        }
      }


    }


    RowLayout {
      Layout.fillWidth: true

      Label {
        text: "Output Folder:"
      }

      TextField {
        id: outField
        Layout.fillWidth: true
        readOnly: true
      }

      Button {
        text: "..."
        Layout.preferredWidth: 30
        onClicked: fileDialog.visible = true
      }

      FileDialog {
        id: outDialog
        selectFolder: true
        title: "Please choose output folder"
        folder: shortcuts.home

        onFolderChanged: {
          var path = folder.toString()
          path = path.replace(/^(file:\/{3})/,"");
          outField.text = path + "/output"
        }

      }

      Button {
        text: "Merge"
        enabled: newFiles.ready && transFiles.ready

        onClicked: manager.mergeFiles(outField.text)
      }
    }

  }


}
