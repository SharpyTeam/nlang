package org.nlang;

import com.intellij.lang.Language;

public class NLang extends Language {
    public static final NLang INSTANCE = new NLang();

    private NLang() {
        super("nlang");
    }
}
