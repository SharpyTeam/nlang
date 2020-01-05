package org.nlang;

import com.intellij.openapi.fileTypes.LanguageFileType;

import javax.swing.*;

public class NLangFileType extends LanguageFileType {
    public static final NLangFileType INSTANCE = new NLangFileType();

    private NLangFileType() {
        super(NLang.INSTANCE);
    }

    @Override
    public String getName() {
        return "nlang file";
    }

    @Override
    public String getDescription() {
        return "nlang language file";
    }

    @Override
    public String getDefaultExtension() {
        return "n";
    }

    @Override
    public Icon getIcon() {
        return NLangIcons.FILE;
    }
}
