//KoHyphenator test, Lukas Tinkl <lukas.tinkl@suse.cz>

#include <stdio.h>
#include <stdlib.h>

#include <QStringList>
#include <QApplication>


#include "kohyphen.h"
#include <kdebug.h>

static bool check(const QString& a, const QString& b)
{
  if (a == b) {
    kDebug(32500) <<"checking '" << a <<"' against expected value '" << b <<"'..." <<"ok";
  }
  else {
    kDebug(32500) <<"checking '" << a <<"' against expected value '" << b <<"'..." <<"KO !";
    exit(1);
  }
  return true;
}

KoHyphenator * hypher = 0L;

void check_hyphenation( const QStringList& tests, const QStringList& results, const char* lang )
{
    QStringList::ConstIterator it, itres;
    for ( it = tests.begin(), itres = results.begin(); it != tests.end() ; ++it, ++itres ) {
        QString result = hypher->hyphenate((*it), lang);
        kDebug(32500) << (*it) <<" hyphenates like this:" << result;
        check( result.replace(QChar(0xad),'-'), *itres );
    }
}

int main (int argc, char ** argv)
{
    QApplication app(argc, argv);

    try {
        hypher = KoHyphenator::self();
    }
    catch (KoHyphenatorException &e)
    {
        kDebug(32500) << e.message();
        return 1;
    }

    QStringList::ConstIterator it, itres;

    //testing Czech language, this text is in UTF-8!
    QStringList cs_tests = QStringList() << "Žluťoučký" << "kůň" << "úpěl" <<
                        "ďábelské" << "ódy";

    for ( it = cs_tests.begin(); it != cs_tests.end() ; ++it )
        kDebug(32500) << (*it) <<" hyphenates like this:" << hypher->hyphenate((*it),"cs");

    //testing English
    QStringList en_tests = QStringList() << "Follow" << "white" << "rabbit";
    QStringList en_results = QStringList() << "Fol-low" << "white" << "rab-bit";
    check_hyphenation( en_tests, en_results, "en" );

    QStringList fr_tests = QStringList() << "constitution" ;
    QStringList fr_results = QStringList() << "consti-tu-tion" ;
    check_hyphenation( fr_tests, fr_results, "fr" );

    return 0;
}
