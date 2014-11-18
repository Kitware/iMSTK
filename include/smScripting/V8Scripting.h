#ifndef V8SCRIPTINNG_H
#define V8SCRIPTINNG_H

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smErrorLog.h"

#include <v8.h>
#include <string.h>
#include <iostream>
#include <QThread>


using namespace std;
using namespace v8;

#define SM_V8_MAXSOURCECODELENGTH 5000
#define SM_V8_MAXSCRIPTNAME 100

const char* ToCString(const v8::String::Utf8Value& value) ;
void myPrint(const v8::FunctionCallbackInfo<v8::Value>& args);


static inline v8::Local<v8::Value> v8_num(double x) {
	return v8::Number::New(v8::Isolate::GetCurrent(), x);
}


static inline v8::Local<v8::String> v8_str(const char* x) {
	return v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), x);
}

static inline v8::Local<v8::String> v8_strfromIsolate(const char* x,v8::Isolate* p_isolate) {
	return v8::String::NewFromUtf8(p_isolate, x);
}
static inline v8::Local<v8::Script> v8_compile(const char* x) {
	return v8::Script::Compile(v8_str(x));
}


static inline v8::Local<v8::Script> v8_compile(v8::Local<v8::String> x) {
	return v8::Script::Compile(x);
}

template<class T>
struct smVariableRegistration{
public:
	static void (XGetter)(Local<String> ,  const v8::PropertyCallbackInfo<Value>&);
	static void (XSetter)(v8::Local<String> , v8::Local<Value> ,  const v8::PropertyCallbackInfo<void>&);

};

template <>
struct smVariableRegistration<smInt>{
public:
	static void (XGetter)(Local<String> p_property, const v8::PropertyCallbackInfo<Value>& p_info);
	static void (XSetter)(Local<String> p_property, Local<Value> p_value, const v8::PropertyCallbackInfo<void>&p_info);

};



typedef smVariableRegistration<smInt> smVariableRegistrationI;

class Point {
public :
	int a;
	int b;
	Point(){
		a=-3213;
		b=-66666;
	}
};


enum smV8_EXECUTIONTYPE{
	smV8_EXECUTION_SOURCE,
	smV8_EXECUTION_FUNCTION
};


class smV8ScriptingEngine:public smCoreClass, public QThread{

public:



	smChar scriptName[SM_V8_MAXSCRIPTNAME];
	smChar *scriptSource;

	smChar *scriptOutput;
	smChar *scriptExecutionOutput;
	smInt scriptId;
	smInt nextSciptId;
	smV8_EXECUTIONTYPE scriptType;

	smInt currentScriptId;
	smInt scriptCheckInterval;
	smBool isV8EngineStarted;
	smBool registerVariableInQueue;

	//exception handler
	//v8::TryCatch try_catch;
	Isolate* isolate;
	v8::Handle<v8::ObjectTemplate> global;
	//Persistent<Context> persistentContext;
	v8::Local<v8::Context>  context;
	HandleScope *handle_scope;
	v8::Persistent<Context> contextPersistent;


	smV8ScriptingEngine();

	static smChar *redirectedOutput;
	static smInt bufferIndex;



	template <class T>
	void registerVariable(smChar *p_name,T *p_value){
		//v8::Isolate* isolate = context->GetIsolate();
		//v8::Locker locker(isolate);

		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope scope(isolate);
		//context->Enter();

		const AccessorGetterCallback get = (AccessorGetterCallback)&smVariableRegistration<smInt>::XGetter;
		const AccessorSetterCallback set = (AccessorSetterCallback)&smVariableRegistration<smInt>::XSetter;

		// v8::Handle<v8::ObjectTemplate> global_templ  = v8::ObjectTemplate::New(isolate);
		// v8::Local<v8::Context> context  = 	v8::Context::New(isolate, NULL, global);
		//v8::Persistent<v8::Context> context =isolate->GetCurrentContext();
		//v8::Handle<v8::ObjectTemplate> obj = ObjectTemplate::New(isolate);
		//obj->SetAccessor(v8_str(p_name), (smVariableRegistration::XGetter),(smVariableRegistration::XSetter));
		//global->SetAccessor(v8_strfromIsolate(p_name,isolate), ((AccessorGetterCallback)&smVariableRegistration<T>::XGetter),((AccessorSetterCallback)&smVariableRegistration<T>::XSetter),  v8::External::New(isolate, p_value),ALL_CAN_WRITE);
		//global->SetAccessor(v8_strfromIsolate(p_name,isolate), (get),(set),  v8::External::New(isolate, p_value),ALL_CAN_WRITE);
		context->Global()->SetAccessor(v8_strfromIsolate(p_name,isolate), (get),(set),  v8::External::New(isolate, p_value),ALL_CAN_WRITE);

		//obj->SetAccessorProperty(v8_strfromIsolate(p_name,isolate), v8::FunctionTemplate::New(isolate, smVariableRegistration<T>::XGetter) ,v8::FunctionTemplate::New(smVariableRegistration<T>::XSetter));
		//v8::Handle<v8::Object> inst = obj->NewInstance();
		//global->Set(v8_strfromIsolate(p_name,isolate), inst);
		//context->Exit();

		//context  = 	v8::Context::New(isolate, NULL, global);


	}

