#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include "UndoCommand.h"
#include  <QVector>

namespace xcanvas {

    class UndoStack
    {
    public:
        void push(UndoCommand* cmd)
        {
            if (!cmd) {
                return;
            }

            if (!m_commands.isEmpty())
            {
                if (UndoCommand* last = m_commands.last(); last && last->mergeWith(cmd))
                {
                    delete cmd;
                    return;
                }
            }

            if (m_index < m_commands.size())
            {
                for (int i = m_index; i < m_commands.size(); ++i) {
                    delete m_commands[i];
                }
                m_commands.resize(m_index);
            }

            m_commands.append(cmd);
            cmd->redo();
            m_index = m_commands.size();
        }

        void undo()
        {
            if (m_index > 0)
            {
                --m_index;
                m_commands[m_index]->undo();
            }
        }

        void redo()
        {
            if (m_index < m_commands.size())
            {
                m_commands[m_index]->redo();
                ++m_index;
            }
        }

        void clear()
        {
            for (const auto* cmd : m_commands) {
                delete cmd;
            }
            m_commands.clear();
            m_index = 0;
        }

    private:
        QVector<UndoCommand*> m_commands;
        int m_index = 0;
    };

}

#endif //UNDOSTACK_H
