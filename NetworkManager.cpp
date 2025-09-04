#include "NetworkManager.h"
#include <QHostAddress>
#include <QFile>
#include <QTcpSocket>
#include <QFileInfo>
#include <QNetworkProxy>


NetworkManager::NetworkManager(Circuit* circuit, QObject* parent)
    : QObject(parent), circuit(circuit), role(NetworkRole::None), connected(false) {
    server = nullptr;
    clientSocket = nullptr;
}

NetworkManager::~NetworkManager() {
    if (server) {
        server->close();
        delete server;
        server = nullptr;
    }
    if (clientSocket) {
        clientSocket->disconnectFromHost();
        if (clientSocket->state() == QAbstractSocket::ConnectedState) {
            clientSocket->waitForDisconnected(1000);
        }
        delete clientSocket;
        clientSocket = nullptr;
    }
}

// NetworkManager::~NetworkManager() {
//     disconnect();
// }

bool NetworkManager::startServer(quint16 port) {
    qDebug() << "Starting server on port:" << port;
    if (server) {
        delete server;
        server = nullptr;
    }

    server = new QTcpServer(this);

    server->setProxy(QNetworkProxy::NoProxy); // added

    if (!server->listen(QHostAddress::Any, port)) {
        emit connectionStatusChanged(false, "Server failed to start: " + server->errorString());
        return false;
    }

    connect(server, &QTcpServer::newConnection, this, &NetworkManager::newConnection);
    role = NetworkRole::Server;
    emit connectionStatusChanged(true, "Server started on port " + QString::number(port));
    return true;
}

bool NetworkManager::connectToServer(const QString& host, quint16 port) {
    qDebug() << "Connecting to server:" << host << ":" << port;
    if (clientSocket) {
        clientSocket->disconnectFromHost();
        delete clientSocket;
        clientSocket = nullptr;
    }

    clientSocket = new QTcpSocket(this);

    clientSocket->setProxy(QNetworkProxy::NoProxy);//added

    connect(clientSocket, &QTcpSocket::connected, this, [this]() {
        connected = true;
        emit connectionStatusChanged(true, "Connected to server");
    });
    connect(clientSocket, &QTcpSocket::readyRead, this, &NetworkManager::readyRead);
    connect(clientSocket, &QTcpSocket::errorOccurred, this, &NetworkManager::socketError);
    connect(clientSocket, &QTcpSocket::disconnected, this, &NetworkManager::socketDisconnected);

    clientSocket->connectToHost(host, port);
    if (!clientSocket->waitForConnected(10000)) { //increasedd to 10 seconds!
        emit connectionStatusChanged(false, "Connection timeout");
        return false;
    }

    role = NetworkRole::Client;
    return true;
}

void NetworkManager::disconnect() {
    if (server) {
        server->close();
        delete server;
        server = nullptr;
    }
    if (clientSocket) {
        clientSocket->disconnectFromHost();
        delete clientSocket;
        clientSocket = nullptr;
    }
    connected = false;
    role = NetworkRole::None;
    emit connectionStatusChanged(false, "Disconnected");
}

void NetworkManager::sendVoltageSource(const QString& name, const QString& node1, const QString& node2,
                                      double value, bool isSinusoidal,
                                      double offset, double amplitude, double frequency) {
    if (!connected) return;

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_5);

    stream << name << node1 << node2 << value << isSinusoidal;
    if (isSinusoidal) {
        stream << offset << amplitude << frequency;
    }

    sendMessage(MessageType::VoltageSource, data);
}

void NetworkManager::sendCircuitFile() {
    if (!connected) return;

    // Save circuit to temporary file
    QString tempFile = QCoreApplication::applicationDirPath() + "/temp_circuit.psp";
    circuit->saveToFile(tempFile);

    QFile file(tempFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray circuitData = file.readAll();
    file.close();
    QFile::remove(tempFile);

    sendMessage(MessageType::CircuitFile, circuitData);
}

void NetworkManager::sendSignalData(const std::map<double, double>& signalData, const QString& signalName) {
    if (!connected) return;

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_5);

    stream << signalName;
    stream << static_cast<quint32>(signalData.size());
    for (const auto& point : signalData) {
        stream << point.first << point.second;
    }

    sendMessage(MessageType::SignalData, data);
}

void NetworkManager::newConnection() {
    qDebug() << "New connection established";
    if (clientSocket) {
        clientSocket->disconnectFromHost();
        delete clientSocket;
    }

    clientSocket = server->nextPendingConnection();
    connect(clientSocket, &QTcpSocket::readyRead, this, &NetworkManager::readyRead);
    connect(clientSocket, &QTcpSocket::errorOccurred, this, &NetworkManager::socketError);
    connect(clientSocket, &QTcpSocket::disconnected, this, &NetworkManager::socketDisconnected);

    connected = true;
    emit connectionStatusChanged(true, "Client connected");

    // Send connection accepted message
    sendMessage(MessageType::ConnectionAccepted);
}

// void NetworkManager::readyRead() {
//     QByteArray message = clientSocket->readAll();
//     processMessage(message);
// }
void NetworkManager::readyRead() {
    while (clientSocket->bytesAvailable() > 0) {
        QByteArray message = clientSocket->readAll();
        if (!message.isEmpty()) {
            processIncomingData(message);
        }
    }
}

void NetworkManager::socketError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    emit connectionStatusChanged(false, "Socket error: " + clientSocket->errorString());
    disconnect();
}

