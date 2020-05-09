#ifndef QQMLOBJECTLISTMODEL_H
#define QQMLOBJECTLISTMODEL_H

#include <QAbstractListModel>
#include <QByteArray>
#include <QChar>
#include <QDebug>
#include <QHash>
#include <QMetaMethod>
#include <QMetaObject>
#include <QMetaProperty>
#include <QObject>
#include <QString>
#include <QStringBuilder>
#include <QVariant>
#include <QVector>

template<typename T> QVector<T> qVectorFromVariant (const QVariantList & list) {
    QVector<T> ret;
    ret.reserve (list.size ());
    for (QVariantList::const_iterator it = list.constBegin (); it != list.constEnd (); ++it) {
        const QVariant & var = static_cast<QVariant>(* it);
        ret.append (var.value<T> ());
    }
    return ret;
}

template<typename T> QVariantList qVectorToVariant (const QVector<T> & list) {
    QVariantList ret;
    ret.reserve (list.size ());
    for (typename QVector<T>::const_iterator it = list.constBegin (); it != list.constEnd (); ++it) {
        const T & val = static_cast<T>(* it);
        ret.append (QVariant::fromValue (val));
    }
    return ret;
}

// custom foreach for QVector, which uses no copy and check pointer non-null
#define FOREACH_PTR_IN_LIST(_type_, _var_, _list_) \
    for (typename QVector<_type_ *>::const_iterator it = _list_.constBegin (); it != _list_.constEnd (); ++it) \
        if (_type_ * _var_ = (* it))

class QQmlObjectListModelBase : public QAbstractListModel { // abstract Qt base class
    Q_OBJECT
    Q_PROPERTY (int count READ count NOTIFY countChanged)

public:
    explicit QQmlObjectListModelBase (QObject * parent = Q_NULLPTR) : QAbstractListModel (parent) { }

public slots: // virtual methods API for QML
    virtual int size (void) const = 0;
    virtual int count (void) const = 0;
    virtual bool isEmpty (void) const = 0;
    virtual bool contains (QObject * item) const = 0;
    virtual int indexOf (QObject * item) const = 0;
    virtual int roleForName (const QByteArray & name) const = 0;
    virtual void clear (void) = 0;
    virtual void append (QObject * item) = 0;
    virtual void prepend (QObject * item) = 0;
    virtual void insert (int idx, QObject * item) = 0;
    virtual void move (int idx, int pos) = 0;
    virtual void remove (QObject * item) = 0;
    virtual void remove (int idx) = 0;
    virtual QObject * get (int idx) const = 0;
    virtual QObject * get (const QString & uid) const = 0;
    virtual QObject * getFirst (void) const = 0;
    virtual QObject * getLast (void) const = 0;
    virtual QVariantList toVarArray (void) const = 0;

protected slots: // internal callback
    virtual void onItemPropertyChanged (void) = 0;

signals: // notifier
    void countChanged (void);
};

