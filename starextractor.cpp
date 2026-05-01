#include "starextractor.h"

StarExtractor::StarExtractor(QObject *parent)
{
    setBackground_box(64);
    setBackground_filter(3);
    setBackgroup_filter_threshold(0.0);
    setMin_area_pixels(5);
    setDetect_threshold(0.0);
    setDeblend_threshold(32);
    setDeblend_cont(0.005);
    setClean_flag(1);
    setClean_param(1.0);
    star_catalog = nullptr;
}

int StarExtractor::set_param(eScope scope, eCcd ccd, eWx wx)
/*
 * Here I can try and set/adjust some of the parameters that match the
 *
 *
 * */

{
    switch (ccd)
    {
    case eCcd::SV905: {

    }
    case eCcd::SV405: {

    }
    default:
    {
        qWarning() << "Internal Error No matching CCD";
        return 1;
    }

    }
}

int StarExtractor::loadFits(QString filePath)
{
    fitsfile *fptr;
    int status = 0;

    if (fits_open_file(&fptr, filePath.toUtf8().constData(), READONLY, &status)) {
        fits_report_error(stderr, status);
        return 1;
    }

    int bitpix, naxis;
    long naxes[2] = {0, 0};
    fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status);
    setWidth(naxes[1]);
    setHeight(naxes[0]);

    qInfo() << "File has a width of " << getWidth() << " and height of " << getHeight();
    long npixels = getWidth() * getHeight();


    int anynul;
    QVector<float> tmp_img_data(npixels);
    fits_read_img(fptr, TFLOAT, 1, npixels, nullptr, tmp_img_data.data(), &anynul,
                  &status);
    setImageData(tmp_img_data);
    fits_close_file(fptr, &status);

    return 0;   // All ok
}

int StarExtractor::extractBackground()
{
    /*
     * This copies the original data into the image_less_background
     * And then performs a destructive operation on the image_less_background data.
     * The Original image should NOT be effected
     * */
    sep_image im;
    // Zero out the struct so optional pointers (noise, mask, etc.) default to
    // null   
    std::memset(&im, 0, sizeof(sep_image));
    setImage_less_background(getImageData());    // Put the Original image into the BackgroupData object
    // --- NEW: Construct the sep_image struct ---

    im.dtype = SEP_TFLOAT;
    im.w=getWidth();
    im.h=getHeight();
    im.data = image_less_background.data();


    // a. Background estimation using sep_image
    sep_bkg *bkg = nullptr;
    int sep_status =
        sep_background(&im,    // Pass the address of our configured sep_image
                       getBackground_box(), getBackground_box(), // Background box width and height
                       getBackground_filter(), getBackground_filter(),   // Filter width and height
                       getBackgroup_filter_threshold(),    // Filter threshold
                       &bkg);

    if (sep_status != 0) {
        qDebug() << "SEP background failed:" << sep_status;
        return 1;
    }

    // Subtract background globally (modifies the data inside our QVector)
    sep_status=sep_bkg_subarray(bkg, image_less_background.data(), SEP_TFLOAT);
    // At this point the image_less_background should be original_imageData-background
    // So if we have "adjusted"/calculated the background - we can now just operate on that data for star extraction


    if (image_less_background==original_imageData)
        qWarning()<<"background and original are still the same.";
    else
        qInfo()<<"Background and originals are now different. This is expected";
    setBkgrms(bkg->globalrms);



    if (sep_status)
    {
        qWarning() <<"Background extaction has an issue. status "<<sep_status;
        return sep_status;
    }
    else
    {
        qInfo() << "Background Extraction successfull";
    return 0;
    }
}

