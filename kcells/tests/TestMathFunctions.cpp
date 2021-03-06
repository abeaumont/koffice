/* This file is part of the KDE project
   Copyright 2007 Ariya Hidayat <ariya@kde.org>
   Copyright 2007 Sascha Pfau <MrPeacock@gmail.com>

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
#include "TestMathFunctions.h"

#include "TestKspreadCommon.h"

#include <KCCellStorage.h>
#include <KCFormula.h>
#include <KCMap.h>
#include <KCSheet.h>

// NOTE: we do not compare the numbers _exactly_ because it is difficult
// to get one "true correct" expected values for the functions due to:
//  - different algorithms among spreadsheet programs
//  - precision limitation of floating-point number representation
//  - accuracy problem due to propagated error in the implementation
#define CHECK_EVAL(x,y) QCOMPARE(TestDouble(x,y,6),y)
#define CHECK_EVAL_SHORT(x,y) QCOMPARE(TestDouble(x,y,10),y)
#define ROUND(x) (roundf(1e10 * x) / 1e10)

KCValue TestMathFunctions::TestDouble(const QString& formula, const KCValue& v2, int accuracy)
{
    double epsilon = DBL_EPSILON * pow(10.0, (double)(accuracy));

    KCFormula f(m_map->sheet(0));
    QString expr = formula;
    if (expr[0] != '=')
        expr.prepend('=');
    f.setExpression(expr);
    KCValue result = f.eval();

    bool res = fabs(v2.asFloat() - result.asFloat()) < epsilon;

    if (!res)
        kDebug(36002) << "check failed -->" << "Epsilon =" << epsilon << "" << (double) v2.asFloat() << " to" << (double)result.asFloat() << "  diff =" << (double)(v2.asFloat() - result.asFloat());
    /*  else
        kDebug(36002)<<"check -->" <<"  diff =" << v2.asFloat()-result.asFloat();*/
    if (res)
        return v2;
    else
        return result;
}

// round to get at most 10-digits number
inline static KCValue RoundNumber(double f)
{
    return KCValue(ROUND(f));
}

// round to get at most 10-digits number
inline static KCValue RoundNumber(const KCValue& v)
{
    if (v.isNumber()) {
        double d = numToDouble(v.asFloat());
        if (fabs(d) < DBL_EPSILON)
            d = 0.0;
        return KCValue(ROUND(d));
    } else
        return v;
}

KCValue TestMathFunctions::evaluate(const QString& formula)
{
    KCFormula f(m_map->sheet(0));
    QString expr = formula;
    if (expr[0] != '=')
        expr.prepend('=');
    f.setExpression(expr);
    KCValue result = f.eval();

#if 0
    // this magically generates the CHECKs
    printf("  CHECK_EVAL( \"%s\",  %15g) );\n", qPrintable(formula), result.asFloat());
#endif

    return RoundNumber(result);
}

void TestMathFunctions::initTestCase()
{
    KCFunctionModuleRegistry::instance()->loadFunctionModules();

    m_map = new KCMap(0 /* no KCDoc */);
    m_map->addNewSheet();
    KCSheet* sheet = m_map->sheet(0);
    KCCellStorage* storage = sheet->cellStorage();

    // B3:B7
    storage->setValue(2, 3, KCValue("7"));
    storage->setValue(2, 4, KCValue(2));
    storage->setValue(2, 5, KCValue(3));
    storage->setValue(2, 6, KCValue(true));
    storage->setValue(2, 7, KCValue("Hello"));
    // B9
    storage->setValue(2, 9, KCValue::errorDIV0());
}

void TestMathFunctions::cleanupTestCase()
{
    delete m_map;
}

///////////////////////////////////////////////////////////////////////////////////////////////

void TestMathFunctions::testABS()
{
    CHECK_EVAL("ABS(0)",   KCValue(0));
    CHECK_EVAL("ABS(-1)",  KCValue(1));
    CHECK_EVAL("ABS(-2)",  KCValue(2));
    CHECK_EVAL("ABS(-3)",  KCValue(3));
    CHECK_EVAL("ABS(-4)",  KCValue(4));
    CHECK_EVAL("ABS(1)",   KCValue(1));
    CHECK_EVAL("ABS(2)",   KCValue(2));
    CHECK_EVAL("ABS(3)",   KCValue(3));
    CHECK_EVAL("ABS(4)",   KCValue(4));
    CHECK_EVAL("ABS(1/0)", KCValue::errorDIV0());
}

void TestMathFunctions::testACOS()
{
    // ODF-tests
    CHECK_EVAL("ACOS(SQRT(2)/2)*4/PI()", KCValue(1.0));     // arc cosine of SQRT(2)/2 is PI()/4 radians.
    CHECK_EVAL("ACOS(TRUE())",           KCValue(0.0));     // TRUE() is 1 if inline.
    CHECK_EVAL("ACOS(-1.0)/PI()",        KCValue(1.0));     // The result must be between 0.0 and PI().
    CHECK_EVAL("ACOS(2.0)",       KCValue::errorVALUE());   // The argument must be between -1.0 and 1.0.

    // ACosinus needs to be a numeric value between >=-1.0 and <=1.0
    CHECK_EVAL("ACOS()", KCValue::errorVALUE());
    CHECK_EVAL("ACOS(-1.1)", KCValue::errorVALUE());
    CHECK_EVAL("ACOS(1.1)", KCValue::errorVALUE());

    CHECK_EVAL("ACOS(1.0)", KCValue(0));
    CHECK_EVAL_SHORT("2-ACOS(-1.0)", KCValue(-1.14159265));
}

void TestMathFunctions::testACOSH()
{
    // ODF-tests
    CHECK_EVAL("ACOSH(1)", KCValue(0));               //
    CHECK_EVAL("ACOSH(2)", KCValue(1.316957897));     //
}

void TestMathFunctions::testACOT()
{
    // ODF-tests
    CHECK_EVAL("ACOT(0)-PI()/2", KCValue(0));         //
}

