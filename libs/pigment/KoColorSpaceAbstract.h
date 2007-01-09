/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOCOLORSPACEABSTRACT_H
#define KOCOLORSPACEABSTRACT_H

#include <klocale.h>

#include <KoColorSpace.h>
#include <KoColorSpaceMaths.h>
#include <KoIntegerMaths.h>
#include "KoCompositeOp.h"
#include "KoColorTransformation.h"

namespace {

    class CompositeCopy : public KoCompositeOp {

        public:

            explicit CompositeCopy(KoColorSpace * cs)
            : KoCompositeOp(cs, COMPOSITE_OVER, i18n("Copy" ) )
            {
            }

        public:

            void composite(quint8 *dstRowStart,
                           qint32 dstRowStride,
                           const quint8 *srcRowStart,
                           qint32 srcRowStride,
                           const quint8 *maskRowStart,
                           qint32 maskRowStride,
                           qint32 rows,
                           qint32 numColumns,
                           quint8 opacity,
                           const QBitArray & channelFlags) const
            {

                Q_UNUSED( maskRowStart );
                Q_UNUSED( maskRowStride );
                Q_UNUSED( channelFlags );
                quint8 *dst = dstRowStart;
                const quint8 *src = srcRowStart;
                qint32 bytesPerPixel = m_colorSpace->pixelSize();

                while (rows > 0) {
                    memcpy(dst, src, numColumns * bytesPerPixel);

                    if (opacity != OPACITY_OPAQUE) {
                        m_colorSpace->multiplyAlpha(dst, opacity, numColumns);
                    }

                    dst += dstRowStride;
                    src += srcRowStride;
                    --rows;
                }
            }
    };


    template<class _CSTraits>
    class KoMixColorsOpImpl : public KoMixColorsOp {
        public:
            KoMixColorsOpImpl() { }
            virtual ~KoMixColorsOpImpl() { }
            virtual void mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const
            {
                // Create and initialize to 0 the array of totals
                typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype totals[_CSTraits::channels_nb];
                typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype totalAlpha = 0;
                memset(totals, 0, sizeof(typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype) * _CSTraits::channels_nb);
                // Compute the total for each channel by summing each colors multiplied by the weight
                while(nColors--)
                {
                    const typename _CSTraits::channels_type* color = _CSTraits::nativeArray(*colors);
                    quint8 alphaTimesWeight =  KoColorSpaceMaths<quint8>::multiply(_CSTraits::alpha(*colors), *weights);
                    for(uint i = 0; i < _CSTraits::channels_nb; i++)
                    {
                        totals[i] += color[i] * alphaTimesWeight;
                    }
                    totalAlpha += alphaTimesWeight;
                    colors++;
                    weights++;
                }
                // set totalAlpha to the minimum between its value and the maximum value of the channels
                if (totalAlpha > KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::max()) {
                    totalAlpha = KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::max();
                }
                typename _CSTraits::channels_type* dstColor = _CSTraits::nativeArray(dst);
                if (totalAlpha > 0) {
                    for(uint i = 0; i < _CSTraits::channels_nb; i++)
                    {
                        typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype v = totals[i] / totalAlpha;
                        if(v > KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::max()) {
                            v = KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::max();
                        }
                        dstColor[ i ] = v;
                    }
                    dstColor[ _CSTraits::alpha_pos ] = totalAlpha;
                } else {
                    memset(dst, 0, sizeof(typename _CSTraits::channels_type) * _CSTraits::channels_nb);
                }
              
            }
    };
    
