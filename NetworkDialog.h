#ifndef NETWORKDIALOG_H
#define NETWORKDIALOG_H

#include <QDialog>
#include <QButtonGroup>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QRadioButton;
class QLabel;
QT_END_NAMESPACE

class NetworkDialog : public QDialog {
    Q_OBJECT

public:
    explicit NetworkDialog(QWidget* parent = nullptr);
    ~NetworkDialog();

    QString getHost() const;
    quint16 getPort() const;
    bool isServer() const;

    private slots:
        void toggleServerClient();

private:
    QRadioButton* serverRadio;
    QRadioButton* clientRadio;
    QLineEdit* hostEdit;
    QLineEdit* portEdit;
    QPushButton* connectButton;
    QPushButton* cancelButton;
    QLabel* hostLabel;

    QString getLocalIP();
};

#endif // NETWORKDIALOG_H