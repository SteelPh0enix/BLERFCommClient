#include "blerfcomm.hpp"

#include <algorithm>

BLERFComm::BLERFComm(QObject *parent) : QObject(parent) {
  m_comm = new BLEComm{this};

  QObject::connect(m_comm, &BLEComm::connectedToDevice, this,
                   &BLERFComm::connectedToDevice);
  QObject::connect(m_comm, &BLEComm::disconnectedFromDevice, this,
                   &BLERFComm::disconnectedFromDevice);
  QObject::connect(m_comm, &BLEComm::connectionError, this,
                   &BLERFComm::connectionError);
  QObject::connect(m_comm, &BLEComm::commsReady, this, &BLERFComm::deviceReady);
  QObject::connect(m_comm, &BLEComm::dataReceived, this, &BLERFComm::handleRx);
  QObject::connect(m_comm, &BLEComm::commServiceUuidChanged, this,
                   &BLERFComm::serviceUuidChanged);
  QObject::connect(m_comm, &BLEComm::commCharacteristicUuidChanged, this,
                   &BLERFComm::charUuidChanged);

  QObject::connect(m_comm, &BLEComm::commsReady,
                   [&]() { m_deviceReady = true; });
  QObject::connect(m_comm, &BLEComm::connectedToDevice,
                   [&]() { m_deviceConnected = true; });
  QObject::connect(m_comm, &BLEComm::disconnectedFromDevice, [&]() {
    m_deviceReady = false;
    m_deviceConnected = false;
  });
}

QBluetoothUuid BLERFComm::serviceUuid() const {
  return m_comm->commServiceUuid();
}

QBluetoothUuid BLERFComm::charUuid() const {
  return m_comm->commCharacteristicUuid();
}

bool BLERFComm::isDeviceConnected() const { return m_deviceConnected; }

bool BLERFComm::isDeviceReady() const { return m_deviceReady; }

void BLERFComm::connectToDevice(QBluetoothDeviceInfo const& device) {
  m_comm->connectToDevice(device);
}

void BLERFComm::disconnectFromDevice() { m_comm->disconnectFromDevice(); }

void BLERFComm::sendData(QByteArray const& data) {
  QByteArray properData{data.size() + 1, 0x00};
  properData[0] = data.size();
  std::copy(data.begin(), data.end(), properData.begin() + 1);
  m_comm->transmitData(properData);
}

void BLERFComm::setServiceUuid(QBluetoothUuid const& serviceUuid) {
  m_comm->setCommServiceUuid(serviceUuid);
}

void BLERFComm::setCharUuid(QBluetoothUuid const& charUuid) {
  m_comm->setCommCharacteristicUuid(charUuid);
}

void BLERFComm::handleRx(QByteArray const& data) {
  if (data.length() == 0) {
    return;
  }

  if (!rxInProgress()) {
    // casting to prevent getting negative length (not sure if can even happen)
    m_expectedBytes = static_cast<std::uint8_t>(data[0]);
    m_rxBuffer = data.mid(1);
    m_expectedBytes -= m_rxBuffer.size();
  } else {
    m_rxBuffer.append(data);
    m_expectedBytes -= data.size();
  }

  m_rxInProgress = m_expectedBytes > 0;

  if (!rxInProgress()) {
    emit dataReceived(m_rxBuffer);
  }
}

bool BLERFComm::rxInProgress() const { return m_rxInProgress; }
