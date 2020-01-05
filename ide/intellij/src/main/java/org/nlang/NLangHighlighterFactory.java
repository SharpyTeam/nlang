package org.nlang;

import com.intellij.lexer.Lexer;
import com.intellij.openapi.editor.DefaultLanguageHighlighterColors;
import com.intellij.openapi.editor.HighlighterColors;
import com.intellij.openapi.editor.colors.TextAttributesKey;
import com.intellij.openapi.fileTypes.SyntaxHighlighter;
import com.intellij.openapi.fileTypes.SyntaxHighlighterFactory;
import com.intellij.openapi.project.Project;
import com.intellij.openapi.vfs.VirtualFile;
import com.intellij.psi.tree.IElementType;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.util.HashMap;
import java.util.Map;

public class NLangHighlighterFactory extends SyntaxHighlighterFactory {
    @NotNull
    @Override
    public SyntaxHighlighter getSyntaxHighlighter(@Nullable Project project, @Nullable VirtualFile virtualFile) {
        return new SyntaxHighlighter() {
            private TextAttributesKey[] restAttributesKeys = new TextAttributesKey[] { DefaultLanguageHighlighterColors.PREDEFINED_SYMBOL };

            private Map<String, TextAttributesKey[]> attributesKeys = new HashMap<String, TextAttributesKey[]>() {
                {
                    put("INVALID", new TextAttributesKey[] { HighlighterColors.BAD_CHARACTER });

                    put("STRING", new TextAttributesKey[] { DefaultLanguageHighlighterColors.STRING });
                    put("IDENTIFIER", new TextAttributesKey[] { DefaultLanguageHighlighterColors.IDENTIFIER });
                    put("COMMENT", new TextAttributesKey[] { DefaultLanguageHighlighterColors.BLOCK_COMMENT });
                    put("NUMBER", new TextAttributesKey[] { DefaultLanguageHighlighterColors.NUMBER });

                    put("LEFT_PAR", new TextAttributesKey[] { DefaultLanguageHighlighterColors.PARENTHESES });
                    put("RIGHT_PAR", new TextAttributesKey[] { DefaultLanguageHighlighterColors.PARENTHESES });
                    put("LEFT_BRACE", new TextAttributesKey[] { DefaultLanguageHighlighterColors.BRACES });
                    put("RIGHT_BRACE", new TextAttributesKey[] { DefaultLanguageHighlighterColors.BRACES });
                    put("COMMA", new TextAttributesKey[] { DefaultLanguageHighlighterColors.COMMA });
                    put("SEMICOLON", new TextAttributesKey[] { DefaultLanguageHighlighterColors.SEMICOLON });

                    put("IF", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("ELSE", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("FOR", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("WHILE", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("LOOP", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("FN", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("LET", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("CONST", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("RETURN", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("CONTINUE", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("BREAK", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("THE_NULL", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("TRUE", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("FALSE", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("AND", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("OR", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("XOR", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                    put("NOT", new TextAttributesKey[] { DefaultLanguageHighlighterColors.KEYWORD });
                }
            };

            private Lexer lexer = new NLangLexer();

            @NotNull
            @Override
            public Lexer getHighlightingLexer() {
                return lexer;
            }

            @NotNull
            @Override
            public TextAttributesKey[] getTokenHighlights(IElementType tokenType) {
                TextAttributesKey[] attributesKeys = this.attributesKeys.get(tokenType.toString());
                if (attributesKeys == null) {
                    return restAttributesKeys;
                }
                return attributesKeys;
            }
        };
    }
}
