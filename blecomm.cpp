#include "blecomm.hpp"

#include <algorithm>

BLEComm::BLEComm(QObject* parent) : QObject{parent} {}

BLEComm::BLEComm(QBluetoothUuid const& serviceUuid,
                 QBluetoothUuid const& charUuid, QObject* parent)
    : QObject{parent} {
  setCommServiceUuid(serviceUuid);
  setCommCharacteristicUuid(charUuid);
}

void BLEComm::connectToDevice(const QBluetoothDeviceInfo& device) {
  if (connected()) {
    disconnectFromDevice();
  }

  m_controller = QLowEnergyController::createCentral(device, this);

  QObject::connect(m_controller, &QLowEnergyController::disconnected, this,
                   &BLEComm::disconnectedFromDevice, Qt::QueuedConnection);
  QObject::connect(m_controller, &QLowEnergyController::connected, this,
                   &BLEComm::handleConnection, Qt::QueuedConnection);
  QObject::connect(m_controller, &QLowEnergyController::discoveryFinished, this,
                   &BLEComm::handleDiscovery, Qt::QueuedConnection);
  QObject::connect(
      m_controller,
      static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(
          &QLowEnergyController::error),
      [&](QLowEnergyController::Error errorCode) {
        emit connectionError(BLEComm::Error::ConnectonError,
                             QString("Connection error: %1 (code %2)")
                                 .arg(m_controller->errorString(), errorCode));
        disconnectFromDevice();
      });

  m_controller->connectToDevice();
}

void BLEComm::disconnectFromDevice() {
  if (m_service != nullptr) {
    m_service->deleteLater();
    m_service = nullptr;
  }

  if (m_controller != nullptr) {
    m_controller->disconnectFromDevice();
    m_controller->deleteLater();
    m_controller = nullptr;
  }
}

void BLEComm::setCommServiceUuid(QBluetoothUuid const& uuid) {
  if (m_serviceUuid != uuid) {
    m_serviceUuid = uuid;
    emit commServiceUuidChanged(m_serviceUuid);
  }
}

void BLEComm::setCommCharacteristicUuid(QBluetoothUuid const& uuid) {
  if (m_charUuid != uuid) {
    m_charUuid = uuid;
    emit commCharacteristicUuidChanged(m_charUuid);
  }
}

void BLEComm::transmitData(const QByteArray& data) {
  if (connected() && m_service != nullptr) {
    m_service->writeCharacteristic(m_char, data);
  }
}

auto BLEComm::connected() const -> bool {
  return (m_controller != nullptr &&
          (m_controller->state() == QLowEnergyController::ConnectedState ||
           m_controller->state() == QLowEnergyController::DiscoveredState));
}

auto BLEComm::ready() const -> bool {
  if (m_service != nullptr) {
    return m_service->state() == QLowEnergyService::ServiceDiscovered;
  }
  return false;
}

auto BLEComm::connectedDeviceName() const -> QString {
  if (connected()) {
    return m_controller->remoteName();
  }
  return QString{};
}

auto BLEComm::connectedDeviceAddress() const -> QBluetoothAddress {
  if (connected()) {
    return m_controller->remoteAddress();
  }
  return QBluetoothAddress{};
}

auto BLEComm::commServiceUuid() const -> QBluetoothUuid {
  return m_serviceUuid;
}

auto BLEComm::commCharacteristicUuid() const -> QBluetoothUuid {
  return m_charUuid;
}

void BLEComm::handleConnection() {
  emit connectedToDevice();
  m_controller->discoverServices();
}

void BLEComm::handleDiscovery() {
  auto found_services = m_controller->services();
  auto service_ptr = std::find_if(found_services.begin(), found_services.end(),
                                  [this](auto const& serviceUuid) {
                                    return serviceUuid == commServiceUuid();
                                  });
  if (service_ptr == found_services.end()) {
    emit connectionError(
        BLEComm::Error::ServiceError,
        QString("Cannot find service %1 on device %2!")
            .arg(commServiceUuid().toString(), m_controller->remoteName()));
    disconnectFromDevice();
  } else {
    m_service = m_controller->createServiceObject(*service_ptr, this);

    QObject::connect(
        m_service,
        static_cast<void (QLowEnergyService::*)(
            QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
        [&](QLowEnergyService::ServiceError errorCode) {
          emit connectionError(
              BLEComm::Error::ServiceError,
              QString("An unknown service error happened (code %1)")
                  .arg(errorCode));
        });

    QObject::connect(m_service, &QLowEnergyService::characteristicChanged, this,
                     &BLEComm::handleData, Qt::QueuedConnection);

    QObject::connect(
        m_service, &QLowEnergyService::stateChanged,
        [&](QLowEnergyService::ServiceState newState) {
          if (newState == QLowEnergyService::ServiceDiscovered) {
            m_char = m_service->characteristic(m_charUuid);

            if (!m_char.isValid()) {
              emit connectionError(
                  BLEComm::Error::CharacteristicError,
                  QString("Invalid characteristic %1 on device %2!")
                      .arg(m_charUuid.toString(), m_controller->remoteName()));
              disconnectFromDevice();
              return;
            }

            emit commsReady();
          }
        });

    m_service->discoverDetails();
  }
}

void BLEComm::handleData(const QLowEnergyCharacteristic& characteristic,
                         const QByteArray& data) {
  if (characteristic == m_char) {
    emit dataReceived(data);
  }
}
