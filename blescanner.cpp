#include "blescanner.hpp"

#include <algorithm>

BLEScanner::BLEScanner(QObject *parent) : QObject(parent)
{
  m_agent = new QBluetoothDeviceDiscoveryAgent{this};
  m_agent->setLowEnergyDiscoveryTimeout(5000);

  QObject::connect(m_agent, &QBluetoothDeviceDiscoveryAgent::canceled, this,
                   &BLEScanner::scanCancelled);

  // wtf who with the sane mind put a signal and method with the same name in
  // this fucking class
  QObject::connect(m_agent,
                   static_cast<void (QBluetoothDeviceDiscoveryAgent::*)(
                       QBluetoothDeviceDiscoveryAgent::Error)>(
                       &QBluetoothDeviceDiscoveryAgent::error),
                   [&](QBluetoothDeviceDiscoveryAgent::Error error_code) {
                     emit scanError(error_code, m_agent->errorString());
                   });

  QObject::connect(m_agent, &QBluetoothDeviceDiscoveryAgent::finished,
                   [&]() { emit scanCompleted(devicesFound()); });

  QObject::connect(m_agent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                   this, &BLEScanner::handleDeviceDiscovered);
}

void BLEScanner::scan(BLEScanner::ScanType type) {
  m_devices.clear();
  switch (type) {
    case ScanType::LowEnergy:
      m_agent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
      break;
    case ScanType::Normal:
      m_agent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod);
      break;
  }
}

auto BLEScanner::isBusy() -> bool {
  return (m_agent != nullptr && m_agent->isActive());
}

auto BLEScanner::getDeviceByName(const QString &name)
    -> QBluetoothDeviceInfo * {
  auto found_it =
      std::find_if(m_devices.begin(), m_devices.end(),
                   [&](auto device) { return device.name() == name; });

  if (found_it != m_devices.end()) {
    return found_it;
  }
  return nullptr;
}

auto BLEScanner::getDeviceByAddress(const QBluetoothAddress &address)
    -> QBluetoothDeviceInfo * {
  auto found_it =
      std::find_if(m_devices.begin(), m_devices.end(),
                   [&](auto device) { return device.address() == address; });

  if (found_it != m_devices.end()) {
    return found_it;
  }
  return nullptr;
}

auto BLEScanner::devicesFound() const -> DeviceList::size_type {
  return m_devices.size();
}

auto BLEScanner::deviceList() const -> DeviceList const & { return m_devices; }

void BLEScanner::handleDeviceDiscovered(const QBluetoothDeviceInfo &device) {
  m_devices.append(device);
}
