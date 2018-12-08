
#include "QQmlObjectListModel.h"

/*!
    \class QQmlObjectListModel

    \ingroup QT_QML_MODELS

    \brief Provides a generic way to generate a list model from QObject derived class, suitable for QML

    QQmlObjectListModel is a convenience subclass \c QAbstractListModel that makes use of C++ templates
    and Qt Meta Object to extract properties from a \c QObject derived class and create according roles
    inside the model.

    This is a far better way than to expose directly a \c QList<QObject *> inside a \c QVariant.

    And this is far simpler than doing all Qt model stuff manually : no subclassing or reimplementing need.

    The class was designed so that most of the added API is really common with \c QList one.

    \b Note : Simply needs that the class used for items inherits \c QObject and has Qt Meta Properties.

    \sa QQmlVariantListModel
*/

/*!
    \fn static QQmlObjectListModel * QQmlObjectListModel::create (QObject * parent = Q_NULLPTR)

    \details A factory to create a new model from a class that will be used as item type.

    \tparam ItemType The class to use as item
    \param parent The owner object for the model
    \return The newly created and configurerd model

    This is a template method, meant to be used like this :
    \code
        QQmlObjectListModel * myModel = QQmlObjectListModel::create<MyItemClass>(this);
    \endcode

    No other customization in needed after that.
*/

/*!
    \fn ItemType * QQmlObjectListModel::getAs () const

    \details A template method to retreive a given item as a precise \c T* Qt object pointer.

    \tparam ItemType The class to use as return pointer type
    \param idx The position of the item in the model
    \return The typed pointer to the object, or \c Q_NULLPTR if the type doesn't match

    \sa get(int) const, getByUid(QString) const
*/

/*!
    \fn QList<ItemType *> QQmlObjectListModel::listAs () const

    \details A template method to retreive all the items as \c QList<T*> typed Qt object pointer list.

    \tparam ItemType The class as object type to use in the returned pointer list
    \return A strongly typed \c QList of items Qt object pointers

    \sa list() const
*/


/*!
    \details Returns the data in a specific index for a given role.

    Reimplemented for QAbstractItemModel.

    \param index The item index (row, column and parent)
    \param role The role for property
    \return The data in the role

    \b Note : the \c 0 role is a pointer to item object itself.
*/

/*!
    \details Returns the roles available in the model.

    Reimplemented for QAbstractItemModel.

    \return The hash table of role to name matching

    \b Note : an additional \c 'qtObject' role is added for convenience.
*/

/*!
    \details Modifies the data in a specific index for a given role.

    Reimplemented for QAbstractItemModel.

    \param index The item index (row, column and parent)
    \param value The data to write
    \param role The role for property
    \return Weither the modification was done
*/

/*!
    \details Returns the role associated to the given property name.

    \param name The property name inside the item class
    \return The matching role, \c -1 if not found
*/


/*!
    \details Counts the items in the model.

    \return The count of items in the model
*/

/*!
    \details Counts the items in the model.

    \return The count of items in the model
*/
/*!
    \details Tests the content of the model.

    \return Whether the model contains no item
*/

/*!
    \details Tests the presence of a given item in the model.

    \param item The pointer to the item
    \return Whether the item was found
*/

/*!
    \details Finds the position of given item in the model.

    \param item The pointer to the item
    \return The row index of the item, \c -1 if not found
*/

/*!
    \details Delete all the items in the model.

    \b Note : The items objects will be removed from the model but they will be destructed
    only if they have no parent (because the model took the ownership).
*/


/*!
    \details Adds the given item at the end of the model.

    \param item The pointer to the item

    \sa prepend(QObject*), insert(int,QObject*)
*/


/*!
    \details Adds the given item at the beginning of the model.

    \param item The pointer to the item

    \sa append(QObject*), insert(int,QObject*)
*/


/*!
    \details Adds the given item at a certain position in the model.

    \param idx The position where the item must be added
    \param item The pointer to the item

    \sa append(QObject*), prepend(QObject*)
*/


/*!
    \details Adds the given list of items at the end of the model.

    \param itemList The list of items

    \sa prepend(QObjectList), insert(int, QObjectList)
*/


/*!
    \details Adds the given list of items at the beginning of the model.

    \param itemList The list of items

    \sa append(QObjectList), insert(int, QObjectList)
*/


/*!
    \details Adds the given list of items at a certain position in the model.

    \param idx The position where the items must be added
    \param itemList The list of items

    \sa append(QObjectList), prepend(QObjectList)
*/

/*!
    \details Moves an item from the model to another position.

    \param idx The current position of the item
    \param pos The position where it willl be after the move
*/


/*!
    \details Remove an item from the model.

    \param item The pointer to the item object
*/


/*!
    \details Remove an item from the model.

    \param idx The position of the item in the model
*/


/*!
    \details Retreives a model item as standard Qt object pointer.

    \param idx The position of the item in the model
    \return A pointer to the \c QObject

    \sa getAs(int) const, getByUid(QString) const
*/

/*!
    \details Retreives the first item of the model as standard Qt object pointer.

    \return A pointer to the \c QObject

    \sa last()
*/

/*!
    \details Retreives the last item of the model as standard Qt object pointer.

    \return A pointer to the \c QObject

    \sa first()
*/

/*!
    \details Retreives all the items of the model as a standard Qt object pointer list.

    \return A \c QObjectList containing all the pointers

    \sa listAs() const
*/

/*!
    \details Retreives a model item as standard Qt object pointer using its indexed property.
    Works only if setRoleNameForUid() was used correctly at start.

    \param uid The identifier value that points to the item in the index
    \return A pointer to the \c QObject

    \sa getAs(int) const, get(int) const
*/

/*!
    \details Sets which property of the items will be used as an index key.
    This can be used or not, but if not, getByUid() won't work.

    Ideally, the property used for UID should not change after items are added
    to the model, because it could have some side-effects.

    \param name The name of the property / role that is used as the index key
*/
