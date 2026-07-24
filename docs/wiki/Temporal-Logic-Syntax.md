# Temporal Logic Syntax

LoomRV accepts past-time temporal formulas. Propositions are enclosed in braces.

## Operators

| Operation | Symbol | Word form | Example |
|---|---|---|---|
| Proposition | `{name}` | - | `{engine_on}` |
| Negation | `!` | `not` | `!{p}` |
| Conjunction | `&&` | `and` | `{p} and {q}` |
| Disjunction | `\|\|` | `or` | `{p} or {q}` |
| Implication | `->` | `implies` | `{p} implies {q}` |
| Once | `P` | `once` | `once({p})` |
| Historically | `H` | `historically` | `historically({p})` |
| Since | `S` | `since` | `{p} since {q}` |

`once` means that a condition held at some point in the relevant past window. `historically` means that it held throughout the relevant past window. `since` requires its left operand to hold continuously since a point where its right operand held.

## Temporal Bounds

Temporal operators may use integer bounds:

| Form | Meaning | Example |
|---|---|---|
| `[a:b]` | Lower and upper bound | `once[3:10]({p})` |
| `[a:]` | Lower bound with no finite upper bound | `{p} since[10:] {q}` |
| `[a:inf]` | Explicit unbounded upper limit | `H[5:inf]({p})` |
| `[:b]` | Upper bound starting from zero | `once[:10]({p})` |

Unbounded operators are equivalent to bounds beginning at zero with an infinite upper limit.

## Precedence

From highest to lowest:

1. unary operators and parenthesized expressions,
2. `since`,
3. `and`,
4. `or`,
5. `implies`.

Use parentheses whenever a formula's grouping should be unambiguous.

## Naming Rules

A proposition name:

- begins with a letter or underscore,
- continues with letters, digits, or underscores,
- is case-sensitive.

Valid examples:

```text
{p}
{engine_on}
{sensor2}
{_internal}
```

## Formula File Format

The CLI reads one property per non-empty line:

```text
historically({request} implies once[:5]({response}))
once({error})
not({shutdown}) since {started}
```

Verdicts are emitted in the same order as these lines.

## Validation Utility

After building the project, validate one formula with:

```bash
./build/check-grammar "historically({p} implies once[:10]({q}))"
```

Expected output:

```text
Formula is valid!
```

## Notes

- Bounds are parsed as integers by the implementation.
- Negative numbers are accepted syntactically but are not meaningful temporal windows for normal use.
- Prefix negation applies directly to an atom or parenthesized expression.
- The grammar is for past-time operators; future-time operators are not supported.
- Temporal subformulas share computation only when their operators, operands, and bounds are identical. Overlapping but different bounds are evaluated by distinct nodes.
