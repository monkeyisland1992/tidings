#include "htmlsed.h"

#include <QUrl>
#include <QDebug>

namespace
{

const QRegExp RE_TAG("<[^!>][^>]*>");
const QRegExp RE_TAG_NAME("[a-zA-Z0-9]+[\\s/>]");
const QRegExp RE_TAG_ATTRIBUTE("[a-zA-Z0-9]+\\s*=\\s*(\"[^\"]*\"|'[^']*'|[^\\s\"']*)");



/* Finds and returns the next tag in the given HTML string.
 * HTML comments (<!-- -->) are recognized as tags.
 * pos will be set to the start position of the tag in the string.
 */
QString findTag(const QString& html, int& pos)
{
    QRegExp tag(RE_TAG);
    int tagPos = tag.indexIn(html);
    int commentBeginPos = html.indexOf("<!--");

    if (commentBeginPos != -1 && commentBeginPos < tagPos)
    {
        int commentEndPos = html.indexOf("-->", commentBeginPos);
        if (commentEndPos != -1)
        {
            pos = commentBeginPos;
            int length = commentEndPos + 3 - commentBeginPos;
            return html.mid(commentBeginPos, length);
        }
        else
        {
            return QString();
        }
    }
    else
    {
        if (tagPos != -1)
        {
            pos = tagPos;
            int length = tag.matchedLength();
            return html.mid(pos, length);
        }
        else
        {
            return QString();
        }
    }
}

}


HtmlSed::Tag::Tag(const QString& data)
    : myIsOpening(false)
    , myIsClosing(false)
    , myIsReplaced(false)
{
    //qDebug() << "TAG" << data;

    if (data.trimmed().startsWith("</"))
    {
        myIsClosing = true;
    }
    else
    {
        myIsOpening = true;
    }
    if (data.trimmed().endsWith("/>"))
    {
        myIsClosing = true;
    }


    QRegExp tagName(RE_TAG_NAME);
    int pos = tagName.indexIn(data);

    if (pos == -1)
    {
        return;
    }

    int length = tagName.matchedLength();
    myName = data.mid(pos, tagName.matchedLength() - 1).trimmed().toUpper();
    //qDebug() << "NAME" << myName;

    int offset = pos + length;

    QRegExp tagAttribute(RE_TAG_ATTRIBUTE);
    while (true)
    {
        int attrPos = tagAttribute.indexIn(data.mid(offset));
        if (attrPos == -1)
        {
            break;
        }
        const QString attr = data.mid(offset + attrPos,
                                      tagAttribute.matchedLength());
        if (attr.isEmpty())
        {
            break;
        }
        offset += attrPos + tagAttribute.matchedLength();
        //qDebug() << "ATTR" << attr;

        int splitPos = attr.indexOf('=');
        if (splitPos != -1)
        {
            const QString attrName = attr.left(splitPos).trimmed().toUpper();
            QString attrValue = attr.mid(splitPos + 1).trimmed();
            if (attrValue.startsWith("'") || attrValue.startsWith("\""))
            {
                attrValue = attrValue.mid(1, attrValue.size() - 2);
            }
            myAttributes[attrName] = attrValue;
        }
    }
}

QString HtmlSed::Tag::toString() const
{
    if (myIsReplaced)
    {
        return myBeforeText + myReplaceWith + myAfterText;
    }

    QString out = "<";
    if (! myIsOpening && myIsClosing)
    {
        out += "/";
    }
    out += myName;

    foreach (const QString& attr, myAttributes.keys())
    {
        out += " " + attr + "=\"" + myAttributes.value(attr) + "\"";
    }

    if (myIsOpening && myIsClosing)
    {
        out += "/";
    }
    out += ">";

    return myBeforeText + out + myAfterText;
}




void HtmlSed::Rule::replaceTag(const QString& t,
                               const QString& w,
                               bool o,
                               bool c)
{
    mode = REPLACE_TAG;
    tag = t;
    replaceWith = w;
    openingTag = o;
    closingTag = c;
}

