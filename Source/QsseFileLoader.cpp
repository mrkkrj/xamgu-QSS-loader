/** @file   QsseFileLoader.cpp
   @author  Marek Krajewski (mrkkrj), www.ib-krajewski.de
   @date    28.09.2020

   @brief Implementation of the QsseFileLoader class.

   @copyright  Copyright 2020, Marek Krajewski, released under the terms of LGPL v3 
*/

#include "QsseFileLoader.h"

#ifdef QSSLD_LOGGING_ENABLED
#  include "Logging/Logger.h"
#else
#include <QDebug>
#endif

#include <QtCore/QDir>
#include <QtCore/QRegularExpression>


// OPEN TODO::: enable configurable logging???
#ifndef QSSLD_LOGGING_ENABLED
//#  define QTUTILS_LOG_TRACE(theme, a) do {} while (0)
//#  define QTUTILS_LOG_INFO(theme, a) do {} while (0)
////#  define QTUTILS_LOG_ERROR(type, theme, a) do {} while (0) --> OPEN TODO:::
//#  define QTUTILS_LOG_ERROR(theme, a) do {} while (0)

// minimal fallback
#  define QTUTILS_LOG_TRACE(theme, a) do { qDebug() << " [qss_ld]" << a; } while (0)
#  define QTUTILS_LOG_INFO(theme, a) do  { qDebug() << " [qss_ld] INFO:" << a; } while (0)
#  define QTUTILS_LOG_ERROR(theme, a) do { qDebug() << " [qss_ld] ERROR:" << a; } while (0)

#endif




using namespace ibkrj::xamgu;


namespace {
   
    // impl. constants
    const QString c_defaultResourcePath = ":/StyleSheets/";

    const QRegularExpression c_commentRegex(
             QStringLiteral("/\\*(?>(?:(?>[^*]+)|\\*(?!/))*)\\*/"), QRegularExpression::MultilineOption);

    const QRegularExpression c_trailingWhitespcRegex(QStringLiteral("\\s+$"));

    const QRegularExpression c_variableNameRegex(QStringLiteral("\\$[A-Za-z](?:-?[0-9A-Za-z]+)*"));

    const QRegularExpression c_variableDefRegex(
             QStringLiteral("^(\\s*)(%1)\\s*:\\s*(.+?)\\s*;\\s*").arg(c_variableNameRegex.pattern()));

    const QRegularExpression c_importClauseRegex(
             QStringLiteral("^(\\s*)@import\\s*%1\\s*(;?)\\s*").arg("([\"'])((?:(?!\\%1).)*)\\2"));
   

    // impl. helper
    QString strgShift(QString& oldStrg, const QString& newStrg)
    {
       QString tmp = oldStrg;
       oldStrg = newStrg;

       return tmp;
    }

}


// public methods

QsseFileLoader::QsseFileLoader(const QString& stylesheetsDir)
    : m_stylesheetsDir(stylesheetsDir.isEmpty() 
                         ? c_defaultResourcePath 
                         : stylesheetsDir)
{
}


QString QsseFileLoader::loadStyleSheet(const QString& fileToLoad)
{
    Q_ASSERT(m_filePathList.isEmpty());
    Q_ASSERT(m_directoryPathList.isEmpty());
    Q_ASSERT(m_parseContext.isEmpty());

    m_processedFiles.clear();
    m_errorMessages.clear();

    if (fileToLoad.isEmpty())
    {
        reportStyleFileError("", "will be ignored - empty style sheet file name.");
        return QString();
    }

    traceFileInfo("loading style sheet file ", fileToLoad);

    return processFile(fileToLoad);
}


QString QsseFileLoader::lookupVariable(const QString& name) const
{
    Q_ASSERT(name.startsWith(QChar('$')));

    const auto pos = m_variableDefs.find(name);
    
    if (pos != m_variableDefs.end())
    {
        return pos.value();
    }
    else
    {
        reportVariableError(name, "is not defined.");
        return QString();
    }    
}


void QsseFileLoader::createVariable(const QString& name, const QString& value)
{
    Q_ASSERT(name.startsWith(QChar('$')));

    const auto pos = m_variableDefs.find(name);
    
    if (pos == m_variableDefs.end())
    {
        m_variableDefs[name] = value;
    }
    else
    {
        reportVariableError(name, "is already defined.");
    }
}


const QStringList& QsseFileLoader::messages() const
{
    return m_errorMessages;
}


// private methods

QString QsseFileLoader::processFile(const QString& fileToLoad)
{
    QString filePath;
    QString currentDirPath;

    if (!getEffectiveFilePath(fileToLoad, filePath, &currentDirPath))
    {
        return QString();
    }

    Q_ASSERT(!filePath.isEmpty());
    Q_ASSERT(!currentDirPath.isEmpty()); // it's a canonical path!

    if (alreadyProcessed(filePath))
    {
       return QString();
    }

    // else: load it (recursively!)
    m_processedFiles.insert(filePath);

    m_filePathList.append(filePath);
    m_directoryPathList.append(currentDirPath);

    QString rawContents;
    QString contents;

    if (getFileContents(filePath, rawContents))
    {
       contents = processFileContents(rawContents);
    }

    m_filePathList.removeLast();
    m_directoryPathList.removeLast();

    return contents;
}


