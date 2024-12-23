# Bootstrap C into basic Java

## Lexing

keywords: 

`char, int, long, double, boolean`, 

`for, while, do, switch, case, default, if, else, continue, break, return`,

`class, abstract, interface, extends, implements, this, super, new, instanceof`,

`public, private`, `try, catch, finally, throw, throws`, 

`static, final`, `true, false, null`, `void`, 

`import`,

`native`

symbols: `+`, `-`, `*`, `/`, `%`, `?`, `:`, `|`, `&`, `!`, `~`, `^`, `>`, `<`, `=`, `@`, `.`

brace and bracket: `{`, `}`, `(`, `)`

number: any number that can begin with an optional minus sign and a decimal point.

identifier.

string literals.

semicolon.


## parsing

#### variable declaration

this includes arrays.

if no access modifier, default is public. we implement a simpler version: only public and private.

```
<variableDeclaration> ::=  
[<accessModifier>] {<nonAccessModifier>} <type> [ '[' ']' ] ( <identifier> | <assignment>) {',' ( <identifier> | <assignment>)} 

<accessModifier> ::= 'public' | 'private'
<nonAccessModifier> ::= 'static' | 'final' | 'abstract'
```

#### types

```
<type> ::= <referenceType> | <primitiveType>

<referenceType> ::= <identifier> [ <generics> ]

<generics> ::= '<' <typeArgument> { ',' <typeArgument> } '>'

<typeArgument> ::= <referenceType> 
				 | '?'
                 | '?' 'extends' <referenceType>
                 | '?' 'super' <referenceType>

<primitiveType> ::= 'char' | 'int' | 'long' | 'double' | 'boolean' 
```

#### assignment

```
<assignment> ::= <identifier> [ '[' <expression> ']' ] <assignmentOperator> <expression>

<assignmentOperator> ::= '=' | '+=' | '-=' | '*=' | '/='
```

#### expression and term

expression refers to a snippet of code that has a value. this could be a boolean value, other primitive type value or reference type value.

we don't check if an expression is boolean or not until semantics analysis.

`<number>` , `<identifier>`and `<string>` are dealt with during lexing.

```
<expression> ::= <ternaryExpression>

<ternaryExpression> ::= <logicalOrExpression> [ '?' <expression> ':' <expression> ]

<logicalOrExpression> ::= <logicalAndExpression> { '||' <logicalAndExpression> }

<logicalAndExpression> ::= <bitwiseOrExpression> { '&&' <bitwiseOrExpression> }

<bitwiseOrExpression> ::= <bitwiseXorExpression> { '|' <bitwiseXorExpression> }

<bitwiseXorExpression> ::= <bitwiseAndExpression> { '^' <bitwiseAndExpression> }

<bitwiseAndExpression> ::= <equalityExpression> { '&' <equalityExpression> }

<equalityExpression> ::= <relationalExpression> { ('==' | '!=') <relationalExpression> }

<relationalExpression> ::= <shiftExpression> [ ('<' | '<=' | '>' | '>=') <shiftExpression> ]
                         | <shiftExpression> <generics> // this is to simply parser
                         | <shiftExpression> 'instanceof' <shiftExpression>

<shiftExpression> ::= <additiveExpression> { ('<<' | '>>') <additiveExpression> }

<additiveExpression> ::= <multiplicativeExpression> { ('+' | '-') <multiplicativeExpression> }

<multiplicativeExpression> ::= <castExpression> { ('*' | '/' | '%') <castExpression> }

<castExpression> ::= '(' <type> ')' <castExpression> | <unaryExpression>

<unaryExpression> ::= <unaryOperator> <unaryExpression>
                    | <selfOperator> <term>   // Prefix: ++x, --y
                    | <postfixExpression>
                    
<postfixExpression> ::= <term> <selfOperator>   // Postfix: x++, y--
                      | <term>
```

| Precedence   | Operators                                  | Grammar Rule                 | Associativity |
| ------------ | ------------------------------------------ | ---------------------------- | ------------- |
| 1 (Lowest)   | `? :`                                      | `<ternaryExpression>`        | Right-to-left |
| 2            |  `\|\|`                                    | `<logicalOrExpression>`      | Left-to-right |
| 3            | `&&`                                       | `<logicalAndExpression>`     | Left-to-right |
| 4            | `\|`                                       | `<bitwiseOrExpression>`      | Left-to-right |
| 5            | `^`                                        | `<bitwiseXorExpression>`     | Left-to-right |
| 6            | `&`                                        | `<bitwiseAndExpression>`     | Left-to-right |
| 7            | `==`, `!=`                                 | `<equalityExpression>`       | Left-to-right |
| 8            | `<`, `<=`, `>`, `>=`, `instanceof`         | `<relationalExpression>`     | Left-to-right |
| 9            | `<<`, `>>`                                 | `<shiftExpression>`          | Left-to-right |
| 10           | `+`, `-`                                   | `<additiveExpression>`       | Left-to-right |
| 11           | `*`, `/`, `%`                              | `<multiplicativeExpression>` | Left-to-right |
| 12           | casting                                    | `<castExpression>`           | Left-to-right |
| 13           | `!`, `~`, `-` (unary) `++`, `--` (prefix)  | `<unaryExpression>`          | Right-to-left |
| 14 (Highest) | `.`, `()`, `[]`, `++`, `--` (postfix)      | `<term>`                     | Left-to-right |