void NetworkManager::socketDisconnected() {
    emit connectionStatusChanged(false, "Disconnected from peer");
    disconnect();
}

void NetworkManager::processMessage(const QByteArray& message) {
    QDataStream stream(message);
    stream.setVersion(QDataStream::Qt_6_5);

    quint32 type;
    stream >> type;
    MessageType msgType = static_cast<MessageType>(type);

    switch (msgType) {
    case MessageType::VoltageSource: {
        QString name, node1, node2;
        double value;
        bool isSinusoidal;
        double offset = 0.0, amplitude = 0.0, frequency = 0.0;

        stream >> name >> node1 >> node2 >> value >> isSinusoidal;
        if (isSinusoidal) {
            stream >> offset >> amplitude >> frequency;
        }

        emit voltageSourceReceived(name, node1, node2, value, isSinusoidal, offset, amplitude, frequency);
        break;
    }
    case MessageType::CircuitFile: {
        QByteArray circuitData = message.mid(sizeof(quint32));
        QString tempFile = QCoreApplication::applicationDirPath() + "/received_circuit.psp";

        QFile file(tempFile);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(circuitData);
            file.close();
            circuit->loadFromFile(tempFile);
            QFile::remove(tempFile);
            emit circuitFileReceived();
        }
        break;
    }
    case MessageType::SignalData: {
        QString signalName;
        quint32 pointCount;
        std::map<double, double> signalData;

        stream >> signalName >> pointCount;
        for (quint32 i = 0; i < pointCount; ++i) {
            double x, y;
            stream >> x >> y;
            signalData[x] = y;
        }

        emit signalDataReceived(signalData, signalName);
        break;
    }
    case MessageType::ConnectionAccepted:
        emit connectionStatusChanged(true, "Connection accepted by server");
        break;
    case MessageType::ConnectionRejected:
        emit connectionStatusChanged(false, "Connection rejected by server");
        disconnect();
        break;
    default:
        break;
    }
}

void NetworkManager::sendMessage(MessageType type, const QByteArray& data) {
    if (!connected) return;

    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_5);

    stream << static_cast<quint32>(type);
    if (!data.isEmpty()) {
        stream.writeRawData(data.constData(), data.size());
    }

    clientSocket->write(message);
}

///added
// void NetworkManager::sendData(const QByteArray& data) {
//     if (!connected || !clientSocket) return;
//     clientSocket->write(data);
// }
// In NetworkManager.cpp, improve the sendData method:
void NetworkManager::sendData(const QByteArray& data) {
    if (!connected || !clientSocket) {
        qDebug() << "Cannot send data: Not connected";
        return;
    }

    if (data.isEmpty()) {
        qDebug() << "Cannot send empty data";
        return;
    }

    qint64 bytesWritten = clientSocket->write(data);
    if (bytesWritten == -1) {
        qDebug() << "Failed to write data:" << clientSocket->errorString();
    } else if (bytesWritten < data.size()) {
        qDebug() << "Partial data written:" << bytesWritten << "of" << data.size() << "bytes";
    } else {
        qDebug() << "Data sent successfully:" << bytesWritten << "bytes";
    }

    // Ensure data is actually sent
    if (!clientSocket->waitForBytesWritten(5000)) {
        qDebug() << "Data transmission timeout:" << clientSocket->errorString();
    }
}
///added
// In NetworkManager.cpp, fix the processIncomingData method:
void NetworkManager::processIncomingData(const QByteArray& data) {
    if (data.isEmpty()) {
        qDebug() << "Received empty data";
        return;
    }

    QString dataStr = QString::fromUtf8(data);
    QString type;
    QByteArray content;

    qDebug() << "Raw received data:" << dataStr.left(100) << "..."; // Show first 100 chars

    // Parse the message type based on prefixes
    if (dataStr.startsWith("CIRCUIT:")) {
        type = "circuit";
        content = data.mid(8); // Remove "CIRCUIT:" prefix
        qDebug() << "Identified as circuit data";
    }
    else if (dataStr.startsWith("SIGNAL:")) {
        type = "signal";
        content = data.mid(7); // Remove "SIGNAL:" prefix
        qDebug() << "Identified as signal data";
    }
    else if (dataStr.startsWith("VOLTAGE:")) {
        type = "voltage";
        content = data.mid(8); // Remove "VOLTAGE:" prefix
        qDebug() << "Identified as voltage data:" << QString::fromUtf8(content);
    }
    else {
        type = "unknown";
        content = data;
        qDebug() << "Unknown data type received";
    }

    emit dataReceived(content, type);
}
// void NetworkManager::processIncomingData(const QByteArray& data) {
//     if (data.isEmpty()) return;
//
//     QString dataStr = QString::fromUtf8(data);
//     QString type;
//     QByteArray content;
//
//     // Parse the message type based on prefixes
//     if (dataStr.startsWith("CIRCUIT:")) {
//         type = "circuit";
//         content = data.mid(8); // Remove "CIRCUIT:" prefix
//     }
//     else if (dataStr.startsWith("VOLTAGE_NODE")) {
//         type = "voltage";
//         content = data;
//     }
//     else if (dataStr.startsWith("SIGNAL_INPUT:")) {
//         type = "signal";
//         content = data.mid(13); // Remove "SIGNAL_INPUT:" prefix
//     }
//     else if (dataStr.startsWith("COMPONENT:")) {
//         type = "component";
//         content = data.mid(10); // Remove "COMPONENT:" prefix
//     }
//     else {
//         type = "unknown";
//         content = data;
//     }
//
//     emit dataReceived(content, type);
// }