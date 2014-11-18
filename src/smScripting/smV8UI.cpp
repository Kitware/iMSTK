#include "smScripting/smV8UI.h"

void smV8UI::show(){
	
	 //uiForm.;
	
	
		this->setVisible(true);
		this->exec();
		 
		
}

void smV8UI::executeScript()

{
		cout<<uiForm.textEdit->toPlainText().toLocal8Bit().data()<<endl;
		//V8Engine->executeString(uiForm.textEdit->toPlainText().toLocal8Bit().data());
		//V8Engine->test();
		V8Engine->executeSource(uiForm.textEdit->toPlainText().toLocal8Bit().data());
		uiForm.outputTextEdit->setPlainText(QString("----Print Output----"));
		uiForm.outputTextEdit->append(QString(V8Engine->scriptOutput));
		uiForm.outputTextEdit->append(QString("----Execution Output----"));
		uiForm.outputTextEdit->append(QString(V8Engine->scriptExecutionOutput));

	}


void smV8UI::testScript()

{
		
		V8Engine->executeFunction_TEST();
		return;
		cout<<uiForm.textEdit->toPlainText().toLocal8Bit().data()<<endl;
		//V8Engine->executeString(uiForm.textEdit->toPlainText().toLocal8Bit().data());
		//V8Engine->test();
		V8Engine->executeFunction("TEST_FUNC()",0,0,0);
		
		uiForm.outputTextEdit->setPlainText(QString("----Print Output----"));
		uiForm.outputTextEdit->append(QString(V8Engine->scriptOutput));
		uiForm.outputTextEdit->append(QString("----Execution Output----"));
		uiForm.outputTextEdit->append(QString(V8Engine->scriptExecutionOutput));

	}