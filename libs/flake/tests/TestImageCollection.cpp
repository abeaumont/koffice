#include "TestImageCollection.h"

#include <KoImageData.h>
#include <KoImageCollection.h>
#include <KoStore.h>

#include <QImage>
#include <QPixmap>
#include <kurl.h>
#include <kdebug.h>

#include <qtest_kde.h>

void TestImageCollection::testGetImageImage()
{
    KoImageCollection collection;
    QImage image(KDESRCDIR "/logo-koffice.png");

    KoImageData id1 = collection.getImage(image);
    QCOMPARE(id1.hasCachedImage(), true);
    KoImageData id2 = collection.getImage(image);
    QCOMPARE(id2.hasCachedImage(), true);
    QCOMPARE(id1, id2);
    KoImageData id3 = collection.getImage(image);
    QCOMPARE(id3.hasCachedImage(), true);
    QCOMPARE(id1.key(), id3.key());
    QCOMPARE(id1, id3);
    QImage image2(KDESRCDIR "/logo-kpresenter.png");
    {
        KoImageData id4 = collection.getImage(image2);
        QCOMPARE(id4.hasCachedImage(), true);
        QVERIFY(id1.key() != id4.key());
        QCOMPARE(collection.count(), 2);
    } // id4 goes out of scope
    QCOMPARE(id1.hasCachedImage(), true);
    QCOMPARE(id2.hasCachedImage(), true);
    QCOMPARE(id3.hasCachedImage(), true);
    QCOMPARE(collection.count(), 1);
    id1 = KoImageData();
    QCOMPARE(id1.hasCachedImage(), false);
    QCOMPARE(id2.hasCachedImage(), true);
    QCOMPARE(id3.hasCachedImage(), true);
    QCOMPARE(collection.size(), 1);
    id2 = KoImageData();
    QCOMPARE(id1.hasCachedImage(), false);
    QCOMPARE(id2.hasCachedImage(), false);
    QCOMPARE(id3.hasCachedImage(), true);
    QCOMPARE(collection.size(), 1);
    id3 = KoImageData();
    QCOMPARE(id1.hasCachedImage(), false);
    QCOMPARE(id2.hasCachedImage(), false);
    QCOMPARE(id3.hasCachedImage(), false);
    QCOMPARE(collection.size(), 0);
}

void TestImageCollection::testGetImageKUrl()
{
    KoImageCollection collection;
    KUrl url(KDESRCDIR "/logo-koffice.png");
    KoImageData id1 = collection.getImage(url);
    QCOMPARE(id1.hasCachedImage(), false);
    KoImageData id2 = collection.getImage(url);
    QCOMPARE(id2.hasCachedImage(), false);
    QCOMPARE(id1, id2);
    KoImageData id3 = collection.getImage(url);
    QCOMPARE(id1.key(), id3.key());
    QCOMPARE(id1, id3);
    KUrl url2(KDESRCDIR "/logo-kpresenter.png");
    {
        KoImageData id4 = collection.getImage(url2);
        QCOMPARE(id4.hasCachedImage(), false);
        QVERIFY(id1.key() != id4.key());
        QCOMPARE(collection.size(), 2);
    } // id4 goes out of scope
    QCOMPARE(collection.size(), 1);
}

void TestImageCollection::testGetImageStore()
{
    KoImageCollection collection;
    KoStore *store = KoStore::createStore(KDESRCDIR "/store.zip", KoStore::Read);
    QString image("logo-koffice.png");
    KoImageData id1 = collection.getImage(image, store);
    QCOMPARE(id1.hasCachedImage(), false);
    KoImageData id2 = collection.getImage(image, store);
    QCOMPARE(id2.hasCachedImage(), false);
    QCOMPARE(id1, id2);
    QCOMPARE(id1.key(), id2.key());
    QCOMPARE(collection.count(), 1);

    // opening the exact same file from either a KoStore or a "File://" url may
    // naively make you think it should have the same key, but thats not the case.
    // Opening a file from your local filesystem prioritizes over the url instead
    // of on the content since if the user updates the image on his filesystem we
    // want to data to be refreshed.  So the key is based on the url.
    // Opening a KoStore based file we only have the content, and we always have to
    // read the full content anyway due to the store being deleted soon after.
    // So the key is based on the image data.
    KoImageData id3 = collection.getImage(image);
    QCOMPARE(collection.count(), 2);
    QVERIFY(id1.key() != id3.key());
    QVERIFY(id1 != id3);
    QString image2("logo-kpresenter.png");
    {
        KoImageData id4 = collection.getImage(image2);
        QCOMPARE(id4.hasCachedImage(), false);
        QVERIFY(id1.key() != id4.key());
        QCOMPARE(collection.count(), 3);
    } // id4 goes out of scope
    QCOMPARE(collection.count(), 2);
    delete store;
}