void TestMathFunctions::testACOTH()
{
    // ODF-tests
    CHECK_EVAL("ACOTH(2)", KCValue(0.5493061443));     //
}

void TestMathFunctions::testASIN()
{
    // ODF-tests
    CHECK_EVAL("ASIN(SQRT(2)/2)*4/PI()",  KCValue(1.0));      // arc sine of SQRT(2)/2 is PI()/4 radians.
    CHECK_EVAL("ASIN(TRUE())*2/PI()",     KCValue(1.0));      // TRUE() is 1 if inline.
    CHECK_EVAL("ASIN(-1)*2/PI()",         KCValue(-1.0));     // The result must be between -PI()/2 and PI()/2.
    CHECK_EVAL("ASIN(2)",           KCValue::errorVALUE());   // The argument must be between -1.0 and 1.0.

    // ASinus needs to be a numeric value between >=-1.0 and <=1.0
    CHECK_EVAL("ASIN(1.2)", KCValue::errorVALUE());
    CHECK_EVAL("ASIN(-99)", KCValue::errorVALUE());

    CHECK_EVAL_SHORT("1-ASIN(1)", KCValue(-0.57079633));
    CHECK_EVAL_SHORT("1+ASIN(-1.0)", KCValue(-0.57079633));
}

void TestMathFunctions::testASINH()
{
    // ODF-tests
    CHECK_EVAL("ASINH(0)", KCValue(0));               //
    CHECK_EVAL("ASINH(1)", KCValue(0.881373587));     //
}

void TestMathFunctions::testATAN()
{
    // ODF-tests
    CHECK_EVAL("ATAN(1)*4/PI()", KCValue(1));                   // arc tangent of 1 is PI()/4 radians.
    CHECK_EVAL_SHORT("ATAN(-1.0e16)",  KCValue(-1.570796));     // TODO expand / Check if ATAN gives reasonably accurate results,
    // and that slightly negative values as input produce numbers near -PI/2.
}

void TestMathFunctions::testATAN2()
{
    // ODF-tests
    CHECK_EVAL("ATAN2(1;1)*4/PI()",        KCValue(1));      // arc tangent of 1.0/1.0 is PI()/4 radians.
    CHECK_EVAL("ATAN2(1;-1)*4/PI()",       KCValue(-1));     // Location of sign makes a difference.
    CHECK_EVAL("ATAN2(-1;1)*4/PI()",       KCValue(3));      // Location of sign makes a difference.
    CHECK_EVAL("ATAN2(-1;-1)*4/PI()",      KCValue(-3));     // Location of sign makes a difference.
    CHECK_EVAL("SIGN(ATAN2(-1.0;0.001))",  KCValue(1));      // If y is small, it's still important
    CHECK_EVAL("SIGN(ATAN2(-1.0;-0.001))", KCValue(-1));     // If y is small, it's still important
    CHECK_EVAL("ATAN2(-1.0;0)/PI()",       KCValue(1));      // By definition ATAN2(-1,0) should give PI() rather than -PI().
}

void TestMathFunctions::testATANH()
{
    // ODF-tests
    CHECK_EVAL_SHORT("ATANH(0)",   KCValue(0));               // TODO expand
    CHECK_EVAL_SHORT("ATANH(0.5)", KCValue(0.549306144));     // TODO expand
}

void TestMathFunctions::testBESSELI()
{
    // ODF-tests
    CHECK_EVAL_SHORT("BESSELI(2;2)",  KCValue(0.688948));     // TODO expand
    CHECK_EVAL_SHORT("BESSELI(0.7;3)", KCValue(0.007367));
    // alternate function name
    CHECK_EVAL_SHORT("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBESSELI(2;2)", KCValue(0.688948));
}

void TestMathFunctions::testBESSELJ()
{
    // ODF-tests
    CHECK_EVAL_SHORT("BESSELJ(1;0)",  KCValue(0.765198));     // TODO expand
    CHECK_EVAL_SHORT("BESSELJ(0.89;3)", KCValue(0.013974));
    // alternate function name
    CHECK_EVAL_SHORT("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBESSELJ(1;0)", KCValue(0.765198));
}

void TestMathFunctions::testBESSELK()
{
    // ODF-tests
    CHECK_EVAL_SHORT("BESSELK(3;0)",  KCValue(0.03474));      // TODO expand
    // alternate function name
    CHECK_EVAL_SHORT("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBESSELK(3;0)", KCValue(0.03474));
}

void TestMathFunctions::testBESSELY()
{
    // ODF-tests
    CHECK_EVAL_SHORT("BESSELY(1;1)", KCValue(-0.781213));     // TODO expand
    CHECK_EVAL_SHORT("BESSELY(4;2)", KCValue(0.215903595));
    // alternate function name
    CHECK_EVAL_SHORT("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBESSELY(1;1)", KCValue(-0.781213));
}

void TestMathFunctions::testCEIL()
{
    CHECK_EVAL("CEIL(0)", KCValue(0));

    CHECK_EVAL("CEIL(0.1)", KCValue(1));
    CHECK_EVAL("CEIL(0.01)", KCValue(1));
    CHECK_EVAL("CEIL(0.001)", KCValue(1));
    CHECK_EVAL("CEIL(0.0001)", KCValue(1));
    CHECK_EVAL("CEIL(0.00001)", KCValue(1));
    CHECK_EVAL("CEIL(0.000001)", KCValue(1));
    CHECK_EVAL("CEIL(0.0000001)", KCValue(1));

    CHECK_EVAL("CEIL(1.1)", KCValue(2));
    CHECK_EVAL("CEIL(1.01)", KCValue(2));
    CHECK_EVAL("CEIL(1.001)", KCValue(2));
    CHECK_EVAL("CEIL(1.0001)", KCValue(2));
    CHECK_EVAL("CEIL(1.00001)", KCValue(2));
    CHECK_EVAL("CEIL(1.000001)", KCValue(2));
    CHECK_EVAL("CEIL(1.0000001)", KCValue(2));

    CHECK_EVAL("CEIL(-0.1)", KCValue(0));
    CHECK_EVAL("CEIL(-0.01)", KCValue(0));
    CHECK_EVAL("CEIL(-0.001)", KCValue(0));
    CHECK_EVAL("CEIL(-0.0001)", KCValue(0));
    CHECK_EVAL("CEIL(-0.00001)", KCValue(0));
    CHECK_EVAL("CEIL(-0.000001)", KCValue(0));
    CHECK_EVAL("CEIL(-0.0000001)", KCValue(0));


    CHECK_EVAL("CEIL(-1.1)", KCValue(-1));
    CHECK_EVAL("CEIL(-1.01)", KCValue(-1));
    CHECK_EVAL("CEIL(-1.001)", KCValue(-1));
    CHECK_EVAL("CEIL(-1.0001)", KCValue(-1));
    CHECK_EVAL("CEIL(-1.00001)", KCValue(-1));
    CHECK_EVAL("CEIL(-1.000001)", KCValue(-1));
    CHECK_EVAL("CEIL(-1.0000001)", KCValue(-1));
}

