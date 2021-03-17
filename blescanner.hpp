#pragma once

#include <QBluetoothAddress>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QObject>
#include <QVector>

class BLEScanner : public QObject
{
  Q_OBJECT
 public:
  using DeviceList = QVector<QBluetoothDeviceInfo>;
  enum class ScanType { Normal, LowEnergy };

  explicit BLEScanner(QObject* parent = nullptr);

  void scan(ScanType type = ScanType::LowEnergy);
  auto isBusy() -> bool;

  auto getDeviceByName(QString const& name) -> QBluetoothDeviceInfo*;
  auto getDeviceByAddress(QBluetoothAddress const& address)
      -> QBluetoothDeviceInfo*;

  auto devicesFound() const -> DeviceList::size_type;
  auto deviceList() const -> DeviceList const&;

 signals:
  void scanCancelled();
  void scanCompleted(int devicesFound);
  void scanError(QBluetoothDeviceDiscoveryAgent::Error error_code,
                 QString const& description);

 private slots:
  void handleDeviceDiscovered(QBluetoothDeviceInfo const& device);

 private:
  DeviceList m_devices{};
  QBluetoothDeviceDiscoveryAgent* m_agent{nullptr};
};
