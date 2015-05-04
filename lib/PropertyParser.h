#pragma once

#define BOOST_RESULT_OF_USE_DECLTYPE
#define BOOST_SPIRIT_USE_PHOENIX_V3
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include <functional>

#include <QString>
#include <QVariant>
#include <QDebug>

#include <lib/PropertyTable.h>

namespace HydraParser {

class Value
{
public:
    enum ValueType
    {
        eInvalid,
        eVariantType,
        eSymbol
    };

    Value() {}
    Value(std::string value) : m_value(QString::fromStdString(value)) {}
    Value(QVariant value) : m_value(value) {}

    Value& operator=(const Value& other) { m_value = other.value(); return *this; }
    Value& operator=(Value&& other) { m_value = std::move(other.m_value); return *this; }
    Value& operator=(const std::vector<Value>& list)
    {
        if (list.size() == 1)
        {
            m_value = list.front().value();
        }
        else
        {
            QVariantList values;
            for (auto it = list.begin(); it != list.end(); ++it)
                values.push_back((*it).value());
            m_value = values;
        }
        return *this;
    }

    Value(const Value& value) { operator=(value); }
    Value(Value&& value) { operator=(value); }
    explicit Value(const std::vector<Value>& list) { operator=(list); }

    static QList<QVariant::Type> typePriority()
    {
        static QList<QVariant::Type> priority;
        if (priority.isEmpty())
            priority << QVariant::Bool << QVariant::Double << QVariant::Int << QVariant::String;
        return priority;
    }

    template <class T>
    Value binaryOperatorByType(const Value& other, QString op)
    {
        T lhs = value().value<T>();
        T rhs = other.value().value<T>();
        if (op == "+")
            return lhs + rhs;
        if (op == "-")
            return lhs - rhs;
        if (op == "*")
            return lhs * rhs;
        if (op == "/" && rhs != 0)
            return lhs / rhs;
        if (op == "**")
            return pow(lhs, rhs);
        return QVariant();
    }

    template <>
    Value binaryOperatorByType<QString>(const Value& other, QString op)
    {
        QString lhs = value().toString();
        QString rhs = other.value().toString();
        if (op == "+")
            return lhs + rhs;
        return QVariant();
    }

    Value binaryOperator(const Value& other, QString op)
    {
        QVariant::Type target = QVariant::Invalid;
        foreach (auto type, typePriority())
        {
            if (type == QVariant::Bool)
                continue;
            if (op != "+" && type == QVariant::String)
                continue;

            if (value().type() == type && other.value().canConvert(type) ||
                    other.value().type() == type && value().canConvert(type))
            {
                target = type;
                break;
            }
        }

        switch(target)
        {
        case QVariant::Double:
            return binaryOperatorByType<double>(other, op);
        case QVariant::Int:
            return binaryOperatorByType<int>(other, op);
        case QVariant::String:
            return binaryOperatorByType<QString>(other, op);
        }
        return QVariant();
    }

    Value executeFunction(const Value& other)
    {
        return QVariant("FUNCTION");
    }

    inline QVariant value() const { return m_value; }

    QString toString() const
    {
        if (!m_value.isValid())
            return "Undefined";
        return m_value.toString();
    }

private:
    QVariant m_value;
};

std::ostream& operator<<(std::ostream& stream, const Value& value)
{
    stream << value.toString().toStdString();
    return stream;
}

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

template <typename Iterator>
struct HydraGrammar : qi::grammar<Iterator, Value(), ascii::space_type>
{
    qi::rule<Iterator, std::string(std::string)> stringContents;
    qi::rule<Iterator, std::string(), qi::locals<std::string> > stringLiteral;
    qi::rule<Iterator, bool()> boolLiteral;
    qi::rule<Iterator, Value()> literal;
    qi::rule<Iterator, Value(), qi::locals<std::string>> symbol;
    qi::rule<Iterator, Value()> value;
    qi::rule<Iterator, Value(), qi::locals<Value,Value>, ascii::space_type> expression;
    qi::rule<Iterator, std::vector<Value>(), ascii::space_type> tuple;
    qi::rule<Iterator, Value(), ascii::space_type> addsub;
    qi::rule<Iterator, Value(), ascii::space_type> term;
    qi::rule<Iterator, Value(), ascii::space_type> exponent;
    qi::rule<Iterator, Value(), ascii::space_type> factor;
    qi::rule<Iterator, Value(), ascii::space_type> statement;

    HydraGrammar(PropertyContainer* context) : HydraGrammar::base_type(statement)
    {
        using namespace qi;
        using namespace phoenix;

        auto binaryOp = [](QString op)
        { return (_val = bind(&Value::binaryOperator, _val, _1, op)); };
        auto getProperty = bind([context](std::string name)->Value
        {return context ? context->property(QString::fromStdString(name)) : QVariant();}, _a);
        auto callFunction = bind(&Value::executeFunction, _a, _b);

        stringContents = ( lit("\\") >> ascii::string(_r1))
                         | (ascii::char_ - ascii::string(_r1));
        stringLiteral =  ( lit("\"")[_a = "\""]
                         | lit("'")[_a = '\''])
                         [ _val = ""] >> +stringContents(_a)[_val += _1] > lit(_a);
        boolLiteral =    lit("false")[_val = false] | lit("true")[_val = true];
        literal =        lexeme[ stringLiteral
                               | boolLiteral
                               | (int_ >> !lit("."))
                               | double_];
        symbol =         lexeme[(ascii::alpha[_a = _1] | ascii::string("_")[_a = '_'])
                         >> *(ascii::alnum[_a += _1] | ascii::string("_")[_a+='_'])]
                         [_val = getProperty];
        value =          literal | symbol;
        expression =     ( tuple[_a = _1] >> expression[_b = _1])[_val = callFunction]
                         | tuple[_val = _1];
        tuple =          addsub % ',';
        addsub =         term [_val = _1] >>     *('+' >> term     [binaryOp("+")]
                                                  |'-' >> term     [binaryOp("-")]);
        term =           exponent [_val = _1] >> *('*' >> exponent [binaryOp("*")]
                                                  |'/' >> exponent [binaryOp("/")]);
        exponent =       factor [_val = _1] >>   *("**" >> factor  [binaryOp("**")]);
        factor =         value [_val = _1]
                         | '(' >> expression [_val = _1] >> ')';
        statement =      expression >> eoi;

//        BOOST_SPIRIT_DEBUG_NODE(stringContents);
//        BOOST_SPIRIT_DEBUG_NODE(stringLiteral);
//        BOOST_SPIRIT_DEBUG_NODE(literal);
//        BOOST_SPIRIT_DEBUG_NODE(expression);
//        BOOST_SPIRIT_DEBUG_NODE(statement);
    }
};

Value parse(QString value, PropertyContainer* context)
{
    //    auto op = phoenix::bind(onParse, qi::_1);
    //    auto parser = qi::double_[op] >> *(',' >> qi::double_[op]);
    HydraGrammar<std::string::const_iterator> parser(context);
    auto emptyParser = ascii::space;

    auto stdString = value.toStdString();
    auto first=  stdString.begin();
    auto last = stdString.end();

    Value result;

    bool r = qi::phrase_parse(
                first,
                last,
                parser,
                emptyParser,
                result);
    if (!r || first != last) // fail if we did not get a full match
        return QVariant();
    return result;
}
}
