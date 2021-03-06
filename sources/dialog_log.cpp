/****************************************************************************
** Copyright (c) 2019 Evgeny Teterin (nayk) <sutcedortal@gmail.com>
** All right reserved.
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
** LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
** OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
** WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
****************************************************************************/
#include <QApplication>
#include <QScrollBar>
#include <QFile>
#include <QIcon>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "ImagesConst"
#include "dialog_log.h"

namespace nayk { //=============================================================

const QString clDark           = "#111416";
const QString clLight          = "#fdfdfd";
const QString clLogDateDark    = "#929292";
const QString clLogDateLight   = "#333333";
const QString clLogPrefixDark  = "#c88dee";
const QString clLogPrefixLight = "#562873";
const QString clLogInfDark     = "#ffffff";
const QString clLogInfLight    = "#000000";
const QString clLogWrnDark     = "#ff9c54";
const QString clLogWrnLight    = "#8d3c00";
const QString clLogErrDark     = "#ff4040";
const QString clLogErrLight    = "#a50000";
const QString clLogInDark      = "#55d864";
const QString clLogInLight     = "#003706";
const QString clLogOutDark     = "#dd69bb";
const QString clLogOutLight    = "#53003b";
const QString clLogTxtDark     = "#d8d8d8";
const QString clLogTxtLight    = "#1f1f1f";
const QString clLogDbgDark     = "#00ddc6";
const QString clLogDbgLight    = "#006766";
const QString clLogOtherDark   = "#8f8f8f";
const QString clLogOtherLight  = "#888888";

//==============================================================================
DialogLog::DialogLog(QWidget *parent) : QDialog(parent)
{
    initializeDialog();
    m_actionEnable = true;
}
//==============================================================================
DialogLog::DialogLog(int maximumBlockCount, QWidget *parent)
    : QDialog(parent)
    , m_maximumBlockCount {maximumBlockCount}
{
    initializeDialog();
    m_actionEnable = true;
}
//==============================================================================
DialogLog::~DialogLog()
{

}
//==============================================================================
int DialogLog::maximumBlockCount() const
{
    return m_maximumBlockCount;
}
//==============================================================================
void DialogLog::setMaximumBlockCount(int maximumBlockCount)
{
    m_maximumBlockCount = maximumBlockCount;
    checkBlockCount();
}
//==============================================================================
bool DialogLog::openLogDirButtonVisible() const
{
    return m_openLogDirButtonVisible;
}
//==============================================================================
void DialogLog::setOpenLogDirButtonVisible(bool openLogDirButtonVisible)
{
    m_openLogDirButtonVisible = openLogDirButtonVisible;

    if(pushButtonOpenLogDir) {
        pushButtonOpenLogDir->setVisible( m_openLogDirButtonVisible );
    }
}
//==============================================================================
void DialogLog::initializeDialog()
{
    setAttribute(Qt::WA_DeleteOnClose, false);
    setWindowFlag( Qt::WindowMaximizeButtonHint );
    setWindowTitle(tr("Log"));

    if(QFile::exists(iconConsole)) {
        setWindowIcon( QIcon(iconConsole) );
    }

    this->setMinimumSize( 800, 600 );

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(10);
    layout->setSpacing(10);

    textEditLog = new QTextEdit(this);
    textEditLog->setReadOnly(true);
    textEditLog->sizePolicy().setHorizontalPolicy( QSizePolicy::Expanding );
    textEditLog->sizePolicy().setVerticalPolicy( QSizePolicy::Expanding );
    textEditLog->setWordWrapMode( QTextOption::WordWrap );
    textEditLog->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    textEditLog->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

    layout->addWidget( textEditLog );

    QHBoxLayout *bottomLayout = new QHBoxLayout();

    QLabel *label = new QLabel(tr("Filter:"), this);
    bottomLayout->addWidget(label);

    lineEditFilter = new QLineEdit(this);
    lineEditFilter->setMinimumWidth(100);
    connect(lineEditFilter, &QLineEdit::editingFinished, this, &DialogLog::lineEditFilter_editingFinished);
    bottomLayout->addWidget(lineEditFilter);
    bottomLayout->addSpacerItem( new QSpacerItem(10, 10) );

    QCheckBox *checkBox = new QCheckBox( tr("Dark theme"), this );
    checkBox->setChecked(m_dark);
    connect(checkBox, &QCheckBox::toggled, this, &DialogLog::checkBoxDark_toggled);
    bottomLayout->addWidget(checkBox);
    bottomLayout->addStretch();

    pushButtonOpenLogDir = new QPushButton(tr("Log folder..."), this);
    pushButtonOpenLogDir->setMinimumSize(120, 32);
    pushButtonOpenLogDir->setIconSize( QSize(28, 28) );

    if(QFile::exists(iconOpen)) {
        pushButtonOpenLogDir->setIcon( QIcon(iconOpen) );
    }

    connect(pushButtonOpenLogDir, &QPushButton::clicked, this, &DialogLog::openLogDirButtonClicked);
    bottomLayout->addWidget(pushButtonOpenLogDir);

    QPushButton *button = new QPushButton(tr("Clear"), this);
    button->setMinimumSize( pushButtonOpenLogDir->minimumSize() );
    button->setIconSize( pushButtonOpenLogDir->iconSize() );

    if(QFile::exists(iconClear)) {
        button->setIcon( QIcon(iconClear) );
    }

    connect(button, &QPushButton::clicked, this, &DialogLog::pushButtonClear_clicked);
    bottomLayout->addWidget(button);

    button = new QPushButton(tr("Close"), this);
    button->setMinimumSize( pushButtonOpenLogDir->minimumSize() );
    button->setIconSize( pushButtonOpenLogDir->iconSize() );

    if(QFile::exists(iconCancel)) {
        button->setIcon( QIcon(iconCancel) );
    }

    connect(button, &QPushButton::clicked, this, &DialogLog::close);
    bottomLayout->addWidget(button);

    layout->addLayout( bottomLayout );
    this->setLayout(layout);

    logList.reserve(m_maximumBlockCount);
    pushButtonOpenLogDir->setVisible( m_openLogDirButtonVisible );
    checkBoxDark_toggled( m_dark );
}
//==============================================================================
void DialogLog::checkBlockCount()
{
    if(logList.size() < m_maximumBlockCount) return;
    while(logList.size() >= m_maximumBlockCount) {

        logList.removeFirst();
    }

    applyFilter();
}
//==============================================================================
void DialogLog::applyFilter()
{
    if(!textEditLog) return;
    QScrollBar* sb = textEditLog->verticalScrollBar();
    bool bScroll = sb && (sb->value() == sb->maximum());

    textEditLog->setUpdatesEnabled(false);
    textEditLog->clear();

    for(int i=0; i<logList.size(); ++i) {

        QString str = logList.at(i);
        if(m_filtrStr.isEmpty() || str.contains(m_filtrStr)) textEditLog->append( highlight(str, m_dark) );
    }

    textEditLog->setUpdatesEnabled(true);
    textEditLog->update();

    if (sb && bScroll) {
        sb->setValue(sb->maximum());
    }
}
//==============================================================================
QString DialogLog::highlight(const QString &text, bool dark)
{
    QString suffix = "</font>";
    QString preDate = "<font color=\"" + (dark ? clLogDateDark : clLogDateLight) + "\">";
    QString prePrefix = "<font color=\"" + (dark ? clLogPrefixDark : clLogPrefixLight) + "\">";
    QString preInf = "<font color=\"" + (dark ? clLogInfDark : clLogInfLight) + "\">";
    QString preWrn = "<font color=\"" + (dark ? clLogWrnDark : clLogWrnLight) + "\">";
    QString preErr = "<font color=\"" + (dark ? clLogErrDark : clLogErrLight) + "\">";
    QString preIn = "<font color=\"" + (dark ? clLogInDark : clLogInLight) + "\">";
    QString preOut = "<font color=\"" + (dark ? clLogOutDark : clLogOutLight) + "\">";
    QString preTxt = "<font color=\"" + (dark ? clLogTxtDark : clLogTxtLight) + "\">";
    QString preDbg = "<font color=\"" + (dark ? clLogDbgDark : clLogDbgLight) + "\">";
    QString preOther = "<font color=\"" + (dark ? clLogOtherDark : clLogOtherLight) + "\">";
    QString str = text;
    QString dtStr = "";
    QString typeStr = "";

    if((str.length() > 20) && (str[0] == QChar('[')) && (str[13] == QChar(']')) &&
            (str[14] == QChar('[')) && (str[18] == QChar(']')) ) {
        dtStr = str.left(14);
        typeStr = str.mid(14, 5);
        str.remove(0,19);
    }

    QString line = "";

    if(!dtStr.isEmpty())
        line += preDate + dtStr + suffix;

    if(!typeStr.isEmpty())
        line += prePrefix + typeStr.toHtmlEscaped() + suffix;

    if(str.isEmpty())
        str = "&nbsp;";

    Log::LogType logType = Log::strToLogType(typeStr);

    switch (logType) {
    case Log::LogInfo:    return line + preInf + str + suffix;
    case Log::LogWarning: return line + preWrn + "<i>" + str + "</i>" + suffix;
    case Log::LogError:   return line + preErr + "<b><i>" + str + "</i></b>" + suffix;
    case Log::LogIn:      return line + preIn + str + suffix;
    case Log::LogOut:     return line + preOut + str + suffix;
    case Log::LogText:    return line + preTxt + str + suffix;
    case Log::LogDbg:     return line + preDbg + str + suffix;
    default:              return line + preOther + str + suffix;
    }
}
//==============================================================================
void DialogLog::saveToLog(const QString &text, Log::LogType logType)
{
    QString prefix = Log::getLogPrefix( logType );
    QStringList list = text.split("\n");

    for (QString str: list) {
        write( prefix + str );
    }
}
//==============================================================================
void DialogLog::write(const QString &text)
{
    logList.append(text);

    if(textEditLog && (m_filtrStr.isEmpty() || text.contains(m_filtrStr))) {

        QScrollBar* sb = textEditLog->verticalScrollBar();
        bool bScroll = sb && (sb->value() == sb->maximum());
        textEditLog->append( highlight(text, m_dark) );

        if (sb && bScroll) {
            sb->setValue(sb->maximum());
        }
    }
    checkBlockCount();
}
//==============================================================================
void DialogLog::lineEditFilter_editingFinished()
{
    QLineEdit *edit = qobject_cast<QLineEdit*>(sender());

    if(!edit)
        return;

    m_filtrStr = edit->text().trimmed();
    applyFilter();
}
//==============================================================================
void DialogLog::pushButtonClear_clicked()
{
    logList.clear();
    if(textEditLog)
        textEditLog->clear();
}
//==============================================================================
void DialogLog::checkBoxDark_toggled(bool checked)
{
    m_dark = checked;

    if(textEditLog) {
        textEditLog->setStyleSheet(QString("QTextEdit { "
                                           "color: %1; "
                                           "background-color: %2; "
                                           "font-family: Courier New, Lucida Console, Monospace; "
                                           "font-size: 11pt; "
                                           "}")
                                   .arg(m_dark ? clLight : clDark)
                                   .arg(m_dark ? clDark : clLight)
                                   );
    }

    if(m_actionEnable)
        applyFilter();
}
//==============================================================================

} // namespace nayk //==============================================================================
