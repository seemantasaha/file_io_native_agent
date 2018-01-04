#include <jvmti.h>
#include <memory.h>
#include <iostream>
#include <set>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <cassert>
#include <ctime>

#include "file-io-event.h"

using namespace std;

ofstream myfile;

//static jfieldID fis_fd;
//static jfieldID IO_fd_fdID;
void (*open1_orig) (JNIEnv *env, jobject thisObj, jstring filename, jboolean append);
void (*open2_orig) (JNIEnv *env, jobject thisObj, jstring filename);
void (*close_orig) (JNIEnv *env, jobject thisObj);
void (*read0_orig) (JNIEnv *env, jobject thisObj);
void (*readBytes_orig) (JNIEnv *env, jobject thisObj, jbyteArray b, jint off, jint len);
void (*write_orig) (JNIEnv *env, jobject thisObj, jint val, jboolean append);
void (*writeBytes_orig) (JNIEnv *env, jobject thisObj, jbyteArray b, jint off, jint len, jboolean append);
//void (*socketWrite0_orig) (JNIEnv *env, jobject thisObj, jobject fdObj, jbyteArray data, jint off, jint len);
void (*UnixNativeDispatcher_open0_orig) (JNIEnv *env, jobject thisObj, jlong pathAddress, jint flags, jint mode);
void (*FileDispatcherImpl_close0_orig) (JNIEnv *env, jobject thisObj, jobject fdObj);
void (*FileDispatcherImpl_read0_orig) (JNIEnv *env, jobject thisObj, jobject fdObj, jlong addr, jint len);
void (*FileDispatcherImpl_write0_orig) (JNIEnv *env, jobject thisObj, jobject fdObj, jlong addr, jint len);

char* split_to_get_filename(const char* yourStr) {
    char str[1000];
    strncpy(str, yourStr, sizeof(str));
    char* parts[100] = {0};
    unsigned int index = 0;
    parts[index] = strtok(str,"/");

    while(parts[index] != 0)
    {
      ++index;
      parts[index] = strtok(0, "/");
    }
    return strdup(parts[index-1]);
} 

const char* GetClassName(JNIEnv *env, jobject obj, jclass cls) {
  // First get the class object
  jmethodID mid = env->GetMethodID(cls, "getClass", "()Ljava/lang/Class;");
  jobject clsObj = env->CallObjectMethod(obj, mid);

  // Now get the class object's class descriptor
  cls = env->GetObjectClass(clsObj);

  // Find the getName() method on the class object
  mid = env->GetMethodID(cls, "getName", "()Ljava/lang/String;");

  // Call the getName() to get a jstring object back
  jstring strObj = (jstring)env->CallObjectMethod(clsObj, mid);

  // Now get the c string from the java jstring object
  const char* name = env->GetStringUTFChars(strObj, NULL);

  return name;

  // Release the memory pinned char array
  //env->ReleaseStringUTFChars(strObj, str);
}


void JNICALL callbackVMInit(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread)
{
  cout << "[AGENT] VM init" << endl;

  myfile.open ("output/log.csv");
  myfile << "Event Type,File Name,File Location,Content,Offset,Length,Timestamp\n";
}


void JNICALL callbackVMDeath(jvmtiEnv *jvmti_env, JNIEnv* jni_env)
{
  cout << "[AGENT] VM done" << endl;
  myfile.close();
}


void JNICALL callbackMethodEntry(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method) {
  char* method_name;
  char* signature;
  char* class_generic;
  
  jvmti_env->GetMethodName(method, &method_name, &signature, &class_generic);

  cout << "[AGENT][callbackMethodEntry] " << method_name << endl;
  
  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(method_name));
  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(signature));
  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(class_generic));
}

JNIEXPORT void JNICALL fuzzer_UnixNativeDispatcher_open0 (JNIEnv *env, jobject thisObj, jlong pathAddress, jint flags, jint mode) {
  cout << "File opened" << endl;
  cout << pathAddress << endl;

  time_t now = time(0);
  //char *dt = ctime(&now);
  FileIOEvent *fie = new FileIOEvent(now, open);
  fie->set_file_info("N/A", "N/A");
  fie->set_content_info("N/A", 0, 0);
  fie->save_to_csv();

  UnixNativeDispatcher_open0_orig(env, thisObj, pathAddress, flags, mode);
}