	/*template<class T,class K>
	void registerVariableClass(smChar *p_name,T *p_value,smChar *p_propertyName,K *p_property){


	v8::Isolate::Scope isolate_scope(isolate);

	v8::HandleScope scope(isolate);


	v8::Handle<v8::ObjectTemplate>  result =ObjectTemplate::New(isolate);
	result->SetInternalFieldCount(1);


	const AccessorGetterCallback get = (AccessorGetterCallback)&smVariableRegistration<K>::XGetter;
	const AccessorSetterCallback set = (AccessorSetterCallback)&smVariableRegistration<K>::XSetter;
	result->SetAccessor(v8_strfromIsolate(p_propertyName,isolate), (get),(set),v8::External::New(isolate, p_property),ALL_CAN_WRITE);
	Local<Object> obj = result->NewInstance();///returns null, so it doesn't work
	obj->SetInternalField(0,External::New(isolate,p_value));
	context->Global()->Set(v8_strfromIsolate(p_name,isolate),   obj);
	}*/

	template<class T,class K>
	void registerVariableClass(smChar *p_name,T *p_value,smChar *p_propertyName,K *p_property){


		v8::Isolate::Scope isolate_scope(isolate);

		v8::HandleScope scope(isolate);


		v8::Handle<v8::FunctionTemplate>  result =FunctionTemplate::New(isolate);
		result->SetClassName(v8_strfromIsolate(p_name,isolate));


		const AccessorGetterCallback get = (AccessorGetterCallback)&smVariableRegistration<K>::XGetter;
		const AccessorSetterCallback set = (AccessorSetterCallback)&smVariableRegistration<K>::XSetter;

		Handle<ObjectTemplate> obj = result->InstanceTemplate();
		obj->SetInternalFieldCount(1);
		obj->SetAccessor(v8_strfromIsolate(p_propertyName,isolate), (get),(set),v8::External::New(isolate, p_property),ALL_CAN_WRITE);


		//obj->SetInternalField(0,External::New(isolate,p_value));

		Handle<Function> point_ctor = result->GetFunction();
		Local<Object> obj1 = point_ctor->NewInstance();
		obj1->SetInternalField(0, External::New(isolate,p_value));
		context->Global()->Set(v8_strfromIsolate(p_name,isolate),   obj1);
	}

	template<class T,class K>
	void registerVariableClassTEST(smChar *p_name,T *p_value,smChar *p_propertyName,K *p_property){


		v8::Isolate::Scope isolate_scope(isolate);

		v8::HandleScope scope(isolate);


		v8::Handle<v8::ObjectTemplate>  result =ObjectTemplate::New(isolate);
		result->SetInternalFieldCount(1);


		const AccessorGetterCallback get = (AccessorGetterCallback)&smVariableRegistration<K>::XGetter;
		const AccessorSetterCallback set = (AccessorSetterCallback)&smVariableRegistration<K>::XSetter;

		result->SetAccessor(v8_strfromIsolate(p_propertyName,isolate), (get),(set),v8::External::New(isolate, p_property),ALL_CAN_WRITE);
		Local<Object> obj1 = result->NewInstance();
		obj1->SetInternalField(0, External::New(isolate,p_value));

		context->Global()->Set(v8_strfromIsolate(p_name,isolate),   obj1);
	}

	template<class T,class K>
	void registerVariableClassTEST2(smChar *p_name,T *p_value,smChar *p_propertyName,K *p_property){


		v8::Isolate::Scope isolate_scope(isolate);

		v8::HandleScope scope(isolate);


		v8::Handle<v8::FunctionTemplate>  result =FunctionTemplate::New(isolate);
		result->InstanceTemplate()->SetInternalFieldCount(1);


		const AccessorGetterCallback get = (AccessorGetterCallback)&smVariableRegistration<K>::XGetter;
		const AccessorSetterCallback set = (AccessorSetterCallback)&smVariableRegistration<K>::XSetter;
		result->InstanceTemplate()->SetAccessor(v8_strfromIsolate(p_propertyName,isolate), (get),(set),v8::External::New(isolate, p_property),ALL_CAN_WRITE);

		Local<Function> obj1 = result->GetFunction();
		//obj1->SetAccessor(v8_strfromIsolate(p_name,isolate), (get),(set),v8::External::New(isolate, p_property),ALL_CAN_WRITE);
		//obj1->SetAccessorProperty(v8_strfromIsolate(p_propertyName,isolate), (get),(set),v8::External::New(isolate, p_property),ALL_CAN_WRITE);

		//obj1->SetInternalField(0, External::New(isolate,p_value));
		context->Global()->Set(v8_strfromIsolate(p_name,isolate),   obj1);
	}

