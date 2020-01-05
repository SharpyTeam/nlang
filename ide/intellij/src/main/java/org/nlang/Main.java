package org.nlang;

import java.util.Map;
import java.util.TreeMap;

public class Main {
    public static void main(String[] args) {
        TreeMap<Integer, String> m = new TreeMap<>();
        String s = "kek лул \"asdfas\"\"\"\" kek";
        JNI.tokenize(s, 0, s.length(), m);
        for (Map.Entry<Integer, String> e : m.entrySet()) {
            System.out.println(e.getKey() + " " + e.getValue());
        }
    }
}
