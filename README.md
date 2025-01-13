# Bootstrap C into basic Java

how to use this program:

```
$ make 
$ ./j2c INPUTFILE OUTPUTFILE
```

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

we have three accessibility: public > default > private.

NOTICE: only java style array declaration (int[] a) is allowed.

```
<variableDeclaration> ::=  
[<accessModifier>] {<nonAccessModifier>} <type> { '[' ']' } ( <identifier> | <assignment>) {',' ( <identifier> | <assignment>)} 

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
                 | ('?'| <identifier> ) 'extends' <referenceType>
                 | ('?'| <identifier> ) 'super' <referenceType>
 
<primitiveType> ::= 'char' | 'int' | 'long' | 'double' | 'boolean' 
```

#### assignment

the `<term>` in `<assignment>` must be a valid left value.

```
<assignment> ::= <term> <assignmentOperator> <expression>

<assignmentOperator> ::= '=' | '+=' | '-=' | '*=' | '/='
```

#### expression and term

expression refers to a snippet of code that has a value. this could be a boolean value, other primitive type value or reference type value.

we don't check if an expression is boolean or not until semantics analysis.

`<number>` , `character`, `<identifier>`and `<string>` are dealt with during lexing.

```
<expression> ::= <ternaryExpression>

<ternaryExpression> ::= <logicalOrExpression> [ '?' <expression> ':' <expression> ]

<logicalOrExpression> ::= <logicalAndExpression> { '||' <logicalAndExpression> }

<logicalAndExpression> ::= <bitwiseOrExpression> { '&&' <bitwiseOrExpression> }

<bitwiseOrExpression> ::= <bitwiseXorExpression> { '|' <bitwiseXorExpression> }

<bitwiseXorExpression> ::= <bitwiseAndExpression> { '^' <bitwiseAndExpression> }

<bitwiseAndExpression> ::= <equalityExpression> { '&' <equalityExpression> }

<equalityExpression> ::= <relationalExpression> [ ('==' | '!=') <relationalExpression> ]

<relationalExpression> ::= <shiftExpression> [ ('<' | '<=' | '>' | '>=') <shiftExpression> ]
                         | <shiftExpression> <generics> // this is to simplify parser
                         | <shiftExpression> 'instanceof' <referenceType>

<shiftExpression> ::= <additiveExpression> { ('<<' | '>>') <additiveExpression> }

<additiveExpression> ::= <multiplicativeExpression> { ('+' | '-') <multiplicativeExpression> }

<multiplicativeExpression> ::= <castExpression> { ('*' | '/' | '%') <castExpression> }

<castExpression> ::= '(' <type> ')' <expression> | <unaryExpression>

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
<term> ::= 'true' | 'false' | 'null' | 'this' | 'super'
		 | <number>
		 | <character>
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
<newObject> ::= 'new' <type> '(' <expressionList> ')' // constructor call
              | 'new' <type> { '[' <expression> ']' } { '[' ']'} // array
              | 'new' <type> '{' <classBody> '}' // anonymous class
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

if the subroutine does not have a return type or void, it's a constructor.

For current version of grammar we only use annotations for method overridence.

```
<subroutineDeclaration> ::= [<annotation>] [<accessModifier>] {<nonAccessModifier>} [`native`] [<typeBounds>] [ ( <type> {'[' ']'} | 'void' ) ] <identifier> '(' <parameterList> ')' '{' <subroutineBody> '}'

<parameterList> ::= [ ['final'] <type> { '[' ']' } <identifier> { ',' ['final'] <type> { '[' ']' } <identifier> }]

<typeBoundsList> ::= '<' <typeBound> {',' <typeBound>} '>'
<typeBound> ::= <identifier> [ <constraint> ]
<constraint> ::= 'extends' <type> {'&' <type>}

<subroutineBody> ::= [ { ( <variableDeclaration> ';' ) | <statement> } ]

