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


struct Star_Summary{
    double x;
    double y;
    int id;
    float flux;
    double hfr;
};
enum eScope {SV555,GSO6};
enum eCcd  {SV405,SV905};
enum eWx {Clear,Ok,LightCloud,Hope};


class StarExtractor : public QObject{
    Q_OBJECT
public:
    explicit StarExtractor(QObject *parent = nullptr);
    int set_param(eScope scope, eCcd ccd, eWx wx);
    int loadFits(QString filePath);
    int extractBackground();
    int extractStars();
    int listStars();


    void processFits(QString filePath);

    long getWidth() const;
    void setWidth(long newWidth);

    long getHeight() const;
    void setHeight(long newHeight);

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

    QVector<float> getImage_less_background() const;
    void setImage_less_background(const QVector<float> &newImage_less_background);

    float getBkgrms() const;
    void setBkgrms(float newBkgrms);

    int getImage_size() const;
    void setImage_size(int newImage_size);
    QVector<Star_Summary> getStar_Summary(){return stars;}
    QVector<float> getOrigFits(){return original_imageData;}
    QVector<float> getOrigLessBackground(){return image_less_background;}


private:

    QVector<Star_Summary> stars;
    long fits_width;
    long fits_height;
    int background_box;   //default is 64 (height and width)
    int background_filter; // default 3 (height and width)
    double backgroup_filter_threshold; // default 0
    int deblend_threshold; //default is 32;
    double deblend_cont;//default is 0.005;
    int clean_flag;     //default 1;
    double clean_param;//default is 1.0;
    int min_area_pixels;//default is 5;
    double detect_threshold;//default is 0.0;
    int filter_type=SEP_FILTER_MATCHED;     // Matched as we are not using CONV
    float bkgrms;                           // Only updated when the background is extracted
    int image_size;                         // Set when Original image is loaded
    QVector<float> original_imageData;
    QVector<float> image_less_background;   // Take original_image and remove the calculated background
    sep_catalog *star_catalog;
    // HFR Parameters
    double max_star_radius=15.0;
    int sub_pix_sampling=5;
};

#endif // STAREXTRACTOR_H
