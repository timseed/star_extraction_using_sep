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

// Example Usage:
// int main(int argc, char *argv[]) {
//     QCoreApplication a(argc, argv);
//     StarExtractor::processFits("/path/to/your/image.fits");
//     return 0; // Return immediately for testing
// }



class StarExtractor : public QObject{
    Q_OBJECT
public:
    explicit StarExtractor(QObject *parent = nullptr);
    int loadFits(QString filePath);



    void processFits(QString filePath);

    long getNx() const;
    void setNx(long newNx);

    long getNy() const;
    void setNy(long newNy);

    int getBackground_box() const;
    void setBackground_box(int newBackground_box);

    int getBackground_filter() const;
    void setBackground_filter(int newBackground_filter);

    double getBackgroup_filter_threshold() const;
    void setBackgroup_filter_threshold(double newBackgroup_filter_threshold);

    int getMin_area_pixels() const;
    void setMin_area_pixels(int newMin_area_pixels);

    double getDetect_threshold() const;
    void setDetect_threshold(double newDetect_threshold);

    int getDeblend_threshold() const;
    void setDeblend_threshold(int newDeblend_threshold);

    double getDeblend_cont() const;
    void setDeblend_cont(double newDeblend_cont);

    int getClean_flag() const;
    void setClean_flag(int newClean_flag);

    double getClean_param() const;
    void setClean_param(double newClean_param);

    QVector<float> getImageData() const;
    void setImageData(const QVector<float> &newImageData);

private:


    long nx;
    long ny;
    int background_box;   //default is 64 (height and width)
    int background_filter; // default 3 (heght and width)
    double backgroup_filter_threshold; // default 0
    int deblend_threshold; //default is 32;
    double deblend_cont;//default is 0.005;
    int clean_flag;     //default 1;
    double clean_param;//default is 1.0;
    int min_area_pixels;//default is 5;
    double detect_threshold;//default is 0.0;
    int filter_type=SEP_FILTER_MATCHED;   // Matched as we are not using CONV
    QVector<float> imageData;


};

#endif // STAREXTRACTOR_H
