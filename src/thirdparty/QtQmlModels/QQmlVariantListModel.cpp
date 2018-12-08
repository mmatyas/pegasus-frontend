
#include "QQmlVariantListModel.h"

#define NO_PARENT QModelIndex ()
#define BASE_ROLE Qt::UserRole
#define EMPTY_STR QStringLiteral ("")
#define EMPTY_BA  QByteArrayLiteral ("")

/*!
    \class QQmlVariantListModel

    \ingroup QT_QML_MODELS

    \brief Provides a generic way to generate a list model from QVariant, suitable for QML

    QQmlVariantListModel is a convenience subclass \c QAbstractListModel that makes use of the versatile
    nature of QVariant to allow creating a list model from every type :
    \li Booleans
    \li Numbers
    \li Strings
    \li Lists
    \li Maps
    \li Object pointers
    \li etc...

    This is a far better way than to expose directly a \c QList<____> inside a \c QVariant.

    And this is far simpler than doing all Qt model stuff manually : no subclassing or reimplementing need.

    The class was designed so that most of the added API is really common with \c QList one.

    \b Note : Simply needs that the type items inherits is handled by Qt MetaType system and \c QVariant.

    \sa QQmlObjectListModel
*/


/*!
    \details Constructs a new model that will hold QVariant as items.

    \param parent The parent object for the model memory management
*/
QQmlVariantListModel::QQmlVariantListModel (QObject * parent) : QAbstractListModel (parent)
  , m_count(0)
  , m_items()
  , m_roles()
{
    m_roles.insert (BASE_ROLE, QByteArrayLiteral ("qtVariant"));
}

/*!
    \internal
*/
QQmlVariantListModel::~QQmlVariantListModel (void) {
    clear ();
}

/*!
    \internal
*/
int QQmlVariantListModel::rowCount (const QModelIndex & parent) const
{
    Q_UNUSED (parent);
    return m_items.count ();
}

/*!
    \details Returns the data in a specific index for a given role.

    Reimplemented for QAbstractItemModel.

    \param index The item index (row, column and parent)
    \param role The role
    \return The data in the role

    \b Note : the \c 0 role contains the QVariant itself.
*/
QVariant QQmlVariantListModel::data (const QModelIndex & index, int role) const
{
    QVariant ret;
    int idx = index.row ();
    if (idx >= 0 && idx < count () && role == BASE_ROLE) {
        ret = m_items.value (idx);
    }
    return ret;
}

/*!
    \details Returns the roles available in the model.

    Reimplemented for QAbstractItemModel.

    \return The hash table of role to name matching

    \b Note : the only role is \c 'qtVariant'.
*/
QHash<int, QByteArray> QQmlVariantListModel::roleNames () const
{
    return m_roles;
}

/*!
    \details Modifies the data in a specific index for a given role.

    Reimplemented for QAbstractItemModel.

    \param index The item index (row, column and parent)
    \param value The data to write
    \param role The role
    \return Weither the modification was done

    \b Note : as the only role is \c 0 ('qtVariant'), it replaces the QVariant value
*/
bool QQmlVariantListModel::setData (const QModelIndex & index, const QVariant & value, int role)
{
    bool ret = false;
    int idx = index.row ();
    if (idx >= 0 && idx < count () && role == BASE_ROLE) {
        m_items.replace (idx, value);
        QModelIndex item = QAbstractListModel::index (idx, 0, NO_PARENT);
        emit dataChanged (item, item, QVector<int> (1, role));
        ret = true;
    }
    return ret;
}

/*!
    \details Counts the items in the model.

    \return The count of items in the model
*/
int QQmlVariantListModel::count () const
{
    return m_items.size ();
}

/*!
    \details Tests the content of the model.

    \return Whether the model contains no item
*/
bool QQmlVariantListModel::isEmpty () const
{
    return m_items.isEmpty ();
}

/*!
    \details Delete all the items in the model.
*/
void QQmlVariantListModel::clear ()
{
    if (!m_items.isEmpty ()) {
        beginRemoveRows (NO_PARENT, 0, count () -1);
        m_items.clear ();
        endRemoveRows ();
        updateCounter ();
    }
}

/*!
    \details Adds the given item at the end of the model.

    \param item The variant value

    \sa prepend(QVariant), insert(int,QVariant)
*/
void QQmlVariantListModel::append (const QVariant & item)
{
    int pos = m_items.count ();
    beginInsertRows (NO_PARENT, pos, pos);
    m_items.append (item);
    endInsertRows ();
    updateCounter ();
}

