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

fn print_fib(left, right) {
    imagine left - right {
        vomit - fib(left) * (50 + 50);
        suppose exit = print_fib(left+1, right);
        checkit exit;
    }
    bummer {
        vomit - fib(left) * (50 + 50);
        checkit 0;
    }
}

suppose exit = print_fib(3, 10);
```

Output: 

```bash
-300
-500
-800
-1300
-2100
-3400
-5500
-8900
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

