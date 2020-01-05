package org.nlang;

import com.intellij.extapi.psi.PsiFileBase;
import com.intellij.openapi.fileTypes.FileType;
import com.intellij.psi.FileViewProvider;
import org.jetbrains.annotations.NotNull;

import javax.swing.*;

public class NLangFile extends PsiFileBase {
    public NLangFile(@NotNull FileViewProvider viewProvider) {
        super(viewProvider, NLang.INSTANCE);
    }

    @NotNull
    @Override
    public FileType getFileType() {
        return NLangFileType.INSTANCE;
    }

    @Override
    public String toString() {
        return "nlang file";
    }

    @Override
    public Icon getIcon(int flags) {
        return super.getIcon(flags);
    }
}