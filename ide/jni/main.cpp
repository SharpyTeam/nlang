//
// Created by selya on 05.01.2020.
//

#include "org_nlang_JNI.h"

#include <parser/scanner.hpp>

#include <jni.h>
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>


extern "C" JNIEXPORT void JNICALL Java_org_nlang_JNI_tokenize(JNIEnv* env, jclass, jobject source, jint start, jint end, jobject tokens_map) {
    jclass CharSequence_class = env->FindClass("java/lang/CharSequence");
    //jmethodID CharSequence_length = env->GetMethodID(CharSequence_class, "length", "()I");
    jmethodID CharSequence_charAt = env->GetMethodID(CharSequence_class, "charAt", "(I)C");

    jclass TreeMap_class = env->FindClass("java/util/TreeMap");
    jmethodID TreeMap_put = env->GetMethodID(TreeMap_class, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    //gjint length = env->CallIntMethod(source, CharSequence_length);

    icu::UnicodeString s(end - start, 0, 0);
    for (jint i = start; i < end; ++i) {
        jchar code = env->CallCharMethod(source, CharSequence_charAt, i);
        s.append((char16_t)code);
    }

    jclass Integer_class = env->FindClass("java/lang/Integer");
    jmethodID Integer_init = env->GetMethodID(Integer_class, "<init>", "(I)V");

    auto sc = nlang::Scanner::New(nlang::TokenStream::New(nlang::UString(std::move(s))));
    while (sc->NextTokenLookahead().token != nlang::Token::THE_EOF) {
        auto& token = sc->NextToken();
        env->CallObjectMethod(tokens_map, TreeMap_put, env->NewObject(Integer_class, Integer_init, (jint)token.pos), env->NewStringUTF(nlang::TokenUtils::GetTokenName(token.token).GetStdStr().c_str()));
    }
}
