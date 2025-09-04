/****************************************************************************
** Meta object code from reading C++ file 'NetworkManager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../NetworkManager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NetworkManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN14NetworkManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto NetworkManager::qt_create_metaobjectdata<qt_meta_tag_ZN14NetworkManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "NetworkManager",
        "connectionStatusChanged",
        "",
        "connected",
        "message",
        "voltageSourceReceived",
        "name",
        "node1",
        "node2",
        "value",
        "isSinusoidal",
        "offset",
        "amplitude",
        "frequency",
        "circuitFileReceived",
        "signalDataReceived",
        "std::map<double,double>",
        "data",
        "signalName",
        "dataReceived",
        "type",
        "newConnection",
        "readyRead",
        "socketError",
        "QAbstractSocket::SocketError",
        "error",
        "socketDisconnected"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'connectionStatusChanged'
        QtMocHelpers::SignalData<void(bool, const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'voltageSourceReceived'
        QtMocHelpers::SignalData<void(const QString &, const QString &, const QString &, double, bool, double, double, double)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 }, { QMetaType::QString, 7 }, { QMetaType::QString, 8 }, { QMetaType::Double, 9 },
            { QMetaType::Bool, 10 }, { QMetaType::Double, 11 }, { QMetaType::Double, 12 }, { QMetaType::Double, 13 },
        }}),
        // Signal 'circuitFileReceived'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'signalDataReceived'
        QtMocHelpers::SignalData<void(const std::map<double,double> &, const QString &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 16, 17 }, { QMetaType::QString, 18 },
        }}),
        // Signal 'dataReceived'
        QtMocHelpers::SignalData<void(const QByteArray &, const QString &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QByteArray, 17 }, { QMetaType::QString, 20 },
        }}),
        // Slot 'newConnection'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'readyRead'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'socketError'
        QtMocHelpers::SlotData<void(QAbstractSocket::SocketError)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 24, 25 },
        }}),
        // Slot 'socketDisconnected'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<NetworkManager, qt_meta_tag_ZN14NetworkManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject NetworkManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14NetworkManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14NetworkManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14NetworkManagerE_t>.metaTypes,
    nullptr
} };

void NetworkManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<NetworkManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->connectionStatusChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 1: _t->voltageSourceReceived((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[6])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[7])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[8]))); break;
        case 2: _t->circuitFileReceived(); break;
        case 3: _t->signalDataReceived((*reinterpret_cast< std::add_pointer_t<std::map<double,double>>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 4: _t->dataReceived((*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 5: _t->newConnection(); break;
        case 6: _t->readyRead(); break;
        case 7: _t->socketError((*reinterpret_cast< std::add_pointer_t<QAbstractSocket::SocketError>>(_a[1]))); break;
        case 8: _t->socketDisconnected(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 7:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QAbstractSocket::SocketError >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (NetworkManager::*)(bool , const QString & )>(_a, &NetworkManager::connectionStatusChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (NetworkManager::*)(const QString & , const QString & , const QString & , double , bool , double , double , double )>(_a, &NetworkManager::voltageSourceReceived, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (NetworkManager::*)()>(_a, &NetworkManager::circuitFileReceived, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (NetworkManager::*)(const std::map<double,double> & , const QString & )>(_a, &NetworkManager::signalDataReceived, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (NetworkManager::*)(const QByteArray & , const QString & )>(_a, &NetworkManager::dataReceived, 4))
            return;
    }
}

const QMetaObject *NetworkManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *NetworkManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14NetworkManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int NetworkManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void NetworkManager::connectionStatusChanged(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void NetworkManager::voltageSourceReceived(const QString & _t1, const QString & _t2, const QString & _t3, double _t4, bool _t5, double _t6, double _t7, double _t8)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2, _t3, _t4, _t5, _t6, _t7, _t8);
}

// SIGNAL 2
void NetworkManager::circuitFileReceived()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void NetworkManager::signalDataReceived(const std::map<double,double> & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void NetworkManager::dataReceived(const QByteArray & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}
QT_WARNING_POP