void HtmlSed::Rule::replaceAttribute(const QString& t,
                                     const QString& a,
                                     const QRegExp& r,
                                     const QString& w)
{
    mode = REPLACE_ATTRIBUTE;
    tag = t;
    attribute = a;
    regExp = r;
    replaceWith = w;
}

void HtmlSed::Rule::replaceContents(const QString& t,
                                    const QRegExp& r,
                                    const QString& w)
{
    mode = REPLACE_CONTENTS;
    tag = t;
    regExp = r;
    replaceWith = w;
}

void HtmlSed::Rule::resolveUrl(const QString& t,
                               const QString& a,
                               const QString& u)
{
    mode = RESOLVE_URL;
    tag = t;
    attribute = a;
    resolveBaseUrl = u;
}

void HtmlSed::Rule::surroundTag(const QString& t,
                                const QString& b,
                                const QString& a,
                                bool o,
                                bool c)
{
    mode = SURROUND_TAG;
    tag = t;
    replaceWith = b;
    replaceWithAfter = a;
    openingTag = o;
    closingTag = c;
}

void HtmlSed::Rule::modifyTag(const QString& t,
                              Modifier* m)
{
    mode = MODIFY_TAG;
    tag = t;
    tagModifier = m;
}



HtmlSed::HtmlSed(const QString& html)
    : myHtml(html)
{

}

void HtmlSed::addRule(const QString& tag, const HtmlSed::Rule& rule)
{
    if (! myRuleSet.contains(tag))
    {
        myRuleSet[tag] = QList<Rule>();
    }
    myRuleSet[tag] << rule;
}

void HtmlSed::replaceTag(const QString& tagToReplace,
                         const QString& replaceWith,
                         bool openingTag,
                         bool closingTag)
{
    Rule rule;
    rule.replaceTag(tagToReplace,
                    replaceWith,
                    openingTag,
                    closingTag);
    addRule(tagToReplace, rule);
}

void HtmlSed::replaceAttribute(const QString& tagToReplace,
                               const QString& attributeToReplace,
                               const QString& regExp,
                               const QString& replaceWith)
{
    Rule rule;
    rule.replaceAttribute(tagToReplace,
                          attributeToReplace,
                          QRegExp(regExp),
                          replaceWith);
    addRule(tagToReplace, rule);
}

void HtmlSed::replaceContents(const QString& enclosingTag,
                              const QString& regExp,
                              const QString& replaceWith)
{
    Rule rule;
    rule.replaceContents(enclosingTag,
                         QRegExp(regExp),
                         replaceWith);
    addRule(enclosingTag, rule);
}

void HtmlSed::surroundTag(const QString& tag,
                          const QString& before,
                          const QString& after,
                          bool openingTag,
                          bool closingTag)
{
    Rule rule;
    rule.surroundTag(tag,
                     before,
                     after,
                     openingTag,
                     closingTag);
    addRule(tag, rule);
}

void HtmlSed::resolveUrl(const QString& tagToResolve,
                         const QString& attributeToResolve,
                         const QString& baseUrl)
{
    Rule rule;
    rule.resolveUrl(tagToResolve,
                    attributeToResolve,
                    baseUrl);
    addRule(tagToResolve, rule);
}

void HtmlSed::modifyTag(const QString& tag,
                        Modifier* modifier)
{
    Rule rule;
    rule.modifyTag(tag,
                   modifier);
    addRule(tag, rule);
}