    template<class _CSTraits>
    class KoConvolutionOpImpl : public KoConvolutionOp{
        public:
            KoConvolutionOpImpl() { }
            virtual ~KoConvolutionOpImpl() { }
            virtual void convolveColors(quint8** colors, qint32* kernelValues, KoChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nPixels) const
            {
                // Create and initialize to 0 the array of totals
                typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype totals[_CSTraits::channels_nb];
                qint32 totalAlpha = 0;
                memset(totals, 0, sizeof(typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype) * _CSTraits::channels_nb);
                for (;nPixels--; colors++, kernelValues++)
                {
                    const typename _CSTraits::channels_type* color = _CSTraits::nativeArray(*colors);
                    quint8 alphaTimesWeight =  KoColorSpaceMaths<quint8>::multiply(_CSTraits::alpha(*colors), *kernelValues);
                    for(uint i = 0; i < _CSTraits::channels_nb; i++)
                    {
                        totals[i] += color[i] * alphaTimesWeight;
                    }
                    totalAlpha += alphaTimesWeight;
                }
                
                if (channelFlags & KoChannelInfo::FLAG_COLOR) {
                    typename _CSTraits::channels_type* dstColor = _CSTraits::nativeArray(dst);
                    for(uint i = 0; i < _CSTraits::channels_nb; i++)
                    {
                        typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype v = totals[i] / factor + offset;
                        dstColor[ i ] = CLAMP(v, KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::min(), KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::max());
                    }
    
                }
                if (channelFlags & KoChannelInfo::FLAG_ALPHA) {
                    _CSTraits::setAlpha(dst, CLAMP((totalAlpha/ factor) + offset, 0, 0xFF ),1);
                }
            }
    };

    class KoInvertColorTransformation : public KoColorTransformation {
        public:
            KoInvertColorTransformation(const KoColorSpace* cs) : m_colorSpace(cs)
            {
            }
            virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
            {
                quint16 rgba[4];
                quint32 psize = m_colorSpace->pixelSize();
                while(nPixels--)
                {
                    m_colorSpace->toRgbA16(src, reinterpret_cast<quint8 *>(rgba), 1);
                    rgba[0] = KoColorSpaceMathsTraits<quint16>::max() - rgba[0];
                    rgba[1] = KoColorSpaceMathsTraits<quint16>::max() - rgba[1];
                    rgba[2] = KoColorSpaceMathsTraits<quint16>::max() - rgba[2];
                    m_colorSpace->fromRgbA16(reinterpret_cast<quint8 *>(rgba), dst, 1);
                    src += psize;
                }
            }
        private:
            const KoColorSpace* m_colorSpace;
    };

}

/**
 * This in an implementation of KoColorSpace which can be used as a base for colorspaces with as many
 * different channels of the same type.
 * the template parameters must be a class wich inherits KoColorSpaceTrait (or a class with the same signature).
 * Where SOMETYPE is the type of the channel for instance (quint8, quint32...), SOMENBOFCHANNELS is the number of channels
 * including the alpha channel, SOMEALPHAPOS is the position of the alpha channel in the pixel (can be equal to -1 if no
 * alpha channel).
 */

template<class _CSTraits>
class KoColorSpaceAbstract : public KoColorSpace {
    public:
        KoColorSpaceAbstract(const QString &id, const QString &name, KoColorSpaceRegistry * parent, DWORD cmType,
                         icColorSpaceSignature colorSpaceSignature) : KoColorSpace(id, name, parent), m_cmType(cmType), m_colorSpaceSignature(colorSpaceSignature) {
            this->m_compositeOps.insert( COMPOSITE_COPY, new CompositeCopy( this ) );
            m_mixColorsOp = new KoMixColorsOpImpl< _CSTraits>();
            m_convolutionOp = new KoConvolutionOpImpl< _CSTraits>();
        };
        
        virtual void setColorSpaceType(quint32 type) { m_cmType = type; }
        virtual quint32 colorSpaceType() const { return m_cmType; }
        virtual icColorSpaceSignature colorSpaceSignature() const { return m_colorSpaceSignature; }
        
        virtual quint32 colorChannelCount() const { return _CSTraits::channels_nb - 1; }
        virtual quint32 channelCount() const { return _CSTraits::channels_nb; };
        virtual quint32 pixelSize() const { return _CSTraits::pixelSize(); }

        virtual QString channelValueText(const quint8 *pixel, quint32 channelIndex) const
        {
            if(channelIndex > _CSTraits::channels_nb) return QString("Error");
            typename _CSTraits::channels_type c = _CSTraits::nativeArray(pixel)[channelIndex];
            return QString().setNum(c);
        }

        virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const
        {
            if(channelIndex > _CSTraits::channels_nb) return QString("Error");
            typename _CSTraits::channels_type c = _CSTraits::nativeArray(pixel)[channelIndex];
            return QString().setNum( 100. * ((double)c ) / KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::max() );
        }
        