<annotation> ::= '@' <identifier>
```

#### flow control

we will wait until semantics analysis to check if the expression are boolean or not inside those brackets.

for simplicity we don't allow compound statements without braces.

```
<statement> ::= { <variableDeclaration> ';' | <assignment> ';' | <expression> ';' | <ifStatement> | <switchStatement> | <forStatement> | <whileStatement> |  <doWhileStatement> | <returnStatement> | <breakStatement> | <continueStatement> | <staticStatement> | <codeBlock> | ';'}

<ifStatement> ::= 'if' '(' <expression> ')' '{' [<statement>] '}' { 'else' 'if' '(' <expression> ')' '{' [<statement>] '}' } [ 'else' '{' [<statement>] '}' ]

<switchStatement> ::= 'switch' '(' <expression> ')' '{' ( [<defaultBranch>] {<caseBranch>} | {<caseBranch>} [<defaultBranch>] {<caseBranch>} | {<caseBranch>} [<defaultBranch>] ) '}'

<caseBranch> ::= 'case' <expression> ':' { <statement> }
<defaultBranch> ::= 'default' ':' { <statement> }

<forStatement> ::= 'for' '(' [ <variableDeclaration> | <assignment> ] ';' [ <expression> ] ';' [ <assignment> | <expression> ] ')' ( '{' { <statement> } '}' | ';' )

<whileStatement> ::= 'while' '(' <expression> ')' ( '{' { <statement> } '}' | ';' )

<doWhileStatement> ::= 'do' '{' { <statement> } '}' 'while' '(' <expression> ')' ';'

<returnStatement> ::= 'return' [ <expression> ] ';'

<breakStatement> ::= 'break' ';'

<continueStatement> ::= 'continue' ';'

<staticStatement> ::= 'static' '{' { <statement> } '}'

<codeBlock> ::= '{' { <statement> } '}'
```

#### class declaration and polymorphism

no inner classes and initializer blocks allowed.

```
<classDeclaration> ::= [<accessModifier>] [<nonAccessModifier>] ['abstract'] 'class' <identifier> [ <generics> ] ['extends' <identifier> [ <generics> ] ] ['implements' <identifier> [ <generics> ] { ',' <identifier> [ <generics> ] }] '{' <classBody> '}'

<classBody> ::= { <variableDeclaration> | <subroutineDeclaration> }

<interfaceDeclaration> ::= [<accessModifier>] 'interface' <identifier> [ <generics> ] ['extends' <identifier> [ <generics> ] { ',' <identifier> [ <generics> ] }] '{' <interfaceBody> '}'

<interfaceBody> ::= { <subroutinePrototype> }

<subroutinePrototype> ::= [<annotation>] ['public'] ['abstract'] [<typeBounds>] [ ( <type> {'[' ']'} | 'void' ) ] <identifier> '(' <parameterList> ')' ';'

```

#### file structure

no package. 

our import goes like `import home.kj24716.j2c.testfolder.*`. this will import all .java files in that folder recursively.

```
<file> ::= {<importStatement>} {<classDeclaration> | <interfaceDeclaration>}

<importStatement> ::= 'import' <identifier> {'.' <identifier>} ['.*'] ';'
```

#### advanced techniques

method reference, enhanced for, lambda expressions, varargs, multi-thread, reflection and exceptions (`try-catch-finally` and `throws`) are not included in this grammar to keep it simple.

## Symbol tables

before semantics, we attach symbol tables and virtual tables to class tree nodes.



castExpression
## Semantics


#### method overriding


if two methods share name, argument type and order, type boundedness, then they can override.

overrides:

`void method(String str)` and `void method(String s)`
`<K extends Number> void method(K k)` and `<T extends Number> void method(T t)`


does not override:

`<K extends Number> void method(K k)` and `<K extends Object> void method(K k)`


overrides in java, but i want to simplify my code so they are invalid:

`<K extends List<E>, E xtends Number> void method(K k)` and `<K extends List<V>, V extends Number> void method(K k)`


#### modifiers


interface: 

access: can only be `public` or `default`. 

non-access: implicitly `abstract`, cannot be declared explicitly.

interface methods: implicitly `public` and `abstract`. can be declared explicitly.


## Compile

does not compile .c and .h file for a .java file in /system folder, but refer to its corresponding .c and .h file.



#### reminders

