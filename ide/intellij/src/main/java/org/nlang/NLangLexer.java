package org.nlang;

import com.intellij.lexer.Lexer;
import com.intellij.lexer.LexerPosition;
import com.intellij.psi.TokenType;
import com.intellij.psi.tree.IElementType;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.util.Map;
import java.util.TreeMap;

public class NLangLexer extends Lexer {
    private CharSequence charSequence;

    private TreeMap<Integer, IElementType> tokens = new TreeMap<>();
    private int currentTokenPos;
    private int bufferEnd;

    @Override
    public void start(@NotNull CharSequence buffer, int startOffset, int endOffset, int initialState) {
        charSequence = buffer;
        bufferEnd = endOffset;
        currentTokenPos = startOffset;

        tokens.subMap(startOffset, endOffset).clear();
        TreeMap<Integer, String> m = new TreeMap<>();
        JNI.tokenize(charSequence, startOffset, endOffset, m);
        for (Map.Entry<Integer, String> entry : m.entrySet()) {
            tokens.put(entry.getKey(), new NLangToken(entry.getValue()));
        }
    }

    @Override
    public int getState() {
        return 0;
    }

    @Nullable
    @Override
    public IElementType getTokenType() {
        return currentTokenPos < bufferEnd ? tokens.get(currentTokenPos) : null;
    }

    @Override
    public int getTokenStart() {
        return currentTokenPos;
    }

    @Override
    public int getTokenEnd() {
        Integer k = tokens.higherKey(currentTokenPos);
        return k == null ? charSequence.length() : k;
    }

    @Override
    public void advance() {
        Integer k = tokens.higherKey(currentTokenPos);
        currentTokenPos = k == null ? charSequence.length() : k;
    }

    @NotNull
    @Override
    public LexerPosition getCurrentPosition() {
        return new LexerPosition() {
            private int offset = currentTokenPos;

            @Override
            public int getOffset() {
                return offset;
            }

            @Override
            public int getState() {
                return 0;
            }
        };
    }

    @Override
    public void restore(@NotNull LexerPosition position) {
        currentTokenPos = position.getOffset();
    }

    @NotNull
    @Override
    public CharSequence getBufferSequence() {
        return charSequence;
    }

    @Override
    public int getBufferEnd() {
        return bufferEnd;
    }
}
