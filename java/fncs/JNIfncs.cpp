#include <cassert>
#include <string>
#include <vector>

#include "fncs_JNIfncs.h"
#include "fncs.hpp"

JNIEXPORT void JNICALL Java_fncs_JNIfncs_initialize__
  (JNIEnv *, jclass)
{
    fncs::initialize();
}

JNIEXPORT void JNICALL Java_fncs_JNIfncs_initialize__Ljava_lang_String_2
  (JNIEnv *env, jclass, jstring j_configuration)
{
    const char *c_configuration = NULL;
    jsize configuration_size = 0;
    std::string cpp_configuration;

    c_configuration = env->GetStringUTFChars(j_configuration, NULL);
    configuration_size = env->GetStringLength(j_configuration);
    cpp_configuration.assign(c_configuration, configuration_size);

    fncs::initialize(cpp_configuration);

    env->ReleaseStringUTFChars(j_configuration, c_configuration);
}

JNIEXPORT void JNICALL Java_fncs_JNIfncs_agentRegister__
  (JNIEnv *, jclass)
{
	fncs::agentRegister();
}

JNIEXPORT void JNICALL Java_fncs_JNIfncs_agentRegister__Ljava_lang_String_2
  (JNIEnv *env, jclass, jstring j_configuration)
{
	const char *c_configuration = NULL;
	jsize configuration_size = 0;
	std::string cpp_configuration;

	c_configuration = env->GetStringUTFChars(j_configuration, NULL);
	configuration_size = env->GetStringLength(j_configuration);
	cpp_configuration.assign(c_configuration, configuration_size);

	fncs::agentRegister(cpp_configuration);

	env->ReleaseStringUTFChars(j_configuration, c_configuration);
}

JNIEXPORT jboolean JNICALL Java_fncs_JNIfncs_is_1initialized
  (JNIEnv *, jclass)
{
    return static_cast<jboolean>(fncs::is_initialized());
}

JNIEXPORT jlong JNICALL Java_fncs_JNIfncs_time_1request
  (JNIEnv *, jclass, jlong j_requested)
{
    jlong j_granted;
    fncs::time cpp_requested;
    fncs::time cpp_granted;

    /* jlong is unsigned, make sure we have a positive value */
    assert(j_requested >= 0);

    cpp_requested = static_cast<fncs::time>(j_requested);
    cpp_granted = fncs::time_request(cpp_requested);
    j_granted = static_cast<jlong>(cpp_granted);

    return j_granted;
}

JNIEXPORT void JNICALL Java_fncs_JNIfncs_publish
  (JNIEnv *env, jclass, jstring j_key, jstring j_value)
{
    const char *c_key = NULL;
    jsize key_size = 0;
    std::string cpp_key;
    const char *c_value = NULL;
    jsize value_size = 0;
    std::string cpp_value;

    c_key = env->GetStringUTFChars(j_key, NULL);
    key_size = env->GetStringLength(j_key);
    cpp_key.assign(c_key, key_size);
    c_value = env->GetStringUTFChars(j_value, NULL);
    value_size = env->GetStringLength(j_value);
    cpp_value.assign(c_value, value_size);

    fncs::publish(cpp_key, cpp_value);

    env->ReleaseStringUTFChars(j_key, c_key);
    env->ReleaseStringUTFChars(j_value, c_value);
}

JNIEXPORT void JNICALL Java_fncs_JNIfncs_publish_1anon
  (JNIEnv *env, jclass, jstring j_key, jstring j_value)
{
    const char *c_key = NULL;
    jsize key_size = 0;
    std::string cpp_key;
    const char *c_value = NULL;
    jsize value_size = 0;
    std::string cpp_value;

    c_key = env->GetStringUTFChars(j_key, NULL);
    key_size = env->GetStringLength(j_key);
    cpp_key.assign(c_key, key_size);
    c_value = env->GetStringUTFChars(j_value, NULL);
    value_size = env->GetStringLength(j_value);
    cpp_value.assign(c_value, value_size);

    fncs::publish_anon(cpp_key, cpp_value);

    env->ReleaseStringUTFChars(j_key, c_key);
    env->ReleaseStringUTFChars(j_value, c_value);
}