int StarExtractor::extractStars()
{
    stars.clear();      //Remove any previus stars

    // Extract sources using sep_image



    // Parameters:
    // 1.  im: the sep_image struct
    // 2.  thresh: detection threshold
    // 3.  minarea: min pixels for a source
    // 4.  filter: convolution filter (nullptr for none)
    // 5.  filter_type: 0 for flat, 1 for matched
    // 6.  deblend_nthresh: deblending thresholds
    // 7.  deblend_cont: deblending contrast
    // 8.  clean_flag: perform cleaning (1 = yes)
    // 9.  clean_param: cleaning parameter
    // 10. noise_type: SEP_NOISE_STDDEV or SEP_NOISE_VAR (0 if using internal RMS)
    // 11. noise: pointer to noise/variance array (nullptr to use globalrms from
    // bkg)
    // 12. gain: for Poisson noise (0.0 if not used)
    // 13. segmap: pointer to int array for segmentation map (nullptr if not
    // needed) Note: sep_extract also updated its signature in newer versions
    star_catalog = nullptr;
    qInfo() << "Extract Stars";

    int filter_type=SEP_FILTER_MATCHED;   // Matched as we are not using CONV
    sep_image im;
    // Zero out the struct so optional pointers (noise, mask, etc.) default to
    // null
    std::memset(&im, 0, sizeof(sep_image));
    im.dtype = SEP_TFLOAT;
    im.w=getWidth();
    im.h=getHeight();
    im.data = image_less_background.data();
    im.noise_type = SEP_NOISE_STDDEV;
    im.noiseval= 4.0;
    int sep_status = sep_extract(
        &im,               // 1
        getDetect_threshold(),
        SEP_THRESH_REL,   // Check Units of Standard Deviation
        getMin_area_pixels(),                 // 3
        nullptr,           // 4 conv_array
        0,0,        //Conv height and width
        filter_type,
        getDeblend_threshold(),
        getDeblend_cont(),
        clean_flag,
        clean_param,
        &star_catalog
        );
    qInfo() << "sep_extract produced "<<star_catalog->nobj << " objects.";

    if (sep_status == 0 && star_catalog != nullptr) {
        int limit = qMin(star_catalog->nobj, 10);

        double frac[1] = {0.5};
        double r[1];
        short flag[1];

        double fluxtot[1];
        //fluxtot[0] = star_catalog->objects[i].flux;

        for (int i = 0; i < limit; ++i) {
#ifdef DEBUG
            qDebug().nospace() << "Star " << i + 1 << ": "
                                << "X=" << star_catalog->x[i] << ", "
                                << "Y=" << star_catalog->y[i] << ", "
                                << "Flux=" << star_catalog->flux[i];
#endif
            fluxtot[0] = star_catalog->flux[i];
            sep_status = sep_flux_radius(
                &im,
                star_catalog->x[i],
                star_catalog->y[i],
                10.0,        // rmax
                i,           // object id
                5,           // subpix sampling
                0,           // inflag
                fluxtot,
                frac,
                1,
                r,
                flag
                );
            //r[0] is the output using 0.5 i.e. HFR
            Star_Summary s={star_catalog->x[i],
                star_catalog->y[i],
                              i,
                              star_catalog->flux[i],r[0] };
            stars.append(s);

        }
    }
    else
    {
        char msg_text[250];
        sep_get_errmsg(sep_status, msg_text);
        qWarning() << "Sep_Extract returned: " << sep_status << " : "<<msg_text;
        return sep_status;
    }


    return sep_status;
}

int StarExtractor::listStars()
{
    /*
     * This assumes we have read Fits, calculated background, extracted the stars
     * If not... there will be nothing here.
     */
    qInfo()<<"List Stars";
    qInfo()<<"=======================================================";
    if (stars.count()>0)
    {
        // Read-only traversal
        QVector<Star_Summary>::const_iterator i;
        for (i = stars.constBegin(); i != stars.constEnd(); ++i) {
            qInfo() << "id:"<<i->id<<" x:"<<i->x<<"y:"<<i->y<<" hft:"<<i->hfr<<" flux:"<<i->flux;
        }
    }
    else
    {
        qWarning() << "There are no stars in the summary data";
        return(1);
    }
    return(0);
}

void StarExtractor::processFits(QString filePath) {
    fitsfile *fptr;
    int status = 0;

    if (fits_open_file(&fptr, filePath.toUtf8().constData(), READONLY, &status)) {
        fits_report_error(stderr, status);
        return;
    }

    int bitpix, naxis;
    long naxes[2] = {0, 0};
    fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status);
    setWidth(naxes[1]);
    setHeight(naxes[0]);

    qInfo() << "File has a width of " << getWidth() << " and height of " << getHeight();
    setImage_size(getWidth() * getHeight());
    QVector<float> imageData(getImage_size());
    int anynul;

    fits_read_img(fptr, TFLOAT, 1, getImage_size(), nullptr, imageData.data(), &anynul,
                  &status);
    fits_close_file(fptr, &status);

    if (status)
        return;

    // --- NEW: Construct the sep_image struct ---
    int dims[2] = {static_cast<int>(getWidth()), static_cast<int>(getHeight())};

    sep_image im;
    // Zero out the struct so optional pointers (noise, mask, etc.) default to
    // null
    std::memset(&im, 0, sizeof(sep_image));

    im.data = imageData.data();
    im.dtype = SEP_TFLOAT;

    // im.ndim = 2;
    // im.dims = dims;

    // -------------------------------------------

    // a. Background estimation using sep_image
    sep_bkg *bkg = nullptr;
    int sep_status =
        sep_background(&im,    // Pass the address of our configured sep_image
                                    getBackground_box(), getBackground_box(), // Background box width and height
                                    getBackground_filter(), getBackground_filter(),   // Filter width and height
                                    getBackgroup_filter_threshold(),    // Filter threshold
                                    &bkg);

    if (sep_status != 0) {
        qDebug() << "SEP background failed:" << sep_status;
        return;
    }

    float bkgrms = bkg->globalrms;

    // Subtract background globally (modifies the data inside our QVector)
    sep_bkg_subarray(bkg, imageData.data(), SEP_TFLOAT);

    // b. Extract sources using sep_image
    sep_catalog *catalog = nullptr;


    // Parameters:
    // 1.  im: the sep_image struct
    // 2.  thresh: detection threshold
    // 3.  minarea: min pixels for a source
    // 4.  filter: convolution filter (nullptr for none)
    // 5.  filter_type: 0 for flat, 1 for matched
    // 6.  deblend_nthresh: deblending thresholds
    // 7.  deblend_cont: deblending contrast
    // 8.  clean_flag: perform cleaning (1 = yes)
    // 9.  clean_param: cleaning parameter
    // 10. noise_type: SEP_NOISE_STDDEV or SEP_NOISE_VAR (0 if using internal RMS)
    // 11. noise: pointer to noise/variance array (nullptr to use globalrms from
    // bkg)
    // 12. gain: for Poisson noise (0.0 if not used)
    // 13. segmap: pointer to int array for segmentation map (nullptr if not
    // needed) Note: sep_extract also updated its signature in newer versions

    double detect_threshold = 3.0;
    int filter_type=SEP_FILTER_MATCHED;   // Matched as we are not using CONV



    double clean_param=1.0;

    sep_status = sep_extract(
        &im,               // 1
        getDetect_threshold(),
        SEP_THRESH_REL,   // Check Units of Standard Deviation
        getMin_area_pixels(),                 // 3
        nullptr,           // 4 conv_array
        0,0,        //Conv height and width
        filter_type,
        getDeblend_threshold(),
        getDeblend_cont(),
        clean_flag,
        clean_param,
        &catalog
        );

    if (sep_status == 0 && catalog != nullptr) {
        int limit = qMin(catalog->nobj, 10);
        for (int i = 0; i < limit; ++i) {
            qDebug().nospace() << "Star " << i + 1 << ": "
                               << "X=" << catalog->x[i] << ", "
                               << "Y=" << catalog->y[i] << ", "
                               << "Flux=" << catalog->flux[i];
        }
    }

    if (catalog)
        sep_catalog_free(catalog);
    if (bkg)
        sep_bkg_free(bkg);
}