QString QsseFileLoader::processFileContents(const QString& contents)
{
    // 1. clean up
    QString input = removeComments(contents);
    
    // - helpers: line reader & writer
    QTextStream istrm(&input, QIODevice::ReadOnly);    
    QString output;
    QTextStream ostrm(&output, QIODevice::WriteOnly);    

    // 2. iterate over lines
    while (!istrm.atEnd())
    {
        if (processLine(istrm.readLine(), ostrm))
        {
            ostrm << QChar('\n');
        }
    }

    return output;
}


QString QsseFileLoader::removeComments(const QString& contents) const
{
    return QString(contents).remove(c_commentRegex);
}


QString QsseFileLoader::replaceVariables(const QString& strg) const
{
    QString cleanedStrg = QString(strg).remove(c_trailingWhitespcRegex);

    if (cleanedStrg.isEmpty())
    {
        return cleanedStrg;
    }

    while (!cleanedStrg.trimmed().isEmpty())
    {
        if (!substVariableName(cleanedStrg))
        {
            // replaced string does not contain any (further) identifiers
            return cleanedStrg;
        }
    }

    // replaced everything away
    return QString();
}


bool QsseFileLoader::getEffectiveFilePath(const QString& fileToLoad, QString& effectiveFilePath, QString* efectiveLoadDir)
{
    Q_ASSERT(!fileToLoad.isEmpty());

    // fileToLoad with a relative or an absolute path?
    QFileInfo finfo(fileToLoad);
        
    if (finfo.isRelative() &&
         !m_directoryPathList.isEmpty())
    {
        // extendt to absolute path!
        auto lastDir = m_directoryPathList.last();
        finfo.setFile(QDir(lastDir), fileToLoad);
    }

    if (efectiveLoadDir != nullptr)
    {
        QFileInfo dirInfo(finfo.path());
        *efectiveLoadDir = dirInfo.canonicalFilePath();
    }

    if (!m_stylesheetsDir.isEmpty())
    {
        QFileInfo infoSsDir(QDir(m_stylesheetsDir), finfo.fileName());

        if (infoSsDir.exists())
        {
            effectiveFilePath = infoSsDir.canonicalFilePath();

            traceFileInfo("style sheet file resolved in the default path: ", effectiveFilePath);
            return true;
        }
    }

    if (finfo.exists())
    {
        effectiveFilePath = finfo.canonicalFilePath();

        traceFileInfo("style sheet file resolved in a custom path: ", effectiveFilePath);
        return true;
    }
    else
    {
        // report the path we were looking up
        reportStyleFileError(finfo.absoluteFilePath(), "does not exist.");
        return false;
    }
}


bool QsseFileLoader::alreadyProcessed(const QString& filePath)
{
   if (m_filePathList.contains(filePath))
   {
      reportStyleFileError(filePath, "cannot be processed recursively.");
      return true;
   }

   if (m_processedFiles.contains(filePath))
   {
      reportStyleFileError(filePath, "already processed.");
      return true;
   }

   // not processed!
   return false;
}


bool QsseFileLoader::getFileContents(const QString& filePath, QString& fileContents)
{
   QFile file(filePath);

   if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
   {
      reportStyleFileError(filePath, "couldn't be opened.");
      return false;
   }

   if (file.size() > maxAllowedFileSize())
   {
      reportStyleFileError(filePath, QString("exceeds limit of %L2 bytes.").arg(maxAllowedFileSize()));
      return false;
   }

   traceFileInfo("processing style sheet file ", filePath);

   fileContents = file.readAll();
   file.close();

   return true;
}


bool QsseFileLoader::processLine(const QString& line, QTextStream& out)
{
    QString cleanedLine = QString(line).remove(c_trailingWhitespcRegex);

    if (cleanedLine.isEmpty())
    {
        return true;
    }

    // 1. need to import sth?
    while (!cleanedLine.trimmed().isEmpty())
    {
        QString importedContents;

        if (!processImportClause(cleanedLine, importedContents))
        {
                break;
        }

        out << importedContents;
    }

    // 2. some definitions in the current line?
    while (!cleanedLine.trimmed().isEmpty())
    {
        if (!processVariableDefinition(cleanedLine))
        {  
             // 3. true CSS content
             auto old = strgShift(m_parseContext, cleanedLine);

             auto resolvedLine = replaceVariables(cleanedLine);
             out << resolvedLine;
             
             (void)strgShift(m_parseContext, old);
             return 
                !resolvedLine.isNull();
        }
    }

    // oops, no content, only definitions!
    return false;
}


