# Insider documentation

Retro uses a recursive descent parser. It is left associative.

## Retro syntax in BNF

*	`* {...}*` : 0 or more times
*	`+ {...}+`: 1 or more times
*	`|` : choice (one in the list)
*	`-` : sequence
*	`{...}` : grouping (exactly 1 time)
*	`[...]` : optionnal (0 or 1 time)

```
letter      := {'A'-'Z'} | {'a'-'z'}.
digit       := {'0'-'9'}.
hexadigit   := {'0'-'9'} | {'A'-'F'} | {'a'-'f'}.
identifier  := {letter | '_'} {letter | digit}*.
integer     := ['+'|'-'] {'1'-'9'} {'0'-'9'}*.
hexinteger  := '0x' hexadigit+.
bininteger  := '0b' {'0'-'1'}+.
real        := ['+'|'-'] {'0' | {{'1'-'9'} {'0'-'9'}*}} '.' {0-9}+ [ {'E' | 'e'} ['+' | '-'] {'1'-'9'} {'0'-'9'}* ].
complex     := real 'i'.
number      := integer | hexinteger | bininteger | real.

factor      := identifier | number.
product     := factor {'*' | '.*' | '/' | '.*' | '.' | '\'} factor.
sum         := product {'+' | '-'} product.
range       := sum ':' sum [ ':' sum ].
logicalterm := range {'!' | '>' | '>=' | '<' | '<=' | '==' | '!=' | '~=' | '_' | '|'} range
logicalexpr := logicalterm {'||' | '&&'} logicalterm.
expr        := { '{' logicalexpr '}' } | logicalexpr.

```

## Elements of the language

There are 3 levels in functions

*	**commands** which implement the core keywords of the language.
	These keywords are are listed in the **command_list** array.
	
	```
	typedef struct {
	  char *name;				/* name of the command */
	  comtyp nr;				/* command category */
	  void (*f)(void);			/* implementation */
	} commandtyp;
	
	commandtyp command_list[] = {
	  {"quit",c_quit,do_quit},
	  ...
	  {"hexdump",c_global,do_hexdump},
	  ...
	  {0,0,0}
	};

	```
	
*	**builtins** functions that use the Retro API explained in the
	[extend documention](extend.md).
	
	a builtin function is run through the call
	
	```
	int exec_builtin(char *name, int nargs, header *hd);
	```
	
	with
	
	`name` : the name of the function
	
	`nargs` : the number of arguments
	
	`hd` : a pointer to the first parameter on the stack

	The function checks for existence by name and number of 
	arguments, so that 2 function can have the same name if they
	have a different numberof arguments.
	
	Results may be pushed on the stack.
	
	The function itself returns a boolean if the function was executed.
	
	
*	**user defined functions**

## Use of the stack


*	Organization of the stack

	```
	----------------------------- <-- ramend
	   
	   
	            free
	   
	
	----------------------------- <-- newram = endlocal (top of stack)
	running function local scope
	----------------------------- <-- startlocal     [endlocal]
	global variables
	----------------------------- <-- udfend         [startlocal]
	udf (user defined functions)                          ^
	----------------------------- <-- ramstart            |
                                                     when global
	                                                    scope
	```

*	Calling a function: the callee must push parameters on the stack

*	Evaluating expression

	When evaluating expressions, the expression is parsed.
	
	*	When encountering **identifiers** a `s_reference` object 
		is pushed on the stack.
	*	When encoutering a binary operator, the second operand is
		parsed and pushed on the stack. The recursive structure of
		the parser enforces precedence of some operators on others.
		
		Then the operator is evaluated. For example,
		
		```
		void add (header *hd, header *hd1)
		{	header *result,
			       *st=hd;		// remind the stack pointer
		            			//         <-- newram
		            			//   *hd1
		            			//   *hd   <-- st
			hd=getvalue(hd);	// get the actual value: reference
			                	//   is evaluated. Here an error
			                	//   can occur if the variable
			                	//   does not exist.
			hd1=getvalue(hd1);	// same for operand 2
			
			// evaluate the operator, push the result onto the stack
			//            <-- newram
			//   *result
			//   *hd1
			//   *hd      <-- st
			result=map2(real_add,complex_add,hd,hd1);
			
			// eliminate *hd and *hd1 from the stack (and lower newram)
			//           <-- newram
			//   *result <-- st
			moveresult(st,result);
		}
		```
