/****************************************************************************
** Meta object code from reading C++ file 'QubitViewer.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../omniq-debugger/src/widgets/QubitViewer.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QubitViewer.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN11QubitViewerE_t {};
} // unnamed namespace

template <> constexpr inline auto QubitViewer::qt_create_metaobjectdata<qt_meta_tag_ZN11QubitViewerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "QubitViewer",
        "qubitSelectionChanged",
        "",
        "qubit",
        "setSelectedQubit",
        "updateQubitInfo",
        "prob0",
        "prob1",
        "alpha",
        "beta",
        "theta",
        "phi",
        "updateMeasurementResults",
        "QList<int>",
        "results",
        "updateExpectationValues",
        "x",
        "y",
        "z",
        "resetQubitData",
        "onQubitSelected",
        "onMeasurementBasisChanged",
        "basis",
        "onMeasureButtonClicked",
        "onResetButtonClicked",
        "onAnimateState"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'qubitSelectionChanged'
        QtMocHelpers::SignalData<void(int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'setSelectedQubit'
        QtMocHelpers::SlotData<void(int)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'updateQubitInfo'
        QtMocHelpers::SlotData<void(int, double, double, double, double, double, double)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { QMetaType::Double, 6 }, { QMetaType::Double, 7 }, { QMetaType::Double, 8 },
            { QMetaType::Double, 9 }, { QMetaType::Double, 10 }, { QMetaType::Double, 11 },
        }}),
        // Slot 'updateQubitInfo'
        QtMocHelpers::SlotData<void(int, double, double, double, double, double)>(5, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { QMetaType::Double, 6 }, { QMetaType::Double, 7 }, { QMetaType::Double, 8 },
            { QMetaType::Double, 9 }, { QMetaType::Double, 10 },
        }}),
        // Slot 'updateQubitInfo'
        QtMocHelpers::SlotData<void(int, double, double, double, double)>(5, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { QMetaType::Double, 6 }, { QMetaType::Double, 7 }, { QMetaType::Double, 8 },
            { QMetaType::Double, 9 },
        }}),
        // Slot 'updateQubitInfo'
        QtMocHelpers::SlotData<void(int, double, double, double)>(5, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { QMetaType::Double, 6 }, { QMetaType::Double, 7 }, { QMetaType::Double, 8 },
        }}),
        // Slot 'updateQubitInfo'
        QtMocHelpers::SlotData<void(int, double, double)>(5, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { QMetaType::Double, 6 }, { QMetaType::Double, 7 },
        }}),
        // Slot 'updateMeasurementResults'
        QtMocHelpers::SlotData<void(int, const QVector<int> &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { 0x80000000 | 13, 14 },
        }}),
        // Slot 'updateExpectationValues'
        QtMocHelpers::SlotData<void(int, double, double, double)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { QMetaType::Double, 16 }, { QMetaType::Double, 17 }, { QMetaType::Double, 18 },
        }}),
        // Slot 'resetQubitData'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onQubitSelected'
        QtMocHelpers::SlotData<void(int)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'onMeasurementBasisChanged'
        QtMocHelpers::SlotData<void(const QString &)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 22 },
        }}),
        // Slot 'onMeasureButtonClicked'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onResetButtonClicked'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAnimateState'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<QubitViewer, qt_meta_tag_ZN11QubitViewerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject QubitViewer::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11QubitViewerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11QubitViewerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11QubitViewerE_t>.metaTypes,
    nullptr
} };

void QubitViewer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<QubitViewer *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->qubitSelectionChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->setSelectedQubit((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->updateQubitInfo((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[6])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[7]))); break;
        case 3: _t->updateQubitInfo((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[6]))); break;
        case 4: _t->updateQubitInfo((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[5]))); break;
        case 5: _t->updateQubitInfo((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4]))); break;
        case 6: _t->updateQubitInfo((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3]))); break;
        case 7: _t->updateMeasurementResults((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[2]))); break;
        case 8: _t->updateExpectationValues((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4]))); break;
        case 9: _t->resetQubitData(); break;
        case 10: _t->onQubitSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->onMeasurementBasisChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 12: _t->onMeasureButtonClicked(); break;
        case 13: _t->onResetButtonClicked(); break;
        case 14: _t->onAnimateState(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 7:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (QubitViewer::*)(int )>(_a, &QubitViewer::qubitSelectionChanged, 0))
            return;
    }
}

const QMetaObject *QubitViewer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QubitViewer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11QubitViewerE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int QubitViewer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void QubitViewer::qubitSelectionChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}
QT_WARNING_POP
