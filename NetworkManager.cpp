#include "NetworkManager.h"
#include <QHostAddress>
#include <QFile>
#include <QFileDialog>
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
//     while (clientSocket->bytesAvailable() > 0) {
//         QByteArray message = clientSocket->readAll();
//         if (!message.isEmpty()) {
//             processIncomingData(message);
//         }
//     }
// }
void NetworkManager::readyRead() {
    buffer.append(clientSocket->readAll());

    while (buffer.size() >= static_cast<int>(sizeof(quint32))) {
        QDataStream in(&buffer, QIODevice::ReadOnly);
        in.setVersion(QDataStream::Qt_6_5);

        // Read the message size
        quint32 messageSize;
        in >> messageSize;

        // Check if we have the full message
        if (buffer.size() < static_cast<int>(messageSize + sizeof(quint32))) {
            // Incomplete message, wait for more data
            return;
        }

        // Extract the complete message
        QByteArray message = buffer.mid(sizeof(quint32), messageSize);
        buffer = buffer.mid(sizeof(quint32) + messageSize);  // Remove processed data

        // Process the message
        QDataStream messageStream(message);
        messageStream.setVersion(QDataStream::Qt_6_5);

        int typeInt;
        messageStream >> typeInt;
        MessageType type = static_cast<MessageType>(typeInt);

        QByteArray payload;
        messageStream >> payload;

        if (messageStream.status() == QDataStream::Ok) {
            processMessage(payload);
        } else {
            qWarning() << "Invalid message format received";
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


// void NetworkManager::processMessage(MessageType type, const QByteArray& payload) {
//     switch (type) {
//         case MessageType::File: {
//             QDataStream payloadStream(payload);
//             payloadStream.setVersion(QDataStream::Qt_6_5);
//
//             QString fileName;
//             QByteArray fileData;
//             payloadStream >> fileName >> fileData;
//
//             if (payloadStream.status() != QDataStream::Ok) {
//                 qWarning() << "Invalid file payload received";
//                 return;
//             }
//
//             emit fileReceived(fileName, fileData);
//             qDebug() << "File received:" << fileName << "Size:" << fileData.size() << "bytes";
//             break;
//         }
//         // Handle other MessageTypes as needed
//         default:
//             qWarning() << "Unknown message type received:" << static_cast<int>(type);
//         break;
//     }
// }
void NetworkManager::processMessage(const QByteArray& message) {
    QDataStream stream(message);
    stream.setVersion(QDataStream::Qt_6_5);

    QString messageType;
    stream >> messageType;

    if (messageType == "FILE") {
        QString fileName;
        QByteArray fileData;
        stream >> fileName >> fileData;

        if (stream.status() != QDataStream::Ok) {
            qWarning() << "Invalid file payload received";
            return;
        }

        emit fileReceived(fileName, fileData);
        qDebug() << "File received:" << fileName << "Size:" << fileData.size() << "bytes";
    } else {
        qWarning() << "Unknown message type received:" << messageType;
    }
}
// void NetworkManager::processMessage(const QByteArray& message) {
//     QDataStream stream(message);
//     stream.setVersion(QDataStream::Qt_6_5);
//
//     QString messageType;
//     stream >> messageType;
//
//     if (messageType == "FILE") {
//         QString fileName;
//         QByteArray fileData;
//         stream >> fileName >> fileData;
//
//         // Emit signal instead of showing dialog here
//         emit fileReceived(fileName, fileData);
//         qDebug() << "File received:" << fileName << "Size:" << fileData.size() << "bytes";
//
//
//         // Ask user where to save the file
//         QString savePath = QFileDialog::getSaveFileName(
//             nullptr,
//             "Save Received File",
//             QCoreApplication::applicationDirPath() + "/" + fileName,
//             "All Files (*)"
//         );
//
//         if (!savePath.isEmpty()) {
//             QFile file(savePath);
//             if (file.open(QIODevice::WriteOnly)) {
//                 file.write(fileData);
//                 file.close();
//                 qDebug() << "File received and saved:" << savePath;
//
//                 // Emit signal for main window
//                 emit dataReceived(fileData, "file");
//             }
//         }
//     }
//     // ... rest of your existing message processing code
// }

void NetworkManager::sendMessage(MessageType type, const QByteArray& data) {
    if (!connected) return;

    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_5);

    stream << static_cast<quint32>(type);

    QByteArray packet;
    QDataStream packetStream(&packet, QIODevice::WriteOnly);
    packetStream.setVersion(QDataStream::Qt_6_5);
    packetStream << static_cast<quint32>(message.size()) << message;

    if (clientSocket->write(packet) == -1) {
        qWarning() << "Failed to send message:" << clientSocket->errorString();
    } else {
        qDebug() << "Sent message of type:" << static_cast<int>(type) << "Size:" << message.size();
    }

    // if (!data.isEmpty()) {
    //     stream.writeRawData(data.constData(), data.size());
    // }
    //
    // clientSocket->write(message);
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



void NetworkManager::sendFile(const QString& filePath) {
    if (!isConnected()) {
        qWarning() << "Cannot send file: Not connected to any client/server";
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file for reading:" << filePath;
        return;
    }

    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    QByteArray fileData = file.readAll();
    file.close();

    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_5);
    stream << QString("FILE") << fileName << fileData;

    // Add size prefix
    QByteArray packet;
    QDataStream packetStream(&packet, QIODevice::WriteOnly);
    packetStream.setVersion(QDataStream::Qt_6_5);
    packetStream << static_cast<quint32>(message.size()) << message;

    if (clientSocket->write(packet) == -1) {
        qWarning() << "Failed to send file:" << clientSocket->errorString();
    } else {
        qDebug() << "File sent successfully:" << fileName << "Size:" << fileData.size() << "bytes";
    }
}

// Add this method implementation
// void NetworkManager::sendFile(const QString& filePath) {
//     if (!connected || !clientSocket) {
//         qWarning() << "Cannot send file: Not connected to any client/server";
//         return;
//     }
//
//     QFile file(filePath);
//     if (!file.open(QIODevice::ReadOnly)) {
//         qWarning() << "Cannot open file for reading:" << filePath;
//         return;
//     }
//
//     QFileInfo fileInfo(filePath);
//     QString fileName = fileInfo.fileName();
//     QByteArray fileData = file.readAll();
//     file.close();
//
//     // Create payload: filename + file data
//     QByteArray payload;
//     QDataStream payloadStream(&payload, QIODevice::WriteOnly);
//     payloadStream.setVersion(QDataStream::Qt_6_5);
//     payloadStream << fileName << fileData;
//
//     // Send as MessageType::File
//     sendMessage(MessageType::File, payload);
//
//     qDebug() << "File sent successfully:" << fileName << "Size:" << fileData.size() << "bytes";
//     // // Create message structure: FILE:<filename>:<filedata>
//     // QByteArray message;
//     // QDataStream stream(&message, QIODevice::WriteOnly);
//     // stream.setVersion(QDataStream::Qt_6_5);
//     //
//     // stream << QString("FILE") << fileName << fileData;
//     //
//     // // Send the message
//     // if (clientSocket->write(message) == -1) {
//     //     qWarning() << "Failed to send file:" << clientSocket->errorString();
//     // } else {
//     //     qDebug() << "File sent successfully:" << fileName << "Size:" << fileData.size() << "bytes";
//     // }
// }