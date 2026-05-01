#ifndef QIMAGEFITS_H
#define QIMAGEFITS_H

#include <QObject>
#include <QLabel>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <vector>
#include <algorithm>
#include <cmath>
#include "starextractor.h"

class QImageFits : public QObject
{
    Q_OBJECT
public:
    explicit QImageFits(QObject *parent = nullptr);
    QImage   convertFitsToGrayscale(const std::vector<float>& fitsData, int width, int height);
    void displayFitsImage(QLabel* displayWidget, const QImage& sourceImage);
    QImage createStarMask(const QVector<Star_Summary>& stars, int width, int height);
signals:
};

#endif // QIMAGEFITS_H
