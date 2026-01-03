#ifndef SELECTIONSUMMARY_H
#define SELECTIONSUMMARY_H

struct SelectionSummary
{
    int total = 0;
    int vectorCount = 0;
    int textCount   = 0;
    int imageCount  = 0;

    bool mixed() const
    {
        int types = 0;
        if (vectorCount) ++types;
        if (imageCount)  ++types;
        if (textCount)   ++types;
        return types > 1;
    }

    bool onlyVector() const { return total > 0 && vectorCount == total; }
    bool onlyImage()  const { return total > 0 && imageCount  == total; }
    bool onlyText()   const { return total > 0 && textCount   == total; }

    bool multi() const { return total > 1; }
};

#endif //SELECTIONSUMMARY_H
