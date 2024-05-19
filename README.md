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

