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
    QImage   convertFitsToGrayscale(const QVector<float>& fitsData, int width, int height);
    void displayFitsImage(QLabel* displayWidget, const QImage& sourceImage);
    QImage createStarMask(const QVector<Star_Summary>& stars, int width, int height);


    QImage getGrayscale() const;
    void setGrayscale(const QImage &newGrayscale);

    QImage getStarmask() const;
    void setStarmask(const QImage &newStarmask);
    QImage merge(QString SaveAs="");
private:
    QImage grayscale; // This is the FIts file converted (badly) to Grayscale
    QImage starmask;  // This is a Transparent layer which has used the Star_Summary data (from sep_extract) to show where I think stars are.

signals:
};

#endif // QIMAGEFITS_H