template<class ItemType> class QQmlObjectListModel : public QQmlObjectListModelBase {
public:
    explicit QQmlObjectListModel (QObject *          parent      = Q_NULLPTR,
                                  const QByteArray & displayRole = QByteArray (),
                                  const QByteArray & uidRole     = QByteArray ())
        : QQmlObjectListModelBase (parent)
        , m_count (0)
        , m_uidRoleName (uidRole)
        , m_dispRoleName (displayRole)
        , m_metaObj (ItemType::staticMetaObject)
    {
#ifndef NDEBUG
        static QSet<QByteArray> roleNamesBlacklist;
        if (roleNamesBlacklist.isEmpty ()) {
            roleNamesBlacklist << QByteArrayLiteral ("id")
                               << QByteArrayLiteral ("index")
                               << QByteArrayLiteral ("class")
                               << QByteArrayLiteral ("model")
                               << QByteArrayLiteral ("modelData");
        }
#endif // NDEBUG
        static const char * HANDLER = "onItemPropertyChanged()";
        m_handler = metaObject ()->method (metaObject ()->indexOfMethod (HANDLER));
        if (!displayRole.isEmpty ()) {
            m_roles.insert (Qt::DisplayRole, QByteArrayLiteral ("display"));
        }
        m_roles.insert (baseRole (), QByteArrayLiteral ("modelData"));
        const int len = m_metaObj.propertyCount ();
        for (int propertyIdx = 0, role = (baseRole () +1); propertyIdx < len; propertyIdx++, role++) {
            QMetaProperty metaProp = m_metaObj.property (propertyIdx);
            const QByteArray propName = QByteArray (metaProp.name ());
#ifndef NDEBUG
            if (roleNamesBlacklist.contains (propName)) {
                static const QByteArray CLASS_NAME = (QByteArrayLiteral ("QQmlObjectListModel<") % m_metaObj.className () % '>');
                qWarning () << "Can't have" << propName << "as a role name in" << CLASS_NAME.toStdString().c_str();
                continue;
            }
#endif // NDEBUG
            m_roles.insert (role, propName);
            if (metaProp.hasNotifySignal ()) {
                m_signalIdxToRole.insert (metaProp.notifySignalIndex (), role);
            }
        }
    }
    bool setData (const QModelIndex & index, const QVariant & value, int role) Q_DECL_FINAL {
        bool ret = false;
        ItemType * item = at (index.row ());
        const QByteArray rolename = (role != Qt::DisplayRole ? m_roles.value (role, emptyBA ()) : m_dispRoleName);
        if (item != Q_NULLPTR && role != baseRole () && !rolename.isEmpty ()) {
            ret = item->setProperty (rolename, value);
        }
        return ret;
    }
    QVariant data (const QModelIndex & index, int role) const Q_DECL_FINAL {
        QVariant ret;
        ItemType * item = at (index.row ());
        const QByteArray rolename = (role != Qt::DisplayRole ? m_roles.value (role, emptyBA ()) : m_dispRoleName);
        if (item != Q_NULLPTR && !rolename.isEmpty ()) {
            ret.setValue (role != baseRole () ? item->property (rolename) : QVariant::fromValue (static_cast<QObject *> (item)));
        }
        return ret;
    }
    QHash<int, QByteArray> roleNames (void) const Q_DECL_FINAL {
        return m_roles;
    }
    typedef typename QVector<ItemType *>::const_iterator const_iterator;
    const_iterator begin (void) const {
        return m_items.begin ();
    }
    const_iterator end (void) const {
        return m_items.end ();
    }
    const_iterator constBegin (void) const {
        return m_items.constBegin ();
    }
    const_iterator constEnd (void) const {
        return m_items.constEnd ();
    }

public: // C++ API
    ItemType * at (int idx) const {
        ItemType * ret = Q_NULLPTR;
        if (idx >= 0 && idx < m_items.size ()) {
            ret = m_items.value (idx);
        }
        return ret;
    }
    ItemType * getByUid (const QString & uid) const {
        return (!m_indexByUid.isEmpty () ? m_indexByUid.value (uid, Q_NULLPTR) : Q_NULLPTR);
    }
    int roleForName (const QByteArray & name) const Q_DECL_FINAL {
        return m_roles.key (name, -1);
    }
    int count (void) const Q_DECL_FINAL {
        return m_count;
    }
    int size (void) const Q_DECL_FINAL {
        return m_count;
    }
    bool isEmpty (void) const Q_DECL_FINAL {
        return m_items.isEmpty ();
    }
    bool contains (ItemType * item) const {
        return m_items.contains (item);
    }
    int indexOf (ItemType * item) const {
        return m_items.indexOf (item);
    }
    void clear (void) Q_DECL_FINAL {
        if (!m_items.isEmpty ()) {
            beginRemoveRows (noParent (), 0, m_items.count () -1);
            FOREACH_PTR_IN_LIST (ItemType, item, m_items) {
                dereferenceItem (item);
            }
            m_items.clear ();
            updateCounter ();
            endRemoveRows ();
        }
    }
    void append (ItemType * item) {
        if (item != Q_NULLPTR) {
            const int pos = m_items.count ();
            beginInsertRows (noParent (), pos, pos);
            m_items.append (item);
            referenceItem (item);
            updateCounter ();
            endInsertRows ();
        }
    }
    void prepend (ItemType * item) {
        if (item != Q_NULLPTR) {
            beginInsertRows (noParent (), 0, 0);
            m_items.prepend (item);
            referenceItem (item);
            updateCounter ();
            endInsertRows ();
        }
    }
    void insert (int idx, ItemType * item) {
        if (item != Q_NULLPTR) {
            beginInsertRows (noParent (), idx, idx);
            m_items.insert (idx, item);
            referenceItem (item);
            updateCounter ();
            endInsertRows ();
        }
    }
    void append (const QVector<ItemType *> & itemList) {
        if (!itemList.isEmpty ()) {
            const int pos = m_items.count ();
            beginInsertRows (noParent (), pos, pos + itemList.count () -1);
            m_items.reserve (m_items.count () + itemList.count ());
            m_items.append (itemList);
            FOREACH_PTR_IN_LIST (ItemType, item, itemList) {
                referenceItem (item);
            }
            updateCounter ();
            endInsertRows ();
        }
    }
    void prepend (const QVector<ItemType *> & itemList) {
        if (!itemList.isEmpty ()) {
            beginInsertRows (noParent (), 0, itemList.count () -1);
            m_items.reserve (m_items.count () + itemList.count ());
            int offset = 0;
            FOREACH_PTR_IN_LIST (ItemType, item, itemList) {
                m_items.insert (offset, item);
                referenceItem (item);
                offset++;
            }
            updateCounter ();
            endInsertRows ();
        }
    }
    void insert (int idx, const QVector<ItemType *> & itemList) {
        if (!itemList.isEmpty ()) {
            beginInsertRows (noParent (), idx, idx + itemList.count () -1);
            m_items.reserve (m_items.count () + itemList.count ());
            int offset = 0;
            FOREACH_PTR_IN_LIST (ItemType, item, itemList) {
                m_items.insert (idx + offset, item);
                referenceItem (item);
                offset++;
            }
            updateCounter ();
            endInsertRows ();
        }
    }
    void move (int idx, int pos) Q_DECL_FINAL {
        if (idx != pos) {
            // FIXME : use begin/end MoveRows when supported by Repeater, since then use remove/insert pair
            //beginMoveRows (noParent (), idx, idx, noParent (), (idx < pos ? pos +1 : pos));
            beginRemoveRows (noParent (), idx, idx);
            beginInsertRows (noParent (), pos, pos);
            m_items.move (idx, pos);
            endRemoveRows ();
            endInsertRows ();
            //endMoveRows ();
        }
    }
    void remove (ItemType * item) {
        if (item != Q_NULLPTR) {
            const int idx = m_items.indexOf (item);
            remove (idx);
        }
    }
    void remove (int idx) Q_DECL_FINAL {
        if (idx >= 0 && idx < m_items.size ()) {
            beginRemoveRows (noParent (), idx, idx);
            ItemType * item = m_items.takeAt (idx);
            dereferenceItem (item);
            updateCounter ();
            endRemoveRows ();
        }
    }
    ItemType * first (void) const {
        return m_items.first ();
    }
    ItemType * last (void) const {
        return m_items.last ();
    }
    const QVector<ItemType *> & asList (void) const {
        return m_items;
    }
    using SorterFn = std::function<bool(const ItemType* const, const ItemType* const)>;
    void sort_uniq(const SorterFn& sorter) {
        beginResetModel();

        // keep only unique pointers
        std::sort(m_items.begin(), m_items.end());
        const auto del_it = std::unique(m_items.begin(), m_items.end());
        for (auto it = del_it; it != m_items.end(); ++it) {
            if (*it != Q_NULLPTR) {
                disconnect(this, Q_NULLPTR, *it, Q_NULLPTR);
                disconnect(*it, Q_NULLPTR, this, Q_NULLPTR);
            }
        }
        m_items.erase(del_it, m_items.end());
        m_items.squeeze();

        // sort the actual entries
        // TODO: avoid double sorting
        std::sort(m_items.begin(), m_items.end(), sorter);

        updateCounter();
        endResetModel();
    }

public: // QML slots implementation
    void append (QObject * item) Q_DECL_FINAL {
        append (qobject_cast<ItemType *> (item));
    }
    void prepend (QObject * item) Q_DECL_FINAL {
        prepend (qobject_cast<ItemType *> (item));
    }
    void insert (int idx, QObject * item) Q_DECL_FINAL {
        insert (idx, qobject_cast<ItemType *> (item));
    }
    void remove (QObject * item) Q_DECL_FINAL {
        remove (qobject_cast<ItemType *> (item));
    }
    bool contains (QObject * item) const Q_DECL_FINAL {
        return contains (qobject_cast<ItemType *> (item));
    }
    int indexOf (QObject * item) const Q_DECL_FINAL {
        return indexOf (qobject_cast<ItemType *> (item));
    }
    int indexOf (const QString & uid) const {
        return indexOf (get (uid));
    }
    QObject * get (int idx) const Q_DECL_FINAL {
        return static_cast<QObject *> (at (idx));
    }
    QObject * get (const QString & uid) const Q_DECL_FINAL {
        return static_cast<QObject *> (getByUid (uid));
    }
    QObject * getFirst (void) const Q_DECL_FINAL {
        return static_cast<QObject *> (first ());
    }
    QObject * getLast (void) const Q_DECL_FINAL {
        return static_cast<QObject *> (last ());
    }
    QVariantList toVarArray (void) const Q_DECL_FINAL {
        return qVectorToVariant<ItemType *> (m_items);
    }

protected: // internal stuff
    static const QString & emptyStr (void) {
        static const QString ret = QStringLiteral ("");
        return ret;
    }
    static const QByteArray & emptyBA (void) {
        static const QByteArray ret = QByteArrayLiteral ("");
        return ret;
    }
    static const QModelIndex & noParent (void) {
        static const QModelIndex ret = QModelIndex ();
        return ret;
    }
    static const int & baseRole (void) {
        static const int ret = Qt::UserRole;
        return ret;
    }
    int rowCount (const QModelIndex & parent = QModelIndex ()) const Q_DECL_FINAL {
        return (!parent.isValid () ? m_items.count () : 0);
    }
    void referenceItem (ItemType * item) {
        if (item != Q_NULLPTR) {
            /*if (!item->parent ()) {
                item->setParent (this);
            }*/
            for (QHash<int, int>::const_iterator it = m_signalIdxToRole.constBegin (); it != m_signalIdxToRole.constEnd (); ++it) {
                connect (item, item->metaObject ()->method (it.key ()), this, m_handler, Qt::UniqueConnection);
            }
            if (!m_uidRoleName.isEmpty ()) {
                const QString key = m_indexByUid.key (item, emptyStr ());
                if (!key.isEmpty ()) {
                    m_indexByUid.remove (key);
                }
                const QString value = item->property (m_uidRoleName).toString ();
                if (!value.isEmpty ()) {
                    m_indexByUid.insert (value, item);
                }
            }
        }
    }
    void dereferenceItem (ItemType * item) {
        if (item != Q_NULLPTR) {
            disconnect (this, Q_NULLPTR, item, Q_NULLPTR);
            disconnect (item, Q_NULLPTR, this, Q_NULLPTR);
            if (!m_uidRoleName.isEmpty ()) {
                const QString key = m_indexByUid.key (item, emptyStr ());
                if (!key.isEmpty ()) {
                    m_indexByUid.remove (key);
                }
            }
            if (item->parent () == this) { // FIXME : maybe that's not the best way to test ownership ?
                item->deleteLater ();
            }
        }
    }
    void onItemPropertyChanged (void) Q_DECL_FINAL {
        ItemType * item = qobject_cast<ItemType *> (sender ());
        const int row = m_items.indexOf (item);
        const int sig = senderSignalIndex ();
        const int role = m_signalIdxToRole.value (sig, -1);
        if (row >= 0 && role >= 0) {
            const QModelIndex index = QAbstractListModel::index (row, 0, noParent ());
            QVector<int> rolesList;
            rolesList.append (role);
            if (m_roles.value (role) == m_dispRoleName) {
                rolesList.append (Qt::DisplayRole);
            }
            emit dataChanged (index, index, rolesList);
        }
        if (!m_uidRoleName.isEmpty ()) {
            const QByteArray roleName = m_roles.value (role, emptyBA ());
            if (!roleName.isEmpty () && roleName == m_uidRoleName) {
                const QString key = m_indexByUid.key (item, emptyStr ());
                if (!key.isEmpty ()) {
                    m_indexByUid.remove (key);
                }
                const QString value = item->property (m_uidRoleName).toString ();
                if (!value.isEmpty ()) {
                    m_indexByUid.insert (value, item);
                }
            }
        }
    }
    inline void updateCounter (void) {
        if (m_count != m_items.count ()) {
            m_count = m_items.count ();
            emit countChanged ();
        }
    }

private: // data members
    int                        m_count;
    QByteArray                 m_uidRoleName;
    QByteArray                 m_dispRoleName;
    QMetaObject                m_metaObj;
    QMetaMethod                m_handler;
    QHash<int, QByteArray>     m_roles;
    QHash<int, int>            m_signalIdxToRole;
    QVector<ItemType *>        m_items;
    QHash<QString, ItemType *> m_indexByUid;
};

#define QML_OBJMODEL_PROPERTY(type, name) \
    protected: Q_PROPERTY (QQmlObjectListModelBase * name READ name CONSTANT) \
    private: QQmlObjectListModel<type> * const m_##name; \
    public: QQmlObjectListModel<type> * name (void) const { return m_##name; } \
    private:

#endif // QQMLOBJECTLISTMODEL_H
