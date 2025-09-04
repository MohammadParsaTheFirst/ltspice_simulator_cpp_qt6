#include "NetworkDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHostAddress>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>

#include <QHostAddress>        // For QHostAddress
#include <QAbstractSocket>     // For QAbstractSocket::IPv4Protocol
#include <QNetworkInterface>   // For QNetworkInterface::allAddresses


NetworkDialog::NetworkDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Network Configuration");
    setMinimumWidth(300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Server/Client selection
    QHBoxLayout* roleLayout = new QHBoxLayout();
    serverRadio = new QRadioButton("Server", this);
    clientRadio = new QRadioButton("Client", this);
    clientRadio->setChecked(true);
    roleLayout->addWidget(serverRadio);
    roleLayout->addWidget(clientRadio);
    mainLayout->addLayout(roleLayout);

    // Host input
    QFormLayout* formLayout = new QFormLayout();
    hostLabel = new QLabel("Server Host:", this);
    hostEdit = new QLineEdit("127.0.0.1", this);//"localhost", this);
    formLayout->addRow(hostLabel, hostEdit);
    mainLayout->addLayout(formLayout);

    // Port input
    portEdit = new QLineEdit("12345", this);
    portEdit->setValidator(new QIntValidator(1024, 65535, this));
    formLayout->addRow("Port:", portEdit);

    // Add local IP info for convenience
    QLabel* ipInfo = new QLabel(this);
    QString localIp = getLocalIP();
    ipInfo->setText("Your local IP: " + localIp);
    ipInfo->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", ipInfo);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    connectButton = new QPushButton("Connect", this);
    cancelButton = new QPushButton("Cancel", this);
    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(serverRadio, &QRadioButton::toggled, this, &NetworkDialog::toggleServerClient);
    connect(connectButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    toggleServerClient();
}

NetworkDialog::~NetworkDialog() {
}

QString NetworkDialog::getHost() const {
    return hostEdit->text();
}

quint16 NetworkDialog::getPort() const {
    return portEdit->text().toUShort();
}

bool NetworkDialog::isServer() const {
    return serverRadio->isChecked();
}

void NetworkDialog::toggleServerClient() {
    bool isClient = clientRadio->isChecked();
    hostLabel->setEnabled(isClient);
    hostEdit->setEnabled(isClient);
}

// Add helper function to get local IP
QString NetworkDialog::getLocalIP() {
    QString localIp = "127.0.0.1";
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();

    for (const QHostAddress &address : ipAddressesList) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
            address != QHostAddress(QHostAddress::LocalHost)) {
            localIp = address.toString();
            break;
            }
    }
    return localIp;
}