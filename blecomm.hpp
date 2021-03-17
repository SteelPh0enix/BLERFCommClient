#pragma once
#include <QBluetoothAddress>
#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QByteArray>
#include <QList>
#include <QLowEnergyCharacteristic>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QObject>
#include <QString>

class BLEComm : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QBluetoothUuid commServiceUuid READ commServiceUuid WRITE
                 setCommServiceUuid NOTIFY commServiceUuidChanged)
  Q_PROPERTY(
      QBluetoothUuid commCharacteristicUuid READ commCharacteristicUuid WRITE
          setCommCharacteristicUuid NOTIFY commCharacteristicUuidChanged)

 public:
  enum Error {
    NoError,
    ConnectonError,
    ServiceError,
    CharacteristicError,
    UnknownError
  };

  using ServiceList = QList<QBluetoothUuid>;

  explicit BLEComm(QObject* parent = nullptr);
  explicit BLEComm(QBluetoothUuid const& serviceUuid,
                   QBluetoothUuid const& charUuid, QObject* parent = nullptr);

  void connectToDevice(QBluetoothDeviceInfo const& device);
  void disconnectFromDevice();

  void setCommServiceUuid(QBluetoothUuid const& uuid);
  void setCommCharacteristicUuid(QBluetoothUuid const& uuid);

  void transmitData(QByteArray const& data);

  Q_INVOKABLE auto connected() const -> bool;
  Q_INVOKABLE auto ready() const -> bool;
  Q_INVOKABLE auto connectedDeviceName() const -> QString;
  Q_INVOKABLE auto connectedDeviceAddress() const -> QBluetoothAddress;
  auto commServiceUuid() const -> QBluetoothUuid;
  auto commCharacteristicUuid() const -> QBluetoothUuid;

 signals:
  void connectedToDevice();
  void disconnectedFromDevice();
  void connectionError(BLEComm::Error errorType, QString const& description);
  void commsReady();
  void dataReceived(QByteArray const& data);

  void commServiceUuidChanged(QBluetoothUuid commServiceUuid);
  void commCharacteristicUuidChanged(QBluetoothUuid commCharacteristicUuid);

 private slots:
  void handleConnection();
  void handleDiscovery();
  void handleData(QLowEnergyCharacteristic const& characteristic,
                  QByteArray const& data);

 private:
  QLowEnergyController* m_controller{nullptr};
  QLowEnergyService* m_service{nullptr};
  QLowEnergyCharacteristic m_char{};

  QBluetoothUuid m_serviceUuid{};
  QBluetoothUuid m_charUuid{};
};