void TestMathFunctions::testCEILING()
{
    CHECK_EVAL("CEILING(0  ; 0.1)", KCValue(0));
    CHECK_EVAL("CEILING(0  ; 0.2)", KCValue(0));
    CHECK_EVAL("CEILING(0  ; 1.0)", KCValue(0));
    CHECK_EVAL("CEILING(0  ;10.0)", KCValue(0));

    CHECK_EVAL("CEILING(0.1; 0.2)", KCValue(0.2));
    CHECK_EVAL("CEILING(0.1; 0.4)", KCValue(0.4));
    CHECK_EVAL("CEILING(1.1; 0.2)", KCValue(1.2));

    // because can't divide by 0
    CHECK_EVAL("CEILING(1; 0)", KCValue::errorDIV0());
    CHECK_EVAL("CEILING(2; 0)", KCValue::errorDIV0());

    // but this one should be just fine !
    CHECK_EVAL("CEILING(0; 0)", KCValue(0));

    // different sign does not make sense
    CHECK_EVAL("CEILING(-1; 2)", KCValue::errorNUM());
    CHECK_EVAL("CEILING(1; -2)", KCValue::errorNUM());
}

void TestMathFunctions::testCOMBIN()
{
    // ODF-tests
    CHECK_EVAL("COMBIN(5;3)",  KCValue(10));           //
    CHECK_EVAL("COMBIN(6;3)",  KCValue(20));           //
    CHECK_EVAL("COMBIN(42;3)", KCValue(11480));        //
    CHECK_EVAL("COMBIN(-1;3)", KCValue::errorNUM());   // N must be >= 0
    CHECK_EVAL("COMBIN(4;-3)", KCValue::errorNUM());   // M must be >= 0
}

void TestMathFunctions::testCOMBINA()
{
    // ODF-tests
    CHECK_EVAL("COMBINA(5;3)",  KCValue(35));           //
    CHECK_EVAL("COMBINA(-1;3)", KCValue::errorNUM());   // N must be >= 0
    CHECK_EVAL("COMBINA(4;-3)", KCValue::errorNUM());   // M must be >= 0
}

void TestMathFunctions::testCONVERT()
{
    // ODF-tests

    // TODO add missing SI-units and expand up to 10 digits

    CHECK_EVAL("CONVERT(   1; \"ft\";     \"in\")",   KCValue(12));               // 1 foot is 12 inches.  Conversion between
    // units might involve an intermediate SI unit
    // in some implementations, and such round-off
    // error is considered acceptable.
    CHECK_EVAL("CONVERT(   1; \"in\";     \"cm\")",   KCValue(2.54));             // 1 inch is 2.54 cm.  The result is exact, because
    // this needs to be represented as accurately as the
    // underlying numerical model permits
    CHECK_EVAL("CONVERT(   5; \"m\";      \"mm\")",   KCValue(5000));             // 5 meters is 5000 millimeters
    CHECK_EVAL("CONVERT( 100; \"C\";      \"F\")",    KCValue(212));              // 212 degrees F is 100 degrees C.  Note that this
    // is not simply a multiplicative relationship.
    //  Since internally this is (100/5*9+32), where
    // 100/5 is exactly 20, this result needs to be
    // exact even on floating-point implementations
    CHECK_EVAL("CONVERT(   2; \"Ym\";     \"Zm\")",   KCValue(2000));             // Must support Y and Z prefixes. (wrong ODF-specs 100)
    CHECK_EVAL("CONVERT(  20; \"F\";      \"m\")",    KCValue::errorNA());        // Different groups produce an error.
    CHECK_EVAL_SHORT("CONVERT(1000;\"qt\";\"l\")",    KCValue(946.5588641));      // Quart is U.S. customary, liquid measure
    CHECK_EVAL_SHORT("CONVERT(1000;\"tbs\";\"l\")",   KCValue(14.78998225));      // Tablespoon uses U.S. customary historic definition
    // - note that there are many other definitions
    CHECK_EVAL("CONVERT(1000; \"tsp\";    \"l\")",    KCValue(4.929994084));      // Teaspoon uses U.S. customary historic definition
    // - note that there are many other definitions
    CHECK_EVAL("CONVERT(   1; \"das\";    \"sec\")",  KCValue(10));               // Does it support both "s" and "sec" for second?
    // Does it support "da" as the SI standard deka prefix?
    CHECK_EVAL("CONVERT(   1; \"ar\";     \"m^2\")",  KCValue(100));              // A hectare (ar) is 100 square meters.
//   CHECK_EVAL( "CONVERT(   1; \"cal\";    \"J\")",      KCValue( 4.1868 ) );    // "cal" is an International Table (IT) calorie, 4.1868 J.
    CHECK_EVAL("CONVERT(   1; \"lbf\";    \"N\")",    KCValue(4.448222));         // Converting pound-force to Newtons
    CHECK_EVAL("CONVERT(   1; \"HP\";     \"W\")",    KCValue(745.701));          // Horsepower to Watts
    CHECK_EVAL("CONVERT(   1; \"Mibyte\"; \"bit\")",  KCValue(8388608));           // Converts bytes to bits, and tests binary prefixes
    CHECK_EVAL("CONVERT(   1; \"Gibyte\"; \"Mibyte\")", KCValue(1024));           // Converts bytes to bits, and tests binary prefixes
    CHECK_EVAL("CONVERT(   1; \"T\";      \"ga\")",   KCValue(10000));            // Tesla to Gauss
//   CHECK_EVAL( "CONVERT(   1; \"lbm\";    \"g\")",    KCValue( 453.59237 ) );   // International pound mass (avoirdupois) to grams.
    // (This is actually exact.)
    CHECK_EVAL("CONVERT(   1; \"uk_ton\"; \"lbm\")",  KCValue(2240));             // Imperial ton, aka "long ton", "deadweight ton",
    // or "weight ton", is 2240 lbm.
//   CHECK_EVAL( "CONVERT(   1; \"psi\";    \"Pa\")",   KCValue( 6894.76 ) );     // Pounds per square inch to Pascals.
    CHECK_EVAL("CONVERT(  60; \"mph\";    \"km/h\")", KCValue(96.56064));         // Miles per hour to kilometers per hour.
    CHECK_EVAL("CONVERT(   1; \"day\";    \"s\")",    KCValue(86400));            // Day to seconds.  Note: This test uses the
    // international standard abbreviation for second (s),
    // not the abbreviation traditionally used in spreadsheets
    // (sec); both "s" and "sec" must be supported.
//   CHECK_EVAL_SHORT( "CONVERT( 1; \"qt\";    \"L\")", KCValue( 0.9463529460 ) ); // Quart (U.S. customary liquid measure) to liter.
    // This is 0.946352946 liters,
}