JNIEXPORT void JNICALL fuzzer_FileStream_open1(JNIEnv *env, jobject thisObj, jstring filename, jboolean append) {
  const char *name = env->GetStringUTFChars(filename, 0);
  cout << "File opened : " << name << endl;

  time_t now = time(0);
  //char *dt = ctime(&now);
  FileIOEvent *fie = new FileIOEvent(now, open);
  fie->set_file_info(split_to_get_filename(name), name);
  fie->set_content_info("N/A", 0, 0);
  fie->save_to_csv();

  env->ReleaseStringUTFChars(filename, name);
  open1_orig(env, thisObj, filename, append);
}

JNIEXPORT void JNICALL fuzzer_FileStream_open2(JNIEnv *env, jobject thisObj, jstring filename) {
  const char *name = env->GetStringUTFChars(filename, 0);
  cout << "File opened : " << name << endl;

  time_t now = time(0);
  //char *dt = ctime(&now);
  FileIOEvent *fie = new FileIOEvent(now, open);
  fie->set_file_info(split_to_get_filename(name), name);
  fie->set_content_info("N/A", 0, 0);
  fie->save_to_csv();

  env->ReleaseStringUTFChars(filename, name);
  open2_orig(env, thisObj, filename);
}

JNIEXPORT void JNICALL fuzzer_FileStream_close(JNIEnv *env, jobject thisObj) {
  cout << "File closed" << endl;

  jclass cls = env->GetObjectClass(thisObj);
  assert(cls != NULL);
  
  jfieldID fieldID = env->GetFieldID(cls, "path", "Ljava/lang/String;");
  if (fieldID == NULL)
    return;

  jstring str = (jstring) env->GetObjectField(thisObj, fieldID);
  if(str != NULL)
  {
    const char *cstr = env->GetStringUTFChars(str, 0);
    time_t now = time(0);
    //char *dt = ctime(&now);
    FileIOEvent *fie = new FileIOEvent(now, close);
    fie->set_file_info(split_to_get_filename(cstr), cstr);
    fie->set_content_info("N/A", 0, 0);
    fie->save_to_csv();
    env->ReleaseStringUTFChars(str, cstr);
  }

  close_orig(env, thisObj);
}

JNIEXPORT void JNICALL fuzzer_FileDispatcherImpl_close0(JNIEnv *env, jobject thisObj, jobject fdObj) {
  cout << "File closed" << endl;

  time_t now = time(0);
  //char *dt = ctime(&now);
  FileIOEvent *fie = new FileIOEvent(now, close);
  fie->set_file_info("N/A", "N/A");
  fie->set_content_info("N/A", 0, 0);
  fie->save_to_csv();

  FileDispatcherImpl_close0_orig(env, thisObj, fdObj);
}

JNIEXPORT void JNICALL fuzzer_FileDispatcherImpl_read0(JNIEnv *env, jobject thisObj, jobject fdObj, jlong addr, jint len) {
  cout << "[AGENT] [FileDispatcherImpl_read0] len=" << len << endl;

  time_t now = time(0);
  //char *dt = ctime(&now);
  FileIOEvent *fie = new FileIOEvent(now, read);
  fie->set_file_info("N/A", "N/A");
  fie->set_content_info("N/A", 0, len);
  fie->save_to_csv();

  FileDispatcherImpl_read0_orig(env, thisObj, fdObj, addr, len);
}

JNIEXPORT void JNICALL fuzzer_FileInputStream_read0(JNIEnv *env, jobject thisObj) {
  jclass cls = env->GetObjectClass(thisObj);
  assert(cls != NULL);
  
  jfieldID fieldID = env->GetFieldID(cls, "path", "Ljava/lang/String;");
  //assert(fieldID != NULL);
  if (fieldID == NULL)
    return;
  jstring str = (jstring) env->GetObjectField(thisObj, fieldID);

  if(str != NULL)
  {
    const char *cstr = env->GetStringUTFChars(str, 0);
    cout << "[AGENT] [FileInputStream_read] this.path=" << cstr << endl;
    cout << "[AGENT] [FileInputStream_read] data= N/A" << endl;
    cout << "[AGENT] [FileInputStream_read] off=" << "0" << endl;
    cout << "[AGENT] [FileInputStream_read] len=" << "1" << endl;


    time_t now = time(0);
    //char *dt = ctime(&now);
    FileIOEvent *fie = new FileIOEvent(now, read);
    fie->set_file_info(split_to_get_filename(cstr), cstr);
    fie->set_content_info("N/A", 0, 1); //Get content and replace N/A  
    fie->save_to_csv();

    env->ReleaseStringUTFChars(str, cstr);
  }
  
  read0_orig(env, thisObj);
}