long StarExtractor::getWidth() const
{
    return fits_width;
}

void StarExtractor::setWidth(long newWidth)
{
    fits_width = newWidth;
}

long StarExtractor::getHeight() const
{
    return fits_height;
}

void StarExtractor::setHeight(long newHeight)
{
    fits_height = newHeight;
}

int StarExtractor::getBackground_box() const
{
    return background_box;
}

void StarExtractor::setBackground_box(int newBackground_box)
{
    background_box = newBackground_box;
}

int StarExtractor::getBackground_filter() const
{
    return background_filter;
}

void StarExtractor::setBackground_filter(int newBackground_filter)
{
    background_filter = newBackground_filter;
}

double StarExtractor::getBackgroup_filter_threshold() const
{
    return backgroup_filter_threshold;
}

void StarExtractor::setBackgroup_filter_threshold(double newBackgroup_filter_threshold)
{
    backgroup_filter_threshold = newBackgroup_filter_threshold;
}

int StarExtractor::getMin_area_pixels() const
{
    return min_area_pixels;
}

void StarExtractor::setMin_area_pixels(int newMin_area_pixels)
{
    min_area_pixels = newMin_area_pixels;
}

double StarExtractor::getDetect_threshold() const
{
    return detect_threshold;
}

void StarExtractor::setDetect_threshold(double newDetect_threshold)
{
    detect_threshold = newDetect_threshold;
}

int StarExtractor::getDeblend_threshold() const
{
    return deblend_threshold;
}

void StarExtractor::setDeblend_threshold(int newDeblend_threshold)
{
    deblend_threshold = newDeblend_threshold;
}

double StarExtractor::getDeblend_cont() const
{
    return deblend_cont;
}

void StarExtractor::setDeblend_cont(double newDeblend_cont)
{
    deblend_cont = newDeblend_cont;
}

int StarExtractor::getClean_flag() const
{
    return clean_flag;
}

void StarExtractor::setClean_flag(int newClean_flag)
{
    clean_flag = newClean_flag;
}

double StarExtractor::getClean_param() const
{
    return clean_param;
}

void StarExtractor::setClean_param(double newClean_param)
{
    clean_param = newClean_param;
}

QVector<float> StarExtractor::getImageData() const
{
    return original_imageData;
}

void StarExtractor::setImageData(const QVector<float> &newImageData)
{
    qInfo() << "Storing ImageData size " << newImageData.size();
    original_imageData = newImageData;
}

QVector<float> StarExtractor::getImage_less_background() const
{
    return image_less_background;
}

void StarExtractor::setImage_less_background(const QVector<float> &newImage_less_background)
{
    image_less_background = newImage_less_background;
}

float StarExtractor::getBkgrms() const
{
    return bkgrms;
}

void StarExtractor::setBkgrms(float newBkgrms)
{
    bkgrms = newBkgrms;
}

int StarExtractor::getImage_size() const
{
    return image_size;
}

void StarExtractor::setImage_size(int newImage_size)
{
    image_size = newImage_size;
}
