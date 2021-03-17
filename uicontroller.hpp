#pragma once

#include <QObject>
#include <QStringList>

#include "blerfcomm.hpp"
#include "blescanner.hpp"

class UIController : public QObject
{
  Q_OBJECT

  Q_PROPERTY(int serviceUuid READ serviceUuid WRITE setServiceUuid NOTIFY
                 serviceUuidChanged)
  Q_PROPERTY(
      int charUuid READ charUuid WRITE setCharUuid NOTIFY charUuidChanged)
  Q_PROPERTY(QStringList bleDeviceDescriptionList READ bleDeviceDescriptionList
                 NOTIFY bleDeviceDescriptionListChanged)

  int m_serviceUuid{-1};
  int m_charUuid{-1};

  BLEScanner* m_scanner{nullptr};
  BLERFComm* m_comm{nullptr};
  QStringList m_bleDeviceDescriptionList{};

 public:
  explicit UIController(QObject* parent = nullptr);

  int serviceUuid() const;
  int charUuid() const;

  QStringList bleDeviceDescriptionList() const;

  Q_INVOKABLE bool isConnectedToDevice() const;

 public slots:
  void setServiceUuid(int serviceUuid);
  void setCharUuid(int charUuid);
  void setServiceUuid(QString const& serviceUuid);
  void setCharUuid(QString const& charUuid);

  void connectToDevice(int deviceIndex);
  void disconnectFromDevice();
  void scanForDevices();
  void sendMessageToDevice(QString const& message);

 private slots:
  void bleScanCompletedHandler(int foundDevices);
  void bleScanErrorHandler(QBluetoothDeviceDiscoveryAgent::Error error_code,
                           QString const& description);

 signals:
  void serviceUuidChanged(int serviceUuid);
  void charUuidChanged(int charUuid);
  void bleDeviceDescriptionListChanged(QStringList devList);
  void bleScanCompleted(int foundDevices);
  void bleScanError(QString const& description);

  void bleDeviceConnected();
  void bleDeviceReady();
  void bleDeviceDisconnected();
  void bleDeviceError(QString const& description);
  void messageReceived(QString const& data);
};
