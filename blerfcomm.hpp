#pragma once

#include <QBluetoothUuid>
#include <QByteArray>
#include <QObject>

#include "blecomm.hpp"

class BLERFComm : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QBluetoothUuid serviceUuid READ serviceUuid WRITE setServiceUuid
                 NOTIFY serviceUuidChanged)
  Q_PROPERTY(QBluetoothUuid charUuid READ charUuid WRITE setCharUuid NOTIFY
                 charUuidChanged)

  BLEComm* m_comm{nullptr};

  QByteArray m_rxBuffer{};
  int m_expectedBytes{0};
  bool m_rxInProgress{false};

  bool m_deviceReady{false};
  bool m_deviceConnected{false};

 public:
  explicit BLERFComm(QObject* parent = nullptr);

  QBluetoothUuid serviceUuid() const;
  QBluetoothUuid charUuid() const;
  bool isDeviceConnected() const;
  bool isDeviceReady() const;

 signals:
  void dataReceived(QByteArray const& data);
  void connectedToDevice();
  void deviceReady();
  void disconnectedFromDevice();
  void connectionError(BLEComm::Error error, QString const& description);

  void serviceUuidChanged(QBluetoothUuid const& serviceUuid);
  void charUuidChanged(QBluetoothUuid const& charUuid);

 public slots:
  void sendData(QByteArray const& data);
  void connectToDevice(QBluetoothDeviceInfo const& device);
  void disconnectFromDevice();

  void setServiceUuid(QBluetoothUuid const& serviceUuid);
  void setCharUuid(QBluetoothUuid const& charUuid);

 private slots:
  void handleRx(QByteArray const& data);

 private:
  bool rxInProgress() const;
};
