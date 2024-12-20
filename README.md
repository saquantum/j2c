# Bootstrap C into basic Java

## Lexing

keywords: 

`char, int, long, double, boolean`, 

`for, while, do, switch, case, default, if, else, continue, break, return`,

`class, abstract, interface, extends, implements, this, super, new, instanceof`,

`public, private`, `try, catch, finally, throw`, 

`static, final`, `true, false, null`, 

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
  [<accessModifier>] {<nonAccessModifier>} <type> ( <identifier> | <assignment>) {',' ( <identifier> | <assignment>)} 
| [<accessModifier>] {<nonAccessModifier>} <type> '[' ']' (<identifier> | <assignment>) {',' (<identifier> | <assignment>)}
```

#### access modifier

```
<accessModifier> ::= 'public' | 'private'
```

#### nonaccess modifier

```
<nonAccessModifier> ::= 'static' | 'final' | 'abstract'
```

#### types

```
<type> ::= <referenceType> | <primitiveType>

<referenceType> ::= <identifier> [ '<' <generics> '>' ]

<generics> ::= <typeArgument> { ',' <typeArgument> }

<typeArgument> ::= <referenceType> 
				 | '?'
                 | '?' 'extends' <referenceType>
                 | '?' 'super' <referenceType>

<primitiveType> ::= 'char' | 'int' | 'long' | 'double' | 'boolean' 
```

#### assignment

```
<assignment> ::= <identifier> [ '[' <expression> ']' ] <assignmentOperator> ['new'] ( <expression> | <type> '[' <expression> ']' ) 
			   | <selfOperator> <identifier> 
			   | <identifier> <selfOperator>

<assignmentOperator> ::= '=' | '+=' | '-=' | '*=' | '/='
```

#### expression

expression refers to a snippet of code that has a value. this could be a boolean value, other primitive type value or reference type value.

we don't check if an expression is boolean or not until semantics analysis.

`<number>` , `<identifier>`and `<string>` are dealt with during lexing.

```
<expression> ::= <term> {(<binaryOperator> | <logicalOperator>) <term>} | | <term> 'instanceof' <type>

<term> ::= 'true' | 'false' | 'null' | 'this'
		 | <number>
		 | <string>
		 | <identifier> [ '[' <expression> ']' ] // can be either a variable or array entry
		 | '(' <expression> ')'
		 | <unaryOperator> <term>
		 | <selfOperator> <identifier>
		 | <identifier> <selfOperator>
		 | <expression> '?' <expression> ':' <expression>
		 | <subroutineCall>
		 
		 
<subroutineCall> ::= <identifier> {'.' <identifier> '(' <expressionList> ')'} '(' <expressionList> ')'

<expressionList> ::= [<expression> {',' <expression> } ]

<binaryOperator> ::= '+' | '-' | '*' | '/' | '%' | '^' | '&' | '|' | '<<' | '>>'
<unaryOperator> ::= '!' | '-' | '~'
<selfOperator> ::= '++' | '--'
<logicalOperator> ::= '==' | '!=' | '>' | '>=' | '<' | '<=' | '&&' | '||'
```

subroutine call is merely calling a function. 

#### method declaration and body

```
<subroutineDeclaration> ::= [<accessModifier>] {<nonAccessModifier>} ( <type> | 'void' ) <identifier> '(' <parameterList> ')' '{' <subroutineBody> '}'

<parameterList> ::= [ <type> <identifier> { ',' <type> <identifier> }]

<subroutineBody> ::= [ { ( <variableDeclaration> ';' ) | <statement> } ]
```

#### flow control

we will wait until semantics analysis to check if the expression are boolean or not inside those brackets.

for simplicity we don't allow compound statements without braces.

```
<statement> ::= { <assignment> ';' | <subroutineCall> ';' | <ifStatement> | <switchStatement> | <forStatement> | <whileStatement> |  <doWhileStatement> | <returnStatement> }

<ifStatement> ::= 'if' '(' <expression> ')' '{' <statement> '}' { 'else' 'if' '(' <expression> ')' '{' <statement> '}' } [ 'else' '{' <statement> '}' ]

<switchStatement> ::= 'switch' '(' <identifier> ')' '{' ( <defaultBranch> {<caseBranch>} | {<caseBranch>} <defaultBranch> {<caseBranch>} | {<caseBranch>} <defaultBranch> ) '}'
<caseBranch> ::= 'case' <expression> ':' [<statement>] ['break']
<defaultBranch> ::= 'default' ':' <statement> ['break']

<forStatement> ::= 'for' '(' <assignment> ';' <expression> ';' <assignment> ')' ( '{' [<statement>] '}' | ';' )

<whileStatement> ::= 'while' '(' <expression> ')' ( '{' [<statement>] '}' | ';' )

<doWhileStatement> ::= 'do' '{' <statement> '}' 'while' '(' <expression> ')' ';'

<returnStatement> ::= 'return' [<expression>] ';'
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

method reference, lambda expressions, inner classes are not included in this grammar to keep it simple.
