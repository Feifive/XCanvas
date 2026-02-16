#ifndef GROUPCOMMAND_H
#define GROUPCOMMAND_H

#include <QUndoCommand>
#include <map>

#include "Shape.h"
#include "ShapeManager.h"

namespace xcanvas
{
class GroupCommand final : public QUndoCommand
{
public:
    GroupCommand(ShapeManager* manager, std::map<Shape*, QString> beforeGroupId, const QString& cmdText, QUndoCommand* parent = nullptr)
        : QUndoCommand(parent), m_beforeGroupId(std::move(beforeGroupId))
    {
        Q_UNUSED(manager);
        setText(cmdText);
        for (auto const& [shape, groupId] : m_beforeGroupId)
        {
            m_afterGroupId[shape] = shape->groupId();
        }
    }

    void undo() override
    {
        for (auto const& [shape, groupId] : m_beforeGroupId)
        {
            shape->setGroupId(groupId);
        }
    }

    void redo() override
    {
        for (auto const& [shape, groupId] : m_afterGroupId)
        {
            shape->setGroupId(groupId);
        }
    }

private:
    std::map<Shape*, QString> m_beforeGroupId;
    std::map<Shape*, QString> m_afterGroupId;
};
}// namespace xcanvas

#endif// GROUPCOMMAND_H
