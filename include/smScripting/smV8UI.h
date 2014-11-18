#ifndef SMV8UI_H
#define SMV8UI_H
#include <QtGui/QDialog>
#include "ui_v8.h"
#include <iostream>
#include "V8Scripting.h"

using namespace std;


class smV8UI: public QDialog {
	Q_OBJECT
public:
	smV8ScriptingEngine *V8Engine;
	Ui_V8UI uiForm;
	
	smV8UI(){
		uiForm.setupUi(this);
		V8Engine=new smV8ScriptingEngine();
		V8Engine->start();
		///spin loop
		while(1){
			if(V8Engine->isV8EngineStarted)
				break;

		}

	
	}
		
	virtual void show();

public slots:
	void executeScript();
	void refreshVariables(){
	  cout<<"Heyooo"<<endl;
	}
	void testScript();




};




#endif