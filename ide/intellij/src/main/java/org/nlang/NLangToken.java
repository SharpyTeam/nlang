package org.nlang;

import com.intellij.psi.tree.IElementType;
import org.jetbrains.annotations.NotNull;

public class NLangToken extends IElementType {
    public NLangToken(@NotNull String debugName) {
        super(debugName, NLang.INSTANCE);
    }
}
