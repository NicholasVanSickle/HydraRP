#include <QtTest/qtest.h>
#include <QDebug>

//#define BOOST_SPIRIT_DEBUG
#include <lib/PropertyParser.h>
using namespace HydraParser;

class PropertyMap : public PropertyContainer
{
public:
    PropertyMap(){}
    QVariantMap map;

    // PropertyContainer interface
    QStringList properties()
    {
        return map.keys();
    }
    QVariant property(QString propertyName)
    {
        return map.value(propertyName);
    }
    void setProperty(QString propertyName, QVariant value)
    {
        map[propertyName] = value;
    }
};

QVariant parseValue(QString s)
{
    return parse(s, nullptr).value();
}

class TestParser : public QObject
{
    Q_OBJECT

private slots:
    void testParseLiterals()
    {
        auto parseValue = [](QString text)->QVariant{return HydraParser::parse(text, nullptr).value();};
        QCOMPARE(parseValue("\"BASIC STRING\"").toString(), QString("BASIC STRING"));
        QCOMPARE(parseValue("\"IT'S A \\\"STRING\\\"\"").toString(), QString("IT'S A \"STRING\""));
        QCOMPARE(parseValue("'ALSO A STRING'").toString(), QString("ALSO A STRING"));
        QCOMPARE(parseValue("555").toInt(), 555);
        QCOMPARE(parseValue("3.14159").toDouble(), 3.14159);
        QCOMPARE(parseValue("true").toBool(), true);
        QCOMPARE(parseValue("false").toBool(), false);
    }

    void testParseOperators()
    {
        auto parseValue = [](QString text)->QVariant{return HydraParser::parse(text, nullptr).value();};
        QCOMPARE(parseValue("2+2").toInt(), 4);
        QCOMPARE(parseValue("2+2+2").toInt(), 6);
        QCOMPARE(parseValue("2-2").toInt(), 0);
        QCOMPARE(parseValue("2-3").toInt(), -1);
        QCOMPARE(parseValue("2 + 2").toInt(), 4);
        QCOMPARE(parseValue("3 + 3 + 2").toInt(), 8);
        QCOMPARE(parseValue("3 + 3 * 2").toInt(), 9);
        QCOMPARE(parseValue("1 / 0"), QVariant());
        QCOMPARE(parseValue("(2*(3-3)").toInt(), 0);
        QCOMPARE(parseValue("2 * 2.5").toDouble(), 5.0);
        QCOMPARE(parseValue("2.4 / 0.5").toDouble(), 4.8);
        QCOMPARE(parseValue("\"STRING1 \" + \"STRING2\"").toString(), QString("STRING1 STRING2"));
        QCOMPARE(parseValue("2+2+\"3\"").toInt(), 7);
        QCOMPARE(parseValue("\"2\"+\"2\"").toString(), QString("22"));
        QCOMPARE(parseValue("2**3").toInt(), 8);
        QCOMPARE(parseValue("2.0**-1").toDouble(), 0.5);
    }

    void testParseSymbols()
    {
        PropertyMap pmap;
        QVariantMap& map = pmap.map;
        auto parseValue = [&pmap](QString text)->QVariant{return HydraParser::parse(text, &pmap).value();};
        map["FOO"] = 2;
        QCOMPARE(parseValue("FOO").toInt(), 2);
        QCOMPARE(parseValue("Foo"), QVariant());
        QCOMPARE(parseValue("3+FOO").toInt(), 5);
        QCOMPARE(parseValue("FOO * FOO").toInt(), 4);
        qDebug() << parseValue("FUNCTION FOO");
        qDebug() << parseValue("1, 'FOO', 2.5");
    }
};


QTEST_APPLESS_MAIN(TestParser)

#include "tst_parser.moc"