void TestImageCollection::testImageDataAsSharedData()
{
    KoImageData data;
    QCOMPARE(data.isValid(), false);

    QImage image(100, 101, QImage::Format_RGB32);
    data.setImage(image);
    QCOMPARE(data.isValid(), true);
    QCOMPARE(data.hasCachedImage(), true);
    QCOMPARE(data.image(), image);

    KoImageData data2(data);
    QCOMPARE(data, data2);
    QCOMPARE(data.isValid(), true);
    QCOMPARE(data.image(), image);
    QCOMPARE(data2.isValid(), true);
    QCOMPARE(data2.image(), image);
    {
        KoImageData data3;
        data3 = data;
        QCOMPARE(data3.isValid(), true);
        QCOMPARE(data3.image(), image);
    }
    QCOMPARE(data, data2);
    QCOMPARE(data.isValid(), true);
    QCOMPARE(data.image(), image);
    QCOMPARE(data2.isValid(), true);
    QCOMPARE(data2.image(), image);
}

void TestImageCollection::testPreload1()
{
    KoImageData data;
    QImage image(100, 102, QImage::Format_RGB32);
    data.setImage(image);
    QCOMPARE(data.hasCachedImage(), true);

    QCOMPARE(data.hasCachedPixmap(), false);
    QPixmap pixmap = data.pixmap(QSize(40, 41));
    QCOMPARE(pixmap.width(), 40);
    QCOMPARE(pixmap.height(), 41);
    QCOMPARE(data.hasCachedPixmap(), true);
    QPixmap pixmap2 = data.pixmap(QSize(40, 41));
    QCOMPARE(pixmap.cacheKey(), pixmap2.cacheKey());

    QPixmap pixmap3 = data.pixmap();
    QCOMPARE(pixmap.cacheKey(), pixmap3.cacheKey());
    QCOMPARE(data.hasCachedImage(), true);
}

void TestImageCollection::testPreload2()
{
    KoImageData data;
    KUrl url(KDESRCDIR "/logo-koffice.png");
    data.setImage(url);

    QCOMPARE(data.hasCachedImage(), false);
    QCOMPARE(data.hasCachedPixmap(), false);
    QPixmap pixmap = data.pixmap(QSize(40, 41));
    QCOMPARE(data.hasCachedImage(), true);
    QCOMPARE(pixmap.width(), 40);
    QCOMPARE(pixmap.height(), 41);
    QCOMPARE(data.hasCachedPixmap(), true);
    QCOMPARE(data.hasCachedImage(), true);
    QPixmap pixmap2 = data.pixmap(QSize(40, 41));
    QCOMPARE(pixmap.cacheKey(), pixmap2.cacheKey());

    QPixmap pixmap3 = data.pixmap();
    QCOMPARE(pixmap.cacheKey(), pixmap3.cacheKey());
}

void TestImageCollection::testPreload3()
{
    KoImageData data;
    KoStore *store = KoStore::createStore(KDESRCDIR "/store.zip", KoStore::Read);
    QString image("logo-koffice.png");
    data.setImage(image, store);

    QCOMPARE(data.hasCachedImage(), false);
    QCOMPARE(data.hasCachedPixmap(), false);
    QPixmap pixmap = data.pixmap(QSize(40, 41));
    QCOMPARE(pixmap.width(), 40);
    QCOMPARE(pixmap.height(), 41);
    QCOMPARE(data.hasCachedPixmap(), true);
    QCOMPARE(data.hasCachedImage(), true);
    QPixmap pixmap2 = data.pixmap(QSize(40, 41));
    QCOMPARE(pixmap.cacheKey(), pixmap2.cacheKey());

    QPixmap pixmap3 = data.pixmap();
    QCOMPARE(pixmap.cacheKey(), pixmap3.cacheKey());

    // now get a differen size;
    QPixmap pixmap4 = data.pixmap(QSize(10, 12));
    QCOMPARE(pixmap.width(), 40);
    QCOMPARE(pixmap.height(), 41);
    QCOMPARE(pixmap4.width(), 10);
    QCOMPARE(pixmap4.height(), 12);
    QVERIFY(pixmap.cacheKey() != pixmap4.cacheKey());

    QPixmap pixmap5 = data.pixmap();
    QCOMPARE(pixmap5.cacheKey(), pixmap4.cacheKey());
}

QTEST_KDEMAIN(TestImageCollection, GUI)
#include "TestImageCollection.moc"
