#ifndef QQMLVARIANTLISTMODEL_H
#define QQMLVARIANTLISTMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QVariant>
#include <QList>

class QQmlVariantListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY (int count READ count NOTIFY countChanged)

public:
    explicit QQmlVariantListModel (QObject * parent = Q_NULLPTR);
    ~QQmlVariantListModel (void);

public: // QAbstractItemModel interface reimplemented
    int rowCount (const QModelIndex & parent = QModelIndex ()) const;
    bool setData (const QModelIndex & index, const QVariant & value, int role);
    QVariant data (const QModelIndex & index, int role) const;
    QHash<int, QByteArray> roleNames (void) const;

public slots: // public API
    void clear (void);
    int count (void) const;
    bool isEmpty (void) const;
    void append (const QVariant & item);
    void prepend (const QVariant & item);
    void insert (int idx, const QVariant & item);
    void appendList (const QVariantList & itemList);
    void prependList (const QVariantList & itemList);
    void replace (int pos, const QVariant & item);
    void insertList (int idx, const QVariantList & itemList);
    void move (int idx, int pos);
    void remove (int idx);
    QVariant get (int idx) const;
    QVariantList list (void) const;

signals: // notifiers
    void countChanged (int count);

protected:
    void updateCounter (void);

private:
    int                    m_count;
    QVariantList           m_items;
    QHash<int, QByteArray> m_roles;
};

#endif // QQMLVARIANTLISTMODEL_H