```
<term> ::= 'true' | 'false' | 'null' | 'this'
		 | <number>
		 | <string>
		 | <identifier>
		 | '(' <expression> ')'
		 | <subroutineCall>
		 | <fieldAccess>
		 | <arrayAccess>
		 | <newObject>
		 | <arrayInitialization>
		 
<arrayAccess> ::= <term> '[' <expression> ']' 
<fieldAccess> ::= <term> '.' <identifier>
<subroutineCall> ::= (<fieldAccess> | <identifier>) '(' <expressionList> ')'
<newObject> ::= 'new' <type> [ <generics> ] '(' <expressionList> ')' // constructor call
              | 'new' <type> [ <generics> ] { '[' <expression> ']' } { '[' ']'} [ <arrayInitialization> ] // array
<arrayInitialization> ::= '{' [ <term> {',' <term>} ] '}' | '{' [ <arrayInitialization> {',' <arrayInitialization>} ] '}'

<expressionList> ::= [ <expression> {',' <expression> } ]

<binaryOperator> ::= '+' | '-' | '*' | '/' | '%' | '^' | '&' | '|' | '<<' | '>>'
<unaryOperator> ::= '!' | '-' | '~'
<selfOperator> ::= '++' | '--'
<logicalOperator> ::= '==' | '!=' | '>' | '>=' | '<' | '<=' | '&&' | '||'
```

subroutine call is merely calling a function. 

#### method declaration and body

method declaration should include a `native` modifier for methods that are realized using lower level programming and tell the code generator to look for its source C code.

```
<subroutineDeclaration> ::= [<accessModifier>] {<nonAccessModifier>} [`native`] ( <type> | 'void' ) <identifier> '(' <parameterList> ')' '{' <subroutineBody> '}'

<parameterList> ::= [ <type> <identifier> { ',' <type> <identifier> }]

<subroutineBody> ::= [ { ( <variableDeclaration> ';' ) | <statement> } ]
```

#### flow control

we will wait until semantics analysis to check if the expression are boolean or not inside those brackets.

for simplicity we don't allow compound statements without braces.

```
<statement> ::= { <assignment> ';' | <expression> ';' | <ifStatement> | <switchStatement> | <forStatement> | <whileStatement> |  <doWhileStatement> | <returnStatement> | <breakStatement> | <continueStatement> | ';'}

<ifStatement> ::= 'if' '(' <expression> ')' '{' <statement> '}' { 'else' 'if' '(' <expression> ')' '{' <statement> '}' } [ 'else' '{' <statement> '}' ]

<switchStatement> ::= 'switch' '(' <identifier> ')' '{' ( [<defaultBranch>] {<caseBranch>} | {<caseBranch>} [<defaultBranch>] {<caseBranch>} | {<caseBranch>} [<defaultBranch>] ) '}'
<caseBranch> ::= 'case' <expression> ':' [<statement>] [<breakStatement>]
<defaultBranch> ::= 'default' ':' <statement> [<breakStatement>]

<forStatement> ::= 'for' '(' <assignment> ';' <expression> ';' <assignment> ')' ( '{' [<statement>] '}' | ';' )

<whileStatement> ::= 'while' '(' <expression> ')' ( '{' [<statement>] '}' | ';' )

<doWhileStatement> ::= 'do' '{' <statement> '}' 'while' '(' <expression> ')' ';'

<returnStatement> ::= 'return' [<expression>] ';'

<breakStatement> ::= 'break' ';'

<continueStatement>::= 'continue' ';'
```

#### class declaration and polymorphism

no inner classes allowed.

```
<classDeclaration> ::= [<accessModifier>] ['abstract'] 'class' <identifier> ['<' <generics> '>'] ['extends' <identifier> ['<' <generics> '>'] ] ['implements' <identifier> ['<' <generics> '>'] { ',' <identifier> ['<' <generics> '>'] }] '{' <classBody> '}'

<classBody> ::= { <variableDeclaration> | <subroutineDeclaration> }

<interfaceDeclaration> ::= [<accessModifier>] 'interface' <identifier> ['<' <generics> '>'] ['extends' <identifier> ['<' <generics> '>'] { ',' <identifier> ['<' <generics> '>'] }] '{' <interfaceBody> '}'

<interfaceBody> ::= { <subroutineDeclaration> }

```

#### file structure

no package. 

```
<file> ::= {<importStatement>} {<classDeclaration> | <interfaceDeclaration>}

<importStatement> ::= 'import' <identifier> {'.' <identifier>} ['.*'] ';'
```

#### advanced techniques

method reference, lambda expressions, inner classes and exceptions (`try-catch-finally` and `throws`) are not included in this grammar to keep it simple.

#### reminders

new expression needs modification.

after parsing and before semantics:

we need to create symbol tables at proper tree nodes.

we need a second sweep of the cst to combine field access with subroutine call to make sure chained accesses are handled.


