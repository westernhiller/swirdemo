#include "global.h"
#include <QTime>
#include <QDebug>
#include <opencv2/xphoto.hpp>

void msleep(unsigned int msec)
{
    QTime dueTime = QTime::currentTime().addMSecs(msec);
    while(QTime::currentTime() < dueTime);
}

QImage mat2qimage(const Mat &mat)
{
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);          // Set the color table (used to translate colour indexes to qRgb values)          image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }      // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }else if(mat.type() == CV_8UC4)
    {
        qDebug() << "CV_8UC4";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

Mat qimage2mat(const QImage &qimage)
{
    Mat mat, mat2;
    int from_to[] = {0, 0, 1, 1, 2, 2, 3, 3};
    switch(qimage.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = Mat(qimage.height(), qimage.width(), CV_8UC4, (uchar *)qimage.constBits(), qimage.bytesPerLine());
        mat2 = Mat(mat.rows, mat.cols, CV_8UC4);
        mixChannels(&mat, 1, &mat2, 1, from_to, 4);
        return mat2;
        break;
    case QImage::Format_RGB888:
        mat = Mat(qimage.height(), qimage.width(), CV_8UC3, (uchar *)qimage.constBits(), qimage.bytesPerLine());
        mat2 = Mat(mat.rows, mat.cols, CV_8UC3);
        mixChannels(&mat, 1, &mat2, 1, from_to, 3);
        return mat2;
        break;
    case QImage::Format_Indexed8:
        mat = Mat(qimage.height(), qimage.width(), CV_8UC1, (uchar *)qimage.constBits(), qimage.bytesPerLine());
        break;
    default:
        break;
    }
    return mat;
}

Mat autoWhiteBalance(const Mat& imageSource)
{
/*    Mat imageResult;
    vector<Mat> imageRGB;
    //RGB三通道分离
    split(imageSource, imageRGB);
    //求原始图像的RGB分量的均值
    double R, G, B;
    B = mean(imageRGB[0])[0];
    G = mean(imageRGB[1])[0];
    R = mean(imageRGB[2])[0];
    //需要调整的RGB分量的增益

    double KR, KG, KB;
    KB = (R + G + B) / (3 * B);
    KG = (R + G + B) / (3 * G);
    KR = (R + G + B) / (3 * R);
    //调整RGB三个通道各自的值
    imageRGB[0] = imageRGB[0] * KB;
    imageRGB[1] = imageRGB[1] * KG;
    imageRGB[2] = imageRGB[2] * KR;
    //RGB三通道图像合并
    merge(imageRGB, imageResult);
*/
    Mat imageResult;
    Ptr<xphoto::WhiteBalancer> simpleWB = xphoto::createSimpleWB();
    simpleWB->balanceWhite(imageSource, imageResult);

    return imageResult;
}

