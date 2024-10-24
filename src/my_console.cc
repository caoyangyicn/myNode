#include "env-inl.h"
#include "node_external_reference.h"
#include "string_bytes.h"

#ifdef __MINGW32__
# include <io.h>
#endif  // __MINGW32__

#ifdef __POSIX__
# include <climits>         // PATH_MAX on Solaris.
#endif  // __POSIX__

#include <array>
#include <cerrno>
#include <cstring>

namespace node {
namespace my_console {

using v8::Array;
using v8::ArrayBuffer;
using v8::Boolean;
using v8::Context;
using v8::Float64Array;
using v8::FunctionCallbackInfo;
using v8::Int32;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::NewStringType;
using v8::Null;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Value;


static void Log(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    std::string str = "";
    for(int i = 0;  i < args.Length(); i++) {
        Local<Value> arg = args[i];
        if(arg.IsEmpty()) {
            str += "undefined";
        } else if(arg->IsNull()) {
            str += "null";
        } else if(arg->IsTrue()) {
            str += "true";
        } else if(arg->IsFalse()) {
            str += "false";
        } else if(arg->IsInt32()) {
            str += std::to_string(arg->Int32Value(context).ToChecked());
        } else if(arg->IsNumber()) {
            str += std::to_string(arg->NumberValue(context).ToChecked());
        } else if(arg->IsString()) {
            String::Utf8Value value(isolate, arg);
            str += *value;
        } else if(arg->IsArray()) {
            Local<Array> array = Local<Array>::Cast(arg);
            str += "[";
            for(int i = 0; i < array->Length(); i++) {
                if(i > 0) {
                    str += ", ";
                }
                Local<Value> element = array->Get(context, i).ToLocalChecked();
                if(element->IsInt32()) {
                    str += std::to_string(element->Int32Value(context).ToChecked());
                } else if(element->IsNumber()) {
                    str += std::to_string(element->NumberValue(context).ToChecked());
                } else if(element->IsString()) {
                    String::Utf8Value value(isolate, element);
                    str += *value;
                }
            }
            str += "]";
        } else if(arg->IsObject()) {
            v8::Local<v8::String> tmp = v8::JSON::Stringify(context, arg).ToLocalChecked();
            v8::String::Utf8Value value(isolate, tmp);
            str += *value;
        } 
        
    }
    // 打印参数个数
    int length = args.Length();
    printf("Number of arguments: %d\n", length);
    printf("content: %s\n", str.c_str());
    
}



void Initialize(Local<Object> target,
                Local<Value> unused,
                Local<Context> context,
                void* priv) {
  Environment* env = Environment::GetCurrent(context);
  SetMethod(context, target, "log", Log);
  target
      ->Set(context,
            FIXED_ONE_BYTE_STRING(env->isolate(), "isBigEndian"),
            Boolean::New(env->isolate(), IsBigEndian()))
      .Check();
}

void RegisterExternalReferences(ExternalReferenceRegistry* registry) {
  registry->Register(Log);
}

}  // namespace os
}  // namespace node

NODE_BINDING_CONTEXT_AWARE_INTERNAL(my_console, node::my_console::Initialize)
NODE_BINDING_EXTERNAL_REFERENCE(my_console, node::my_console::RegisterExternalReferences)
