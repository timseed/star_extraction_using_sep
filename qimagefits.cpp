#include "qimagefits.h"

QImageFits::QImageFits(QObject *parent)
    : QObject{parent}
{}



QImage QImageFits::convertFitsToGrayscale(const std::vector<float>& fitsData, int width, int height)
/*
 * This is a VERY basic way of displaying the data
 * The code above applies a simple Linear Stretch.
 * Because astronomical images often contain very bright stars and very faint nebulosity/background signals,
 * a linear stretch might leave the background looking entirely black. For better visual representation of deep-sky objects,
 * you may eventually want to replace the linear normalization step (rawValue - minVal) / range with a non-linear stretch,
 * such as an Asinh (inverse hyperbolic sine) or Logarithmic function.
 *
 * */
{
    // 1. Validate inputs
    if (fitsData.empty() || width <= 0 || height <= 0) {
        return QImage();
    }

    // 2. Create an 8-bit grayscale QImage
    // Format_Grayscale8 is highly efficient for monochrome astronomical data
    QImage image(width, height, QImage::Format_Grayscale8);

    // 3. Find Minimum and Maximum values for normalization (Linear Stretch)
    auto [minIt, maxIt] = std::minmax_element(fitsData.begin(), fitsData.end());
    float minVal = *minIt;
    float maxVal = *maxIt;
    float range = maxVal - minVal;

    // Prevent division by zero on flat images
    if (range == 0.0f) {
        range = 1.0f;
    }

    // 4. Map FITS data to 0-255 using fast row-by-row memory access
    for (int y = 0; y < height; ++y) {
        // scanLine provides a pointer to the start of the pixel data for row 'y'
        uchar* rowPointer = image.scanLine(y);

        for (int x = 0; x < width; ++x) {
            float rawValue = fitsData[y * width + x];

            // Linear stretch
            float normalized = (rawValue - minVal) / range;
            int scaledValue = static_cast<int>(normalized * 255.0f);

            // Clamp to ensure we stay within 8-bit bounds
            rowPointer[x] = static_cast<uchar>(std::clamp(scaledValue, 0, 255));
        }
    }

    return image;
}



void QImageFits::displayFitsImage(QLabel* displayWidget, const QImage& sourceImage)
/*
 * This is expecting to display the image on a QLabel object
 *
 */
{
    if (sourceImage.isNull()) return;

    // Get the current available size of the widget
    QSize widgetSize = displayWidget->size();

    // Scale the image.
    // Qt::KeepAspectRatio ensures stars remain round and the sensor ratio is maintained.
    // Qt::SmoothTransformation applies bilinear filtering (better quality, slightly slower).
    QImage scaledImage = sourceImage.scaled(
        widgetSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        );

    // Set the pixmap on the label
    displayWidget->setPixmap(QPixmap::fromImage(scaledImage));

    // Optional: center the image in the label
    displayWidget->setAlignment(Qt::AlignCenter);
}


QImage QImageFits::createStarMask(const QVector<Star_Summary>& stars, int width, int height) {
    // 1. Create a 32-bit image to support color (RGB or ARGB)
    QImage mask(width, height, QImage::Format_ARGB32);
    mask.fill(Qt::transparent);

    // 2. Fill the image with solid black
    mask.fill(Qt::black);

    // 3. Initialize QPainter to draw on the mask
    QPainter painter(&mask);

    // Enable anti-aliasing so the ellipses are smooth, not jagged
    painter.setRenderHint(QPainter::Antialiasing);

    // 4. Set up the pen (outline) for the stars
    QPen redPen(Qt::red);
    redPen.setWidth(2); // 2 pixels wide so it's easily visible
    painter.setPen(redPen);

    // Optional: If you want the ellipses filled with a semi-transparent color
    // painter.setBrush(QColor(255, 0, 0, 50));

    // 5. Draw the stars
    for (const auto& star : stars) {
        // HFR is a radius. You might want to multiply it by a factor (e.g., 2.0 or 3.0)
        // so the ellipse fully encircles the visible star profile rather than cutting through it.
        double drawRadius = star.hfr * 1.5;

        painter.drawEllipse(QPointF(star.x, star.y), drawRadius, drawRadius);
    }

    // 6. End painting
    painter.end();

    return mask;
}
