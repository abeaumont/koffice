/* This file is part of the KDE project
   Copyright 2007 Sascha Pfau <MrPeacock@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "TestDatetimeFunctions.h"

#include "TestKspreadCommon.h"

void TestDatetimeFunctions::initTestCase()
{
    KCFunctionModuleRegistry::instance()->loadFunctionModules();
}

#define CHECK_EVAL(x,y) { KCValue z(RoundNumber(y)); QCOMPARE(evaluate(x,z), (z)); }

#define CHECK_FAIL(x,y,txt) { KCValue z(RoundNumber(y)); QEXPECT_FAIL("", txt, Continue); QCOMPARE(evaluate(x,z), (z));}
#define ROUND(x) (roundf(1e10 * x) / 1e10)

// changelog
/////////////////////////////////////
// 18.05.07
// - fix typo in yearfrac
// - indend
// - added missing tests EOMONTH()
// - added missing values in DATEDIF
// 02.06.07
// - added Isoweeknum tests starts on sunday
// - added WEEKINYEAR unittests
// - added ISLEAPYEAR unittests
// - added DAYSINMONTH unittests
// 15.07.07
// - modified YEARFRAC basis=1
// 30.10.07
// - fixed WEEKNUM tests
// - corrected wrong DAYS360,EDATE and EOMONTH unittests
// - commented out last issue on YEARFRAC

#if 0 // not used?
// round to get at most 10-digits number
static KCValue RoundNumber(double f)
{
    return KCValue(ROUND(f));
}
#endif

// round to get at most 10-digits number
static KCValue RoundNumber(const KCValue& v)
{
    if (v.isNumber()) {
        double d = numToDouble(v.asFloat());
        if (fabs(d) < DBL_EPSILON)
            d = 0.0;
        return KCValue(ROUND(d));
    } else
        return v;
}

KCValue TestDatetimeFunctions::evaluate(const QString& formula, KCValue& ex)
{
    KCFormula f;
    QString expr = formula;
    if (expr[0] != '=')
        expr.prepend('=');
    f.setExpression(expr);
    KCValue result = f.eval();

    if (result.isFloat() && ex.isInteger())
        ex = KCValue(ex.asFloat());
    if (result.isInteger() && ex.isFloat())
        result = KCValue(result.asFloat());

    return RoundNumber(result);
}

void TestDatetimeFunctions::testYEARFRAC()
{

    // basis 0 US
    CHECK_EVAL("YEARFRAC( \"1999-01-01\" ; \"1999-06-30\" ; 0)", KCValue(0.4972222222));
    CHECK_EVAL("YEARFRAC( \"1999-01-01\" ; \"1999-07-01\" ; 0)", KCValue(0.5000000000));
    CHECK_EVAL("YEARFRAC( \"2000-01-01\" ; \"2000-06-30\" ; 0)", KCValue(0.4972222222));
    CHECK_EVAL("YEARFRAC( \"2000-01-15\" ; \"2000-09-17\" ; 0)", KCValue(0.6722222222));
    CHECK_EVAL("YEARFRAC( \"2000-01-01\" ; \"2001-01-01\" ; 0)", KCValue(1.0000000000));
    CHECK_EVAL("YEARFRAC( \"2001-01-01\" ; \"2002-01-01\" ; 0)", KCValue(1.0000000000));
    CHECK_EVAL("YEARFRAC( \"2001-12-05\" ; \"2001-12-30\" ; 0)", KCValue(0.0694444444));
    CHECK_EVAL("YEARFRAC( \"2000-02-05\" ; \"2006-08-10\" ; 0)", KCValue(6.5138888889));

    // basis 1 Actual/actual
    // other values are taken from OOo-2.2.1
    CHECK_EVAL("YEARFRAC( \"1999-01-01\" ; \"1999-06-30\" ; 1)", KCValue(0.4931506849));
    CHECK_EVAL("YEARFRAC( \"1999-01-01\" ; \"1999-07-01\" ; 1)", KCValue(0.4958904110));
    CHECK_EVAL("YEARFRAC( \"2000-01-01\" ; \"2000-06-30\" ; 1)", KCValue(0.4945355191));
    CHECK_EVAL("YEARFRAC( \"2000-01-15\" ; \"2000-09-17\" ; 1)", KCValue(0.6721311475));
    CHECK_EVAL("YEARFRAC( \"2000-01-01\" ; \"2001-01-01\" ; 1)", KCValue(1.0000000000));
    CHECK_EVAL("YEARFRAC( \"2001-12-05\" ; \"2001-12-30\" ; 1)", KCValue(0.0684931507));
    CHECK_EVAL("YEARFRAC( \"2000-02-05\" ; \"2006-08-10\" ; 1)", KCValue(6.5099726242));     // specs 6.5099726242 OOo-2.3.0 6.5081967213
    CHECK_EVAL("YEARFRAC( \"2003-12-06\" ; \"2004-03-05\" ; 1)", KCValue(0.2459016393));
    CHECK_EVAL("YEARFRAC( \"2003-12-31\" ; \"2004-03-31\" ; 1)", KCValue(0.2486338798));
    CHECK_EVAL("YEARFRAC( \"2004-10-01\" ; \"2005-01-11\" ; 1)", KCValue(0.2794520548));
    CHECK_EVAL("YEARFRAC( \"2004-10-26\" ; \"2005-02-06\" ; 1)", KCValue(0.2821917808));
    CHECK_EVAL("YEARFRAC( \"2004-11-20\" ; \"2005-03-04\" ; 1)", KCValue(0.2849315068));
    CHECK_EVAL("YEARFRAC( \"2004-12-15\" ; \"2005-03-30\" ; 1)", KCValue(0.2876712329));
    CHECK_EVAL("YEARFRAC( \"2000-12-01\" ; \"2001-01-16\" ; 1)", KCValue(0.1260273973));
    CHECK_EVAL("YEARFRAC( \"2000-12-26\" ; \"2001-02-11\" ; 1)", KCValue(0.1287671233));

    // basis 2 Actual/360
    CHECK_EVAL("YEARFRAC( \"2000-01-01\" ; \"2000-06-30\" ; 2)", KCValue(0.5027777778));
    CHECK_EVAL("YEARFRAC( \"1999-01-01\" ; \"1999-06-30\" ; 2)", KCValue(0.5000000000));
    CHECK_EVAL("YEARFRAC( \"1999-01-01\" ; \"1999-07-01\" ; 2)", KCValue(0.5027777778));
    CHECK_EVAL("YEARFRAC( \"2000-01-15\" ; \"2000-09-17\" ; 2)", KCValue(0.6833333333));
    CHECK_EVAL("YEARFRAC( \"2000-01-01\" ; \"2001-01-01\" ; 2)", KCValue(1.0166666667));
    CHECK_EVAL("YEARFRAC( \"2001-01-01\" ; \"2002-01-01\" ; 2)", KCValue(1.0138888889));
    CHECK_EVAL("YEARFRAC( \"2001-12-05\" ; \"2001-12-30\" ; 2)", KCValue(0.0694444444));
    CHECK_EVAL("YEARFRAC( \"2000-02-05\" ; \"2006-08-10\" ; 2)", KCValue(6.6055555556));

    // basis 3 Actual/365
    CHECK_EVAL("YEARFRAC( \"2000-01-01\" ; \"2001-01-01\" ; 3)", KCValue(1.0027397260));
    CHECK_EVAL("YEARFRAC( \"2001-01-01\" ; \"2002-01-01\" ; 3)", KCValue(1.0000000000));
    CHECK_EVAL("YEARFRAC( \"2001-12-05\" ; \"2001-12-30\" ; 3)", KCValue(0.0684931507));
    CHECK_EVAL("YEARFRAC( \"2000-02-05\" ; \"2006-08-10\" ; 3)", KCValue(6.5150684932));

    // basis 4 European 30/360
    CHECK_EVAL("YEARFRAC( \"1999-01-01\" ; \"1999-06-30\" ; 4)", KCValue(0.4972222222));
    CHECK_EVAL("YEARFRAC( \"2000-01-01\" ; \"2000-06-30\" ; 4)", KCValue(0.4972222222));
    CHECK_EVAL("YEARFRAC( \"2000-01-15\" ; \"2000-09-17\" ; 4)", KCValue(0.6722222222));
    CHECK_EVAL("YEARFRAC( \"2000-01-01\" ; \"2001-01-01\" ; 4)", KCValue(1.0000000000));
    CHECK_EVAL("YEARFRAC( \"2001-01-01\" ; \"2002-01-01\" ; 4)", KCValue(1.0000000000));
    CHECK_EVAL("YEARFRAC( \"2001-12-05\" ; \"2001-12-30\" ; 4)", KCValue(0.0694444444));
    CHECK_EVAL("YEARFRAC( \"2000-02-05\" ; \"2006-08-10\" ; 4)", KCValue(6.5138888889));

    // alternate function name
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETYEARFRAC(\"1999-01-01\";\"1999-06-30\";1)", KCValue(0.4931506849));
}

void TestDatetimeFunctions::testDATEDIF()
{
    // interval y  ( years )
    CHECK_EVAL("DATEDIF(DATE(1990;2;15); DATE(1993;9;15); \"y\")", KCValue(3));        // TODO check value; kcells says 3

    // interval m  ( Months. If there is not a complete month between the dates, 0 will be returned.)
    CHECK_EVAL("DATEDIF(DATE(1990;2;15); DATE(1993;9;15); \"m\")", KCValue(43));

    // interval d  ( Days )
    CHECK_EVAL("DATEDIF(DATE(1990;2;15); DATE(1993;9;15); \"d\")", KCValue(1308));     // TODO check value; kcells says 1308

    // interval md ( Days, ignoring months and years )
    CHECK_EVAL("DATEDIF(DATE(1990;2;15); DATE(1993;9;15); \"md\")", KCValue(0));

    // interval ym ( Months, ignoring years )
    CHECK_EVAL("DATEDIF(DATE(1990;2;15); DATE(1993;9;15); \"ym\")", KCValue(7));

    // interval yd ( Days, ignoring years )
    CHECK_EVAL("DATEDIF(DATE(1990;2;15); DATE(1993;9;15); \"yd\")", KCValue(212));     // TODO check value; kcells says 212
}

void TestDatetimeFunctions::testISLEAPYEAR()
{
    // only every 400 years ...
    CHECK_EVAL("ISLEAPYEAR(1900)", KCValue(false));
    CHECK_EVAL("ISLEAPYEAR(2000)", KCValue(true));
    CHECK_EVAL("ISLEAPYEAR(2100)", KCValue(false));
    CHECK_EVAL("ISLEAPYEAR(2200)", KCValue(false));
    CHECK_EVAL("ISLEAPYEAR(2300)", KCValue(false));
    CHECK_EVAL("ISLEAPYEAR(2400)", KCValue(true));
    CHECK_EVAL("ISLEAPYEAR(1900)", KCValue(false));
    // and every 4th year
    CHECK_EVAL("ISLEAPYEAR(2000)", KCValue(true));
    CHECK_EVAL("ISLEAPYEAR(2001)", KCValue(false));
    CHECK_EVAL("ISLEAPYEAR(2002)", KCValue(false));
    CHECK_EVAL("ISLEAPYEAR(2003)", KCValue(false));
    CHECK_EVAL("ISLEAPYEAR(2004)", KCValue(true));
    // test alternate name for the ISLEAPYEAR function
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETISLEAPYEAR(1900)", KCValue(false));
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETISLEAPYEAR(2000)", KCValue(true));
}

void TestDatetimeFunctions::testWEEKNUM()
{
    // is known as weeknum_add() in OOo

    // type default ( type 1 )
    CHECK_EVAL("WEEKNUM(DATE(2000;05;21))", KCValue(22));     //
    CHECK_EVAL("WEEKNUM(DATE(2005;01;01))", KCValue(01));     //
    CHECK_EVAL("WEEKNUM(DATE(2000;01;02))", KCValue(02));     //
    CHECK_EVAL("WEEKNUM(DATE(2000;01;03))", KCValue(02));     //
    CHECK_EVAL("WEEKNUM(DATE(2000;01;04))", KCValue(02));     //
    CHECK_EVAL("WEEKNUM(DATE(2006;01;01))", KCValue(01));     //

    // type 1
    CHECK_EVAL("WEEKNUM(DATE(2000;05;21);1)", KCValue(22));
    CHECK_EVAL("WEEKNUM(DATE(2008;03;09);1)", KCValue(11));

    // type 2
    CHECK_EVAL("WEEKNUM(DATE(2000;05;21);2)", KCValue(21));
    CHECK_EVAL("WEEKNUM(DATE(2005;01;01);2)", KCValue(01));     // ref. OOo-2.2.0 = 1
    CHECK_EVAL("WEEKNUM(DATE(2000;01;02);2)", KCValue(01));     // ref. OOo-2.2.0 = 1
    CHECK_EVAL("WEEKNUM(DATE(2000;01;03);2)", KCValue(02));     // ref. OOo-2.2.0 = 2
    CHECK_EVAL("WEEKNUM(DATE(2000;01;04);2)", KCValue(02));     // ref. OOo-2.2.0 = 2
    CHECK_EVAL("WEEKNUM(DATE(2008;03;09);2)", KCValue(10));

    // additional tests for method 2
    CHECK_EVAL("WEEKNUM(DATE(2006;01;01);2)", KCValue(01));
    CHECK_EVAL("WEEKNUM(DATE(2006;01;02);2)", KCValue(02));

    // alternate function name
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETWEEKNUM(DATE(2000;05;21);1)", KCValue(22));
}

void TestDatetimeFunctions::testWEEKSINYEAR()
{
    //
    CHECK_EVAL("WEEKSINYEAR(1995)", KCValue(52));
    CHECK_EVAL("WEEKSINYEAR(1992)", KCValue(53));
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETWEEKSINYEAR(1992)", KCValue(53));
}

void TestDatetimeFunctions::testWORKDAY()
{
    // 2001 JAN 01 02 03 04 05 06 07 08
    //          MO TU WE TH FR SA SU MO
    //          01 02 -- --
    CHECK_EVAL("WORKDAY(DATE(2001;01;01);2;2)=DATE(2001;01;05)", KCValue(true));
    CHECK_EVAL("WORKDAY(DATE(2001;01;01);2;3)=DATE(2001;01;08)", KCValue(true));
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETWORKDAY(DATE(2001;01;01);2;3)=DATE(2001;01;08)", KCValue(true));
}

void TestDatetimeFunctions::testNETWORKDAY()
{
    // 2001 JAN 01 02 03 04 05 06 07 08 09
    //          MO TU WE TH FR SA SU MO TU
    //             01 02 03 04 05 05 05 06 ... networkdays
    CHECK_EVAL("NETWORKDAY(DATE(2001;01;01);DATE(2001;01;08))", KCValue(5));
    CHECK_EVAL("NETWORKDAY(DATE(2001;01;01);DATE(2001;01;07))", KCValue(5));
    CHECK_EVAL("NETWORKDAY(DATE(2001;01;01);DATE(2001;01;06))", KCValue(5));
    CHECK_EVAL("NETWORKDAY(DATE(2001;01;01);DATE(2001;01;05))", KCValue(4));

    // 2008 FEB 25 26 27 28 29 01 02 03 04
    //          MO TU WE TH FR SA SU MO TU
    //             01 02 03 04 05 05 05 06 ... networkdays
    CHECK_EVAL("NETWORKDAY(DATE(2008;02;25);DATE(2008;02;28))", KCValue(3));
    CHECK_EVAL("NETWORKDAY(DATE(2008;02;25);DATE(2008;02;29))", KCValue(4));
    CHECK_EVAL("NETWORKDAY(DATE(2008;02;25);DATE(2008;03;01))", KCValue(5));
    CHECK_EVAL("NETWORKDAY(DATE(2008;02;25);DATE(2008;03;02))", KCValue(5));
    CHECK_EVAL("NETWORKDAY(DATE(2008;02;25);DATE(2008;03;03))", KCValue(5));
    CHECK_EVAL("NETWORKDAY(DATE(2008;02;25);DATE(2008;03;04))", KCValue(6));
}

void TestDatetimeFunctions::testUNIX2DATE()
{
    // 01/01/2001 = 946684800
    CHECK_EVAL("UNIX2DATE(946684800)=DATE(2000;01;01)", KCValue(true));     // TODO result of various unix-timestamp calculator is 946681200 (UTC?)
}

void TestDatetimeFunctions::testDATE2UNIX()
{
    // 946681200 = 01/01/2001
    CHECK_EVAL("DATE2UNIX(DATE(2000;01;01))=946684800", KCValue(true));     // TODO
}

void TestDatetimeFunctions::testDATE()
{
    //
    CHECK_EVAL("DATE(2005;12;31)-DATE(1904;01;01)",   KCValue(37255));
    CHECK_EVAL("DATE(2004;02;29)=DATE(2004;02;28)+1", KCValue(true));     // leap year
    CHECK_EVAL("DATE(2000;02;29)=DATE(2000;02;28)+1", KCValue(true));     // leap year
    CHECK_EVAL("DATE(2005;03;01)=DATE(2005;02;28)+1", KCValue(true));     // no leap year
    CHECK_EVAL("DATE(2017.5;01;02)=DATE(2017;01;02)", KCValue(true));     // fractional values for year are truncated
    CHECK_EVAL("DATE(2006; 2.5; 3)=DATE(2006; 2; 3)", KCValue(true));     // fractional values for month are truncated
    CHECK_EVAL("DATE(2006;01;03.5)=DATE(2006;01;03)", KCValue(true));     // fractional values for day are truncated
    CHECK_EVAL("DATE(2006;13;03)=DATE(2007;01;03)",   KCValue(true));     // months > 12 roll over to year
    CHECK_EVAL("DATE(2006;01;32)=DATE(2006;02;01)",   KCValue(true));     // days greater than month limit roll over to month
    CHECK_EVAL("DATE(2006;25;34)=DATE(2008;02;03)",   KCValue(true));     // days and months roll over transitively
    CHECK_EVAL("DATE(2006;-01;01)=DATE(2005;11;01)",  KCValue(true));     // negative months roll year backward
    CHECK_EVAL("DATE(2006;04;-01)=DATE(2006;03;30)",  KCValue(true));     // negative days roll month backward
    CHECK_EVAL("DATE(2006;-4;-1)=DATE(2005;07;30)",   KCValue(true));     // negative days and months roll backward transitively
    CHECK_EVAL("DATE(2003;2;29)=DATE(2003;03;01)",    KCValue(true));     // non-leap year rolls forward
}

void TestDatetimeFunctions::testDATEVALUE()
{
    //
    CHECK_EVAL("DATEVALUE(\"2004-12-25\")=DATE(2004;12;25)", KCValue(true));
}

void TestDatetimeFunctions::testDAY()
{
    //
    CHECK_EVAL("DAY(DATE(2006;05;21))", KCValue(21));
    CHECK_EVAL("DAY(\"2006-12-15\")",   KCValue(15));
}

void TestDatetimeFunctions::testDAYS()
{
    //
    CHECK_EVAL("DAYS(DATE(1993;4;16); DATE(1993;9;25))", KCValue(-162));     //
}

void TestDatetimeFunctions::testDAYSINMONTH()
{
    // non leapyear
    CHECK_EVAL("DAYSINMONTH(1995;01)", KCValue(31));
    CHECK_EVAL("DAYSINMONTH(1995;02)", KCValue(28));
    CHECK_EVAL("DAYSINMONTH(1995;03)", KCValue(31));
    CHECK_EVAL("DAYSINMONTH(1995;04)", KCValue(30));
    CHECK_EVAL("DAYSINMONTH(1995;05)", KCValue(31));
    CHECK_EVAL("DAYSINMONTH(1995;06)", KCValue(30));
    CHECK_EVAL("DAYSINMONTH(1995;07)", KCValue(31));
    CHECK_EVAL("DAYSINMONTH(1995;08)", KCValue(31));
    CHECK_EVAL("DAYSINMONTH(1995;09)", KCValue(30));
    CHECK_EVAL("DAYSINMONTH(1995;10)", KCValue(31));
    CHECK_EVAL("DAYSINMONTH(1995;11)", KCValue(30));
    CHECK_EVAL("DAYSINMONTH(1995;12)", KCValue(31));

    // leapyear
    CHECK_EVAL("DAYSINMONTH(2000;02)", KCValue(29));
    CHECK_EVAL("DAYSINMONTH(1900;02)", KCValue(28));     // non leapyear
    CHECK_EVAL("DAYSINMONTH(2004;02)", KCValue(29));

    // test alternate name for the DAYSINMONTH function
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDAYSINMONTH(1995;01)", KCValue(31)); // alternate function name
}

void TestDatetimeFunctions::testDAYSINYEAR()
{
    CHECK_EVAL("DAYSINYEAR(2000)", KCValue(366));
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDAYSINYEAR(2000)", KCValue(366)); // alternate function name
}

void TestDatetimeFunctions::testDAYS360()
{
    // TODO Note: Lotus 1-2-3v9.8 has a function named DAYS but with different semantics.  It supports an optional "Basis" parameter
    // with many different options.  Without the optional parameter, it defaults to a 30/360 basis, not calendar days; thus, in Lotus 1-2-3v9.8,
    // DAYS(DATE(1993;4;16);  DATE(1993;9;25)) computes -159, not -162.

    CHECK_EVAL("DAYS360(DATE(1993;4;16);DATE(1993;9;25); FALSE)", KCValue(159));     // specs. -162 but OOo and KCells calculate 159
    CHECK_EVAL("DAYS360(\"2002-02-22\"; \"2002-04-21\" ; FALSE)", KCValue(59));      // ref. docs
}

void TestDatetimeFunctions::testEDATE()
{
    //
    CHECK_EVAL("EDATE(\"2006-01-01\";0)  =DATE(2006;01;01)", KCValue(true));     // If zero, unchanged.
    CHECK_EVAL("EDATE(DATE(2006;01;01);0)=DATE(2006;01;01)", KCValue(true));     // You can pass strings or serial numbers to EDATE
    CHECK_EVAL("EDATE(\"2006-01-01\"; 2) =DATE(2006;03;01)", KCValue(true));     //
    CHECK_EVAL("EDATE(\"2006-01-01\";-2) =DATE(2005;11;01)", KCValue(true));     // 2006 is not a leap year. Last day of March, going back to February
    CHECK_EVAL("EDATE(\"2000-04-30\";-2) =DATE(2000; 2;29)", KCValue(true));     // TODO 2000 was a leap year, so the end of February is the 29th
    CHECK_EVAL("EDATE(\"2000-04-05\";24 )=DATE(2002;04;05)", KCValue(true));     // EDATE isn't limited to 12 months
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETEDATE(\"2006-01-01\";0)  =DATE(2006;01;01)", KCValue(true)); // alternate function name
}

void TestDatetimeFunctions::testEOMONTH()
{
    //
    CHECK_EVAL("EOMONTH(\"2006-01-01\";0)  =DATE(2006;01;31)", KCValue(true));      // If zero, unchanged V just returns
    // end of that date's month. (January in this case)
    CHECK_EVAL("EOMONTH(DATE(2006;01;01);0)=DATE(2006;01;31)", KCValue(true));      // You can pass strings or serial numbers to EOMONTH
    CHECK_EVAL("EOMONTH(\"2006-01-01\";2)  =DATE(2006;03;31)", KCValue(true));      // End of month of March is March 31.
    CHECK_EVAL("EOMONTH(\"2006-01-01\";-2) =DATE(2005;11;30)", KCValue(true));      // Nov. 30 is the last day of November
    CHECK_EVAL("EOMONTH(\"2006-03-31\";-1) =DATE(2006;02;28)", KCValue(true));      // 2006 is not a leap year. Last day of  February is Feb. 28.
    CHECK_EVAL("EOMONTH(\"2000-04-30\";-2) =DATE(2000;02;29)", KCValue(true));      // 2000 was a leap year, so the end of February is the 29th
    CHECK_EVAL("EOMONTH(\"2000-04-05\";24) =DATE(2002;04;30)", KCValue(true));      // Not limited to 12 months, and this tests April
    CHECK_EVAL("EOMONTH(\"2006-01-05\";04) =DATE(2002;05;31)", KCValue(false));     // End of May is May 31
    CHECK_EVAL("EOMONTH(\"2006-01-05\";05) =DATE(2002;06;30)", KCValue(false));     // June 30
    CHECK_EVAL("EOMONTH(\"2006-01-05\";06) =DATE(2002;07;31)", KCValue(false));     // July 31
    CHECK_EVAL("EOMONTH(\"2006-01-05\";07) =DATE(2002;08;31)", KCValue(false));     // August 31
    CHECK_EVAL("EOMONTH(\"2006-01-05\";08) =DATE(2002;09;30)", KCValue(false));     // Sep 30
    CHECK_EVAL("EOMONTH(\"2006-01-05\";09) =DATE(2002;10;31)", KCValue(false));     // Oct 31
    CHECK_EVAL("EOMONTH(\"2006-01-05\";10) =DATE(2002;11;30)", KCValue(false));     // Nov. 30
    CHECK_EVAL("EOMONTH(\"2006-01-05\";11) =DATE(2002;12;31)", KCValue(false));     // Dec. 31
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETEOMONTH(\"2006-01-01\";0)  =DATE(2006;01;31)", KCValue(true)); // alternate function name
}

void TestDatetimeFunctions::testHOUR()
{
    //
    CHECK_EVAL("HOUR(5/24)",              KCValue(5));      // 5/24ths of a day is 5 hours, aka 5AM.
    CHECK_EVAL("HOUR(5/24-1/(24*60*60))", KCValue(4));      // A second before 5AM, it's 4AM.
    CHECK_EVAL("HOUR(\"14:00\")",         KCValue(14));     // TimeParam accepts text
    CHECK_EVAL("HOUR(\"9:00\")",          KCValue(9));
    CHECK_EVAL("HOUR(\"09:00\")",         KCValue(9));
    CHECK_EVAL("HOUR(\"23:00\")",         KCValue(23));
    CHECK_EVAL("HOUR(\"11:00 PM\")",      KCValue(23));
    CHECK_EVAL("HOUR(\"11:00 AM\")",      KCValue(11));
}

void TestDatetimeFunctions::testISOWEEKNUM()
{
    // ODF-tests
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;1;1);1)",   KCValue(1));      // January 1, 1995 was a Sunday
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;1;1);2)",   KCValue(52));     // January 1, 1995 was a Sunday, so if Monday is the beginning of the week,
    // then it's week 52 of the previous year
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;1;1))",     KCValue(52));     // Default is Monday is beginning of week (per ISO)
    CHECK_EVAL("ISOWEEKNUM(DATE(2000;5;21))",    KCValue(20));     // ref OOo-2.2.0
    CHECK_EVAL("ISOWEEKNUM(DATE(2000;5;21);1)",  KCValue(21));     // ref OOo-2.2.0
    CHECK_EVAL("ISOWEEKNUM(DATE(2000;5;21);2)",  KCValue(20));     // ref OOo-2.2.0
    CHECK_EVAL("ISOWEEKNUM(DATE(2005;1;1))",     KCValue(53));     // ref OOo-2.2.0
    CHECK_EVAL("ISOWEEKNUM(DATE(2005;1;2))",     KCValue(53));     // ref OOo-2.2.0
    CHECK_EVAL("ISOWEEKNUM(DATE(2006;1;1))",     KCValue(52));     // ref OOo-2.2.0

    // method 2 - week begins on sunday
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;01);2)", KCValue(52));     // January 1, 1995 was a Sunday
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;02);2)", KCValue(1));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;03);2)", KCValue(1));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;04);2)", KCValue(1));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;05);2)", KCValue(1));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;06);2)", KCValue(1));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;07);2)", KCValue(1));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;08);2)", KCValue(1));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;09);2)", KCValue(2));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;10);2)", KCValue(2));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;11);2)", KCValue(2));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;12);2)", KCValue(2));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;13);2)", KCValue(2));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;14);2)", KCValue(2));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;15);2)", KCValue(2));      //

    // method 1 - week begins on monday
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;01);1)", KCValue(1));      // January 1, 1995 was a Sunday
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;02);1)", KCValue(1));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;03);1)", KCValue(1));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;04);1)", KCValue(1));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;05);1)", KCValue(1));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;06);1)", KCValue(1));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;07);1)", KCValue(1));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;08);1)", KCValue(2));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;09);1)", KCValue(2));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;10);1)", KCValue(2));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;11);1)", KCValue(2));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;12);1)", KCValue(2));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;13);1)", KCValue(2));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;14);1)", KCValue(2));      //
    CHECK_EVAL("ISOWEEKNUM(DATE(1995;01;15);1)", KCValue(3));      //
}

void TestDatetimeFunctions::testMINUTE()
{
    //
    CHECK_EVAL("MINUTE(1/(24*60))",         KCValue(1));     // 1 minute is 1/(24*60) of a day.
    CHECK_EVAL("MINUTE(TODAY()+1/(24*60))", KCValue(1));     // If you start with today, and add a minute, you get a minute.
    CHECK_EVAL("MINUTE(1/24)",              KCValue(0));     // At the beginning of the hour, we have 0 minutes.
}

void TestDatetimeFunctions::testMONTH()
{
    //
    CHECK_EVAL("MONTH(DATE(2006;5;21))", KCValue(5));        // Month extraction from DATE() value
}

void TestDatetimeFunctions::testMONTHS()
{
    CHECK_EVAL("MONTHS(\"2002-01-18\"; \"2002-02-26\"; 0)", KCValue(1));
    CHECK_EVAL("MONTHS(\"2002-01-19\"; \"2002-02-26\"; 1)", KCValue(0));
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFMONTHS(\"2002-01-18\"; \"2002-02-26\"; 0)", KCValue(1));
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFMONTHS(\"2002-01-19\"; \"2002-02-26\"; 1)", KCValue(0));
}

void TestDatetimeFunctions::testNOW()
{
    //
    CHECK_EVAL("NOW()>DATE(2006;1;3)", KCValue(true));       // NOW constantly changes, but we know it's beyond this date.
    CHECK_EVAL("INT(NOW())=TODAY()",   KCValue(true));
}

void TestDatetimeFunctions::testSECOND()
{
    //
    CHECK_EVAL("SECOND(1/(24*60*60))",   KCValue(1));        // This is one second into today.
    CHECK_EVAL("SECOND(1/(24*60*60*2))", KCValue(1));        // Rounds.
    CHECK_EVAL("SECOND(1/(24*60*60*4))", KCValue(0));        // Rounds.
}

void TestDatetimeFunctions::testTIME()
{
    //
    CHECK_EVAL("TIME(0;0;0)",                 KCValue(0));         // All zero arguments becomes midnight, 12:00:00 AM.
    CHECK_EVAL("TIME(23;59;59)*60*60*24",     KCValue(86399));     // This is 11:59:59 PM.
    CHECK_EVAL("TIME(11;125;144)*60*60*24",   KCValue(47244));     // Seconds and minutes roll over transitively; this is 1:07:24 PM.
    CHECK_EVAL("TIME(11;0; -117)*60*60*24",   KCValue(39483));     // Negative seconds roll minutes backwards, 10:58:03 AM
    CHECK_EVAL("TIME(11;-117;0)*60*60*24",    KCValue(32580));     // Negative minutes roll hours backwards, 9:03:00 AM

    CHECK_EVAL("TIME(11;-125;-144)*60*60*24", KCValue(31956));     // Negative seconds and minutes roll backwards transitively, 8:52:36 AM
    // WARNING specs says -31956, but calc and kcells calculate 31956
}

void TestDatetimeFunctions::testTIMEVALUE()
{
    //
    CHECK_EVAL("TIMEVALUE(\"06:05\")   =TIME(6;5;0)", KCValue(true));
    CHECK_EVAL("TIMEVALUE(\"06:05:07\")=TIME(6;5;7)", KCValue(true));
}

void TestDatetimeFunctions::testTODAY()
{
    //
    CHECK_EVAL("TODAY()>DATE(2006;1;3)", KCValue(true));     // Every date TODAY() changes, but we know it's beyond this date.
    CHECK_EVAL("INT(TODAY())=TODAY()",   KCValue(true));
}

void TestDatetimeFunctions::testWEEKDAY()
{
    //    | type 1 |  type 2 | type 3
    // ---+--------+---------+--------
    // 01 |  SU    |   MO    |  TU
    // 02 |  MO    |   TU    |  WE
    // 03 |  TU    |   WE    |  TH
    // 04 |  WE    |   TH    |  FR
    // 05 |  TH    |   FR    |  SA
    // 06 |  FR    |   SA    |  SU
    // 07 |  SA    |   SU    |  MO

    CHECK_EVAL("WEEKDAY(DATE(2006;05;21))",   KCValue(1));     // Year-month-date format
    CHECK_EVAL("WEEKDAY(DATE(2005;01;01))",   KCValue(7));     // Saturday
    CHECK_EVAL("WEEKDAY(DATE(2005;01;01);1)", KCValue(7));     // Saturday
    CHECK_EVAL("WEEKDAY(DATE(2005;01;01);2)", KCValue(6));     // Saturday
    CHECK_EVAL("WEEKDAY(DATE(2005;01;01);3)", KCValue(5));     // Saturday
}

void TestDatetimeFunctions::testYEAR()
{
    CHECK_EVAL("YEAR(DATE(1904;1;1))", KCValue(1904));
    CHECK_EVAL("YEAR(DATE(2004;1;1))", KCValue(2004));
}

void TestDatetimeFunctions::testYEARS()
{
    CHECK_EVAL("YEARS(\"2001-02-19\"; \"2002-02-26\"; 0)", KCValue(1));
    CHECK_EVAL("YEARS(\"2002-02-19\"; \"2002-02-26\"; 1)", KCValue(0));
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFYEARS(\"2001-02-19\";\"2002-02-26\";0)", KCValue(1));
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFYEARS(\"2002-02-19\";\"2002-02-26\";1)", KCValue(0));
}

void TestDatetimeFunctions::testWEEKS()
{
    CHECK_EVAL("WEEKS(\"2002-02-18\"; \"2002-02-26\"; 0)", KCValue(1));
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFWEEKS(\"2002-02-18\"; \"2002-02-26\"; 0)", KCValue(1));
}

QTEST_KDEMAIN(TestDatetimeFunctions, GUI)

#include "TestDatetimeFunctions.moc"