void TestMathFunctions::testCOT()
{
    // ODF-tests
    CHECK_EVAL("COT(PI()/4.0)", KCValue(1));             // cotangent of PI()/4.0 radians.
    CHECK_EVAL("COT(PI()/2.0)", KCValue(0));             // cotangent of PI()/2 radians.  Not the same as TAN.
    CHECK_EVAL("COT(0)", KCValue::errorDIV0());          // cotangent of PI()/4.0 radians.
}

void TestMathFunctions::testCOTH()
{
    // ODF-tests
    CHECK_EVAL("COTH(1)",      KCValue(1.3130352855));     //
    CHECK_EVAL("COTH(EXP(1))", KCValue(1.0087469296));     //
}

void TestMathFunctions::testDEGREES()
{
    // ODF-tests
    CHECK_EVAL("DEGREES(PI())", KCValue(180));      // PI() radians is 180 degrees.
}

void TestMathFunctions::testDELTA()
{
    // ODF-tests
    CHECK_EVAL("DELTA(2;3)", KCValue(0));      // Different numbers are not equal
    CHECK_EVAL("DELTA(2;2)", KCValue(1));      // Same numbers are equal
    CHECK_EVAL("DELTA(0)"  , KCValue(1));      // 0 equal to default 0
}

void TestMathFunctions::testEVEN()
{
    // ODF-tests
    CHECK_EVAL("EVEN(6)",    KCValue(6));      // Positive even integers remain unchanged.
    CHECK_EVAL("EVEN(-4)",   KCValue(-4));     // Negative even integers remain unchanged.
    CHECK_EVAL("EVEN(1)",    KCValue(2));      // Non-even positive integers round up.
    CHECK_EVAL("EVEN(0.3)",  KCValue(2));      // Positive floating values round up.
    CHECK_EVAL("EVEN(-1)",   KCValue(-2));     // Non-even negative integers round down.
    CHECK_EVAL("EVEN(-0.3)", KCValue(-2));     // Negative floating values round down.
    CHECK_EVAL("EVEN(0)",    KCValue(0));      // Since zero is even, EVEN(0) returns zero.
}

void TestMathFunctions::testEXP()
{
    // ODF-tests
    CHECK_EVAL("EXP(0)",     KCValue(1));                          // Anything raised to the 0 power is 1.
    CHECK_EVAL("EXP(LN(2))", KCValue(2));                          // The EXP function is the inverse of the LN function.
    CHECK_EVAL("EXP(1)",     KCValue(2.71828182845904523536));     // The value of the natural logarithm e.
}

void TestMathFunctions::testFACT()
{
    CHECK_EVAL("FACT(0)", KCValue(1));
    CHECK_EVAL("FACT(1)", KCValue(1));
    CHECK_EVAL("FACT(2)", KCValue(2));
    CHECK_EVAL("FACT(3)", KCValue(6));
    CHECK_EVAL("FACT(-1)",        KCValue::errorNUM());
    CHECK_EVAL("FACT(\"xyzzy\")", KCValue::errorNUM());
}

void TestMathFunctions::testFACTDOUBLE()
{
    CHECK_EVAL("FACTDOUBLE(0)", KCValue(1));
    CHECK_EVAL("FACTDOUBLE(1)", KCValue(1));
    CHECK_EVAL("FACTDOUBLE(7)", KCValue(105));
    CHECK_EVAL("FACTDOUBLE(6)", KCValue(48));
    CHECK_EVAL("FACTDOUBLE(-1)",        KCValue::errorNUM());
    CHECK_EVAL("FACTDOUBLE(\"xyzzy\")", KCValue::errorNUM());
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETFACTDOUBLE(7)", KCValue(105)); // alternate function name
}