bool QsseFileLoader::processVariableDefinition(QString& strg)
{
    auto match = c_variableDefRegex.match(strg);

    if (!match.hasMatch())
    {
        return false;
    }

    Q_ASSERT(match.lastCapturedIndex() == 3);

    auto matchedTxt = match.captured(0);
    auto lineIndent = match.captured(1);
    auto varName = match.captured(2);
    auto expr = match.captured(3);

    auto old = strgShift(m_parseContext, matchedTxt);

    auto varValue = replaceVariables(expr);
    createVariable(varName, varValue);

    // replace first occurrence of matched strg (only!)
    Q_ASSERT(strg.indexOf(matchedTxt) == 0);
        
    strg.replace(0, matchedTxt.length(), lineIndent);

    (void)strgShift(m_parseContext, old);
    return true;
}


bool QsseFileLoader::processImportClause(QString& strg, QString& fileContents)
{
    auto match = c_importClauseRegex.match(strg);
    if (!match.hasMatch())
    {
        return false;
    }

    Q_ASSERT(match.lastCapturedIndex() == 4);
    
    auto matchedTxt = match.captured(0);
    auto lineIndent = match.captured(1);
    auto quotMark = match.captured(2);
    auto expr = match.captured(3);
    auto terminator = match.captured(4);

    Q_UNUSED(quotMark);

    if (terminator.isEmpty() && !m_filePathList.isEmpty())
    {
        QString msg("import clause for \"%1\" in file \"%2\" not terminated with semicolon.");
        auto& filePath = m_filePathList.last();
        
        QTUTILS_LOG_INFO(tags::QssLoader, msg.arg(expr, filePath));
        m_errorMessages << msg.arg(expr, filePath);

        // ok, we are lenient...
    }

    auto old = strgShift(m_parseContext, matchedTxt);

    // imported file name may contain variables!
    //  - this can be used to switch between themes
    auto fileName = replaceVariables(expr);       

    Q_ASSERT(!m_filePathList.isEmpty()); 
    Q_ASSERT(!m_parseContext.isEmpty());

    if (fileName.isEmpty())
    {
        auto msg = QString("contains empty style sheet file name in \"%1\" - ignoring...").arg(m_parseContext);
        auto& importingFilePath = m_filePathList.last();

        reportStyleFileError(importingFilePath, msg);
        fileContents = QString();
    }
    else
    {
        //QTUTILS_LOG_TRACE(tags::QssLoader, QString("importing style sheet file \"") + fileName + "\".");
        traceFileInfo("importing style sheet file ", fileName);
        fileContents = processFile(fileName);
    }

    Q_ASSERT(strg.indexOf(matchedTxt) == 0);
    strg.replace(0, matchedTxt.length(), lineIndent);

    (void)strgShift(m_parseContext, old);
    return true;
}


bool QsseFileLoader::substVariableName(QString& strg) const
{
    auto match = c_variableNameRegex.match(strg);

    if (!match.hasMatch())
    {
        return false;
    }

    Q_ASSERT(match.lastCapturedIndex() == 0);

    QString name  = match.captured(0);
    QString val = lookupVariable(name);

    // replace the first occurrence of variable name (only!)
    Q_ASSERT(strg.indexOf(name) == match.capturedStart(0));

    strg.replace(match.capturedStart(0), name.length(), val);

    return true;
}


void QsseFileLoader::reportVariableError(const QString& varName, const QString& errTxt) const
{
    if (!m_filePathList.isEmpty() &&
         !m_parseContext.isEmpty())
    {
        QString msg = "style sheet variable %1 in \"%2\" in file \"%3\" " + errTxt;
        auto& filePath = m_filePathList.last();

        QTUTILS_LOG_ERROR(tags::QssLoader, msg.arg(varName, m_parseContext, filePath));
        m_errorMessages << msg.arg(varName, m_parseContext, filePath);
    }
    else
    {
        QString msg = "style sheet variable %1 " + errTxt;

        QTUTILS_LOG_ERROR(tags::QssLoader, msg.arg(varName));
        m_errorMessages << msg.arg(varName);
    }
}


void QsseFileLoader::reportStyleFileError(const QString& filePath, const QString& errTxt) const
{
    QString msg = "style sheet file \"%1\" " + errTxt;

    QTUTILS_LOG_ERROR(tags::QssLoader, msg.arg(filePath));
    m_errorMessages << msg.arg(filePath);
}


void QsseFileLoader::traceFileInfo(const char* message, const QString& filePath) const
{
    QTUTILS_LOG_TRACE(tags::QssLoader, message + QString("\"") + filePath + "\".");
}


qint64 QsseFileLoader::maxAllowedFileSize() const
{
    return 
        Q_INT64_C(1000000); // OPEN TODO::: move to constants???
}