JNIEXPORT void JNICALL fuzzer_FileInputStream_readBytes(JNIEnv *env, jobject thisObj, jbyteArray b, jint off, jint len) {
  jclass cls = env->GetObjectClass(thisObj);
  assert(cls != NULL);
  jfieldID fieldID = env->GetFieldID(cls, "path", "Ljava/lang/String;");
  assert(fieldID != NULL);
  jstring str = (jstring) env->GetObjectField(thisObj, fieldID);
  if(str != NULL)
  {
    const char *cstr = env->GetStringUTFChars(str, 0);
    const char* oracle = strstr(cstr,"oracle");
    const char* appclass = strstr(cstr,"class");
    if (oracle == NULL && appclass == NULL) {
      cout << "[AGENT] [FileInputStream_readBytes] this.path=" << cstr << endl;
      cout << "[AGENT] [FileInputStream_readBytes] off=" << off << endl;
      cout << "[AGENT] [FileInputStream_readBytes] len=" << len << endl;


      time_t now = time(0);
      //char *dt = ctime(&now);
      FileIOEvent *fie = new FileIOEvent(now, read);
      fie->set_file_info(split_to_get_filename(cstr), cstr);
      fie->set_content_info("N/A", off, len);//Get content and replace N/A
      fie->save_to_csv();
    }
    env->ReleaseStringUTFChars(str, cstr);
  }

  /*jboolean isCopy;
  jbyte* a = env->GetByteArrayElements(b,&isCopy);
  char* d = (char*)a;
  d[len-off] = '\0';
  printf("[AGENT] [FileOutputStream_readBytes] data= %s\n",d);
  env->ReleaseByteArrayElements(b, a, 0);*/
  
  readBytes_orig(env, thisObj, b, off, len);
}

JNIEXPORT void JNICALL fuzzer_FileDispatcherImpl_write0(JNIEnv *env, jobject thisObj, jobject fdObj, jlong addr, jint len) {
  cout << "[AGENT] [FileDispatcherImpl_write0] len=" << len << endl;

  time_t now = time(0);
  //char *dt = ctime(&now);
  FileIOEvent *fie = new FileIOEvent(now, write);
  fie->set_file_info("N/A", "N/A");
  fie->set_content_info("N/A", 0, len);
  fie->save_to_csv();

  FileDispatcherImpl_write0_orig(env, thisObj, fdObj, addr, len);
}

// Our native method to replace SocketOutputStream.write0
/*JNIEXPORT void JNICALL fuzzer_SocketOutputStream_write0(JNIEnv *env, jobject thisObj, jobject fdObj, jbyteArray b, jint off, jint len) {
  cout << "[AGENT] [SocketOutputStream_write0] len=" << len << endl;
  socketWrite0_orig(env, thisObj, fdObj, b, off, len);
}*/

JNIEXPORT void JNICALL fuzzer_FileOutputStream_write(JNIEnv *env, jobject thisObj, jint val, jboolean append) {
  jclass cls = env->GetObjectClass(thisObj);
  assert(cls != NULL);
  jfieldID fieldID = env->GetFieldID(cls, "path", "Ljava/lang/String;");
  assert(fieldID != NULL);
  jstring str = (jstring) env->GetObjectField(thisObj, fieldID);
  if(str != NULL)
  {
    const char *cstr = env->GetStringUTFChars(str, 0);
    cout << "[AGENT] [FileOutputStream_write] this.path=" << cstr << endl;
    cout << "[AGENT] [FileOutputStream_write] data=" << val << endl;
    cout << "[AGENT] [FileOutputStream_write] off=" << "0" << endl;
    cout << "[AGENT] [FileOutputStream_write] len=" << "1" << endl;

    time_t now = time(0);
    //char *dt = ctime(&now);
    FileIOEvent *fie = new FileIOEvent(now, write);
    fie->set_file_info(split_to_get_filename(cstr), cstr);
    fie->set_content_info("N/A", 0, 1);//Get content and replace N/A
    fie->save_to_csv();
    
    env->ReleaseStringUTFChars(str, cstr);
  }
  
  write_orig(env, thisObj, val, append);
}

