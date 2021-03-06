/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <string.h>
#include <stdio.h>
#include <jni.h>

#include <android/log.h>

#define APPNAME "JSCTest"
#define log(...) __android_log_print(ANDROID_LOG_ERROR, APPNAME, __VA_ARGS__)

#include <JavaScriptCore/JavaScriptCore.h>

#if defined(__arm__)
  #if defined(__ARM_ARCH_7A__)
    #if defined(__ARM_NEON__)
      #define ABI "armeabi-v7a/NEON"
    #else
      #define ABI "armeabi-v7a"
    #endif
  #else
   #define ABI "armeabi"
  #endif
#elif defined(__i386__)
   #define ABI "x86"
#elif defined(__mips__)
   #define ABI "mips"
#else
   #define ABI "unknown"
#endif

/* This is a trivial JNI example where we use a native method
 * to return a new VM String. See the corresponding Java source
 * file located at:
 *
 *   src/com/adcolony/jsctest/JSCTest.java
 */
jstring
Java_com_adcolony_jsctest_JSCTest_stringFromJNI(JNIEnv* env, jobject self)
{
     return (*env)->NewStringUTF(env, "Hello from JNI!\nThis is JSCTest.\nCompiled with ABI " ABI ".");
}

/* This is a JNI/JSC example where we use a native method to return a
 * new VM String that we get from executing a JavaScript fragment. See
 * the corresponding Java source file located at:
 *
 *   src/com/adcolony/jsctest/JSCTest.java
 */
jstring
Java_com_adcolony_jsctest_JSCTest_stringFromJSC(JNIEnv *env, jobject self)
{
     char scriptStr[] = "var main = function () { return 'Hello from JSC!\\n' }";
     char jsValue[128];

     evaluateScript(scriptStr, "main", jsValue);

     return (*env)->NewStringUTF(env, jsValue);
}

void
evaluateScript(const char *src, const char *entryFn, char *retStr)
{
     JSValueRef exception;

     JSGlobalContextRef ctx = JSGlobalContextCreate(NULL);
     JSObjectRef jsGlobalObject = JSContextGetGlobalObject(ctx);

     JSStringRef scriptJS = JSStringCreateWithUTF8CString(src);
     //bool validScript = JSCheckScriptSyntax(ctx, scriptJS, NULL, 0, exception);
     JSValueRef ret = JSEvaluateScript(ctx, scriptJS, NULL, NULL, 0, &exception);
     JSStringRelease(scriptJS);

     JSStringRef nameJS = JSStringCreateWithUTF8CString(entryFn);
     //bool hasMain = JSObjectHasProperty(ctx, jsGlobalObject, nameJS);
     JSObjectRef function = (JSObjectRef)JSObjectGetProperty(ctx, jsGlobalObject, nameJS, NULL);
     JSStringRelease(nameJS);

     JSValueRef args[] = {};
     JSValueRef result = JSObjectCallAsFunction(ctx, function, NULL, 0, args, &exception);
     JSStringRef resultStr = JSValueToStringCopy(ctx, result, NULL);
     JSStringGetUTF8CString(resultStr, retStr, 128);
     JSStringRelease(resultStr);

     JSGlobalContextRelease(ctx);
}

jstring
Java_com_adcolony_jsctest_JSCTest_runTest(JNIEnv *env, jobject self, jbyteArray src)
{
     char *csrc = (char *)(*env)->GetByteArrayElements(env, src, NULL);
     jsize clen = (*env)->GetArrayLength(env, src);
     char retValue[128];

     csrc[clen] = '\0';
     evaluateScript(csrc, "test", retValue);

     return (*env)->NewStringUTF(env, retValue);
}
