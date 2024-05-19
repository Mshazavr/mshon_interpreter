# Mshon - A Toy Interpreted Language 

## Description 

Mshon is toy interpreted language written in C. Here is an example code in Mshon 

```
fn fib(i) {
    if i - 0 {
        if i - 1 {
            return fib(i-1)+fib(i-2);
        }
        else {
            return 1;
        }
    }
    else {
        return 1;
    }
}

print fib(5);

fn sum(a, b) { 
    return a + b;
} 

let x = 6;
let y = x;
y = y + 1;

print -1 * (fib(6) + fib(7));
```

Output: 

```
8
-34
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