	void setScriptAndResetOutputBuf(smChar *p_script){
		smInt sizeOfString=strlen(p_script);
		if(sizeOfString<SM_V8_MAXSOURCECODELENGTH)
			strcpy(scriptSource,p_script);
		redirectedOutput=this->scriptOutput;
		memset(this->scriptOutput,SM_V8_MAXSOURCECODELENGTH,0);
		bufferIndex=0;
	}

	void executeSource(smChar *p_script){
		setScriptAndResetOutputBuf(p_script);
		scriptType=smV8_EXECUTION_SOURCE;
		nextSciptId=currentScriptId+1;
		while(nextSciptId>currentScriptId){
			//cout<<".";
		}



	}

	void executeFunction(smChar *p_functionName, void *p_arg1,void *p_arg2,void *p_arg3){

		setScriptAndResetOutputBuf(p_functionName);
		scriptType=smV8_EXECUTION_FUNCTION;
		nextSciptId=currentScriptId+1;
		while(nextSciptId>currentScriptId){
			//cout<<".";
		}

	}
	void test(){
		// Get the default Isolate created at startup.
		Isolate* isolate = Isolate::GetCurrent();
		if(isolate==NULL)
			isolate=Isolate::New();
		//V8::Initialize();

		v8::Isolate::Scope isolate_scope(isolate);
		// Create a stack-allocated handle scope.
		HandleScope handle_scope(isolate);


		// Create a new context.
		v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);

		v8::Local<v8::Context> context  = 	v8::Context::New(isolate, NULL, global);



		// Enter the context for compiling and running the hello world script.
		Context::Scope context_scope(context);

		// Create a string containing the JavaScript source code.
		Handle<String> source = String::NewFromUtf8(isolate, "'Hello' + ', World!'");

		// Compile the source code.
		Handle<Script> script = Script::Compile(source);

		// Run the script to get the result.
		Handle<Value> result = script->Run();

