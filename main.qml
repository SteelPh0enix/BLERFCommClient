import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

ApplicationWindow {
    id: mainWindow
    width: 800
    height: 600
    visible: true
    title: qsTr("BLE RFComm terminal")

    function logColor(logText, color) {
        textAreaLog.text += "<p style='color: %1; margin-top: 0em; margin-bottom: 0em;'>%2</p>".arg(color).arg(logText);

        logAreaFlickable.contentY = textAreaLog.height - logAreaFlickable.height
    }

    function log(logText) {
        logColor(logText, "#dedede");
    }

    function logInfo(logText) {
        logColor(logText, Material.color(Material.Blue));
    }

    function logWarning(logText) {
        logColor(logText, Material.color(Material.Orange));
    }

    function logError(logText) {
        logColor(logText, Material.color(Material.Red));
    }

    function sendMessage(message) {
        if (message.length > 0) {
            if (!uiController.isConnectedToDevice()) {
                logError("Connect to device first!");
                return;
            }

            if (message.length > 254) {
                logError("Message is too long!");
                return;
            }

            logColor("ᐅ %1".arg(message), "#91d184");
            uiController.sendMessageToDevice(message);
        }
    }

    Connections {
        id: controllerConnectionsHandler
        target: uiController

        function onBleScanCompleted(devices_found) {
            logInfo("BLE scan completed! Found %1 devices.".arg(devices_found));
        }

        function onBleScanError(error_message) {
            logError("BLE scan error: %1".arg(error_message));
        }

        function onMessageReceived(message) {
            log(message);
        }

        function onBleDeviceConnected() {
            logInfo("Connected! Checking if device has specified characteristic...");
            buttonConnect.text = "Disconnect"
        }

        function onBleDeviceDisconnected() {
            logInfo("Disconnected!");
            buttonConnect.text = "Connect"
        }

        function onBleDeviceReady() {
            logInfo("Device ready to communicate!");
        }

        function onBleDeviceError(description) {
            logError(description);
        }
    }

    GridLayout {
        id: rowConnectionSettings
        anchors.fill: parent
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        columns: 5
        rows: 3

        ComboBox {
            id: comboBoxAvailableDevices
            Layout.preferredWidth: 250
            Layout.fillWidth: true
            model: uiController.bleDeviceDescriptionList
        }

        Button {
            id: buttonScanDevices
            text: qsTr("Scan")

            onClicked: {
                logInfo("Scanning for BLE devices...");
                uiController.scanForDevices();
            }
        }

        TextField {
            id: textFieldServiceUUID
            placeholderText: qsTr("Service UUID")
            validator: RegularExpressionValidator {
                regularExpression: /^[0-9a-fA-F]{4}/
            }
        }

        TextField {
            id: textFieldCharUUID
            placeholderText: qsTr("Char UUID")
            validator: RegularExpressionValidator {
                regularExpression: /^[0-9a-fA-F]{4}/
            }
        }

        Button {
            id: buttonConnect
            text: qsTr("Connect")
            Layout.fillWidth: true

            onClicked: {
                if (uiController.isConnectedToDevice()) {
                    // Connected
                    logInfo("Disconnecting...")
                    uiController.disconnectFromDevice();
                } else {
                    // Not connected
                    // Check if any device is selected
                    if (comboBoxAvailableDevices.currentIndex === -1) {
                        logError("Scan for devices and pick one from the list before trying to connect!");
                        return;
                    }

                    // Set the UUIDs in the backend
                    if (textFieldServiceUUID.text.length > 0) {
                        uiController.setServiceUuid(textFieldServiceUUID.text);
                    } else {
                        logError("Enter RFComm service UUID first!");
                        return;
                    }

                    if (textFieldCharUUID.text.length > 0) {
                        uiController.setCharUuid(textFieldCharUUID.text);
                    } else {
                        logError("Enter RFComm characteristic UUID first!");
                        return;
                    }

                    // Double-check if backend has validated the UUIDs
                    if (uiController.serviceUuid === -1) {
                        logError("Invalid service UUID - specify UUID in hexadecimal format in range [1, FFFE]");
                        return;
                    }

                    if (uiController.charUuid === -1) {
                        logError("Invalid characteristic UUID - specify UUID in hexadecimal format in range [1, FFFE]");
                        return;
                    }

                    logInfo("Connecting to device %1 @ service 0x%2, characteristic 0x%3...".arg(
                                comboBoxAvailableDevices.currentText).arg(
                                uiController.serviceUuid.toString(16).toUpperCase()).arg(
                                uiController.charUuid.toString(16).toUpperCase()));
                    uiController.connectToDevice(comboBoxAvailableDevices.currentIndex);
                }
            }
        }

        Flickable {
            id: logAreaFlickable
            boundsMovement: Flickable.FollowBoundsBehavior
            flickableDirection: Flickable.VerticalFlick
            contentHeight: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.columnSpan: 5
            clip: true

            TextArea.flickable: TextArea {
                id: textAreaLog
                wrapMode: TextArea.Wrap
                font.hintingPreference: Font.PreferFullHinting
                font.pointSize: 10
                textFormat: Text.RichText
                placeholderText: qsTr("Device log")
                readOnly: true
            }

            ScrollBar.vertical: ScrollBar {}
        }

        TextField {
            id: textFieldMessage
            placeholderText: qsTr("Enter message here")
            Layout.fillWidth: true
            Layout.columnSpan: 4
            validator: RegularExpressionValidator {
                regularExpression: /[\x00-\xff]+/
            }
            maximumLength: 254

            onAccepted: {
                sendMessage(text);
                text = ""
            }
        }

        Button {
            id: buttonSendMessage
            text: qsTr("Send")
            autoRepeat: true
            Layout.fillWidth: true

            onClicked: {
                sendMessage(textFieldMessage.text);
            }
        }
    }
}