JNIEXPORT void JNICALL Java_fncs_JNIfncs_agentPublish
  (JNIEnv *env, jclass, jstring j_value)
{
	const char *c_value = NULL;
	jsize value_size = 0;
	std::string cpp_value;

	c_value = env->GetStringUTFChars(j_value, NULL);
	value_size = env->GetStringLength(j_value);
	cpp_value.assign(c_value, value_size);

	fncs::agentPublish(cpp_value);

	env->ReleaseStringUTFChars(j_value, c_value);
}

JNIEXPORT void JNICALL Java_fncs_JNIfncs_route
  (JNIEnv *env, jclass, jstring j_from, jstring j_to, jstring j_key, jstring j_value)
{
    const char *c_from = NULL;
    jsize from_size = 0;
    std::string cpp_from;
    const char *c_to = NULL;
    jsize to_size = 0;
    std::string cpp_to;
    const char *c_key = NULL;
    jsize key_size = 0;
    std::string cpp_key;
    const char *c_value = NULL;
    jsize value_size = 0;
    std::string cpp_value;

    c_from = env->GetStringUTFChars(j_from, NULL);
    from_size = env->GetStringLength(j_from);
    cpp_from.assign(c_from, from_size);
    c_to = env->GetStringUTFChars(j_to, NULL);
    to_size = env->GetStringLength(j_to);
    cpp_to.assign(c_to, to_size);
    c_key = env->GetStringUTFChars(j_key, NULL);
    key_size = env->GetStringLength(j_key);
    cpp_key.assign(c_key, key_size);
    c_value = env->GetStringUTFChars(j_value, NULL);
    value_size = env->GetStringLength(j_value);
    cpp_value.assign(c_value, value_size);

    fncs::route(cpp_from, cpp_to, cpp_key, cpp_value);

    env->ReleaseStringUTFChars(j_from, c_from);
    env->ReleaseStringUTFChars(j_to, c_to);
    env->ReleaseStringUTFChars(j_key, c_key);
    env->ReleaseStringUTFChars(j_value, c_value);
}

JNIEXPORT void JNICALL Java_fncs_JNIfncs_die
  (JNIEnv *, jclass)
{
    fncs::die();
}

JNIEXPORT void JNICALL Java_fncs_JNIfncs_end
  (JNIEnv *, jclass)
{
    fncs::finalize();
}

JNIEXPORT void JNICALL Java_fncs_JNIfncs_update_1time_1delta
  (JNIEnv *, jclass, jlong j_delta)
{
    fncs::time cpp_delta;

    /* jlong is unsigned, make sure we have a positive value */
    assert(j_delta >= 0);

    cpp_delta = static_cast<fncs::time>(j_delta);
    fncs::update_time_delta(cpp_delta);
}

JNIEXPORT jobjectArray JNICALL Java_fncs_JNIfncs_get_1events
  (JNIEnv *env, jclass)
{
    jobjectArray ret;
    jsize size;
    std::vector<std::string> events;

    events = fncs::get_events();
    size = static_cast<jsize>(events.size());
    ret = env->NewObjectArray(size,
            env->FindClass("java/lang/String"),
            env->NewStringUTF(""));
    for (jsize i=0; i<size; ++i) {
        env->SetObjectArrayElement(ret, i,
                env->NewStringUTF(events[i].c_str()));
    }

    return ret;
}

JNIEXPORT jstring JNICALL Java_fncs_JNIfncs_agentGetEvents
  (JNIEnv *env, jclass)
{
	jstring ret;
	jsize size;
	std::string events;

	events = fncs::agentGetEvents();
	ret = env->NewStringUTF(events.c_str());
	return ret;
}