JNIEXPORT void JNICALL fuzzer_FileOutputStream_writeBytes(JNIEnv *env, jobject thisObj, jbyteArray b, jint off, jint len, jboolean append) {
  jclass cls = env->GetObjectClass(thisObj);
  assert(cls != NULL);
  jfieldID fieldID = env->GetFieldID(cls, "path", "Ljava/lang/String;");
  assert(fieldID != NULL);
  jstring str = (jstring) env->GetObjectField(thisObj, fieldID);
  if(str != NULL)
  {
    const char *cstr = env->GetStringUTFChars(str, 0);
    cout << "[AGENT] [FileOutputStream_writeBytes] filepath=" << cstr << endl;
    cout << "[AGENT] [FileOutputStream_writeBytes] off=" << off << endl;
    cout << "[AGENT] [FileOutputStream_writeBytes] len=" << len << endl;

    jboolean isCopy;
    jbyte* a = env->GetByteArrayElements(b,&isCopy);
    char* c = (char*)a;
    c[len-off] = '\0';
    
    time_t now = time(0);
    //char *dt = ctime(&now);
    FileIOEvent *fie = new FileIOEvent(now, write);
    fie->set_file_info(split_to_get_filename(cstr), cstr);
    fie->set_content_info("N/A", off, len);//Get content and replace N/A
    fie->save_to_csv();

    env->ReleaseByteArrayElements(b, a, 0);
    env->ReleaseStringUTFChars(str, cstr);
  }
  //printf("[AGENT] [FileOutputStream_writeBytes] data= %s\n",c);

  writeBytes_orig(env, thisObj, b, off, len, append);
}


void JNICALL callbackNativeMethodBind(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method, void* address, void** new_address_ptr) {
  char* method_name;
  char* signature;
  char *class_signature;
  char *class_generic;

  jvmti_env->GetMethodName(method, &method_name, &signature, &class_generic);
  jclass cls;
  jvmti_env->GetMethodDeclaringClass(method, &cls);
  jvmti_env->GetClassSignature(cls, &class_signature, &class_generic);
  
  //cout << "nativemethodbind: " << class_signature << " " << method_name << " " << signature << endl;
  
  /*if (!strcmp(class_signature,"Ljava/io/FileDescriptor;")) {

        IO_fd_fdID = jni_env->GetFieldID(cls, "fd", "I");

  } else if (!strcmp(class_signature, "Ljava/net/SocketOutputStream;")) {

    if (!strcmp(method_name, "socketWrite0")) {
      cout << "nativemethodbind: " << class_signature << " " << method_name << endl;
      *new_address_ptr = (void*)fuzzer_SocketOutputStream_write0;
      socketWrite0_orig = (void (*)(JNIEnv*, jobject, jobject, jbyteArray, jint, jint))address;
    }

  } else*/ 
  if (!strcmp(class_signature, "Lsun/nio/ch/FileDispatcherImpl;")) {

    if (!strcmp(method_name, "write0")) {
      //cout << "nativemethodbind: " << class_signature << " " << method_name << endl;
      *new_address_ptr = (void*)fuzzer_FileDispatcherImpl_write0;
      FileDispatcherImpl_write0_orig = (void (*)(JNIEnv*, jobject, jobject, jlong, jint))address;
    }

    if (!strcmp(method_name, "read0")) {
      //cout << "nativemethodbind: " << class_signature << " " << method_name << endl;
      *new_address_ptr = (void*)fuzzer_FileDispatcherImpl_read0;
      FileDispatcherImpl_read0_orig = (void (*)(JNIEnv*, jobject, jobject, jlong, jint))address;
    }

    if (!strcmp(method_name, "close0")) {
      //cout << "nativemethodbind: " << class_signature << " " << method_name << endl;
      *new_address_ptr = (void*)fuzzer_FileDispatcherImpl_close0;
      FileDispatcherImpl_close0_orig = (void (*)(JNIEnv*, jobject, jobject))address;
    }

  } else if (!strcmp(class_signature, "Lsun/nio/fs/UnixNativeDispatcher;")) {

    if (!strcmp(method_name, "open0")) {
      //cout << "nativemethodbind: " << class_signature << " " << method_name << endl;
      *new_address_ptr = (void*)fuzzer_UnixNativeDispatcher_open0;
      UnixNativeDispatcher_open0_orig = (void (*)(JNIEnv*, jobject, jlong, jint, jint))address;
    }

  } else {

    if (!strcmp(method_name,"writeBytes")) {
      //cout << "nativemethodbind: " << class_signature << " " << method_name << endl;
      *new_address_ptr = (void*)fuzzer_FileOutputStream_writeBytes;
      writeBytes_orig = (void (*)(JNIEnv*, jobject, jbyteArray, jint, jint, jboolean))address;
    }

    else if (!strcmp(method_name,"write")) {
      //cout << "nativemethodbind: " << class_signature << " " << method_name << endl;
      *new_address_ptr = (void*)fuzzer_FileOutputStream_write;
      write_orig = (void (*)(JNIEnv*, jobject, jint, jboolean))address;
    }

    if (!strcmp(method_name,"readBytes")) {
      //cout << "nativemethodbind: " << class_signature << " " << method_name << endl;
      *new_address_ptr = (void*)fuzzer_FileInputStream_readBytes;
      readBytes_orig = (void (*)(JNIEnv*, jobject, jbyteArray, jint, jint))address;
    }

    else if (!strcmp(method_name,"read0")) {
      //cout << "nativemethodbind: " << class_signature << " " << method_name << endl;
      *new_address_ptr = (void*)fuzzer_FileInputStream_read0;
      read0_orig = (void (*)(JNIEnv*, jobject))address;
    }

    if (!strcmp(method_name,"open0")) {
      if (!strcmp(signature,"(Ljava/lang/String;Z)V")) {
        *new_address_ptr = (void*)fuzzer_FileStream_open1;
        open1_orig = (void (*)(JNIEnv*, jobject, jstring, jboolean))address;
      } else {
        *new_address_ptr = (void*)fuzzer_FileStream_open2;
        open2_orig = (void (*)(JNIEnv*, jobject, jstring))address;
      }
    }

    if (!strcmp(method_name,"close0")) {
      *new_address_ptr = (void*)fuzzer_FileStream_close;
      close_orig = (void (*)(JNIEnv*, jobject))address;
    }
  }

  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(method_name));
  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(signature));
  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(class_generic));
  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(class_signature));
}