QString HtmlSed::toString() const
{
    QString html = myHtml;

    int offset = 0;
    int tagPosition = 0;

    QList<QPair<QString, int> > contentStack;

    Tag previousTag("");

    while (true)
    {
        // find the next tag, and the PCDATA up to that tag
        const QString tagData = findTag(html.mid(offset), tagPosition);
        //const QString pcData = html.mid(offset, tagPosition);

        //qDebug() << "DATA" << tagData;

        if (tagData.isEmpty())
        {
            break;
        }

        if (tagData.startsWith("<!--"))
        {
            offset += tagData.size();
            continue;
        }

        Tag tag(tagData);

        // try to detect and ignore false tags
        if (previousTag.name() == "SCRIPT" &&
            ! previousTag.isClosing() &&
            (tag.name() != "SCRIPT" || ! tag.isClosing()))
        {
            offset += tagData.size();
            continue;
        }


        // apply the rules
        if (myRuleSet.contains(tag.name()) || myRuleSet.contains(QString()))
        {
            QList<Rule> rules = myRuleSet.value(tag.name(),
                                                         QList<Rule>()) +
                                         myRuleSet.value(QString(),
                                                         QList<Rule>());

            foreach (const Rule& rule, rules)
            {
                if (rule.mode == Rule::REPLACE_CONTENTS)
                {
                    // replace contents
                    if (tag.isOpening())
                    {
                        //qDebug() << "begin content at" << tag.name() << (offset + tagPosition);
                        contentStack << QPair<QString, int>(tag.name(),
                                                            offset + tagPosition + tag.toString().size());
                    }
                    if (tag.isClosing() &&
                        contentStack.size() &&
                        contentStack.last().first == tag.name())
                    {
                        //qDebug() << "end content at" << tag.name() << (offset + tagPosition);
                        QPair<QString, int> item = contentStack.takeLast();
                        QString contents = html.mid(item.second,
                                                    offset + tagPosition - item.second);
                        //qDebug() << "contents" << contents;
                        contents.replace(rule.regExp,
                                         rule.replaceWith);
                        //qDebug() << "replaced contents" << contents;

                        //qDebug() << "html at" << html.mid(item.second, offset + tagPosition - item.second);
                        html.replace(item.second,
                                     offset + tagPosition - item.second,
                                     contents);
                        offset = item.second + contents.size();
                        tagPosition = 0;
                    }
                    break;
                }
                else if (rule.mode == Rule::RESOLVE_URL)
                {
                    // resolve URL
                    QString url = tag.attribute(rule.attribute);
                    if (url.startsWith("http://") ||
                        url.startsWith("https://"))
                    {
                        // do nothing
                        break;
                    }
                    else
                    {
                        tag.setAttribute(rule.attribute,
                                         QUrl(rule.resolveBaseUrl)
                                         .resolved(url)
                                         .toString());
                    }
                }
                else if (rule.mode == Rule::REPLACE_ATTRIBUTE)
                {
                    // replace tag attribute
                    if (tag.hasAttribute(rule.attribute))
                    {
                        QString value = tag.attribute(rule.attribute);
                        tag.setAttribute(rule.attribute,
                                         value.replace(rule.regExp,
                                                       rule.replaceWith));
                    }
                }
                else if (rule.mode == Rule::REPLACE_TAG)
                {
                    // replace tag
                    if (rule.openingTag && tag.isOpening() ||
                        rule.closingTag && tag.isClosing())
                    {
                        tag.replaceWith(rule.replaceWith);
                    }
                }
                else if (rule.mode == Rule::SURROUND_TAG)
                {
                    if (rule.openingTag && tag.isOpening() ||
                        rule.closingTag && tag.isClosing())
                    {
                        tag.setSurroundings(rule.replaceWith,
                                            rule.replaceWithAfter);
                    }
                }
                else if (rule.mode == Rule::MODIFY_TAG)
                {
                    rule.tagModifier->modifyTag(tag);
                }

            }//foreach rule


        }//if in rule set

        const QString newTagData = tag.toString();
        html.replace(offset + tagPosition,
                     tagData.size(),
                     newTagData);
        offset += tagPosition + newTagData.size();

        previousTag = tag;

    }//while

    return html;
}
