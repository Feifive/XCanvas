#ifndef EDITTEXTCOMMAND_H
#define EDITTEXTCOMMAND_H

#include <QUndoCommand>
#include "ShapeManager.h"
#include "ShapeText.h"

namespace xcanvas {
class EditTextCommand final : public QUndoCommand
{
public:
    EditTextCommand(ShapeManager* manager, ShapeText* shape, QString beforeText, QString afterText, QUndoCommand* parent = nullptr)
        : QUndoCommand(parent), m_manager(manager), m_shape(shape), m_beforeText(std::move(beforeText)), m_afterText(std::move(afterText))
    {
        setText("Edit Text");
    }

    void undo() override
    {
        if (!m_shape)
        {
            return;
        }
        m_shape->setText(m_beforeText);
        if (m_manager)
        {
            m_manager->invalidateSelectedRect();
        }
    }

    void redo() override
    {
        if (!m_shape)
        {
            return;
        }
        m_shape->setText(m_afterText);
        if (m_manager)
        {
            m_manager->invalidateSelectedRect();
        }
    }

private:
    ShapeManager* m_manager;
    ShapeText*    m_shape;
    QString       m_beforeText;
    QString       m_afterText;
};
}

#endif //EDITTEXTCOMMAND_H
