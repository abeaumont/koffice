
#include "koImage.h"

#include <qshared.h>

class KoImageCollection;


KoImage::KoImage()
{
    d = new KoImagePrivate;
    d->m_valid = false;
}

KoImage::KoImage( const QString &key, const QImage &image, const KURL &url )
{
    d = new KoImagePrivate;
    d->m_image = image.copy();
    d->m_url = url;
    d->m_key = key;
}

KoImage::~KoImage()
{
    if ( d->deref() )
        delete d;
}

KoImage &KoImage::operator=( const KoImage &_other )
{
    KoImage &other = const_cast<KoImage &>( _other );

    other.d->ref();

    if ( d->deref() )
        delete d;

    d = other.d;

    return *this;
}

KoImage KoImage::scaleImage( const QSize &size ) const
{
    KoImage originalImage;

    if ( d->m_originalImage.isValid() )
        originalImage = d->m_originalImage;
    else
        originalImage = *this;

    if ( originalImage.image().size() == size )
        return originalImage;

    QImage scaledImg = originalImage.image().smoothScale( size.width(), size.height() );

    KoImage result( d->m_key, scaledImg, d->m_url );
    result.d->m_originalImage = originalImage;

    return result;
}

