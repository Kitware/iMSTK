/********************************************************************************
** Form generated from reading UI file 'v8.ui'
**
** Created: Tue Jul 22 16:16:50 2014
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_V8_H
#define UI_V8_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_V8UI
{
public:
    QPushButton *pushButton;
    QLabel *label;
    QLabel *label_2;
    QTextEdit *textEdit_2;
    QPushButton *pushButton_2;
    QTextEdit *outputTextEdit;
    QLabel *label_3;
    QPushButton *refresh;
    QTabWidget *tabWidget;
    QWidget *tab;
    QTextEdit *textEdit;
    QWidget *tab_2;
    QTextEdit *textEdit_3;
    QPushButton *TestScript;

    void setupUi(QDialog *V8UI)
    {
        if (V8UI->objectName().isEmpty())
            V8UI->setObjectName(QString::fromUtf8("V8UI"));
        V8UI->resize(1090, 868);
        pushButton = new QPushButton(V8UI);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(10, 810, 161, 51));
        QFont font;
        font.setPointSize(10);
        pushButton->setFont(font);
        label = new QLabel(V8UI);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 20, 231, 31));
        QFont font1;
        font1.setPointSize(10);
        font1.setBold(true);
        font1.setWeight(75);
        label->setFont(font1);
        label_2 = new QLabel(V8UI);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(420, 46, 371, 17));
        label_2->setFont(font1);
        textEdit_2 = new QTextEdit(V8UI);
        textEdit_2->setObjectName(QString::fromUtf8("textEdit_2"));
        textEdit_2->setGeometry(QRect(420, 70, 661, 291));
        textEdit_2->setFont(font);
        textEdit_2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        pushButton_2 = new QPushButton(V8UI);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setGeometry(QRect(180, 810, 231, 51));
        pushButton_2->setFont(font);
        pushButton_2->setFlat(false);
        outputTextEdit = new QTextEdit(V8UI);
        outputTextEdit->setObjectName(QString::fromUtf8("outputTextEdit"));
        outputTextEdit->setGeometry(QRect(410, 480, 661, 301));
        outputTextEdit->setFont(font);
        outputTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        outputTextEdit->setLineWrapMode(QTextEdit::NoWrap);
        outputTextEdit->setReadOnly(true);
        outputTextEdit->setAcceptRichText(false);
        label_3 = new QLabel(V8UI);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(420, 450, 121, 17));
        label_3->setFont(font1);
        refresh = new QPushButton(V8UI);
        refresh->setObjectName(QString::fromUtf8("refresh"));
        refresh->setGeometry(QRect(420, 370, 151, 41));
        refresh->setFont(font);
        tabWidget = new QTabWidget(V8UI);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(10, 50, 391, 731));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(tab->sizePolicy().hasHeightForWidth());
        tab->setSizePolicy(sizePolicy);
        textEdit = new QTextEdit(tab);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        textEdit->setGeometry(QRect(0, 0, 381, 701));
        sizePolicy.setHeightForWidth(textEdit->sizePolicy().hasHeightForWidth());
        textEdit->setSizePolicy(sizePolicy);
        textEdit->setFont(font);
        textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        textEdit_3 = new QTextEdit(tab_2);
        textEdit_3->setObjectName(QString::fromUtf8("textEdit_3"));
        textEdit_3->setGeometry(QRect(0, 0, 381, 701));
        textEdit_3->setFont(font);
        textEdit_3->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        textEdit_3->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        tabWidget->addTab(tab_2, QString());
        TestScript = new QPushButton(V8UI);
        TestScript->setObjectName(QString::fromUtf8("TestScript"));
        TestScript->setGeometry(QRect(420, 810, 121, 51));

        retranslateUi(V8UI);
        QObject::connect(pushButton, SIGNAL(clicked()), V8UI, SLOT(executeScript()));
        QObject::connect(refresh, SIGNAL(clicked()), V8UI, SLOT(refreshVariables()));
        QObject::connect(TestScript, SIGNAL(clicked()), V8UI, SLOT(testScript()));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(V8UI);
    } // setupUi

    void retranslateUi(QDialog *V8UI)
    {
        V8UI->setWindowTitle(QApplication::translate("V8UI", "Dialog", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("V8UI", "Execute Script", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("V8UI", "JavaScript Code", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("V8UI", "SoFMIS Registrated Variables", 0, QApplication::UnicodeUTF8));
        pushButton_2->setText(QApplication::translate("V8UI", "Execute Script in New Thread", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("V8UI", "Output", 0, QApplication::UnicodeUTF8));
        refresh->setText(QApplication::translate("V8UI", "Refresh Variables", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("V8UI", "Script 1", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("V8UI", "Script 2", 0, QApplication::UnicodeUTF8));
        TestScript->setText(QApplication::translate("V8UI", "PushButton", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class V8UI: public Ui_V8UI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_V8_H
