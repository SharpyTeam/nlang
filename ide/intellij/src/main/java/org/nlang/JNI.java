package org.nlang;

import java.lang.CharSequence;
import java.util.TreeMap;

class JNI {
    static {
        System.loadLibrary("nlang_jni");
    }

    public static native void tokenize(CharSequence source, int start, int end, TreeMap<Integer, String> tokens);
}