void TestMathFunctions::testFIB()
{
    CHECK_EVAL("FIB(1)",  KCValue(1));
    CHECK_EVAL("FIB(2)",  KCValue(1));
    CHECK_EVAL("FIB(3)",  KCValue(2));
    CHECK_EVAL("FIB(4)",  KCValue(3));
    CHECK_EVAL("FIB(5)",  KCValue(5));
    CHECK_EVAL("FIB(6)",  KCValue(8));
    CHECK_EVAL("FIB(7)",  KCValue(13));
    CHECK_EVAL("FIB(8)",  KCValue(21));
    CHECK_EVAL("FIB(9)",  KCValue(34));
    CHECK_EVAL("FIB(10)", KCValue(55));

    // large number
    CHECK_EVAL("FIB(100)/1E+20",  KCValue(3.54224848179263));
    CHECK_EVAL("FIB(200)/1E+41",  KCValue(2.80571172992512));
    CHECK_EVAL("FIB(300)/1E+62",  KCValue(2.22232244629423));
    CHECK_EVAL("FIB(400)/1E+83",  KCValue(1.76023680645016));
    CHECK_EVAL("FIB(500)/1E+104", KCValue(1.394232245617));
    CHECK_EVAL("FIB(600)/1E+125", KCValue(1.10433070572954));

    // invalid
    CHECK_EVAL("FIB(0)",        KCValue::errorNUM());
    CHECK_EVAL("FIB(-1)",       KCValue::errorNUM());
    CHECK_EVAL("FIB(\"text\")", KCValue::errorVALUE());
}

void TestMathFunctions::testFLOOR()
{
    // ODF-Tests
    CHECK_EVAL("=FLOOR(2; 1)",       KCValue(2));
    CHECK_EVAL("=FLOOR(2.5; 1)",     KCValue(2));
    CHECK_EVAL("=FLOOR(5; 2)",       KCValue(4));
    CHECK_EVAL("=FLOOR(5; 2.2)",     KCValue(4.4));
    CHECK_EVAL("=FLOOR(-2.5;1)",     KCValue::errorVALUE());
    CHECK_EVAL("=FLOOR(-2.5; -1)",   KCValue(-3));
    CHECK_EVAL("=FLOOR(-2.5; -1;1)", KCValue(-2));
    CHECK_EVAL("=FLOOR(-2.5;0)",     KCValue(0));
    CHECK_EVAL("=FLOOR(0;-1)",       KCValue(0));
    CHECK_EVAL("=FLOOR(-1.1)",       KCValue(-2));
}

void TestMathFunctions::testGAMMA()
{
    // ODF-Tests
    CHECK_EVAL("GAMMA(1.00)", KCValue(1.0000000000));
    CHECK_EVAL("GAMMA(1.10)", KCValue(0.9513507700));
    CHECK_EVAL("GAMMA(1.50)", KCValue(0.8862269255));
}

void TestMathFunctions::testGAMMALN()
{
    // ODF-Tests
    CHECK_EVAL("GAMMALN(1.00)", KCValue(0));
    CHECK_EVAL("GAMMALN(2.00)", KCValue(0));
    CHECK_EVAL("GAMMALN(3.00)", KCValue(0.6931471806));
    CHECK_EVAL("GAMMALN(1.50)", KCValue(-0.1207822376));
}

void TestMathFunctions::testGCD()
{
    CHECK_EVAL("GCD(5;15;25)",      KCValue(5));
    CHECK_EVAL("GCD(2;3)",          KCValue(1));
    CHECK_EVAL("GCD(18;24)",        KCValue(6));
    CHECK_EVAL("GCD(18.1;24.1)",    KCValue(6));
    CHECK_EVAL("GCD(1.1;2.2)",      KCValue(1));
    CHECK_EVAL("GCD(18.9;24.9)",    KCValue(6));
    CHECK_EVAL("GCD(7)",            KCValue(7));
    CHECK_EVAL("GCD(5;0)",          KCValue(5));
    CHECK_EVAL("GCD(0;0)",          KCValue(0));
    CHECK_EVAL("GCD(-2;3)",  KCValue::errorNUM());
    CHECK_EVAL("GCD(2;-4)",  KCValue::errorNUM());
    CHECK_EVAL("GCD(-2;-4)", KCValue::errorNUM());
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETGCD(18;24)", KCValue(6)); // alternate function name
}

void TestMathFunctions::testGESTEP()
{
    // ODF-tests
    CHECK_EVAL("GESTEP(2;1)",      KCValue(1));     //
    CHECK_EVAL("GESTEP(-1;-2)",    KCValue(1));     // Negative arguments are valid
    CHECK_EVAL("GESTEP(1)",        KCValue(1));     // Second parameter assumed 0 if omitted
    CHECK_EVAL("GESTEP(-2;1)",     KCValue(0));     //
    CHECK_EVAL("GESTEP(3;3)",      KCValue(1));     // KCNumber identical to step value.
    CHECK_EVAL("GESTEP(1.3;1.2)",  KCValue(1));     // Floating point values where X is greater than Step.
    CHECK_EVAL("GESTEP(-2;\"xxx\")", KCValue::errorNUM());   //
    CHECK_EVAL("GESTEP(\"xxx\";-2)", KCValue::errorNUM());   //
}

void TestMathFunctions::testINT()
{
    // ODF-tests
    CHECK_EVAL("=INT(2)",       KCValue(2));
    CHECK_EVAL("=INT(-3)",      KCValue(-3));
    CHECK_EVAL("=INT(1.2)",     KCValue(1));
    CHECK_EVAL("=INT(1.7)",     KCValue(1));
    CHECK_EVAL("=INT(-1.2)",    KCValue(-2));
    CHECK_EVAL("=INT((1/3)*3)", KCValue(1));
}

void TestMathFunctions::testLCM()
{
    CHECK_EVAL("LCM(5;15;25)",   KCValue(75));
    CHECK_EVAL("LCM(2;3)",       KCValue(6));
    CHECK_EVAL("LCM(18;12)",     KCValue(36));
    CHECK_EVAL("LCM(12;18)",     KCValue(36));
    CHECK_EVAL("LCM(12.1;18.1)", KCValue(36));
    CHECK_EVAL("LCM(18.1;12.1)", KCValue(36));
    CHECK_EVAL("LCM(18.9;12.9)", KCValue(36));
    CHECK_EVAL("LCM(7)",         KCValue(7));
    CHECK_EVAL("LCM(5;0)",       KCValue(0));
    CHECK_EVAL("LCM(-2;4)",  KCValue::errorNUM());
    CHECK_EVAL("LCM(2;-4)",  KCValue::errorNUM());
    CHECK_EVAL("LCM(-2;-4)", KCValue::errorNUM());
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETLCM(18;12)", KCValue(36)); // alternate function name
}

