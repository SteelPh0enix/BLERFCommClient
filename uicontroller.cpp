#include "uicontroller.hpp"

UIController::UIController(QObject *parent) : QObject(parent) {
  m_scanner = new BLEScanner{this};
  m_comm = new BLERFComm{this};

  QObject::connect(m_scanner, &BLEScanner::scanCompleted, this,
                   &UIController::bleScanCompletedHandler);
  QObject::connect(m_scanner, &BLEScanner::scanCompleted, this,
                   &UIController::bleScanCompleted);
  QObject::connect(m_scanner, &BLEScanner::scanError, this,
                   &UIController::bleScanErrorHandler);

  QObject::connect(m_comm, &BLERFComm::deviceReady, this,
                   &UIController::bleDeviceReady);
  QObject::connect(m_comm, &BLERFComm::connectedToDevice, this,
                   &UIController::bleDeviceConnected);
  QObject::connect(m_comm, &BLERFComm::disconnectedFromDevice, this,
                   &UIController::bleDeviceDisconnected);
  QObject::connect(
      m_comm, &BLERFComm::connectionError,
      [&](BLEComm::Error error, QString const &description) {
        QString errorMessage =
            QString("Device error #%1: %2").arg(error).arg(description);
        emit bleDeviceError(errorMessage);
      });
  QObject::connect(m_comm, &BLERFComm::dataReceived,
                   [&](QByteArray const &data) {
                     emit messageReceived(QString::fromUtf8(data));
                   });
}

int UIController::serviceUuid() const { return m_serviceUuid; }

int UIController::charUuid() const { return m_charUuid; }

QStringList UIController::bleDeviceDescriptionList() const {
  return m_bleDeviceDescriptionList;
}

bool UIController::isConnectedToDevice() const {
  return m_comm->isDeviceReady();
}

void UIController::setServiceUuid(int serviceUuid) {
  if (m_serviceUuid == serviceUuid) {
    return;
  }

  if (serviceUuid <= 0x0000 || serviceUuid >= 0xFFFF) {
    m_serviceUuid = -1;
    return;
  }

  m_serviceUuid = serviceUuid;
  m_comm->setServiceUuid(QBluetoothUuid(static_cast<quint16>(serviceUuid)));
  emit serviceUuidChanged(m_serviceUuid);
}

void UIController::setCharUuid(int charUuid) {
  if (m_charUuid == charUuid) return;

  if (charUuid <= 0x0000 || charUuid >= 0xFFFF) {
    m_charUuid = -1;
    return;
  }

  m_charUuid = charUuid;
  m_comm->setCharUuid(QBluetoothUuid(static_cast<quint16>(charUuid)));
  emit charUuidChanged(m_charUuid);
}

void UIController::setServiceUuid(const QString &serviceUuid) {
  setServiceUuid(serviceUuid.toUInt(nullptr, 16));
}

void UIController::setCharUuid(const QString &charUuid) {
  setCharUuid(charUuid.toUInt(nullptr, 16));
}

void UIController::connectToDevice(int deviceIndex) {
  if (serviceUuid() == -1 || charUuid() == -1 || deviceIndex < 0 ||
      deviceIndex >= m_bleDeviceDescriptionList.length()) {
    return;
  }

  m_comm->connectToDevice(m_scanner->deviceList().at(deviceIndex));
}

void UIController::disconnectFromDevice() { m_comm->disconnectFromDevice(); }

void UIController::scanForDevices() {
  m_scanner->scan();
}

void UIController::sendMessageToDevice(const QString &message) {
  m_comm->sendData(message.toUtf8());
}

void UIController::bleScanCompletedHandler(int) {
  m_bleDeviceDescriptionList.clear();
  for (auto const &device : m_scanner->deviceList()) {
    m_bleDeviceDescriptionList.append(
        QString("%1 (%2)").arg(device.name(), device.address().toString()));
  }

  emit bleDeviceDescriptionListChanged(m_bleDeviceDescriptionList);
}

void UIController::bleScanErrorHandler(QBluetoothDeviceDiscoveryAgent::Error,
                                       const QString &description) {
  emit bleScanError(description);
}