void JNICALL callbackCompiledMethodLoad (jvmtiEnv *jvmti_env, jmethodID method, jint code_size, const void* code_addr,
          jint map_length, const jvmtiAddrLocationMap* map, const void* compile_info) {
  char* method_name;
  char* signature;
  char *class_signature;
  char *class_generic;

  jvmti_env->GetMethodName(method, &method_name, &signature, &class_generic);
  jclass cls;
  jvmti_env->GetMethodDeclaringClass(method, &cls);
  jvmti_env->GetClassSignature(cls, &class_signature, &class_generic);
  
  cout << "CompiledMethodLoad: " << class_signature << " " << method_name << endl;

  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(method_name));
  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(signature));
  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(class_generic));
  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(class_signature));
}

void JNICALL callbackClassLoad (jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jclass klass) {
  char *class_signature;
  char *class_generic;
  int number_of_methods;
  jmethodID *method_ids;
  jvmti_env->GetClassSignature(klass, &class_signature, &class_generic);
  cout<<class_signature<<endl;
  if (!strcmp(class_signature,"Ljava/io/PrintWriter;")) {
    jvmti_env->GetClassMethods(klass, &number_of_methods, &method_ids);
    cout<< "Number of methods : " << number_of_methods << endl;
  }

  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(class_generic));
  jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(class_signature));
}


JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved)
{
  static jvmtiEnv *jvmti = NULL;
  static jvmtiCapabilities capa;

  jvmtiError error;
  jvm->GetEnv((void**) &jvmti, JVMTI_VERSION_1_0);

  (void)memset(&capa, 0, sizeof(jvmtiCapabilities));
  //capa.can_access_local_variables = 1;
  //capa.can_generate_method_entry_events = 1;
  //capa.can_generate_method_exit_events = 1;
  //capa.can_generate_exception_events = 1;
  //capa.can_generate_single_step_events = opts.mode==Options::MODE_COUNTMETHODS?0:1;
  //capa.can_tag_objects = 1;
  capa.can_generate_native_method_bind_events = 1;

  error = jvmti->AddCapabilities(&capa);
  error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, (jthread)NULL);
  error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_DEATH, (jthread)NULL);
  //error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, (jthread)NULL);
  //error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_LOAD, (jthread)NULL);
  error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_NATIVE_METHOD_BIND, (jthread)NULL);
  //error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_COMPILED_METHOD_LOAD, (jthread)NULL);

  jvmtiEventCallbacks callbacks;
  (void)memset(&callbacks, 0, sizeof(callbacks));    
  callbacks.VMInit = &callbackVMInit;
  callbacks.VMDeath = &callbackVMDeath;
  //callbacks.SingleStep = &callbackSingleStep_Count;    
  //callbacks.MethodEntry = &callbackMethodEntry;
  //callbacks.ClassLoad = &callbackClassLoad; 
  callbacks.NativeMethodBind = &callbackNativeMethodBind;
  //callbacks.CompiledMethodLoad = &callbackCompiledMethodLoad;
  error = jvmti->SetEventCallbacks(&callbacks,(jint)sizeof(callbacks));

  return JNI_OK;
}