void TestMathFunctions::testLN()
{
    // ODF-tests
    CHECK_EVAL("LN(1)",         KCValue(0));                // The logarithm of 1 (in any base) is 0.
    CHECK_EVAL("LN(EXP(1))",    KCValue(1));                // The natural logarithm of e is 1.
    CHECK_EVAL_SHORT("LN(20)",  KCValue(2.995732274));      // TODO expand / Trivial test
    CHECK_EVAL_SHORT("LN(0.2)", KCValue(-1.609437912));     // TODO expand / This tests a value between 0 and 0.5.
    // Values in this domain are valid, but implementations that compute LN(x)
    // by blindly summing the series (1/n)((x-1)/x)^n won't get this value
    // correct, because that series requires x > 0.5.
    CHECK_EVAL("LN(0)",     KCValue::errorNUM());           // The argument must be greater than zero.
    CHECK_EVAL("LN(\"s\")", KCValue::errorNUM());           // The argument must be a number.
}

void TestMathFunctions::testLOG()
{
    CHECK_EVAL("LOG(1;10)",          KCValue(0));
    CHECK_EVAL("LOG(1;EXP(1))",      KCValue(0));
    CHECK_EVAL("LOG(10;10)",         KCValue(1));
    CHECK_EVAL("LOG(EXP(1);EXP(1))", KCValue(1));
    CHECK_EVAL("LOG(10)",            KCValue(1));
    CHECK_EVAL("LOG(8*8*8;8)",       KCValue(3));
    CHECK_EVAL("LOG(0;10)",       KCValue::errorNUM());
    CHECK_EVAL("LOG(\"foo\";10)", KCValue::errorNUM());
    CHECK_EVAL("LOG(2;\"foo\")",  KCValue::errorNUM());
    CHECK_EVAL("LOG(NA();10)",    KCValue::errorNA());
    CHECK_EVAL("LOG(10;NA())",    KCValue::errorNA());
    CHECK_EVAL("LOG(NA();NA())",  KCValue::errorNA());
}

void TestMathFunctions::testLOG10()
{
    CHECK_EVAL("LOG10(1)",   KCValue(0));
    CHECK_EVAL("LOG10(10)",  KCValue(1));
    CHECK_EVAL("LOG10(100)", KCValue(2));
    CHECK_EVAL("LOG10(0)",     KCValue::errorNUM());
    CHECK_EVAL("LOG10(\"H\")", KCValue::errorNUM());
    CHECK_EVAL("LOG10(-2)",    KCValue::errorNUM());
}

void TestMathFunctions::testMDETERM()
{
    CHECK_EVAL("MDETERM({2;4|3;5})", KCValue(-2));
    CHECK_EVAL("MDETERM({2;4})",     KCValue::errorVALUE());
    CHECK_EVAL("MDETERM({2;4|3;6})", KCValue(0));
    CHECK_EVAL("MDETERM(2)",         KCValue(2));
}

void TestMathFunctions::testMINVERSE()
{
    KCValue value(KCValue::Array);
    value.setElement(0, 0, KCValue(-2.5));
    value.setElement(1, 0, KCValue(2.0));
    value.setElement(0, 1, KCValue(1.5));
    value.setElement(1, 1, KCValue(-1.0));
    CHECK_EVAL("MINVERSE({2;4|3;5})", value);                // simply invertible
    value.setElement(0, 0, KCValue(5.0));
    value.setElement(1, 0, KCValue(1.0));
    value.setElement(2, 0, KCValue(-2.0));
    value.setElement(0, 1, KCValue(-1.0));
    value.setElement(1, 1, KCValue(-1.0));
    value.setElement(2, 1, KCValue(1.0));
    value.setElement(0, 1, KCValue(-2.0));
    value.setElement(1, 1, KCValue(1.0));
    value.setElement(2, 1, KCValue(0.0));
    CHECK_EVAL("MINVERSE({1;2;1|2;4;3|3;7;4}", value);       // fails without pivoting
    CHECK_EVAL("MINVERSE({2;4})", KCValue::errorVALUE());      // non-square matrix
    CHECK_EVAL("MINVERSE({2;4|3;6})", KCValue::errorDIV0());   // singular matrix
    CHECK_EVAL("MINVERSE(2)", evaluate("{0.5}"));            // one elementary matrix
}

void TestMathFunctions::testMMULT()
{
    CHECK_EVAL("MMULT({2;4|3;5};{2;4|3;5})", evaluate("{16.0;28.0|21.0;37.0}"));
}

void TestMathFunctions::testMOD()
{
    CHECK_EVAL("MOD(10;3)",      KCValue(1));        // 10/3 has remainder 1.
    CHECK_EVAL("MOD(2;8)",       KCValue(2));        // 2/8 is 0 remainder 2.
    CHECK_EVAL("MOD(5.5;2.5)",   KCValue(0.5));      // The numbers need not be integers.
    CHECK_EVAL("MOD(-2;3)",      KCValue(1));        // The location of the sign matters.
    CHECK_EVAL("MOD(2;-3)",      KCValue(-1));       // The location of the sign matters.
    CHECK_EVAL("MOD(-2;-3)",     KCValue(-2));       // The location of the sign matters.
    CHECK_EVAL("MOD(10;3)",      KCValue(1));        // 10/3 has remainder 1.
    CHECK_EVAL("MOD(10;0)", KCValue::errorDIV0());   // Division by zero is not allowed
}

void TestMathFunctions::testMROUND()
{
    // ODF-tests
    CHECK_EVAL("=MROUND(1564;100)", KCValue(1600));
    CHECK_EVAL("=MROUND(1520;100)", KCValue(1500));
    CHECK_EVAL("=MROUND(1550;100)", KCValue(1600));
    CHECK_EVAL("=MROUND(41.89;8)",  KCValue(40));
    // alternate function name
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETMROUND(1520;100)", KCValue(1500));
}

