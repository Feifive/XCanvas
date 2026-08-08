#ifndef TEXTEDITCOMMIT_H
#define TEXTEDITCOMMIT_H

#include <QString>

struct TextEditCommit final
{
    QString originalText;
    QString editedText;

    bool shouldPush(const bool commit) const
    {
        return commit && editedText != originalText;
    }
};

#endif// TEXTEDITCOMMIT_H
