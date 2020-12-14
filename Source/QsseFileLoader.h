/** @file   QsseFileLoader.h
   @author  Marek Krajewski (mrkkrj), www.ib-krajewski.de
   @date    28.09.2020

   @brief Declaration of Xamgu's QsseFileLoader class

   @copyright  Copyright 2020, Marek Krajewski, released under the terms of LGPL v3 
*/

#pragma once

// OPEN TODO:::
#ifdef QSSLD_LOGGING_ENABLED
#  include "Logging/Logger.h"
#endif

#include <QtCore/QSet>
#include <QtCore/QTextStream>


namespace ibkrj { 
   namespace xamgu { 

    
   /**
      @brief: Loader for QSS style sheet files and file groups using extended syntax.

      The supported extensions are:
        1. CSS variables:
            definition - $default-font-size: 16px;
                         $default-font-family: "Arial";
                         $default-font-family-for-widgets: $default-font-family;
            usage -  QWidget { font-size: $default-font-size; color: red; }

        2. QSS file imports:
            @import "SomeStylesheetFile.qsse"    
    */
    class QsseFileLoader
    {
    public:
       /**
          @brief: constructor

          @param stylesheetsDir: directory to look up for stylesheets, if none provided, 
                     stylesheets will be loaded from a pre-configured Qt resource filesystem 
                     path (":/StyleSheets/").
        */
        QsseFileLoader(const QString& stylesheetsDir = QString());

        /**
          @brief: Load a stylesheet file

          @param fileToLoad: either stylesheet name to load from the pre-configured path or a full 
                             file path if another directory has to be used.                           
          @return: Contents of the extended QSS file with all the variables and imports resolved,
                   or empty if an error was encountered. In this case messages() will return some diagnostics.
         */
        QString loadStyleSheet(const QString& fileToLoad);

        /**
           @brief: Create a variable to pre-configure the loader.

           This can be used to switch between several themes, e.g.: @import "Colors$theme-name.qsse"
         */
        void createVariable(const QString& name, const QString& value);

        /**
           @brief: Get the value of a variable or empty string on error.
         */
        QString lookupVariable(const QString& name) const;

        /**
           @brief: get current parse messages
         */
        const QStringList& messages() const;

    private:
        QString processFile(const QString& fileToLoad);

        QString processFileContents(const QString& contents);
        QString removeComments(const QString& contents) const;
        QString replaceVariables(const QString& strg) const;

        bool getEffectiveFilePath(const QString& fileToLoad, QString& effectiveFilePath, QString* efectiveLoadDir = nullptr);
        bool alreadyProcessed(const QString& filePath);
        bool getFileContents(const QString& filePath, QString& fileContents);

        bool processLine(const QString& line, QTextStream& out);
        bool processVariableDefinition(QString& strg);
        bool processImportClause(QString& strg, QString& fileContents);
        bool substVariableName(QString& strg) const;

        void reportVariableError(const QString& varName, const QString& errTxt) const;
        void reportStyleFileError(const QString& filePath, const QString& errTxt) const;
        void traceFileInfo(const char* message, const QString& filePath) const;

        qint64 maxAllowedFileSize() const;

        // members
        const QString m_stylesheetsDir;

        QStringList m_filePathList;
        QStringList m_directoryPathList;
        
        QSet<QString> m_processedFiles;        
        QMap<QString, QString> m_variableDefs;

        QString m_parseContext;
        mutable QStringList m_errorMessages;         
    };

   } 
} 

