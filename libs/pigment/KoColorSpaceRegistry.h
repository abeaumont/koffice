/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef KOCOLORSPACEFACTORYREGISTRY_H
#define KOCOLORSPACEFACTORYREGISTRY_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>
#include <pigment_export.h>

#include <KoGenericRegistry.h>

class KisPaintDeviceAction;
class KoColorSpace;
class KoColorSpaceFactory;
class KoColorProfile;

/**
 *
 * XXX: USE STATIC DELETER! USE STATIC DELETER!
 *
 * The registry for colorspaces and profiles.
 * This class contains:
 *      - a registry of colorspace instantiated with specific profiles.
 *      - a registry of singleton colorspace factories.
 *      - a registry of icc profiles
 */
class PIGMENT_EXPORT KoColorSpaceRegistry : public QObject,  public KoGenericRegistry<KoColorSpaceFactory *> {


    Q_OBJECT

public:

    /**
     * Return an instance of the KoColorSpaceRegistry
     * Creates an instance if that has never happened before and returns the singleton instance.
     */
    static KoColorSpaceRegistry * instance();

    virtual ~KoColorSpaceRegistry();

public:

    /**
     * register the profile with the color space registry
     * @param profile the new profile to be registered so it can be combined with
     *  colorspaces.
     */
    void addProfile(KoColorProfile * profile);

    /**
     * Return a profile by its given name, or 0 if none registered.
     * @return a profile by its given name, or 0 if none registered.
     * @param name the product name as set on the profile.
     * @see addProfile()
     * @see KoColorProfile::productName()
     */
    KoColorProfile *  profileByName(const QString & name) const ;

    /**
     * Return the list of profiles for the argument colorspacefactory.
     * Profiles will not work with any color space, you can query which profiles
     * that are registered with this registry can be used in combination with the
     * argument factory.
     * @param factory the factory with which all the returned profiles will work.
     * @return a list of profiles for the factory
     */
    QList<KoColorProfile *>  profilesFor(KoColorSpaceFactory * factory);

    /**
     * Return the list of profiles for a colorspace with the argument id.
     * Profiles will not work with any color space, you can query which profiles
     * that are registered with this registry can be used in combination with the
     * argument factory.
     * @param id the colorspace-id with which all the returned profiles will work.
     * @return a list of profiles for the factory
     */
    QList<KoColorProfile *>  profilesFor(KoID id) {
        return profilesFor(id.id());
    }

    /**
     * Return the list of profiles for a colorspace with the argument id.
     * Profiles will not work with any color space, you can query which profiles
     * that are registered with this registry can be used in combination with the
     * argument factory.
     * @param colorSpaceId the colorspace-id with which all the returned profiles will work.
     * @return a list of profiles for the factory
     */
    QList<KoColorProfile *>  profilesFor(const QString &colorSpaceId);

    /**
     * Return a colorspace that works with the parameter profile.
     * @param csID the ID of the colorspace that you want to have returned
     * @param profileName the name of the KoColorProfile to be combined with the colorspace
     * @return the wanted colorspace, or 0 when the cs and profile can not be combined.
     */
    KoColorSpace *  colorSpace(const KoID &csID, const QString & profileName) {
        return colorSpace(csID.id(), profileName);
    }

    /**
     * Return a colorspace that works with the parameter profile.
     * @param colorSpaceId the ID string of the colorspace that you want to have returned
     * @param profile the profile be combined with the colorspace
     * @return the wanted colorspace, or 0 when the cs and profile can not be combined.
     */
    KoColorSpace * colorSpace(const QString &colorSpaceId, const KoColorProfile *profile);

    /**
     * Return a colorspace that works with the parameter profile.
     * @param profileName the name of the KoColorProfile to be combined with the colorspace
     * @return the wanted colorspace, or 0 when the cs and profile can not be combined.
     */
    KoColorSpace * colorSpace(const QString &colorSpaceId, const QString &profileName);

    /**
     * Convenience method to get the often used alpha colorspace
     */
    KoColorSpace * alpha8();

    /**
     * Convenience method to get an RGB 8bit colorspace with the default lcms profile
     */
    KoColorSpace * rgb8();
    
    /**
     * Convenience method to get an LAB 16bit colorspace with the default lcms profile
     */
    KoColorSpace * lab16();

    /**
     * Convenience method to get an RGB 16bit colorspace with the default lcms profile
     */
    KoColorSpace * rgb16();

    /**
     * add a KisConstructPaintDeviceAction to the registry for a colorspace
     */
    void addPaintDeviceAction(KoColorSpace* cs, KisPaintDeviceAction* action);

    /**
     * Get a list of KisConstructPaintDeviceAction for a colorspace
     */
    QList<KisPaintDeviceAction *> paintDeviceActionsFor(KoColorSpace* cs);

private:
    KoColorSpaceRegistry();
    KoColorSpaceRegistry(const KoColorSpaceRegistry&);
    KoColorSpaceRegistry operator=(const KoColorSpaceRegistry&);
    void init();

private:
    struct Private;
    Private * const d;
};

#endif // KOCOLORSPACEFACTORYREGISTRY_H

