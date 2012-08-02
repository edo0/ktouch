/*
 *  Copyright 2012  Sebastian Gottfried <sebastiangottfried@web.de>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "keyboardlayoutcommands.h"

#include <core/keyboardlayout.h>
#include <core/abstractkey.h>
#include <core/key.h>

SetKeyboardLayoutTitleCommand::SetKeyboardLayoutTitleCommand(KeyboardLayout* layout, const QString& newTitle, QUndoCommand* parent) :
    QUndoCommand(parent),
    m_layout(layout),
    m_oldTitle(layout->title()),
    m_newTitle(newTitle)
{
}

void SetKeyboardLayoutTitleCommand::undo()
{
    m_layout->setTitle(m_oldTitle);
}

void SetKeyboardLayoutTitleCommand::redo()
{
    m_layout->setTitle(m_newTitle);
}

int SetKeyboardLayoutTitleCommand::id() const
{
    return 0xa7b18eae;
}

bool SetKeyboardLayoutTitleCommand::mergeWith(const QUndoCommand* other)
{
    const SetKeyboardLayoutTitleCommand* setKeyboardLayoutTitleCommand = static_cast<const SetKeyboardLayoutTitleCommand*>(other);

    if (m_layout != setKeyboardLayoutTitleCommand->m_layout)
        return false;

    m_newTitle = setKeyboardLayoutTitleCommand->m_newTitle;
    return true;
}

SetKeyboardLayoutNameCommand::SetKeyboardLayoutNameCommand(KeyboardLayout* layout, const QString& newName, QUndoCommand* parent) :
    QUndoCommand(parent),
    m_layout(layout),
    m_oldName(layout->name()),
    m_newName(newName)
{
}

void SetKeyboardLayoutNameCommand::undo()
{
    m_layout->setName(m_oldName);
}

void SetKeyboardLayoutNameCommand::redo()
{
    m_layout->setName(m_newName);
}

int SetKeyboardLayoutNameCommand::id() const
{
    return 0x24c31e8b;
}

bool SetKeyboardLayoutNameCommand::mergeWith(const QUndoCommand* other)
{
    const SetKeyboardLayoutNameCommand* setKeyboardLayoutNameCommand = static_cast<const SetKeyboardLayoutNameCommand*>(other);

    if (m_layout != setKeyboardLayoutNameCommand->m_layout)
        return false;

    m_newName = setKeyboardLayoutNameCommand->m_newName;
    return true;
}

SetKeyboardLayoutSizeCommand::SetKeyboardLayoutSizeCommand(KeyboardLayout* layout, const QSize& newSize, QUndoCommand* parent) :
    QUndoCommand(parent),
    m_layout(layout),
    m_oldSize(layout->size()),
    m_newSize(newSize)
{
    for (int i = 0; i < m_layout->keyCount(); i++)
    {
        AbstractKey* const key = m_layout->key(i);
        QRect rect(key->rect());

        if (key->left() + key->width() > newSize.width())
        {
            rect.moveLeft(newSize.width() - key->width());
        }

        if (key->top() + key->height() > newSize.height())
        {
            rect.moveTop(newSize.height() - key->height());
        }

        if (rect != key->rect())
        {
            new SetKeyGeometryCommand(m_layout, i, rect, this);
        }
    }
}

void SetKeyboardLayoutSizeCommand::undo()
{
    QUndoCommand::undo();
    m_layout->setSize(m_oldSize);
}

void SetKeyboardLayoutSizeCommand::redo()
{
    QUndoCommand::redo();
    m_layout->setSize(m_newSize);
}

int SetKeyboardLayoutSizeCommand::id() const
{
    return 0xaad8b7c8;
}

bool SetKeyboardLayoutSizeCommand::mergeWith(const QUndoCommand* other)
{
    Q_UNUSED(other);
    return false;
}


RemoveKeyCommand::RemoveKeyCommand(KeyboardLayout* layout, int keyIndex, QUndoCommand* parent) :
    QUndoCommand(parent),
    m_layout(layout),
    m_keyIndex(keyIndex),
    m_backupKey(0)
{
}

RemoveKeyCommand::~RemoveKeyCommand()
{
    delete m_backupKey;
}

void RemoveKeyCommand::undo()
{
    Q_ASSERT(m_backupKey);

    if (m_keyIndex == m_layout->keyCount())
    {
        m_layout->addKey(m_backupKey);
    }
    else
    {
        m_layout->insertKey(m_keyIndex, m_backupKey);
    }

    m_backupKey = 0;
}

void RemoveKeyCommand::redo()
{
    AbstractKey* abstractKey = m_layout->key(m_keyIndex);

    if (Key* key = qobject_cast<Key*>(abstractKey))
    {
        Key* backupKey = new Key();
        backupKey->copyFrom(key);
        m_backupKey = backupKey;
    }
    else if (SpecialKey* specialKey = qobject_cast<SpecialKey*>(abstractKey))
    {
        SpecialKey* specialKeyBackup = new SpecialKey();
        specialKey->copyFrom(specialKey);
        m_backupKey = specialKeyBackup;
    }

    m_layout->removeKey(m_keyIndex);
}

int RemoveKeyCommand::id() const
{
    return 0xf992f4a7;
}

bool RemoveKeyCommand::mergeWith(const QUndoCommand* other)
{
    Q_UNUSED(other)
    return false;
}

SetKeyGeometryCommand::SetKeyGeometryCommand(KeyboardLayout* layout, int keyIndex, const QRect& newRect, QUndoCommand* parent) :
    QUndoCommand(parent),
    m_layout(layout),
    m_keyIndex(keyIndex),
    m_oldRect(layout->key(keyIndex)->rect()),
    m_newRect(newRect)
{
}

void SetKeyGeometryCommand::undo()
{
    m_layout->key(m_keyIndex)->setRect(m_oldRect);
}

void SetKeyGeometryCommand::redo()
{
    m_layout->key(m_keyIndex)->setRect(m_newRect);
}

int SetKeyGeometryCommand::id() const
{
    return 0x7260c67c;
}

bool SetKeyGeometryCommand::mergeWith(const QUndoCommand* other)
{
    const SetKeyGeometryCommand* setKeyGeometryCommand = static_cast<const SetKeyGeometryCommand*>(other);

    if (m_layout != setKeyGeometryCommand->m_layout)
        return false;

    if (m_keyIndex != setKeyGeometryCommand->m_keyIndex)
        return false;

    m_newRect = setKeyGeometryCommand->m_newRect;
    return true;
}

SetKeyFingerIndexCommand::SetKeyFingerIndexCommand(KeyboardLayout* layout, int keyIndex, int newFingerIndex, QUndoCommand* parent) :
    QUndoCommand(parent),
    m_layout(layout),
    m_keyIndex(keyIndex),
    m_newFingerIndex(newFingerIndex)
{
    Key* key = qobject_cast<Key*>(m_layout->key(m_keyIndex));
    Q_ASSERT(key);
    m_oldFingerIndex = key->fingerIndex();
}

void SetKeyFingerIndexCommand::undo()
{
    Key* key = qobject_cast<Key*>(m_layout->key(m_keyIndex));
    Q_ASSERT(key);
    key->setFingerIndex(m_oldFingerIndex);
}

void SetKeyFingerIndexCommand::redo()
{
    Key* key = qobject_cast<Key*>(m_layout->key(m_keyIndex));
    Q_ASSERT(key);
    key->setFingerIndex(m_newFingerIndex);
}

int SetKeyFingerIndexCommand::id() const
{
    return 0xcecb02ad;
}

bool SetKeyFingerIndexCommand::mergeWith(const QUndoCommand* other)
{
    const SetKeyFingerIndexCommand* setKeyFingerIndexCommand = static_cast<const SetKeyFingerIndexCommand*>(other);

    if (m_layout != setKeyFingerIndexCommand->m_layout)
        return false;

    if (m_keyIndex != setKeyFingerIndexCommand->m_keyIndex)
        return false;

    m_newFingerIndex = setKeyFingerIndexCommand->m_newFingerIndex;
    return true;
}

SetKeyHasHapticMarkerCommand::SetKeyHasHapticMarkerCommand(KeyboardLayout* layout, int keyIndex, bool newHasHapticMarker, QUndoCommand* parent) :
    QUndoCommand(parent),
    m_layout(layout),
    m_keyIndex(keyIndex),
    m_newHasHapticMarker(newHasHapticMarker)
{
    Key* key = qobject_cast<Key*>(m_layout->key(m_keyIndex));
    Q_ASSERT(key);
    m_oldHasHapticMarker = key->hasHapticMarker();
}

void SetKeyHasHapticMarkerCommand::undo()
{
    Key* key = qobject_cast<Key*>(m_layout->key(m_keyIndex));
    Q_ASSERT(key);
    key->setHasHapticMarker(m_oldHasHapticMarker);
}

void SetKeyHasHapticMarkerCommand::redo()
{
    Key* key = qobject_cast<Key*>(m_layout->key(m_keyIndex));
    Q_ASSERT(key);
    key->setHasHapticMarker(m_newHasHapticMarker);
}

int SetKeyHasHapticMarkerCommand::id() const
{
    return 0xf436020c;
}

bool SetKeyHasHapticMarkerCommand::mergeWith(const QUndoCommand* other)
{
    const SetKeyHasHapticMarkerCommand* setKeyHasHapticMarkerCommand = static_cast<const SetKeyHasHapticMarkerCommand*>(other);

    if (m_layout != setKeyHasHapticMarkerCommand->m_layout)
        return false;

    if (m_keyIndex != setKeyHasHapticMarkerCommand->m_keyIndex)
        return false;

    m_newHasHapticMarker = setKeyHasHapticMarkerCommand->m_newHasHapticMarker;
    return true;
}

SetSpecialKeyTypeCommand::SetSpecialKeyTypeCommand(KeyboardLayout* layout, int keyIndex, SpecialKey::Type newType, QUndoCommand* parent) :
    QUndoCommand(parent),
    m_layout(layout),
    m_keyIndex(keyIndex),
    m_newType(newType)
{
    SpecialKey* key = qobject_cast<SpecialKey*>(m_layout->key(m_keyIndex));
    m_oldType = key->type();
}

void SetSpecialKeyTypeCommand::undo()
{
    SpecialKey* key = qobject_cast<SpecialKey*>(m_layout->key(m_keyIndex));
    key->setType(m_oldType);
}

void SetSpecialKeyTypeCommand::redo()
{
    SpecialKey* key = qobject_cast<SpecialKey*>(m_layout->key(m_keyIndex));
    key->setType(m_newType);
}

int SetSpecialKeyTypeCommand::id() const
{
    return 0xf1ce4bee;
}

bool SetSpecialKeyTypeCommand::mergeWith(const QUndoCommand* other)
{
    const SetSpecialKeyTypeCommand* setSpecialKeyTypeCommand = static_cast<const SetSpecialKeyTypeCommand*>(other);

    if (m_layout != setSpecialKeyTypeCommand->m_layout)
        return false;

    if (m_keyIndex != setSpecialKeyTypeCommand->m_keyIndex)
        return false;

    m_newType = setSpecialKeyTypeCommand->m_newType;
    return true;
}

SetSpecialKeyLabelCommand::SetSpecialKeyLabelCommand(KeyboardLayout* layout, int keyIndex, const QString& newLabel, QUndoCommand* parent) :
    QUndoCommand(parent),
    m_layout(layout),
    m_keyIndex(keyIndex),
    m_newLabel(newLabel)
{
    SpecialKey* key = qobject_cast<SpecialKey*>(m_layout->key(m_keyIndex));
    m_oldLabel = key->label();
}

void SetSpecialKeyLabelCommand::undo()
{
    SpecialKey* key = qobject_cast<SpecialKey*>(m_layout->key(m_keyIndex));
    key->setLabel(m_oldLabel);
}

void SetSpecialKeyLabelCommand::redo()
{
    SpecialKey* key = qobject_cast<SpecialKey*>(m_layout->key(m_keyIndex));
    key->setLabel(m_newLabel);
}

int SetSpecialKeyLabelCommand::id() const
{
    return 0x6a1ddec9;
}

bool SetSpecialKeyLabelCommand::mergeWith(const QUndoCommand* other)
{
    const SetSpecialKeyLabelCommand* setSpecialKeyLabelCommand = static_cast<const SetSpecialKeyLabelCommand*>(other);

    if (m_layout != setSpecialKeyLabelCommand->m_layout)
        return false;

    if (m_keyIndex != setSpecialKeyLabelCommand->m_keyIndex)
        return false;

    m_newLabel = setSpecialKeyLabelCommand->m_newLabel;
    return true;
}

SetSpecialKeyModifierIdCommand::SetSpecialKeyModifierIdCommand(KeyboardLayout* layout, int keyIndex, const QString& newModifiewId, QUndoCommand* parent) :
    QUndoCommand(parent),
    m_layout(layout),
    m_keyIndex(keyIndex),
    m_newModifierId(newModifiewId)
{
    SpecialKey* key = qobject_cast<SpecialKey*>(m_layout->key(m_keyIndex));
    m_oldModifierId = key->modifierId();
}

void SetSpecialKeyModifierIdCommand::undo()
{
    SpecialKey* key = qobject_cast<SpecialKey*>(m_layout->key(m_keyIndex));
    key->setModifierId(m_oldModifierId);
}

void SetSpecialKeyModifierIdCommand::redo()
{
    SpecialKey* key = qobject_cast<SpecialKey*>(m_layout->key(m_keyIndex));
    key->setModifierId(m_newModifierId);
}

int SetSpecialKeyModifierIdCommand::id() const
{
    return 0xafabebaf;
}

bool SetSpecialKeyModifierIdCommand::mergeWith(const QUndoCommand* other)
{
    const SetSpecialKeyModifierIdCommand* setSpecialKeyModifierIdCommand = static_cast<const SetSpecialKeyModifierIdCommand*>(other);

    if (m_layout != setSpecialKeyModifierIdCommand->m_layout)
        return false;

    if (m_keyIndex != setSpecialKeyModifierIdCommand->m_keyIndex)
        return false;

    m_newModifierId = setSpecialKeyModifierIdCommand->m_newModifierId;
    return true;
}
