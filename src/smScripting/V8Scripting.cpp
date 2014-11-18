#include "smScripting/V8Scripting.h"
using namespace v8;
smChar *smV8ScriptingEngine::redirectedOutput;///use this one to redirect the output;
smInt smV8ScriptingEngine::bufferIndex=0;


const char* ToCString(const v8::String::Utf8Value& value) {
		 return *value ? *value : "<string conversion failed>";
}
void myPrint(const v8::FunctionCallbackInfo<v8::Value>& args) {
	  bool first = true;
	  
	  for (int i = 0; i < args.Length(); i++) {
		v8::HandleScope handle_scope(args.GetIsolate());
		if (first) {
		  first = false;
		} else {
		  printf(" ");
		}
		v8::String::Utf8Value str(args[i]);
		const char* cstr = ToCString(str);
		smInt sizeOfString=strlen(cstr);
	     if(sizeOfString+smV8ScriptingEngine::bufferIndex+1<SM_V8_MAXSOURCECODELENGTH&&smV8ScriptingEngine::redirectedOutput!=NULL){
		   strcpy(smV8ScriptingEngine::redirectedOutput+smV8ScriptingEngine::bufferIndex,cstr);
	      }
	    smV8ScriptingEngine::bufferIndex+=sizeOfString;
		//printf("%s", cstr);
		 /*if(smV8ScriptingEngine::bufferIndex+1<SM_V8_MAXSOURCECODELENGTH&&smV8ScriptingEngine::redirectedOutput!=NULL){
		   strcpy(smV8ScriptingEngine::redirectedOutput+smV8ScriptingEngine::bufferIndex,"\n");
	     }*/
	  }
	  printf("\n");
	  fflush(stdout);
}
smV8ScriptingEngine::smV8ScriptingEngine(){
	
	
	
	
	scriptId=0;
	nextSciptId=0;
	scriptType=smV8_EXECUTION_SOURCE;
	currentScriptId=0;
	scriptCheckInterval=10;//10 milliseconds. 100hz
	isV8EngineStarted=false;
	registerVariableInQueue=false;
	/*v8::Isolate *iso=Isolate::GetCurrent();
	if(iso)
		v8::V8::TerminateExecution(iso);
	 v8::V8::InitializeICU();*/

	//HandleScope handle_scope(isolate);
	//context=isolate->GetCurrentContext();
	//isolate_scope=new v8::Isolate::Scope(isolate);
	
	scriptSource=new smChar[SM_V8_MAXSOURCECODELENGTH];
	scriptOutput=new smChar[SM_V8_MAXSOURCECODELENGTH];
	scriptExecutionOutput=new smChar[SM_V8_MAXSOURCECODELENGTH];
	memset(scriptSource,0,SM_V8_MAXSOURCECODELENGTH);
	memset(scriptOutput,0,SM_V8_MAXSOURCECODELENGTH);
	memset(scriptExecutionOutput,0,SM_V8_MAXSOURCECODELENGTH);

	sprintf(scriptName,"script%d",scriptId);
}


 void smVariableRegistration<smInt>::XGetter(Local<String> p_property, const v8::PropertyCallbackInfo<Value>& p_info){
	
	  //return Integer::New(v8::Isolate::GetCurrent(),*value);
		 int* value =static_cast<int*>(v8::Handle<v8::External>::Cast(p_info.Data())->Value());
	     p_info.GetReturnValue().Set(v8_num(*value));
		  
	
}

 void smVariableRegistration<smInt>::XSetter(Local<String> p_property, Local<Value> p_value, const v8::PropertyCallbackInfo<void>&p_info){
	    //*value = p_value->Int32Value();
		 int* field =static_cast<int*>(v8::Handle<v8::External>::Cast(p_info.Data())->Value());
		*field = p_value->Int32Value();
}
