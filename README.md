# Mshon - A Toy Interpreted Language 

## Description 

Mshon is toy interpreted language written in C. Here is an example code in Mshon 

```
fn fib(i) {
    imagine i - 0 {
        imagine i - 1 {
            checkit fib(i-1)+fib(i-2);
        }
        bummer {
            checkit 1;
        }
    }
    bummer {
        checkit 1;
    }
}

vomit fib(5);

fn sum(a, b) { 
    checkit a + b;
} 

suppose x = 6;
suppose y = x;
y = y + 1;

vomit -1 * (fib(6) + fib(7));
```

Output: 

```bash
8
-34
``` 

### Error handling
Mshon also supports simple syntax and semantic error handling 

#### Syntax error examples

Input:
```
suppose 34 = 4;
```

Output:
```bash
error message: Syntex error: Expected IDENTIFIER. Instead got: NUMERIC_LITERAL[34]
```

Input:
```
suppose x = 4
vomit x;
```

Output:
```bash
error message: Syntex error: Expected SEMICOLON. Instead got: PRINT
```


#### Semantic error examples

Input:
```
suppose x = 4
vomit x();
```

Output:
```bash
error message: Variable is not callable: x
```

Input:
```
suppose x = 4
vomit x + y - 1;
```

Output:
```bash
error message: Undeclared Identifier: y
```

## Installation 

```bash
make 
make test
```

## Usage 

Running tests

```bash
bin/test
```

Running mshon 

```bash 
bin/mshon path/to/script.shr
```

