#ifndef STAREXTRACTOR_H
#define STAREXTRACTOR_H

#include <QObject>
#include <QCoreApplication>
#include <QString>
#include <QVector>
#include <QDebug>
#include <cstring> // For memset
// C Libraries
#include <fitsio.h>
#include <sep.h>

// class StarExtractor : public QObject
// {
//     Q_OBJECT
// public:
//     explicit StarExtractor(QObject *parent = nullptr);

// signals:
// };





class StarExtractor : public QObject{
    Q_OBJECT
public:
    explicit StarExtractor(QObject *parent = nullptr) {};
    void processFits(QString filePath);

// Example Usage:
// int main(int argc, char *argv[]) {
//     QCoreApplication a(argc, argv);
//     StarExtractor::processFits("/path/to/your/image.fits");
//     return 0; // Return immediately for testing
// }
};

#endif // STAREXTRACTOR_H
