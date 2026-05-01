#include "mainwindow.h"

#include <QApplication>
#include "starextractor.h"
#include "qimagefits.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    StarExtractor se;
    QImageFits myImg;
    se.loadFits("/Users/tim/Dev/Qt/StarSummary2/fake_image/fake_stars.fits");
    //se.loadFits("/Users/tim/Dev/Astro/Image/M_66/lights/M_66_Light_23.fits");
    se.extractBackground();
    se.extractStars();
    se.listStars();



    //se.processFits("/Users/tim/Dev/Astro/Image/M_66/lights/M_66_Light_23.fits");
    return 0;
}