void TestMathFunctions::testMULTINOMIAL()
{
    // ODF-tests
    CHECK_EVAL("=MULTINOMIAL(3;4;5)", KCValue(27720));
    // alternate function name
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETMULTINOMIAL(3;4;5)", KCValue(27720));
}

void TestMathFunctions::testMUNIT()
{
    CHECK_EVAL("MUNIT(2)", evaluate("{1;0|0;1}"));
    CHECK_EVAL("MUNIT(3)", evaluate("{1;0;0|0;1;0|0;0;1}"));
}

void TestMathFunctions::testODD()
{
    CHECK_EVAL("ODD(5)",    KCValue(5));
    CHECK_EVAL("ODD(-5)",   KCValue(-5));
    CHECK_EVAL("ODD(2)",    KCValue(3));
    CHECK_EVAL("ODD(0.3)",  KCValue(1));
    CHECK_EVAL("ODD(-2)",   KCValue(-3));
    CHECK_EVAL("ODD(-0.3)", KCValue(-1));
    CHECK_EVAL("ODD(0)",    KCValue(1));
}

void TestMathFunctions::testPOWER()
{
    CHECK_EVAL("POWER(10;0)", KCValue(1));       // Anything raised to the 0 power is 1
    CHECK_EVAL("POWER(2;8)" , KCValue(256));     // 2^8 is 256
}

void TestMathFunctions::testPRODUCT()
{
    CHECK_EVAL("PRODUCT(2;3;4)",       KCValue(24));     // Anything raised to the 0 power is 1
    CHECK_EVAL("PRODUCT(TRUE();2;3)" , KCValue(6));      // TRUE() is 1 if inline
    CHECK_EVAL("PRODUCT()",            KCValue(0));      // Product with no parameters returns 0
//TODO
// check inline-values e.g. product(2;3;"2")
}

void TestMathFunctions::testQUOTIENT()
{
    CHECK_EVAL("QUOTIENT(10;5)",     KCValue(2));      //
    CHECK_EVAL("QUOTIENT(14;5)" ,    KCValue(2));      //
    CHECK_EVAL("QUOTIENT(-204;-23)", KCValue(8));      //
    CHECK_EVAL("QUOTIENT(-45;8)",    KCValue(-5));     //
    CHECK_EVAL("QUOTIENT(24;-5)" ,   KCValue(-4));     //
    CHECK_EVAL("QUOTIENT(21;-5)",    KCValue(-4));     //
    CHECK_EVAL("QUOTIENT(-14;5)",    KCValue(-2));     //
    CHECK_EVAL("QUOTIENT(5;0)" ,     KCValue::errorDIV0());   //
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETQUOTIENT(14;5)", KCValue(2)); // alternate function name
}

void TestMathFunctions::testRADIANS()
{
    CHECK_EVAL("RADIANS(180)/PI()", KCValue(1));     // 180 degrees is PI() radians.
}

void TestMathFunctions::testRAND()
{
    CHECK_EVAL("RAND()>=0", KCValue(true));     // The random number must be between 0 and 1.
    CHECK_EVAL("RAND()<=1", KCValue(true));     // The random number must be between 0 and 1.
}

void TestMathFunctions::testRANDBETWEEN()
{
    CHECK_EVAL("RANDBETWEEN(8;8)",      KCValue(8));        // If A=B, return A.
    CHECK_EVAL("RANDBETWEEN(5;15)>=5",  KCValue(true));     // Must return value in range
    CHECK_EVAL("RANDBETWEEN(5;15)<=15", KCValue(true));     // Must return value in range
    CHECK_EVAL("RANDBETWEEN(15;5)>=5",  KCValue(true));     // Must return value in range
    CHECK_EVAL("RANDBETWEEN(15;5)<=15", KCValue(true));     // Must return value in range
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETRANDBETWEEN(8;8)", KCValue(8)); // alternate function name
}

void TestMathFunctions::testROUND()
{
    // ODF-tests
    CHECK_EVAL("=ROUND(10.1;0)",   KCValue(10));
    CHECK_EVAL("=ROUND(9.8;0)",     KCValue(10));
    CHECK_EVAL("=ROUND(0.5;0)",    KCValue(1));
    CHECK_EVAL("=ROUND(1/3;0) ",    KCValue(0));
    CHECK_EVAL("=ROUND(1/3;1)",     KCValue(0.3));
    CHECK_EVAL("=ROUND(1/3;2)",     KCValue(0.33));
    CHECK_EVAL("=ROUND(1/3;2.9)",   KCValue(0.33));
    CHECK_EVAL("=ROUND(5555;-1)",   KCValue(5560));
    CHECK_EVAL("=ROUND(-1.1; 0)",   KCValue(-1));
    CHECK_EVAL("=ROUND(-1.5; 0)",   KCValue(-2));
    CHECK_EVAL("=ROUND(-1.5)",      KCValue(-2));
    CHECK_EVAL("=ROUND(1.1)",       KCValue(1));
    CHECK_EVAL("=ROUND(9.8)",       KCValue(10));
}

void TestMathFunctions::testROUNDDOWN()
{
    // ODF-tests
    CHECK_EVAL("=ROUNDDOWN(1.45673;2)",    KCValue(1.45));
    CHECK_EVAL("=ROUNDDOWN(1;0)",           KCValue(1));
    CHECK_EVAL("=ROUNDDOWN(1)",             KCValue(1));
    CHECK_EVAL("=ROUNDDOWN(9;-1)",          KCValue(0));
    CHECK_EVAL("=ROUNDDOWN(-9;-1)",          KCValue(0));
    CHECK_EVAL("=ROUNDDOWN(9;0)",           KCValue(9));
    CHECK_EVAL("=ROUNDDOWN(-1.1)",          KCValue(-1));
    CHECK_EVAL("=ROUNDDOWN(-1.9)",          KCValue(-1));
}