        virtual quint8 scaleToU8(const quint8 * srcPixel, qint32 channelIndex) const {
            typename _CSTraits::channels_type c = _CSTraits::nativeArray(srcPixel)[channelIndex];
            return KoColorSpaceMaths<typename _CSTraits::channels_type, quint8>::scaleToA(c);
        }

        virtual quint16 scaleToU16(const quint8 * srcPixel, qint32 channelIndex) const {
            typename _CSTraits::channels_type c = _CSTraits::nativeArray(srcPixel)[channelIndex];
            return KoColorSpaceMaths<typename _CSTraits::channels_type,quint16>::scaleToA(c);
        }
        virtual void singleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex) const
        {
            const typename _CSTraits::channels_type* src = _CSTraits::nativeArray(srcPixel);
            typename _CSTraits::channels_type* dst = _CSTraits::nativeArray(dstPixel);
            for(uint i = 0; i < _CSTraits::channels_nb;i++)
            {
                if( i != channelIndex )
                {
                    dst[i] = 0;
                } else {
                    dst[i] = src[i];
                }
            }
        }
        virtual quint8 alpha(const quint8 * U8_pixel) const
        {
            return _CSTraits::alpha(U8_pixel);
        }
        virtual void setAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const
        {
            _CSTraits::setAlpha(pixels, alpha, nPixels);
        }
        virtual void multiplyAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const
        {
            if (_CSTraits::alpha_pos < 0) return;

            qint32 psize = pixelSize();
            typename _CSTraits::channels_type valpha =  KoColorSpaceMaths<quint8,typename _CSTraits::channels_type>::scaleToA(alpha);

            for (; nPixels > 0; --nPixels, pixels += psize) {
                typename _CSTraits::channels_type* alphapixel = _CSTraits::nativeArray(pixels) + _CSTraits::alpha_pos;
                *alphapixel = KoColorSpaceMaths<typename _CSTraits::channels_type>::multiply( *alphapixel, valpha );
            }
        }

        virtual void applyAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const
        {
            if (_CSTraits::alpha_pos < 0) return;
            qint32 psize = pixelSize();

            for (; nPixels > 0; --nPixels, pixels += psize, ++alpha) {
                typename _CSTraits::channels_type valpha =  KoColorSpaceMaths<quint8,typename _CSTraits::channels_type>::scaleToA(*alpha);
                typename _CSTraits::channels_type* alphapixel = _CSTraits::nativeArray(pixels) + _CSTraits::alpha_pos;
                *alphapixel = KoColorSpaceMaths<typename _CSTraits::channels_type>::multiply( *alphapixel, valpha );
            }
        }
        
        virtual void applyInverseAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const
        {
            if (_CSTraits::alpha_pos < 0) return;
            qint32 psize = pixelSize();

            for (; nPixels > 0; --nPixels, pixels += psize, ++alpha) {
                typename _CSTraits::channels_type valpha =  KoColorSpaceMaths<quint8,typename _CSTraits::channels_type>::scaleToA(OPACITY_OPAQUE - *alpha);
                typename _CSTraits::channels_type* alphapixel = _CSTraits::nativeArray(pixels) + _CSTraits::alpha_pos;
                *alphapixel = KoColorSpaceMaths<typename _CSTraits::channels_type>::multiply( *alphapixel, valpha );
            }
        }
        
        virtual quint8 intensity8(const quint8 * src) const
        {
            QColor c;
            quint8 opacity;
            const_cast<KoColorSpaceAbstract<_CSTraits> *>(this)->toQColor(src, &c, &opacity);
            return static_cast<quint8>((c.red() * 0.30 + c.green() * 0.59 + c.blue() * 0.11) + 0.5);
        }

        virtual KoInvertColorTransformation* createInvertTransformation() const
        {
            return new KoInvertColorTransformation(this);
        }

        virtual KoID mathToolboxId() const
        {
            return KoID("Basic");
        }

    private:
        DWORD m_cmType;                           // The colorspace type as defined by littlecms
        icColorSpaceSignature m_colorSpaceSignature; // The colorspace signature as defined in icm/icc files

};


#endif
