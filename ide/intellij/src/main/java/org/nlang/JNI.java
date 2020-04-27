package org.nlang;

import java.lang.CharSequence;
import java.util.TreeMap;
import java.net.URL;
import java.io.File;
import java.io.InputStream;
import java.io.IOException;
import java.nio.file.Files;

class JNI {
    static {
        try {
            String prefix = "";
            String suffix = "";
            String os = System.getProperty("os.name").toLowerCase();
            if (os.contains("win")) {
                prefix = "";
                suffix = ".dll";
            } else if (os.contains("nix") || os.contains("nux") || os.contains("aix")) {
                prefix = "lib";
                suffix = ".so";
            } else if (os.contains("mac")) {
                prefix = "lib";
                suffix = ".dylib";
            }

            String libName = prefix + "nlang_jni" + suffix;
            InputStream in = JNI.class.getClassLoader().getResourceAsStream("/native/lib/" + libName);
            File tmpDir = Files.createTempDirectory("nlang_jni_tmp").toFile();
            tmpDir.deleteOnExit();
            File nativeLibTmpFile = new File(tmpDir, libName);
            nativeLibTmpFile.deleteOnExit();
            Files.copy(in, nativeLibTmpFile.toPath());
            System.load(nativeLibTmpFile.getAbsolutePath());
        } catch (IOException e) {}
    }

    public static native void tokenize(CharSequence source, int start, int end, TreeMap<Integer, String> tokens);
}