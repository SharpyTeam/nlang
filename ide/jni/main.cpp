//
// Created by selya on 05.01.2020.
//

#include "org_nlang_JNI.h"

#include <scanner.hpp>

#include <jni.h>
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

static size_t utf32_to_utf8(uint8_t* buffer, uint32_t code) {
    if (code <= 0x7Fu) {
        buffer[0] = code;
        return 1;
    }
    if (code <= 0x7FFu) {
        buffer[0] = 0xC0u | (code >> 6u);               /* 110xxxxx */
        buffer[1] = 0x80u | (code & 0x3Fu);             /* 10xxxxxx */
        return 2;
    }
    if (code <= 0xFFFF) {
        buffer[0] = 0xE0u | (code >> 12u);              /* 1110xxxx */
        buffer[1] = 0x80u | ((code >> 6u) & 0x3Fu);     /* 10xxxxxx */
        buffer[2] = 0x80u | (code & 0x3Fu);             /* 10xxxxxx */
        return 3;
    }
    if (code <= 0x10FFFF) {
        buffer[0] = 0xF0u | (code >> 18u);              /* 11110xxx */
        buffer[1] = 0x80u | ((code >> 12u) & 0x3Fu);    /* 10xxxxxx */
        buffer[2] = 0x80u | ((code >> 6u) & 0x3Fu);     /* 10xxxxxx */
        buffer[3] = 0x80u | (code & 0x3Fu);             /* 10xxxxxx */
        return 4;
    }
    return 0;
}

void Java_org_nlang_JNI_tokenize(JNIEnv* env, jclass, jobject source, jint start, jint end, jobject tokens_map) {
    jclass CharSequence_class = env->FindClass("java/lang/CharSequence");
    jmethodID CharSequence_length = env->GetMethodID(CharSequence_class, "length", "()I");
    jmethodID CharSequence_charAt = env->GetMethodID(CharSequence_class, "charAt", "(I)C");

    jclass TreeMap_class = env->FindClass("java/util/TreeMap");
    jmethodID TreeMap_put = env->GetMethodID(TreeMap_class, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    jint length = env->CallIntMethod(source, CharSequence_length);

    uint8_t buf[4];
    std::vector<jint> relation;
    std::string s;
    s.reserve(end - start);
    for (jint i = start; i < end; ++i) {
        jchar code = env->CallCharMethod(source, CharSequence_charAt, i);
        size_t count = utf32_to_utf8(buf, uint32_t(code));
        for (size_t j = 0; j < count; ++j) {
            s += buf[j];
            relation.push_back(i);
        }
    }

    jclass Integer_class = env->FindClass("java/lang/Integer");
    jmethodID Integer_init = env->GetMethodID(Integer_class, "<init>", "(I)V");

    auto sc = nlang::Scanner::Create(nlang::CharStream::Create<nlang::StringCharStream>(s));
    while (sc->NextTokenLookahead().token != nlang::Token::THE_EOF) {
        auto token = sc->NextToken(nlang::Scanner::AdvanceBehaviour::NO_IGNORE);
        auto j_pos = relation[token.pos];
        env->CallObjectMethod(tokens_map, TreeMap_put, env->NewObject(Integer_class, Integer_init, j_pos), env->NewStringUTF(nlang::TokenUtils::TokenToString(token.token).c_str()));
    }
}