		// Convert the result to an UTF8 string and print it.
		String::Utf8Value utf8(result);
		printf("%s\n", *utf8);





	}



	void run(){
		// Get the default Isolate created at startup.
		V8::Initialize();
		isolate = Isolate::GetCurrent();
		if(isolate==NULL)
			isolate=Isolate::New();


		v8::Isolate::Scope isolate_scope(isolate);
		// Create a stack-allocated handle scope.
		HandleScope handle_scope(isolate);
		this->handle_scope=&handle_scope;
		// Create a new context.
		global = v8::ObjectTemplate::New(isolate);
		global->Set(v8::String::NewFromUtf8(isolate, "print"),v8::FunctionTemplate::New(isolate, myPrint));
		global->Set(v8::String::NewFromUtf8(isolate, "alert"),v8::FunctionTemplate::New(isolate, myPrint));



		smInt tanselValue=99;
		smInt trans=512;

		smChar *name="torik";
		smChar *nameTrans="trans";
		const AccessorGetterCallback get = (AccessorGetterCallback)&smVariableRegistration<smInt>::XGetter;
		const AccessorSetterCallback set = (AccessorSetterCallback)&smVariableRegistration<smInt>::XSetter;
		//Handle<ObjectTemplate> realm_template = ObjectTemplate::New(isolate);
		global->SetAccessor(v8::String::NewFromUtf8(isolate, name), (get),(set),  v8::External::New(isolate, &tanselValue),ALL_CAN_WRITE);
		//v8::Handle<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate);
		//templ->InstanceTemplate()->SetAccessor(v8_str("foo"),  (get),(set),  v8::External::New(isolate, &tanselValue),ALL_CAN_WRITE);
		// global->SetAccessor(v8_strfromIsolate(nameTrans,isolate), (get),(set),  v8::External::New(isolate, &trans),ALL_CAN_WRITE);



		context  = 	v8::Context::New(isolate, NULL, global);




		// Enter the context for compiling and running the hello world script.
		Context::Scope context_scope(context);
		Handle<String> source;
		Handle<Script> script;
		Handle<Value> result;
		isV8EngineStarted=true;

		Point *point=new Point();
		while(true){

			
			static int test=0;
			static int myFloat=433;
			if(test<1){
				registerVariable<int>("UCA",&myFloat);
				registerVariableClassTEST<Point,int>("TP",point,"a",&point->a);
				test++;
			}
			if(nextSciptId>currentScriptId)
			{
				
				if(scriptType==smV8_EXECUTION_SOURCE){ 
					source.Clear();
					source= String::NewFromUtf8(isolate, scriptSource);
					// Compile the source code.
					script= Script::Compile(source);
					// Run the script to get the result.
					result = script->Run();

					///send event of completion
					//printf("%s\n", *utf8);

				}
				else if (scriptType==smV8_EXECUTION_FUNCTION){
					Handle<v8::Object> gb = context->Global();
					Handle<v8::Value> value = gb->Get(String::NewFromUtf8(isolate,"test_function")); 
					if(value->IsFunction()){
						Handle<v8::Function> func = v8::Handle<v8::Function>::Cast(value);
						Handle<Value> args[2];
						args[0] = v8::String::NewFromUtf8(isolate,"value1");
						args[1] = v8::String::NewFromUtf8(isolate,"value2");

						result = func->Call(gb, 2, args);
						//if (js_result->IsInt32()) {
						//	int32_t result = js_result->ToInt32()->Value();
						// // do something with the result
						//}

					}
				}
				smChar *resultString;
				// Convert the result to an UTF8 string and print it.
				String::Utf8Value utf8(result);
				currentScriptId++; 
				if(*utf8){
					resultString=*utf8;
					smInt strlength=strlen(resultString);
					if(strlength<SM_V8_MAXSOURCECODELENGTH-1)
						strcpy(scriptExecutionOutput,*utf8);
				}


			}

			
			//this->msleep(scriptCheckInterval);///check the script at scriptCheckInterval milliseconds.

		}

	}
	void copyToOutput(Handle<Value> &result){


	}


	void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
		v8::HandleScope handle_scope(isolate);
		v8::String::Utf8Value exception(try_catch->Exception());
		const char* exception_string = ToCString(exception);
		v8::Handle<v8::Message> message = try_catch->Message();
		if (message.IsEmpty()) {
			// V8 didn't provide any extra information about this error; just
			// print the exception.
			printf("%s\n", exception_string);
		} else {
			// Print (filename):(line number): (message).
			v8::String::Utf8Value filename(message->GetScriptResourceName());
			const char* filename_string = ToCString(filename);
			int linenum = message->GetLineNumber();
			printf("%s:%i: %s\n", filename_string, linenum, exception_string);
			// Print line of source code.
			v8::String::Utf8Value sourceline(message->GetSourceLine());
			const char* sourceline_string = ToCString(sourceline);
			printf("%s\n", sourceline_string);
			// Print wavy underline (GetUnderline is deprecated).
			int start = message->GetStartColumn();
			for (int i = 0; i < start; i++) {
				printf(" ");
			}
			int end = message->GetEndColumn();
			for (int i = start; i < end; i++) {
				printf("^");
			}
			printf("\n");
		}
	}
	void executeFunction_TEST(){
		v8::Locker lock(isolate);
			v8::Isolate::Scope isolate_scope(isolate);
			HandleScope handle_scope(isolate);
			
			//Local<Context>	context  = 	v8::Context::New(isolate, NULL, global);
			Local<v8::Value> value = context->Global()->Get(String::NewFromUtf8(isolate,"test_function")); 
			if(value->IsFunction()){
				Local<v8::Function> func= v8::Local<v8::Function>::Cast(value);
				Local<Value> args[2];
				args[0] = v8::String::NewFromUtf8(isolate,"value1");
				args[1] = v8::String::NewFromUtf8(isolate,"value2");

				Local<Value> result = func->Call(context->Global(), 2, args);
			
				cout<<"test_function is called"<<endl;
			}
		cout<<"[End of executeFunction_TEST]"<<endl;
		//v8::Unlocker unlocker(isolate);
		
	}



	void test1(){


		v8::Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		// Create a new context.
		v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);

		v8::Local<v8::Context> context  = 	v8::Context::New(isolate, NULL, global);
		Context::Scope context_scope(context);
		// Create a string containing the JavaScript source code.
		Handle<String> source = String::NewFromUtf8(isolate, "'Hello' + ', World!'");

		// Compile the source code.
		Handle<Script> script = Script::Compile(source);

		// Run the script to get the result.
		Handle<Value> result = script->Run();

		// Convert the result to an UTF8 string and print it.
		String::Utf8Value utf8(result);
		printf("%s\n", *utf8);




	}





};












#endif