/*!
    \details Adds the given item at the beginning of the model.

    \param item The variant value

    \sa append(QVariant), insert(int,QVariant)
*/
void QQmlVariantListModel::prepend (const QVariant & item)
{
    beginInsertRows (NO_PARENT, 0, 0);
    m_items.prepend (item);
    endInsertRows ();
    updateCounter ();
}

/*!
    \details Adds the given item at a certain position in the model.

    \param idx The position where the item must be added
    \param item The variant value

    \sa append(QVariant), prepend(QVariant)
*/
void QQmlVariantListModel::insert (int idx, const QVariant & item)
{
    beginInsertRows (NO_PARENT, idx, idx);
    m_items.insert (idx, item);
    endInsertRows ();
    updateCounter ();
}

/*!
    \details Replace the variant at a certain position in the model with another value.

    \param pos The position where the item must be replaced
    \param item The variant value

    \b Note : this is the regular way in C++ to modify the variant value.
*/
void QQmlVariantListModel::replace (int pos, const QVariant & item)
{
    if (pos >= 0 && pos < count ()) {
        m_items.replace (pos, item);
        QModelIndex index = QAbstractListModel::index (pos, 0, NO_PARENT);
        emit dataChanged (index, index, QVector<int> (1, BASE_ROLE));
    }
}

/*!
    \details Adds the given list of items at the end of the model.

    \param itemList The list of items

    \sa prepend(QVariantList), insert(int, QVariantList)
*/
void QQmlVariantListModel::appendList (const QVariantList & itemList)
{
    if (!itemList.isEmpty ()) {
        int pos = m_items.count ();
        beginInsertRows (NO_PARENT, pos, pos + itemList.count () -1);
        m_items.append (itemList);
        endInsertRows ();
        updateCounter ();
    }
}

/*!
    \details Adds the given list of items at the beginning of the model.

    \param itemList The list of items

    \sa append(QVariantList), insert(int, QVariantList)
*/
void QQmlVariantListModel::prependList (const QVariantList & itemList)
{
    if (!itemList.isEmpty ()) {
        beginInsertRows (NO_PARENT, 0, itemList.count () -1);
        int offset = 0;
        foreach (QVariant item, itemList) {
            m_items.insert (offset, item);
        }
        endInsertRows ();
        updateCounter ();
    }
}

/*!
    \details Adds the given list of items at a certain position in the model.

    \param idx The position where the items must be added
    \param itemList The list of items

    \sa append(QVariantList), prepend(QVariantList)
*/
void QQmlVariantListModel::insertList (int idx, const QVariantList & itemList)
{
    if (!itemList.isEmpty ()) {
        beginInsertRows (NO_PARENT, idx, idx + itemList.count () -1);
        int offset = 0;
        foreach (QVariant item, itemList) {
            m_items.insert (idx + offset, item);
            offset++;
        }
        endInsertRows ();
        updateCounter ();
    }
}

/*!
    \details Moves an item from the model to another position.

    \param idx The current position of the item
    \param pos The position where it willl be after the move
*/
void QQmlVariantListModel::move (int idx, int pos)
{
    if (idx != pos) {
        // FIXME : use begin/end MoveRows when supported by Repeater, since then use remove/insert pair
        //beginMoveRows (NO_PARENT, idx, idx, NO_PARENT, (idx < pos ? pos +1 : pos));
        beginRemoveRows (NO_PARENT, idx, idx);
        beginInsertRows (NO_PARENT, pos, pos);
        m_items.move (idx, pos);
        endRemoveRows ();
        endInsertRows ();
        //endMoveRows ();
    }
}

/*!
    \details Remove an item from the model.

    \param idx The position of the item in the model
*/
void QQmlVariantListModel::remove (int idx)
{
    if (idx >= 0 && idx < m_items.size ()) {
        beginRemoveRows (NO_PARENT, idx, idx);
        m_items.removeAt (idx);
        endRemoveRows ();
        updateCounter ();
    }
}

/*!
    \details Retreives a model item as a standard Qt variant object.

    \param idx The position of the item in the model
    \return A variant containing the item
*/
QVariant QQmlVariantListModel::get (int idx) const
{
    QVariant ret;
    if (idx >= 0 && idx < m_items.size ()) {
        ret = m_items.value (idx);
    }
    return ret;
}

/*!
    \details Retreives all the items of the model as a standard Qt variant list.

    \return A \c QVariantList containing all the variants
*/
QVariantList QQmlVariantListModel::list () const
{
    return m_items;
}

/*!
    \internal
*/
void QQmlVariantListModel::updateCounter ()
{
    if (m_count != m_items.count ()) {
        m_count = m_items.count ();
        emit countChanged (m_count);
    }
}