void TestMathFunctions::testROUNDUP()
{
    // ODF-tests
    CHECK_EVAL("=ROUNDUP(1.45673;2)",  KCValue(1.46));
    CHECK_EVAL("=ROUNDUP(1.1;0)",       KCValue(2));
    CHECK_EVAL("=ROUNDUP(1.9;0)",       KCValue(2));
    CHECK_EVAL("=ROUNDUP(1)",           KCValue(1));
    CHECK_EVAL("=ROUNDUP(9;-1)",        KCValue(10));
    CHECK_EVAL("=ROUNDUP(-9;-1)",        KCValue(-10));
    CHECK_EVAL("=ROUNDUP(9;0)",         KCValue(9));
    CHECK_EVAL("=ROUNDUP(-1.1)",        KCValue(-2));
    CHECK_EVAL("=ROUNDUP(-1.9)",        KCValue(-2));
}

void TestMathFunctions::testSERIESSUM()
{
    CHECK_EVAL("SERIESSUM(2;0;2;{1;2})",           KCValue(9));            //
    CHECK_EVAL("SERIESSUM(2;0;2;{1;2;3;4})",       KCValue(313));          //
    CHECK_EVAL("SERIESSUM(2;0;2;{1;2;3;4;5;6;7})", KCValue(36409));        //
    CHECK_EVAL("SERIESSUM(2;2;2;{1;6;5;4;3;2;7})", KCValue(127396));       //
    CHECK_EVAL("SERIESSUM(3;0;2;{1;2;3;4})",       KCValue(3178));         //
    CHECK_EVAL("SERIESSUM(\"error\";0;2;{1;2})",   KCValue::errorNUM());   // Text is not allowed
}

void TestMathFunctions::testSIGN()
{
    CHECK_EVAL("SIGN(-4)", KCValue(-1));     // N < 0 returns -1
    CHECK_EVAL("SIGN(4)",  KCValue(1));      // N > 0 returns +1
    CHECK_EVAL("SIGN(0)",  KCValue(0));      // N == 0 returns 0
}

void TestMathFunctions::testSQRT()
{
    CHECK_EVAL("SQRT(4)",  KCValue(2));            // The square root of 4 is 2.
    CHECK_EVAL("SQRT(-4)", KCValue::errorNUM());   // N > 0 returns +1
}

void TestMathFunctions::testSQRTPI()
{
    CHECK_EVAL_SHORT("SQRTPI(1)",  KCValue(1.77245385));       // TODO more digits / The square root of PI
    CHECK_EVAL("SQRTPI(2)",  KCValue(2.5066282746));     // The square root of 2PI
    CHECK_EVAL("SQRTPI(-4)", KCValue::errorNUM());       // The argument must be non-negative
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETSQRTPI(2)", KCValue(2.5066282746)); // alternate function name
}

void TestMathFunctions::testSUBTOTAL()
{
    CHECK_EVAL("SUBTOTAL(1;7)", KCValue(7));     // Average.
    CHECK_EVAL("SUBTOTAL(2;8)", KCValue(1));     // Count.
    CHECK_EVAL("SUBTOTAL(3;11)", KCValue(1));     // Count.
    CHECK_EVAL("SUBTOTAL(11;33)", KCValue(0));
    CHECK_EVAL("SUBTOTAL(12;33)", KCValue::errorVALUE());
    CHECK_EVAL("SUBTOTAL(102;8)", KCValue(1)); // Count.
    CHECK_EVAL("SUBTOTAL(111;33)", KCValue(0)); // Average.
    CHECK_EVAL("SUBTOTAL(1111;33)", KCValue(0)); // Average.
}

void TestMathFunctions::testSUMA()
{
    CHECK_EVAL("SUMA(1;2;3)",      KCValue(6));     // Simple sum.
    CHECK_EVAL("SUMA(TRUE();2;3)", KCValue(6));     // TRUE() is 1.
}

void TestMathFunctions::testSUMIF()
{
    // B3 = 7
    // B4 = 2
    // B5 = 3
    CHECK_EVAL("SUMIF(B4:B5;\">2.5\")",    KCValue(3));     // B4 is 2 and B5 is 3, so only B5 has a value greater than 2.5.
    CHECK_EVAL("SUMIF(B3:B5;B4)",          KCValue(2));     // Test if a cell equals the value in B4.
    CHECK_EVAL("SUMIF("";B4)",      KCValue::errorNUM());   // Constant values are not allowed for the range.
    CHECK_EVAL("SUMIF(B3:B4;\"7\";B4:B5)", KCValue(2));     // B3 is the string "7", but its match is mapped to B4 for the summation.
    CHECK_EVAL("SUMIF(B3:B10;1+1)",        KCValue(2));     // The criteria can be an expression.
    CHECK_EVAL("SUMIF(B3:B4;\"7\")",       KCValue(0));     // TODO B3 is the string "7", but only numbers are summed.
}

void TestMathFunctions::testSUMSQ()
{
    CHECK_EVAL("SUMSQ(1;2;3)",      KCValue(14));     // Simple sum.
    CHECK_EVAL("SUMSQ(TRUE();2;3)", KCValue(14));     // TRUE() is 1.
    CHECK_EVAL("SUMSQ(B4:B5)",      KCValue(13));     // 2*2+3*3 is 13.
}

void TestMathFunctions::testTRUNC()
{
    // ODF-tests
    CHECK_EVAL("=TRUNC(10.1)",     KCValue(10));
    CHECK_EVAL("=TRUNC(0.5)",      KCValue(0));
    CHECK_EVAL("=TRUNC(1/3;0)",    KCValue(0));
    CHECK_EVAL("=TRUNC(1/3;1)",    KCValue(0.3));
    CHECK_EVAL("=TRUNC(1/3;2)",    KCValue(0.33));
    CHECK_EVAL("=TRUNC(1/3;2.9)",  KCValue(0.33));
    CHECK_EVAL("=TRUNC(5555;-1)",  KCValue(5550));
    CHECK_EVAL("=TRUNC(-1.1)",     KCValue(-1));
    CHECK_EVAL("=TRUNC(-1.5)",     KCValue(-1));
}

QTEST_KDEMAIN(TestMathFunctions, GUI)

#include "TestMathFunctions.moc"