JNIEXPORT jstring JNICALL Java_fncs_JNIfncs_get_1value
  (JNIEnv *env, jclass, jstring j_key)
{
    std::string cpp_value;
    const char *c_key = NULL;
    jsize key_size = 0;
    std::string cpp_key;

    c_key = env->GetStringUTFChars(j_key, NULL);
    key_size = env->GetStringLength(j_key);
    cpp_key.assign(c_key, key_size);

    cpp_value = fncs::get_value(cpp_key);

    env->ReleaseStringUTFChars(j_key, c_key);

    return env->NewStringUTF(cpp_value.c_str());
}

JNIEXPORT jobjectArray JNICALL Java_fncs_JNIfncs_get_1values
  (JNIEnv *env, jclass, jstring j_key)
{
    jobjectArray ret;
    jsize size;
    const char *c_key = NULL;
    jsize key_size = 0;
    std::string cpp_key;
    std::vector<std::string> values;

    c_key = env->GetStringUTFChars(j_key, NULL);
    key_size = env->GetStringLength(j_key);
    cpp_key.assign(c_key, key_size);

    values = fncs::get_values(cpp_key);

    size = static_cast<jsize>(values.size());
    ret = env->NewObjectArray(size,
            env->FindClass("java/lang/String"),
            env->NewStringUTF(""));
    for (jsize i=0; i<size; ++i) {
        env->SetObjectArrayElement(ret, i,
                env->NewStringUTF(values[i].c_str()));
    }

    return ret;
}

JNIEXPORT jobjectArray JNICALL Java_fncs_JNIfncs_get_1keys
  (JNIEnv *env, jclass)
{
    jobjectArray ret;
    jsize size;
    std::vector<std::string> keys;

    keys = fncs::get_keys();
    size = static_cast<jsize>(keys.size());
    ret = env->NewObjectArray(size,
            env->FindClass("java/lang/String"),
            env->NewStringUTF(""));
    for (jsize i=0; i<size; ++i) {
        env->SetObjectArrayElement(ret, i,
                env->NewStringUTF(keys[i].c_str()));
    }

    return ret;
}

JNIEXPORT jstring JNICALL Java_fncs_JNIfncs_get_1name
  (JNIEnv *env, jclass)
{
    std::string cpp_name;

    cpp_name = fncs::get_name();

    return env->NewStringUTF(cpp_name.c_str());
}

JNIEXPORT jlong JNICALL Java_fncs_JNIfncs_get_1time_1delta
  (JNIEnv *, jclass)
{
    fncs::time cpp_delta;
    jlong j_delta;

    cpp_delta = fncs::get_time_delta();
    j_delta = static_cast<jlong>(cpp_delta);

    return j_delta;
}

JNIEXPORT jlong JNICALL Java_fncs_JNIfncs_get_1id
  (JNIEnv *, jclass)
{
    fncs::time cpp_id;
    jlong j_id;

    cpp_id = fncs::get_id();
    j_id = static_cast<jlong>(cpp_id);

    return j_id;
}

JNIEXPORT jlong JNICALL Java_fncs_JNIfncs_get_1simulator_1count
  (JNIEnv *, jclass)
{
    fncs::time cpp_count;
    jlong j_count;

    cpp_count = fncs::get_simulator_count();
    j_count = static_cast<jlong>(cpp_count);

    return j_count;
}

JNIEXPORT jintArray JNICALL Java_fncs_JNIfncs_get_1version
  (JNIEnv *env, jclass)
{
    jintArray ret;
    jint *arr;
    int major;
    int minor;
    int patch;

    fncs::get_version(&major, &minor, &patch);

    ret = env->NewIntArray(3);
    arr = env->GetIntArrayElements(ret, NULL);
    arr[0] = static_cast<jint>(major);
    arr[1] = static_cast<jint>(minor);
    arr[2] = static_cast<jint>(patch);
    env->ReleaseIntArrayElements(ret, arr, 0);

    return ret;
}

