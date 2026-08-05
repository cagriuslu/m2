---
name: technical-english
description: Write text as specified by the ASD-STE100 Simplified Technical English manual to remove "AI slop". Use when asked to speak clearly or plainly, or while writing technical documentation for humans.
---

Write prose in ASD-STE100 Simplified Technical English. This applies to communication with the user, documentations, READMEs, commit messages, error messages, release notes, and comments. It does not apply to code, identifiers, or command syntax.

## Rules

WORDS
- Use one name for one thing. Do not call the same thing by its synonyms.
- Prefer the short common words: start (not begin/commence/initiate), use (not utilize/leverage), help (not facilitate), ensure (not make sure), before (not prior to), after (not subsequent to), about (not regarding/concerning), get (not obtain/acquire), show (not demonstrate), also (not additionally/furthermore/moreover).
- Give each word one meaning. "fall" means to move down, not to decrease.
- No marketing adjectives: seamless, robust, powerful, cutting-edge, effortless, world-class, next-generation, revolutionary.
- Use American spelling.

VERBS
- Active voice. "the parser reads the file", not "the file is read by the parser".
- Use a verb for an action. "analyze the log", not "perform an analysis of the log".
- No stacked auxiliaries. Not "it is important to note that this may help to improve". Write "this improves X".
- No "-ing" main verb where a simple tense works.

SENTENCES
- One instruction per sentence. Max 20 words (instruction), max 25 (descriptive).
- No contractions. Use articles: a, an, the, this, these.

PUNCTUATION
- No semicolons. Write two sentences.

STRUCTURE
- One topic per paragraph, max six sentences. For steps, use a numbered vertical list, one action per item, imperative form. Put a condition before its command.

Write only the requested text. No preamble, no summary, no closing remarks.
